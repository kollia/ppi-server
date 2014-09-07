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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

#include "../../pattern/util/LogHolderPattern.h"

#include "../GlobalStaticMethods.h"

#include "../properties/configpropertycasher.h"

#include "Thread.h"
#include "StatusLogRoutine.h"

/*
 * toDo: commend all functionality from this class
 * 		 on 27/08/2014
 * 		 because do not need status for any time before
 * 		 maybe class can removed when in the future
 * 		 never needed
 * 		 functionality of class also
 * 		 not runnable
 */
#define __never_used__

using namespace std;
using namespace util;

map<pid_t, pos_t> StatusLogRoutine::m_mStatus;
pthread_mutex_t* StatusLogRoutine::m_POSITIONSTATUS= Thread::getMutex("POSITIONSTATUS");


StatusLogRoutine::StatusLogRoutine()
{
}

void StatusLogRoutine::positionA(const string file, const int line, const string identif, const string* info2, const int* ninfo2)
{
#ifndef __never_used__
	pid_t tid= Thread::gettid();
	map<pid_t, pos_t>::iterator t;

	LOCK(m_POSITIONSTATUS);
	t= m_mStatus.find(tid);
	if(t == m_mStatus.end())
	{
		UNLOCK(m_POSITIONSTATUS);
		return;
	}
	t->second.tid= tid;
	t->second.file= file;
	t->second.line= line;
	t->second.identif= identif;
	if(info2)
		t->second.info2= *info2;
	if(ninfo2)
		t->second.ninfo2= *ninfo2;
	time(&t->second.time);
	UNLOCK(m_POSITIONSTATUS);
#endif // __never_used__

}

void StatusLogRoutine::initstatus(const string threadName, IStatusLogPattern* thread)
{
#ifndef __never_used__
	pid_t tid= Thread::gettid();
	pos_t pos;

	pos.threadname= threadName;
	pos.thread= thread;
	pos.tid= tid;
	pos.file= "";
	pos.line= 0;
	pos.identif= "";
	pos.time= 0;
	pos.info1= "";
	pos.info2= "";
	pos.ninfo1= -9999;
	pos.ninfo2= -9999;
	LOCK(m_POSITIONSTATUS);
	m_mStatus[tid]= pos;
	UNLOCK(m_POSITIONSTATUS);
#endif // __never_used__
}

void StatusLogRoutine::statusattrib(IStatusLogPattern* thread, string* info1, int* ninfo1)
{
#ifndef __never_used__
	pid_t tid= Thread::gettid();
	map<pid_t, pos_t>::iterator t;

	LOCK(m_POSITIONSTATUS);
	t= m_mStatus.find(tid);
	if(t == m_mStatus.end())
	{
		UNLOCK(m_POSITIONSTATUS);
		return;
	}
	if(info1 != NULL)
		t->second.info1= *info1;
	if(ninfo1 != NULL)
		t->second.ninfo1= *ninfo1;
	if(thread != NULL)
		t->second.thread= thread;
	UNLOCK(m_POSITIONSTATUS);
#endif // __never_used__
}

string StatusLogRoutine::getStatusInfo(string params)
{
	pid_t nshowthread= 0;
	time_t act;
	char stime[20];
	struct tm tm_time;
	string line, oRv;
	string param, sshowthread;
	istringstream iparams(params);
	map<pid_t, pos_t>::iterator it;

	if(	params == "" ||
		params == "text"	)
	{
		ostringstream output;

		if(params == "text")
			output << "on " << GlobalStaticMethods::getProcessName() << " running ";
		LOCK(m_POSITIONSTATUS);
		output << dec << m_mStatus.size();
		UNLOCK(m_POSITIONSTATUS);
		if(params == "text")
			output << " threads";
		output << endl;
		return output.str();

	}else if(params == "pid")
	{
		ostringstream output;

		output << getpid() << endl;
		return output.str();
	}
	time(&act);
	LOCK(m_POSITIONSTATUS);
	while(!iparams.eof())
	{
		iparams >> param;
		if(param.substr(0, 7) == "thread:")
		{
			istringstream ishowthread(param.substr(7));

			sshowthread= ishowthread.str();
			ishowthread >> nshowthread;
			break;
		}
	}
	if(nshowthread != 0)
	{
		it= m_mStatus.find(nshowthread);
		if(it != m_mStatus.end())
		{
			if(localtime_r(&it->second.time, &tm_time) == NULL)
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", &tm_time);
			oRv= it->second.thread->getStatusInfo(params, it->second, (act - it->second.time), stime);
			oRv+= "\n";
		}else
		{
			oRv= "thread [";
			oRv+= sshowthread + "] does not exists\n";
		}
	}else
	{
		for(it= m_mStatus.begin(); it != m_mStatus.end(); ++it)
		{
			if(localtime_r(&it->second.time, &tm_time) == NULL)
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", &tm_time);
			if(it->second.thread)
				line= it->second.thread->getStatusInfo(params, it->second, (act - it->second.time), stime);
			else
				// main-thread having not defined an thread
				line= getStatus(params, it->second, (act - it->second.time), stime);
			if(line != "")
			{
#ifdef SHOWCLIENTSONSHELL
				cout << line << endl;
#endif // SHOWCLIENTSONSHELL
				oRv+= line;
				oRv+= "\n";
			}
		}
	}
	UNLOCK(m_POSITIONSTATUS);
	return oRv;
}

