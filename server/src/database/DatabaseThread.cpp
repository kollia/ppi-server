/**
 *   This file is part of ppi-server.
 *
 *   ppi-server is free software: you can redistribute it and/or modify
 *   it under the terms of the Lesser GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   ppi-server is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   Lesser GNU General Public License for more details.
 *
 *   You should have received a copy of the Lesser GNU General Public License
 *   along with ppi-server.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <limits.h>
#include <iostream>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include <sstream>
#include <algorithm>

#include "DatabaseThread.h"
#include "DefaultChipConfigReader.h"

#include "../logger/lib/LogInterface.h"

#include "../util/URL.h"
#include "../util/Calendar.h"
#include "../util/configpropertycasher.h"
#include "../util/ExternClientInputTemplate.h"

#include "../pattern/server/IClientPattern.h"

using namespace util;
using namespace server;
using namespace ports;

namespace ppi_database
{

DatabaseThread* DatabaseThread::_instance= NULL;

DatabaseThread::DatabaseThread(string dbDir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep)
:	Thread("database", defaultSleep, false),
	m_mCurrent()
{
	bool m_bDbLoaded;
	float newdbafter;
	char* pHostN= NULL;
	char hostname[]= "HOSTNAME";
	string prop;

	prop= "newdbafter";
	newdbafter= (float)properties->getDouble(prop);
	if(newdbafter == 0)
		newdbafter= 15;
	pHostN= getenv(hostname);
	if(pHostN == NULL)
		m_sMeasureName= "noHostDefined";
	else
		m_sMeasureName= *pHostN;
	prop= "waitnewentry";
	m_nCondWait= properties->getUShort(prop, /*warning*/false);
	if(m_nCondWait == 0)
		m_nCondWait= 5;
	prop= "tothindbfilerows";
	m_nReadRows= properties->getUInt(prop, /*warning*/false);
	if(m_nReadRows == 0)
		m_nReadRows= 100;
	m_bError= false;
	m_sptEntrys= auto_ptr<vector<db_t> >(new vector<db_t>());
	m_DBENTRYITEMSCOND= getCondition("DBENTRYITEMSCOND");
	m_DBENTRYITEMS= getMutex("DBENTRYITEMS");
	m_DBCURRENTENTRY= getMutex("DBCURRENTENTRY");
	m_DBMEASURECURVES= getMutex("DBMEASURECURVES");
	m_CHANGINGPOOL= getMutex("CHANGINGPOOL");
	m_DBLOADED= getMutex("DBLOADED");
	m_CHANGINGPOOLCOND= getCondition("CHANGINGPOOLCOND");
	m_sConfDir= confDir;
	m_sWorkDir= dbDir;
	m_nAfter= (unsigned int)(newdbafter * 1000000);
	m_bAnyChanged= false;
	if(m_sWorkDir.substr(m_sWorkDir.length()-1, 1) != "/")
		m_sWorkDir+= "/";
	LOG(LOG_INFO, "Storage database " + m_sWorkDir);
}

void DatabaseThread::initial(string workDir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep)
{
	if(_instance == NULL)
	{
		_instance= new DatabaseThread(workDir, confDir, properties, defaultSleep);
		_instance->start(NULL, false);
	}
}

void DatabaseThread::deleteObj()
{
	delete _instance;
	_instance= NULL;
}

bool DatabaseThread::isDbLoaded() const
{
	bool bRv;

	LOCK(m_DBLOADED);
	bRv= m_bDbLoaded;
	UNLOCK(m_DBLOADED);
	return bRv;
}

db_t DatabaseThread::splitDbLine(const string& line)
{
	struct tm entryTime;
	db_t entry;
	vector<string> columns;
	vector<string>::size_type count;

	columns= ConfigPropertyCasher::split(line, "|");
	count= columns.size();

	entry.bNew= true;
	entry.device= true;
	if(count > 0)
	{
		if(strptime(columns[1].c_str(), "%Y%m%d:%H%M%S", &entryTime) == NULL)
			entry.tm= 0;
		else
			entry.tm= mktime(&entryTime);
		entryTime= *localtime(&entry.tm);
		entry.folder= columns[2];
		entry.subroutine= columns[3];
		entry.identif= columns[4];
		for(vector<string>::iterator iter= (columns.begin() + 5); iter != columns.end(); ++iter)
		{
			string value(ConfigPropertyCasher::trim(&(*iter)[0]));

			if(value != "")
			{
				double dvalue= 0;

				if(entry.identif == "access")
				{
					if(value == "NaN")
						entry.device= false;
					else
						entry.device= true;

				}else
					dvalue= atof(&(*iter)[0]);
				//if(entry.subroutine == "dallas_switch_PIO_1")
				//	cout << ":" << dvalue;
				entry.values.push_back(dvalue);
			}
			//if(entry.subroutine == "dallas_switch_PIO_1")
			//	cout << endl;
		}
	}
	return entry;
}

