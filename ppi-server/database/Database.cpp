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

#include "Database.h"
#include "DefaultChipConfigReader.h"

#include "../logger/LogThread.h"

#include "../util/URL.h"
#include "../util/configpropertycasher.h"

#include "../portserver/owserver.h"

using namespace util;
using namespace server;
using namespace ports;

namespace ppi_database
{

Database* Database::_instance= NULL;

Database::Database(string dbDir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep)
:	Thread("database", defaultSleep, true),
	m_mCurrent()
{
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
	m_ptEntrys= new vector<db_t>();
	m_DBENTRYITEMSCOND= getCondition("DBENTRYITEMSCOND");
	m_DBENTRYITEMS= getMutex("DBENTRYITEMS");
	m_DBCURRENTENTRY= getMutex("DBCURRENTENTRY");
	m_DBMEASURECURVES= getMutex("DBMEASURECURVES");
	m_CHANGINGPOOL= getMutex("CHANGINGPOOL");
	m_CHANGINGPOOLCOND= getCondition("CHANGINGPOOLCOND");
	m_sConfDir= confDir;
	m_sWorkDir= dbDir;
	m_nAfter= (unsigned int)(newdbafter * 1000000);
	if(m_sWorkDir.substr(m_sWorkDir.length()-1, 1) != "/")
		m_sWorkDir+= "/";
}

void Database::initial(string workDir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep)
{
	if(_instance == NULL)
	{
		_instance= new Database(workDir, confDir, properties, defaultSleep);
		_instance->start(NULL, false);
	}
}

db_t Database::splitDbLine(const string& line)
{
	struct tm entryTime;
	db_t entry;
	vector<string> columns;
	vector<string>::size_type count;

	columns= ConfigPropertyCasher::split(line, "|");
	count= columns.size();

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

bool Database::init(void *args)
{
	char stime[16];
	string dbfile;
	string line;
	db_t entry;
	bool bNew= false;
	off_t size;
	map<string, string> files;

	cout << "### initial database ... " << flush;
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
	files= readDirectory(m_sWorkDir, "entrys_", ".new");
	for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
	{
		unlink((m_sWorkDir + it->second).c_str());
	}
	files= readDirectory(m_sWorkDir, "entrys_", ".done");
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
			LOG(AKALERT, error);
			return false;
		}
		while(!file.eof())
		{
			getline(file, line);
			//cout << line << endl;
			entry= splitDbLine(line);
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

		if(size > 15000000)
		{// create an new database file
			createNewDbFile(/*check whether*/false);
		}
	}

	cout << " OK" << endl;
	return true;
}

void Database::fillMeasureCurve(const db_t entry, bool doSort/*= true*/)
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

string Database::getLastDbFile(string path, string filter, off_t &size)
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
		LOG(AKALERT, msg);
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
			LOG(AKERROR, error);
			cerr << endl << error;
			size= 15000001;// to create an new file
		}else
			size= fileStat.st_size;

	}else
		size= 0;
	return lastFile;
}

double* Database::getActEntry(const string folder, const string subroutine, const string identif, const vector<double>::size_type number/*= 0*/)
{
	double* pnRv;
	db_t tvalue;
	map<string, map<string, db_t> > fEntrys;
	map<string, db_t> sEntrys;

	LOCK(m_DBCURRENTENTRY);
	fEntrys= m_mCurrent[folder];
	UNLOCK(m_DBCURRENTENTRY);

	if(fEntrys.size() == 0)
		return NULL;
	sEntrys= fEntrys[subroutine];
	if(sEntrys.size() == 0)
		return NULL;
	tvalue= sEntrys[identif];
	if(tvalue.folder == "")
		return NULL;

	pnRv= new double(tvalue.values[number]);
	return pnRv;
}

bool Database::createNewDbFile(bool bCheck)
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
			LOG(AKERROR, error);
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

void Database::changeNeededIds(unsigned long oldId, unsigned long newId)
{
	vector<db_t> entrys;

	LOCK(m_CHANGINGPOOL);
	entrys= m_mvoChanges[oldId];
	m_mvoChanges.erase(oldId);
	m_mvoChanges[newId]= entrys;
	UNLOCK(m_CHANGINGPOOL);
}

