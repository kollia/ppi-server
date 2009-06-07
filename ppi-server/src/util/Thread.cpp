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
#include "../logger/LogThread.h"

#include "../util/configpropertycasher.h"

using namespace std;
using namespace util;


pthread_mutex_t g_READMUTEX;
map<pthread_mutex_t*, mutexnames_t> g_mMutex;
map<pthread_cond_t*, string> g_mCondition;
map<pid_t, pos_t> Thread::m_mStatus;
pthread_mutex_t* Thread::m_POSITIONSTATUS= Thread::getMutex("POSITIONSTATUS");

Thread::Thread(string threadName, useconds_t defaultSleep, bool waitInit)
{
	m_nThreadId= 0;
	m_nPosixThreadID= 0;
	m_bRun= false;
	m_bStop= false;
	m_sThreadName= threadName;
	m_nDefaultSleep= defaultSleep;
	m_bWaitInit= waitInit;
	m_RUNTHREAD= getMutex("RUNTHREAD");
	m_THREADNAME= getMutex("THREADNAME");
	m_STOPTHREAD= getMutex("STOPTHREAD");
	m_STARTSTOPTHREAD= getMutex("STARTSTOPTHREAD");
	m_STARTSTOPTHREADCOND= getCondition("STARTSTOPTHREADCOND");
}

void *Thread::start(void *args, bool bHold)
{
	int nRv= 0;
	void *Rv;
	string threadName(getThreadName());
	pthread_attr_t attr;

#ifdef SINGLETHREADING
	run();
	return NULL;
#else // SINGLETHREADING



	m_bHold= bHold;
	m_pArgs= args;
	LOCK(m_STOPTHREAD);
	m_bStop= false;
	UNLOCK(m_STOPTHREAD);
	pthread_attr_init(&attr);
	nRv= pthread_create (&m_nPosixThreadID, &attr, Thread::EntryPoint, this);

	if(nRv != 0)
	{
		LOG(AKALERT, "Error by creating thread " + threadName + "\n-> does not start thread");
		return NULL;
	}
	if(bHold)
	{
		POSS("###Thread_join", getThreadName());
		nRv= pthread_join(m_nPosixThreadID, &Rv);
		if(nRv != 0)
		{
			LOG(AKALERT, "ERROR: cannot join correctly to thread " + threadName);
		}
		return Rv;
	}else
	{
		LOCK(m_STARTSTOPTHREAD);
		while(!running())
		{
#ifdef DEBUG
			cout << "." << flush;
#endif
			if(stopping())
			{// an error is occured in init methode
				nRv= pthread_join(m_nPosixThreadID, &Rv);
				if(nRv != 0)
				{
					LOG(AKALERT, "ERROR: cannot join correctly to thread " + threadName);
				}
				return Rv;
			}
			CONDITION(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD);
		}
		UNLOCK(m_STARTSTOPTHREAD);
#ifdef DEBUG
		cout << endl;
#endif
	}
	detach();
	return NULL;
#endif // else SUNGLETHREADING
}

void Thread::run()
{
	string error("undefined error by ");
	string thname(getThreadName());
	string startmsg("starting thread with name '");
	pos_t pos;

	m_nThreadId= gettid();
	initstatus(getThreadName(), this);
	startmsg+= thname + "'";
	LOG(AKDEBUG, startmsg);
#ifdef SERVERDEBUG
	cout << startmsg << endl;
#endif // SERVERDEBUG
	setThreadLogName(thname);
	try{

		if(!m_bWaitInit)
		{
			LOCK(m_RUNTHREAD);
			m_bRun= true;
			UNLOCK(m_RUNTHREAD);
		}
		LOCK(m_STARTSTOPTHREAD);
		if(init(m_pArgs))
		{
			LOCK(m_RUNTHREAD);
			m_bRun= true;
			UNLOCK(m_RUNTHREAD);
		}else
		{
			error+= "### thread ";
			error+= thname;
			error+= " cannot inital correcty";
			LOG(AKERROR, error);
			LOCK(m_STOPTHREAD);
			m_bStop= true;
			UNLOCK(m_STOPTHREAD);
			LOCK(m_RUNTHREAD);
			m_bRun= false;
			UNLOCK(m_RUNTHREAD);
		}
		AROUSE(m_STARTSTOPTHREADCOND);
		UNLOCK(m_STARTSTOPTHREAD);
		if(running())
		{
			try{
				while(!stopping())
				{
					POS("###THREAD_execute_start");
					execute();
				}
			}catch(...)
			{
				error+= "execute thread ";
				error+= thname;
				LOG(AKALERT, error);
				cerr << error << endl;
			}
		}
	}catch(...)
	{
		error+= "initialisation on thread ";
		error+= thname;
		LOG(AKALERT, error);
		cerr << error << endl;
	}
	ending();

#ifndef SINGLETHREADING

	string msg("thread ");

	msg+= getThreadName() + " do stopping";
	if(!LogThread::instance()->stopping())
		LOG(AKDEBUG, msg);
#ifdef DEBUG
	cout << msg << endl;
#endif
	//}
#endif // SINGLETHREADING

	LOCK(m_RUNTHREAD);
	m_bRun= false;
	UNLOCK(m_RUNTHREAD);
	POS("###THREAD_execute_stop");
	LOCK(m_STARTSTOPTHREAD);
	AROUSE(m_STARTSTOPTHREADCOND);
	UNLOCK(m_STARTSTOPTHREAD);
}

