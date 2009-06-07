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

#include "../util/Thread.h"

using namespace std;

struct log
{
	time_t tmnow;
	string file;
	int line;
	pthread_t thread;
	pid_t pid;
	pid_t tid;
	int type;
	string message;
	string identif;
};

struct timelog_t
{
	pthread_t thread;
	string identif;
	string file;
	int line;
	time_t tmold;
};

struct threadNames
{
	unsigned int count;
	pthread_t thread;
	string name;
};

class LogThread : public Thread
{
	public:
		/**
		 * instanciate class of log-server
		 *
		 * @param asServer whether LogThread running as server, or logging directly on harddisk
		 */
		static LogThread *instance(bool asServer= true);
		/**
		 * start method to running the thread paralell
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 */
		virtual void* start(void *args= NULL, bool bHold= false);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void *stop(bool bWait= true);
		void setProperties(string logFile, int minLogLevel, int logAllSec, int writeLogDays);
		void setThreadName(string threadName);
		string getThreadName(pthread_t threadID= 0);
		bool ownThread(string threadName, pid_t currentPid);
		void log(string file, int line, int type, string message, string sTimeLogIdentif= "");

	protected:
		vector<struct threadNames> m_vtThreads;
		vector<struct log> *m_pvtLogs;

		/**
		 * creating instance of LogThread
		 *
		 * @param asServer whether LogThread running as server, or logging directly on harddisk
		 */
		LogThread(bool asServer= true);
		virtual bool init(void *arg);
		virtual void execute();
		virtual void ending();

	private:
		bool m_bAsServer;
		int m_nMinLogLevel;
		string m_sConfLogFile;
		time_t m_tmbegin;
		time_t m_tmWriteLogDays;
		string m_sLogFile;
		string m_sCurrentLogFile;
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
		vector<struct timelog_t> m_vtTimeLog;

		vector<struct log> *getLogVector();
};

#define AKDEBUG 0
#define AKINFO  1
#define AKSERVER 2
#define AKWARNING 3
#define AKERROR 4
#define AKALERT 5

#define LOG(type, message) LogThread::instance()->log(__FILE__, __LINE__, type, message)
#define TIMELOG(type, identif, message) LogThread::instance()->log(__FILE__, __LINE__, type, message, identif)

#endif /*LOGTHREAD_H_*/
