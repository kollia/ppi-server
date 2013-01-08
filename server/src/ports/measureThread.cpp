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
#include <sys/io.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <sstream>

#include "../util/debug.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../ports/ExternPort.h"

#include "measureThread.h"

using namespace std;
using namespace ports;

SHAREDPTR::shared_ptr<meash_t> meash_t::firstInstance= SHAREDPTR::shared_ptr<meash_t>();
string meash_t::clientPath= "";

MeasureThread::MeasureThread(const string& threadname, const time_t& nServerSearch) :
Thread(threadname, /*defaultSleep*/0)
{
#ifdef DEBUG
	cout << "constructor of measurethread for folder " << getThreadName() << endl;
#endif // DEBUG
	m_nServerSearchSeconds= nServerSearch;
	m_DEBUGLOCK= Thread::getMutex("DEBUGLOCK");
	m_VALUE= Thread::getMutex("VALUE");
	m_ACTIVATETIME= Thread::getMutex("ACTIVATETIME");
	m_VALUECONDITION= Thread::getCondition("VALUECONDITION");
	m_bDebug= false;
	m_nActCount= 0;
}

void MeasureThread::setDebug(bool bDebug, const string& subroutine)
{
	bool bOpen(false);
	bool isDebug(false);
	ostringstream open;
	ostringstream out;

	if(bDebug)
	{// check before output which subroutines set for debug
	 // if an new subroutine will be set
		open << " follow subroutines currently be set also for debugging:" << endl;
		for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
		{
			if(	it->bCorrect &&
				it->portClass->isDebug() == true	)
			{
				if(it->type != "DEBUG")
					isDebug= true;
				if(it->name != subroutine)
				{
					bOpen= true;
					open << "       subroutine " << it->name << endl;
				}
			}
		}
	}
	out << "-------------------------------------------------------------------" << endl;
	out << " set debug to " << boolalpha << bDebug << " from folder " << getThreadName() << endl;
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->bCorrect)
		{
			if(	subroutine == "" ||
				it->name == subroutine	)
			{
				if(it->portClass->isDebug() != bDebug)
					out << "       in subroutine " << it->name << endl;
				it->portClass->setDebug(bDebug);
				if(	bDebug &&
					it->type != "DEBUG"	)
				{
					isDebug= true;
				}
			}

		}
	}
	if(!bDebug)
	{// check which subroutines be set for debugging
	 // when one subroutine will be lose debug level
		open << " follow subroutines currently be set for debugging:" << endl;
		for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
		{
			if(	it->bCorrect &&
				it->portClass->isDebug() == true	)
			{
				if(it->type != "DEBUG")
					isDebug= true;
				if(it->name != subroutine)
				{
					bOpen= true;
					open << "       subroutine " << it->name << endl;
				}
			}
		}
	}
	if(bOpen)
		out << open.str();
	LOCK(m_DEBUGLOCK);
	if(isDebug)
	{
		m_bDebug= true;
		out << "  write begin- and end-time of folder" << endl;
	}else
	{
		if(m_bDebug)
			out << "  finish writing begin- and end-time of folder" << endl;
		m_bDebug= false;
	}
	out << "-------------------------------------------------------------------" << endl;
	tout << out.str();
	TERMINALEND;
	UNLOCK(m_DEBUGLOCK);
}

bool MeasureThread::isDebug()
{
	bool debug;

	LOCK(m_DEBUGLOCK);
	debug= m_bDebug;
	UNLOCK(m_DEBUGLOCK);

	return debug;
}

unsigned short MeasureThread::getActCount(const string& subroutine)
{
	if(subroutine == "")
		return ++m_nActCount;
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(	it->bCorrect &&
			it->portClass->getSubroutineName() == subroutine	)
		{
			return it->portClass->getActCount();
		}
	}
	return 0;
}

int MeasureThread::init(void *arg)
{
	int nMuch;
	SHAREDPTR::shared_ptr<IListObjectPattern> port;
	MeasureArgArray tArg= *((MeasureArgArray*)arg);
	vector<string>::iterator found;

	m_pvlPorts= tArg.ports;
	m_pvtSubroutines= tArg.subroutines;

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		if((*m_pvtSubroutines)[n].bCorrect)
		{
			//cout << "define subroutine " << (*m_pvtSubroutines)[n].name << endl;
			port= (*m_pvtSubroutines)[n].portClass;
			port->setDebug(false);
			port->setObserver(this);
			port->setRunningThread(this);
		}
	}
	for(vector<string>::iterator it= tArg.debugSubroutines.begin(); it != tArg.debugSubroutines.end(); ++it)
	{
		setDebug(true, *it);
	}
	return 0;
}

