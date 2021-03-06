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
#include "ThreadErrorHandling.h"

#include "../exception.h"
#include "../GlobalStaticMethods.h"

#include "../properties/configpropertycasher.h"

#include "../../pattern/util/LogHolderPattern.h"
#include "../../database/logger/lib/logstructures.h"

using namespace std;
using namespace util;
using namespace util::thread;

#ifdef MUTEXLOCKDEBUG
		bool Thread::m_bAllMutex= false;
		vector<string> Thread::m_vMutexProcesses;
		vector<string> Thread::m_vMutexes;
#endif // MUTEXLOCKDEBUG
#ifdef CONDITIONSDEBUG
		bool Thread::m_bAllCondition= false;
		vector<string> Thread::m_vConditionProcesses;
		vector<string> Thread::m_vConditions;
#endif // CONDITIONSDEBUG

pthread_mutex_t Thread::g_READMUTEX;
SHAREDPTR::shared_ptr<map<pthread_mutex_t*, mutexnames_t> > Thread::g_mMutex= Thread::init_globalMutex();
SHAREDPTR::shared_ptr<map<pthread_cond_t*, string> > Thread::g_mCondition= Thread::init_globalCondition();
bool Thread::m_bAppRun= true;
bool Thread::m_bGlobalObjDefined= false;

Thread::Thread(const string& threadName, bool waitInit/*= true*/, const int policy/*= -1*/,
				const int priority/*= -9999*/, IClientSendMethods* logger)
{
	m_nThreadId= 0;
	m_nPosixThreadID= 0;
	m_bRun= false;
	m_bInitialed= false;
	m_bStop= false;
	m_nSchedPolicy= policy;
	m_nSchedPriority= priority;
	m_eErrorType= NONE;
	m_nErrorCode= 0;
	m_sThreadName= threadName;
	m_bWaitInit= waitInit;
	m_pExtLogger= logger;
	m_pError= EHObj(new ThreadErrorHandling);
	m_RUNTHREAD= getMutex("RUNTHREAD", logger);
	m_THREADNAME= getMutex("THREADNAME", logger);
	m_STARTSTOPTHREAD= getMutex("STARTSTOPTHREAD", logger);
	m_ERRORCODES= getMutex("ERRORCODES", logger);
	m_SLEEPMUTEX= getMutex("SLEEPMUTEX", logger);
	m_SLEEPCOND= getCondition("SLEEPCOND", logger);
	m_STARTSTOPTHREADCOND= getCondition("STARTSTOPTHREADCOND", logger);
}

SHAREDPTR::shared_ptr<map<pthread_mutex_t*, mutexnames_t> > Thread::init_globalMutex()
{
	if(m_bGlobalObjDefined)
		return g_mMutex;
	pthread_mutex_init(&g_READMUTEX, NULL);
	m_bGlobalObjDefined= true;
	g_mMutex= SHAREDPTR::shared_ptr<map<pthread_mutex_t*, mutexnames_t> >(new map<pthread_mutex_t*, mutexnames_t>());
	g_mCondition= SHAREDPTR::shared_ptr<map<pthread_cond_t*, string> >(new map<pthread_cond_t*, string>());
	return g_mMutex;
}

SHAREDPTR::shared_ptr<map<pthread_cond_t*, string> > Thread::init_globalCondition()
{
	init_globalMutex();
	return g_mCondition;
}

EHObj Thread::start(void *args, bool bHold)
{
	int ret= 0;
	string threadName("");
	pthread_attr_t attr;

	try{
		if(running())
		{
			LOCK(m_ERRORCODES);
			m_eErrorType= BASIC;
			m_nErrorCode= 1;
			mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			m_pError->setWarning("Thread", "start");
			return m_pError;
		}
		mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
		m_eErrorType= NONE;
		m_nErrorCode= 0;
		mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
#ifdef SINGLETHREADING
		run();
		return m_nEndValue;
#else // SINGLETHREADING

		threadName= getThreadName();
		m_bHold= bHold;
		m_pArgs= args;
		mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
		m_bStop= false;
		mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
		ret= pthread_attr_init(&attr);
		if(ret != 0)
		{
			ThreadErrorHandling handle;

			handle.setPThreadError("Thread", "pthread_attr_init", "pthread_attr_init", ret, threadName);
			(*m_pError)= handle;
			mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			m_eErrorType= BASIC;
			m_nErrorCode= -1;
			mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			m_bStop= true;
			mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			return m_pError;
		}
		ret= pthread_create (&m_nPosixThreadID, &attr, Thread::EntryPoint, this);
		if(ret != 0)
		{
			ThreadErrorHandling handle;

			handle.setPThreadError("Thread", "pthread_create", "pthread_create", ret, threadName);
			(*m_pError)= handle;
			LOGEX(LOG_ALERT, m_pError->getDescription(), m_pExtLogger);
			mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			m_eErrorType= BASIC;
			m_nErrorCode= -1;
			mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			m_bStop= true;
			mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			return m_pError;
		}
		if(bHold)
		{
			POSS("###Thread_join", threadName);
			ret= pthread_join(m_nPosixThreadID, NULL);
			if(ret != 0)
			{
				ThreadErrorHandling handle;

				handle.setPThreadError("Thread", "pthread_join", "pthread_join", ret, threadName);
				(*m_pError)= handle;
				LOGEX(LOG_ALERT, m_pError->getDescription(), m_pExtLogger);
				mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
				if(m_eErrorType == NONE)
				{
					m_eErrorType= BASIC;
					m_nErrorCode= -2;
				}
				mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
				return m_pError;
			}
			return m_pError;
		}else
		{
			if(m_bWaitInit)
			{
				mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
				while(!running())
				{
					if(m_bStop)
					{// an error is occured in init methode
						mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
						ret= pthread_join(m_nPosixThreadID, NULL);
						if(ret != 0)
						{
							ThreadErrorHandling handle;

							handle.setPThreadError("Thread", "pthread_join", "pthread_join", ret, threadName);
							(*m_pError)= handle;
							LOGEX(LOG_ALERT, m_pError->getDescription(), m_pExtLogger);
							mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
							if(m_eErrorType == NONE)
							{
								m_eErrorType= BASIC;
								m_nErrorCode= -3;
							}
							mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
						}
						return m_pError;
					}
					CONDITIONEX(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD, m_pExtLogger);
				}
				mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			}
		}
		return m_pError;
#endif // else SUNGLETHREADING
	}catch(SignalException& ex)
	{
		string err;

		m_pError->setError("Thread", "start_exception", threadName);
		ex.addMessage("try to start thread " + threadName + "\nso ending hole thread routine");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOGEX(LOG_ALERT, err, m_pExtLogger);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}

	}catch(std::exception& ex)
	{
		string err;

		m_pError->setError("Thread", "start_exception", threadName);
		err=  "ERROR: STD exception by try to start thread " + threadName + "\n       so ending hole thread routine\n";
		err+= "what(): " + string(ex.what());
		cerr << err << endl;
		try{
			LOGEX(LOG_ALERT, err, m_pExtLogger);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}

	}catch(...)
	{
		string error;

		m_pError->setError("Thread", "start_exception", threadName);
		error+= "ERROR: catching UNKNOWN exception by running thread of " + threadName;
		error+= "\n       so ending hole thread routine";
		cerr << error << endl;
		try{
			LOGEX(LOG_ALERT, error, m_pExtLogger);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}
	}
	mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	if(m_eErrorType == NONE)
	{
		m_eErrorType= BASIC;
		m_nErrorCode= -4;
	}
	mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	return m_pError;
}

