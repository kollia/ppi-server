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
#include <sys/resource.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <unistd.h>
#include <cmath>
#include <time.h>

#include <string>
#include <vector>
#include <iostream>
//#include <sstream>
#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../util/debug.h"
#include "../util/exception.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../ports/ExternPort.h"

#include "../database/lib/DbInterface.h"

#include "measureThread.h"

using namespace std;
using namespace ports;
using namespace ppi_database;
using namespace boost;
using namespace boost::algorithm;

// output on command line also this statistics
// to calculate middle length of folder or reach finish
// when definition __showStatitic be defined and debug set
#define __showStatistic

SHAREDPTR::shared_ptr<meash_t> meash_t::firstInstance= SHAREDPTR::shared_ptr<meash_t>();
string meash_t::clientPath= "";


/**
 * global mutex by calculating CPU time
 */
pthread_mutex_t *globalCPUMUTEX= Thread::getMutex("globalCPUMUTEX");

MeasureThread::MeasureThread(const string& threadname, const MeasureArgArray& tArg,
				const SHAREDPTR::shared_ptr<measurefolder_t> pFolderStart,
				const time_t& nServerSearch, bool bNoDbRead, short folderCPUtime) :
Thread(threadname, true),
m_oRunnThread(threadname, "parameter_run", "run", false, true)
{
	string run;
	SHAREDPTR::shared_ptr<measurefolder_t> pCurrent;

#ifdef DEBUG
	cout << "constructor of measurethread for folder " << getThreadName() << endl;
#endif // DEBUG
	m_bNeedFolderRunning= false;
	m_bFolderRunning= false;
	m_nServerSearchSeconds= nServerSearch;
	m_DEBUGLOCK= Thread::getMutex("DEBUGLOCK");
	m_VALUE= Thread::getMutex("VALUE");
	m_ACTIVATETIME= Thread::getMutex("ACTIVATETIME");
	m_FOLDERRUNMUTEX= Thread::getMutex("FOLDERRUNMUTEX");
	m_VALUECONDITION= Thread::getCondition("VALUECONDITION");
	m_bDebug= false;
	m_nActCount= 0;
	m_tRunThread= Thread::getThreadID();
	m_pvlPorts= tArg.ports;
	m_pvtSubroutines= tArg.subroutines;
	m_vStartDebugSubs= tArg.debugSubroutines;
	m_bNeedLength= false;
	m_bNoDbReading= bNoDbRead;
	m_nFolderCPUtime= folderCPUtime;
	pCurrent= pFolderStart;
	while(pCurrent)
	{
		if(pCurrent->name == threadname)
		{
			run= pCurrent->folderProperties->getValue("run");
			if(run != "")
			{
				size_t pos;

				pos= run.find(":");
				if(pos != string::npos)
				{// define calculation whether folder is running
					m_oRunnThread.init(pFolderStart, run.substr(pos+1));
					run= run.substr(0, pos);
				}
				split(m_vsFolderSecs, run, is_any_of(" "));
				for(vector<string>::iterator it= m_vsFolderSecs.begin(); it != m_vsFolderSecs.end(); ++it)
					trim(*it);
			}
			break;
		}
		pCurrent= pCurrent->next;
	}
}

