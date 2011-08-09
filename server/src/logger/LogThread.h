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
#ifndef LOGTHREAD_H_
#define LOGTHREAD_H_

//#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#include <string>
#include <vector>

#include "lib/logstructures.h"

#include "../../util/thread/Thread.h"

#include "../../pattern/util/ILogPattern.h"

using namespace std;

class LogThread : 	public Thread,
					public ILogPattern
{
	public:
		/**
		 * creating instance of LogThread
		 *
		 * @param check whether polling execute should check for identif strings
		 * @param asServer whether LogThread running as server, or logging directly on harddisk
		 */
		LogThread(bool check, bool asServer= true);
		/**
		 * start method to running the thread paralell
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 */
		virtual int start(void *args= NULL, bool bHold= false);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(bool bWait= true);
		/**
		 * set propertys for logging thread
		 *
		 * @param logFile name of log file with path
		 * @param minLogLevel minimal logging level (LOG_DEBUG, LOG_WARNING, LOG_ERROR, ...)
		 * @param logAllSec logging after seconds for log messages with identif string
		 * @param writeLogDays writing an new log file after this days
		 * @param nDeleteAfter after how much days the old log files should be delete
		 */
		virtual void setProperties(string logFile, int minLogLevel, int logAllSec, int writeLogDays, const unsigned short nDeleteAfter);
		/**
		 * set name of thread to running thread-id
		 *
		 * @param threadName name of thread
		 */
		virtual void setThreadName(const string& threadName)
		{ setThreadName(threadName, pthread_self()); };
		/**
		 * set name of thread to specified thread-id
		 *
		 * @param threadName name of thread
		 * @param threadID id of thread
		 */
		void setThreadName(const string& threadName, const pthread_t threadID);
		/**
		 * return name of thread from given thread-id.<br />
		 * If no threadID is given, it returning the name of the actual running thread.
		 *
		 * @param threadID id of thread
		 */
		virtual string getThreadName(const pthread_t threadID= 0);
		bool ownThread(string threadName, pid_t currentPid);
		/**
		 * write log message into files
		 *
		 * @param file name of source file witch call this method
		 * @param line the line of the source file
		 * @param type which type of message should be written
		 * @param sTimeLogIdentif identification for messages whitch are not be write by every call (default= "" -> write every call)
		 */
		virtual void log(const string& file, const int line, const int type, const string& message, const string& sTimeLogIdentif= "");
		/**
		 * write log message into files
		 *
		 * @param messageStruct structure of log message which contains all info (file, line, message, thread-id, ...)
		 */
		void log(const log_t& messageStruct);
		/**
		 * callback method to inform when logging object destroy or can be used
		 *
		 * @param usable function whether can use logging process
		 */
		virtual void callback(void (*usable)(bool))
		{ /* not used */ };

	protected:
		/**
		 * vector for all thread names with id
		 */
		vector<struct threadNames> m_vtThreads;
		/**
		 * vector for all getting log messages
		 */
		auto_ptr<vector<struct log_t> > m_pvtLogs;

		/**
		 * this method will be called before running
		 * the method execute to initial class
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual int init(void *arg);
		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param error code for not correctly done
		 */
		virtual int execute();
		/**
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();

	private:
		bool m_bAsServer;
		/**
		 * whether polling execute should check for identif strings.<br />
		 * This boolean will be set to false if the check is made by an interface.
		 */
		bool m_bIdentifCheck;
		/**
		 * after how much days the old log files should be delete.<br />
		 * if variable is 0, do not delete any files
		 */
		unsigned short m_nDeleteDays;
		/**
		 * next time to check whether files exist to delete
		 */
		time_t m_nNextDeleteTime;
		int m_nMinLogLevel;
		/**
		 * prefix of log file
		 */
		string m_sLogFilePrefix;
		/**
		 * path of all log files
		 */
		string m_sLogFilePath;
		time_t m_tmbegin;
		time_t m_tmWriteLogDays;
		string m_sLogFile;
		string m_sCurrentLogFile;
		/**
		 * how many time an log with an identif string should wait to write again into the logfile
		 */
		int m_nTimeLogWait;
		pthread_mutex_t* m_READTHREADS;
		/**
		 * mutex lock for reading log messages
		 */
		pthread_mutex_t* m_READLOGMESSAGES;
		/**
		 * condition for waiting until message vector is filled
		 */
		pthread_cond_t* m_READLOGMESSAGESCOND;
		/**
		 * all logging threads with identif strings and the last logging time
		 */
		vector<struct timelog_t> m_vtTimeLog;

		auto_ptr<vector<log_t> > getLogVector();
};

#endif /*LOGTHREAD_H_*/
