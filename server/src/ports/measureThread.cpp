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
#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../util/debug.h"
#include "../util/debugsubroutines.h"
#include "../util/exception.h"
#include "../util/thread/Terminal.h"
#include "../util/properties/PPIConfigFileStructure.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../ports/output.h"
#include "../ports/ExternPort.h"

#include "../database/lib/DbInterface.h"

#include "measureThread.h"

using namespace std;
using namespace ports;
using namespace ppi_database;
using namespace boost;
using namespace boost::algorithm;

SHAREDPTR::shared_ptr<meash_t> meash_t::firstInstance= SHAREDPTR::shared_ptr<meash_t>();
string meash_t::clientPath= "";


/**
 * global mutex by calculating CPU time
 */
pthread_mutex_t *globalCPUMUTEX= Thread::getMutex("globalCPUMUTEX");

MeasureThread::MeasureThread(const string& threadname, const MeasureArgArray& tArg,
				const SHAREDPTR::shared_ptr<measurefolder_t> pFolderStart,
				const time_t& nServerSearch, bool bNoDbRead, short folderCPUtime)
: Thread(threadname, true),
#ifdef __followSETbehaviorToFolder
  m_oToFolderExp(__followSETbehaviorToFolder),
  m_oToSubExp(__followSETbehaviorToSubroutine),
#endif
  m_sFolder(threadname),
  m_oRunnThread(threadname, "parameter_run", "run", false, true, pFolderStart->subroutines[0].portClass.get()),
  m_oInformOutput(new Output(threadname, threadname, /*objectID*/0)),
  m_ACTIVATETIME(Thread::getMutex("ACTIVATETIME")),
  m_FOLDERRUNMUTEX(Thread::getMutex("FOLDERRUNMUTEX")),
  m_DEBUGLOCK(Thread::getMutex("DEBUGLOCK")),
  m_WANTINFORM(Thread::getMutex("WANTINFORM")),
  m_INFORMERCACHECREATION(Thread::getMutex("INFORMERCACHECREATION")),
  m_VALUECONDITION(Thread::getCondition("VALUECONDITION")),
  m_oInformer(threadname, this),
  m_oDbFiller(DbFillerFactory::getInstance(threadname, PPIConfigFileStructure::instance()->getFolderDbThreads()))
{
	int res;
	string run;
	SHAREDPTR::shared_ptr<measurefolder_t> pCurrent;

#ifdef DEBUG
	cout << "constructor of measurethread for folder " << getFolderName() << endl;
#endif // DEBUG


	m_bNeedFolderRunning= false;
	m_bFolderRunning= false;
	m_nServerSearchSeconds= nServerSearch;
	m_bDebug= false;
	m_bInformParam= false;
	m_nActCount= 0;
	m_nRunCount= -1;
	m_tRunThread= Thread::getThreadID();
	m_pvlPorts= tArg.ports;
	m_pvtSubroutines= tArg.subroutines;
	m_vStartDebugSubs= tArg.debugSubroutines;
	m_bNeedLength= false;
	m_bNoDbReading= bNoDbRead;
	m_nFolderCPUtime= folderCPUtime;
	m_nSchedPolicy= SCHED_OTHER;
	m_nSchedPriority= 0;
	pCurrent= pFolderStart;
	while(pCurrent)
	{
		if(pCurrent->name == threadname)
		{
			run= pCurrent->folderProperties->getValue("run", /*warning*/false);
			if(run != "")
			{
				size_t pos;

				pos= run.find(";");
				if(pos != string::npos)
				{// define calculation whether folder is running
					m_oRunnThread.init(pFolderStart, run.substr(pos+1));
					run= run.substr(0, pos);
				}
				split(m_vsFolderSecs, run, is_any_of(" "));
				for(vector<string>::iterator it= m_vsFolderSecs.begin(); it != m_vsFolderSecs.end(); ++it)
					trim(*it);
			}
			m_sInformeThreadStatement= pCurrent->folderProperties->getValue("inform", /*warning*/false);
			run= pCurrent->folderProperties->getValue("policy", /*warning*/false);
			if(run != "")
			{
				if(run == "SCHED_OTHER")
					m_nSchedPolicy= SCHED_OTHER;
				else if(run == "SCHED_BATCH")
					m_nSchedPolicy= SCHED_BATCH;
				else if(run == "SCHED_IDLE")
					m_nSchedPolicy= SCHED_IDLE;
				else if(run == "SCHED_FIFO")
					m_nSchedPolicy= SCHED_FIFO;
				else if(run == "SCHED_RR")
					m_nSchedPolicy= SCHED_RR;
				else
				{
					string err;

					err=  "ERROR: found undefined scheduling policy for folder " + m_sFolder + "\n";
					err+= "       only SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_FIFO or SCHED_RR are allowed\n";
					err+= "       do not set folder thread into any other priority";
					LOG(LOG_ERROR, err);
					tout << err << endl;

				}
			}
			string param("priority");

			m_nSchedPriority= pCurrent->folderProperties->getInt(param, /*warning*/false);
			if(param != "#ERROR")
			{
				int min, max;

				min= sched_get_priority_min(m_nSchedPolicy);
				max= sched_get_priority_max(m_nSchedPolicy);
				if(	m_nSchedPriority < min ||
					m_nSchedPriority > max		)
				{
					ostringstream warn;

					if(run == "")
						run= "SCHED_OTHER";
					if(m_nSchedPriority < min)
						m_nSchedPriority= min;
					else if(m_nSchedPriority > max)
						m_nSchedPriority= max;
					warn << "WARNING: cannot define specific priority for " << run << " policy\n";
					warn << "         inside folder " << m_sFolder << endl;
					warn << "         write now priority " << m_nSchedPriority << " for defined policy";
					LOG(LOG_WARNING, warn.str());
					tout << warn.str() << endl;
				}
			}else
			{
				if(	m_nSchedPolicy == SCHED_FIFO ||
					m_nSchedPolicy == SCHED_RR		)
				{
					m_nSchedPriority= 1;
				}else
					m_nSchedPriority= 0;
			}
			if(run != "")// run is policy
			{
				int min, max;

				min= sched_get_priority_min(m_nSchedPolicy);
				max= sched_get_priority_max(m_nSchedPolicy);
				if(	m_nSchedPriority < min ||
					m_nSchedPriority > max		)
				{
					ostringstream warn;

					warn << "WARNING: found wrong priority " << m_nSchedPriority;
					warn << " for " + run + " policy" << endl;
					warn << "         inside folder " << m_sFolder << endl;
					if(m_nSchedPriority < min)
						m_nSchedPriority= min;
					else if(m_nSchedPriority > max)
						m_nSchedPriority= max;
					warn << "         write now priority " << m_nSchedPriority << " for defined policy";
					LOG(LOG_WARNING, warn.str());
					tout << warn.str() << endl;
				}
			}
			break;
		}
		pCurrent= pCurrent->next;
	}
	if(PPIConfigFileStructure::instance()->needInformThreads())
	{
		res= m_oInformer.start();
		if(res)
		{
			string err("measuring thread for folder '" + threadname +
							"' can not activate external thread '");

			err+= m_oInformer.getThreadName() + "' to inform other folders\n";
			cerr << "### WARNING: " << err;
			cerr << "              so folder list inform directly other folders, which has more bad performance" << endl;
			LOG(LOG_WARNING, err +"so folder list inform directly other folders, which has more bad performance");
		}
	}
}

bool MeasureThread::setDebug(bool bDebug, bool bInform, const string& subroutine)
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
	out << " set debug to " << boolalpha << bDebug << " from folder " << getFolderName() << endl;
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
	if(!isDebug)
		bInform= false;
	LOCK(m_INFORMERCACHECREATION);
	for(vector<SHAREDPTR::shared_ptr<MeasureInformerCache> >::iterator it= m_voInformerCaches.begin();
					it != m_voInformerCaches.end(); ++it						)
	{
		(*it)->doOutput(bInform);
	}
	UNLOCK(m_INFORMERCACHECREATION);
	LOCK(m_DEBUGLOCK);
	m_oInformOutput->out().str("");
	if(isDebug)
	{
		m_bDebug= true;
		m_bInformParam= bInform;
	}else
	{
		m_bDebug= false;
		m_bInformParam= false;
	}
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
	string sFault, folder(getFolderName());
	DbInterface *db= DbInterface::instance();

