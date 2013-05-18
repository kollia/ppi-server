/**
 *   This file 'DatabaseThinning.cpp' is part of ppi-server.
 *   Created on: 21.04.2013
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

#include "DatabaseThinning.h"

#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>

#include "../../util/URL.h"

#include "../../pattern/util/LogHolderPattern.h"

namespace ppi_database
{
	using namespace std;
	using namespace util;

	int DatabaseThinning::init(void *args)
	{
		int res;

		while(	!stopping() &&
				!m_pChipReader->chipsAreDefined()	)
		{
			LOCK(m_THINNINGMUTEX);
			res= RELTIMECONDITION(m_THINNINGWAITCONDITION, m_THINNINGMUTEX, 1);
			UNLOCK(m_THINNINGMUTEX);
			if(	res != 0 &&
				res != ETIMEDOUT	)
			{
				sleep(1);// by error sleeping fix second
			}			 // when res was 0 maybe database has finished loading
		}				 // or thread will be stopping, so do not sleep
		return 0;
	}

	int DatabaseThinning::execute()
	{
		int nRv;

		// when server has loaded database an configured all folder/subroutines
		// DefaultChipConfigReader is finished and thread coming out from init() method
		// checking first whether one database file is for thinning
		// after that always when new database file created and DatabaseThinning object
		// will be aroused
		while(!stopping())
		{
			if(!thinDatabase())
				break;
		}
		LOCK(m_THINNINGMUTEX);
		if(m_nNextThinningTime == 0)
			nRv= CONDITION(m_THINNINGWAITCONDITION, m_THINNINGMUTEX);
		else
			nRv= TIMECONDITION(m_THINNINGWAITCONDITION, m_THINNINGMUTEX, m_nNextThinningTime);
		if(	nRv != 0 &&
			nRv != ETIMEDOUT	)
		{
			sleep(10);	// by error sleeping fix seconds
		}			 	// when res was 0 maybe database has finished loading
					 	// or thread will be stopping, so do not sleep

		UNLOCK(m_THINNINGMUTEX);
		return 0;
	}

	void DatabaseThinning::startDatabaseThinning()
	{
		LOCK(m_THINNINGMUTEX);
		AROUSE(m_THINNINGWAITCONDITION);
		UNLOCK(m_THINNINGMUTEX);
	}

	bool DatabaseThinning::thinDatabase()
	{
		int filecount, nextcount, count;
		db_t entry;
		time_t acttime, timediff;
		map<string, string> files, thinfiles;
		ofstream writeHandler;
		map<string, time_t>::iterator fileparsed;

		m_nNextThinningTime= 0;
		if(m_sThinFile == "")
		{ // search for the next to thin file

			files= URL::readDirectory(m_sWorkDir, "entrys_", ".dat");
			filecount= files.size();
			nextcount= m_mOldest.size();
			if((filecount - 1) != nextcount)
			{
				count= 0;
				for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
				{
					++count;
					fileparsed= m_mOldest.find(it->second.substr(0, 21));
					if(fileparsed == m_mOldest.end())
					{
						if(count != filecount)
						{// do not thin database if it is the last file
							m_sThinFile= it->second.substr(0, 21);
							m_mOldest[m_sThinFile]= 0;
						}
						break;
					}
				}
			}
			if(m_sThinFile == "")
			{
				tm l;
				char stime[22];
				string msg;

				time(&acttime);
				msg=  "next thinning time for follow database files:";
				if(m_mOldest.size() == 0)
					msg+= "\n       found no file(s) for thinning";
				for(fileparsed= m_mOldest.begin(); fileparsed != m_mOldest.end(); ++fileparsed)
				{
					localtime_r(&fileparsed->second, &l);
					strftime(stime, 21, "%d.%m.%Y %H:%M:%S", &l);
					msg+= "\n           " + fileparsed->first + " for thinning at " + string(stime);
					if(acttime >= fileparsed->second)
					{
						msg+= "\n                    THINNING FILE\n";
						m_sThinFile= fileparsed->first;
						fileparsed->second= 0;
						break;

					}else
					{
						if(	m_nNextThinningTime == 0 ||
							fileparsed->second < m_nNextThinningTime)
						{
							m_nNextThinningTime= fileparsed->second;
						}
					}
				}
				//cout << msg << endl;
				if(m_sThinFile == "")
				{
					LOG(LOG_INFO, msg);
					return false;
				}
				m_nNextThinningTime= 0;
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

		LOG(LOG_INFO, "thinning database file '" + m_sThinFile + ".dat'");
		time(&timediff);
		if(file.is_open())
		{
			while(getline(file, line))
			{
				if(stopping())
					return false;
				++nCountF;
				//cout << "line: " << line << endl;
				entry= splitDbLine(line);
				if(entry.identif == "value")
				{
					write.action= "no";

					for(vector<double>::iterator v= entry.values.begin(); v != entry.values.end(); ++v)
					{
						write= m_pChipReader->allowDbWriting(entry.folder, entry.subroutine, *v, entry.tm, &newOrder);
						//cout << "write " << write.action << endl;
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
											LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
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
								LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
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
								LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
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
								LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
								return false;
							}
						}
						writeEntry(entry, writeHandler);
					}
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
							LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
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

			tm ttime;
			ostringstream timemsg;

			line= "end correctly database thinning of file '" + m_sThinFile + ".dat'\n";
			time(&acttime);
			timediff= static_cast<time_t>(difftime(acttime, timediff));
			ttime.tm_isdst= -1;
			if(localtime_r(&timediff, &ttime) != NULL)
			{
				timemsg << "  need ";
				ttime.tm_hour-= 1;
				if(ttime.tm_hour)
				{
					timemsg << ttime.tm_hour << " hour";
					if(ttime.tm_hour > 1)
						timemsg << "s";
					timemsg << ", ";
				}
				if(	ttime.tm_min ||
					ttime.tm_hour	)
				{
					timemsg << ttime.tm_min << " minute";
					if(ttime.tm_min != 1)
						timemsg << "s";
					timemsg << " and ";
				}
				timemsg << ttime.tm_sec << " seconds";
				line+= timemsg.str();
			}else
				line+= "  ERROR: cannot define correctly time difference with localtime_r method";
			LOG(LOG_INFO, line);
			m_sThinFile= "";
		}else
		{
			string msg("### ERROR: cannot read file '");

			msg+= readName + "'\n";
			msg+= "    ERRNO: ";
			msg+= strerror(errno);
			cout << msg << endl;
			TIMELOG(LOG_ERROR, "readdirectory", msg);
			LOG(LOG_ERROR, "end database thinning file '" + m_sThinFile + ".dat' by ERROR");
			return false;
		}
		// maybe an next file is to thin
		return true;
	}

	db_t DatabaseThinning::splitDbLine(const string& line)
	{
		struct tm entryTime;
		db_t entry;
		vector<string> columns;
		vector<string>::size_type count;

		columns= ConfigPropertyCasher::split(line, "|");
		count= columns.size();

		entry.bNew= false;
		entry.device= true;
		if(count > 0)
		{
			entry.measureHost= columns[0];
			if(strptime(columns[1].c_str(), "%Y%m%d:%H%M%S", &entryTime) == NULL)
				entry.tm= 0;
			else
				entry.tm= mktime(&entryTime);
			if(localtime_r(&entry.tm, &entryTime) == NULL)
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
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

	void DatabaseThinning::writeEntry(const db_t& entry, ofstream &dbfile)
	{
		char stime[18];
		tm l;

		dbfile << entry.measureHost << "|";
		if(localtime_r(&entry.tm, &l) == NULL)
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
		strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
		dbfile << stime << "|";
		dbfile << entry.folder << "|" << entry.subroutine << "|";
		dbfile << entry.identif;
		dbfile << "|";
		if(entry.identif == "access")
		{
			if(entry.device)
				dbfile << "true";
			else
				dbfile << "NaN";
		}else
		{
			for(vector<double>::const_iterator valIt= entry.values.begin(); valIt != entry.values.end(); ++valIt)
				dbfile << dec << *valIt << "|";
		}
		dbfile << endl;
	}

	void DatabaseThinning::calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<otime_t> &older)
	{
		SHAREDPTR::shared_ptr<otime_t> act;
		time_t acttime, nextThin;
		Calendar::time_e unit(Calendar::seconds);

		act= older;
		if(act.get())
		{
			if(act->older) // next thinning at next older structure
				act= act->older;
			time(&acttime);
			if(act->unit == 's')
				unit= Calendar::seconds;
			else if(act->unit == 'm')
				unit= Calendar::minutes;
			else if(act->unit == 'h')
				unit= Calendar::hours;
			else if(act->unit == 'D')
				unit= Calendar::days;
			else if(act->unit == 'W')
				unit= Calendar::weeks;
			else if(act->unit == 'M')
				unit= Calendar::months;
			else if(act->unit == 'Y')
				unit= Calendar::years;
			else
			{
				string msg(&act->unit);

				msg= "undefined time unit '" + msg + "' for calendar\nset older calculation to one year";
				TIMELOG(LOG_ALERT, "time_units"+string(&act->unit), msg);
				unit= Calendar::years;
				act->more= 1;
			}
			nextThin= Calendar::calcDate(/*newer*/true, fromtime, act->more, unit);
			if(nextThin <= acttime)
				nextThin= Calendar::calcDate(/*newer*/true, fromtime, (act->more + 1), unit);
		/*	tm tmstr;
			char ctime[23];
			localtime_r(&fromtime, &tmstr);
			strftime(ctime, 21, "%Y.%m.%d %H:%M:%S", &tmstr);
			cout << "   actual file time " << string(ctime) << " after " << act->more << act->unit << endl;
			localtime_r(&nextThin, &tmstr);
			strftime(ctime, 21, "%Y.%m.%d %H:%M:%S", &tmstr);
			cout << "          mext thinning " << string(ctime) << endl;*/
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

	int DatabaseThinning::stop(const bool* bWait)
	{
		int nRv;

		nRv= Thread::stop(false);
		if(nRv != 0)
			return nRv;
		AROUSE(m_THINNINGWAITCONDITION);
		if(	bWait != NULL &&
			*bWait == true	)
		{
			nRv= Thread::stop(bWait);
		}
		return nRv;
	}

} /* namespace ppi_database */