void Thread::positionA(const string file, const int line, const string identif, const string* info2, const int* ninfo2)
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

void Thread::initstatus(const string threadName, IStatusLogPattern* thread)
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

void Thread::statusattrib(IStatusLogPattern* thread, string* info1, int* ninfo1)
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

string Thread::getStatusInfo(string params)
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

		output << "ppi-server running with ";
		LOCK(m_POSITIONSTATUS);
		output << dec << m_mStatus.size() << " threads" << endl;
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

string Thread::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
{
	return getStatus(params, pos, elapsed, lasttime);
}

string Thread::getStatus(string params, pos_t& pos, time_t elapsed, string lasttime)
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

void Thread::removestatus(pid_t threadid)
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
		LOG(AKERROR, msg.str());
		msg << endl;
		cerr << msg.str();
	}
	UNLOCK(m_POSITIONSTATUS);
}

/*static */
void *Thread::EntryPoint(void *pthis)
{
	Thread *pt = (Thread*)pthis;
	pt->run();
	pthread_exit(NULL);
}

pid_t Thread::gettid()
{
	return (pid_t) syscall(SYS_gettid);
}

pthread_mutex_t* Thread::getMutex(string name)
{
	int result;
	pthread_mutex_t *mutex= new pthread_mutex_t;
	mutexnames_t tName;

#ifdef MUTEXLOCKDEBUG
	bool bSet= false;
	vector<string> split;

	split= ConfigPropertyCasher::split(MUTEXLOCKDEBUG, " ");
	if(split.size() == 0)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == name)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
		cout << "create mutex " << name << " on thread (" << dec << gettid() << ")" << endl;
#endif
	result= pthread_mutex_init(mutex, NULL);
	if(result != 0)
	{
		char msg[40];

		sprintf(msg, "ERROR:: cannot lock mutex -> error:%d", result);
		LOG(AKALERT, msg);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(AKERROR, "error by mutex lock " + getMutexName(mutex));
		}

		tName.name= name;
		tName.threadid= 0;
		g_mMutex[mutex]= tName;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(AKERROR, "error by mutex unlock " + getMutexName(mutex));
		}
	}
	return mutex;
}

pthread_cond_t* Thread::getCondition(string name)
{
	int result;
	pthread_cond_t *cond= new pthread_cond_t;

#ifdef CONDITIONSDEBUG
	bool bSet= false;
	vector<string> split;

	split= ConfigPropertyCasher::split(CONDITIONSDEBUG, " ");
	if(split.size() == 0)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == name)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
		cout << "create condition " << name << " on thread (" << dec << gettid() << ")" << endl;
#endif
	result= pthread_cond_init(cond, NULL);
	if(result != 0)
	{
		char msg[40];

		sprintf(msg, "ERROR:: cannot lock mutex -> error:%d", result);
		LOG(AKALERT, msg);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(AKERROR, "error by mutex lock " + getConditionName(cond));
		}

		g_mCondition[cond]= name;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(AKERROR, "error by mutex unlock " + getConditionName(cond));
		}
	}
	return cond;
}

string Thread::getMutexName(pthread_mutex_t* mutex)
{
	//bool bFound= false;
	string name;
	int error;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex lock READMUTEX by get name");
		return "unknown";
	}
	i= g_mMutex.find(mutex);
	if(i != g_mMutex.end())
		name= i->second.name;
	else
		name= "ERROR: undefined mutex";
	/*for(iter i= g_mMutex.begin(); i!=g_mMutex.end(); ++i)
	{
		//if(i->second == mutex)
		{
			name= i->first;
			bFound= true;
			break;
		}
	}*/
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex unlock READMUTEX by get name");
	}


	return name;
}