#ifdef __followSETbehaviorFromFolder
	boost::regex folderExp(__followSETbehaviorFromFolder);

	if(boost::regex_match(folder, folderExp))
		m_btimer= true;
	else
		m_btimer= false;
#endif // __followSETbehaviorFromFolder
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
			// debugging output for different folders, subroutines
			/*if(	m_sFolder == "Raff1_Zeit" &&
				(*m_pvtSubroutines)[n].name == "grad"	)
			{
				cout << "stopping by set observer for " << m_sFolder << ":" << (*m_pvtSubroutines)[n].name << endl;
				cout << __FILE__ << __LINE__ << endl;
			}*/
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
		if(!setDebug(true, false, *it))
			sFault+= "                             " + *it + "\n";
	}
	if(sFault != "")
	{
		cout << "### WARNING: cannot find follow subroutine(s) inside folder " << getFolderName() << ":" << endl;
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
		m_tLengthType.subroutine= getFolderName();
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
					m_tLengthType.percentSyncDiff["none"][n].dbValue= dLength;
					(*m_tLengthType.percentSyncDiff["none"][n].reachedPercent)[0]= pair<short, double>(1, dLength);
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
	if(m_nSchedPolicy != SCHED_OTHER)
	{
		if(setSchedulingParameter(m_nSchedPolicy, m_nSchedPriority))
		{
			ostringstream info;

			info << "set scheduling policy from folder " << m_sFolder << " to ";
			switch(m_nSchedPolicy)
			{
			case SCHED_OTHER:
				info << "SCHED_OTHER";
				break;
			case SCHED_BATCH:
				info << "SCHED_BATCH";
				break;
			case SCHED_IDLE:
				info << "SCHED_IDLE";
				break;
			case SCHED_RR:
				info << "SCHED_RR";
				break;
			case SCHED_FIFO:
				info << "SCHED_FIFO";
				break;
			default:
				info << "unknown";
				break;
			}
			info << " with priority " << m_nSchedPriority << endl;
			if(m_nSchedPolicy == SCHED_RR)
			{
				timespec slice;

				if(sched_rr_get_interval(gettid(), &slice) == 0)
				{
					ppi_time tm;

					tm.tv_sec= slice.tv_sec;
					tm.tv_usec= slice.tv_nsec / 1000;
					info << "with real time round robin time slices of ";
					info << tm.toString(/*as date*/false) << " seconds" << endl;
				}
			}
			LOG(LOG_INFO, info.str());
			//tout << info.str() << endl;
		}else
		{
			m_nSchedPolicy= SCHED_OTHER;
			m_nSchedPriority= 0;
		}
		db->writeIntoDb("folder", m_sFolder, "policy");
		db->writeIntoDb("folder", m_sFolder, "priority");
		db->fillValue("folder", m_sFolder, "policy", m_nSchedPolicy, /*new*/true);
		db->fillValue("folder", m_sFolder, "priority", m_nSchedPriority, /*new*/true);
	}else
	{
		bool exist;
		int value;

		db->writeIntoDb("folder", m_sFolder, "policy");
		value= static_cast<int>(db->getActEntry(exist, "folder", m_sFolder, "policy"));
		if(exist && value != SCHED_OTHER)
			db->fillValue("folder", m_sFolder, "policy", SCHED_OTHER);
		db->writeIntoDb("folder", m_sFolder, "priority");
		value= static_cast<int>(db->getActEntry(exist, "folder", m_sFolder, "priority"));
		if(exist && value != 0)
			db->fillValue("folder", m_sFolder, "priority", 0);
	}
	return 0;
}

