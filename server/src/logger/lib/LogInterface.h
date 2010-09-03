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
#include "LogConnectionChecker.h"

#include "../../server/libs/client/ProcessInterfaceTemplate.h"

#include "../../pattern/util/ILogPattern.h"
#include "../../pattern/util/ILogInterfacePattern.h"

using namespace std;
using namespace design_pattern_world::server_pattern;
using namespace design_pattern_world::util_pattern;

namespace logger
{
	class LogInterface :	public util::ProcessInterfaceTemplate,
							public ILogPattern,
							public ILogInterfacePattern
	{
		public:
			/**
			 * Instantiate class of log-server.<br/>
			 * By cancel this LogInterface object, second parameter object will be also delete in parent class.
			 *
			 * @param process name of process in which the log-interface running
			 * @param connection to which server this interface should connect to send messages
			 * @param identifwait how many seconds an log with an identif string should wait to write again into the logfile
			 * @param wait whether connection should wait for correct access
			 * @return whether connection to LogProccess was correct
			 */
			static bool initial(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait);
			/**
			 * delete object of LogInterface
			 */
			static void deleteObj();
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
			virtual void setThreadName(string threadName);
			/**
			 * return name of thread from given thread-id.<br />
			 * When no parameter of thread id is given, method take actual thread.
			 *
			 * @param threadID id of thread
			 */
			virtual string getThreadName(pthread_t threadID= 0);
			/**
			 * callback method to inform when logging object destroy or can be used
			 *
			 * @param usable function whether can use logging process
			 */
			virtual void callback(void (*usable)(bool));
			/**
			 * to log an message
			 *
			 * @param file name from witch source file the method is called, specified with <code>__FILE__</code>
			 * @param line number of line in the source file, specified with <code>__LINE__</code>
			 * @param type defined type of log-message (<code>LOG_DEBUG, LOG_INFO, ...</code>)
			 * @param message string witch should written into log-files
			 * @param sTimeLogIdentif if this identifier be set, the message will be write only in an defined time
			 */
			virtual void log(string file, int line, int type, string message, string sTimeLogIdentif= "");
			/**
			 * write vector log and threadNames which was read
			 * before get connection
			 *
			 * @return whether write any message over connection
			 */
			virtual bool writeVectors();
			/**
			 * this method will be called when ConnectionChecker ending
			 */
			virtual void connectionCheckerStops()
			{ m_poChecker= NULL; };
			/**
			 * open the connection to server for sending questions
			 * <b>errorcodes:</b>
			 * <table>
			 * 	<tr>
			 * 		<td>
			 * 			0
			 * 		</td>
			 * 		<td>
			 * 			no error occurred
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			-1
			 * 		</td>
			 * 		<td>
			 * 			WARNING: connection exist before
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			1
			 * 		</td>
			 * 		<td>
			 * 			ERROR: no <code>IClientConnectArtPattern</code> be given for sending
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			2
			 * 		</td>
			 * 		<td>
			 * 			cannot connect with server, or initialization was fail
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td colspan="2">
			 * 			all other ERRORs or WARNINGs see in <code>IClientConnectArtPattern</code>
			 * 			for beginning connection by sending
			 * 		</td>
			 * 	</tr>
			 * </table>
			 *
			 * @param toopen string for open question, otherwise by null the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @return error number
			 */
			virtual int openSendConnection(string toopen= "");
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
			 * function to inform whether own object is usable
			 */
			void (*m_funcUsable)(bool);
			/**
			 * whether ConnectionChecker is running
			 */
			LogConnectionChecker* m_poChecker;


	}; // class LogInterface
} // namespace logger

#ifndef LOG
#define LOG(type, message) logger::LogInterface::instance()->log(__FILE__, __LINE__, type, message)
#endif // LOG
#ifndef TIMELOG
#define TIMELOG(type, identif, message) logger::LogInterface::instance()->log(__FILE__, __LINE__, type, message, identif)
#endif // TIMELOG

#endif /*LOGINTERFACE_H_*/