vector<string> Database::getChangedEntrys(unsigned long connection)
{
	int conderror= 0;
	map<string, map<string, map<string, db_t> > >::iterator fEntrys;
	map<string, map<string, db_t> >::iterator sEntrys;
	map<string, db_t>::iterator iEntrys;
	vector<string> vsRv;

	do{
		LOCK(m_DBCURRENTENTRY);
		LOCK(m_CHANGINGPOOL);
		for(vector<db_t>::iterator i= m_mvoChanges[connection].begin(); i != m_mvoChanges[connection].end(); ++i)
		{
			if(i->identif == "owserver")
			{
				OWServer* server= OWServer::getServer((unsigned short)i->tm);

				if(server != NULL)
				{
					UNLOCK(m_CHANGINGPOOL);
					UNLOCK(m_DBCURRENTENTRY);
					return server->getDebugInfo();
				}
			}else
			{
				if(	i->folder == ""
					&&
					i->subroutine == "stopclient"	)
				{
					vsRv.push_back("stopclient");
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
		if(vsRv.size() == 0)
		{
			LOCK(m_CHANGINGPOOL);
			conderror= CONDITION(m_CHANGINGPOOLCOND, m_CHANGINGPOOL);
			UNLOCK(m_CHANGINGPOOL);
		}
		if(	conderror
			&&
			conderror != EINTR	)
		{
			usleep(500000);
		}
	}while(vsRv.size() == 0);
	return vsRv;
}

bool Database::needSubroutines(unsigned long connection, string name)
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
		LOG(AKALERT, "found correct folder and subroutine but no entry for 'value'");
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}
	newentry= iEntrys->second;
	newentry.device= true;// if client ask for an subroutine
						  // he always beleve the device is reached
						  // so he should get an noaccess notification
						  // every first query
	newentry.values.clear();// also for values
	LOCK(m_CHANGINGPOOL);
	changeIt= m_mvoChanges.find(connection);
	if(changeIt != m_mvoChanges.end())
	{// search for exist entry
		iNeed= find(changeIt->second.begin(), changeIt->second.end(), &newentry);
		if(iNeed != changeIt->second.end())
		{// subroutine is decleared for needing
		 // do not need the same value again
			//cout << "found " << iNeed->folder << ":" << iNeed->subroutine << endl;
			AROUSEALL(m_CHANGINGPOOLCOND);
			UNLOCK(m_CHANGINGPOOL);
			UNLOCK(m_DBCURRENTENTRY);
			return true;
		}
	}
	m_mvoChanges[connection].push_back(newentry);
	AROUSEALL(m_CHANGINGPOOLCOND);
	UNLOCK(m_CHANGINGPOOL);
	UNLOCK(m_DBCURRENTENTRY);
	return true;
}


bool Database::setActEntry(const db_t entry)
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
		std::map<string, db_t> isEntry;//= new std::map<string, db_t>();
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

void* Database::stop(const bool *bWait)
{
	void* vRv= NULL;

	Thread::stop();

	LOCK(m_DBENTRYITEMS);
	AROUSE(m_DBENTRYITEMSCOND);
	UNLOCK(m_DBENTRYITEMS);

	if(bWait)
		vRv= Thread::stop(/*wait*/bWait);
	return vRv;
}

void Database::execute()
{
	bool bNewValue;
	static bool bWait= false;
	vector<db_t> *entrys= getDbEntryVector(bWait);

	if(entrys != NULL)
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

				AROUSEALL(m_CHANGINGPOOLCOND);
				found= find(m_vtDbValues.begin(), m_vtDbValues.end(), &entry);
				if(found != m_vtDbValues.end())
					writeDb(*i);
			}
		}
		delete entrys;
		if(bWait)
			bWait= !thinDatabase(/*ask*/true);
	}else
		bWait= !thinDatabase(/*ask*/false);
	createNewDbFile(/*check whether*/true);
	//usleep(1000);
}

vector<convert_t> Database::getNearest(string subroutine, string definition, double value)
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
			convert_t *beforeLast= NULL;
			convert_t *lastValue= NULL; //= DOUBLE_MAX;

			for(curveIter= defIter->second.begin(); curveIter != defIter->second.end(); ++curveIter)
			{
				double current= curveIter->first;

				if(current >= value)
				{
					convert_t val;

					if(lastValue != NULL)
						result.push_back(*lastValue);
					val.be= curveIter->second;
					val.nMikrosec= (unsigned long)curveIter->first;
					result.push_back(val);
					++curveIter;
					if(	lastValue == NULL
						&&
						curveIter != defIter->second.end()	)
					{
						val.be= curveIter->second;
						val.nMikrosec= (unsigned long)curveIter->first;
						result.push_back(val);
					}
					break;
				}
				if(	lastValue != NULL
					&&
					beforeLast == NULL	)
				{
					beforeLast= new convert_t;
				}
				if(beforeLast != NULL)
				{
					beforeLast->be= lastValue->be;
					beforeLast->nMikrosec= lastValue->nMikrosec;
				}
				if(lastValue == NULL)
					lastValue= new convert_t;
				lastValue->be= curveIter->second;
				lastValue->nMikrosec= (unsigned long)curveIter->first;
			}
			if(	result.size() == 0
				&&
				lastValue != NULL	)
			{
				if(beforeLast != NULL)
					result.push_back(*beforeLast);
				result.push_back(*lastValue);
			}
			if(lastValue != NULL)
				delete lastValue;
		}
	}
	UNLOCK(m_DBMEASURECURVES);
	// toDo: write getNearestOhm into database
	//return TimeMeasure::getNearestOhm(measuredTime, m_vOhm);
	return result;
}