void MeasureThread::changedValue(const string& folder, const string& from)
{
	LOCK(m_VALUE);
	m_vFolder.push_back(from);
	AROUSE(m_VALUECONDITION);
	UNLOCK(m_VALUE);
}

int MeasureThread::stop(const bool* bWait/*=NULL*/)
{
	int nRv;

	nRv= Thread::stop(false);
	//LOCK(m_VALUE);
	AROUSE(m_VALUECONDITION);
	//UNLOCK(m_VALUE);
	if(	bWait
		&&
		*bWait	)
	{
		nRv= Thread::stop(bWait);
	}
	return nRv;
}

struct time_sort : public binary_function<timeval, timeval, bool>
{
	bool operator()(timeval x, timeval y)
	{
		if(x.tv_sec < y.tv_sec) return true;
		if(x.tv_sec == y.tv_sec) return x.tv_usec < y.tv_usec;
		return false;
	}
};

string MeasureThread::getTimevalString(const timeval& time, const bool debug)
{
	char stime[22];
	struct tm ttime;
	string sRv;

	if(localtime_r(&time.tv_sec, &ttime) != NULL)
	{
		strftime(stime, 21, "%d.%m.%Y %H:%M:%S  ", &ttime);
		sRv= stime;
	}else
	{
		if(debug)
			tout << "++ cannot create localtime_r from seconds ++" << endl;
		sRv= "xx.xx.xxxx xx:xx:xx  ";
	}
	sRv+= getUsecString(time.tv_usec);
	return sRv;
}

string MeasureThread::getUsecString(const suseconds_t usec)
{
	string::size_type nLen;
	string sRv;
	ostringstream stream;

	stream << usec;
	nLen= stream.str().length();
	for(string::size_type o= 6; o > nLen; --o)
		sRv+= "0";
	sRv+= stream.str();
	return sRv;
}

