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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Database.h"

#include "../../util/URL.h"
#include "../../util/Calendar.h"
#include "../../util/properties/configpropertycasher.h"

#include "../../pattern/util/LogHolderPattern.h"


using namespace util;
using namespace boost;

namespace ppi_database
{
	Database::Database(IPropertyPattern* properties, IChipConfigReaderPattern* chipreader) :
		m_pChipReader(chipreader)
	{
		float newdbafter;
		unsigned int nSleepAll;
		char* pHostN= NULL;
		char hostname[]= "HOSTNAME";
		string prop;

		m_bError= false;
		m_bDbStop= false;
		m_sConfigureLevel= "NONE";
		prop= "newdbafter";
		newdbafter= (float)properties->getDouble(prop);
		if(newdbafter == 0)
			newdbafter= 15;
		pHostN= getenv(hostname);
		if(pHostN == NULL)
			m_sMeasureName= "noHostDefined";
		else
			m_sMeasureName= *pHostN;
		m_sptEntrys= auto_ptr<vector<db_t> >(new vector<db_t>());
		m_SERVERSTARTINGMUTEX= Thread::getMutex("SERVERSTARTINGMUTEX");
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
		m_pEndReading= NULL;
		m_nReadBlock= 0;
		prop= "sleepAllThinRows";
		nSleepAll= properties->getUInt(prop, /*warning*/false);
		if(prop != "sleepAllThinRows")
		{
			string msg("### WARNING: property 'sleepAllThinRows' is not set inside server.conf\n"
							"             so set property to default sleeping of 1000");

			cout << msg << endl;
			LOG(LOG_WARNING, msg);
			nSleepAll= 1000;
		}
		m_oDbThinning= std::auto_ptr<DatabaseThinning>(new DatabaseThinning(m_sWorkDir,
												chipreader, static_cast<__useconds_t>(nSleepAll)));
		m_oDbThinning->start();
	}

	void Database::setServerConfigureStatus(const string& sProcess, const short& nPercent)
	{
		LOCK(m_SERVERSTARTINGMUTEX);
		m_sConfigureLevel= sProcess;
		m_nConfPercent= nPercent;
		UNLOCK(m_SERVERSTARTINGMUTEX);
	}

	bool Database::isServerConfigured(string& sProcess, short& nPercent)
	{
		bool bRv(false);

		LOCK(m_SERVERSTARTINGMUTEX);
		if(m_sConfigureLevel == "NONE")
		{
			sProcess= "starting";
			nPercent= -1;

		}else if(	m_sConfigureLevel == "database" &&
					m_nConfPercent == -1				)
		{
			sProcess= m_sConfigureLevel;
			if(m_nDbLoaded != -1)
				nPercent= static_cast<short>(100 / static_cast<float>(m_nDbSize) * m_nDbLoaded);
			else
				nPercent= -1;
		}else
		{
			sProcess= m_sConfigureLevel;
			nPercent= m_nConfPercent;
			if(m_sConfigureLevel == "finished")
				bRv= true;
		}
		UNLOCK(m_SERVERSTARTINGMUTEX);
		return bRv;
	}