string Thread::getConditionName(pthread_cond_t *cond)
{
	string name;
	int error;
	typedef map<pthread_cond_t*, string>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex lock READMUTEX by get condition name");
		return "unknown";
	}
	i= g_mCondition.find(cond);
	if(i != g_mCondition.end())
		name= i->second;
	else
		name= "ERROR: undefined condition";
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex unlock READMUTEX by get condition name");
	}
	return name;
}

int Thread::mutex_lock(string file, int line, pthread_mutex_t *mutex)
{
	int error;

#ifdef MUTEXLOCKDEBUG
	bool bSet= false;
	string mutexname(getMutexName(mutex));
	ostringstream before, behind;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	split= ConfigPropertyCasher::split(MUTEXLOCKDEBUG, " ");
	if(	split.size() == 0
		||
		mutexname == "ERROR: undefined mutex"	)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == mutexname)
			{
				bSet= true;
				break;
			}else if(	*it == "undefined"
						&&
						mutexname == "ERROR: undefined mutex"	)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "want to lock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
#endif

	error= pthread_mutex_lock(mutex);
	if(error != 0)
	{
		string msg("error by mutex lock ");

		msg+= getMutexName(mutex);
		LOG(AKERROR, msg);
#ifdef MUTEXLOCKDEBUG
		cerr << msg << endl;
#endif // MUTEXLOCKDEBUG
	}
#ifdef MUTEXLOCKDEBUG
	if(error == 0)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex.find(mutex);
		if(i != g_mMutex.end())
			i->second.threadid= gettid();
		pthread_mutex_unlock(&g_READMUTEX);
		if(bSet)
		{
			behind << "[";
			behind.fill(' ');
			behind.width(5);
			behind << dec << gettid() << "] ";
			behind << "mutex " << mutexname << "  be locked on file:" << file << " line:" << line  << endl;
			cout << behind.str();
		}
	}else if(bSet)
		cout << "mutex " << mutexname << " cannot lock by thread(" << dec << gettid() << ") an ERROR occured" << endl;
#endif // MUTEXLOCKDEBUG
	return error;
}

int Thread::mutex_trylock(string file, int line, pthread_mutex_t *mutex)
{
	int error;

#ifdef MUTEXLOCKDEBUG
	bool bSet= false;
	string mutexname(getMutexName(mutex));
	ostringstream before, behind;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	split= ConfigPropertyCasher::split(MUTEXLOCKDEBUG, " ");
	if(	split.size() == 0
		||
		mutexname == "ERROR: undefined mutex"	)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == mutexname)
			{
				bSet= true;
				break;
			}else if(	*it == "undefined"
						&&
						mutexname == "ERROR: undefined mutex"	)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "try to lock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
#endif

	error= pthread_mutex_lock(mutex);
	if(	error != 0
		&&
		error != EBUSY	)
	{
		LOG(AKERROR, "error by try to lock mutex " + getMutexName(mutex));
	}
#ifdef MUTEXLOCKDEBUG
	if(error == 0)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex.find(mutex);
		if(i != g_mMutex.end())
			i->second.threadid= gettid();
		pthread_mutex_unlock(&g_READMUTEX);
		if(bSet)
		{
			behind << "[";
			behind.fill(' ');
			behind.width(5);
			behind << dec << gettid() << "] ";
			behind << "mutex " << mutexname << "  be locked on file:" << file << " line:" << line  << endl;
			cout << behind.str();
		}
	}else if(	bSet
				&&
				error == EBUSY)
	{
		behind << "[";
		behind.fill(' ');
		behind.width(5);
		behind << dec << gettid() << "] ";
		behind << "mutex " << mutexname << "  is busy do not lock on file:" << file << " line:" << line  << endl;
		cout << behind.str();
	}
#endif // MUTEXLOCKDEBUG
	return error;
}

