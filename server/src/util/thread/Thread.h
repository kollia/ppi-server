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
#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>

#include "StatusLogRoutine.h"

#include "../debug.h"

#include "../../pattern/util/IErrorHandlingPattern.h"
#include "../../pattern/util/ithreadpattern.h"

#include "../../pattern/server/IClientSendMethods.h"

// reading in c't extra programmieren
// article: Tauziehen (Programmieren mit POSIX-Threads) under side 62
// adjustment from cacheline to 128 bit
#define CACHE_LINE_SIZE 128
#if defined(WIN32) && !defined(BCC32)
#define CAHCE_ALIGN __declespec(align(CACHE_LINE_SIZE))
#else
#define CACHE_ALIGN __attribute__((aligned(CACHE_LINE_SIZE), packed))
#endif

using namespace design_pattern_world;
using namespace design_pattern_world::util_pattern;
using namespace design_pattern_world::client_pattern;
using namespace std;

struct mutexnames_t
{
	/**
	 * defined name for mutex with getMutex()
	 */
	string name;
#ifdef MUTEXLOCKDEBUG
#define MUTEX_CONDITION_DEBUG
#endif
#ifdef CONDITIONSDEBUG
#ifndef MUTEX_CONDITION_DEBUG
#define MUTEX_CONDITION_DEBUG
#endif
#endif
#ifdef MUTEX_CONDITION_DEBUG
	/**
	 * thread id which have locked mutex.<br />
	 * this id is only defined if MUTEXLOCKDEBUG is defined
	 */
	pid_t threadid;
	/**
	 * mutex locked in file
	 */
	string fileLocked;
	/**
	 * mutex locked on line
	 */
	int lineLocked;
#endif
};

/**
 * globaly mutex for variables g_mMutex and g_mCondition
 */
extern pthread_mutex_t g_READMUTEX;
/**
 * all defined mutex with names and thread ids for debugging
 */
extern map<pthread_mutex_t*, mutexnames_t> g_mMutex;
/**
 * all defined conditions with names
 */
extern map<pthread_cond_t*, string> g_mCondition;

/**
 * base class for all threads.<br />
 * to use compiling with pthread's
 * application need library pthread with option -lpthread
 * by creating with make
 *
 * @autor Alexander Kolli
 * @version 1.0.0
 */