int MeasureThread::execute()
{
	bool debug(isDebug());
	string folder;
	static timeval start_tv;
	timeval end_tv, diff_tv;
	timespec waittm;
	vector<timeval>::iterator akttime, lasttime;

	//Debug info before measure routine to stop by right folder
	/*folder= getThreadName();
	if(folder == "display_settings")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << "starting folder " << folder << endl;
	}*/
	measure();
	//Debug info behind measure routine to stop by right folder
	/*folder= getThreadName();
	if(folder == "display_settings")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << "starting folder " << folder << endl;
	}*/

	if(gettimeofday(&end_tv, NULL))
	{
		string err("ERROR: cannot calculate ending time of hole folder list from '");

		err+= getThreadName();
		if(debug)
		{
			tout << "--------------------------------------------------------------------" << endl;
			tout << err << endl;
		}
		TIMELOG(LOG_ERROR, "ending_time", err);
		end_tv.tv_sec= 0;
		end_tv.tv_usec= 0;
	}
	if(debug)
	{
		char stime[18];
		struct tm ttime;

		folder= getThreadName();
		tout << "--------------------------------------------------------------------" << endl;
		tout << " folder '" << folder << "' STOP (";
		if(localtime_r(&end_tv.tv_sec, &ttime) != NULL)
		{
			strftime(stime, 16, "%Y%m%d:%H%M%S", &ttime);
			tout << stime << " ";
		}else
			tout << " cannot create localtime_r from seconds ";
		tout << getUsecString(end_tv.tv_usec) << ")";

		if(end_tv.tv_sec != 0)
			timersub(&end_tv, &start_tv, &diff_tv);
		else
			diff_tv= end_tv;
		tout << "  running time (" << diff_tv.tv_sec << ".";
		tout << getUsecString(diff_tv.tv_usec) << ")" << endl;
		tout << "--------------------------------------------------------------------" << endl;
		TERMINALEND;
	}
	LOCK(m_VALUE);
	m_vInformed.clear();
	if(m_vFolder.empty())
	{
		TERMINALEND;
		LOCK(m_ACTIVATETIME);
		if(	!m_vtmNextTime.empty() ||
			!m_osUndefServers.empty()	)
		{
			bool fold(false);
			char stime[18];

			if(!m_vtmNextTime.empty())
			{
				sort(m_vtmNextTime.begin(), m_vtmNextTime.end(), time_sort());
				akttime= m_vtmNextTime.begin();
				while(akttime != m_vtmNextTime.end())
				{// remove all older times than actual from NextTime vector
				 // and make no condition when any older found
					if(timercmp(&end_tv, &*akttime, <))
						break;
					fold= true;
					m_vFolder.push_back("#timecondition " + getTimevalString(*akttime, debug));
					m_vtmNextTime.erase(akttime);
					akttime= m_vtmNextTime.begin();
				}
				do{ // search for same times
					// and erase until one on of them
					lasttime= akttime;
					++akttime;
					if(	akttime == m_vtmNextTime.end() ||
						timercmp(&*akttime, &*lasttime, !=)	)
					{
						akttime= lasttime;
						break;
					}
					if(lasttime != m_vtmNextTime.end())
						m_vtmNextTime.erase(lasttime);
					akttime= m_vtmNextTime.begin();
				}while(akttime != m_vtmNextTime.end());
				//cout << __FILE__ << " " << __LINE__ << endl;
				//cout << "akttime:  " << end_tv.tv_sec << " " << end_tv.tv_usec << endl;
				//cout << "polltime: " << akttime->tv_sec << " " << akttime->tv_usec << endl;
			}
			if(	fold == false &&
				(	m_vtmNextTime.empty() ||
					akttime != m_vtmNextTime.end()	)	)
			{// no older times be found
			 // or folder should start again for searching external port server
				int condRv= 0;
				bool bSearchServer(false);

				if(akttime != m_vtmNextTime.end())
				{
					waittm.tv_sec= akttime->tv_sec;
					waittm.tv_nsec= akttime->tv_usec * 1000;
				}else
				{// searching for external port server
					timeval tv;

					bSearchServer= true;
					if(gettimeofday(&tv, NULL))
					{
						string msg("ALERT: cannot get time of day,\n");

						msg+= "       so cannot measure time for next start of folder ";
						msg+= getThreadName() + " to search for external port server\n";
						msg+= "       waiting only for 10 seconds !!!";
						TIMELOG(LOG_ALERT, "gettimeofday", msg);
						if(debug)
							tout << msg << endl;
						UNLOCK(m_ACTIVATETIME);
						sleep(10);
						LOCK(m_ACTIVATETIME);
					}else
					{
						waittm.tv_sec= tv.tv_sec + m_nServerSearchSeconds;
						waittm.tv_nsec= tv.tv_usec * 1000;
					}
				}
				UNLOCK(m_ACTIVATETIME);
				while(m_vFolder.empty())
				{
					condRv= TIMECONDITION(m_VALUECONDITION, m_VALUE, &waittm);
					if(condRv == ETIMEDOUT)
					{
						if(!bSearchServer)
							m_vFolder.push_back("#timecondition " + getTimevalString(*akttime, debug));
						else
							m_vFolder.push_back("#searchserver");
						break;
					}
					if(m_vFolder.empty())
					{
						if(stopping())
							break;
						cout << "WARNING: condition for folder list " << getThreadName()
										<< " get's an spurious wakeup" << endl;
					}
				}
				LOCK(m_ACTIVATETIME);
				if(	!bSearchServer &&
					condRv == ETIMEDOUT	)
				{
					m_vtmNextTime.erase(akttime);
				}
			}// else found only old times
			//  and make now an new pass of older
			UNLOCK(m_ACTIVATETIME);
		}else
		{
			UNLOCK(m_ACTIVATETIME);
			while(m_vFolder.empty())
			{
				CONDITION(m_VALUECONDITION, m_VALUE);
				if(m_vFolder.empty())
				{
					if(stopping())
						break;
					cout << "WARNING: condition for folder list " << getThreadName()
									<< " get's an spurious wakeup" << endl;
				}else
					break;
			}
		}
	}
	if(stopping())
	{
		UNLOCK(m_VALUE);
		if(debug)
			TERMINALEND;
		return 0;
	}
	m_vInformed= m_vFolder;
	m_vFolder.clear();

	if(isDebug())
	{
		string msg("### DEBUGGING for folder ");

		msg+= folder + " is aktivated!";
		TIMELOG(LOG_INFO, folder, msg);

		tout << "--------------------------------------------------------------------" << endl;
		if(gettimeofday(&start_tv, NULL))
			tout << " ERROR: cannot calculate time of beginning" << endl;
		else
		{
			char stime[18];
			struct tm ttime;

			tout << " folder '" << folder << "' START (";
			if(localtime_r(&start_tv.tv_sec, &ttime) != NULL)
			{
				strftime(stime, 16, "%Y%m%d:%H%M%S", &ttime);
				tout << stime << " ";
			}else
				tout << " cannot create localtime_r from seconds ";
			tout << getUsecString(start_tv.tv_usec) << ")" << endl;
			for(vector<string>::iterator i= m_vInformed.begin(); i != m_vInformed.end(); ++i)
			{
				if(i->substr(0, 15) == "#timecondition ")
				{
					tout << "      awaked from setting time " << i->substr(15) << endl;

				}else if(i->substr(0, 13) == "#searchserver")
				{
					tout << "      awaked to search again for external port server (owserver)" << endl;
				}else
				{
					tout << "    informed ";
					if(i->substr(0, 1) == "|")
					{
						if(i->substr(1, 1) == "|")
							tout << "from ppi-reader '" << i->substr(2) << "'" << endl;
						else
							tout << "over Internet connection account '" << i->substr(1) << "'" << endl;
					}else
						tout << "from " << *i << " because value was changed" << endl;
				}

			}
		}
		tout << "--------------------------------------------------------------------" << endl;
		TERMINALEND;
	}
	UNLOCK(m_VALUE);
	return 0;
}

