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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

#include "Thread.h"

#include "../GlobalStaticMethods.h"
#include "../properties/configpropertycasher.h"

#include "../../pattern/util/LogHolderPattern.h"

using namespace std;
using namespace util;
//using namespace logger;


bool Thread::m_bAppRun= true;
pthread_mutex_t g_READMUTEX;
map<pthread_mutex_t*, mutexnames_t> g_mMutex;
map<pthread_cond_t*, string> g_mCondition;

Thread::Thread(string threadName, useconds_t defaultSleep, bool waitInit)
{
	m_nThreadId= 0;
	m_nPosixThreadID= 0;
	m_bRun= false;
	m_bInitialed= false;
	m_bStop= false;
	m_nEndValue= 0;
	m_sThreadName= threadName;
	m_nDefaultSleep= defaultSleep;
	m_bWaitInit= waitInit;
	m_RUNTHREAD= getMutex("RUNTHREAD");
	m_THREADNAME= getMutex("THREADNAME");
	m_STARTSTOPTHREAD= getMutex("STARTSTOPTHREAD");
	m_STARTSTOPTHREADCOND= getCondition("STARTSTOPTHREADCOND");
}

int Thread::start(void *args, bool bHold)
{
	int nRv= 0;
	string threadName(getThreadName());
	pthread_attr_t attr;

#ifdef SINGLETHREADING
	run();
	return NULL;
#else // SINGLETHREADING



	m_bHold= bHold;
	m_pArgs= args;
	LOCK(m_STARTSTOPTHREAD);
	m_bStop= false;
	UNLOCK(m_STARTSTOPTHREAD);
	pthread_attr_init(&attr);
	nRv= pthread_create (&m_nPosixThreadID, &attr, Thread::EntryPoint, this);

	if(nRv != 0)
	{
		LOG(LOG_ALERT, "Error by creating thread " + threadName + "\n-> does not start thread");
		return 2;
	}
	if(bHold)
	{
		POSS("###Thread_join", getThreadName());
		nRv= pthread_join(m_nPosixThreadID, NULL);
		if(nRv != 0)
		{
			LOG(LOG_ALERT, "ERROR: cannot join correctly to thread " + threadName);
			return 3;
		}
		return 0;
	}else
	{
		if(m_bWaitInit)
		{
			LOCK(m_STARTSTOPTHREAD);
			while(!running() && !m_bStop)
			{
				if(m_bStop)
				{// an error is occured in init methode
					nRv= pthread_join(m_nPosixThreadID, NULL);
					if(nRv != 0)
					{
						LOG(LOG_ALERT, "ERROR: cannot join correctly to thread " + threadName);
					}
					UNLOCK(m_STARTSTOPTHREAD);
					return 4;
				}
				CONDITION(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD);
			}
			UNLOCK(m_STARTSTOPTHREAD);
		}
		//detach();
#ifdef DEBUG
		cout << endl;
#endif
	}
	return m_nEndValue;
#endif // else SUNGLETHREADING
}

void Thread::run()
{
	LogHolderPattern* logObj;
	string error("undefined error by ");
	string thname(getThreadName());
	string startmsg("starting thread with name '");
	pos_t pos;
	int err;

	m_nThreadId= gettid();
	initstatus(getThreadName(), this);
	startmsg+= thname + "'";
	logObj= LogHolderPattern::instance();
	logObj->setThreadName(thname);
	logObj->log(__FILE__, __LINE__, 0, startmsg, "");
#ifdef SERVERDEBUG
	cout << startmsg << endl;
#endif // SERVERDEBUG
	setThreadLogName(thname);
	try{

		LOCK(m_RUNTHREAD);
		m_bRun= true;
		m_bInitialed= false;
		UNLOCK(m_RUNTHREAD);
		LOCK(m_STARTSTOPTHREAD);
		err= init(m_pArgs);
		if(err == 0)
		{
			LOCK(m_RUNTHREAD);
			m_bInitialed= true;
			UNLOCK(m_RUNTHREAD);
		}else
		{
			m_nEndValue= err;
			error+= "### thread ";
			error+= thname;
			error+= " cannot inital correcty";
			LOG(LOG_ERROR, error);
			m_bStop= true;
			LOCK(m_RUNTHREAD);
			m_bRun= false;
			m_bInitialed= false;
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
				LOG(LOG_ALERT, error);
				cerr << error << endl;
			}
		}
	}catch(...)
	{
		error+= "initialisation on thread ";
		error+= thname;
		LOG(LOG_ALERT, error);
		cerr << error << endl;
	}
	ending();

	glob::threadStopMessage("Thread::run(): running thread of '" + getThreadName() + "' was reaching end and will be destroy");
	removestatus(m_nThreadId);
}

