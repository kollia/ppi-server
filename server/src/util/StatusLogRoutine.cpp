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

#include "Thread.h"
#include "StatusLogRoutine.h"
#include "../logger/lib/LogInterface.h"

#include "../util/configpropertycasher.h"

using namespace std;
using namespace util;
using namespace logger;

map<pid_t, pos_t> StatusLogRoutine::m_mStatus;
pthread_mutex_t* StatusLogRoutine::m_POSITIONSTATUS= Thread::getMutex("POSITIONSTATUS");

StatusLogRoutine::StatusLogRoutine()
{
}

void StatusLogRoutine::positionA(const string file, const int line, const string identif, const string* info2, const int* ninfo2)
{
	pid_t tid= Thread::gettid();

	//cout << "want to lock from " << file << " line:" << line << endl;
	LOCK(m_POSITIONSTATUS);
	//cout << "be locked from " << file << " line:" << line << endl;
	m_mStatus[tid].tid= tid;
	m_mStatus[tid].file= file;
	m_mStatus[tid].line= line;
	m_mStatus[tid].identif= identif;
	if(info2)
		m_mStatus[tid].info2= *info2;
	if(ninfo2)
		m_mStatus[tid].ninfo2= *ninfo2;
	time(&m_mStatus[tid].time);
	UNLOCK(m_POSITIONSTATUS);

}

void StatusLogRoutine::initstatus(const string threadName, IStatusLogPattern* thread)
{
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
}

void StatusLogRoutine::statusattrib(IStatusLogPattern* thread, string* info1, int* ninfo1)
{
	pos_t pos;
	pid_t tid= Thread::gettid();

	LOCK(m_POSITIONSTATUS);
	pos= m_mStatus[tid];
	if(info1 != NULL)
		pos.info1= *info1;
	if(ninfo1 != NULL)
		pos.ninfo1= *ninfo1;
	if(thread != NULL)
		pos.thread= thread;
	m_mStatus[tid]= pos;
	UNLOCK(m_POSITIONSTATUS);
}

string StatusLogRoutine::getStatusInfo(string params)
{
	pid_t nshowthread= 0;
	time_t act;
	char stime[20];
	struct tm* tm_time;
	string line, oRv;
	string param, sshowthread;
	istringstream iparams(params);
	map<pid_t, pos_t>::iterator it;

	if(params == "")
	{
		ostringstream output;

		LOCK(m_POSITIONSTATUS);
		output << dec << m_mStatus.size() << endl;
		UNLOCK(m_POSITIONSTATUS);
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
			tm_time= localtime(&it->second.time);
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", tm_time);
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
			tm_time= localtime(&it->second.time);
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", tm_time);
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
	}else if(pos.identif == "###Thread_join")
	{
		oRv << " wait for ending thread " << pos.info2 << " since " << lasttime;
	}else if(pos.identif == "###THREAD_execute_stop")
	{
		oRv << " thread is stopped, running as zombie";
	}else
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
	map<pid_t, pos_t>::iterator del;
	map<pid_t, pos_t>::size_type n;

	LOCK(m_POSITIONSTATUS);
	del= m_mStatus.find(threadid);
	if(del != m_mStatus.end())
	{
		n= m_mStatus.size();
		m_mStatus.erase(del);
		n= m_mStatus.size();
	}else
	{
		ostringstream msg;

		msg << "cannot found own thread ";
		msg << dec << threadid;
		msg << getStatusInfo("");
		LOG(LOG_ERROR, msg.str());
		msg << endl;
		cerr << msg.str();
	}
	UNLOCK(m_POSITIONSTATUS);
}

void StatusLogRoutine::setThreadLogName(string threadName)
{
	LogInterface *log= LogInterface::instance();

	if(log)
		log->setThreadName(threadName);
}

StatusLogRoutine::~StatusLogRoutine()
{
}