bool MeasureThread::usleep(const IPPITimePattern& time)
{
	useconds_t usWait;

	m_tvSleepLength+= time;
	usWait= time.tv_sec * 1000000;
	usWait+= time.tv_usec;
	Thread::USLEEP(usWait);
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

				ex.addMessage("by stopping subroutine " + it->name + " inside folder " + getFolderName());
				err= ex.getTraceString();
				cerr << err << endl;
				LOG(LOG_ERROR, err);

			}catch(std::exception& ex)
			{
				string err("std exception by stopping subroutine " + it->name + " inside folder " + getFolderName());

				err+= "\n" + string(ex.what());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);

			}catch(...)
			{
				string err("undefined exception by stopping subroutine " + it->name + " inside folder " + getFolderName());
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

void MeasureThread::beginCounting()
{
	LOCK(m_FOLDERRUNMUTEX);
	m_nRunCount= 0;
	UNLOCK(m_FOLDERRUNMUTEX);
}

int MeasureThread::getRunningCount(map<ppi_time, vector<string> >& starts)
{
	int nRv;

	LOCK(m_FOLDERRUNMUTEX);
	nRv= m_nRunCount;
	m_nRunCount= -1;
	starts= m_vStartingCounts;
	m_vStartingCounts.clear();
	UNLOCK(m_FOLDERRUNMUTEX);
	return nRv;
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

IInformerCachePattern* MeasureThread::getInformerCache(const string& folder)
{
	IInformerCachePattern* pRv;

	pRv= getUsedInformerCache(folder);
	if(pRv == NULL)
	{
		LOCK(m_INFORMERCACHECREATION);
		m_voInformerCaches.push_back(
						SHAREDPTR::shared_ptr<MeasureInformerCache>(
										new MeasureInformerCache(folder, this, m_oInformOutput)	)	);
		pRv= static_cast<IInformerCachePattern*>(m_voInformerCaches.back().get());
		UNLOCK(m_INFORMERCACHECREATION);
	}
	return pRv;
}

IMeasurePattern::awakecond_t MeasureThread::getAwakeConditions()
{
	awakecond_t awake;

	awake.wantinform= m_WANTINFORM;
	awake.activatetime= m_ACTIVATETIME;
	awake.valuecondition= m_VALUECONDITION;
	return awake;
}

IInformerCachePattern* MeasureThread::getUsedInformerCache(const string& folder)
{
	IInformerCachePattern* pRv= NULL;

	LOCK(m_INFORMERCACHECREATION);
	for(vector<SHAREDPTR::shared_ptr<MeasureInformerCache> >::iterator it= m_voInformerCaches.begin();
					it != m_voInformerCaches.end(); ++it	)
	{
		if((*it)->getFolderName() == folder)
		{
			pRv= static_cast<IInformerCachePattern*>(it->get());
			break;
		}
	}
	UNLOCK(m_INFORMERCACHECREATION);
	return pRv;
}

void MeasureThread::removeObserverCache(const string& folder)
{
	LOCK(m_INFORMERCACHECREATION);
	for(vector<SHAREDPTR::shared_ptr<MeasureInformerCache> >::iterator it= m_voInformerCaches.begin();
					it != m_voInformerCaches.end(); ++it	)
	{
		if((*it)->getFolderName() == folder)
		{
			m_voInformerCaches.erase(it);
			break;
		}
	}
	UNLOCK(m_INFORMERCACHECREATION);
}

void MeasureThread::informFolders(const folders_t& folders, const string& from,
										const string& as, const bool debug, pthread_mutex_t *lock)
{
	m_oInformer.informFolders(folders, from, as, debug, lock);
	if(m_oInformer.running())
	{
		vector<string> spl;
		/*
		 * when no informer thread running,
		 * informing was made directly
		 * and no thread is to arouse
		 */
		split(spl, from, is_any_of(":"));
		if(	spl[0] == "e" || // < from external need arousing
			(	spl.size() > 1 && // < value was changed not from own folder,
				spl[0] == "i" &&  //   so arouse informer thread
				spl[1] != m_sFolder	)	)
		{
			m_oInformer.arouseInformerThread();
		}
	}
}

int MeasureThread::execute()
{
	bool debug(isDebug());
	/**
	 * from which folder:subroutine the thread
	 * was informed to change
	 */
	vector<string> vInformed;
	string folder;
	/**
	 * time of measure ending
	 * from all subroutines
	 */
	ppi_time end_tv;
	ppi_time diff_tv;
	timespec waittm;
	vector<ppi_time>::iterator akttime, lasttime;

	//Debug info before measure routine to stop by right folder
	folder= getFolderName();
	if(!m_tvStartTime.isSet())
	{
		if(!m_tvStartTime.setActTime())
		{
			tout << " ERROR: cannot calculate time of beginning" << endl;
			tout << " " << m_tvStartTime.errorStr() << endl;
			m_tvStartTime.clear();
		}
	}
	m_vStartTimes.push_back(m_tvStartTime);
	LOCK(m_FOLDERRUNMUTEX);
	if(m_nRunCount >= 0)
	{
		map<ppi_time, vector<string> >::iterator found;
		ppi_time nullTime;

		++m_nRunCount;
		found= m_vStartingCounts.find(nullTime);
		if(found != m_vStartingCounts.end())
		{
			m_vStartingCounts[m_tvStartTime]= found->second;
			m_vStartingCounts.erase(found);
		}else
			m_vStartingCounts[m_tvStartTime]= vector<string>();
	}
	UNLOCK(m_FOLDERRUNMUTEX);
	measure();
	//Debug info behind measure routine to stop by right folder
	/*folder= getFolderName();
	if(folder == "Raff1_Zeit")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << "starting folder " << folder << endl;
	}*/

	if(!end_tv.setActTime())
	{
		string err("ERROR: cannot calculate ending time of hole folder list from '");

		err+= getFolderName() + "'\n" + end_tv.errorStr();
		if(debug)
		{
			tout << "--------------------------------------------------------------------" << endl;
			tout << err << endl;
		}
		TIMELOG(LOG_ERROR, "ending_time_" + folder, err);
		end_tv.tv_sec= 0;
		end_tv.tv_usec= 0;
	}
	diff_tv.clear();
	if(	m_bNeedLength &&
		end_tv.isSet() &&
		m_tvStartTime.isSet()	)
	{
		diff_tv= end_tv - m_tvStartTime;
		if(m_tvSleepLength.isSet())
		{
			diff_tv-= m_tvSleepLength;
			m_tvSleepLength.clear();
		}
		LOCK(m_DEBUGLOCK);
		calcLengthDiff(&m_tLengthType, diff_tv, debug);
		UNLOCK(m_DEBUGLOCK);
	}


	if(debug)
	{
		ostringstream out;

		if(!diff_tv.isSet())
		{
			diff_tv= end_tv - m_tvStartTime;
			if(m_tvSleepLength.isSet())
			{
				diff_tv-= m_tvSleepLength;
				m_tvSleepLength.clear();
			}
		}
		folder= getFolderName();
		out << "--------------------------------------------------------------------" << endl;
		out << " folder '" << folder << "' STOP (" << end_tv.toString(/*as date*/true) << ")" ;
		out << "  running time (" << diff_tv.toString(/*as date*/false) << ")" << endl;
		out << "--------------------------------------------------------------------" << endl;
		tout << out.str();
		TERMINALEND;
	}
	diff_tv.clear();
	TERMINALEND;
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
				if(end_tv < *akttime)
					break;
				fold= true;
				vInformed.push_back("#timecondition " + getTimevalString(*akttime, /*as date*/true, debug));
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
					msg+= getFolderName() + " to search for external port server\n";
					msg+= "       waiting only for 10 seconds !!!";
					TIMELOG(LOG_ALERT, "gettimeofday", msg);
					if(debug)
						tout << msg << endl;
					UNLOCK(m_ACTIVATETIME);
					SLEEP(10);
					LOCK(m_ACTIVATETIME);
				}else
				{
					waittm.tv_sec= tv.tv_sec + m_nServerSearchSeconds;
					waittm.tv_nsec= tv.tv_usec * 1000;
				}
			}
			bool bRun(false);

			LOCK(m_ACTIVATETIME);
			bRun= checkToStart(vInformed, debug);
			if(!vInformed.empty())
				bRun= true;
			while(bRun == false)
			{
				if(m_bNeedFolderRunning)
				{
					LOCK(m_FOLDERRUNMUTEX);
					m_bFolderRunning= false;
					UNLOCK(m_FOLDERRUNMUTEX);
				}
				// set timevec (nanoseconds) into timeval (microseconds)
				// for wanted awake time
				diff_tv.tv_sec= waittm.tv_sec;
				diff_tv.tv_usec= waittm.tv_nsec / 1000;
				condRv= TIMECONDITION(m_VALUECONDITION, m_ACTIVATETIME, &waittm);
				if(stopping())
				{
					UNLOCK(m_ACTIVATETIME);
					if(debug)
						TERMINALEND;
					return 0;
				}
				if(gettimeofday(&m_tvStartTime, NULL))
				{
					string msg("### DEBUGGING for folder ");

					folder= getFolderName();
					msg+= folder + " is aktivated!\n";
					msg+= "    ERROR: cannot calculate time of beginning";
					TIMELOGEX(LOG_ERROR, folder, msg, getExternSendDevice());
					if(isDebug())
						tout << " ERROR: cannot calculate time of beginning" << endl;
					timerclear(&m_tvStartTime);

				}
				if(m_bNeedFolderRunning)
				{
					LOCK(m_FOLDERRUNMUTEX);
					m_bFolderRunning= true;
					UNLOCK(m_FOLDERRUNMUTEX);
				}
				if(condRv == ETIMEDOUT)
				{
					if(!bSearchServer)
						vInformed.push_back("#timecondition " + getTimevalString(*akttime, /*as date*/true, debug));
					else
						vInformed.push_back("#searchserver");
					bRun= true;
				}else
					bRun= checkToStart(vInformed, debug);
			}//while(bRun == false)
			UNLOCK(m_ACTIVATETIME);
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
	}else
	{
		bool bRun;

		LOCK(m_ACTIVATETIME);
		bRun= checkToStart(vInformed, debug);
		while(bRun == false)
		{
			if(m_bNeedFolderRunning)
			{
				LOCK(m_FOLDERRUNMUTEX);
				m_bFolderRunning= false;
				UNLOCK(m_FOLDERRUNMUTEX);
			}
			CONDITION(m_VALUECONDITION, m_ACTIVATETIME);
			if(stopping())
			{
				UNLOCK(m_ACTIVATETIME);
				if(debug)
					TERMINALEND;
				return 0;
			}
			if(gettimeofday(&m_tvStartTime, NULL))
			{
				string msg("### DEBUGGING for folder ");

				folder= getFolderName();
				msg+= folder + " is aktivated!\n";
				msg+= "    ERROR: cannot calculate time of beginning";
				TIMELOGEX(LOG_ERROR, folder, msg, getExternSendDevice());
				if(isDebug())
					tout << " ERROR: cannot calculate time of beginning" << endl;

			}
			if(m_bNeedFolderRunning)
			{
				LOCK(m_FOLDERRUNMUTEX);
				m_bFolderRunning= true;
				UNLOCK(m_FOLDERRUNMUTEX);
			}
			bRun= checkToStart(vInformed, debug);
		}//while(bRun == false)
		UNLOCK(m_ACTIVATETIME);
	}
#ifdef __followSETbehaviorFromFolder
	if(	m_btimer &&
		__followSETbehaviorFrom <= 6 &&
		__followSETbehaviorTo >= 6		)
	{
		string informed;

		for(vector<pair<string, ppi_time> >::iterator i= vInformed.begin(); i != vInformed.end(); ++i)
		{
			vector<string> spl;

			split(spl, i->first, is_any_of(":"));
			if(	string(__followSETbehaviorFromFolder) == "" ||
				boost::regex_match(spl[0], m_oToFolderExp)		)
			{
				if(	string(__followSETbehaviorFromSubroutine) == "" ||
					boost::regex_match(spl[1], m_oToSubExp)				)
				{
					if(informed != "")
						informed+= "\n";
					informed+= "         " + i->first;
				}
			}
		}
		cout << "[6] " << getFolderName() << " starting ";
		if(informed != "")
			cout << " informed over:" << endl << informed << endl;
		else
			cout << "without informing from right wanted folder:subroutine" << endl;
	}