int Thread::mutex_unlock(string file, int line, pthread_mutex_t *mutex)
{
	int error;

#ifdef MUTEXLOCKDEBUG
	bool bSet= false;
	string mutexname(getMutexName(mutex));
	ostringstream before;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	split= ConfigPropertyCasher::split(MUTEXLOCKDEBUG, " ");
	if(	split.size() == 0
		||
		mutexname == "ERROR: undefined mutex"	)
		{
			bSet= true;
		}else
		{
			for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
			{
				if(*it == mutexname)
				{
					bSet= true;
					break;
				}else if(	*it == "undefined"
							&&
							mutexname == "ERROR: undefined mutex"	)
				{
					bSet= true;
					break;
				}
			}
		}
	pthread_mutex_lock(&g_READMUTEX);
	i= g_mMutex.find(mutex);
	if(i != g_mMutex.end())
		i->second.threadid= 0;
	pthread_mutex_unlock(&g_READMUTEX);
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "unlock mutex " << getMutexName(mutex) << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
#endif

	error= pthread_mutex_unlock(mutex);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex unlock " + getMutexName(mutex));
	}
	return error;
}

void Thread::destroyMutex(string file, int line, pthread_mutex_t* mutex)
{
#ifdef MUTEXCREATEDEBUG
	bool bSet= false;
	string mutexname(getMutexName(mutex));
	vector<string> split;

	split= ConfigPropertyCasher::split(MUTEXCREATEDEBUG, " ");
	if(	split.size() == 0
		||
		mutexname == "ERROR: undefined mutex"	)
		{
			bSet= true;
		}else
		{
			for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
			{
				if(*it == mutexname)
				{
					bSet= true;
					break;
				}else if(	*it == "undefined"
							&&
							mutexname == "ERROR: undefined mutex"	)
				{
					bSet= true;
					break;
				}
			}
		}
	if(bSet)
		cout << "destroy mutex " << getMutexName(mutex) << " on thread (" << dec << gettid() << ")" << endl;
#endif

	int error;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex lock READMUTEX by destroy");
		return;
	}
	i= g_mMutex.find(mutex);
	if(i != g_mMutex.end()) // erase mutex from map
		g_mMutex.erase(i);
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex unlock READMUTEX by destroy");
	}
	pthread_mutex_destroy(mutex);
}

void Thread::destroyAllMutex()
{
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;

	LOCK(&g_READMUTEX);
	for(iter i= g_mMutex.begin(); i!=g_mMutex.end(); ++i)
	{
		g_mMutex.erase(i->first);
		pthread_mutex_destroy(i->first);
		delete i->first;
	}
	UNLOCK(&g_READMUTEX);
	// toDo: do not take READMUTEX for mutex lock and conditions
	//pthread_mutex_destroy(&g_READMUTEX);
}

void Thread::destroyCondition(string file, int line, pthread_cond_t *cond)
{
#ifdef MUTEXCREATEDEBUG
	bool bSet= false;
	string mutexname(getConditionName(cond));
	vector<string> split;

	split= ConfigPropertyCasher::split(MUTEXCREATEDEBUG, " ");
	if(	split.size() == 0
		||
		mutexname == "ERROR: undefined condition"	)
		{
			bSet= true;
		}else
		{
			for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
			{
				if(*it == mutexname)
				{
					bSet= true;
					break;
				}else if(	*it == "undefined"
							&&
							mutexname == "ERROR: undefined condition"	)
				{
					bSet= true;
					break;
				}
			}
		}
	if(bSet)
		cout << "destroy condition " << mutexname << " on thread (" << dec << gettid() << ")" << endl;
#endif

	int error;
	typedef map<pthread_cond_t*, string>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex lock READMUTEX by destroy condition");
		return;
	}
	i= g_mCondition.find(cond);
	if(i != g_mCondition.end()) // erase mutex from map
		g_mCondition.erase(i);
	//conderror= pthread_cond_in
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(AKERROR, "error by mutex unlock READMUTEX by destroy condition");
	}
	pthread_cond_destroy(cond);
}

void Thread::destroyAllConditions()
{
	typedef map<pthread_cond_t*, string>::iterator iter;

	LOCK(&g_READMUTEX);
	for(iter i= g_mCondition.begin(); i!=g_mCondition.end(); ++i)
	{
		g_mCondition.erase(i->first);
		pthread_cond_destroy(i->first);
		delete i->first;
	}
	UNLOCK(&g_READMUTEX);
}