class Thread :	public virtual IThreadPattern,
				public virtual StatusLogRoutine
{
	public:
		/**
		 * error types
		 */
		enum ERRORtype
		{
			NONE= 0,
			BASIC,
			INIT,
			EXECUTE
		};
		/**
		 * creating instance of thread
		 *
		 * @param threadName Name of thread to identify in log messages
		 * @param waitInit if flag is true (default), starting thread waiting until this thread initial with method init()
		 * @param policy thread policy for scheduling
		 * @param priority new other scheduling priority for thread
		 * @param logger sending object over which logging should running
		 */
		Thread(const string& threadName, bool waitInit= true, const int policy= -1,
						const int priority= -9999, IClientSendMethods* logger= NULL);
		/**
		 * start method to running the thread paralell
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 * @return error handling object
		 */
		OVERWRITE EHObj start(void *args= NULL, bool bHold= false);
		/**
		 * to ask whether the thread should stopping.<br />
		 * This method should be call into running thread to know whether the thread should stop.
		 *
		 * @return 1 if the thread should stop otherwise 0 or -1 by error
		 */
		OVERWRITE short stopping();
		/**
		 * method to ask whether thread was initialed
		 *
		 * @return true if thread finisdhed by initialing
		 */
		int initialed();
		/**
		 * external query whether the thread is running
		 *
		 * @return 1 when thread running otherwise 0 or -1 by error
		 */
		OVERWRITE short running();
		/**
		 * sleep for seconds with condition to stop
		 *
		 * @param wait how much seconds the method should wait
		 * @param file file-name where method was calling
		 * @param line line count where method was calling
		 * @return ETIMEDOUT by success or 0 when thread should stopping
		 */
		int sleep(const unsigned int wait, string file= "", int line= 0);
		/**
		 * sleep for microseconds with condition to stop
		 *
		 * @param wait how much microseconds the method should wait
		 * @param file file-name where method was calling
		 * @param line line count where method was calling
		 * @return ETIMEDOUT by success or 0 when thread should stopping
		 */
		int usleep(const useconds_t wait, string file= "", int line= 0);
		/**
		 * sleep for nanoseconds with condition to stop
		 *
		 * @param req how much nanoseconds the method should wait
		 * @param rem return remaining nanoseconds, otherwise NULL
		 * @param file file-name where method was calling
		 * @param line line count where method was calling
		 * @return ETIMEDOUT by success or 0 when thread should stopping
		 */
		int nanosleep(const struct timespec *req, struct timespec *rem, string file= "", int line= 0);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return Thread::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * the thread will be uncoupled from the starting thread
		 *
		 * @return error level if exist, otherwise 0
		 */
		EHObj detach();
		/**
		 * return id from thread
		 *
		 * @return id
		 */
		static pid_t gettid();
		/**
		 * returning type of error<br />
		 *         <table>
		 *           <tr>
		 *             <td>
		 *               NONE
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               no error is occurred
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               BASIC
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               error caused inside main thread routine<br />
		 *               (see codes like <code>getErrorCode()</code>
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               INIT
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               error/warning caused inside initialization of extended thread object
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               EXECUTE
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               error/warning caused by execute extended thread object
		 *             </td>
		 *           </tr>
		 *         </table>
		 *
		 * @return error type
		 */
		ERRORtype getErrorType();
		/**
		 * returning current error handling object
		 *
		 * @return object of error handling
		 */
		OVERWRITE EHObj getErrorHandlingObj() const
		{ return m_pError; };
		/**
		 * return error code differ by error type<br />
		 * INIT and EXECUTE error types handled by extended classes,<br />
		 * BASIC errors see as follow<br />
		 *         <table>
		 *           <th>
		 *             <td colspan="3">
		 *               BASIC error codes from thread
		 *             </td>
		 *           </th>
		 *           <tr>
		 *             <td>
		 *               1
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               cannot start thread again when running
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               0
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               no error is occurred
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -1
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               cannot create an new thread
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -2
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               cannot join correctly to thread until thread running
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -3
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               cannot join correctly to thread until waiting for running thread by stopping
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -4
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               exception was throwing while start method running
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -5
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               exception throwing while initialization from extended thread object running
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -6
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               exception throwing while execute extended thread
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -7
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               -
		 *             </td>
		 *           </tr>
		 *           <tr>
		 *             <td>
		 *               exception was throwing while thread running
		 *             </td>
		 *           </tr>
		 *         </table>
		 *
		 * @return error code
		 */
		int getErrorCode();
		/**
		 * possible to set other logging application than standard from LogHolderPattern
		 *
		 * @param log other logging object
		 */
		void setOtherLogger(IClientSendMethods* log)
		{ m_pExtLogger= log; };
		/**
		 * creating an new mutex to lock the thread or an hole part
		 *
		 * @param name name of mutex for logging information
		 * @param logger other extern logger than LogHolderPattern
		 * @return mutex variable
		 */
   		static pthread_mutex_t* getMutex(const string& name, IClientSendMethods* logger= NULL);
   		/**
   		 * creating an new condition to wait for an other thread
   		 *
		 * @param name name of condition for logging information
		 * @param logger other extern logger than LogHolderPattern
		 * @return mutex variable
		 */
   		static pthread_cond_t* getCondition(const string& name, IClientSendMethods* logger= NULL);
   		/**
   		 * locking thread
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param mutex mutex variable which should be locked
		 * @param logger other extern logger than LogHolderPattern
   		 */
   		static int mutex_lock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger= NULL);
   		/**
   		 * try to lock an thread.<br />
   		 * If THE LOCK IS successful, returnvalue is 0, by locking from an other thread,
   		 * returnvalue is EBUSY, else returnvalue is an ERROR seen in variable erno
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param mutex mutex variable which should be unlocked
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, all other is an ERROR or EBUSY
   		 */
   		static int mutex_trylock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger= NULL);
   		/**
   		 * unlock thread
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param mutex mutex variable which should be unlocked
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, all other is an ERROR
   		 */
   		static int mutex_unlock(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger= NULL);
   		/**
   		 * method should wait to get condition
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param cond pointer to condition
   		 * @param mutex pointer to mutex in witch he should wait
   		 * @param time how much time the method maximal should wait if set
   		 * @param absolute if time be set, this parameter define whether the time is relative or absolute (default:true)
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, ETIMEDOUT if end of time reached, otherwise an error occured
   		 */
   		static int conditionWait(const string& file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex,
   						const struct timespec *time= NULL, const bool absolute= true, IClientSendMethods* logger= NULL);
   		/**
   		 * method should wait to get condition
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param cond pointer to condition
   		 * @param mutex pointer to mutex in witch he should wait
   		 * @param sec how much seconds the method maximal should wait if set
   		 * @param absolute if time be set, this parameter define whether the time is relative or absolute (default:true)
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, ETIMEDOUT if end of time reached, otherwise an error occured
   		 */
   		static int conditionWait(const string& file, int line, pthread_cond_t* cond, pthread_mutex_t* mutex,
   						const time_t sec, const bool absolute= true, IClientSendMethods* logger= NULL);
   		/**
   		 * arose one or more threads which wating for given condition
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param cond pointer to condition
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, otherwise an error occured
   		 */
   		static int arouseCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger= NULL);
   		/**
   		 * arose all threads which waiting for the given condition
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param cond pointer to condition
		 * @param logger other extern logger than LogHolderPattern
   		 * @return 0 if successful, otherwise an error occured
   		 */
   		static int arouseAllCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger= NULL);
   		/**
   		 * destroy mutex in the map vector and give it free to hold it petit for performance
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param mutex mutex variable which should deleted
		 * @param logger other extern logger than LogHolderPattern
   		 */
   		static void destroyMutex(const string& file, int line, pthread_mutex_t *mutex, IClientSendMethods* logger= NULL);
   		static void destroyAllMutex();
   		/**
   		 * destroy condition in map vector and give it free
   		 *
   		 * @param file in which file this method be called
   		 * @param line on which line in the file this method be called
   		 * @param cpmd pointer of condition which should deleted
		 * @param logger other extern logger than LogHolderPattern
   		 */
   		static void destroyCondition(const string& file, int line, pthread_cond_t *cond, IClientSendMethods* logger= NULL);
   		static void destroyAllConditions();
   		/**
   		 * return name of mutex
   		 *
   		 * @param mutex pointer of mutex
		 * @param logger other extern logger than LogHolderPattern
   		 * @return name of mutex
   		 */
   		static string getMutexName(pthread_mutex_t *mutex, IClientSendMethods* logger= NULL);
   		/**
   		 * return name of condition
   		 *
   		 * @param cond pointer of condition
		 * @param logger other extern logger than LogHolderPattern
   		 * @return name of condition
   		 */
   		static string getConditionName(pthread_cond_t *cond, IClientSendMethods* logger= NULL);
   		/**
   		 * set the running application flag to false.
		 * This flag is set for destroyMutex or destroyCondition
		 * because by destroying and the end of the app is reached
		 * the global variable g_mCondition will be destroy before
		 * the last destroy is done. So it gives an "Segmentation fault"
   		 */
   		static void applicationStops();
   		/**
   		 * method returning name of thread
   		 *
   		 * @return name of thread
   		 */
		string getThreadName() const;
		/**
		 * returning thread id from OS
		 *
		 * @return thread id
		 */
		pid_t getThreadID()
		{ return m_nThreadId; };
		/**
		 * return POSIX thread from creation
		 *
		 * @return thread id
		 */
		pthread_t getPosixThreadID()
		{ return m_nPosixThreadID; };
		/**
		 * set new scheduling priority and or policy.<br />
		 * from inside every time
		 * and outside running thread
		 * only when no thread running
		 *
		 * @param policy thread policy for scheduling
		 * @param priority scheduling priority
		 * @return object of error handling
		 */
		EHObj setSchedulingParameter(int policy, int priority);
		/**
		 * get setting scheduling parameters
		 * of policy and priority
		 *
		 * @param policy thread policy for scheduling
		 * @param priority scheduling priority
		 */
		OVERWRITE void getSchedulingParameter(int& policy, int& priority)
		{ policy= m_nSchedPolicy; priority= m_nSchedPriority; };
		/**
		 * destructor of class Thread
		 */
		virtual ~Thread();

	protected:
		/**
		 * abstract method to initial the thread
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		virtual EHObj init(void *args)=0;
		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending when method ending with return value 0
		 * and the method stop() isn't called.
		 *
		 * @return whether execute should start again
		 */
		virtual bool execute()=0;
		/**
		 * abstract method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending()=0;

	protected:
		/**
		 * ThreadErrorHandling object for error/warning,
		 * usable for all depend classes
		 */
		EHObj m_pError;


	private:
		/**
		 * thrad id from system (OS/Linux)
		 */
		pid_t m_nThreadId;
		/**
		 * thread ID from current thread
		 */
		pthread_t m_nPosixThreadID;
		/**
		 * should thread waiting for initaliazen of thread
		 */
		bool m_bWaitInit;
		/**
		 * user defined param(s) by beginning thread with ->start()
		 */
		void *m_pArgs;
		/**
		 * flag to should stopping thread
		 */
		bool m_bStop;
		/**
		 * flag set when thread has been initialed
		 */
		bool m_bInitialed;
		/**
		 * flag set while running thread
		 */
		bool m_bRun;
		/**
		 * if flag is set, starting thread
		 * waiting while this thread running
		 */
		bool m_bHold;
		/**
		 * remaining nanoseconds for method <code>nanosleep()</code>
		 * when sleep method wake up earlier
		 */
		timespec m_nRemainSecs;
		/**
		 * error type of error code when not 0
		 */
		ERRORtype m_eErrorType;
		/**
		 * error code from thread when exist
		 * elsewhere 0
		 */
		int m_nErrorCode;
		/**
		 * scheduling policy of actual thread
		 */
		int m_nSchedPolicy;
		/**
		 * scheduling priority of actual thread
		 */
		int m_nSchedPriority;
		/**
		 * Specified name of thread in constructor
		 */
		string m_sThreadName;
		/**
		 * whether thread is running
		 */
		pthread_mutex_t* m_RUNTHREAD;
		/**
		 * lock to get thread name
		 */
		pthread_mutex_t* m_THREADNAME;
		/**
		 * mutex lock for start or stop thread
		 */
		pthread_mutex_t* m_STARTSTOPTHREAD;
		/**
		 * mutex lock for error codes
		 */
		pthread_mutex_t* m_ERRORCODES;
		/**
		 * mutex lock for sleeping
		 */
		pthread_mutex_t* m_SLEEPMUTEX;
		/**
		 * condition for sleeping
		 */
		pthread_cond_t* m_SLEEPCOND;
		/**
		 * condition for start or stop thread
		 */
		pthread_cond_t* m_STARTSTOPTHREADCOND;
		/**
		 * whether application is running.
		 * This flag is set for destroyMutex or destroyCondition
		 * because by destroying and the end of the app is reached
		 * the global variable g_mCondition will be destroy before
		 * the last destroy is done. So it gives an "Segmentation fault"
		 */
		static bool m_bAppRun;
		/**
		 * 742
		 * first mutex creation of global mutex POSITIONSTATUS makes error
		 * since gcc (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
		 *       g++ (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3
		 *      so write mutex for first time in an buffer
		 *      (gcc (Debian 4.4.5-8) 4.4.5 made no problem's)
		 */
		static bool m_bGlobalObjDefined;
		/**
		 * other logging tool when nessasary
		 */
		IClientSendMethods* m_pExtLogger;

		/**
		 * private copy constructor for not allowed copy
		 *
		 * @param x object for copy
		 */
		Thread(const Thread& x);
		/**
		 * private assignment operator for not allowed allocation
		 *
		 * @param x object for assignment
		 * @return own object
		 */
		Thread& operator=(const Thread& x);
		/**
		 * set new scheduling priority and or policy
		 *
		 * @param policy thread policy for scheduling
		 * @param priority scheduling priority
		 * @return object of error handling
		 */
		EHObj setSchedulingParameterInline(int policy, int priority);
		/**
		 * run method started inside entryPoint to call
		 * execute in an loop
		 */
		void run();
		/**
		 * entry point to creating thread
		 *
		 * @param pthis pointer of own object
		 */
		static void * EntryPoint(void* pthis);

};