#endif // __followSETbehaviorFromFolder
	if(isDebug())
	{
		ostringstream out;
		string msg("### DEBUGGING for folder ");

		if(folder == "")
			folder= getFolderName();
		msg+= folder + " is aktivated!";
		TIMELOG(LOG_INFO, folder, msg);

		out << "--------------------------------------------------------------------" << endl;
		out << " folder '" << folder << "' ";
		if(m_tvStartTime.isSet())
		{
			out << "AWAKED after (";
			out << getTimevalString(m_tvStartTime, /*as date*/true, /*debug*/true) << ")" << endl;
			if(diff_tv.isSet())
			{
				string::size_type nLen;
				string space;

				m_tvStartTime= diff_tv;
				nLen= string(" folder '" + folder + "' AWAKED after ").length();
				nLen-= string("wanted AWAKE by ").length();
				out << "wanted AWAKE by (";
				out << getTimevalString(m_tvStartTime, /*as date*/true, /*debug*/true) << ")" << endl;
			}else
				m_tvStartTime= end_tv; // after last folder end
		}else
		{
			m_tvStartTime= end_tv;
			out << "running after ";
			if(m_nSchedPolicy == SCHED_FIFO)
			{
				sched_yield();
				out << "sched_yield ";
			}
			out << "STOP (";
			out << getTimevalString(m_tvStartTime, /*as date*/true, /*debug*/true) << ")" << endl;
		}
		if(m_nSchedPolicy != SCHED_OTHER)
		{
			out << "  running with scheduling policy ";
			switch(m_nSchedPolicy)
			{
			case SCHED_OTHER:
				out << "SCHED_OTHER";
				break;
			case SCHED_BATCH:
				out << "SCHED_BATCH";
				break;
			case SCHED_IDLE:
				out << "SCHED_IDLE";
				break;
			case SCHED_RR:
				out << "SCHED_RR";
				break;
			case SCHED_FIFO:
				out << "SCHED_FIFO";
				break;
			default:
				out << "unknown";
				break;
			}
			out << " and priority " << m_nSchedPriority << endl;
		}
		if(!m_vStartTimes.empty())
		{
			out << "###StartTHID_" << (m_vStartTimes.size() + 1) << "  showing some information later" << endl;
		}else
			out << "###StartTHID_0" << endl;
		for(vector<string>::iterator i= vInformed.begin(); i != vInformed.end(); ++i)
		{
			if(i->substr(0, 15) == "|SHELL-command_")
			{
				out << "    informed over SHELL script " << i->substr(15) << endl;

			}else
			{
				out << "    informed ";
				if(i->substr(0, 1) == "|")
				{
					if(i->substr(1, 1) == "|")
						out << "from ppi-reader '" << i->substr(2) << "'" << endl;
					else
						out << "over Internet connection account '" << i->substr(1) << "'" << endl;
				}else
					out << "from " << *i << " because value was changed" << endl;
			}

		}
		for(vector<string>::iterator i= vInformed.begin(); i != vInformed.end(); ++i)
		{
			if(i->substr(0, 15) == "#timecondition ")
			{
				out << "      awaked from setting time " << i->substr(15) << endl;

			}else if(i->substr(0, 13) == "#searchserver")
			{
				out << "      awaked to search again for external port server (owserver)" << endl;

			}else
					out << "from " << *i << " because value was changed" << endl;

		}
		if(gettimeofday(&end_tv, NULL))
		{
			string msg("### DEBUGGING for folder ");

			folder= getFolderName();
			msg+= folder + " is aktivated!\n";
			msg+= "    ERROR: cannot calculate time of beginning";
			TIMELOGEX(LOG_ERROR, folder, msg, getExternSendDevice());
			if(isDebug())
				tout << " ERROR: cannot calculate time of beginning" << endl;
			timerclear(&m_tvStartTime);

		}
		out << " folder START by (";
		out << getTimevalString(end_tv, /*as date*/true, /*debug*/true) << ")" << endl;
		out << "--------------------------------------------------------------------" << endl;
		tout << out.str();
		TERMINALEND;

	}else
	{// by debug session when StartTime not set, StartTime will be set inside debug output block
		if(	m_tvStartTime.isSet() &&
			diff_tv.isSet()			)
		{
			m_tvStartTime= diff_tv; // after CONDITION
		}else
			m_tvStartTime= end_tv; // after last folder end
		if(m_nSchedPolicy == SCHED_FIFO)
			sched_yield();
	}
	return 0;
}

bool MeasureThread::checkToStart(vector<string>& vInformed, const bool debug)
{
	bool bLocked, bDoStart(false);
	/**
	 * bDebugShow can be true when
	 * incoming debug is true
	 * or only ppi-client was started
	 * with parameter SHOW
	 */
	bool bDebugShow(debug);
	ostringstream out;
	map<short, vector<string> > mInformed;
	SHAREDPTR::shared_ptr<MeasureInformerCache> lastCache;
	map<short, vector<string> >::iterator found;

	LOCK(m_FOLDERRUNMUTEX);
	if(m_nRunCount >= 0)
		bDebugShow= true;
	UNLOCK(m_FOLDERRUNMUTEX);
	LOCK(m_INFORMERCACHECREATION);
	if(m_voInformerCaches.empty())
	{
		UNLOCK(m_INFORMERCACHECREATION);
		return false;
	}
	lastCache= m_voInformerCaches.back();
	for(vector<SHAREDPTR::shared_ptr<MeasureInformerCache> >::iterator it= m_voInformerCaches.begin();
					it != m_voInformerCaches.end(); ++it	)
	{
		bDoStart= (*it)->shouldStarting(m_vStartTimes, mInformed, &bLocked, bDebugShow);
		if(	*it == lastCache &&
			!bLocked				)
		{
			m_vStartTimes.clear();
			break;
		}
		if(bDoStart)
			break;
	}
	UNLOCK(m_INFORMERCACHECREATION);
	/*
	 * mInformed only be filled
	 * when debugging session
	 * for folder thread be set
	 * or ppi-client was started
	 * with parameter SHOW
	 */
	if(mInformed.empty())
		return bDoStart;

	found= mInformed.find(SHRT_MAX);
	if(found != mInformed.end())
	{
		vInformed.insert(vInformed.end(), found->second.begin(), found->second.end());
		LOCK(m_FOLDERRUNMUTEX);
		if(m_nRunCount >= 0)
		{
			ppi_time nullTime;

			m_vStartingCounts[nullTime].insert(m_vStartingCounts[nullTime].end(),
							found->second.begin(), found->second.end());
		}
		UNLOCK(m_FOLDERRUNMUTEX);
		mInformed.erase(found);
	}
	if(mInformed.empty())
		return bDoStart;
	if(debug)
	{
		out << "--------------------------------------------------------------------" << endl;
		out << "STARTING reason from running sessions before" << endl;
	}
	for(map<short, vector<string> >::iterator it= mInformed.begin();
					it != mInformed.end(); ++it						)
	{
		LOCK(m_FOLDERRUNMUTEX);
		if(m_nRunCount >= 0)
		{
			ppi_time startTime;

			if(m_vStartTimes.size() <= static_cast<vector<ppi_time>::size_type>(it->first))
				startTime= m_vStartTimes[(it->first - 1)];
			m_vStartingCounts[startTime].insert(m_vStartingCounts[startTime].end(),
							it->second.begin(), it->second.end());
		}
		UNLOCK(m_FOLDERRUNMUTEX);
#ifdef __followSETbehaviorFromFolder
		// toDo: running only when debug session for folder thread defined
		//		 but should also run inside other case
		if(	m_btimer &&
			__followSETbehaviorFrom <= 5 &&
			__followSETbehaviorTo >= 5		)
		{
			vector<string> spl;

			split(spl, it->second, is_any_of(":"));
			if(	string(__followSETbehaviorFromFolder) == "" ||
				boost::regex_match(spl[0], m_oToFolderExp)		)
			{
				if(	string(__followSETbehaviorFromSubroutine) == "" ||
					boost::regex_match(spl[1], m_oToSubExp)				)
				{
					cout << "[5] remove old informing over " << it->first << endl;
				}
			}
		}
#endif // __followSETbehaviorFromFolder

		if(debug)
		{
			out << "###THID_" << it->first << endl;
			for(vector<string>::iterator vit= it->second.begin();
							vit != it->second.end(); ++vit		)
			{
				if(vit->substr(0, 15) == "|SHELL-command_")
				{
					out << "    informed over SHELL script " << vit->substr(15) << endl;

				}else
				{
					out << "    informed ";
					if(vit->substr(0, 1) == "|")
					{
						if(vit->substr(1, 1) == "|")
							out << "from ppi-reader '" << vit->substr(2) << "'" << endl;
						else
							out << "over Internet connection account '" << vit->substr(1) << "'" << endl;
					}else
						out << "from " << *vit << " because value was changed" << endl;
				}
			}//foreach(second var[vector] of mInformed)
		}//if(debug)
	}//foreach(mInformed)
	if(debug)
		out << "--------------------------------------------------------------------" << endl;
	tout << out.str();
	TERMINALEND;
	return bDoStart;
}