int DatabaseThread::init(void *args)
{
	char stime[16];
	string dbfile;
	string line;
	db_t entry;
	bool bNew= false;
	off_t size;
	map<string, string> files;

	dbfile= getLastDbFile(m_sWorkDir, "entrys_", size);
	if(dbfile == "")
	{
		dbfile= "entrys_";
		time(&m_tmDb);
		strftime(stime, 15, "%Y%m%d%H%M%S", localtime(&m_tmDb));
		dbfile+= stime;
		dbfile+= ".dat";
		bNew= true;
	}
	m_sDbFile= URL::addPath(m_sWorkDir, dbfile);

	// initial configuration path to reading default configuration for any chips
	DefaultChipConfigReader::init(m_sConfDir);

	// delete all .new files because the are half finished
	// and all .done files change to .dat
	files= URL::readDirectory(m_sWorkDir, "entrys_", ".new");
	for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
	{
		unlink((m_sWorkDir + it->second).c_str());
	}
	files= URL::readDirectory(m_sWorkDir, "entrys_", ".done");
	for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
	{
		dbfile= m_sWorkDir;
		dbfile+= it->second.substr(0, 21);
		dbfile+= ".dat";
		unlink(dbfile.c_str());
		rename((m_sWorkDir + it->second).c_str(), dbfile.c_str());
	}

#ifdef DEBUG
	cout << "read/write file:" << m_sDbFile << " " << flush;
#endif // DEBUG
	if(!bNew)
	{
		// read last status of any subroutines in folders
		ifstream file(&m_sDbFile[0]);
		if(!file.is_open())
		{
			string error("### ERROR: cannot open file '");

			error+= m_sDbFile + "'\n";
			error+= "           start no database\n    ERRNO: ";
			error+= strerror(errno);
			cerr << endl << error << endl;
			LOG(LOG_ALERT, error);
			return 1;
		}
		LOG(LOG_DEBUG, "beginning to read database file " + m_sDbFile);
		while(!file.eof())
		{
			getline(file, line);
			//cout << line << endl;
			entry= splitDbLine(line);
#if 0
			if(entry.subroutine == "holder")
			{
				cout << "device  " << entry.folder << ":" << entry.subroutine << endl;
				cout << " access " << boolalpha << entry.device << endl;
				cout << " new    " << boolalpha << entry.bNew << endl;
				cout << endl;
			}
#endif
			if(entry.identif != "")
			{
				if(	entry.identif.substr(0, 4) == "def:"
					||
					entry.identif.substr(0, 6) == "clear:"	)
				{
					fillMeasureCurve(entry, false/*do not sort vector*/);
				}else
					setActEntry(entry);
			}
		}
		file.close();
		LOG(LOG_DEBUG, "reading of database file " + m_sDbFile + " is finished");

		if(size > 15000000)
		{// create an new database file
			createNewDbFile(/*check whether*/false);
		}
	}

	LOCK(m_DBLOADED);
	m_bDbLoaded= true;
	UNLOCK(m_DBLOADED);
	return 0;
}

void DatabaseThread::fillMeasureCurve(const db_t entry, bool doSort/*= true*/)
{
	bool bClear= false;
	string vor;
	string subroutine;
	vector<string> identif;
	string svalue;

	identif= ConfigPropertyCasher::split(entry.identif, ":");
	if(identif.size() < 4)
	{
		cout << "### ERROR: wrong identif value in database vor measure-curve" << endl;
		return;
	}
	if(identif[0] == "clear")
		bClear= true;
	vor= identif[1];
	subroutine= identif[2] + ":" + identif[3];

	LOCK(m_DBMEASURECURVES);
	if(bClear)
	{
		//map<double, double> clearObj;

		m_mmmMeasureCurves[subroutine][vor].clear();
		m_vMeasureCurves.clear();
	}else
	{
		m_mmmMeasureCurves[subroutine][vor][entry.values[0]]= entry.values[1];
		m_vMeasureCurves.push_back(entry);
	}

	UNLOCK(m_DBMEASURECURVES);
}

string DatabaseThread::getLastDbFile(string path, string filter, off_t &size)
{
	struct dirent *dirName;
	string file;
	string lastFile("");
	string lastDate("");
	int fileLen;
	int filterLen= filter.length();
	DIR *dir;

	dir= opendir(&path[0]);
	if(dir == NULL)
	{
		string msg("### ERROR: cannot read in subdirectory '");

		msg+= path + "'\n";
		msg+= "    ERRNO: ";
		msg+= strerror(errno);
		cout << msg << endl;
		LOG(LOG_ALERT, msg);
		return "";
	}
	while((dirName= readdir(dir)) != NULL)
	{
		if(dirName->d_type == DT_REG)
		{
			//printf ("%s\n", dirName->d_name);
			file= dirName->d_name;
			fileLen= file.length();
			if(	fileLen == (filterLen + 18)
				&&
				file.substr(0, filterLen) == filter
				&&
				file.substr(fileLen - 4) == ".dat"	)
			{
				string date(file.substr(filterLen, 8));

				if(	lastDate == ""
					||
					lastDate < date	)
				{
					lastDate= date;
					lastFile= file;
				}
			}
		}
	}
	closedir(dir);
	if(lastFile != "")
	{
		struct stat fileStat;

		path+= "/" + lastFile;
		if(stat(&path[0], &fileStat) != 0)
		{
			char cerrno[20];
			string error("   cannot read size of database file ");

			error+= lastFile + "\n   ERRNO(";
			sprintf(cerrno, "%d", errno);
			error+= cerrno;
			error+= "): ";
			error+= strerror(errno);
			error+= "\n   create an new one          ";
			LOG(LOG_ERROR, error);
			cerr << endl << error;
			size= 15000001;// to create an new file
		}else
			size= fileStat.st_size;

	}else
		size= 0;
	return lastFile;
}

unsigned short DatabaseThread::existEntry(const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number/*= 0*/)
{
	unsigned short nRv;
	map<string, map<string, map<string, db_t> > >::iterator fEntrys;
	map<string, map<string, db_t> >::iterator sEntrys;
	map<string, db_t>::iterator iEntrys;

	LOCK(m_DBCURRENTENTRY);
	fEntrys= m_mCurrent.find(folder);
	if(fEntrys != m_mCurrent.end())
	{
		sEntrys= fEntrys->second.find(subroutine);
		if(sEntrys != fEntrys->second.end())
		{
			iEntrys= sEntrys->second.find(identif);
			if(iEntrys != sEntrys->second.end())
			{
				if(iEntrys->second.values.size() > number)
				{
					if(iEntrys->second.device)
						nRv= 5;// device exist correctly
					else
						nRv= 4; // no correct access to device
				}else
					nRv= 3; // count of value do not exist
			}else
				nRv= 2; // entry have not given identifier
		}else
			nRv= 1; // subroutine do not exist in folder
	}else
		nRv= 0; // no folder with given name defined
	UNLOCK(m_DBCURRENTENTRY);
	return nRv;
}