bool MeasureThread::setDebug(bool bDebug, const string& subroutine)
{
	bool bFound(false);
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
		if(	it->bCorrect &&			// set first all subroutines to debug
			it->type != "DEBUG"	)	// which are not from type DEBUG
		{							// because DEBUG subroutines will be set later and should know
			if(	subroutine == "" ||	// whether also an other subroutine was set to debug
				it->name == subroutine	)
			{
				bFound= true;
				if(it->portClass->isDebug() != bDebug)
					out << "       in subroutine " << it->name << endl;
				it->portClass->setDebug(bDebug);
				if(bDebug)
					isDebug= true;
			}
		}
	}
	if(!bFound)
		return false;
	LOCK(m_DEBUGLOCK);
	if(isDebug)
		m_bDebug= true;
	else
		m_bDebug= false;
	UNLOCK(m_DEBUGLOCK);
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{// set now subroutine(s) from type DEBUG
		if(	it->bCorrect &&
			it->type == "DEBUG"	)
		{
			if(	subroutine == "" ||
				it->name == subroutine	)
			{
				if(it->portClass->isDebug() != bDebug)
					out << "       in subroutine " << it->name << endl;
				it->portClass->setDebug(bDebug);
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
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{// set subroutine(s) from type DEBUG again to true
	 // because maybe debugging of other subroutine type
	 // has now changed
		if(	it->bCorrect &&
			it->type == "DEBUG" &&
			it->portClass->isDebug()	)
		{
				it->portClass->setDebug(true);
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
	return true;
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
	bool *pbSubroutines(static_cast<bool*>(arg));
	int nMuch;
	double dLength;
	SHAREDPTR::shared_ptr<IListObjectPattern> port;
	vector<string>::iterator found;
	string sFault, folder(getThreadName());
	DbInterface *db= DbInterface::instance();

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		if((*m_pvtSubroutines)[n].bCorrect)
		{
			if(	pbSubroutines &&
				*pbSubroutines == true	)
			{
				cout << "      subroutine " << (*m_pvtSubroutines)[n].name << endl;
			}
			port= (*m_pvtSubroutines)[n].portClass;
			// set now running folder shortly before subroutine
			// will be initialed (it's before folder starting)
			//port->setRunningThread(this);
			port->setDebug(false);
			if(port->needObserver())
				port->setObserver(this);
		}
	}
	for(vector<string>::iterator it= m_vStartDebugSubs.begin(); it != m_vStartDebugSubs.end(); ++it)
	{
		if(!setDebug(true, *it))
			sFault+= "                             " + *it + "\n";
	}
	if(sFault != "")
	{
		cout << "### WARNING: cannot find follow subroutine(s) inside folder " << getThreadName() << ":" << endl;
		cout << sFault;
	}
	if(	pbSubroutines &&
		*pbSubroutines == true	)
	{
		cout << endl;
	}
	if(m_bNeedLength)
	{
		m_tLengthType.runlength= true;
		m_tLengthType.folder= "folder";
		m_tLengthType.subroutine= getThreadName();
		m_tLengthType.maxVal= 20;
		m_tLengthType.inPercent= m_nFolderCPUtime;
		for(short n= m_nFolderCPUtime; n <= 100; n+= m_nFolderCPUtime)
		{
			bool exist;
			ostringstream dbstr, maxcount;

			dbstr << "runlength";
			maxcount << "maxcount";
			if(m_nFolderCPUtime < 100)
			{
				dbstr << n;
				maxcount << n;
			}
			db->writeIntoDb("folder", folder, dbstr.str());
			db->writeIntoDb("folder", folder, maxcount.str());
			if(!m_bNoDbReading)
			{
				dLength= db->getActEntry(exist, "folder", folder, dbstr.str());
				if(	exist &&
					dLength > 0	)
				{
					m_tLengthType.percentSyncDiff["none"][n].actValue= dLength;
					m_tLengthType.percentSyncDiff["none"][n].reachedPercent[0]= pair<short, double>(1, dLength);
					dLength= db->getActEntry(exist, "folder", folder, maxcount.str());
					if(!exist)
						dLength= 1;
					m_tLengthType.percentSyncDiff["none"][n].maxCount= static_cast<short>(dLength);
					m_tLengthType.percentSyncDiff["none"][n].stype= dbstr.str();
					m_tLengthType.percentSyncDiff["none"][n].scount= maxcount.str();
				}
			}else
			{
				db->fillValue("folder", folder, dbstr.str(), 0, /*new*/true);
				db->fillValue("folder", folder, maxcount.str(), 0, /*new*/true);
			}
		}
	}
	timerclear(&m_tvStartTime);
	timerclear(&m_tvSleepLength);
	return 0;
}

void MeasureThread::changedValue(const string& folder, const string& from)
{
	timeval tv;
	map<string, timeval>::iterator found;

	LOCK(m_VALUE);
	m_vFolder.push_back(from);
	found= m_tChangedTimes.subroutines.find(from);
	if(found != m_tChangedTimes.subroutines.end())
	{
		if(gettimeofday(&tv, NULL))
		{
			string msg("ERROR: cannot get time of day,\n");

			msg+= "       so cannot measure time for TIMER function in folder ";
			msg+= getThreadName() + " for method changedValue()";
			TIMELOG(LOG_ALERT, "changedValue", msg);
		}else
		{
			found->second= tv;
		}
	}
	AROUSE(m_VALUECONDITION);
	UNLOCK(m_VALUE);
}

void MeasureThread::needChangingTime(const string& subroutine, const string& from)
{
	string fsub;
	timeval nulltime;
	vector<string> spl;
	vector<string>::iterator sfound;
	vector<timeval*>::iterator tmfound;

	split(spl, from, is_any_of(":"));
	if(spl.size() == 1)
	{
		fsub= spl[0];
	}
	if(	spl.size() == 2 &&
		spl[0] == getThreadName()	)
	{
		fsub= spl[1];
	}
	if(fsub != "")
	{
		sfound= find(m_tChangedTimes.ownSubs.begin(), m_tChangedTimes.ownSubs.end(), fsub);
		if(sfound == m_tChangedTimes.ownSubs.end())
			m_tChangedTimes.ownSubs.push_back(fsub);
	}else
		fsub= from;
	timerclear(&nulltime);
	m_tChangedTimes.subroutines[fsub]= nulltime;
	m_tChangedTimes.subSubs[subroutine].push_back(pair<string, timeval*>(from, &m_tChangedTimes.subroutines[fsub]));
}

timeval MeasureThread::getMaxChangingTime(const string& subroutine, const string& desc)
{
	bool debug(false);
	string from;
	timeval tmRv;
	vector<pair<string, timeval*> >* times;

	if(desc != "")
		debug= true;
	timerclear(&tmRv);
	LOCK(m_VALUE);
	times= &m_tChangedTimes.subSubs[subroutine];
	for(vector<pair<string, timeval*> >::iterator it= times->begin(); it != times->end(); ++it)
	{
		if(timercmp(it->second, &tmRv, >))
		{
			tmRv= *it->second;
			if(debug)
				from= it->first;
		}
	}
	UNLOCK(m_VALUE);
	if(debug)
	{
		tout << "     take last changing time from subroutine '" << from << "' ";
		tout << "from parameter " << desc << endl;
	}
	return tmRv;
}

bool MeasureThread::usleep(timeval time)
{
	useconds_t usWait;

	timeradd(&m_tvSleepLength, &time, &m_tvSleepLength);
	usWait= time.tv_sec * 1000000;
	usWait+= time.tv_usec;
	::usleep(usWait);
	return true;
}

int MeasureThread::stop(const bool* bWait/*=NULL*/)
{
	int nRv;

	nRv= Thread::stop(false);
	//LOCK(m_VALUE);
	AROUSE(m_VALUECONDITION);
	//UNLOCK(m_VALUE);
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->bCorrect)
		{
			try{
				it->portClass->stop(bWait);

			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("by stopping subroutine " + it->name + " inside folder " + getThreadName());
				err= ex.getTraceString();
				cerr << err << endl;
				LOG(LOG_ERROR, err);

			}catch(std::exception& ex)
			{
				string err("std exception by stopping subroutine " + it->name + " inside folder " + getThreadName());

				err+= "\n" + string(ex.what());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);

			}catch(...)
			{
				string err("undefined exception by stopping subroutine " + it->name + " inside folder " + getThreadName());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);
			}
		}
	}
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

string MeasureThread::getTimevalString(const timeval& tvtime, const bool& bDate, const bool& bDebug)
{
	char stime[21];
	struct tm ttime;
	string::size_type nLen;
	ostringstream stream, sRv;

	if(bDate)
	{
		if(localtime_r(&tvtime.tv_sec, &ttime) != NULL)
		{
			strftime(stime, 20, "%d.%m.%Y %H:%M:%S", &ttime);
			sRv << stime << " ";
		}else
		{
			if(bDebug)
				tout << "++ cannot create localtime_r from seconds ++" << endl;
			sRv << "xx.xx.xxxx xx:xx:xx  ";
		}
	}else
		sRv << tvtime.tv_sec << ".";

	stream << tvtime.tv_usec;
	nLen= stream.str().length();
	for(string::size_type o= 6; o > nLen; --o)
		sRv << "0";
	sRv << stream.str();
	return sRv.str();
}