EHObj Thread::setSchedulingParameter(int policy, int priority)
{
	EHObj handle(EHObj(new ThreadErrorHandling));

	if(!running())
	{
		m_nSchedPolicy= policy;
		m_nSchedPriority= priority;
		return handle;
	}
	if(m_nThreadId == gettid())
		return setSchedulingParameterInline(policy, priority);
	handle->setError("Thread", "no_scheduling_set_possibe", m_sThreadName);
	return handle;
}
EHObj Thread::setSchedulingParameterInline(int policy, int priority)
{
	int res, oldPolicy;
	sched_param param;
	ThreadErrorHandling thHandle;
	EHObj handle(EHObj(new ThreadErrorHandling));

	res= pthread_getschedparam(m_nPosixThreadID, &oldPolicy, &param);
	if(res)
		thHandle.setPThreadWarning("Thread", "pthread_getschedparam",
						"read_scheduling", res, getThreadName());
	if(	(	policy != -1 &&
			policy != oldPolicy	) ||
		(	priority != -9999 &&
			priority != param.sched_priority	)	)

	{
		if(policy == -1)
			policy= oldPolicy;
		m_nSchedPolicy= policy;
		if(priority == -9999)
			priority= param.sched_priority;
		m_nSchedPriority= priority;
		param.sched_priority= priority;
		res= pthread_setschedparam(m_nPosixThreadID, policy, &param);
		if(res)
		{
			int min, max;
			ostringstream decl, opolicy;

			switch(policy)
			{
			case SCHED_OTHER:
				decl << "SCHED_OTHER";
				break;
			case SCHED_FIFO:
				decl << "SCHED_FIFO";
				break;
			case SCHED_RR:
				decl << "SCHED_RR";
				break;
			case SCHED_BATCH:
				decl << "SCHED_BATCH";
				break;
			case SCHED_IDLE:
				decl << "SCHED_IDLE";
				break;
			case SCHED_RESET_ON_FORK:
				decl << "SCHED_RESET_ON_FORK";
				break;
			default:
				opolicy << policy;
				decl << "(unknown policy [ " + opolicy.str() + " ] )";
				break;
			}
			min= sched_get_priority_min(policy);
			if(min < 0)
				min= 0;
			max= sched_get_priority_max(policy);
			if(max < 0)
				max= 0;
			decl << "@" << priority;
			decl << "@" << getThreadName();
			decl << "@" << m_nPosixThreadID;
			decl << "@" << getuid();
			decl << "@" << min;
			decl << "@" << max;
			thHandle.setPThreadError("Thread", "pthread_setschedparam",
							"set_scheduling", res, decl.str()			);
		}
	}else
	{
		m_nSchedPolicy= oldPolicy;
		m_nSchedPriority= param.__sched_priority;
	}
	if(thHandle.fail())
		(*handle)= thHandle;
	return handle;
}