void MeasureThread::changeActivationTime(const string& folder, const IPPITimePattern& time,
				const IPPITimePattern& newtime)
{
	LOCK(m_ACTIVATETIME);
	for(vector<ppi_time>::iterator it= m_vtmNextTime.begin(); it != m_vtmNextTime.end(); ++it)
	{
		if(*it == time)
		{
			*it= newtime;
			break;
		}
	}
	UNLOCK(m_ACTIVATETIME);
}

void MeasureThread::eraseActivateTime(const string& folder, const IPPITimePattern& time)
{
	LOCK(m_ACTIVATETIME);
	for(vector<ppi_time>::iterator it= m_vtmNextTime.begin(); it != m_vtmNextTime.end(); ++it)
	{
		if(*it == time)
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

	// toDo: maybe unused
/*	LOCK(m_VALUE);
	vRv= m_vInformed;
	UNLOCK(m_VALUE);*/
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
	string folder(getFolderName());
	ppi_time tv, tv_start, tv_end;

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
			ValueHolder oldResult;
			ValueHolder result;

			//Debug info to stop always by right folder or subroutine
		/*	string stopfolder("Raff1_Zeit");
			string stopsub("grad_timer");
			if(	getFolderName() == stopfolder &&
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
				ostringstream out;

				classdebug= true;
				out << "--------------------------------------------------------------------" << endl;
				out << "execute '" << folder << ":" << it->name;
				out << "' with value " << oldResult.value << " and type " << it->portClass->getType() << " ";
				if(notime || gettimeofday(&tv_start, NULL))
				{
					out << " (cannot calculate length)" << endl;
					notime= true;
				}else
				{
					tv_end= tv_start - tv;
					out << " (" << tv_end.toString(/*as date*/false) << ")" << endl;
					out << "   starting by time " << tv_start.toString(/*as date*/true) << endl;
				}
				it->portClass->out() << out.str();
			}
			try{
				result= it->portClass->measure(oldResult.value);

			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("running subroutine " + it->name + " inside folder " + getFolderName());
				err= ex.getTraceString();
				cerr << err << endl;
				LOG(LOG_ERROR, err);

			}catch(std::exception& ex)
			{
				string err("std exception by running subroutine " + it->name + " inside folder " + getFolderName());

				err+= "\n" + string(ex.what());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);

			}catch(...)
			{
				string err("undefined exception by running subroutine " + it->name + " inside folder " + getFolderName());
				cerr << "ERROR: " << err << endl;
				LOG(LOG_ERROR, err);
			}
			if(	classdebug &&
				result.value != oldResult.value &&
				!result.lastChanging.isSet()		)
			{
				// when subroutine defined for debug output
				// and measure method from subroutine give no time back,
				// write end time into subroutine to can display for debug output,
				// otherwise end time will be created shorter before value
				// set definitely inside subroutine
				if(!result.lastChanging.setActTime())
				{
					string msg("ERROR: cannot get time of day,\n");

					msg+= "       so cannot measure time for TIMER function in folder ";
					msg+= folder + " to set subroutine modification time for debug output\n";
					msg+= result.lastChanging.errorStr();
					TIMELOG(LOG_ALERT, "changedValueMeasureThread", msg);
				}
			}
			if(result.value != oldResult.value)
				it->portClass->setValue(result, "i:"+folder+":"+it->name);
			else
				it->portClass->noChange();
			m_oDbFiller->informDatabase();
			if(classdebug)
			{
				string modified;
				ppi_time length;
				ostringstream out;

				if(!tv_end.setActTime())
				{
					string msg("ERROR: cannot get time of day,\n");

					msg+= "       so cannot measure time for TIMER function in folder ";
					msg+= getFolderName() + " for changing length time running of subroutine";
					TIMELOG(LOG_ALERT, "changedValueMeasureThread2", msg);
					tv_end.clear();
				}
				length= tv_end - tv_start;
				out << " subroutine running ";
				out << length.toString(/*as date*/false);
				out << " seconds, ending by ";
				out << tv_end.toString(/*as date*/true);
				out << endl;
				if(it->portClass->needChangingTime())
				{
					if(result.value != oldResult.value)
					{
						modified= result.lastChanging.toString(/*as date*/true);

					}else if(	result.value == oldResult.value &&
								oldResult.lastChanging.isSet()		)
					{
						modified= oldResult.lastChanging.toString(/*as date*/true);
					}
					if(modified != "")
						out << "            was last modified by " << modified << endl;
				}
				it->portClass->out() << out.str();
			}


		}else if(debug)
			it->portClass->out() << "Subroutine " << it->name << " is not correct initialized" << endl;
		if(classdebug)
		{
			it->portClass->out() << "--------------------------------------------------------------------\n";
			it->portClass->writeDebugStream();
			TERMINALEND;
		}
		if(stopping())
			break;
	}
	m_oInformer.arouseInformerThread();
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

	try{
		try{
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
		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("getCpuPercent try to read");
			throw(ex);
		}

		diff_idle= idle - *prev_idle;
		diff_total= total - *prev_total;
		if(diff_total <= 0) // method was called to quickly after each other
		{
			//UNLOCK(globalCPUMUTEX);
			return old_usage;
		}

		try{
			diff_usage= (1000 * (diff_total - diff_idle) / diff_total + 5) / 10;
		}catch(SignalException& ex)
		{
			string err;
			ostringstream calc;

			calc << "(1000 * (" << diff_total << " - " << diff_idle << ") / " << diff_total << " + 5) / 10";
			ex.addMessage("getCpuPercent calc " + calc.str());
			throw(ex);
		}

		if(diff_usage <= 0)
		{
			ostringstream oerr;
			vector<string> err(5);

			// toDo: bug >> write no strings into log file ????
			oerr << "cannot create correct CPU time:" << endl;
			err.push_back(oerr.str());
			oerr.str("");
			oerr << "   diff_idle(" << diff_idle << ") = idle(" << idle << ")"
							" - prev_idle(" << prev_idle << ")" << endl;
			err.push_back(oerr.str());
			oerr.str("");
			oerr << "  diff_total(" << diff_total << ") = total(" << total << ")"
							" - prev_total(" << prev_total << ")" << endl;
			err.push_back(oerr.str());
			oerr.str("");
			oerr << "  diff_usage(" << diff_usage << ") = (1000 * ("
							"diff_total(" << diff_total << ")"
							" - diff_idle(" << diff_idle << ")) / "
							"diff_total(" << diff_total << ") + 5) / 10" << endl;
			err.push_back(oerr.str());
			oerr.str("");
			oerr << "so return back old usage " << old_usage << "%";
			err.push_back(oerr.str());
			if(debug)
			{
				tout << "     " << err[0];
				tout << "     " << err[1];
				tout << "     " << err[2];
				tout << "     " << err[3];
				tout << "     " << err[4] << endl;
			}
			TIMELOG(LOG_WARNING, "cputimecreation",
							err[0] + err[1] +
							err[2] + err[3] + err[4]);
			return old_usage;
		}
		*prev_total= total;
		*prev_idle= idle;
		old_usage= diff_usage;
		//cout << cpu << " " << diff_usage << "%" << endl;

	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("try to actually CPU percent with getCpuPercent()");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl << endl;
		}
		return old_usage;

	}
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