folderSpecNeed_t MeasureThread::isFolderRunning(const vector<string>& specs)
{
	typedef vector<string>::const_iterator specIt;
	folderSpecNeed_t tRv;
	specIt found;
	double nRun;

	if(specs.size() > 0)
	{
		tRv.needRun= false;
		tRv.fromCalc= false;
		tRv.isRun= false;
		for(specIt it= m_vsFolderSecs.begin(); it != m_vsFolderSecs.end(); ++it)
		{
			found= find(specs.begin(), specs.end(), *it);
			if(found != specs.end())
			{
				tRv.needRun= true;
				break;
			}
		}
		if(!tRv.needRun)
			return tRv;
	}else
		tRv.needRun= true;
	if(!m_oRunnThread.isEmpty())
	{
		m_oRunnThread.calculate(nRun);
		tRv.fromCalc= true;
		if(	nRun < 0 ||
			nRun > 0	)
		{
			tRv.isRun= true;
		}else
			tRv.isRun= false;
		return tRv;
	}
	if(!m_bNeedFolderRunning)// first initialization of any TIMER subroutine and stay always on same value
		m_bNeedFolderRunning= true; // so variable need no mutex lock to be atomic
	tRv.fromCalc= false;
	LOCK(m_FOLDERRUNMUTEX);
	tRv.isRun= m_bFolderRunning;
	UNLOCK(m_FOLDERRUNMUTEX);
	return tRv;
}