	bool Database::read()
	{
		typedef vector<pair<pair<pair<string, string>, double>, pair<pair<string, string>, double> > >::iterator readVector;
		char stime[16];
		string dbfile;
		string line;
		time_t tmDb;
		tm l;
		db_t entry;
		bool bNew= false;
		off_t size, loaded;
		map<string, string> files;


		m_sDbFile= getLastDbFile(m_sWorkDir, "entrys_", size);
		LOCK(m_SERVERSTARTINGMUTEX);
		m_nDbSize= size;
		m_nDbLoaded= -1;
		UNLOCK(m_SERVERSTARTINGMUTEX);
		loaded= 0;
		if(m_sDbFile == "")
		{
			m_sDbFile= "entrys_";
			time(&tmDb);
			if(localtime_r(&tmDb, &l) == NULL)
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			strftime(stime, 15, "%Y%m%d%H%M%S", &l);
			m_sDbFile+= stime;
			m_sDbFile+= ".dat";
			bNew= true;
		}
		LOG(LOG_INFO, "beginning to read database file " + m_sDbFile);
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
				loaded= static_cast<off_t>(file.tellg());
				LOCK(m_SERVERSTARTINGMUTEX);
				if(loaded > 0)// when end of file reached or error occurred, loaded is -1
					m_nDbLoaded= loaded;
				UNLOCK(m_SERVERSTARTINGMUTEX);
				entry= splitDbLine(line);
				if(entry.bNew == false)
					entry.identif= "";// line of database was corrupt
				else
					entry.bNew= false;
#if 0
				if(	(	entry.folder == "SONY_CMT_MINUS_CP100_KEY_CHANNELUP" ||
						entry.folder == "SONY_CMT_MINUS_CP100_KEY_CHANNELDOWN"	) &&
					entry.subroutine == "actual_step"									)
				{
					cout << line << endl;
					cout << "device  " << entry.folder << ":" << entry.subroutine << endl;
					cout << " access " << boolalpha << entry.device << endl;
					cout << " value  " << entry.values[0] << endl;
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
					{
						setActEntry(entry);
						if(m_pEndReading == NULL)
						{
							for(readVector vec= m_vReadBlockDefs.begin(); vec != m_vReadBlockDefs.end(); ++vec)
							{
								if(	vec->first.first.first == entry.folder &&
									vec->first.first.second == entry.subroutine &&
									vec->first.second == entry.values[0]				)
								{
									vector<db_t> first;

									m_pEndReading= &vec->second;
									first.push_back(entry);
									m_vvDbEntrys.push_back(first);
									m_nReadBlock= m_vvDbEntrys.size()-1;
								}
							}
						}else
						{
							m_vvDbEntrys[m_nReadBlock].push_back(entry);
							if(	m_pEndReading->first.first == entry.folder &&
								m_pEndReading->first.second == entry.subroutine &&
								m_pEndReading->second == entry.values[0]			)
							{
								m_pEndReading= NULL;
							}
						}
					}
				}
			}
			file.close();
		}
		if(size > 15000000)
		{// create an new database file
			createNewDbFile(/*check whether*/false);
		}
		line= "reading of database file " + m_sDbFile + " is finished";
		LOG(LOG_INFO, line);
		m_oDbThinning->startDatabaseThinning();

//	for debugging write out current content of database before ending
#if 0
		LOCK(m_DBCURRENTENTRY);
		cout << endl;
		cout << line << endl;
		cout << "found follow content:" << endl;
		cout << endl;
		for(map<string, map<string, map<string, db_t> > >::iterator fit= m_mCurrent.begin(); fit != m_mCurrent.end(); ++fit)
		{
			if(fit->first == "writeVellemann0")
			{
				for(map<string, map<string, db_t> >::iterator sit= fit->second.begin(); sit != fit->second.end(); ++sit)
				{
					for(map<string, db_t>::iterator iit= sit->second.begin(); iit != sit->second.end(); ++iit)
					{
						cout << "for folder:'" << fit->first << "' subroutine:'" << sit->first << "' and identifier:'" << iit->first << "' found:" << endl;
						cout << "              folder:     " << iit->second.folder << endl;
						cout << "              subroutine: " << iit->second.subroutine << endl;
						cout << "              identifier: " << iit->second.identif << endl;
						cout << "              access:     " << boolalpha << iit->second.device << endl;
						cout << "              value:      ";
						for(vector<double>::iterator vit= iit->second.values.begin(); vit != iit->second.values.end(); ++vit)
						{
							if(vit != iit->second.values.begin())
								cout << "                          ";
							cout << *vit << endl;
						}
						cout << "          write only new: " << boolalpha << iit->second.bNew << endl;
					}
				}
			}
		}
		UNLOCK(m_DBCURRENTENTRY);