IPPITimePattern& MeasureThread::getLengthedTime(const bool& logPercent, const bool& debug)
{
	short percent;

	LOCK(m_DEBUGLOCK);
	m_oOutsideTime= getLengthedTime(&m_tLengthType, &percent, logPercent, debug);
	UNLOCK(m_DEBUGLOCK);
	return m_oOutsideTime;
}

IPPITimePattern& MeasureThread::getLengthedTime(timetype_t* timelength, short *percent,
										const bool& logPercent, const bool& debug)
{
#ifdef __showStatistic
#ifdef __WRITEDEBUGALLLINES
	ostringstream& termout(*Terminal::instance()->out());
#else // __WRITEDEBUGALLLINES
	ostringstream termout;
#endif // __WRITEDEBUGALLLINES
#endif // __showStatistic
	bool nearest;
	short nPercent(100);
	int prev_idle, prev_total;
	double dRv;
	map<short, timeLen_t>* percentDiff;
	map<short, timeLen_t>::const_iterator it, last;

#ifdef __showStatistic
	if(debug)
	{
		termout << "----------------------------------------------------------" << endl;
		termout << " ---   get calculation of time length for ";
		if(timelength->runlength)
			termout << "runlength   --- " << endl;
		else
			termout << "reachend    --- " << endl;
	}
#endif // __showStatistic
	percentDiff= getPercentDiff(timelength, nearest, debug);
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
		termout << "      percentDiff has " << percentDiff->size()
					<< " different percent calculations" << endl;
#endif // __showStatistic

	for(it= percentDiff->begin(); it != percentDiff->end(); ++it)
	{

#ifdef __showStatistic
		if(debug)
		{
			termout << "      found " << it->first << "% with act " << it->second.dbValue << " read " <<
											it->second.readValue;
			if(	!timelength->runlength &&
				timelength->synchroID == ""	)
			{
				termout << " for running folders " << timelength->synchroID;
			}
			termout << endl;
		}
#endif // __showStatistic

		if(it->first >= nPercent)
		{
			*percent= it->first;
			if(it->second.actValue != 0)
				dRv= it->second.actValue;
			else if(it->second.dbValue != 0)
				dRv= it->second.dbValue;
			else
				dRv= it->second.readValue;

#ifdef __showStatistic
			if(debug)
			{
				termout << "     return time " << dRv;
				termout << " from " << it->first << "% for " << nPercent << "%";
				termout << endl;
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
			if(it->second.dbValue != 0)
				dRv= last->second.dbValue;
			else
				dRv= last->second.readValue;

#ifdef __showStatistic
			if(debug)
			{
				termout << "     return last time " << dRv;
				if(timelength->runlength)
					termout << " as 10% more";
				termout << " from " << last->first << "% for " << nPercent << "%";
				termout << endl;
			}
#endif // __showStatistic

		}else
		{
			*percent= nPercent;
			dRv= 0;
#ifdef __showStatistic
			if(debug)
				termout << "     return 0 time for " << nPercent
							<< "% because no saved value exists" << endl;
#endif // __showStatistic
		}
	}
	if(	logPercent &&
		nPercent >= 0	)
	{
		string dbentry;
		if(timelength->runlength)
			dbentry= "runpercent";
		else
			dbentry= "reachpercent";
		fillValue(timelength->folder, timelength->subroutine, dbentry,
						static_cast<double>(nPercent));
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
			termout << "     convert returning percent to " << *percent << "% for next calculation" << endl;
#endif // __showStatistic

	}else
		*percent= nPercent;
	m_oInsideTime= calcResult(dRv, /*seconds*/false);
#ifdef __showStatistic
	if(debug)
		termout << "----------------------------------------------------------" << endl;
#ifndef __WRITEDEBUGALLLINES
	*Terminal::instance()->out() << termout.str();
#endif // __WRITEDEBUGALLLINES
#endif // __showStatistic
	return m_oInsideTime;
}

map<short, IMeasurePattern::timeLen_t>* MeasureThread::getPercentDiff(timetype_t *timelength, bool &nearest, const bool&debug)
{
	typedef map<string, map<short, timeLen_t> >::iterator percentSyncIt;

	map<short, timeLen_t>* percentDiff(NULL);

	nearest= false;
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
			size_t calcBit(0), ownBit(0), beforeBit(0), lastBit(0);
			string before, last;
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
				if(id != timelength->synchroID)
				{
					calcBit= 0;
					for(string::reverse_iterator sit= id.rbegin(); sit != id.rend(); ++sit)
					{
						if(*sit == '1')
							++calcBit;
					}
					if(	calcBit > lastBit &&
						sIt->second.size() > 0	)
					{
						before= id;
						beforeBit= calcBit;
						if(beforeBit > ownBit)
							break;
						last= before;
						lastBit= beforeBit;
						beforeBit= 0;
					}
				}
			}
			if(	beforeBit > 0 &&
				lastBit > 0 	)
			{
				if(beforeBit - ownBit < ownBit -lastBit)
					percentDiff= &timelength->percentSyncDiff[before];
				else
					percentDiff= &timelength->percentSyncDiff[last];
				nearest= true;
			}else if(beforeBit > 0)
			{
				percentDiff= &timelength->percentSyncDiff[before];
				nearest= true;
			}else if(lastBit > 0)
			{
				percentDiff= &timelength->percentSyncDiff[last];
				nearest= true;
			}else
				percentDiff= &timelength->percentSyncDiff[timelength->synchroID];
		}else
			percentDiff= &timelength->percentSyncDiff[timelength->synchroID];
	}
	return percentDiff;
}

void MeasureThread::calcLengthDiff(timetype_t *timelength,
				const IPPITimePattern& length, const bool& debug)
{
#ifdef __showStatistic
#ifdef __WRITEDEBUGALLLINES
	ostringstream& termout(*Terminal::instance()->out());
#else // __WRITEDEBUGALLLINES
	ostringstream termout;
#endif // __WRITEDEBUGALLLINES
#endif // __showStatistic
	bool bSave(false);
	short nPercent;
	int prev_idle, prev_total;
	double value;
	timeLen_t* timevec;
	map<short, timeLen_t>* percentDiff;
	map<short, timeLen_t>::iterator it;
	/**
	 * calculate runlength always by this count
	 * of lengths value
	 */
	const short runlengthCount(3);

#ifdef __showStatistic
	if(debug)
	{
		termout << "------------------------------------------------------" << endl;
		termout << " ---   calculation of time length for ";
		if(timelength->runlength)
			termout << "runlength   --- " << endl;
		else
			termout << "reachend    --- " << endl;
	}
#endif // __showStatistic
	if(timelength->runlength)
		percentDiff= &timelength->percentSyncDiff["none"];
	else
		percentDiff= &timelength->percentSyncDiff[timelength->synchroID];
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
				termout << "     create new object for " << nPercent << "%";
				if(	!timelength->runlength &&
					timelength->synchroID != ""	)
				{
					termout << " by running folders " << timelength->synchroID;
				}
				termout << endl;
			}