auto_ptr<double> DatabaseThread::getActEntry(const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number/*= 0*/)
{
	auto_ptr<double> spnRv;
	db_t tvalue;
	map<string, map<string, db_t> > fEntrys;
	map<string, db_t> sEntrys;

	LOCK(m_DBCURRENTENTRY);
	fEntrys= m_mCurrent[folder];
	UNLOCK(m_DBCURRENTENTRY);

	if(fEntrys.size() == 0)
		return spnRv;
	sEntrys= fEntrys[subroutine];
	if(sEntrys.size() == 0)
		return spnRv;
	tvalue= sEntrys[identif];
	if(tvalue.folder == "")
		return spnRv;

	spnRv= auto_ptr<double>(new double(tvalue.values[number]));
	return spnRv;
}

bool DatabaseThread::createNewDbFile(bool bCheck)
{
	typedef map<string, map<string, map<string, db_t> > >::iterator folderIter;
	typedef map<string, map<string, db_t> >::iterator subIter;
	typedef map<string, db_t>::iterator identifIter;
	typedef vector<db_t>::iterator curveIter;

	char stime[16];
	map<string, map<string, db_t> > subroutines;
	map<string, db_t> identifications;
	struct stat fileStat;
	ofstream dbFile;

	if(bCheck)
	{
		if(stat(&m_sDbFile[0], &fileStat) != 0)
		{
			char cerrno[20];
			string error("   cannot read size of database file ");

			error+= m_sDbFile + "\n   ERRNO(";
			sprintf(cerrno, "%d", errno);
			error+= cerrno;
			error+= "): ";
			error+= strerror(errno);
			error+= "\n   create no new one";
			LOG(LOG_ERROR, error);
			return false;
		}
		if(fileStat.st_size < (int)m_nAfter)
			return false;
	}

	m_sDbFile= URL::addPath(m_sWorkDir, "entrys_");
	time(&m_tmDb);
	strftime(stime, 15, "%Y%m%d%H%M%S", localtime(&m_tmDb));
	//strftime(stime, 9, "%Y%m%d%H%M%S", gmtime(&m_tmDb));
	m_sDbFile+= stime;
	m_sDbFile+= ".dat";

	LOCK(m_DBCURRENTENTRY);

	dbFile.open(&m_sDbFile[0], ios::app);
	for(folderIter fc= m_mCurrent.begin(); fc != m_mCurrent.end(); ++fc)
	{
		for(subIter sc= fc->second.begin(); sc != fc->second.end(); ++sc)
		{
			for(identifIter ic= sc->second.begin(); ic != sc->second.end(); ++ic)
			{
				vector<db_t>::iterator found;

				found= find(m_vtDbValues.begin(), m_vtDbValues.end(), &ic->second);
				if(found != m_vtDbValues.end())
					writeDb(ic->second, &dbFile);
			}
		}
	}
	for(curveIter ci= m_vMeasureCurves.begin(); ci != m_vMeasureCurves.end(); ++ci)
	{
		writeDb(*ci, &dbFile);
	}
	UNLOCK(m_DBMEASURECURVES);

	dbFile.close();
	return true;
}

void DatabaseThread::changeNeededIds(unsigned long oldId, unsigned long newId)
{
	vector<db_t> entrys;

	LOCK(m_CHANGINGPOOL);
	entrys= m_mvoChanges[oldId];
	m_mvoChanges.erase(oldId);
	m_mvoChanges[newId]= entrys;
	UNLOCK(m_CHANGINGPOOL);
}

void DatabaseThread::isEntryChanged()
{
	int conderror= 0;

	LOCK(m_CHANGINGPOOL);
	if(!m_bAnyChanged)
		conderror= CONDITION(m_CHANGINGPOOLCOND, m_CHANGINGPOOL);
	m_bAnyChanged= false;
	UNLOCK(m_CHANGINGPOOL);
	if(	conderror
		&&
		conderror != EINTR	)
	{
		usleep(500000);
	}
}

void DatabaseThread::useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip)
{
	m_mmmvServerContent[onServer][chip][folder].push_back(subroutine);
}

map<string, vector<string> >* DatabaseThread::getSubroutines(const string& onServer, const string& chip)
{
	return &m_mmmvServerContent[onServer][chip];
}

vector<string> DatabaseThread::getDebugInfo(const unsigned short server)
{
	int err;
	IClientPattern* client;
	ostringstream definition;
	vector<string> vRv;
	string answer;

	definition << "OwServerQuestion-" << server;
	client= m_pStarter->getClient(definition.str(), NULL);
	if(client != NULL)
	{
		do{
			answer= client->sendString("getinfo", true);
			err= ExternClientInputTemplate::error(answer);
			vRv.push_back(answer);

		}while(answer != "done" && err <= 0);
	}
	return vRv;
}