void Thread::run()
{
	string error("undefined error by ");
	string thname(getThreadName());
	string startmsg("starting thread with name '");
	pos_t pos;

	try{
		m_pError= setSchedulingParameterInline(m_nSchedPolicy, m_nSchedPriority);
		if(m_pError->fail())
			m_pError->defineAsWarning();
		m_nThreadId= gettid();
		initstatus(thname, this);
		startmsg+= thname + "'";
		LogHolderPattern::instance()->setThreadName(thname, m_pExtLogger);
		LOGEX(LOG_DEBUG, startmsg, m_pExtLogger);

#ifdef SERVERDEBUG
		cout << startmsg << endl;
#endif // SERVERDEBUG

		setThreadLogName(thname, m_pExtLogger);
		mutex_lock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
		m_bRun= true;
		m_bInitialed= false;
		mutex_unlock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
		try{
			(*m_pError)= init(m_pArgs);

		}catch(SignalException& ex)
		{
			ex.addMessage("running init() method");
			mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			m_eErrorType= BASIC;
			m_nErrorCode= -5;
			mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			throw ex;
		}
		if(!m_pError->hasError())
		{
			mutex_lock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
			m_bInitialed= true;
			mutex_unlock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
		}else
		{
			mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			m_eErrorType= INIT;
			mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
			error+= "### thread ";
			error+= thname;
			error+= " cannot initial correctly";
			LOGEX(LOG_ERROR, error, m_pExtLogger);
			mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			m_bStop= true;
			mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			mutex_lock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
			m_bRun= false;
			m_bInitialed= false;
			mutex_unlock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
		}
		mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
		AROUSE(m_STARTSTOPTHREADCOND);
		mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
		if(running())
		{
			bool again(true);

			while(	again &&
					!stopping()	)
			{
				m_pError->clear();
				POS("###THREAD_execute_start");
				try{
					again= execute();

				}catch(SignalException& ex)
				{
					ex.addMessage("running execute() method");
					mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					m_eErrorType= BASIC;
					m_nErrorCode= -6;
					mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					throw ex;
				}catch(std::exception& ex)
				{
					string err;

					err=  "STD exception by execute thread of " + thname + "\nso ending hole thread routine\n";
					err+= "what(): " + string(ex.what());
					cerr << err << endl;
					mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					m_eErrorType= BASIC;
					m_nErrorCode= -6;
					mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					LOGEX(LOG_ALERT, err+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);
					break;

				}catch(...)
				{
					error+= "ERROR: catching UNKNOWN exception by execute thread of " + thname;
					error+= "\n       so ending hole thread routine";
					cerr << error << endl;
					mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					m_eErrorType= BASIC;
					m_nErrorCode= -6;
					mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
					LOGEX(LOG_ALERT, error+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);
					break;
				}
				if( !again &&
					!stopping()	)
				{
					stop();
				}
			}
		}

	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("running thread of " + thname + "\nso ending hole thread routine");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		LOGEX(LOG_ALERT, err+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);

	}catch(std::exception& ex)
	{
		string err;

		err=  "STD exception by running thread of " + thname + "\nso ending hole thread routine\n";
		err+= "what(): " + string(ex.what());
		cerr << err << endl;
		LOGEX(LOG_ALERT, err+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);

	}catch(...)
	{
		error+= "ERROR: catching UNKNOWN exception by running thread of " + thname;
		error+= "\n       so ending hole thread routine";
		cerr << error << endl;
		LOGEX(LOG_ALERT, error+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);
	}

	try{
		ending();
		glob::threadStopMessage("Thread::run(): running thread of '" + thname + "' was reaching end and will be destroy");
		removestatus(m_nThreadId);
		return;

	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("running end of thread " + thname + "\nso ending hole thread routine");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		LOGEX(LOG_ALERT, err+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);

	}catch(std::exception& ex)
	{
		string err;

		err=  "ERROR: STD exception by running end of thread " + thname;
		err+= "\n       so ending hole thread routine\n";
		err+= "what(): " + string(ex.what());
		cerr << err << endl;
		LOGEX(LOG_ALERT, err+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);

	}catch(...)
	{
		error+= "ERROR: catching UNKNOWN exception by running end of thread " + thname;
		error+= "\n       so ending hole thread routine";
		cerr << error << endl;
		LOGEX(LOG_ALERT, error+"\n\n++++++  ending hole thread routine of " + thname + "  +++++", m_pExtLogger);
	}
	mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	if(m_eErrorType == NONE)
	{
		m_eErrorType= BASIC;
		m_nErrorCode= -7;
	}
	mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
}

Thread::ERRORtype Thread::getErrorType()
{
	ERRORtype eRv;

	mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	eRv= m_eErrorType;
	mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	return eRv;
}

int Thread::getErrorCode()
{
	int nRv;

	mutex_lock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	nRv= m_nErrorCode;
	mutex_unlock(__FILE__, __LINE__, m_ERRORCODES, m_pExtLogger);
	return nRv;
}

