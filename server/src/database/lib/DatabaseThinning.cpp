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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../../util/URL.h"

#include "../../pattern/util/LogHolderPattern.h"

namespace ppi_database
{
	using namespace std;
	using namespace util;
	using namespace boost;

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

		// when server has loaded database and configured all folder/subroutines
		// DefaultChipConfigReader is finished and thread coming out from init() method
		// checking first whether one database file is for thinning
		// after that always when new database file created
		// or DatabaseThinning object will be aroused for any archived DB file
		while(!stopping())
		{
			if(!thinDatabase())
				break;
		}
		if(!stopping())
		{
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
		}
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
		/**
		 * whether deleting lines from actual archive database file
		 * to know when deleting one or more to write an new one
		 * otherwise make no changes
		 */
		bool bDeleteLine(false);
		int filecount, nextcount, count;
		db_t rentry, wentry;
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
					msg+= "\n           " + fileparsed->first;
					if(fileparsed->second > 0)
					{
						localtime_r(&fileparsed->second, &l);
						strftime(stime, 21, "%d.%m.%Y %H:%M:%S", &l);
						msg+= " for thinning at " + string(stime);
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
					}else
						msg+= " isn't defined for thinning";
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
		unsigned short nWriting(0);
		const unsigned short nSleepAll(1000);
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
			m_pChipReader->setDbOlderNull();
			m_msNewFile.clear();
			while(getline(file, line))
			{
				usleep(m_nSleepAfterRows);// sleep to holding the process activity lower
				if(stopping())
				{
					file.close();
					return false;
				}
				//cout << "line: " << line << endl;
				rentry= splitDbLine(line);
				if(rentry.bNew == false)
					rentry.identif= ""; // line of database was corrupt
				do{ // while newOrder == true)
					newOrder= false;
					if(rentry.identif == "value")
					{
						double value;

						if(rentry.values.size() == 0)
							break;//fault database entry
						value= rentry.values[0];
						write= m_pChipReader->allowDbWriting(rentry.folder, rentry.subroutine, value, rentry.tm,
																				/*thinning*/"value", &newOrder);
						//cout << "write " << write.action << endl;
						if(	write.action == "write" ||
							write.action == "fractions"	)
						{
							wentry.bNew= true;
							wentry.device= true;
							wentry.folder= write.folder;
							wentry.subroutine= write.subroutine;
							wentry.identif= rentry.identif;
							wentry.measureHost= rentry.measureHost;
							wentry.values.clear(); // for action fraction and write, the value and time
							wentry.values.push_back(write.highest.highest); // will be written into higest
							wentry.tm= write.highest.hightime;
							writeEntry(wentry);
							older= m_pChipReader->getLastActiveOlder(wentry.folder, wentry.subroutine, /*nonactive*/false);
							calcNewThinTime(rentry.tm.tv_sec, older);

						}else if(write.action == "highest")
						{
							wentry.device= true;
							wentry.folder= write.folder;
							wentry.subroutine= write.subroutine;
							wentry.identif= rentry.identif;
							wentry.measureHost= rentry.measureHost;
							wentry.values.clear();
							wentry.values.push_back(write.highest.lowest);
							wentry.tm= write.highest.lowtime;
							writeEntry(wentry);
							wentry.values.clear();
							wentry.values.push_back(write.highest.highest);
							wentry.tm= write.highest.hightime;
							writeEntry(wentry);
							if(timercmp(&write.highest.lowtime, &write.highest.hightime, >))
								wentry.tm= write.highest.lowtime;// <- if not hightime be set
							older= m_pChipReader->getLastActiveOlder(wentry.folder, wentry.subroutine, /*nonactive*/false);
							calcNewThinTime(wentry.tm.tv_sec, older);
						}

					}else if(rentry.identif == "access")
					{
						write= m_pChipReader->allowDbWriting(rentry.folder, rentry.subroutine, /*not needed*/0, rentry.tm, /*thinning*/"access");
						if(write.action == "write")
						{
							writeEntry(rentry);
							older= m_pChipReader->getLastActiveOlder(rentry.folder, rentry.subroutine, /*nonactive*/true);
							calcNewThinTime(rentry.tm.tv_sec, older);

						}
					}
				}while(newOrder == true);
			}// while(getline(file, line))