vector<string> DatabaseThread::getChangedEntrys(unsigned long connection)
{
	map<string, map<string, map<string, db_t> > >::iterator fEntrys;
	map<string, map<string, db_t> >::iterator sEntrys;
	map<string, db_t>::iterator iEntrys;
	vector<string> vsRv;

	LOCK(m_DBCURRENTENTRY);
	LOCK(m_CHANGINGPOOL);
	for(vector<db_t>::iterator i= m_mvoChanges[connection].begin(); i != m_mvoChanges[connection].end(); ++i)
	{
		if(i->identif == "owserver")
		{
			ostringstream info;

			info << "read_owserver_debuginfo " << i->tm;
			vsRv.push_back(info.str());
		}else
		{
			if(	i->folder == ""
				&&
				(	i->subroutine == "stopclient"
					||
					i->subroutine == "serverisstopping"	)	)
			{
				vsRv.push_back(i->subroutine);
				m_mvoChanges.erase(connection);
				break;
			}
			fEntrys= m_mCurrent.find(i->folder);
			if(fEntrys != m_mCurrent.end())
			{
				sEntrys= fEntrys->second.find(i->subroutine);
				if(sEntrys != fEntrys->second.end())
				{
					iEntrys= sEntrys->second.find("value");
					if(iEntrys != sEntrys->second.end())
					{
						double newvalue= 0;
						double actvalue= 0;
						db_t look= iEntrys->second;

						newvalue= iEntrys->second.values.back();
						if(!i->values.empty())
							actvalue= i->values.back();
						else
							actvalue= newvalue > 0 || newvalue < 0 ? 0 : 1;

						if(	i->device != iEntrys->second.device
							||
							newvalue < actvalue
							||
							newvalue > actvalue	)
						{
							ostringstream entry;

							entry << i->folder << ":" << i->subroutine;
							if(i->device != iEntrys->second.device)
							{
								string device(entry.str());

								if(iEntrys->second.device)
									device+= " access";
								else
									device+= " noaccess";
								vsRv.push_back(device);
								i->device= iEntrys->second.device;
							}
							if(iEntrys->second.device)
							{
								entry <<  "=" << dec << newvalue;
								vsRv.push_back(entry.str());
								i->values.clear();
								i->values.push_back(newvalue);
							}
						}
					}
				}
			}
		}
	}
	UNLOCK(m_CHANGINGPOOL);
	UNLOCK(m_DBCURRENTENTRY);
	return vsRv;
}

bool DatabaseThread::needSubroutines(unsigned long connection, string name)
{
	db_t newentry;
	//vector<db_t> needValues;
	vector<db_t>::const_iterator iNeed;
	map<string, map<string, map<string, db_t> > >::iterator fEntrys;
	map<string, map<string, db_t> >::iterator sEntrys;
	map<string, db_t>::iterator iEntrys;
	map<unsigned long, vector<db_t> >::iterator changeIt;
	vector<string> split;

	if(	name == "newentrys"
		||
		name == "stopclient"	)
	{
		LOCK(m_CHANGINGPOOL);
		changeIt= m_mvoChanges.find(connection);
		if(changeIt == m_mvoChanges.end())
		{
			UNLOCK(m_CHANGINGPOOL);
			return true;
		}
		changeIt->second.clear();
		if(name == "stopclient")
		{
			newentry.folder= "";
			newentry.subroutine= "stopclient";
			changeIt->second.push_back(newentry);
		}
		AROUSEALL(m_CHANGINGPOOLCOND);
		m_bAnyChanged= true;
		UNLOCK(m_CHANGINGPOOL);
		return true;

	}else if(name == "serverisstopping")
	{
		LOCK(m_CHANGINGPOOL);
		for(map<unsigned long, vector<db_t> >::iterator it= m_mvoChanges.begin(); it != m_mvoChanges.end(); ++it)
		{
			it->second.clear();
			newentry.folder= "";
			newentry.subroutine= "serverisstopping";
			it->second.push_back(newentry);
		}
		AROUSEALL(m_CHANGINGPOOLCOND);
		UNLOCK(m_CHANGINGPOOL);
		return true;

	}else if(name.substr(0, 9) == "owserver-")
	{
		newentry.identif= "owserver";
		name= name.substr(9);
		newentry.tm= (time_t)atoi(name.c_str());
		LOCK(m_CHANGINGPOOL);
		changeIt= m_mvoChanges.find(connection);
		if(changeIt != m_mvoChanges.end())
			changeIt->second.clear();
		m_mvoChanges[connection].push_back(newentry);
		AROUSEALL(m_CHANGINGPOOLCOND);
		m_bAnyChanged= true;
		UNLOCK(m_CHANGINGPOOL);
		return true;
	}
	split= ConfigPropertyCasher::split(name, ":");
	if(split.size() != 2)
		return false;

	LOCK(m_DBCURRENTENTRY);
	fEntrys= m_mCurrent.find(split[0]);
	if(fEntrys == m_mCurrent.end())
	{
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}
	sEntrys= fEntrys->second.find(split[1]);
	if(sEntrys == fEntrys->second.end())
	{
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}
	iEntrys= sEntrys->second.find("value");
	if(iEntrys == sEntrys->second.end())
	{
		LOG(LOG_ALERT, "found correct folder and subroutine but no entry for 'value'");
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}
	newentry= iEntrys->second;
	newentry.device= true;// if client ask for an subroutine
						  // he always believe the device is reached
						  // so he should get an noaccess notification
						  // every first query when server have no access to device
	newentry.values.clear();// also for values
	LOCK(m_CHANGINGPOOL);
	changeIt= m_mvoChanges.find(connection);
	if(changeIt != m_mvoChanges.end())
	{// search for exist entry
		iNeed= find(changeIt->second.begin(), changeIt->second.end(), &newentry);
		if(iNeed != changeIt->second.end())
		{// subroutine is declared for needing
		 // do not need the same value again
			//cout << "found " << iNeed->folder << ":" << iNeed->subroutine << endl;
			AROUSEALL(m_CHANGINGPOOLCOND);
			m_bAnyChanged= true;
			UNLOCK(m_CHANGINGPOOL);
			UNLOCK(m_DBCURRENTENTRY);
			return true;
		}
	}
	m_mvoChanges[connection].push_back(newentry);
	AROUSEALL(m_CHANGINGPOOLCOND);
	m_bAnyChanged= true;
	UNLOCK(m_CHANGINGPOOL);
	UNLOCK(m_DBCURRENTENTRY);
	return true;
}