#endif
		return true;
	}

	void Database::readInsideSubroutine(const string& fromsub, const double from, const string& tosub,
					const double to, const unsigned short ncount)
	{
		vector<string> fspl, tspl;

		split(fspl, fromsub, is_any_of(":"));
		split(tspl, tosub, is_any_of(":"));

		pair<string, string> fromSub(fspl[0], fspl[1]);
		pair<string, string> toSub(tspl[0], tspl[1]);
		pair<pair<string, string>, double> From(fromSub, from);
		pair<pair<string, string>, double> To(toSub, to);
		pair<pair<pair<string, string>, double>, pair<pair<string, string>, double> > block(From, To);

		m_vReadBlockDefs.push_back(block);
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
		tm l;

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
		if(localtime_r(&tmDb, &l) == NULL)
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
		strftime(stime, 15, "%Y%m%d%H%M%S", &l);
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
		m_oDbThinning->startDatabaseThinning();
		return true;
	}

	db_t Database::splitDbLine(const string& line)
	{
		struct tm entryTime;
		db_t entry;
		vector<string> columns;
		vector<string>::size_type count;

		split(columns, line, is_any_of("|"));
		count= columns.size();

		entry.bNew= false;
		entry.device= true;
		if(count > 0)
		{
			entry.measureHost= columns[0];
			if(count > 1)
			{
				string timestr;
				vector<string> timespl;
				vector<string>::size_type nLen;
				timeval tv;

				split(timespl, columns[1], is_any_of(":"));
				nLen= timespl.size();
				if(nLen > 0)
				{
					timestr= timespl[0] + ":";
					if(nLen > 1)
					{
						timestr+= timespl[1];
						if(strptime(timestr.c_str(), "%Y%m%d:%H%M%S", &entryTime) == NULL)
						{
							string msg("ERROR: cannot read correctly time string '");

							msg+= columns[1] + "' inside database string\n";
							msg+= "       '" + line + "'\n";
							msg+= "       so remove this value from database";
							TIMELOG(LOG_ALERT, "gettimeofday", msg);
							return entry;
						}else
						{
							time_t nMkTime;

							entryTime.tm_isdst= -1;
							nMkTime= mktime(&entryTime);
							if(nMkTime == -1)
							{
								string msg("ERROR: cannot read correctly time string '");

								msg+= columns[1] + "' inside database string\n";
								msg+= "       '" + line + "'\n";
								msg+= "       so remove this value from database";
								TIMELOG(LOG_ALERT, "gettimeofday", msg);
								return entry;
							}else
								entry.tm.tv_sec= nMkTime;
						}
						if(nLen < 3)
						{
							if(gettimeofday(&tv, NULL))
							{
								string msg("ERROR: cannot get time of day to make new microtime for database entry,\n");

								msg+= "       so maybe by sorting lost any value(s) ";
								TIMELOG(LOG_ALERT, "gettimeofday", msg);
								entry.tm.tv_usec= 0;
							}else
								entry.tm.tv_usec= tv.tv_usec;
						}else
						{
							istringstream microseconds(timespl[2]);
							microseconds >> entry.tm.tv_usec;
						}

					}else
					{
						string msg("ERROR: cannot read correctly time string '");

						msg+= columns[1] + "' inside database string\n";
						msg+= "       '" + line + "'\n";
						msg+= "       so remove this value from database";
						TIMELOG(LOG_ALERT, "gettimeofday", msg);
						return entry;
					}
				}else
				{
					string msg("ERROR: cannot read correctly time string '");

					msg+= columns[1] + "' inside database string\n";
					msg+= "       '" + line + "'\n";
					msg+= "       so remove this value from database";
					TIMELOG(LOG_ALERT, "gettimeofday", msg);
					return entry;
				}
				if(count > 2)
				{
					entry.folder= columns[2];
					if(count > 3)
					{
						entry.subroutine= columns[3];
						if(count > 4)
						{
							entry.identif= columns[4];
							if(count > 5)
							{
								entry.bNew= true;
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
							}// if(count > 5)
						}// if(count > 4)
					}// if(count > 3)
				}// if(count > 2)
			}// if(count > 1)
		}// if(count > 0)
		return entry;
	}

	bool Database::setActEntry(const db_t entry)
	{
		db_t tvalue;
		string identif;
		map<string, map<string, db_t> > fEntrys;
		map<string, db_t> sEntrys;

#if 0
#define write_out_found_written_content
		if(	entry.folder == "writeVellemann0" &&
			entry.subroutine == "digital07"		)
		{
			cout << endl;
			cout << "read " << entry.folder << ":" << entry.subroutine << " with value "
							<< entry.values[0] << " and access " << boolalpha << entry.device << endl;
		}
#endif
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
			write_debug_output(entry, NULL, "first insert of folder");
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
				write_debug_output(entry, NULL, "first insert of subroutine in folder " + entry.folder);
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
					write_debug_output(entry, &tvalue, "first insert of new identifier " + entry.identif);
					m_mCurrent[entry.folder][entry.subroutine][entry.identif]= entry;
					UNLOCK(m_DBCURRENTENTRY);
					return true;
				}else
				{
					bool changed= true;
					vector<double>::size_type tcount= tvalue.values.size();
					vector<double>::size_type ecount= entry.values.size();

					if(entry.identif == "access")
					{
						if(tvalue.device != entry.device)
						{
							write_debug_output(entry, &tvalue, "write access identifier");
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
						write_debug_output(entry, &tvalue, "insert databes entry");
						UNLOCK(m_DBCURRENTENTRY);
						return true;
					}
				}
			}
		}
		write_debug_output(entry, NULL, "do not write any content -----------------------------------------");
		UNLOCK(m_DBCURRENTENTRY);
		return false;
	}

	void Database::write_debug_output(const db_t& entry, const db_t* oldentry, const string& message)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// output on command line to set new value as actual