/*static */
void *Thread::EntryPoint(void *pthis)
{
	Thread *pt = (Thread*)pthis;
	pt->run();
	LOCK(pt->m_RUNTHREAD);
	pt->m_bRun= false;
	UNLOCK(pt->m_RUNTHREAD);
	//POS("###THREAD_execute_stop");
	LOCK(pt->m_STARTSTOPTHREAD);
	AROUSE(pt->m_STARTSTOPTHREADCOND);
	UNLOCK(pt->m_STARTSTOPTHREAD);
	return NULL;
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
	static map<pthread_mutex_t*, mutexnames_t> mMutexBuffer;

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

		sprintf(msg, "ERROR:: cannot create lock mutex -> error:%d", result);
		LOG(LOG_ALERT, msg);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(LOG_ERROR, "error by global mutex lock, locking by " + getMutexName(mutex));
		}

		tName.name= name;
		tName.threadid= 0;
		if(name != "POSITIONSTATUS")
		{
			if(mMutexBuffer.size() > 0)
			{// first mutex creation of global mutex POSITIONSTATUS makes error
			 // since gcc (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
			 //       g++ (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
			 // so write mutex for first time in an buffer
			 // (gcc (Debian 4.4.5-8) 4.4.5 made no problem's)
				for(map<pthread_mutex_t*, mutexnames_t>::iterator it= mMutexBuffer.begin(); it != mMutexBuffer.end(); ++it)
					g_mMutex[it->first]= it->second;
				mMutexBuffer.clear();
			}
			g_mMutex[mutex]= tName;

		}else
			mMutexBuffer[mutex]= tName;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(LOG_ERROR, "error by global mutex unlock, by locking for " + getMutexName(mutex));
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
		LOG(LOG_ALERT, msg);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(LOG_ERROR, "error by mutex lock " + getConditionName(cond));
		}

		g_mCondition[cond]= name;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{
			LOG(LOG_ERROR, "error by mutex unlock " + getConditionName(cond));
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
		LOG(LOG_ERROR, "error by mutex lock READMUTEX by get name");
		return "unknown";
	}
	i= g_mMutex.find(mutex);
	if(i != g_mMutex.end())
	{
		mutexnames_t mutexnames;

		mutexnames= i->second;
		name= mutexnames.name;
		//name= i->second.name;
	}else
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
		LOG(LOG_ERROR, "error by mutex unlock READMUTEX by get name");
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
		LOG(LOG_ERROR, "error by mutex lock READMUTEX by get condition name");
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
		LOG(LOG_ERROR, "error by mutex unlock READMUTEX by get condition name");
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
	pid_t lastlockID;
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
	{
		lastlockID= i->second.threadid;
		if(lastlockID == gettid())
		{
			ostringstream msg;

			msg << "[";
			msg.fill(' ');
			msg.width(5);
			msg << dec << gettid() << "] ";
			msg << "WARNING: thread lock's mutex " << mutexname << " again, on file:" << file << " line:" << line << endl;
			cout << msg.str();
		}
	}else
		lastlockID= 0;
	pthread_mutex_unlock(&g_READMUTEX);
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "want to lock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
		if(lastlockID != 0)
		{
			before.fill(' ');
			before.width(8);
			before << " ";
			before << "but mutex was locked from thread " << lastlockID << endl;
		}
		cout << before.str();
	}
	if(mutexname != "POSITIONSTATUS")
		POSS("###mutex_wait", mutexname);
#endif

	error= pthread_mutex_lock(mutex);
	if(error != 0)
	{
		string msg("error by mutex lock ");

		msg+= getMutexName(mutex);
		LOG(LOG_ERROR, msg);
#ifdef MUTEXLOCKDEBUG
		ostringstream thid;

		thid << "[";
		thid.fill(' ');
		thid.width(5);
		thid << dec << gettid() << "] ";
		thid << msg << endl;
		cerr << thid.str();
		if(mutexname != "POSITIONSTATUS")
			POSS("###mutex_wait_error", mutexname);
#endif // MUTEXLOCKDEBUG
	}
#ifdef MUTEXLOCKDEBUG
	if(error == 0)
	{
		if(mutexname != "POSITIONSTATUS")
			POSS("###mutex_have", mutexname);
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
	if(mutexname != "POSITIONSTATUS")
		POSS("###mutex_trylock", mutexname);
#endif

	error= pthread_mutex_trylock(mutex);
	if(	error != 0
		&&
		error != EBUSY	)
	{
		LOG(LOG_ERROR, "error by try to lock mutex " + getMutexName(mutex));
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
	pid_t tid= gettid();

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
	{
		if(i->second.threadid != gettid())
		{
			before << "[";
			before.fill(' ');
			before.width(5);
			before << dec << tid << "] ";
			before << "WARNING: thread " << tid << " want to unlock mutex " << mutexname;
			if(i->second.threadid == 0)
				before << " witch isn't locked from any thread" << endl;
			else
				before << " witch is locked from other thread " << i->second.threadid << endl;
			before <<"        on file:" << file << " line:" << line << endl;
			cout << before.str();
		}
		i->second.threadid= 0;
	}
	pthread_mutex_unlock(&g_READMUTEX);
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "unlock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
#endif

	error= pthread_mutex_unlock(mutex);
	if(error != 0)
	{
		string msg("error by unlock mutex ");

		msg+= getMutexName(mutex);
		LOG(LOG_ERROR, msg);
#ifdef MUTEXLOCKDEBUG
		ostringstream thid;

		thid << "[";
		thid.fill(' ');
		thid.width(5);
		thid << dec << gettid() << "] ";
		thid << msg << endl;
		cerr << thid.str();
		if(mutexname != "POSITIONSTATUS")
			POSS("###mutex_free_error", mutexname);
#endif // MUTEXLOCKDEBUG
	}
#ifdef MUTEXLOCKDEBUG
	else
		if(mutexname != "POSITIONSTATUS")
			POSS("###mutex_free", mutexname);
#endif // MUTEXLOCKDEBUG
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
		cout << "destroy mutex " << mutexname << " on thread (" << dec << gettid() << ")" << endl;
#endif

	int error;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(LOG_ERROR, "error by mutex lock READMUTEX by destroy");
		return;
	}
	if(m_bAppRun)
	{
		i= g_mMutex.find(mutex);
		if(i != g_mMutex.end()) // erase mutex from map
			g_mMutex.erase(i);
	}
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(LOG_ERROR, "error by mutex unlock READMUTEX by destroy");
	}
	pthread_mutex_destroy(mutex);
	delete mutex;
}