bool DatabaseThread::setActEntry(const db_t entry)
{
	db_t tvalue;
	string identif;
	map<string, map<string, db_t> > fEntrys;
	map<string, db_t> sEntrys;

	LOCK(m_DBCURRENTENTRY);
	fEntrys= m_mCurrent[entry.folder];
	// check for folder exist
	if(fEntrys.size() == 0)
	{
		std::map<string, db_t> isEntry;
		map<string, map<string, db_t> > ifEntry;

		isEntry[entry.identif]= entry;
		ifEntry[entry.subroutine]= isEntry;
		m_mCurrent[entry.folder]= ifEntry;
		UNLOCK(m_DBCURRENTENTRY);
		return true;
	}else
	{
		sEntrys= fEntrys[entry.subroutine];
		// check for subroutine exist
		if(sEntrys.size() == 0)
		{
			map<string, db_t> isEntry;

			isEntry[entry.identif]= entry;
			m_mCurrent[entry.folder][entry.subroutine]= isEntry;
			UNLOCK(m_DBCURRENTENTRY);
			return true;
		}else
		{
			identif= entry.identif;
			if(identif == "access")
				identif= "value"; // access identif is only for values
			tvalue= sEntrys[identif];
			// check for identif specification exist
			if(tvalue.folder == "")
			{
				m_mCurrent[entry.folder][entry.subroutine][entry.identif]= entry;
				UNLOCK(m_DBCURRENTENTRY);
				return true;
			}else
			{
				bool changed= true;
				vector<double>::size_type tcount= tvalue.values.size();
				vector<double>::size_type ecount= entry.values.size();

				/*cout << "new " << entry.identif << " of " << entry.subroutine << " is ";
				if(entry.device)
					cout << "true" << endl;
				else
					cout << "false" << endl;
				cout << "value of " << tvalue.subroutine << " is ";
				if(tvalue.device)
					cout << "true" << endl;
				else
					cout << "false" << endl;
				cout << endl;*/
				if(entry.identif == "access")
				{
					if(tvalue.device != entry.device)
					{
						m_mCurrent[tvalue.folder][tvalue.subroutine][tvalue.identif].device= entry.device;
						UNLOCK(m_DBCURRENTENTRY);
						return true;
					}
					changed= false;
				}else
				{
					if(tvalue.device)
					{

						if(tcount == ecount)
						{
							changed= false;
							for(vector<double>::size_type c= 0; c < tcount; ++c)
							{
								if(tvalue.values[c] != entry.values[c])
								{
									changed= true;
									break;
								}
							}
						}
					}else
						changed= false;
				}
				if(changed)
				{
					m_mCurrent[tvalue.folder][tvalue.subroutine][tvalue.identif].values= entry.values;
					UNLOCK(m_DBCURRENTENTRY);
					return true;
				}
			}
		}
	}
	UNLOCK(m_DBCURRENTENTRY);
	return false;
}

int DatabaseThread::stop(const bool *bWait)
{
	int nRv= 0;

	nRv= Thread::stop();

	LOCK(m_DBENTRYITEMS);
	AROUSE(m_DBENTRYITEMSCOND);
	AROUSEALL(m_CHANGINGPOOLCOND);
	UNLOCK(m_DBENTRYITEMS);

	if(	nRv == 0
		&&
		bWait	)
	{
		nRv= Thread::stop(/*wait*/bWait);
	}
	return nRv;
}

int DatabaseThread::execute()
{
	bool bNewValue;
	static bool bWait= false;
	std::auto_ptr<vector<db_t> > entrys= getDbEntryVector(bWait);

	if(entrys->size() > 0)
	{
		typedef vector<db_t>::iterator iter;

		for(iter i= entrys->begin(); i!=entrys->end(); ++i)
		{
			if(	i->identif.substr(0, 4) == "def:"
				||
				i->identif.substr(0, 6) == "clear:"	)
			{
				fillMeasureCurve(*i);
				bNewValue= true;
			}else
				bNewValue= setActEntry(*i);

			if(	bNewValue
				||				// write only if it is an new Value
				!i->bNew	)	// or in structure db_t is set to write every time (bNew is false)
			{
				db_t entry= *i;
				vector<db_t>::iterator found;

				LOCK(m_CHANGINGPOOL);
				AROUSEALL(m_CHANGINGPOOLCOND);
				m_bAnyChanged= true;
				UNLOCK(m_CHANGINGPOOL);
				found= find(m_vtDbValues.begin(), m_vtDbValues.end(), &entry);
				if(found != m_vtDbValues.end())
					writeDb(*i);
			}
		}
		if(bWait)
			bWait= !thinDatabase(/*ask*/true);
	}else
		bWait= !thinDatabase(/*ask*/false);
	createNewDbFile(/*check whether*/true);
	return 0;
}

