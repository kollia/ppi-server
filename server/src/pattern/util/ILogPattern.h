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

#ifndef ILOGPATTERN_H_
#define ILOGPATTERN_H_


#include <string>

#ifndef LOG_DEBUG
#define LOG_DEBUG 0x000 // 00000000
#endif // LOG_DEBUG
#ifndef LOG_INFO
#define LOG_INFO 0x001 // 00000001
#endif // LOG_INFO
#ifndef LOG_WARNING
#define LOG_WARNING 0x002 // 00000010
#endif // LOG_WARNING
#ifndef LOG_ERROR
#define LOG_ERROR 0x003 // 00000011
#endif // LOG_ERROR
#ifndef LOG_ALERT
#define LOG_ALERT 0x004 // 00000100
#endif // LOG_ALERT

using namespace std;

namespace design_pattern_world
{
	namespace util_pattern
	{
	/**
	 * pattern class for an logging object
	 *
	 * @autor Alexander Kolli
	 * @version 1.0.0
	 */
		class ILogPattern
		{
		public:
			/**
			 * set for all process or thread an name to identify in log-messages
			 *
			 * @param name specified name
			 */
			virtual void setThreadName(const string& threadName)= 0;
			/**
			 * return name of thread from given thread-id.<br />
			 * When no parameter of thread id is given, method take actual thread.
			 * If no logging process is given, it returning the string that no logging process exists.
			 *
			 * @param threadID id of thread
			 */
			virtual string getThreadName(const pthread_t threadID= 0)= 0;
			/**
			 * to log an message
			 *
			 * @param file name from witch source file the method is called, specified with <code>__FILE__</code>
			 * @param line number of line in the source file, specified with <code>__LINE__</code>
			 * @param type defined type of log-message (<code>LOG_DEBUG, LOG_INFO, ...</code>)
			 * @param message string witch should written into log-files
			 * @param sTimeLogIdentif if this identifier be set, the message will be write only in an defined time
			 */
			virtual void log(const string& file, const int line, const int type, const string& message, const string& sTimeLogIdentif= "")= 0;
			/**
			 * callback method to inform when logging object destroy or can be used
			 *
			 * @param usable function whether can use logging process
			 */
			virtual void callback(void (*usable)(bool))= 0;
			/**
			 * Dummy destructor for design pattern
			 */
			virtual ~ILogPattern() {};
		};
	}
}

#endif /* ILOGPATTERN_H_ */