#endif // __showStatistic

			if(timelength->runlength)
			{
				dbstr << "runlength";
				maxcount << "maxcount";
				maxcount << nPercent;
			}else
				dbstr << "reachend";
			if(	!timelength->runlength &&
				timelength->synchroID != ""	)
			{
				dbstr << timelength->synchroID << "-";
				//maxcount << timelength->synchroID << "-";
			}
			dbstr << nPercent;
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
		timevec->dbValue= value;
		timevec->actValue= value;
		timevec->readValue= value;
		(*timevec->reachedPercent)[0]= pair<short, double>(1, value);
		bSave= true;
	}else
	{
		if(timevec->actValue == 0)
			timevec->actValue= timevec->dbValue;
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
			timevec->read < runlengthCount	)
		{
			termout << "  >> calculate 7/8 length for " << timevec->stype << " "
							<< timevec->read << ". value";
			if(	!timelength->runlength &&
				timelength->synchroID != ""	)
			{
				termout << " by running folders " << timelength->synchroID;
			}
			termout << endl;
			termout << "     read " << value << " actual ";
			if(timelength->runlength)
				termout << "7/8 ";
			else
				termout << "middle ";
			termout << "value is " << timevec->readValue << " by "
							<< timevec->maxRadCount << " count" << endl;
			termout << "     calculated actual ";
			if(timelength->runlength)
				termout << "7/8 length of folder running ";
			else
				termout << "middle length of reaching result ";
			termout << "is " << timevec->actValue << ", last written into db " << timevec->dbValue << endl;
		}
#endif // __showStatistic

	// nPercent is now percent inside reachedPercent map
	// ----------------------------------------------------------------
		if(	!timelength->runlength ||
			timevec->read >= runlengthCount	)
		{
			++timevec->count;
			nPercent= 100 / timevec->dbValue * timevec->readValue - 100;

#ifdef __showStatistic
			if(debug)
			{
				termout << "  >> calculate percent position for " << timevec->stype << endl;
				termout << "  >> 100 / " << timevec->dbValue << " * " << timevec->readValue
								<< " - 100 = " << nPercent << " = ";
			}
#endif // __showStatistic

			// make percent for holding length map
			// (this percent definition is no CPU time)
			nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);

#ifdef __showStatistic
			if(debug)
				termout << nPercent << "%" << endl;
#endif // __showStatistic

			if(timevec->reachedPercent->find(nPercent) != timevec->reachedPercent->end())
			{
				(*timevec->reachedPercent)[nPercent].first+= 1;
				if(timelength->runlength)
				{// value is for folder length
					if(	(*timevec->reachedPercent)[nPercent].second < timevec->readValue	)
						(*timevec->reachedPercent)[nPercent].second= timevec->readValue;
				}else
				{// value is for middle value to reach finish of result
					(*timevec->reachedPercent)[nPercent].second+= timevec->readValue;
					(*timevec->reachedPercent)[nPercent].second/= 2;
				}
			}else
			{
				(*timevec->reachedPercent)[nPercent].first= 1;
				(*timevec->reachedPercent)[nPercent].second= timevec->readValue;
			}
			if(	(	timelength->runlength &&
					timevec->count > 10			) ||
				(	!timelength->runlength &&
					timevec->count > 2			)	)
			{
				if(timevec->newReachedPercent->find(nPercent) != timevec->newReachedPercent->end())
				{
					(*timevec->newReachedPercent)[nPercent].first+= 1;
					if(timelength->runlength)
					{// value is for folder length
						if(	(*timevec->newReachedPercent)[nPercent].second < timevec->readValue	)
							(*timevec->newReachedPercent)[nPercent].second= timevec->readValue;
					}else
					{// value is for middle value to reach finish of result
						(*timevec->newReachedPercent)[nPercent].second+= timevec->readValue;
						(*timevec->newReachedPercent)[nPercent].second/= 2;
					}
				}else
				{
					(*timevec->newReachedPercent)[nPercent].first= 1;
					(*timevec->newReachedPercent)[nPercent].second= timevec->readValue;
				}
			}
			timevec->read= 0;
			timevec->readValue= 0;

#ifdef __showStatistic
			if(debug)
			{
				for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
								it != timevec->reachedPercent->end(); ++it							)
				{
					termout << "       " << it->second.first << " values are " << it->first << "%"
									<< " value " << it->second.second << endl;
				}
				if(timevec->count < timevec->maxCount)
				{
					termout << "     making new actual value for DB after "
						<< timevec->maxCount - timevec->count
						<< " calculation" << endl;
				}else
					termout << "     check now after " << timevec->count << " calculations" << endl;
			}
#endif // __showStatistic

			bool found(false), next(false);
			short count(0), fcount(0), diff(0), mod;
#ifdef __showStatistic
			double lastActValue(timevec->actValue);
#endif // __showStatistic

			nPercent= 0;
			// count values inside vector
			for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
							it != timevec->reachedPercent->end(); ++it							)
			{
				count+= it->second.first;
			}

			if(!timelength->runlength)
			{
				diff= count / 2;
				mod= count % 2;
				diff+= mod;
			}else
				diff= static_cast<short>(static_cast<float>(count) / 8 * 7);

			for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
							it != timevec->reachedPercent->end(); ++it							)
			{
				if(next == true)
				{
					timevec->actValue+= it->second.second;
					timevec->actValue/= 2;
					break;
				}
				fcount+= it->second.first;
				if(fcount >= diff)
				{
					timevec->actValue= it->second.second;
					if(	!timelength->runlength &&
						mod == 0 &&
						fcount == diff				)
					{
						next= true;
					}
					nPercent= it->first;
					found= true;
					if(next == false)
						break;
				}
			}
#ifdef __showStatistic
			if(debug)
			{
				ostringstream out;
				out << "     count " << count << " values inside vector and take the ";
				if(next)
					out << "half of " << diff << ". and " << (diff+1);
				else
					out << diff;
				out << ". value which is ";
				if(timelength->runlength)
					out << "7/8";
				else
					out << "the average";
				out << " (" << timevec->actValue << ")" << endl;
				termout << out.str();
			}