string StatusLogRoutine::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
{
	return getStatus(params, pos, elapsed, lasttime);
}

string StatusLogRoutine::getStatus(string params, pos_t& pos, time_t elapsed, string lasttime)
{
	ostringstream oRv;

	oRv << "[";
	oRv.width(6);
	oRv << dec << pos.tid << "] ";
	if(pos.threadname == "")
	{
		if(pos.tid == getpid())
			oRv << "main-thread";
		else
			oRv << "????";
	}else
		oRv << pos.threadname;

	if(pos.identif == "###condition_wait")
	{
		oRv << " wait for condition " << pos.info2 << " since " << lasttime;

	}else if(pos.identif == "###condition_arouse")
	{
		if(elapsed < 10)
			oRv << " running since";
		else
			oRv << " get now answer since";
		oRv << " aroused condition " << pos.info2 << " " << lasttime;
	}else if(pos.identif == "###mutex_wait")
	{
		oRv << " wait for mutex " << pos.info2 << " since " << lasttime;
	}else if(pos.identif == "###mutex_wait_error")
	{
		oRv << " had error by log mutex " << pos.info2 << " at time " << lasttime;
	}else if(pos.identif == "###mutex_have")
	{
		oRv << " have mutex " << pos.info2 << " since " << lasttime;
	}else if(pos.identif == "###mutex_free")
	{
		oRv << " give mutex " << pos.info2 << " free at time " << lasttime;
	}else if(pos.identif == "###mutex_free_error")
	{
		oRv << " had error by give mutex " << pos.info2 << " free at time " << lasttime;
	}else if(pos.identif == "###mutex_trylock")
	{
		oRv << " check whether an other thread have access to mutex " << pos.info2 << " at time " << lasttime;
	}else if(pos.identif == "###Thread_join")
	{
		oRv << " wait for ending thread " << pos.info2 << " since " << lasttime;
	}else if(pos.identif == "###PROCESS_execute_stop")
	{
		oRv << " process is stopped, running as zombie";
	}else if(pos.identif == "###THREAD_execute_stop")
	{
		oRv << " thread is stopped, running as zombie";

	}else if(pos.identif != "###Thread-removed######")
	{
		if(elapsed < 10)
			oRv << " is running (" << lasttime << ")";
		else
			oRv << " get no answer (since " << lasttime << ")";
		if(pos.identif != "###THREAD_execute_start")
		{
			oRv << "\n         last reaching point on file:" << pos.file << " line:" << pos.line;
		}
	}
	return oRv.str();
}

void StatusLogRoutine::removestatus(pid_t threadid)
{
#ifndef __never_used__
	map<pid_t, pos_t>::iterator del;

	LOCK(m_POSITIONSTATUS);
	del= m_mStatus.find(threadid);
	if(del == m_mStatus.end())
	{
		ostringstream msg;

		UNLOCK(m_POSITIONSTATUS);
		msg << "by removing, cannot found own thread ";
		msg << dec << threadid;
		msg << getStatusInfo("");
		LOG(LOG_ERROR, msg.str());
		msg << endl;
		cerr << msg.str();
	}else
	{
		m_mStatus.erase(del);
		UNLOCK(m_POSITIONSTATUS);
	}
#endif // __never_used__
}

void StatusLogRoutine::setThreadLogName(string threadName, IClientSendMethods* sendDevice)
{
	LogHolderPattern *log= LogHolderPattern::instance();

	if(log)
		log->setThreadName(threadName, sendDevice);
}

StatusLogRoutine::~StatusLogRoutine()
{
}
