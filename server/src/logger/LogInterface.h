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
#ifndef LOGINTERFACE_H_
#define LOGINTERFACE_H_

//#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#include <string>
#include <vector>

#include "logstructures.h"

#include "../util/RunningStoppingInterfaceTemplate.h"

using namespace std;
using namespace design_pattern_world::server_pattern;

namespace logger
{
	class LogInterface : public util::ProcessInterfaceTemplate
	{
		public:
			/**
			 * Instantiate class of log-server
			 *
			 * @param process name of process in which the log-interface running
			 * @param connection to which server this interface should connect to send messages
			 * @param identifwait how many seconds an log with an identif string should wait to write again into the logfile
			 * @param wait whether connection should wait for correct access
			 * @return whether connection to LogProccess was correct
			 */
			static bool init(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait);
			/**
			 * method to get single instance of interface
			 */
			static LogInterface *instance()
			{ return _instance; };
			/**
			 * set for all process or thread an name to identify in log-messages
			 *
			 * @param name specified name
			 */
			void setThreadName(string threadName);
			//string getThreadName(pthread_t threadID= 0);
			/**
			 * to log an message
			 *
			 * @param file name from witch source file the method is called, specified with <code>__FILE__</code>
			 * @param line number of line in the source file, specified with <code>__LINE__</code>
			 * @param type defined type of log-message (<code>LOG_DEBUG, LOG_INFO, ...</code>)
			 * @param message string witch should written into log-files
			 * @param sTimeLogIdentif if this identifier be set, the message will be write only in an defined time
			 */
			void log(string file, int line, int type, string message, string sTimeLogIdentif= "");
			/**
			 * destructor of log interface
			 */
			virtual ~LogInterface()
			{ _instance= NULL; };

		protected:
			/**
			 * creating instance of LogInterface
			 *
			 * @param process name of process in which the log-interface running
			 * @param connection to which server this interface should connect to send messages
			 * @param identifwait how many seconds an log with an identif string should wait to write again into the logfile
			 * @param wait whether connection should wait for correct access
			 */
			LogInterface(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait);
			/**
			 * check whether connection to logserver is open
			 *
			 * @return whether connection is given
			 */
			bool openedConnection();
			/**
			 * write threadName to log server
			 *
			 * @param thread structure of thread from <code>setThreadName()</code>, contains thread id and name
			 */
			void writethread(const threadNames& thread);
			/**
			 * write log information to log server
			 *
			 * @param log structure of log message from <code>log()</code>, contains parameter and id's
			 */
			void writelog(const log_t& log);

		private:
			/**
			 * instance of single logging interface
			 */
			static LogInterface* _instance;
			/**
			 * status whether connection is open<br />
			 * 0	-	no connection<br />
			 * 1	-	test connection<br />
			 * 2	-	connection is open<br />
			 */
			short m_nOpen;
			/**
			 * vector for all thread names with id
			 */
			vector<struct threadNames> m_vtThreads;
			/**
			 * vector for all getting log messages
			 */
			vector<struct log_t> m_vtLogs;
			/**
			 * all logging threads with identif strings and the last logging time
			 */
			vector<struct timelog_t> m_vtTimeLog;
			/**
			 * how many time an log with an identif string should wait to write again into the logfile
			 */
			const int m_nTimeLogWait;
			/**
			 * mutex lock to write log message or thread name into vector
			 */
			pthread_mutex_t* m_WRITELOOP;

			/**
			 * write vector log and threadNames which was read
			 * befor get connection
			 */
			void writeVectors();

	}; // class LogInterface
} // namespace logger

#define LOG(type, message) logger::LogInterface::instance()->log(__FILE__, __LINE__, type, message)
#define TIMELOG(type, identif, message) logger::LogInterface::instance()->log(__FILE__, __LINE__, type, message, identif)

#endif /*LOGINTERFACE_H_*/