/*static */
void *Thread::EntryPoint(void *pthis)
{
	Thread *pt= NULL;

	try{
		pt= (Thread*)pthis;
		pt->run();
		//POS("###THREAD_execute_stop");
		mutex_lock(__FILE__, __LINE__, pt->m_STARTSTOPTHREAD, pt->m_pExtLogger);
		mutex_lock(__FILE__, __LINE__, pt->m_RUNTHREAD, pt->m_pExtLogger);
		pt->m_bRun= false;
		mutex_unlock(__FILE__, __LINE__, pt->m_RUNTHREAD, pt->m_pExtLogger);
		AROUSE(pt->m_STARTSTOPTHREADCOND);
		mutex_unlock(__FILE__, __LINE__, pt->m_STARTSTOPTHREAD, pt->m_pExtLogger);

	}catch(SignalException& ex)
	{
		string err;

		err= "running entry point of thread";
		try{
			if(pt != NULL)
				err+= " " + pt->m_sThreadName;
		}catch(...){}
		err+= ", so ending hole thread routine";
		ex.addMessage(err);
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
			if(pt != NULL)
			{
				mutex_lock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
				if(pt->m_eErrorType == NONE)
				{
					pt->m_eErrorType= BASIC;
					pt->m_nErrorCode= -7;
				}
				mutex_unlock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
			}
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			cerr << "       '" << err << "'" << endl << endl;
		}

	}catch(std::exception& ex)
	{
		string err;

		err= "ERROR: STD exception by running entry point of thread";
		try{
			if(pt != NULL)
				err+= " " + pt->m_sThreadName;
		}catch(...){}
		err+= ", so ending hole thread routine\n";
		err+= "what(): " + string(ex.what());
		cerr << err << endl;
		try{
			LOG(LOG_ALERT, err);
			if(pt != NULL)
			{
				mutex_lock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
				if(pt->m_eErrorType == NONE)
				{
					pt->m_eErrorType= BASIC;
					pt->m_nErrorCode= -7;
				}
				mutex_unlock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
			}
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			cerr << "       '" << err << "'" << endl << endl;
		}

	}catch(...)
	{
		string err;

		err= "ERROR: catching UNKNOWN exception by running entry point of thread";
		try{
			if(pt != NULL)
				err+= " " + pt->m_sThreadName;
		}catch(...){}
		err+= ", so ending hole thread routine";
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
			if(pt != NULL)
			{
				mutex_lock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
				if(pt->m_eErrorType == NONE)
				{
					pt->m_eErrorType= BASIC;
					pt->m_nErrorCode= -7;
				}
				mutex_unlock(__FILE__, __LINE__, pt->m_ERRORCODES, pt->m_pExtLogger);
			}
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			cerr << "       '" << err << "'" << endl << endl;
		}
	}
	return NULL;
}

pid_t Thread::gettid()
{
	return (pid_t) syscall(SYS_gettid);
}

pthread_mutex_t* Thread::getMutex(const string& name, IClientSendMethods* logger)
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
		LOGEX(LOG_ALERT, msg, logger);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(	error != 0 ||
			!m_bGlobalObjDefined	)
		{
			init_globalMutex();
			if(error != 0)
				error= pthread_mutex_lock(&g_READMUTEX);
		}
		if(error != 0)
		{
			LOGEX(LOG_ERROR, "error by global mutex lock, locking by "
							+ getMutexName(mutex), logger);
		}

		tName.name= name;
#ifdef MUTEXLOCKDEBUG
		tName.threadid= 0;
#endif
		//if(	name != "POSITIONSTATUS" &&
		//	name != "OBJECTSMUTEX"		)
		if(m_bGlobalObjDefined)
		{
			if(mMutexBuffer.size() > 0)
			{// first mutex creation of global mutex POSITIONSTATUS makes error
			 // since gcc (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
			 //       g++ (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
			 // so write mutex for first time in an buffer
			 // (gcc (Debian 4.4.5-8) 4.4.5 made no problem's)
				for(map<pthread_mutex_t*, mutexnames_t>::iterator it= mMutexBuffer.begin(); it != mMutexBuffer.end(); ++it)
					(*g_mMutex)[it->first]= it->second;
				mMutexBuffer.clear();
			}
			(*g_mMutex)[mutex]= tName;

		}else
			mMutexBuffer[mutex]= tName;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{
			LOGEX(LOG_ERROR, "error by global mutex unlock, by locking for "
							+ getMutexName(mutex), logger);
		}
	}
	return mutex;
}

pthread_cond_t* Thread::getCondition(const string& name, IClientSendMethods* logger)
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
		LOGEX(LOG_ALERT, msg, logger);
	}else
	{
		int error= pthread_mutex_lock(&g_READMUTEX);
		if(	error != 0 ||
			!m_bGlobalObjDefined	)
		{
			init_globalCondition();
			if(error != 0)
				error= pthread_mutex_lock(&g_READMUTEX);
		}
		if(error != 0)
		{
			LOGEX(LOG_ERROR, "error by mutex lock " + getConditionName(cond, logger), logger);
		}

		(*g_mCondition)[cond]= name;

		error= pthread_mutex_unlock(&g_READMUTEX);
		if(error != 0)
		{LOGEX(LOG_ERROR, "error by mutex unlock " + getConditionName(cond, logger), logger);
		}
	}
	return cond;
}

string Thread::getMutexName(pthread_mutex_t* mutex, IClientSendMethods* logger)
{
	//bool bFound= false;
	string name;
	int error;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(	error != 0 ||
		!m_bGlobalObjDefined	)
	{
		init_globalMutex();
		if(error != 0)
			error= pthread_mutex_lock(&g_READMUTEX);
	}
	if(error != 0)
	{
		LOGEX(LOG_ERROR, "error by mutex lock READMUTEX by get name", logger);
		return "unknown";
	}
	i= g_mMutex->find(mutex);
	if(i != g_mMutex->end())
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
		LOGEX(LOG_ERROR, "error by mutex unlock READMUTEX by get name", logger);
	}


	return name;
}

string Thread::getConditionName(pthread_cond_t *cond, IClientSendMethods* logger)
{
	string name;
	int error;
	typedef map<pthread_cond_t*, string>::iterator iter;
	iter i;

	error= pthread_mutex_lock(&g_READMUTEX);
	if(	error != 0 ||
		!m_bGlobalObjDefined	)
	{
		init_globalCondition();
		if(error != 0)
			error= pthread_mutex_lock(&g_READMUTEX);
	}
	if(error != 0)
	{
		LOGEX(LOG_ERROR, "error by mutex lock READMUTEX by get condition name", logger);
		return "unknown";
	}
	i= g_mCondition->find(cond);
	if(i != g_mCondition->end())
		name= i->second;
	else
		name= "ERROR: undefined condition";
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LOGEX(LOG_ERROR, "error by mutex unlock READMUTEX by get condition name", logger);
	}
	return name;
}