int Thread::conditionWait(string file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec *time, const bool absolute)
{
	int retcode;
	string condname(getConditionName(cond));
	timespec tabsolute;

#ifdef CONDITIONSDEBUG
	bool bSet= false;
	ostringstream msg;
	string mutexname(getMutexName(mutex));
	ostringstream before, behind;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	split= ConfigPropertyCasher::split(CONDITIONSDEBUG, " ");
	if(	split.size() == 0
		||
		condname == "ERROR: undefined condition"	)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == condname)
			{
				bSet= true;
				break;
			}else if(	*it == "undefined"
						&&
						condname == "ERROR: undefined condition"	)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "wait for condition " << condname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
	pthread_mutex_lock(&g_READMUTEX);
	i= g_mMutex.find(mutex);
	if(	i == g_mMutex.end()
		||
		i->second.threadid == 0	)
	{
		if(i == g_mMutex.end())
			msg << "ERROR: mutex " << mutexname << " not found" << endl;
		msg << "ERROR: in thread (" << gettid() << ")";
		msg << " mutex " << mutexname << " for condition " << condname << endl;
		msg << "   on LINE: " << dec << line << " from FILE:" << file << endl;
		msg << "   is not locked" << endl;
		cerr << msg.str();
		LOG(AKERROR, msg.str());
	}
	pthread_mutex_unlock(&g_READMUTEX);
#endif // CONDITIONSDEBUG

	POSS("###condition_wait", condname);
	if(time)
	{
		if(absolute)
		{
			tabsolute.tv_sec= time->tv_sec;
			tabsolute.tv_nsec= time->tv_nsec;
		}else
		{
			clock_gettime(CLOCK_REALTIME, &tabsolute);
			tabsolute.tv_sec+= time->tv_sec;
			tabsolute.tv_nsec+= time->tv_nsec;
		}
		retcode= pthread_cond_timedwait(cond, mutex, &tabsolute);
	}else
		retcode= pthread_cond_wait(cond, mutex);
	POSS("###condition_arouse", condname);

	if(	retcode != 0
		&&
		retcode != ETIMEDOUT)
	{
		ostringstream msg;
		ostringstream t;

		msg << "ERROR: by waiting for ";
		t << "condwait";
		if(time)
		{
			msg << "timed ";
			t << "timed";
		}
		t << dec << getgid() << errno;
		msg << "condition ";
		msg << getConditionName(cond) << " in mutex area ";
		msg << getMutexName(mutex);
		if(time)
		{
			msg << " with limit of ";
			msg << dec << time->tv_sec << " seconds and ";
			msg << dec << time->tv_nsec << " microseconds";
		}
		msg << "\n       ";
		msg << "RETURNCODE(" << dec << retcode << ":";
		if(retcode == ETIMEDOUT) // this first return code never can reached, is only for documentation
			msg << "ETIMEDOUT) the specified time for condition has passed ";
		else if(retcode == EINVAL)
			msg << "EINVAL) one parameter value is invalid or different mutexes were supplied for concurrent operations on the same condition variable ";
		else if(retcode == EPERM)
			msg << "EPERM) The mutex was not owned by the current thread at the time of the call ";
		else if(retcode == EINTR)
			msg << "EINTR) condition was interrupted by an signal";
		else msg << "UNKNOWN) unknown return code ";
		TIMELOG(AKERROR, t.str(), msg.str());
#ifdef CONDITIONSDEBUG
		cerr << msg.str() << endl;
#endif //CONDITIONSDEBUG
	}
#ifdef CONDITIONSDEBUG
	if(	bSet
		&&
		retcode == 0	)
	{
		behind << "[";
		behind.fill(' ');
		behind.width(5);
		behind << dec << gettid() << "] ";
		behind << "condition " << condname << " is aroused on line:" << dec << line << endl;
		cout << behind.str();

	}else if(	bSet
				&&
				retcode != ETIMEDOUT	)
	{
		behind << "[";
		behind.fill(' ');
		behind.width(5);
		behind << dec << gettid() << "] ";
		if(time->tv_sec != 0)
		{
			behind << dec << time->tv_sec << " second ";
			if(time->tv_nsec != 0)
				behind << " and ";
		}
		if(time->tv_nsec != 0)
			behind << dec << time->tv_nsec << " nanoseconds ";
		behind << " for condition " << condname << " be passed on line:" << dec << line << endl;
		cout << behind.str();

	}
#endif //CONDITIONSDEBUG
	return retcode;
}

int Thread::arouseCondition(string file, int line, pthread_cond_t *cond)
{
	int error;
#ifdef CONDITIONSDEBUG
	bool bSet= false;
	string condname(getConditionName(cond));
	ostringstream before;
	vector<string> split;

	split= ConfigPropertyCasher::split(CONDITIONSDEBUG, " ");
	if(	split.size() == 0
		||
		condname == "ERROR: undefined condition"	)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == condname)
			{
				bSet= true;
				break;
			}else if(	*it == "undefined"
						&&
						condname == "ERROR: undefined condition"	)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "arouse one or more condition " << condname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
