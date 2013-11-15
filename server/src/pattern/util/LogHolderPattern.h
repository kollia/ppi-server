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

#ifndef LOGHOLDERPATTERN_H_
#define LOGHOLDERPATTERN_H_

#include <iostream>
#include <string>

#include "ILogPattern.h"

#include "../server/IClientSendMethods.h"

using namespace std;
using namespace design_pattern_world::util_pattern;
using namespace design_pattern_world::client_pattern;

class LogHolderPattern : public ILogPattern
{
public:
	/**
	 * initialization of object.<br />
	 * This object do not delete instance of giving logging process.
	 *
	 * @param loggingobject object to logging messages
	 */
	static void init(ILogPattern* loggingobject);
	/**
	 * initialize log instance with minimal log level
	 * when no logging object is used
	 *
	 * @param loglevel minimal log level
	 */
	static void init(const int loglevel);
	/**
	 * get actual instance of object
	 */
	static LogHolderPattern* instance()
	{ if(!_instance) _instance= new LogHolderPattern; return _instance; };
	/**
	 * callback routine to get information when logging process destroy
	 */
	static void usable(bool use)
	{ _instance->m_bExists= use; };
	/**
	 * callback method to inform when logging object destroy or can be used
	 *
	 * @param usable function whether can use logging process
	 */
	virtual void callback(void (*usable)(bool))
	{ usable(m_bExists); };
	/**
	 * set for all process or thread an name to identify in log-messages
	 *
	 * @param name specified name
	 * @param sendDevice sending object over which logging should running
	 */
	virtual void setThreadName(const string& threadName, IClientSendMethods* sendDevice= NULL)
	{ if(m_bExists) m_oLogging->setThreadName(threadName, sendDevice); };
	/**
	 * return name of thread from given thread-id.<br />
	 * If no logging process is given, it returning the string that no logging process exists.
	 *
	 * @param threadID id of thread
	 * @param sendDevice sending object over which logging should running
	 */
	string getThreadName(pthread_t threadID= 0, IClientSendMethods* sendDevice= NULL)
	{ if(m_bExists) return m_oLogging->getThreadName(threadID, sendDevice); else return "no logging process exists"; };
	/**
	 * to log an message
	 *
	 * @param file name from witch source file the method is called, specified with <code>__FILE__</code>
	 * @param line number of line in the source file, specified with <code>__LINE__</code>
	 * @param type defined type of log-message (<code>LOG_DEBUG, LOG_INFO, ...</code>)
	 * @param message string witch should written into log-files
	 * @param sTimeLogIdentif if this identifier be set, the message will be write only in an defined time
	 * @param sendDevice sending object over which logging should running
	 */
	virtual void log(const string& file, int line, int type, const string& message,
					const string& sTimeLogIdentif= "", IClientSendMethods* sendDevice= NULL);
	/**
	 * destructor of object
	 */
	virtual ~LogHolderPattern()
	{ _instance= NULL; };


private:
	/**
	 * whether object of logging exists
	 */
	bool m_bExists;
	/**
	 * minimal log level for output
	 */
	int m_nLogLevel;
	/**
	 * instance of object
	 */
	static LogHolderPattern* _instance;
	/**
	 * object to logging messages
	 */
	ILogPattern* m_oLogging;

	/**
	 * Constructor of singelton pattern
	 */
	LogHolderPattern() :
		m_bExists(false),
		m_nLogLevel(0x00000000), // LOG_DEBUG
		m_oLogging(NULL) {};

};


#ifndef LOG
#define LOG(type, message) LogHolderPattern::instance()->log(__FILE__, __LINE__, type, message)
#endif // LOG
#ifndef LOGEX
#define LOGEX(type, message, sendDevice) LogHolderPattern::instance()->log(__FILE__, __LINE__, type, message, "", sendDevice)
#endif // LOGEX
#ifndef TIMELOG
#define TIMELOG(type, identif, message) LogHolderPattern::instance()->log(__FILE__, __LINE__, type, message, identif)
#endif // TIMELOG
#ifndef TIMELOGEX
#define TIMELOGEX(type, identif, message, sendDevice) LogHolderPattern::instance()->log(__FILE__, __LINE__, type, message, identif, sendDevice)
#endif // TIMELOGEX

#endif /* LOGHOLDERPATTERN_H_ */