string Thread::doOutput(pthread_mutex_t *mutex, const string& cond, bool& bProcess, bool& bSet)
{
	vector<string> split;
	vector<string>::iterator found;
	string mutexname, ownprocess(glob::getProcessName());

	bSet= false;
	bProcess= false;
	if(cond == "")
	{
#ifdef MUTEXLOCKDEBUG
		if(m_bAllMutex)
		{
			if(m_vMutexProcesses.size() > 0)
			{
				found= find(m_vMutexProcesses.begin(), m_vMutexProcesses.end(), ownprocess);
				if(found != m_vMutexProcesses.end())
					bProcess= true;
			}else
				bProcess= true;
			if(!bProcess)
				return "";
			mutexname= getMutexName(mutex);
			if(	m_vMutexes.size() == 0 ||
				mutexname == "ERROR: undefined mutex")
			{
				/*
				 * all mutex definition
				 * are defined for output
				 */
				bSet= true;
				return mutexname;
			}
			found= find(m_vMutexes.begin(), m_vMutexes.end(), mutexname);
			if(found != m_vMutexes.end())
				bSet= true;
			return mutexname;
		}
		m_bAllMutex= true;
		split= ConfigPropertyCasher::split(MUTEXLOCKDEBUG, " ");
#endif // MUTEXLOCKDEBUG
	}else
	{
#ifdef CONDITIONSDEBUG
		if(m_bAllCondition)
		{
			if(m_vConditionProcesses.size() > 0)
			{
				found= find(m_vConditionProcesses.begin(), m_vConditionProcesses.end(), ownprocess);
				if(found != m_vConditionProcesses.end())
					bProcess= true;
			}else
				bProcess= true;
			if(!bProcess)
				return "";
			if(mutex != NULL)
				mutexname= getMutexName(mutex);
			if(	m_vConditions.size() == 0 ||
				cond == "ERROR: undefined condition"	)
			{
				/*
				 * all condition definition
				 * are defined for output
				 */
				bSet= true;
				return mutexname;
			}
			found= find(m_vConditions.begin(), m_vConditions.end(), cond);
			if(found != m_vConditions.end())
				bSet= true;
			return mutexname;
		}
		m_bAllCondition= true;
		split= ConfigPropertyCasher::split(CONDITIONSDEBUG, " ");
#endif // CONDITIONSDEBUG
	}
	if(split.size() == 0)
	{
		bProcess= true;
		bSet= true;
		if(mutex != NULL)
			mutexname= getMutexName(mutex);
	}else
	{
		string smutex;
		vector<string> vAllProcesses;
		vector<string> vAllLocks;

		for (vector<string>::iterator it= split.begin(); it != split.end(); ++it)
		{
			if(*it == "ppi-server")
				vAllProcesses.push_back(*it);
			else if(*it == "ppi-db-server")
				vAllProcesses.push_back(*it);
			else if(*it == "ppi-owreader")
				vAllProcesses.push_back(*it);
			else if(*it == "ppi-internet-server")
				vAllProcesses.push_back(*it);
			else if(*it == "ppi-client")
				vAllProcesses.push_back(*it);
			else if(*it == "ppi-mconfig")
				vAllProcesses.push_back(*it);
			else
				vAllLocks.push_back(*it);
			if(*it == ownprocess)
				bProcess= true;
		}
		if(cond == "")
		{
#ifdef MUTEXLOCKDEBUG
			m_vMutexProcesses= vAllProcesses;
			m_vMutexes= vAllLocks;
#endif // MUTEXLOCKDEBUG
		}else
		{
#ifdef CONDITIONSDEBUG
			m_vConditionProcesses= vAllProcesses;
			m_vConditions= vAllLocks;
#endif // CONDITIONSDEBUG
		}
		if(!bProcess)
			return "";
		mutexname= getMutexName(mutex);
		if(cond == "")
			smutex= mutexname;
		else
			smutex= cond;
		found= find(vAllLocks.begin(), vAllLocks.end(), smutex);
		if(	found != vAllLocks.end() ||
			smutex.substr(0, 17) == "ERROR: undefined "	)
		{
			bSet= true;
		}
	}
	return mutexname;
}