int MeasureThread::execute()
{
	bool debug(isDebug());
	string folder;
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
	if(!timerisset(&m_tvStartTime))
	{
		if(gettimeofday(&m_tvStartTime, NULL))
		{
			tout << " ERROR: cannot calculate time of beginning" << endl;
			timerclear(&m_tvStartTime);
		}
	}
	measure();
	//Debug info behind measure routine to stop by right folder
	/*folder= getThreadName();
	if(folder == "Raff1_Zeit")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << "starting folder " << folder << endl;
	}*/

	if(gettimeofday(&end_tv, NULL))
	{
		string err("ERROR: cannot calculate ending time of hole folder list from '");

		err+= getThreadName() + "'";
		if(debug)
		{
			tout << "--------------------------------------------------------------------" << endl;
			tout << err << endl;
		}
		TIMELOG(LOG_ERROR, "ending_time_" + folder, err);
		end_tv.tv_sec= 0;
		end_tv.tv_usec= 0;
	}
	timerclear(&diff_tv);
	if(	m_bNeedLength &&
		timerisset(&end_tv) &&
		timerisset(&m_tvStartTime)	)
	{
		timersub(&end_tv, &m_tvStartTime, &diff_tv);
		if(timerisset(&m_tvSleepLength))
		{
			timersub(&diff_tv, &m_tvSleepLength, &diff_tv);
			timerclear(&m_tvSleepLength);
		}
		LOCK(m_DEBUGLOCK);
		calcLengthDiff(&m_tLengthType, diff_tv, debug);
		UNLOCK(m_DEBUGLOCK);
	}


	if(debug)
	{
		if(!timerisset(&diff_tv))
		{
			timersub(&end_tv, &m_tvStartTime, &diff_tv);
			if(timerisset(&m_tvSleepLength))
			{
				timersub(&diff_tv, &m_tvSleepLength, &diff_tv);
				timerclear(&m_tvSleepLength);
			}
		}
		folder= getThreadName();
		tout << "--------------------------------------------------------------------" << endl;
		tout << " folder '" << folder << "' STOP (";
		tout << getTimevalString(end_tv, /*as date*/true, debug) << ")" ;
		tout << "  running time (" << getTimevalString(diff_tv, /*as date*/false, debug) << ")" << endl;
		tout << "--------------------------------------------------------------------" << endl;
		TERMINALEND;
	}
	LOCK(m_VALUE);
	m_vInformed.clear();
	if(1) //m_vFolder.empty())
	{
		TERMINALEND;
		LOCK(m_ACTIVATETIME);
		if(	!m_vtmNextTime.empty() ||
			!m_osUndefServers.empty()	)
		{
			bool fold(false);

			if(!m_vtmNextTime.empty())
			{
				sort(m_vtmNextTime.begin(), m_vtmNextTime.end(), time_sort());
			//	cout << "exits old next starting times:" << endl;
			//	for(akttime= m_vtmNextTime.begin(); akttime != m_vtmNextTime.end(); ++akttime)
			//		cout << "    " << akttime->tv_sec << "." << MeasureThread::getUsecString(akttime->tv_usec) << endl;
				akttime= m_vtmNextTime.begin();
				while(akttime != m_vtmNextTime.end())
				{// remove all older times than actual from NextTime vector
				 // and make no condition when any older found
					if(timercmp(&end_tv, &*akttime, <))
						break;
					fold= true;
					m_vFolder.push_back("#timecondition " + getTimevalString(*akttime, /*as date*/true, debug));
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
					if(m_bNeedFolderRunning)
					{
						LOCK(m_FOLDERRUNMUTEX);
						m_bFolderRunning= false;
						UNLOCK(m_FOLDERRUNMUTEX);
					}
					condRv= TIMECONDITION(m_VALUECONDITION, m_VALUE, &waittm);
					if(m_bNeedFolderRunning)
					{
						LOCK(m_FOLDERRUNMUTEX);
						m_bFolderRunning= true;
						UNLOCK(m_FOLDERRUNMUTEX);
					}
					if(condRv == ETIMEDOUT)
					{
						if(!bSearchServer)
							m_vFolder.push_back("#timecondition " + getTimevalString(*akttime, /*as date*/true, debug));
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
			//	cout << "exits next starting times:" << endl;
			//	for(akttime= m_vtmNextTime.begin(); akttime != m_vtmNextTime.end(); ++akttime)
			//		cout << "    " << akttime->tv_sec << "." << MeasureThread::getUsecString(akttime->tv_usec) << endl;
			}// else found only old times
			//  and make now an new pass of older
			UNLOCK(m_ACTIVATETIME);
		}else
		{
			UNLOCK(m_ACTIVATETIME);
			while(m_vFolder.empty())
			{
				if(m_bNeedFolderRunning)
				{
					LOCK(m_FOLDERRUNMUTEX);
					m_bFolderRunning= false;
					UNLOCK(m_FOLDERRUNMUTEX);
				}
				CONDITION(m_VALUECONDITION, m_VALUE);
				if(m_bNeedFolderRunning)
				{
					LOCK(m_FOLDERRUNMUTEX);
					m_bFolderRunning= true;
					UNLOCK(m_FOLDERRUNMUTEX);
				}
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
	if(gettimeofday(&m_tvStartTime, NULL))
	{
		string msg("### DEBUGGING for folder ");

		folder= getFolderName();
		msg+= folder + " is aktivated!\n";
		msg+= "    ERROR: cannot calculate time of beginning";
		TIMELOG(LOG_ERROR, folder, msg);
		if(isDebug())
			tout << " ERROR: cannot calculate time of beginning" << endl;

	}else if(isDebug())
	{
		string msg("### DEBUGGING for folder ");

		msg+= folder + " is aktivated!";
		TIMELOG(LOG_INFO, folder, msg);

		tout << "--------------------------------------------------------------------" << endl;
		tout << " folder '" << folder << "' START (";
		tout << getTimevalString(m_tvStartTime, /*as date*/true, /*debug*/true) << ")" << endl;
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
		tout << "--------------------------------------------------------------------" << endl;
		TERMINALEND;
	}
	UNLOCK(m_VALUE);
	return 0;
}

void MeasureThread::changeActivationTime(const string& folder, const timeval& time,
				const timeval& newtime)
{
	LOCK(m_ACTIVATETIME);
	for(vector<timeval>::iterator it= m_vtmNextTime.begin(); it != m_vtmNextTime.end(); ++it)
	{
		if(!timercmp(it, &time, !=)) // == do not work in some systems
		{
			*it= newtime;
			break;
		}
	}
	UNLOCK(m_ACTIVATETIME);
}

void MeasureThread::eraseActivateTime(const string& folder, const timeval& time)
{
	LOCK(m_ACTIVATETIME);
	for(vector<timeval>::iterator it= m_vtmNextTime.begin(); it != m_vtmNextTime.end(); ++it)
	{
		if(!timercmp(it, &time, !=)) // == do not work in some systems
		{
			m_vtmNextTime.erase(it);
			break;
		}
	}
	UNLOCK(m_ACTIVATETIME);
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
	bool debug(isDebug()), notime(false), classdebug(false);
	string folder(getThreadName());
	timeval tv, tv_start, tv_end;

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
		classdebug= false;
		if(it->bCorrect)
		{
			double result, oldResult;

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
			oldResult= it->portClass->getValue("i:"+folder);
			if( debug &&
				it->portClass->isDebug())
			{
				classdebug= true;
				tout << "--------------------------------------------------------------------" << endl;
				tout << "execute '" << folder << ":" << it->name;
				tout << "' with value " << oldResult << " and type " << it->portClass->getType() << " ";
				if(notime || gettimeofday(&tv_start, NULL))
				{
					tout << " (cannot calculate length)" << endl;
					notime= true;
				}else
				{
					timersub(&tv_start, &tv, &tv_end);
					tout << " (" << getTimevalString(tv_end, /*as date*/false, /*debug*/true) << ")" << endl;
				}
			}
			try{
				result= it->portClass->measure(oldResult);

			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("running subroutine " + it->name + " inside folder " + getThreadName());
				err= ex.getTraceString();
				cerr << err << endl;
				LOG(LOG_ERROR, err);

			}catch(std::exception& ex)
			{
				string err("std exception by running subroutine " + it->name + " inside folder " + getThreadName());

				err+= "\n" + string(ex.what());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);

			}catch(...)
			{
				string err("undefined exception by running subroutine " + it->name + " inside folder " + getThreadName());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);
			}
			timerclear(&tv_end);
			if(result != oldResult)
			{
				vector<string>::iterator found;

				it->portClass->setValue(result, "i:"+folder+":"+it->name);
				found= find(m_tChangedTimes.ownSubs.begin(), m_tChangedTimes.ownSubs.end(), it->portClass->getSubroutineName());
				if(found != m_tChangedTimes.ownSubs.end())
				{
					if(gettimeofday(&tv_end, NULL))
					{
						string msg("ERROR: cannot get time of day,\n");

						msg+= "       so cannot measure time for TIMER function in folder ";
						msg+= getThreadName() + " for changing value in subroutine " + *found;
						TIMELOG(LOG_ALERT, "changedValueMeasureThread", msg);
						timerclear(&tv_end);
					}
					LOCK(m_VALUE);
					if(	getThreadName() == "Raff1_Zeit" &&
						*found == "port2_status")
					{
						tout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
						tout << __FILE__ << __LINE__ << endl;
						tout << "last changed time from Raff1_Zeit:port2_status is " << getTimevalString(tv_end, true, debug) << endl;
						tout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
					}
					m_tChangedTimes.subroutines[*found]= tv_end;
					UNLOCK(m_VALUE);
				}
			}
			if(classdebug)
			{
				if(!timerisset(&tv_end))
				{
					if(notime || gettimeofday(&tv_end, NULL))
					{
						tout << " (cannot calculate length running subroutine)" << endl;
						timerclear(&tv_end);
					}
				}
				if(timerisset(&tv_end))
				{
					timeval length;

					timersub(&tv_end, &tv_start, &length);
					tout << " subroutine running ";
					tout << getTimevalString(length, /*as date*/false, /*debug*/true);
					tout << " seconds, ending by ";
					tout << getTimevalString(tv_end, /*as date*/true, /*debug*/true);
					tout << endl;
				}
			}


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

void MeasureThread::setCpuMeasureBegin(timetype_t *timetype)
{
	timetype->prev_idle= m_tLengthType.prev_idle;
	timetype->prev_total= m_tLengthType.prev_total;
}

int MeasureThread::getCpuPercent(const vector<int>::size_type& processor, int *prev_idle,
										int *prev_total, int old_usage, const bool& debug)
{
	// implement bash script from
	// Paul Colby (http://colby.id.au), no rights reserved ;)
	// getting from page (http://colby.id.au/node/39)
	int value, idle, total(0);
	int diff_idle, diff_total, diff_usage;
	string cpu, filename("/proc/stat");
	ifstream file;
	string line;
	vector<int>::size_type actProcessor(0);

	file.open(filename.c_str());
	if(file.is_open())
	{
		while(getline(file, line))
		{
			istringstream oline(line);

			oline >> cpu;
			if(	cpu.size() < 3 ||
				cpu.substr(0, 3) != "cpu"	)
			{
				ostringstream err1, err2;

				err1 << "on computer exist only " << (actProcessor -1) << " processor" << endl;
				err2 << "asking with getCpuPercent for processor " << processor << endl;
				if(debug)
				{
					tout << "### ERROR: " << err1.str();
					tout << "           " << err2.str();
				}
				TIMELOG(LOG_ERROR, "processor_count", err1.str() + err2.str());
				return 0;
			}
			if(processor == actProcessor)
			{
				for(short i= 0; i < 10; ++i)
				{
					oline >> value;
					total+= value;
					if(i == 3)
						idle= value;
				}
				break;
			}
			++actProcessor;
		}
	}else
	{
		string err("cannot read " + filename + " to create CPU time, take old one");

		if(debug)
			tout << err << endl;
		TIMELOG(LOG_ERROR, "cpu_creation", err);
		return old_usage;
	}
	file.close();
	//LOCK(globalCPUMUTEX);
	diff_idle= idle - *prev_idle;
	diff_total= total - *prev_total;
	if(diff_total == 0) // method was called to quickly after each other
	{
		//UNLOCK(globalCPUMUTEX);
		return old_usage;
	}
	diff_usage= (1000 * (diff_total - diff_idle) / diff_total + 5) / 10;
	if(diff_usage <= 0)
	{
		vector<ostringstream> err;

		err[0] << "cannot create correct CPU time:" << endl;
		err[1] << "   diff_idle(" << diff_idle << ") = idle(" << idle << ")"
						" - prev_idle(" << prev_idle << ")" << endl;
		err[2] << "  diff_total(" << diff_total << ") = total(" << total << ")"
						" - prev_total(" << prev_total << ")" << endl;
		err[3] << "  diff_usage(" << diff_usage << ") = (1000 * ("
						"diff_total(" << diff_total << ")"
						" - diff_idle(" << diff_idle << ")) / "
						"diff_total(" << diff_total << ") + 5) / 10" << endl;
		err[4] << "so return back old usage " << old_usage << "%";
		if(debug)
		{
			tout << "     " << err[0].str();
			tout << "     " << err[1].str();
			tout << "     " << err[2].str();
			tout << "     " << err[3].str();
			tout << "     " << err[4].str() << endl;;
		}
		TIMELOG(LOG_WARNING, "cputimecreation",
						err[0].str() + err[1].str() +
						err[2].str() + err[3].str() + err[4].str());
		return old_usage;
	}
	*prev_total= total;
	*prev_idle= idle;
	old_usage= diff_usage;
	//UNLOCK(globalCPUMUTEX);
	//cout << cpu << " " << diff_usage << "%" << endl;
	return diff_usage;
}

timeval MeasureThread::calcResult(double seconds, const bool& secondcalc)
{
	long val;
	timeval tmRv;

	val= static_cast<long>(seconds);
	tmRv.tv_sec= static_cast<__time_t>(val);
	if(!secondcalc)
	{
		seconds-= static_cast<double>(val);
		seconds*= (1000 * 1000);
		tmRv.tv_usec= static_cast<__suseconds_t>(seconds);
	}else
		tmRv.tv_usec= 0;
	return tmRv;
}

double MeasureThread::calcResult(const timeval& tv, const bool& secondcalc)
{
	long val;
	double dRv;

	if(!secondcalc)
	{
		dRv= static_cast<double>(tv.tv_usec) / (1000 * 1000);
		dRv+= static_cast<double>(tv.tv_sec);
	}else
		dRv= static_cast<double>(tv.tv_sec);
	// secure method to calculate only with seconds
	// or microseconds and no more decimal places behind
	if(!secondcalc)
		dRv*= (1000 * 1000);
	val= static_cast<long>(dRv);
	dRv= static_cast<double>(val);
	if(!secondcalc)
		dRv/= (1000 * 1000);
	return dRv;
}

timeval MeasureThread::getLengthedTime(const bool& logPercent, const bool& debug)
{
	short percent;
	timeval tvRv;

	LOCK(m_DEBUGLOCK);
	tvRv= getLengthedTime(&m_tLengthType, &percent, logPercent, debug);
	UNLOCK(m_DEBUGLOCK);
	return tvRv;
}

timeval MeasureThread::getLengthedTime(timetype_t* timelength, short *percent,
										const bool& logPercent, const bool& debug)
{
	short nPercent(100);
	int prev_idle, prev_total;
	double dRv;
	map<short, timeLen_t>* percentDiff;
	map<short, timeLen_t>* nearestPercentDiff(NULL);
	map<short, timeLen_t>::const_iterator it, last;

	percentDiff= getPercentDiff(timelength, nearestPercentDiff, debug);
	if(	percentDiff->size() == 0 &&
		nearestPercentDiff != NULL	)
	{
		percentDiff= nearestPercentDiff;
	}
	last= percentDiff->end();
	if(	timelength->inPercent < 100
		|| debug
		|| logPercent 				)
	{
		prev_idle= timelength->prev_idle;
		prev_total= timelength->prev_total;
		nPercent= static_cast<short>(getCpuPercent(0, &prev_idle, &prev_total, timelength->old_usage, debug));
	}

#ifdef __showStatistic
	if(debug)
		tout << "      percentDiff has " << percentDiff->size()
					<< " different percent calculations" << endl;
#endif // __showStatistic

	for(it= percentDiff->begin(); it != percentDiff->end(); ++it)
	{

#ifdef __showStatistic
		if(debug)
		{
			tout << "      found " << it->first << "% with act " << it->second.actValue << " read " <<
											it->second.readValue;
			if(	!timelength->runlength &&
				timelength->synchroID == ""	)
			{
				tout << " for running folders " << timelength->synchroID;
			}
			tout << endl;
		}
#endif // __showStatistic

		if(it->first >= nPercent)
		{
			*percent= it->first;
			if(it->second.actValue != 0)
				dRv= it->second.actValue;
			else
				dRv= it->second.readValue;

#ifdef __showStatistic
			if(debug)
			{
				tout << "     return time " << dRv;
				tout << " from " << it->first << "% for " << nPercent << "%";
				tout << endl;
			}
#endif // __showStatistic
			break;
		}
		last= it;
	}
	if(it == percentDiff->end())
	{
		if(!percentDiff->empty())
		{
			*percent= last->first;
			if(it->second.actValue != 0)
				dRv= last->second.actValue;
			else
				dRv= last->second.readValue;

#ifdef __showStatistic
			if(debug)
			{
				tout << "     return last time " << dRv;
				if(timelength->runlength)
					tout << " as 10% more";
				tout << " from " << last->first << "% for " << nPercent << "%";
				tout << endl;
			}
#endif // __showStatistic

		}else
		{
			*percent= nPercent;
			dRv= 0;
#ifdef __showStatistic
			if(debug)
				tout << "     return 0 time for " << nPercent
							<< "% because no saved value exists" << endl;
#endif // __showStatistic
		}
	}
	if(	logPercent &&
		nPercent >= 0	)
	{
		string dbentry;
		DbInterface *db= DbInterface::instance();

		if(timelength->runlength)
			dbentry= "runpercent";
		else
			dbentry= "reachpercent";
		db->fillValue(timelength->folder, timelength->subroutine, dbentry,
						static_cast<double>(nPercent), /*new*/true);
	}
	if(	timelength->inPercent > 1 &&
		nPercent != *percent &&
		(	(nPercent + timelength->inPercent - 1) < *percent ||
			nPercent > *percent									)	)
	{// returning correct percent for writing, because reachend time
	 // calculate for this value new length
		if(	timelength->inPercent >= 10 &&
			nPercent >= 10					)
		{
			*percent= static_cast<short>(ceil(static_cast<double>(nPercent) / 10) * 10);
		}else
			*percent= nPercent;
		if(*percent % timelength->inPercent)
			*percent-= (*percent % timelength->inPercent) +  timelength->inPercent;
		if(*percent == 0)
			*percent= timelength->inPercent;

#ifdef __showStatistic
		if(debug)
			tout << "     convert returning percent to " << *percent << "% for next calculation" << endl;
#endif // __showStatistic

	}else
		*percent= nPercent;
	return calcResult(dRv, /*seconds*/false);;
}

map<short, IMeasurePattern::timeLen_t>* MeasureThread::getPercentDiff(timetype_t *timelength, map<short, timeLen_t>* nearest, const bool&debug)
{
	typedef map<string, map<short, timeLen_t> >::iterator percentSyncIt;

	map<short, timeLen_t>* percentDiff(NULL);

	nearest= NULL;
	if(	timelength->runlength ||
		timelength->synchroID == ""	)
	{
		percentDiff= &timelength->percentSyncDiff["none"];
	}else
	{

		if(timelength->percentSyncDiff.find(timelength->synchroID) ==
						timelength->percentSyncDiff.end()				)
		{ // by new map of synchronization ID creation
		  // search also for the nearest to take this as default value
			size_t calcBit, ownBit, beforeBit, lastBit;
			map<short, timeLen_t> *before, *last;
			string id;
			id= timelength->synchroID;
			for(string::reverse_iterator sit= id.rbegin(); sit != id.rend(); ++sit)
			{
				if(*sit == '1')
					++ownBit;
			}
			beforeBit= 0;
			lastBit= 0;
			for(percentSyncIt sIt= timelength->percentSyncDiff.begin();
							sIt != timelength->percentSyncDiff.end(); ++sIt)
			{
				id= sIt->first;
				for(string::reverse_iterator sit= id.rbegin(); sit != id.rend(); ++sit)
				{
					if(*sit == '1')
						++calcBit;
				}
				if(	calcBit > lastBit &&
					sIt->second.size() > 0	)
				{
					before= &sIt->second;
					beforeBit= calcBit;
					if(beforeBit > ownBit)
						break;
					last= before;
					lastBit= beforeBit;
					beforeBit= 0;
				}
			}
			if(	beforeBit > 0 &&
				lastBit > 0 	)
			{
				if(beforeBit - ownBit < ownBit -lastBit)
					nearest= before;
				else
					nearest= last;
			}else if(beforeBit > 0)
				nearest= before;
			else if(lastBit > 0)
				nearest= last;

		}
		percentDiff= &timelength->percentSyncDiff[timelength->synchroID];
	}
	return percentDiff;
}

void MeasureThread::calcLengthDiff(timetype_t *timelength,
				timeval length, const bool& debug)
{
	bool bSave(false);
	short nPercent;
	int prev_idle, prev_total;
	double value;
	DbInterface *db;
	timeLen_t* timevec;
	map<short, timeLen_t>* percentDiff;
	map<short, timeLen_t>* nearestPercentDiff(NULL);
	map<short, timeLen_t>::iterator it;

	percentDiff= getPercentDiff(timelength, nearestPercentDiff, debug);
	prev_idle= timelength->prev_idle;
	prev_total= timelength->prev_total;
	if(timelength->inPercent < 100)
	{// differ between CPU percent
		nPercent= static_cast<short>(getCpuPercent(0, &prev_idle, &prev_total, timelength->old_usage, debug));
		timelength->old_usage= nPercent;
		timevec= NULL;
		if(	timelength->inPercent >= 10 &&
			nPercent >= 10					)
		{
			nPercent= static_cast<short>(ceil(static_cast<double>(nPercent) / 10) * 10);
		}
		if(nPercent == 0)
			nPercent= timelength->inPercent;
		if(nPercent % timelength->inPercent)
			nPercent-= (nPercent % timelength->inPercent) +  timelength->inPercent;
		it= percentDiff->find(nPercent);
		if(it == percentDiff->end())
		{// create new entry
			ostringstream dbstr, maxcount;

#ifdef __showStatistic
			if(debug)
			{
				tout << "     create new object for " << nPercent << "%";
				if(	!timelength->runlength &&
					timelength->synchroID != ""	)
				{
					tout << " by running folders " << timelength->synchroID;
				}
				tout << endl;
			}
#endif // __showStatistic

			if(timelength->runlength)
				dbstr << "runlength";
			else
				dbstr << "reachend";
			maxcount << "maxcount";
			if(	!timelength->runlength &&
				timelength->synchroID != ""	)
			{
				dbstr << timelength->synchroID << "-";
				maxcount << timelength->synchroID << "-";
			}
			dbstr << nPercent;
			maxcount << nPercent;
			(*percentDiff)[nPercent].stype= dbstr.str();
			(*percentDiff)[nPercent].scount= maxcount.str();
			(*percentDiff)[nPercent].maxCount= 0;
			timevec= &(*percentDiff)[nPercent];
		}else
			timevec= &it->second;
	}else
	{// timelength->inPercent == 100
		if(percentDiff->empty())
		{
			if(timelength->runlength)
				(*percentDiff)[100].stype= "runlength";
			else
				(*percentDiff)[100].stype= "reachend";
			(*percentDiff)[100].scount= "maxcount";
			if(	!timelength->runlength &&
				timelength->synchroID != ""	)
			{
				(*percentDiff)[100].stype+= timelength->synchroID;
				(*percentDiff)[100].scount+= timelength->synchroID;
			}
			(*percentDiff)[100].maxCount= 0;
		}
		timevec= &(*percentDiff)[100];
	}
	if(timelength->runlength)
	{// measure begin of CPU time always only from last ending folder
		if(timelength->inPercent == 100)// when inPercent is 100 no CPU time was created
			getCpuPercent(0, &prev_idle, &prev_total, timelength->old_usage, debug);
		timelength->prev_idle= prev_idle; // also for reach end sessions
		timelength->prev_total= prev_total;
	}

	value= calcResult(length, /*seconds*/false);
	if(timevec->maxCount == 0)
	{
		++timevec->read;
		timevec->maxCount= 1;
		timevec->actValue= value;
		timevec->readValue= value;
		timevec->reachedPercent[0]= pair<short, double>(1, value);
		bSave= true;
	}else
	{
		++timevec->read;
		if(timelength->runlength)
		{// value is for folder length
			if(value > timevec->readValue)
			{
				timevec->readValue= value;
				timevec->maxRadCount= timevec->read;
			}
		}else
		{// value is for middle value to reach finish of result
			timevec->readValue= value;
		}
#ifdef __showStatistic
		if(	debug &&
			timelength->runlength &&
			timevec->read < timelength->maxVal	)
		{
			tout << "  >> calculate middle length for " << timevec->stype << " "
							<< timevec->read << ". value";
			if(	!timelength->runlength &&
				timelength->synchroID != ""	)
			{
				tout << " by running folders " << timelength->synchroID;
			}
			tout << endl;
			tout << "     read " << value << " actual ";
			if(timelength->runlength)
				tout << "max ";
			else
				tout << "middle ";
			tout << "value is " << timevec->readValue << " by "
							<< timevec->maxRadCount << " count" << endl;
			tout << "     calculated actual middle length of ";
			if(timelength->runlength)
				tout << "folder running ";
			else
				tout << "reaching result ";
			tout << "is " << timevec->actValue << endl;
		}
#endif // __showStatistic

	// nPercent is now percent inside reachedPercent map
	// ----------------------------------------------------------------
		if(	!timelength->runlength ||
			timevec->read >= timelength->maxVal	)
		{
			++timevec->count;
			nPercent= 100 / timevec->actValue * timevec->readValue - 100;

#ifdef __showStatistic
			if(debug)
			{
				tout << "  >> calculate middle length for " << timevec->stype << endl;
				tout << "  >> 100 / " << timevec->actValue << " * " << timevec->readValue
								<< " - 100 = " << nPercent << " = ";
			}
#endif // __showStatistic

			// make percent for holding length map
			// (this percent definition is no CPU time)
			nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);

#ifdef __showStatistic
			if(debug)
				tout << nPercent << "%" << endl;
#endif // __showStatistic

			if(timevec->reachedPercent.find(nPercent) != timevec->reachedPercent.end())
			{
				timevec->reachedPercent[nPercent].first+= 1;
				if(timelength->runlength)
				{// value is for folder length
					if(	timevec->reachedPercent[nPercent].second < timevec->readValue	)
						timevec->reachedPercent[nPercent].second= timevec->readValue;
				}else
				{// value is for middle value to reach finish of result
					timevec->reachedPercent[nPercent].second+= timevec->readValue;
					timevec->reachedPercent[nPercent].second/= 2;
				}
			}else
			{
				timevec->reachedPercent[nPercent].first= 1;
				timevec->reachedPercent[nPercent].second= timevec->readValue;
			}
			if(	(	timelength->runlength &&
					timevec->count > 10			) ||
				(	!timelength->runlength &&
					timevec->count > 2			)	)
			{
				if(timevec->newReachedPercent.find(nPercent) != timevec->newReachedPercent.end())
				{
					timevec->newReachedPercent[nPercent].first+= 1;
					if(timelength->runlength)
					{// value is for folder length
						if(	timevec->newReachedPercent[nPercent].second < timevec->readValue	)
							timevec->newReachedPercent[nPercent].second= timevec->readValue;
					}else
					{// value is for middle value to reach finish of result
						timevec->newReachedPercent[nPercent].second+= timevec->readValue;
						timevec->newReachedPercent[nPercent].second/= 2;
					}
				}else
				{
					timevec->newReachedPercent[nPercent].first= 1;
					timevec->newReachedPercent[nPercent].second= timevec->readValue;
				}
			}
			timevec->read= 0;
			timevec->readValue= 0;

#ifdef __showStatistic
			if(debug)
			{
				for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent.begin();
								it != timevec->reachedPercent.end(); ++it							)
				{
					tout << "       " << it->second.first << " values are " << it->first << "%"
									<< " value " << it->second.second << endl;
				}
				if(timevec->count < timevec->maxCount)
				{
					tout << "     making new actual value after "
						<< timevec->maxCount - timevec->count
						<< " calculation" << endl;
				}else
					tout << "     check now after " << timevec->count << " calculations" << endl;
			}
#endif // __showStatistic

			if(timevec->count >= timevec->maxCount)
			{
				bool found(false);
				short count(0), diff(0);

				nPercent= 0;
				for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent.begin();
								it != timevec->reachedPercent.end(); ++it							)
				{
					count+= it->second.first;
				}

#ifdef __showStatistic
				if(debug)
				{
					tout << "     count " << count << " values inside vector and take percent by ";
					if(timelength->runlength)
						tout << "3/4" << endl;
					else
						tout << "half" << endl;
				}
#endif // __showStatistic

				if(timelength->runlength)
					diff= count / 4 * 3;
				else
					diff= count / 2;
				count= 0;
				for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent.begin();
								it != timevec->reachedPercent.end(); ++it							)
				{
					count+= it->second.first;
					if(count >= diff)
					{
						nPercent= it->first;
						found= true;
						break;
					}
				}
#ifdef __showStatistic
				if( debug &&
					nPercent		)
				{
					tout << "     found new ";
					if(timelength->runlength)
						tout << "3/4 ";
					else
						tout << "middle ";
					tout << "value of " << nPercent << "%" << endl;
				}
#endif // __showStatistic
				if(found)
				{
					map<short, pair<short, double> > newMap;

					if(nPercent == 0)
						found= false;
					timevec->actValue= timevec->reachedPercent[nPercent].second;
					if(nPercent)
					{
						// new organizing of timevec->reachedPercent
						for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent.begin();
										it != timevec->reachedPercent.end(); ++it							)
						{
							nPercent= 100 / timevec->actValue * it->second.second - 100;
							nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);

#ifdef __showStatistic
							if(debug)
								tout << "       set value " << it->second.second << " to new "
											<< nPercent << "%" << endl;
#endif // __showStatistic

							if(newMap.find(nPercent) != newMap.end())
							{
								newMap[nPercent].first+= it->second.first;
								if(nPercent >= 0)
								{
									if(newMap[nPercent].second < it->second.second)
										newMap[nPercent].second= it->second.second;
								}else
								{
									if(newMap[nPercent].second > it->second.second)
										newMap[nPercent].second= it->second.second;

								}
							}else
							{
								newMap[nPercent].first= it->second.first;
								newMap[nPercent].second= it->second.second;
							}
						}
						timevec->reachedPercent= newMap;

						if(!timevec->newReachedPercent.empty())
						{
							newMap.clear();
							// new organizing of timevec->newReachedPercent
							for(map<short, pair<short, double> >::iterator it= timevec->newReachedPercent.begin();
											it != timevec->newReachedPercent.end(); ++it							)
							{
								nPercent= 100 / timevec->actValue * it->second.second - 100;
								nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);
								if(newMap.find(nPercent) != newMap.end())
								{
									newMap[nPercent].first+= it->second.first;
									if(nPercent >= 0)
									{
										if(newMap[nPercent].second < it->second.second)
											newMap[nPercent].second= it->second.second;
									}else
									{
										if(newMap[nPercent].second > it->second.second)
											newMap[nPercent].second= it->second.second;

									}
								}else
								{
									newMap[nPercent].first= it->second.first;
									newMap[nPercent].second= it->second.second;
								}
							}
							timevec->newReachedPercent= newMap;
						} // if(!timevec->newReachedPercent.empty())
					}
					if(found)
						bSave= true;
					else
						bSave= false;
					timevec->count= 0;
					if(timevec->maxCount < timelength->maxVal)
					{
						if(timevec->maxCount < 10)
							++timevec->maxCount;
						else
							timevec->maxCount+= 10;

	#ifdef __showStatistic
							if(debug)
								tout << "     write new maximal count as " << timelength->folder
									<< ":" << timelength->subroutine << " " << timevec->scount
									<< " with value " << timevec->maxCount << " into database" << endl;
	#endif // __showStatistic

							db= DbInterface::instance();
							db->fillValue(timelength->folder, timelength->subroutine,
											timevec->scount, timevec->maxCount, true);
					}else
					{
						timevec->reachedPercent.clear();
						timevec->reachedPercent= timevec->newReachedPercent;
						timevec->newReachedPercent.clear();
						timevec->reachedPercent[0]= pair<short, double>(1, timevec->actValue);
					}

#ifdef __showStatistic
					if(debug)
					{
						if(found)
							tout << "  >> set new actual value " << timevec->actValue << endl;
						else
							tout << "     found same difference since last check, do not save into database" << endl;
					}
#endif // __showStatistic

				}else
				{
					bSave= false;

#ifdef __showStatistic
					if(debug)
						tout << "     do not found unique percent, make by next calculation again" << endl;
#endif // __showStatistic

				}
			}//if(timevec->count > 5
		}//if(	!timevec->runlength || timevec->read >= nMaxVal)
	}
	if(bSave)
	{
		db= DbInterface::instance();
		db->fillValue(timelength->folder, timelength->subroutine, timevec->stype, timevec->actValue, true);
#ifdef __showStatistic
		if(debug)
		{
			tout << "  >> write new value " << timevec->actValue
							<< " into database" << endl;
			tout << "     as " << timelength->folder << ":" << timelength->subroutine << " "
							<< timevec->stype << endl;
		}
#endif // __showStatistic
	}
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
	DESTROYMUTEX(m_FOLDERRUNMUTEX);
	DESTROYCOND(m_VALUECONDITION);
}