#define LOCK(mutex) Thread::mutex_lock(__FILE__, __LINE__, mutex)
#define LOCKEX(mutex, logger) Thread::mutex_lock(__FILE__, __LINE__, mutex, logger)
#define TRYLOCK(mutex) Thread::mutex_trylock(__FILE__, __LINE__, mutex)
#define TRYLOCKEX(mutex, logger) Thread::mutex_trylock(__FILE__, __LINE__, mutex, logger)
#define UNLOCK(mutex) Thread::mutex_unlock(__FILE__, __LINE__, mutex)
#define UNLOCKEX(mutex, logger) Thread::mutex_unlock(__FILE__, __LINE__, mutex, logger)
#define CONDITION(cond, mutex) Thread::conditionWait(__FILE__, __LINE__, cond, mutex)
#define CONDITIONEX(cond, mutex, logger) Thread::conditionWait(__FILE__, __LINE__, cond, mutex, (const struct timespec*)NULL, true, logger)
#define TIMECONDITION(cond, mutex, time) Thread::conditionWait(__FILE__, __LINE__, cond, mutex, time, true)
#define TIMECONDITIONEX(cond, mutex, time, logger) Thread::conditionWait(__FILE__, __LINE__, cond, mutex, time, true, logger)
#define RELTIMECONDITION(cond, mutex, time) Thread::conditionWait(__FILE__, __LINE__, cond, mutex, time, false)
#define RELTIMECONDITIONEX(cond, mutex, time, logger) Thread::conditionWait(__FILE__, __LINE__, cond, mutex, time, false, logger)
#define AROUSE(cond) Thread::arouseCondition(__FILE__, __LINE__, cond)
#define AROUSEEX(cond, logger) Thread::arouseCondition(__FILE__, __LINE__, cond, logger)
#define AROUSEALL(cond) Thread::arouseAllCondition(__FILE__, __LINE__, cond)
#define DESTROYMUTEX(mutex) Thread::destroyMutex(__FILE__, __LINE__, mutex)
#define DESTROYMUTEXEX(mutex, logger) Thread::destroyMutex(__FILE__, __LINE__, mutex, logger)
#define DESTROYCOND(cond) Thread::destroyCondition(__FILE__, __LINE__, cond)
#define DESTROYCONDEX(cond, logger) Thread::destroyCondition(__FILE__, __LINE__, cond, logger)
#define SLEEP(wait) sleep(wait, __FILE__, __LINE__)
#define USLEEP(wait) usleep(wait, __FILE__, __LINE__)
#define NANOSLEEP(wait) nanosleep(req, rem, __FILE__, __LINE__)

#endif /*THREAD_H_*/