int Thread::mutex_lock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger)
{
	int error;

#ifdef MUTEXLOCKDEBUG
	bool bSet(false), bProcessSet(false);
	ostringstream before, behind;
	pid_t lastlockID;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	string mutexname, ownprocess(glob::getProcessName());
	iter i;

	mutexname= doOutput(mutex, "", bProcessSet, bSet);
	if(bProcessSet)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex->find(mutex);
		if(i != g_mMutex->end())
		{
			lastlockID= i->second.threadid;
			if(lastlockID == gettid())
			{
				ostringstream msg;

				msg << "[";
				msg.fill(' ');
				msg.width(5);
				msg << dec << gettid() << "] ";
				msg << "WARNING: " << ownprocess << " thread lock's mutex " << mutexname << " again, on file:" << file << " line:" << line << endl;
				msg << "           locking before on file:" << i->second.fileLocked << " line:" << i->second.lineLocked << endl;
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
			before << ownprocess << " want to lock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
			if(lastlockID != 0)
			{
				before.fill(' ');
				before.width(8);
				before << " ";
				before << ownprocess << " but mutex was locked from thread " << lastlockID << endl;
			}
			cout << before.str();
		}
	}// if(bProcessSet)
	if(mutexname != "POSITIONSTATUS")
		POSS("###mutex_wait", mutexname);
#endif // MUTEXLOCKDEBUG

	error= pthread_mutex_lock(mutex);
	if(error != 0)
	{
		string msg("error by mutex lock ");

		msg+= getMutexName(mutex);
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, msg, "", logger);
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
		if(bProcessSet)
		{
			pthread_mutex_lock(&g_READMUTEX);
			i= g_mMutex->find(mutex);
			if(i != g_mMutex->end())
			{
				i->second.threadid= gettid();
				i->second.fileLocked= file;
				i->second.lineLocked= line;
			}
			pthread_mutex_unlock(&g_READMUTEX);
			if(bSet)
			{
				behind << "[";
				behind.fill(' ');
				behind.width(5);
				behind << dec << gettid() << "] ";
				behind << ownprocess << " mutex " << mutexname << "  be locked on file:" << file << " line:" << line  << endl;
				cout << behind.str();
			}
		}
	}else if(bSet)
		cout << ownprocess << " mutex " << mutexname << " cannot lock by thread(" << dec << gettid() << ") an ERROR occured" << endl;
#endif // MUTEXLOCKDEBUG
	return error;
}

int Thread::mutex_trylock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger)
{
	int error;

#ifdef MUTEXLOCKDEBUG
	bool bSet, bProcessSet;
	ostringstream before, behind;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	string mutexname, ownprocess(glob::getProcessName());
	iter i;

	mutexname= doOutput(mutex, "", bProcessSet, bSet);
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << ownprocess << " try to lock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
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
		LogHolderPattern::instance()->log(file, line, LOG_ERROR,
						"error by try to lock mutex "
						+ getMutexName(mutex), "", logger);
	}
#ifdef MUTEXLOCKDEBUG
	if(bProcessSet)
	{
		if(error == 0)
		{
			pthread_mutex_lock(&g_READMUTEX);
			i= g_mMutex->find(mutex);
			if(i != g_mMutex->end())
				i->second.threadid= gettid();
			pthread_mutex_unlock(&g_READMUTEX);
			if(bSet)
			{
				behind << "[";
				behind.fill(' ');
				behind.width(5);
				behind << dec << gettid() << "] ";
				behind << ownprocess << " mutex " << mutexname << "  be locked on file:" << file << " line:" << line  << endl;
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
			behind << ownprocess << " mutex " << mutexname << "  is busy do not lock on file:" << file << " line:" << line  << endl;
			cout << behind.str();
		}
	}
#endif // MUTEXLOCKDEBUG
	return error;
}

int Thread::mutex_unlock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger)
{
	int error;
	string mutexname;

#ifdef MUTEXLOCKDEBUG
	bool bSet, bProcessSet;
	ostringstream before;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	string ownprocess(glob::getProcessName());
	iter i;
	pid_t tid= gettid();

	mutexname= doOutput(mutex, "", bProcessSet, bSet);
	if(bProcessSet)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex->find(mutex);
		if(i != g_mMutex->end())
		{
			if(i->second.threadid != gettid())
			{
				if(mutexname == "")
					mutexname= getMutexName(mutex);
				before << "[";
				before.fill(' ');
				before.width(5);
				before << dec << tid << "] ";
				before << "WARNING: " << ownprocess << " thread " << tid << " want to unlock mutex " << mutexname;
				if(i->second.threadid == 0)
					before << " witch isn't locked from any thread" << endl;
				else
					before << " witch is locked from other thread " << i->second.threadid << endl;
				before <<"        on file:" << file << " line:" << line << endl;
				cout << before.str();
			}
			i->second.threadid= 0;
			i->second.fileLocked= "";
			i->second.lineLocked= 0;
		}
		pthread_mutex_unlock(&g_READMUTEX);
		if(bSet)
		{
			before << "[";
			before.fill(' ');
			before.width(5);
			before << dec << gettid() << "] ";
			before << ownprocess << " unlock mutex " << mutexname << " on file:" << file << " line:" << line << endl;
			cout << before.str();
		}
	}
#endif

	error= pthread_mutex_unlock(mutex);
	if(error != 0)
	{
		string msg(glob::getProcessName() + " error by unlock mutex ");

		if(mutexname == "")
			mutexname= getMutexName(mutex);
		msg+= mutexname;
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, msg, "", logger);
#ifdef MUTEXLOCKDEBUG
		ostringstream thid;

		if(bProcessSet)
		{
			thid << "[";
			thid.fill(' ');
			thid.width(5);
			thid << dec << gettid() << "] ";
			thid << msg << endl;
			cerr << thid.str();
		}
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

void Thread::destroyMutex(const string& file, int line, pthread_mutex_t* mutex, IClientSendMethods* logger)
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
		LogHolderPattern::instance()->log(file, line, LOG_ERROR,
						"error by mutex lock READMUTEX by destroy", "", logger);
		return;
	}
	if(m_bAppRun)
	{
		i= g_mMutex->find(mutex);
		if(i != g_mMutex->end()) // erase mutex from map
			g_mMutex->erase(i);
	}
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LogHolderPattern::instance()->log(file, line, LOG_ERROR,
						"error by mutex unlock READMUTEX by destroy", "", logger);
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

	pthread_mutex_lock(&g_READMUTEX);
	for(iter i= g_mMutex->begin(); i!=g_mMutex->end(); ++i)
	{
		g_mMutex->erase(i->first);
		pthread_mutex_destroy(i->first);
		delete i->first;
	}
	pthread_mutex_unlock(&g_READMUTEX);
	// toDo: do not take READMUTEX for mutex lock and conditions
	//pthread_mutex_destroy(&g_READMUTEX);
}