vector<convert_t> DatabaseThread::getNearest(string subroutine, string definition, double value)
{

	map<string, map<string, map<double, double> > >::iterator subIter;
	map<string, map<double, double> >::iterator defIter;
	map<double, double>::iterator curveIter;
	vector<convert_t> result;

	LOCK(m_DBMEASURECURVES);
	subIter= m_mmmMeasureCurves.find(subroutine);
	if(subIter != m_mmmMeasureCurves.end())
	{
		defIter= subIter->second.find(definition);
		if(defIter != subIter->second.end())
		{
			auto_ptr<convert_t> beforeLast;
			auto_ptr<convert_t> lastValue; //= DOUBLE_MAX;

			for(curveIter= defIter->second.begin(); curveIter != defIter->second.end(); ++curveIter)
			{
				double current= curveIter->first;

				if(current >= value)
				{
					convert_t val;

					if(lastValue.get() != NULL)
						result.push_back(*lastValue);
					val.be= curveIter->second;
					val.nMikrosec= (unsigned long)curveIter->first;
					result.push_back(val);
					++curveIter;
					if(	lastValue.get() == NULL
						&&
						curveIter != defIter->second.end()	)
					{
						val.be= curveIter->second;
						val.nMikrosec= (unsigned long)curveIter->first;
						result.push_back(val);
					}
					break;
				}
				if(	lastValue.get() != NULL
					&&
					beforeLast.get() == NULL	)
				{
					beforeLast= auto_ptr<convert_t>(new convert_t);
				}
				if(beforeLast.get() != NULL)
				{
					beforeLast->be= lastValue->be;
					beforeLast->nMikrosec= lastValue->nMikrosec;
				}
				if(lastValue.get() == NULL)
					lastValue= auto_ptr<convert_t>(new convert_t);
				lastValue->be= curveIter->second;
				lastValue->nMikrosec= (unsigned long)curveIter->first;
			}
			if(	result.size() == 0
				&&
				lastValue.get() != NULL	)
			{
				if(beforeLast.get() != NULL)
					result.push_back(*beforeLast);
				result.push_back(*lastValue);
			}
		}
	}
	UNLOCK(m_DBMEASURECURVES);
	// toDo: write getNearestOhm into database
	//return TimeMeasure::getNearestOhm(measuredTime, m_vOhm);
	return result;
}

void DatabaseThread::ending()
{
	unsigned int lcount= 0;
	db_t entry;
	ofstream writeHandler(m_sDbFile.c_str(), ios::app);
	DefaultChipConfigReader::write_t write;
	DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();

	if(writeHandler.is_open())
	{
		entry.device= true;
		entry.identif= "value";
		// read all entrys witch saved in any first older structure
		// and this highest value not be saved in the file
		do{
			write= reader->getLastValues(lcount, /*older*/false);
			if(	write.action == "highest"
				||
				write.action == "fractions"	)
			{
				entry.folder= write.folder;
				entry.subroutine= write.subroutine;
				entry.values.clear();
				entry.values.push_back(write.highest.highest);
				entry.tm= write.highest.hightime;
				writeEntry(entry, writeHandler);
				if(write.action == "highest")
				{
					entry.values.clear();
					entry.values.push_back(write.highest.lowest);
					entry.tm= write.highest.lowtime;
					writeEntry(entry, writeHandler);
				}
			}
			++lcount;
		}while(write.action != "kill");
		writeHandler.close();
	}else
	{
		string msg("### ERROR: cannot write into '");

		msg+= m_sThinFile + ".new by ending application'\n";
		msg+= "    ERRNO: ";
		msg+= strerror(errno);
		cout << msg << endl;
		LOG(LOG_ERROR, msg);
	}

}

void DatabaseThread::writeDb(db_t entry, ofstream *dbfile/*= NULL*/)
{
	typedef vector<double>::iterator iter;

	bool bNewFile= false;
	ofstream oNewDbFile;
	unsigned short writecount;
	vector<DefaultChipConfigReader::write_t> vWrite;
	DefaultChipConfigReader::write_t writeaccess;
	DefaultChipConfigReader* configReader= DefaultChipConfigReader::instance();
	//char sValue[100];

	if(entry.identif == "value")
	{
		writeaccess= configReader->allowDbWriting(entry.folder, entry.subroutine, entry.values[0], entry.tm);
		if(writeaccess.action == "no")
			return;
		if(writeaccess.action == "fractions")
		{
			entry.values.clear();
			entry.values.push_back(writeaccess.highest.highest);
			entry.tm= writeaccess.highest.hightime;
			writeaccess.action= "write";
		}
	}else
		writeaccess.action= "write";

	if(dbfile == NULL)
	{

		bNewFile= true;
		oNewDbFile.open(m_sDbFile.c_str(), ios::app);
		if(oNewDbFile.fail())
		{
			string error("### ERROR: cannot open file '");

			error+= m_sDbFile + "'\n";
			error+= "           so write nothing into database\n    ERRNO: ";
			error+= strerror(errno);
			if(!m_bError)
				cout << error << endl;
			m_bError= true;
			TIMELOG(LOG_ALERT, "opendatabase", error);
			return;
		}
		dbfile= &oNewDbFile;
	}

	writecount= 0;
	do{
		++writecount;
		if(writeaccess.action == "highest") {
			if(writecount == 1)
				entry.tm= writeaccess.highest.hightime;
			else
				entry.tm= writeaccess.highest.lowtime;
		}
		if(entry.device)
		{
			if(writeaccess.action == "highest")
			{
				entry.values.clear();
				if(writecount == 1)
					entry.values.push_back(writeaccess.highest.highest);
				else
					entry.values.push_back(writeaccess.highest.lowest);
			}
		}
		writeEntry(entry, *dbfile);
		if(writeaccess.action == "write")
			break;
	}while(writecount < 2);
	if(bNewFile)
		oNewDbFile.close();
}

void DatabaseThread::writeEntry(const db_t& entry, ofstream &dbfile)
{
	char stime[18];

	dbfile << m_sMeasureName << "|";
	strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&entry.tm));
	dbfile << stime << "|";
	dbfile << entry.folder << "|" << entry.subroutine << "|";
	dbfile << entry.identif;
	dbfile << "|";
	if(entry.device)
	{
		for(vector<double>::const_iterator valIt= entry.values.begin(); valIt != entry.values.end(); ++valIt)
			dbfile << dec << *valIt << "|";
	}else
		dbfile << "NaN";
	dbfile << endl;
}