#endif // __showStatistic

			if(found)
			{

#ifdef __showStatistic
				if( debug &&
					nPercent	)
				{
					ostringstream out;
					out << "     found new ";
					if(timelength->runlength)
						out << "7/8 ";
					else
						out << "middle ";
					out << "value";
					out << " of " << nPercent << "%";
					out << endl;
					termout << out.str();
				}
#endif // __showStatistic

				if(timevec->count >= timevec->maxCount)
				{
					SHAREDPTR::shared_ptr<percenttable_t> newMap(
									SHAREDPTR::shared_ptr<percenttable_t>(new percenttable_t));

					if(nPercent == 0)
						found= false;
					if(nPercent)
					{
						percenttable_t::reverse_iterator lastEntry(timevec->reachedPercent->rend());

						if(timevec->maxCount < timelength->maxVal)
						{// new organizing of timevec->reachedPercent
						 // only when maxCount do not reache maxVal
						 // because otherwise will be taken the newReachedPercent map
							for(percenttable_t::iterator it= timevec->reachedPercent->begin();
											it != timevec->reachedPercent->end(); ++it			)
							{
								nPercent= 100 / timevec->actValue * it->second.second - 100;
								nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);

#ifdef __showStatistic
								if(debug)
									termout << "       set by " << nPercent << "% value of "
																<< it->second.second << endl;
#endif // __showStatistic

								if(	lastEntry != timevec->reachedPercent->rend() &&
									lastEntry->first == nPercent					)
								{// when two keys has the same percent value merge them
#ifdef __showStatistic
									if(debug)
										termout << "     found two same results and merge" << endl;
#endif // __showStatistic
									(*newMap)[nPercent].first+= it->second.first;
									if(timelength->runlength)
									{
										if(nPercent >= 0)
										{
											if((*newMap)[nPercent].second < it->second.second)
												(*newMap)[nPercent].second= it->second.second;
										}else
										{
											if((*newMap)[nPercent].second > it->second.second)
												(*newMap)[nPercent].second= it->second.second;
										}
									}else
									{
										(*newMap)[nPercent].second+= it->second.second;
										(*newMap)[nPercent].second/= 2;
									}
								}else
								{// otherwise only fill in new value
									(*newMap)[nPercent].first= it->second.first;
									(*newMap)[nPercent].second= it->second.second;
								}
								lastEntry= newMap->rbegin();
							}
							timevec->reachedPercent= newMap;
						}// end of if(timevec->maxCount < timelength->maxVal)
#ifdef __showStatistic
						else
						{
							if(debug)
							{
								termout << "     take now new lower vector with only "
												<< timevec->newReachedPercent->size() << " values" << endl;
							}
						}
#endif // __showStatistic

						if(!timevec->newReachedPercent->empty())
						{
							// new organizing of timevec->newReachedPercent
							newMap= SHAREDPTR::shared_ptr<percenttable_t>(new percenttable_t);
							lastEntry= timevec->newReachedPercent->rend();

							for(percenttable_t::iterator it= timevec->newReachedPercent->begin();
											it != timevec->newReachedPercent->end(); ++it			)
							{
								nPercent= 100 / timevec->actValue * it->second.second - 100;
								nPercent= static_cast<short>(round(static_cast<float>(nPercent) / 10) * 10);

#ifdef __showStatistic
								if(	debug &&
									timevec->maxCount >= timelength->maxVal	)
								{
									termout << "       set by " << nPercent << "% value of "
																<< it->second.second << endl;
								}
#endif // __showStatistic
								if(	lastEntry != timevec->newReachedPercent->rend() &&
									lastEntry->first == nPercent						)
								{// when two keys has the same percent value merge them
#ifdef __showStatistic
									if(	debug &&
										timevec->maxCount >= timelength->maxVal	)
									{
										termout << "     found two same results and merge" << endl;
									}
#endif // __showStatistic
									(*newMap)[nPercent].first+= it->second.first;
									if(timelength->runlength)
									{
										if(nPercent >= 0)
										{
											if((*newMap)[nPercent].second < it->second.second)
												(*newMap)[nPercent].second= it->second.second;
										}else
										{
											if((*newMap)[nPercent].second > it->second.second)
												(*newMap)[nPercent].second= it->second.second;
										}
									}else
									{
										(*newMap)[nPercent].second+= it->second.second;
										(*newMap)[nPercent].second/= 2;
									}
								}else
								{// otherwise only fill in new value
									(*newMap)[nPercent].first= it->second.first;
									(*newMap)[nPercent].second= it->second.second;
								}
								lastEntry= newMap->rbegin();
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
							if(	debug &&
								timelength->runlength	)
							{
								ostringstream out;
								out << "     write new maximal count as " << timelength->folder
									<< ":" << timelength->subroutine << " " << timevec->scount
									<< " with value " << timevec->maxCount << " into database" << endl;
								termout << out.str();
							}
#endif // __showStatistic

							if(timelength->runlength)
							{
								fillValue(timelength->folder, timelength->subroutine,
												timevec->scount, timevec->maxCount);
							}
					}else // of if(timevec->maxCount < timelength->maxVal)
					{
						timevec->reachedPercent= timevec->newReachedPercent;
						timevec->newReachedPercent= SHAREDPTR::shared_ptr<percenttable_t>(new percenttable_t);
						(*timevec->newReachedPercent)[0]= pair<short, double>(1, timevec->dbValue);
					}// end of if(timevec->maxCount < timelength->maxVal)

#ifdef __showStatistic
					if(debug)
					{
						if(found)
							termout << "  >> write new actual value " << timevec->dbValue << " and also into database" << endl;
						else
							termout << "     found same difference since last check, do not save into database" << endl;
					}
#endif // __showStatistic
					if(timevec->reachedPercent->size() == 2)
					{
						count= 0;
						for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
										it != timevec->reachedPercent->end(); ++it							)
						{
							count+= it->second.first;
						}
						if(count == 2)
						{
							timevec->actValue= 0;
							for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
											it != timevec->reachedPercent->end(); ++it							)
							{
								if(timelength->runlength)// by runlength take the second higher
									timevec->actValue= it->second.second;
								else
									timevec->actValue+= it->second.second;
							}
							if(!timelength->runlength)
								timevec->actValue/= 2;

#ifdef __showStatistic
							if(debug)
							{
								ostringstream out;

								if(timelength->runlength)
									out << "     but set the second higher of both ";
								else
									out << "     but set the new actual value to half of both ";
								out << "(" << timevec->actValue << ") to actual value" << endl;
								out << "     because there are only two values measured" << endl;
								termout << out.str();
							}
#endif // __showStatistic
						}// end of if(count == 2)

					} // end of if(timevec->reachedPercent->size() == 2)
					if(found)
						timevec->dbValue= timevec->actValue;

				}else // from if(timevec->count >= timevec->maxCount)
				{
					bSave= false;
					if(	count == 2 &&
						timevec->reachedPercent->size() == 2)
					{
						timevec->actValue= 0;
						for(map<short, pair<short, double> >::iterator it= timevec->reachedPercent->begin();
										it != timevec->reachedPercent->end(); ++it							)
						{
							if(timelength->runlength)// by runlength take the second higher
								timevec->actValue= it->second.second;
							else
								timevec->actValue+= it->second.second;
						}
						if(!timelength->runlength)
							timevec->actValue/= 2;

	#ifdef __showStatistic
						if(debug)
						{
							ostringstream out;

							if(timelength->runlength)
								out << "     but set the second higher of both ";
							else
								out << "     but set the new actual value to half of both ";
							out << "(" << timevec->actValue << ") to actual value" << endl;
							out << "     because there are only two values measured" << endl;
							termout << out.str();
						}
	#endif // __showStatistic

					}else // from if(count == 2 && timevec->reachedPercent->size() == 2)
					{
						if(	timelength->runlength &&
							timevec->actValue < timevec->dbValue	)
						{
#ifdef __showStatistic
							if(debug)
							{
								ostringstream out;

								out << "     but write the new actual value to DB value "
												<< timevec->dbValue << endl;
								out << "     because " << timevec->actValue
												<< " is lower" << endl;
								termout << out.str();
							}
#endif // __showStatistic

							timevec->actValue= timevec->dbValue;
						}// end of if(	!timelength->runlength || timevec->actValue < timevec->dbValue)
#ifdef __showStatistic
						else
						{
							if(debug)
							{
								ostringstream out;

								if(timevec->actValue != lastActValue)
								{
									out << "     and write the new actual value to "
												<< timevec->actValue << endl;
								}else
								{
									out << "     and let the actaul value "
												<< timevec->actValue << " by same" << endl;
								}
								termout << out.str();
							}
						}
#endif // __showStatistic
					}// end of if(count == 2 && timevec->reachedPercent->size() == 2)
				}// end of if(timevec->count >= timevec->maxCount)

			}else // from if(found)
			{
				bSave= false;

#ifdef __showStatistic
				if(debug)
					termout << "     do not found unique percent, make by next calculation again" << endl;
#endif // __showStatistic

			} // end of if(found)
		}// end of if(	!timevec->runlength || timevec->read >= nMaxVal)
	}
	if(	bSave ||
		timelength->log	)
	{
		if(timelength->log)
			fillValue(timelength->folder, timelength->subroutine, timevec->stype, timevec->actValue);
		else
			fillValue(timelength->folder, timelength->subroutine, timevec->stype, timevec->dbValue);
#ifdef __showStatistic
		if(debug)
		{
			termout << "  >> write new ";
			if(timelength->log)
				termout << "actual value " << timevec->actValue;
			else
				termout << "value " << timevec->dbValue;
			termout << " into database";
			if(timelength->log)
				termout << " for logging";
			termout << endl;
			termout << "     as " << timelength->folder << ":" << timelength->subroutine << " "
							<< timevec->stype << endl;
		}
#endif // __showStatistic
	}
#ifdef __showStatistic
	if(debug)
		termout << "------------------------------------------------------" << endl;
#ifndef __WRITEDEBUGALLLINES
	*Terminal::instance()->out() << termout.str();
#endif // __WRITEDEBUGALLLINES
#endif // __showStatistic
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
	cout << "destructure of measureThread for folder "<< getFolderName() << endl;
#endif // DEBUG

	m_oDbFiller->remove();
	DESTROYMUTEX(m_DEBUGLOCK);
	DESTROYMUTEX(m_WANTINFORM);
	DESTROYMUTEX(m_ACTIVATETIME);
	DESTROYMUTEX(m_FOLDERRUNMUTEX);
	DESTROYMUTEX(m_INFORMERCACHECREATION);
	DESTROYCOND(m_VALUECONDITION);
}