void Thread::destroyCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger)
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
		LOGEX(LOG_ERROR, "error by mutex lock READMUTEX by destroy condition", logger);
		return;
	}
	if(m_bAppRun)
	{
		i= g_mCondition->find(cond);
		if(i != g_mCondition->end()) // erase mutex from map
			g_mCondition->erase(i);
	}
	//conderror= pthread_cond_in
	error= pthread_mutex_unlock(&g_READMUTEX);
	if(error != 0)
	{
		LogHolderPattern::instance()->log(file, line, LOG_ERROR,
						"error by mutex unlock READMUTEX by destroy condition", "", logger);
	}
	pthread_cond_destroy(cond);
	delete cond;
}

void Thread::destroyAllConditions()
{
	typedef map<pthread_cond_t*, string>::iterator iter;

	pthread_mutex_lock(&g_READMUTEX);
	for(iter i= g_mCondition->begin(); i!=g_mCondition->end(); ++i)
	{
		g_mCondition->erase(i->first);
		pthread_cond_destroy(i->first);
		delete i->first;
	}
	pthread_mutex_unlock(&g_READMUTEX);
}

int Thread::conditionWait(const string& file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex,
				const time_t sec, const bool absolute, IClientSendMethods* logger)
{
	timespec time;

	time.tv_sec= sec;
	time.tv_nsec= 0;
	return conditionWait(file, line, cond, mutex, &time, absolute, logger);
}

int Thread::conditionWait(const string& file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex,
				const struct timespec *time, const bool absolute, IClientSendMethods* logger)
{
	int retcode;
	string condname;
	timespec tabsolute;

	if(	time &&
		!absolute	)
	{
		// measure current actual time on begin of method
		// to get more precise relative waiting time
		clock_gettime(CLOCK_REALTIME, &tabsolute);
	}
	condname= getConditionName(cond, logger);
#ifdef CONDITIONSDEBUG
	bool bSet, bProcessSet;
	ostringstream msg;
	string mutexname;
	ostringstream before, behind;
	vector<string> split;
	typedef map<pthread_mutex_t*, mutexnames_t>::iterator iter;
	iter i;

	doOutput(NULL, condname, bProcessSet, bSet);
	if(bProcessSet)
		mutexname= getMutexName(mutex);
	if(bSet)
	{
		before << "[";
		before.fill(' ');
		before.width(5);
		before << dec << gettid() << "] ";
		before << "wait for condition " << condname << " on file:" << file << " line:" << line << endl;
		cout << before.str();
	}
	if(bProcessSet)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex->find(mutex);
		if(	i == g_mMutex->end()
			||
			i->second.threadid == 0	)
		{
			if(i == g_mMutex->end())
				msg << "ERROR: mutex " << mutexname << " not found" << endl;
			msg << "ERROR: in thread (" << gettid() << ")";
			msg << " mutex " << mutexname << " for condition " << condname << endl;
			msg << "   on LINE: " << dec << line << " from FILE:" << file << endl;
			msg << "   is not locked" << endl;
			cerr << msg.str();
			LOGEX(LOG_ERROR, msg.str(), logger);
		}
		pthread_mutex_unlock(&g_READMUTEX);
	}
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
			// current absolute time was measured
			// on begin of method
			tabsolute.tv_sec+= time->tv_sec;
			tabsolute.tv_nsec+= time->tv_nsec;
			if(tabsolute.tv_nsec > 999999999)
			{
				long int sec(tabsolute.tv_nsec / 1000000000);

				tabsolute.tv_sec+= sec;
				tabsolute.tv_nsec-= (sec * 1000000000);
			}
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
		t << dec << gettid() << errno;
		msg << "condition ";
		msg << getConditionName(cond, logger) << " in mutex area ";
		msg << getMutexName(mutex);
		if(time)
		{
			msg << " with limit of ";
			msg << dec << time->tv_sec << " seconds";
			if(time->tv_nsec)
				msg << " and " << dec << time->tv_nsec << " nanoseconds";
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
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, msg.str(), t.str(), logger);
#ifdef CONDITIONSDEBUG
		cerr << msg.str() << endl;
		cerr << "on line " << line << " by file " << file << endl;
#endif //CONDITIONSDEBUG
	}
#ifdef CONDITIONSDEBUG
	if(bProcessSet)
	{
		pthread_mutex_lock(&g_READMUTEX);
		i= g_mMutex->find(mutex);
		if(i != g_mMutex->end())
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
	}
#endif //CONDITIONSDEBUG
	return retcode;
}

int Thread::arouseCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger)
{
	int error;
#ifdef CONDITIONSDEBUG
	bool bSet, bProcessSet;
	string condname(getConditionName(cond));
	ostringstream before;
	vector<string> split;

	doOutput(NULL, condname, bProcessSet, bSet);
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
		msg+= BaseErrorHandling::getErrnoString(errno);
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, msg, "", logger);
	}
	return error;
}

int Thread::arouseAllCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger)
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

		msg+= getConditionName(cond, logger) + "\n       ";
		msg+= BaseErrorHandling::getErrnoString(error);
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, msg, "", logger);
	}
	return error;
}