bool DatabaseThread::thinDatabase(const bool ask)
{
	int filecount, nextcount, count;
	db_t entry;
	time_t acttime;
	map<string, string> files, thinfiles;
	ofstream writeHandler;
	DefaultChipConfigReader* reader;
	map<string, time_t>::iterator fileparsed;

	reader= DefaultChipConfigReader::instance();
	if(!reader->chipsAreDefined())
		return true;
	if(	ask
		&&
		m_sThinFile != ""	)
	{
		return true;
	}
	if(m_sThinFile == "")
	{ // search for the next to thin file

		m_nReadPos= 0;
		files= URL::readDirectory(m_sWorkDir, "entrys_", ".dat");
		filecount= files.size();
		nextcount= m_mOldest.size();
		if(ask)
		{
			if((filecount - 1) != nextcount)
				return true;
		}
		if((filecount - 1) != nextcount)
		{
			count= 0;
			for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
			{
				++count;
				fileparsed= m_mOldest.find(it->second.substr(0, 21));
				if(fileparsed == m_mOldest.end())
				{
					m_sThinFile= it->second.substr(0, 21);
					m_mOldest[m_sThinFile]= 0;
					// do not thin database if it is the last file
					if(count == filecount)
						return false;
					if(ask)
						return true;
					break;
				}
			}
		}
		if(m_sThinFile == "")
		{
			time(&acttime);
			for(fileparsed= m_mOldest.begin(); fileparsed != m_mOldest.end(); ++fileparsed)
			{
				if(acttime >= fileparsed->second)
				{
					m_sThinFile= fileparsed->first;
					fileparsed->second= 0;
					break;
				}
			}
			if(m_sThinFile == "")
				return false;
			if(ask == true)
				return true;
		}
		m_mOldest[m_sThinFile]= 0;
	}


	bool newOrder;
	unsigned int nCountF= 0;
	unsigned int lcount= 0;
	string line;
	string readName(URL::addPath(m_sWorkDir, m_sThinFile + ".dat"));
	string writeName(URL::addPath(m_sWorkDir, m_sThinFile + ".new"));
	string doneName(URL::addPath(m_sWorkDir, m_sThinFile + ".done"));
	ifstream file(readName.c_str());
	DefaultChipConfigReader::write_t write;
	SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> older;

	if(file.is_open())
	{
		file.seekg(m_nReadPos);
		while(getline(file, line))
		{
			++nCountF;
			//cout << "line: " << line << endl;
			entry= splitDbLine(line);
			if(entry.identif == "value")
			{
				write.action= "no";

				for(vector<double>::iterator v= entry.values.begin(); v != entry.values.end(); ++v)
				{
					write= reader->allowDbWriting(entry.folder, entry.subroutine, *v, entry.tm, &newOrder);
					if(write.action != "no")
					{
						if(newOrder)
						{
							older= reader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/true);
							if(older)
							{
								db_t wolder;

								if(!writeHandler.is_open())
								{
									writeHandler.open(writeName.c_str(), ios::app);
									if(writeHandler.fail())
									{
										string msg("### ERROR: cannot write new file '");

										msg+= m_sThinFile + ".new'\n";
										msg+= "    ERRNO: ";
										msg+= strerror(errno);
										cout << msg << endl;
										TIMELOG(LOG_ALERT, "writenewfile", msg);
										file.close();
										return false;
									}
								}
								wolder.device= true;
								wolder.folder= entry.folder;
								wolder.subroutine= entry.subroutine;
								if(older->dbwrite == "fractions")
								{
									wolder.tm= older->fraction->deeptime;
									wolder.values.push_back(older->fraction->deepvalue);
									writeEntry(wolder, writeHandler);
								}else if(older->dbwrite == "highest")
								{
									wolder.tm= older->highest->lowtime;
									wolder.values.push_back(older->highest->lowest);
									writeEntry(wolder, writeHandler);
									wolder.tm= older->highest->hightime;
									wolder.values.clear();
									wolder.values.push_back(older->highest->highest);
									writeEntry(wolder, writeHandler);
								}
								calcNewThinTime(wolder.tm, older);
							}
						}
						break;
					}
				}
				if(	write.action == "write"
					||
					write.action == "fractions"	)
				{
					if(!writeHandler.is_open())
					{
						writeHandler.open(writeName.c_str(), ios::app);
						if(writeHandler.fail())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(LOG_ALERT, "writenewfile", msg);
							file.close();
							return false;
						}
					}
					if(write.action == "fractions")
					{
						entry.values.clear();
						entry.values.push_back(write.highest.highest);
						entry.tm= write.highest.hightime;
					}
					writeEntry(entry, writeHandler);
					older= reader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
					calcNewThinTime(entry.tm, older);

				}else if (write.action == "highest")
				{
					if(writeHandler.is_open())
					{
						writeHandler.open(writeName.c_str(), ios::app);
						if(writeHandler.fail())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(LOG_ALERT, "writenewfile", msg);
							file.close();
							return false;
						}
					}
					entry.device= true;
					entry.identif= "value";
					entry.values.clear();
					entry.values.push_back(write.highest.lowest);
					entry.tm= write.highest.lowtime;
					writeEntry(entry, writeHandler);
					entry.values.clear();
					entry.values.push_back(write.highest.highest);
					entry.tm= write.highest.hightime;
					writeEntry(entry, writeHandler);
					older= reader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
					calcNewThinTime(entry.tm, older);
				}
			}else if(entry.identif == "access")
			{
				older= reader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
				if(	older
					&&
					older->more == 0	)
				{
					if(!writeHandler)
					{
						writeHandler.open(writeName.c_str(), ios::app);
						if(writeHandler.fail())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(LOG_ALERT, "writenewfile", msg);
							file.close();
							return false;
						}
					}
					writeEntry(entry, writeHandler);
				}
			}
			if(nCountF == m_nReadRows)
			{ // stop to thin after 50 rows to look for new db entrys
				m_nReadPos= file.tellg();
				if(writeHandler.is_open())
					writeHandler.close();
				file.close();
				return true;
			}
		}

		entry.device= true;
		entry.identif= "value";
		// read all entrys witch saved in any older structures
		// and this value not saved in the files
		do{
			write= reader->getLastValues(lcount, /*older*/true);
			if(	write.action == "highest"
				||
				write.action == "fractions"	)
			{
				if(!writeHandler)
				{
					writeHandler.open(writeName.c_str(), ios::app);
					if(writeHandler.fail())
					{
						string msg("### ERROR: cannot write new file '");

						msg+= m_sThinFile + ".new'\n";
						msg+= "    ERRNO: ";
						msg+= strerror(errno);
						cout << msg << endl;
						TIMELOG(LOG_ALERT, "writenewfile", msg);
						file.close();
						return false;
					}
				}
				entry.folder= write.folder;
				entry.subroutine= write.subroutine;
				entry.values.clear();
				entry.values.push_back(write.highest.highest);
				entry.tm= write.highest.hightime;
				writeEntry(entry, writeHandler);
				if(write.action == "highest")
				{
					entry.values.clear();
					entry.values.push_back(write.highest.lowest);
					entry.tm= write.highest.lowtime;
					writeEntry(entry, writeHandler);
				}
				older= reader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
				calcNewThinTime(entry.tm, older);
			}
			++lcount;
		}while(write.action != "kill");

		newOrder= false;
		file.close();
		if(writeHandler.is_open())
		{
			writeHandler.close();
			newOrder= true;
		}else
		{
			files= URL::readDirectory(m_sWorkDir, writeName, "");
			if(files.size() > 0)
				newOrder= true;
		}
		if(newOrder)
			rename(writeName.c_str(), doneName.c_str());
		else
			m_mOldest.erase(m_sThinFile);
		unlink(readName.c_str());
		if(newOrder)
			rename(doneName.c_str(), readName.c_str());
		m_sThinFile= "";
	}else
	{
		string msg("### ERROR: cannot read file '");

		msg+= readName + "'\n";
		msg+= "    ERRNO: ";
		msg+= strerror(errno);
		cout << msg << endl;
		TIMELOG(LOG_ERROR, "readdirectory", msg);
		return false;
	}
	// maybe an next file is to thin
	return true;
}