bool MeasureThread::hasActivatedTime(const string& timeConditionString)
{
	bool bRv= false;

	// this method will be called only from own thread
	// so we need no LOCKING. message from 25/11/2012
	// but WARNING: it's possible to call method also from other threads
	//                      ( method is not locking save !!! )
	for(vector<string>::iterator it= m_vInformed.begin(); it != m_vInformed.end(); ++it)
	{
		if(*it == timeConditionString)
		{
			m_vInformed.erase(it);
			bRv= true;
			break;
		}
	}
	return bRv;
}

vector<string> MeasureThread::wasInformed()
{
	vector<string> vRv;

	LOCK(m_VALUE);
	vRv= m_vInformed;
	UNLOCK(m_VALUE);
	return vRv;
}

void MeasureThread::foundPortServer(const bool bfound, const string& server, const string& id)
{
	string newId(server + "!" + id);

	if(bfound)
	{// found server
		set<string>::iterator it;

		it= find(m_osUndefServers.begin(), m_osUndefServers.end(), newId);
		if(it != m_osUndefServers.end())
			m_osUndefServers.erase(it);
	}else
		m_osUndefServers.insert(newId);
}

void MeasureThread::ending()
{
}

bool MeasureThread::measure()
{
	bool debug(isDebug()), notime(false), classdebug;
	string folder(getThreadName());
	timeval tv, tv_end;

	if(debug)
	{
		if(gettimeofday(&tv, NULL))
		{
			tout << " ERROR: cannot calculate time of ending" << endl;
			notime= true;
		}
	}
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->bCorrect)
		{
			double result;

			//Debug info to stop always by right folder or subroutine
			/*string stopfolder("TRANSMIT_SONY");
			string stopsub("correct_group");
			if(	getThreadName() == stopfolder &&
				( 	stopsub == "" ||
					it->name == stopsub	)	)
			{
				cout << __FILE__ << __LINE__ << endl;
				cout << stopfolder << ":" << it->name << endl;
			}*/
			result= it->portClass->getValue("i:"+folder);
			if( debug &&
				it->portClass->isDebug())
			{
				classdebug= true;
				tout << "--------------------------------------------------------------------" << endl;
				tout << "execute '" << folder << ":" << it->name;
				tout << "' with value " << result << " and type " << it->portClass->getType() << " ";
				if(notime || gettimeofday(&tv_end, NULL))
					tout << " (cannot calculate length)" << endl;
				else
				{
					timersub(&tv_end, &tv, &tv_end);
					tout << " (" << tv_end.tv_sec << ".";
					tout << getUsecString(tv_end.tv_usec) << ")" << endl;
				}
			}else
				classdebug= false;
			result= it->portClass->measure(result);
			it->portClass->setValue(result, "i:"+folder+":"+it->name);


		}else if(debug)
			tout << "Subroutine " << it->name << " is not correct initialized" << endl;
		if(classdebug)
		{
			tout << "--------------------------------------------------------------------" << endl;
			TERMINALEND;
		}
		if(stopping())
			break;
	}
	return true;
}

SHAREDPTR::shared_ptr<IListObjectPattern> MeasureThread::getPortClass(const string name, bool &bCorrect) const
{
	SHAREDPTR::shared_ptr<IListObjectPattern> pRv;

	bCorrect= false;
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->name == name)
		{
			pRv= it->portClass;
			if(it->bCorrect)
				bCorrect= true;
			break;
		}
	}
	return pRv;
}

MeasureThread::~MeasureThread()
{
#ifdef DEBUG
	cout << "destructure of measureThread for folder "<< getThreadName() << endl;
#endif // DEBUG

	DESTROYMUTEX(m_DEBUGLOCK);
	DESTROYMUTEX(m_VALUE);
	DESTROYMUTEX(m_ACTIVATETIME);
	DESTROYCOND(m_VALUECONDITION);
}