EHObj Thread::detach()
{
	int ret= pthread_detach(m_nPosixThreadID);

	if(ret != 0)
	{
		ThreadErrorHandling handle;

		handle.setPThreadError("Thread", "pthread_detach", "pthread_detach", ret, getThreadName());
		(*m_pError)= handle;
		LOGEX(LOG_ERROR, m_pError->getDescription(), m_pExtLogger);

#ifdef DEBUG
		cerr << glob::addPrefix("### ERROR: ", m_pError->getDescription()) << endl;
#endif
	}
	return m_pError;
}

EHObj Thread::stop(const bool *bWait)
{
#ifndef SINGLETHREADING
//	LogInterface* log= LogInterface::instance();

	// before lock STARTSTOPTHREAD mutex lock SLEEPMUTEX
	// because otherwise deadlock can be occured
	mutex_lock(__FILE__, __LINE__, m_SLEEPMUTEX, m_pExtLogger);
	mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
	arouseAllCondition(__FILE__, __LINE__, m_SLEEPCOND, m_pExtLogger);
	mutex_unlock(__FILE__, __LINE__, m_SLEEPMUTEX, m_pExtLogger);
	m_bStop= true;
	if(	bWait
		&&
		*bWait	)
	{
		if(!running())
		{
			mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
			return m_pError;
		}
		CONDITIONEX(m_STARTSTOPTHREADCOND, m_STARTSTOPTHREAD, m_pExtLogger);
	}
	mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);

#endif // SINGLETHREADING
	return m_pError;
}

string Thread::getThreadName() const
{
	string name;

	mutex_lock(__FILE__, __LINE__, m_THREADNAME, m_pExtLogger);
	name= m_sThreadName;
	mutex_unlock(__FILE__, __LINE__, m_THREADNAME, m_pExtLogger);
	return name;
}

short Thread::stopping()
{
	short nRv= 0;

	mutex_lock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
	if(m_bStop)
		nRv= 1;
	mutex_unlock(__FILE__, __LINE__, m_STARTSTOPTHREAD, m_pExtLogger);
	return nRv;
}

short Thread::running()
{
	short nRv= 0;

	mutex_lock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
	if(m_bRun)
		nRv= 1;
	mutex_unlock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
	return nRv;
}

int Thread::sleep(const unsigned int wait, string file/*= ""*/, int line/*= 0*/)
{
	timespec req, *rem(NULL);

	if(file == "")
		file= __FILE__;
	if(line <= 0)
		line= __LINE__;
	req.tv_sec= wait;
	req.tv_nsec= 0;
	return nanosleep(&req, rem, file, line);
}

int Thread::usleep(const useconds_t wait, string file/*= ""*/, int line/*= 0*/)
{
	timespec req, *rem(NULL);

	if(file == "")
		file= __FILE__;
	if(line <= 0)
		line= __LINE__;
	req.tv_sec= 0;
	req.tv_nsec= static_cast<long>(wait * 1000);
	return nanosleep(&req, rem, file, line);
}

int Thread::nanosleep(const struct timespec *req, struct timespec *rem, string file/*= ""*/, int line/*= 0*/)
{
	int nRv;
	timespec time;

	if(file == "")
		file= __FILE__;
	if(line <= 0)
		line= __LINE__;
	rem= NULL;
	mutex_lock(file, line, m_SLEEPMUTEX, m_pExtLogger);
	if(stopping())
	{
		m_nRemainSecs.tv_sec= req->tv_sec;
		m_nRemainSecs.tv_nsec= req->tv_nsec;
		rem= &m_nRemainSecs;
		mutex_unlock(file, line, m_SLEEPMUTEX, m_pExtLogger);
		return 0;
	}
	clock_gettime(CLOCK_REALTIME, &time);
	nRv= conditionWait(file, line, m_SLEEPCOND, m_SLEEPMUTEX, req, /*absolute*/false, m_pExtLogger);
	if(nRv != ETIMEDOUT)
	{
		clock_gettime(CLOCK_REALTIME, &m_nRemainSecs);
		m_nRemainSecs.tv_sec-= time.tv_sec;
		if(time.tv_nsec > m_nRemainSecs.tv_nsec)
		{
			--m_nRemainSecs.tv_sec;
			m_nRemainSecs.tv_nsec+= 1000000000;
		}
		m_nRemainSecs.tv_nsec-= time.tv_nsec;
		rem= &m_nRemainSecs;
	}
	mutex_unlock(file, line, m_SLEEPMUTEX, m_pExtLogger);
	return nRv;
}

int Thread::initialed()
{
	int nRv= 0;

	mutex_lock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
	if(m_bInitialed)
		nRv= 1;
	mutex_unlock(__FILE__, __LINE__, m_RUNTHREAD, m_pExtLogger);
	return nRv;
}

Thread::~Thread()
{
	stop(true);
	DESTROYMUTEXEX(m_RUNTHREAD, m_pExtLogger);
	DESTROYMUTEXEX(m_THREADNAME, m_pExtLogger);
	DESTROYMUTEXEX(m_STARTSTOPTHREAD, m_pExtLogger);
	DESTROYMUTEXEX(m_ERRORCODES, m_pExtLogger);
	DESTROYMUTEXEX(m_SLEEPMUTEX, m_pExtLogger);
	DESTROYCONDEX(m_SLEEPCOND, m_pExtLogger);
	DESTROYCONDEX(m_STARTSTOPTHREADCOND, m_pExtLogger);
}