void DatabaseThread::calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> &older)
{
	SHAREDPTR::shared_ptr<DefaultChipConfigReader::otime_t> act;
	DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();
	time_t acttime, nextThin;

	act= older;
	if(act.get())
	{
		if(act->older)
			act= act->older;
		time(&acttime);
		nextThin= Calendar::calcDate(/*newer*/true, fromtime, act->more, act->unit);
		if(nextThin <= acttime)
			nextThin= Calendar::calcDate(/*newer*/true, fromtime, (act->more + 1), act->unit);
		if(nextThin > acttime)
		{
			acttime= m_mOldest[m_sThinFile];
			if(	acttime == 0
				||
				acttime > nextThin	)
			{
				m_mOldest[m_sThinFile]= nextThin;
			}
		}
	}
}

void DatabaseThread::writeIntoDb(const string folder, const string subroutine)
{
	db_t entry;

	entry.folder= folder;
	entry.subroutine= subroutine;
	m_vtDbValues.push_back(entry);
}

void DatabaseThread::fillValue(string folder, string subroutine, string identif, double value, bool bNew/*=true*/)
{
	vector<double> values;

	values.push_back(value);
	fillValue(folder, subroutine, identif, values, bNew);
}

void DatabaseThread::fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew/*=true*/)
{
	db_t newEntry;

	time(&newEntry.tm);
	newEntry.folder= folder;
	newEntry.subroutine= subroutine;
	newEntry.identif= identif;
	newEntry.values= values;
	newEntry.bNew= bNew;
	if(identif == "access")
		newEntry.device= (bool)values.back();
	else // if identif not 'access' device should be trou
	{	 // because it will be checked in method setActEntry()
		 // whether device in memory is true and an new value
		newEntry.device= true;
	}

	LOCK(m_DBENTRYITEMS);
	m_sptEntrys->push_back(newEntry);
	AROUSE(m_DBENTRYITEMSCOND);
	UNLOCK(m_DBENTRYITEMS);
}

std::auto_ptr<vector<db_t> > DatabaseThread::getDbEntryVector(bool bWait)
{
	struct timespec time;
	int conderror= 0;
	std::auto_ptr<vector<db_t> > pRv(new vector<db_t>());

	if(!bWait)
	{
		clock_gettime(CLOCK_REALTIME, &time);
		time.tv_sec+= m_nCondWait;
	}
	do{
		sleepDefaultTime();
		LOCK(m_DBENTRYITEMS);
		if(m_sptEntrys->size() != 0)
		{
			pRv= m_sptEntrys;
			m_sptEntrys= std::auto_ptr<vector<db_t> >(new vector<db_t>());

		}else if(bWait) {
			conderror= CONDITION(m_DBENTRYITEMSCOND, m_DBENTRYITEMS);
		}else
			conderror= TIMECONDITION(m_DBENTRYITEMSCOND, m_DBENTRYITEMS, &time);
		UNLOCK(m_DBENTRYITEMS);
		if(conderror == ETIMEDOUT)
			break;
		if(conderror)
			usleep(500000);

	}while(	pRv->size() == 0
			&&
			!stopping()	);

	return pRv;
}

DatabaseThread::~DatabaseThread()
{
	DESTROYMUTEX(m_DBENTRYITEMS);
	DESTROYMUTEX(m_DBCURRENTENTRY);
	DESTROYMUTEX(m_DBMEASURECURVES);
	DESTROYMUTEX(m_CHANGINGPOOL);
	DESTROYCOND(m_DBENTRYITEMSCOND);
	DESTROYCOND(m_CHANGINGPOOLCOND);
}

}
