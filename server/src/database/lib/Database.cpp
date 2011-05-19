/**
 *   This file 'Database.cpp' is part of ppi-server.
 *   Created on: 22.03.2011
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

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <climits>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <memory>
#include <iostream>
//#include <sstream>
#include <fstream>
#include <map>

#include "Database.h"

#include "../../util/URL.h"
#include "../../util/Calendar.h"
#include "../../util/properties/configpropertycasher.h"

#include "../../logger/lib/LogInterface.h"


using namespace util;

namespace ppi_database
{
	Database::Database(IPropertyPattern* properties, IChipConfigReaderPattern* chipreader) :
		m_pChipReader(chipreader)
	{
		float newdbafter;
		char* pHostN= NULL;
		char hostname[]= "HOSTNAME";
		string prop;

		m_bError= false;
		m_bDbStop= false;
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
		m_sptEntrys= auto_ptr<vector<db_t> >(new vector<db_t>());
		m_DBENTRYITEMSCOND= Thread::getCondition("DBENTRYITEMSCOND");
		m_DBENTRYITEMS= Thread::getMutex("DBENTRYITEMS");
		m_DBCURRENTENTRY= Thread::getMutex("DBCURRENTENTRY");
		m_DBMEASURECURVES= Thread::getMutex("DBMEASURECURVES");
		m_CHANGINGPOOL= Thread::getMutex("CHANGINGPOOL");
		m_CHANGINGPOOLCOND= Thread::getCondition("CHANGINGPOOLCOND");
		m_STOPDB= Thread::getMutex("STOPDB");
		m_nAfter= (unsigned int)(newdbafter * 1000000);
		m_bAnyChanged= false;
		m_sWorkDir= properties->getValue("workdir");
		m_sWorkDir= URL::addPath(m_sWorkDir, PPIDATABASEPATH, /*always*/false);
		if(m_sWorkDir.substr(m_sWorkDir.length()-1, 1) != "/")
			m_sWorkDir+= "/";
		LOG(LOG_INFO, "Storage database " + m_sWorkDir);
	}

	bool Database::read()
	{
		char stime[16];
		string dbfile;
		string line;
		time_t tmDb;
		db_t entry;
		bool bNew= false;
		off_t size;
		map<string, string> files;


		m_sDbFile= getLastDbFile(m_sWorkDir, "entrys_", size);
		if(m_sDbFile == "")
		{
			dbfile= "entrys_";
			time(&tmDb);
			strftime(stime, 15, "%Y%m%d%H%M%S", localtime(&tmDb));
			m_sDbFile+= stime;
			m_sDbFile+= ".dat";
			bNew= true;
		}
		LOG(LOG_DEBUG, "beginning to read database file " + m_sDbFile);
		m_sDbFile= URL::addPath(m_sWorkDir, m_sDbFile);

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
				return false;
			}
			while(!file.eof())
			{
				getline(file, line);
				entry= splitDbLine(line);
	#if 0
				cout << line << endl;
				if(entry.folder == "TRANSMIT_SONY")
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
		}
		LOG(LOG_DEBUG, "reading of database file " + m_sDbFile + " is finished");
		if(size > 15000000)
		{// create an new database file
			createNewDbFile(/*check whether*/false);
		}

		return true;
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
		time_t tmDb;

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
		time(&tmDb);
		strftime(stime, 15, "%Y%m%d%H%M%S", localtime(&tmDb));
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

	db_t Database::splitDbLine(const string& line)
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

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// output on command line to set new value as actual
	#if 0
					ostringstream out;

					out << "DB >> write new " << entry.identif << " in ";
					out << entry.folder << ":" << entry.subroutine << endl;
					out << "      old value was " << boolalpha << tvalue.device;
					if(tvalue.device)
					{
						out << " and had content ";
						for(vector<double>::iterator it= tvalue.values.begin(); it != tvalue.values.end(); ++it)
							out << "[" << dec << *it << "] ";
					}
					out << endl;
					out << "      new value is " << boolalpha << entry.device;
					if(entry.device)
					{
						out << " and has content ";
						for(vector<double>::const_iterator it= entry.values.begin(); it != entry.values.end(); ++it)
							out << "[" << dec << *it << "] ";
					}
					out << endl;
					cout << out.str();
	#endif
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
	#if 0
						cout << "--  insert " << tvalue.folder << ":" << tvalue.subroutine << " with identifier '" << tvalue.identif << "'";
						cout << " with values ";
						for(vector<double>::const_iterator it= entry.values.begin(); it != entry.values.end(); ++it)
							cout << dec << *it << "  ";
						cout << endl;
	#endif
						UNLOCK(m_DBCURRENTENTRY);
						return true;
					}
				}
			}
		}
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}

	vector<string> Database::getChangedEntrys(unsigned long connection)
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

	unsigned short Database::existEntry(const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number/*= 0*/)
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

	auto_ptr<double> Database::getActEntry(const string& folder, const string& subroutine, const string& identif, const vector<double>::size_type number/*= 0*/)
	{
		auto_ptr<double> spnRv;
		db_t tvalue;
		map<string, map<string, db_t> >* pfEntrys;
		map<string, db_t>* psEntrys;

		LOCK(m_DBCURRENTENTRY);
		// debug display for all saved values from database or filled from measure routines
	#if 0
		static bool first= true;

		if(first)
		{
			for(map<string, map<string, map<string, db_t> > >::iterator itF= m_mCurrent.begin(); itF != m_mCurrent.end(); ++itF)
			{
				cout << "found folder " << itF->first << endl;
				for(map<string, map<string, db_t> >::iterator itS= itF->second.begin(); itS != itF->second.end(); ++itS)
				{
					cout << "  found subroutine " << itS->first << endl;
					for(map<string, db_t>::iterator itI= itS->second.begin(); itI != itS->second.end(); ++itI)
					{
						bool found= true;

						cout << "    found identifier '" << itI->first << "' with ";
						if(itI->second.values.size() < (number+1))
						{
							cout << "no ";
							found= false;
						}
						cout << dec << (number+1) << ". value ";
						if(found)
							cout << dec << itI->second.values[number];
						cout << endl;
					}
				}
			}
			first= false;
		}
		cout << "-- search for " << (number+1) << ". value in " << folder << ":" << subroutine << " with identifier " << identif << endl;
	#endif
		pfEntrys= &m_mCurrent[folder];
		if(pfEntrys->size() == 0)
		{
			UNLOCK(m_DBCURRENTENTRY);
			return spnRv;
		}
		psEntrys= &(*pfEntrys)[subroutine];
		if(psEntrys->size() == 0)
		{
			UNLOCK(m_DBCURRENTENTRY);
			return spnRv;
		}
		tvalue= (*psEntrys)[identif];
		UNLOCK(m_DBCURRENTENTRY);
		if(tvalue.folder == "")
			return spnRv;

		spnRv= auto_ptr<double>(new double(tvalue.values[number]));
		return spnRv;
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

	void Database::changeNeededIds(unsigned long oldId, unsigned long newId)
	{
		vector<db_t> entrys;

		LOCK(m_CHANGINGPOOL);
		entrys= m_mvoChanges[oldId];
		m_mvoChanges.erase(oldId);
		m_mvoChanges[newId]= entrys;
		UNLOCK(m_CHANGINGPOOL);
	}

	void Database::isEntryChanged()
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

	void Database::arouseChangingPoolCondition()
	{
		AROUSEALL(m_CHANGINGPOOLCOND);
	}

	void Database::useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip)
	{
		m_mmmvServerContent[onServer][chip][folder].push_back(subroutine);
	}

	map<string, vector<string> >* Database::getSubroutines(const string& onServer, const string& chip)
	{
		return &m_mmmvServerContent[onServer][chip];
	}

	void Database::writeDb(db_t entry, ofstream *dbfile/*= NULL*/)
	{
		typedef map<string, map<string, map<string, vector<string> > > >::iterator itServer;
		typedef map<string, map<string, vector<string> > >::iterator itChip;
		typedef map<string, vector<string> >::iterator itFolder;
		typedef vector<string>::iterator itSubroutine;
		typedef vector<double>::iterator iter;

		bool bNewFile= false;
		ofstream oNewDbFile;
		unsigned short writecount;
		vector<write_t> vWrite;
		write_t writeaccess;
		map<string, vector<string> >::iterator foundFolder;
		itSubroutine foundSubroutine;
		static map<string, vector<string> > mNeedCheck;
		static map<string, vector<string> > mNoCheck;

		if(entry.identif == "value")
		{
	#if 0
			// check all subroutines whether to write into database
			writeaccess= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, entry.values[0], entry.tm);
			cout << "write access in database for subroutine " << entry.folder << ":" << entry.subroutine << " is '" << writeaccess.action << "'" << endl;
	#else
			bool bNeedCheck;

			writeaccess.action= "write";
			// check subroutine whether should write entry into database
			// but only if the folder:subroutine is an defined chip from an owreader process
			// than the check over DefaultChipConfigReader will be used
			bNeedCheck= true;
			// search first in passing before whther map of folder and subroutines
			// for no checking be defined
			foundFolder= mNoCheck.find(entry.folder);
			if(foundFolder != mNoCheck.end())
			{
				foundSubroutine= find(foundFolder->second.begin(), foundFolder->second.end(), entry.subroutine);
				if(foundSubroutine != foundFolder->second.end())
					bNeedCheck= false;
			}
			if(bNeedCheck)
			{
				// search as second whether map of folder and subroutines
				// for needing check, in passing this routine before, be defined
				foundFolder= mNeedCheck.find(entry.folder);
				if(foundFolder != mNeedCheck.end())
				{
					foundSubroutine= find(foundFolder->second.begin(), foundFolder->second.end(), entry.subroutine);
					if(foundSubroutine != foundFolder->second.end())
						writeaccess= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, entry.values[0], entry.tm);
				}else
				{ // if not found in both container, search in container m_mmmvServerContent
				  // whether the folder:subroutine is an defined chip from owreader
				  // this search will fill the containers mNoCheck and mNeedCheck for faster next search
					//cout << "search for " << m_mmmvServerContent.size() << " server" << endl;
					bNeedCheck= false;
					for(itServer itS= m_mmmvServerContent.begin(); itS != m_mmmvServerContent.end(); ++itS)
					{
						//cout << "search for " << itS->second.size() << " chips in server " << itS->first << endl;
						for(itChip itC= itS->second.begin(); itC != itS->second.end(); ++itC)
						{
							//cout << "search for " << itC->second.size() << " folder in chip " << itC->first << endl;
							for(itFolder itF= itC->second.begin(); itF != itC->second.end(); ++itF)
							{
								if(entry.folder == itF->first)
								{
									//cout << "search for " << itF->second.size() << " subroutines in folder " << itF->first << endl;
									for(itSubroutine itSub= itF->second.begin(); itSub != itF->second.end(); ++itSub)
									{
										if(entry.subroutine == *itSub)
										{
											writeaccess= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, entry.values[0], entry.tm);
											mNeedCheck[entry.folder].push_back(entry.subroutine);
											bNeedCheck= true;
											break;
										}
									}
									if(bNeedCheck)
										break;
								}
							}
							if(bNeedCheck)
								break;
						}
						if(bNeedCheck)
							break;
					}
					if(!bNeedCheck)
						mNoCheck[entry.folder].push_back(entry.subroutine);
				}
			}
	#endif

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
		{ // if no dbfile commes in
		  // open it
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

	void Database::writeEntry(const db_t& entry, ofstream &dbfile)
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

	bool Database::thinDatabase(const bool ask)
	{
		int filecount, nextcount, count;
		db_t entry;
		time_t acttime;
		map<string, string> files, thinfiles;
		ofstream writeHandler;
		map<string, time_t>::iterator fileparsed;

		if(!m_pChipReader->chipsAreDefined())
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
		write_t write;
		SHAREDPTR::shared_ptr<otime_t> older;

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
						write= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, *v, entry.tm, &newOrder);
						if(write.action != "no")
						{
							if(newOrder)
							{
								older= m_pChipReader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/true);
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
						older= m_pChipReader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
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
						older= m_pChipReader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
						calcNewThinTime(entry.tm, older);
					}
				}else if(entry.identif == "access")
				{
					older= m_pChipReader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
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
				write= m_pChipReader->getLastValues(lcount, /*older*/true);
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
					older= m_pChipReader->getLastActiveOlder(entry.folder, entry.subroutine, /*nonactive*/false);
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

	void Database::calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<otime_t> &older)
	{
		SHAREDPTR::shared_ptr<otime_t> act;
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
		m_sptEntrys->push_back(newEntry);
		AROUSE(m_DBENTRYITEMSCOND);
		UNLOCK(m_DBENTRYITEMS);
	}

	std::auto_ptr<vector<db_t> > Database::getDbEntryVector(bool bWait)
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

	int Database::execute()
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

	bool Database::stop()
	{
		LOCK(m_STOPDB);
		m_bDbStop= true;
		UNLOCK(m_STOPDB);
		LOCK(m_DBENTRYITEMS);
		AROUSE(m_DBENTRYITEMSCOND);
		AROUSEALL(m_CHANGINGPOOLCOND);
		UNLOCK(m_DBENTRYITEMS);
		return true;
	}

	void Database::ending()
	{
		unsigned int lcount= 0;
		db_t entry;
		write_t write;
		ofstream writeHandler(m_sDbFile.c_str(), ios::app);

		if(writeHandler.is_open())
		{
			entry.device= true;
			entry.identif= "value";
			// read all entrys witch saved in any first older structure
			// and this highest value not be saved in the file
			do{
				write= m_pChipReader->getLastValues(lcount, /*older*/false);
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

	Database::~Database()
	{
		DESTROYMUTEX(m_DBENTRYITEMS);
		DESTROYMUTEX(m_DBCURRENTENTRY);
		DESTROYMUTEX(m_DBMEASURECURVES);
		DESTROYMUTEX(m_CHANGINGPOOL);
		DESTROYCOND(m_DBENTRYITEMSCOND);
		DESTROYCOND(m_CHANGINGPOOLCOND);
		DESTROYMUTEX(m_STOPDB);
	}
}