void Database::ending()
{
	unsigned int lcount= 0;
	db_t entry;
	ofstream *writeHandler= NULL;
	DefaultChipConfigReader::write_t write;
	DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();

	writeHandler= new ofstream(m_sDbFile.c_str(), ios::app);
	if(writeHandler->is_open())
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
		writeHandler->close();
	}else
	{
		string msg("### ERROR: cannot write into '");

		msg+= m_sThinFile + ".new by ending application'\n";
		msg+= "    ERRNO: ";
		msg+= strerror(errno);
		cout << msg << endl;
		LOG(AKERROR, msg);
	}

}

void Database::writeDb(db_t entry, ofstream* dbfile/*= NULL*/)
{
	typedef vector<double>::iterator iter;

	bool bNewFile= false;
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
		dbfile= new ofstream;
		dbfile->open(m_sDbFile.c_str(), ios::app);
		if(dbfile->fail())
		{
			string error("### ERROR: cannot open file '");

			error+= m_sDbFile + "'\n";
			error+= "           so write nothing into database\n    ERRNO: ";
			error+= strerror(errno);
			if(!m_bError)
				cout << error << endl;
			m_bError= true;
			TIMELOG(AKALERT, "opendatabase", error);
			delete dbfile;
			return;
		}
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
		writeEntry(entry, dbfile);
		if(writeaccess.action == "write")
			break;
	}while(writecount < 2);
	if(bNewFile)
	{
		dbfile->close();
		delete dbfile;
	}
}

void Database::writeEntry(const db_t& entry, ofstream *dbfile)
{
	char stime[18];

	*dbfile << m_sMeasureName << "|";
	strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&entry.tm));
	*dbfile << stime << "|";
	*dbfile << entry.folder << "|" << entry.subroutine << "|";
	*dbfile << entry.identif;
	*dbfile << "|";
	if(entry.device)
	{
		for(vector<double>::const_iterator valIt= entry.values.begin(); valIt != entry.values.end(); ++valIt)
			*dbfile << dec << *valIt << "|";
	}else
		*dbfile << "NaN";
	*dbfile << endl;
}

map<string, string> Database::readDirectory(const string& path, const string& beginfilter, const string& endfilter)
{
	struct dirent *dirName;
	string file;
	map<string, string> files;
	int fileLen;
	int beginfilterLen= beginfilter.length();
	int endfilterLen= endfilter.length();
	DIR *dir;

	dir= opendir(&path[0]);
	if(dir == NULL)
	{
		string msg("### ERROR: cannot read in subdirectory '");

		msg+= path + "'\n";
		msg+= "    ERRNO: ";
		msg+= strerror(errno);
		cout << msg << endl;
		TIMELOG(AKALERT, "readdirectory", msg);
		return files;
	}
	while((dirName= readdir(dir)) != NULL)
	{
		if(dirName->d_type == DT_REG)
		{
			//printf ("%s\n", dirName->d_name);
			file= dirName->d_name;
			fileLen= file.length();
			if(	file.substr(0, beginfilterLen) == beginfilter
				&&
				(	fileLen == beginfilterLen
					||
					(	fileLen == (beginfilterLen + 14 + endfilterLen)
						&&
						file.substr(fileLen - endfilterLen) == endfilter	)	)	)
			{
				string date(file.substr(beginfilterLen, 14));

				files[date]= file;
			}
		}
	}
	closedir(dir);
	return files;
}