#ifdef write_out_found_written_content

		if(	entry.folder == "writeVellemann0" &&
			entry.subroutine == "digital07"		)
		{
			ostringstream out;

			if(message != "")
				out << message << ":" << endl;
			out << "              folder:     " << entry.folder << endl;
			out << "              subroutine: " << entry.subroutine << endl;
			out << "              identifier: " << entry.identif << endl;
			out << "              access:     " << boolalpha << entry.device << endl;
			out << "              value:      ";
			for(vector<double>::const_iterator vit= entry.values.begin(); vit != entry.values.end(); ++vit)
			{
				if(vit != entry.values.begin())
					out << "                          ";
				out << *vit << endl;
			}
			out << "          write only new: " << boolalpha << entry.bNew << endl;
			if(oldentry)
			{
				out << "      old value was " << boolalpha << oldentry->device;
				if(oldentry->device)
				{
					out << " and had content ";
					for(vector<double>::const_iterator it= oldentry->values.begin(); it != oldentry->values.end(); ++it)
						out << "[" << dec << *it << "] ";
				}
				out << endl;
			}
			out << "      new value is " << boolalpha << entry.device;
			if(entry.device)
			{
				out << " and has content ";
				for(vector<double>::const_iterator it= entry.values.begin(); it != entry.values.end(); ++it)
					out << "[" << dec << *it << "] ";
			}
			out << endl;
			cout << out.str();
		}