#endif // CONDITIONSDEBUG


	error= pthread_cond_signal(cond);
	if(error != 0)
	{
		string msg("ERROR: cannot arouse the condition ");

		msg+= getConditionName(cond) + "\n       ";
		msg+= strerror(errno);
		LOG(AKERROR, msg);
	}
	return error;
}

int Thread::arouseAllCondition(string file, int line, pthread_cond_t *cond)
{
	int error;
#ifdef CONDITIONSDEBUG
	bool bSet= false;
	string condname(getConditionName(cond));
	ostringstream before;
	vector<string> split;

	split= ConfigPropertyCasher::split(CONDITIONSDEBUG, " ");
	if(	split.size() == 0
		||
		condname == "ERROR: undefined condition"	)
	{
		bSet= true;
	}else
	{
		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == condname)
			{
				bSet= true;
				break;
			}else if(	*it == "undefined"
						&&
						condname == "ERROR: undefined condition"	)
			{
				bSet= true;
				break;
			}
		}
	}
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "send broudcast for all conditions from " << condname << " on line:" << line << endl;
		cout << before.str();
	}
#endif // CONDITIONSDEBUG

	error= pthread_cond_broadcast(cond);
	if(error != 0)
	{
		string msg("ERROR: cannot arouse all condition from ");

		msg+= getConditionName(cond) + "\n       ";
		msg+= strerror(errno);
		LOG(AKERROR, msg);
	}
	return error;
}

int Thread::detach()
{
	int nRv= pthread_detach(m_nPosixThreadID);

	if(nRv != 0)
	{
		string msg("### ERROR: cannot detach thread ");

		msg+= getThreadName() + "\n           ";
		msg+= strerror(nRv);
		if(LogThread::instance()->running())
			LOG(AKERROR, msg);
#ifdef DEBUG
		cerr << msg << endl;
#endif
	}
	return nRv;
}

void *Thread::stop(const bool *bWait)
{
#ifndef SINGLETHREADING
	int nRv;
	void *Rv= NULL;
	LogThread *log= LogThread::instance();

	LOCK(m_STARTSTOPTHREAD);
	LOCK(m_STOPTHREAD);
	m_bStop= true;
	UNLOCK(m_STOPTHREAD);
	if(	bWait
		&&
		*bWait	)
	{
		if(!running())
			return NULL;
		if(!log->ownThread(getThreadName(), pthread_self()))
		{
			nRv= CONDITION(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD);
			if(nRv != 0)
			{
				string msg("### fatal ERROR: cannot join correctly to thread ");

				msg+= getThreadName() + "\n                 ";
				msg+= strerror(nRv);
#ifdef DEBUG
				cerr << msg << endl;
#endif // DEBUG
				LOG(AKERROR,  msg);
			}
		}else
			LOG(AKERROR, "application cannot stop own thread with stop(true)");
	}
	UNLOCK(m_STARTSTOPTHREAD);

#endif // SINGLETHREADING
	return Rv;
}

string Thread::getThreadName()
{
	string name;

	LOCK(m_THREADNAME);
	name= m_sThreadName;
	UNLOCK(m_THREADNAME);
	return name;
}

void Thread::setThreadLogName(string threadName)
{
	LogThread *log= LogThread::instance();

	log->setThreadName(threadName);
}

bool Thread::stopping()
{
	bool bRv;

	LOCK(m_STOPTHREAD);
	bRv= m_bStop;
	UNLOCK(m_STOPTHREAD);
	return bRv;
}

bool Thread::running()
{
	LOCK(m_RUNTHREAD);
	if(m_bRun)
	{
		UNLOCK(m_RUNTHREAD);
		return true;
	}
	UNLOCK(m_RUNTHREAD);
	return false;
}

Thread::~Thread()
{
	if(m_nThreadId != 0)
		removestatus(m_nThreadId);
	DESTROYMUTEX(m_RUNTHREAD);
	DESTROYMUTEX(m_THREADNAME);
	DESTROYMUTEX(m_STOPTHREAD);
	DESTROYMUTEX(m_STARTSTOPTHREAD);
	DESTROYCOND(m_STARTSTOPTHREADCOND);
}