bool Database::thinDatabase(const bool ask)
{
	int filecount, nextcount, count;
	db_t entry;
	time_t acttime;
	map<string, string> files, thinfiles;
	ofstream *writeHandler= NULL;
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
		files= readDirectory(m_sWorkDir, "entrys_", ".dat");
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
	const DefaultChipConfigReader::otime_t *older;

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

								if(!writeHandler)
								{
									writeHandler= new ofstream(writeName.c_str(), ios::app);
									if(!writeHandler->is_open())
									{
										string msg("### ERROR: cannot write new file '");

										msg+= m_sThinFile + ".new'\n";
										msg+= "    ERRNO: ";
										msg+= strerror(errno);
										cout << msg << endl;
										TIMELOG(AKALERT, "writenewfile", msg);
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
					if(!writeHandler)
					{
						writeHandler= new ofstream(writeName.c_str(), ios::app);
						if(!writeHandler->is_open())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(AKALERT, "writenewfile", msg);
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
					if(!writeHandler)
					{
						writeHandler= new ofstream(writeName.c_str(), ios::app);
						if(!writeHandler->is_open())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(AKALERT, "writenewfile", msg);
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
						writeHandler= new ofstream(writeName.c_str(), ios::app);
						if(!writeHandler->is_open())
						{
							string msg("### ERROR: cannot write new file '");

							msg+= m_sThinFile + ".new'\n";
							msg+= "    ERRNO: ";
							msg+= strerror(errno);
							cout << msg << endl;
							TIMELOG(AKALERT, "writenewfile", msg);
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
				if(writeHandler)
					writeHandler->close();
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
					writeHandler= new ofstream(writeName.c_str(), ios::app);
					if(!writeHandler->is_open())
					{
						string msg("### ERROR: cannot write new file '");

						msg+= m_sThinFile + ".new'\n";
						msg+= "    ERRNO: ";
						msg+= strerror(errno);
						cout << msg << endl;
						TIMELOG(AKALERT, "writenewfile", msg);
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
		if(writeHandler)
		{
			writeHandler->close();
			newOrder= true;
		}else
		{
			files= readDirectory(m_sWorkDir, writeName, "");
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
		TIMELOG(AKERROR, "readdirectory", msg);
		return false;
	}
	// maybe an next file is to thin
	return true;
}

void Database::calcNewThinTime(time_t fromtime, const DefaultChipConfigReader::otime_t* older)
{
	DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();
	time_t acttime, nextThin;

	if(older)
	{
		if(older->older)
			older= older->older;
		time(&acttime);
		nextThin= reader->calcDate(/*newer*/true, fromtime, older->more, older->unit);
		if(nextThin <= acttime)
			nextThin= reader->calcDate(/*newer*/true, fromtime, (older->more + 1), older->unit);
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

void Database::writeIntoDb(const string folder, const string subroutine)
{
	db_t entry;

	entry.folder= folder;
	entry.subroutine= subroutine;
	m_vtDbValues.push_back(entry);
}

void Database::fillValue(string folder, string subroutine, string identif, double value, bool bNew/*=true*/)
{
	vector<double> values;

	values.push_back(value);
	fillValue(folder, subroutine, identif, values, bNew);
}

void Database::fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew/*=true*/)
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
	m_ptEntrys->push_back(newEntry);
	AROUSE(m_DBENTRYITEMSCOND);
	UNLOCK(m_DBENTRYITEMS);
}

vector<db_t>* Database::getDbEntryVector(bool bWait)
{
	struct timespec time;
	int conderror= 0;
	vector<db_t>* pRv= NULL;

	if(!bWait)
	{
		clock_gettime(CLOCK_REALTIME, &time);
		time.tv_sec+= m_nCondWait;
	}
	do{
		sleepDefaultTime();
		LOCK(m_DBENTRYITEMS);
		if(m_ptEntrys->size() != 0)
		{
			pRv= m_ptEntrys;
			m_ptEntrys= new vector<db_t>();

		}else if(bWait) {
			conderror= CONDITION(m_DBENTRYITEMSCOND, m_DBENTRYITEMS);
		}else
			conderror= TIMECONDITION(m_DBENTRYITEMSCOND, m_DBENTRYITEMS, &time);
		UNLOCK(m_DBENTRYITEMS);
		if(conderror == ETIMEDOUT)
			break;
		if(conderror)
			usleep(500000);

	}while(	pRv == NULL
			&&
			!stopping()	);

	return pRv;
}

Database::~Database()
{
	DESTROYMUTEX(m_DBENTRYITEMS);
	DESTROYMUTEX(m_DBCURRENTENTRY);
	DESTROYMUTEX(m_DBMEASURECURVES);
	DESTROYMUTEX(m_CHANGINGPOOL);
	DESTROYCOND(m_DBENTRYITEMSCOND);
	DESTROYCOND(m_CHANGINGPOOLCOND);
	delete m_ptEntrys;
}

}