#endif
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

				info << "read_owserver_debuginfo " << i->tm.tv_sec;
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

							if(	i->device != iEntrys->second.device ||
								newvalue < actvalue ||
								newvalue > actvalue	||
								iEntrys->second.bNew					)
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
			newentry.tm.tv_sec= (time_t)atoi(name.c_str());
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
		m_mmmvServerContent[onServer][chip][folder].insert(subroutine);
	}

	map<string, set<string> >* Database::getSubroutines(const string& onServer, const string& chip)
	{
		map<string, set<string> >* pRv;

		pRv= &m_mmmvServerContent[onServer][chip];
		return pRv;
	}

	void Database::writeDb(db_t entry, ofstream *dbfile/*= NULL*/)
	{
		typedef map<string, map<string, map<string, set<string> > > >::iterator itServer;
		typedef map<string, map<string, set<string> > >::iterator itChip;
		typedef map<string, set<string> >::iterator itFolder;
		typedef set<string>::iterator itSubroutine;
		typedef vector<double>::iterator iter;

		bool bNewFile= false;
		ofstream oNewDbFile;
		unsigned short writecount;
		vector<write_t> vWrite;
		write_t writeaccess;
		map<string, set<string> >::iterator foundFolder;
		itSubroutine foundSubroutine;
		static map<string, set<string> > mNeedCheck;
		static map<string, set<string> > mNoCheck;

		if(entry.identif == "value")
		{
	#if 1
			// check all subroutines whether to write into database
			writeaccess= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, entry.values[0], entry.tm);
			//cout << "write access in database for subroutine " << entry.folder << ":" << entry.subroutine << " is '" << writeaccess.action << "'" << endl;
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
				foundSubroutine= foundFolder->second.find(entry.subroutine);
				//foundSubroutine= find(foundFolder->second.begin(), foundFolder->second.end(), entry.subroutine);
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
											mNeedCheck[entry.folder].insert(entry.subroutine);
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
						mNoCheck[entry.folder].insert(entry.subroutine);
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
		ostringstream line, otime;
		char ctime[18];
		tm l;

		if(entry.measureHost == "")
		{
			if(m_sMeasureName == "")
				line << "noHostDefined";
			else
				line << m_sMeasureName;
		}else
			line << entry.measureHost;
		line << "|";
		if(localtime_r(&entry.tm.tv_sec, &l) == NULL)
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
		strftime(ctime, 16, "%Y%m%d:%H%M%S", &l);
		otime << ctime << ":";
		otime.width(6);
		otime.fill('0');
		otime.setf(ios_base::right);
		otime << entry.tm.tv_usec;
		line << otime.str() << "|";
		line << entry.folder << "|" << entry.subroutine << "|";
		line << entry.identif;
		line << "|";
		if(entry.identif == "access")
		{
			if(entry.device)
				line << "true";
			else
				line << "NaN";
		}else
		{
			for(vector<double>::const_iterator valIt= entry.values.begin(); valIt != entry.values.end(); ++valIt)
				line << dec << *valIt << "|";
		}
		dbfile << line.str() << endl;
	}

	void Database::writeIntoDb(const string folder, const string subroutine)
	{
		db_t entry;

		entry.folder= folder;
		entry.subroutine= subroutine;
		m_vtDbValues.push_back(entry);
	}

	void Database::fillValue(string folder, string subroutine, string identif, double value, bool bNew/*=false*/)
	{
		vector<double> values;

		values.push_back(value);
		fillValue(folder, subroutine, identif, values, bNew);
	}

	void Database::fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew/*=false*/)
	{
		db_t newEntry;

		if(gettimeofday(&newEntry.tm, NULL))
		{
			string msg("ERROR: cannot get time of day to make new time for database filling,\n");

			msg+= "       so make time without microseconds ";
			TIMELOG(LOG_ALERT, "gettimeofday", msg);
			time(&newEntry.tm.tv_sec);
			newEntry.tm.tv_usec= 0;
		}
		newEntry.folder= folder;
		newEntry.subroutine= subroutine;
		newEntry.identif= identif;
		newEntry.values= values;
		newEntry.bNew= bNew;
		if(identif == "access")
			newEntry.device= (bool)values.back();
		else // if identif not 'access' device should be true
		{	 // because it will be checked in method setActEntry()
			 // whether device in memory is true and an new value
			newEntry.device= true;
		}

		LOCK(m_DBENTRYITEMS);
		m_sptEntrys->push_back(newEntry);
		AROUSE(m_DBENTRYITEMSCOND);
		UNLOCK(m_DBENTRYITEMS);
	}

	std::auto_ptr<vector<db_t> > Database::getDbEntryVector()
	{
		int conderror= 0;
		std::auto_ptr<vector<db_t> > pRv(new vector<db_t>());

		do{
			conderror= 0;
			LOCK(m_DBENTRYITEMS);
			if(m_sptEntrys->size() != 0)
			{
				pRv= m_sptEntrys;
				m_sptEntrys= std::auto_ptr<vector<db_t> >(new vector<db_t>());

			}else
				conderror= CONDITION(m_DBENTRYITEMSCOND, m_DBENTRYITEMS);
			UNLOCK(m_DBENTRYITEMS);
			if(conderror)
				usleep(500000);

		}while(	pRv->size() == 0 &&
				!stopping()			);

		return pRv;
	}

	int Database::execute()
	{
		bool bNewValue;
		std::auto_ptr<vector<db_t> > entrys= getDbEntryVector();

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
					{
						//cout << "write this " << entry.folder << ":" << entry.subroutine << " into database" << endl;
						writeDb(*i);
					}
				}
			}
		}
		createNewDbFile(/*check whether*/true);
		return 0;
	}

	bool Database::stop()
	{
		LOCK(m_STOPDB);
		m_bDbStop= true;
		UNLOCK(m_STOPDB);
		m_oDbThinning->stop(true);
		LOCK(m_DBENTRYITEMS);
		AROUSE(m_DBENTRYITEMSCOND);
		AROUSEALL(m_CHANGINGPOOLCOND);
		UNLOCK(m_DBENTRYITEMS);
		return true;
	}

	void Database::ending()
	{
		db_t entry;
		write_t write;
		ofstream writeHandler(m_sDbFile.c_str(), ios::app);

		if(writeHandler.is_open())
		{
			entry.device= true;
			entry.identif= "value";
			// read all entry's witch saved in any first older structure
			// and this highest value not be saved in the file
			do{
				write= m_pChipReader->getLastValues(/*older*/false);
				if(	write.action == "highest"
					||
					write.action == "fractions"	)
				{
					entry.folder= write.folder;
					entry.subroutine= write.subroutine;
					entry.values.clear();
					entry.values.push_back(write.highest.highest);
					entry.tm= write.highest.hightime;
					if(entry.tm.tv_sec != 0)
						writeEntry(entry, writeHandler);
					if(write.action == "highest")
					{
						entry.values.clear();
						entry.values.push_back(write.highest.lowest);
						entry.tm= write.highest.lowtime;
						if(entry.tm.tv_sec != 0)
							writeEntry(entry, writeHandler);
					}
				}
			}while(write.action != "kill");
			writeHandler.close();
		}else
		{
			string msg("### ERROR: cannot write into '");

			msg+= m_sDbFile + " by ending application'\n";
			msg+= "    ERRNO: ";
			msg+= strerror(errno);
			cout << msg << endl;
			LOG(LOG_ERROR, msg);
		}

	}

	Database::~Database()
	{
		DESTROYMUTEX(m_SERVERSTARTINGMUTEX);
		DESTROYMUTEX(m_DBENTRYITEMS);
		DESTROYMUTEX(m_DBCURRENTENTRY);
		DESTROYMUTEX(m_DBMEASURECURVES);
		DESTROYMUTEX(m_CHANGINGPOOL);
		DESTROYCOND(m_DBENTRYITEMSCOND);
		DESTROYCOND(m_CHANGINGPOOLCOND);
		DESTROYMUTEX(m_STOPDB);
	}
}