			rentry.device= true;
			rentry.identif= "value";
			// read all entrys witch saved in any older structures
			// and this value not saved in the files
			do{
				write= m_pChipReader->getLastValues(/*older*/true);
				if(	write.action == "highest"
					||
					write.action == "fractions"	)
				{
					rentry.folder= write.folder;
					rentry.subroutine= write.subroutine;
					rentry.values.clear();
					rentry.values.push_back(write.highest.highest);
					rentry.tm= write.highest.hightime;
					if(rentry.tm.tv_sec > 0)
						writeEntry(rentry);
					if(write.action == "highest")
					{
						rentry.values.clear();
						rentry.values.push_back(write.highest.lowest);
						rentry.tm= write.highest.lowtime;
						if(rentry.tm.tv_sec > 0)
							writeEntry(rentry);
					}
					older= m_pChipReader->getLastActiveOlder(rentry.folder, rentry.subroutine, /*nonactive*/false);
					calcNewThinTime(rentry.tm.tv_sec, older);
				}
			}while(write.action != "kill");

			newOrder= false;
			bDeleteLine= false;
			nWriting= 0;
			// check whether any entries be deleted
			file.seekg(0, ios_base::beg);
			if(!file.good())
			{
				file.close();
				file.open(readName.c_str(), ifstream::in);
			}
			for(map<timeval, string, TimeSort>::iterator it= m_msNewFile.begin(); it != m_msNewFile.end(); ++it)
			{
				++nWriting;
				if(!getline(file, line))
				{// more lines by defined as in original database file
					bDeleteLine= true;	// this case shouldn't
					break;				// but only do as secure
				}
				if(it->second != line)
				{ // an different value be found, database file is thiner
					bDeleteLine= true;
					break;
				}
				if(nWriting >= nSleepAll)
				{
					usleep(m_nSleepAfterRows);// sleep to holding the process activity lower
					if(stopping())
					{
						file.close();
						return false;
					}
					nWriting= 0;
				}
			}
			if(bDeleteLine)
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
				for(map<timeval, string, TimeSort>::iterator it= m_msNewFile.begin(); it != m_msNewFile.end(); ++it)
				{
					++nWriting;
					writeHandler << it->second << endl;
					if(nWriting >= nSleepAll)
					{
						usleep(m_nSleepAfterRows);// sleep to holding the process activity lower
						if(stopping())
						{
							file.close();
							return false;
						}
						nWriting= 0;
					}
				}
			}
			m_msNewFile.clear();

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
			if(bDeleteLine)
				unlink(readName.c_str());
			if(newOrder)
				rename(doneName.c_str(), readName.c_str());

			tm ttime;
			ostringstream timemsg;

			line= "end correctly database thinning of file '" + m_sThinFile + ".dat'\n";
			if(!bDeleteLine)
				line+= "no changing of file be done (only checking)\n";
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

		}else // if(file.is_open())
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

		split(columns, line, is_any_of("|"));
		count= columns.size();

		entry.bNew= false;
		entry.device= true;
		if(count > 0)
		{
			trim(columns[0]);
			if(line[0] == '#')
				return entry;
			entry.measureHost= columns[0];
			if(count > 1)
			{
				string timestr;
				vector<string> timespl;
				vector<string>::size_type nLen;
				timeval tv;

				trim(columns[1]);
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
					trim(columns[2]);
					entry.folder= columns[2];
					if(count > 3)
					{
						trim(columns[3]);
						entry.subroutine= columns[3];
						if(count > 4)
						{
							trim(columns[4]);
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
							// DEBUG output
							/*	char ctime[18];
								tm l;
								localtime_r(&entry.tm.tv_sec, &l);
								strftime(ctime, 16, "%Y%m%d:%H%M%S", &l);
								cout << "           read time:       " << entry.tm.tv_sec << " " << entry.tm.tv_usec << "  is " << ctime << ":";
											cout.width(6);
											cout.fill('0');
											cout << entry.tm.tv_usec << endl;
								cout << "                folder:     " << entry.folder << endl;
								cout << "                subroutine: " << entry.subroutine << endl;
								cout << "                values:     ";
								for(vector<double>::iterator it= entry.values.begin(); it != entry.values.end(); ++it)
									cout << *it << " ";
								cout << endl;*/
							}// if(count > 5)
						}// if(count > 4)
					}// if(count > 3)
				}// if(count > 2)
			}// if(count > 1)
		}// if(count > 0)
		return entry;
	}

	void DatabaseThinning::writeEntry(const db_t& entry)
	{
		ostringstream line, otime;
		char ctime[18];
		tm l;

		line << entry.measureHost << "|";
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
			{
				string number;
				ostringstream str;
				string::size_type nLen, n;

				str << fixed << *valIt;
				number= str.str();
				nLen= number.length();
				for(n= nLen - 1; n > 0; --n)
				{
					if(number[n] != '0')
						break;
				}
				if(number[n] == '.')
					--n;
				++n;
				line << dec << number.substr(0, n) << "|";
			}
		}
		m_msNewFile[entry.tm]= line.str();
		//cout << "       get seconds     : " << entry.tm.tv_sec << endl;
		//cout << "           microseconds: " << entry.tm.tv_usec << endl;
		//cout << "write: " << line.str() << endl;
	}

	void DatabaseThinning::calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<otime_t> &older)
	{
		unsigned short more;
		SHAREDPTR::shared_ptr<otime_t> act;
		time_t acttime, nextThin;
		Calendar::time_e unit(Calendar::seconds);
		map<string, time_t>::iterator timeIt;

		act= older;
		if(act.get())
		{
			if(act->older == NULL)
				return; // do not thinning, because act older thinning be done and no next older time exist
			act= act->older;// next thinning at next older structure
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
			more= act->more;
			if(	more != 0 ||
				unit != Calendar::days	)
			{
				if(more == 0)
					more= 1;
				nextThin= Calendar::calcDate(/*newer*/true, fromtime, more, unit);
				while(nextThin <= acttime)
				{
					++more;
					nextThin= Calendar::calcDate(/*newer*/true, fromtime, more, unit);
				}
			}else // database entry is not defined for thinning
				return;
		/*	tm tmstr;
			char ctime[23];
			localtime_r(&fromtime, &tmstr);
			strftime(ctime, 21, "%Y.%m.%d %H:%M:%S", &tmstr);
			cout << "   actual file time " << string(ctime) << " after " << act->more << act->unit << endl;
			localtime_r(&nextThin, &tmstr);
			strftime(ctime, 21, "%Y.%m.%d %H:%M:%S", &tmstr);
			cout << "          mext thinning " << string(ctime) << endl;*/

			timeIt= m_mOldest.find(m_sThinFile);
			if(timeIt != m_mOldest.end())
			{
				if(	timeIt->second > nextThin ||
					timeIt->second == 0			)
				{
					timeIt->second= nextThin;
				}
			}else
				m_mOldest[m_sThinFile]= nextThin;
		}
	}

	int DatabaseThinning::stop(const bool* bWait)
	{
		int nRv;

		nRv= Thread::stop(false);
		if(nRv != 0)
			return nRv;
		AROUSEALL(m_THINNINGWAITCONDITION);
		if(	bWait != NULL &&
			*bWait == true	)
		{
			nRv= Thread::stop(bWait);
		}
		return nRv;
	}

} /* namespace ppi_database */