void Thread::applicationStops()
{
	pthread_mutex_lock(&g_READMUTEX);
	m_bAppRun= false;
	pthread_mutex_unlock(&g_READMUTEX);
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
		LOG(LOG_ERROR, "error by mutex lock READMUTEX by destroy condition");
		return;
	}
	if(m_bAppRun)
	{
		i= g_mCondition.find(cond);
		if(i != g_mCondition.end()) // erase mutex from map
			g_mCondition.erase(i);
	}
	//conderror= pthread_cond_in
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOG(LOG_ERROR, "error by mutex unlock READMUTEX by destroy condition");
	}
	pthread_cond_destroy(cond);
	delete cond;
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

int Thread::conditionWait(string file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex, const time_t sec, const bool absolute)
{
	timespec time;

	time.tv_sec= sec;
	time.tv_nsec= 0;
	return conditionWait(file, line, cond, mutex, &time, absolute);
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
		LOG(LOG_ERROR, msg.str());
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
		TIMELOG(LOG_ERROR, t.str(), msg.str());
#ifdef CONDITIONSDEBUG
		cerr << msg.str() << endl;
#endif //CONDITIONSDEBUG
	}
#ifdef CONDITIONSDEBUG
	pthread_mutex_lock(&g_READMUTEX);
	i= g_mMutex.find(mutex);
	if(i != g_mMutex.end())
		i->second.threadid= gettid();
	pthread_mutex_unlock(&g_READMUTEX);
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
		LOG(LOG_ERROR, msg);
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
		LOG(LOG_ERROR, msg);
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
		LOG(LOG_ERROR, msg);
#ifdef DEBUG
		cerr << msg << endl;
#endif
	}
	return nRv;
}

int Thread::stop(const bool *bWait)
{
#ifndef SINGLETHREADING
	int nRv;
//	LogInterface* log= LogInterface::instance();

	LOCK(m_STARTSTOPTHREAD);
	m_bStop= true;
	if(	bWait
		&&
		*bWait	)
	{
		if(!running())
		{
			UNLOCK(m_STARTSTOPTHREAD);
			return 5;
		}
//		if(!log->ownThread(getThreadName(), pthread_self()))
//		{
			nRv= CONDITION(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD);
			if(nRv != 0)
			{
				string msg("### fatal ERROR: cannot join correctly to thread ");

				msg+= getThreadName() + "\n                 ";
				msg+= strerror(nRv);
#ifdef DEBUG
				cerr << msg << endl;
#endif // DEBUG
				LOG(LOG_ERROR,  msg);
			}
//		}else
//			LOG(LOG_ERROR, "application cannot stop own thread with stop(true)");
	}
	UNLOCK(m_STARTSTOPTHREAD);

#endif // SINGLETHREADING
	return 0;
}

string Thread::getThreadName() const
{
	string name;

	LOCK(m_THREADNAME);
	name= m_sThreadName;
	UNLOCK(m_THREADNAME);
	return name;
}

int Thread::stopping()
{
	int nRv= 0;

	LOCK(m_STARTSTOPTHREAD);
	if(m_bStop)
		nRv= 1;
	UNLOCK(m_STARTSTOPTHREAD);
	return nRv;
}

int Thread::running()
{
	int nRv= 0;

	LOCK(m_RUNTHREAD);
	if(m_bRun)
		nRv= 1;
	UNLOCK(m_RUNTHREAD);
	return nRv;
}

int Thread::initialed()
{
	int nRv= 0;

	LOCK(m_RUNTHREAD);
	if(m_bInitialed)
		nRv= 1;
	UNLOCK(m_RUNTHREAD);
	return nRv;
}

Thread::~Thread()
{
	stop(true);
	pthread_join(m_nPosixThreadID, NULL);
	DESTROYMUTEX(m_RUNTHREAD);
	DESTROYMUTEX(m_THREADNAME);
	DESTROYMUTEX(m_STARTSTOPTHREAD);
	DESTROYCOND(m_STARTSTOPTHREADCOND);
}
