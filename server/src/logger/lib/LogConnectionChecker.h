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

#ifndef CONNECTIONCHECKER_H_
#define CONNECTIONCHECKER_H_

#include "../../../util/thread/Thread.h"

#include "../../../pattern/util/ILogInterfacePattern.h"

namespace logger {

//using namespace util;
using namespace design_pattern_world::util_pattern;

class LogConnectionChecker: public Thread {
public:
	/**
	 * constructor of ConnectionChecker
	 */
	LogConnectionChecker(ILogInterfacePattern* object, pthread_mutex_t* writeloop) :
		Thread("LogConnectionChecker", 0),
		m_poStarter(object),
		m_WRITELOOP(writeloop)
	{};

protected:
	/**
	 * method to initial the thread.<br />
	 * this method will be called before running
	 * the method execute and do actually nothing
	 *
	 * @param args user defined parameter value or array,<br />
	 * 				Coming as void pointer from the external call
	 * 				method start(void *args).
	 * @return defined error code from extended class
	 */
	virtual int init(void *args)
	{ return 0; };
	/**
	 * method to running thread and check every second to have connection
	 * to send logging messages.
	 *
	 * @return defined error code from extended class
	 */
	virtual int execute();
	/**
	 * method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() {};

private:
	/**
	 * starting object LogInterface to writing log messages
	 * after connection is available
	 */
	ILogInterfacePattern* m_poStarter;
	/**
	 * mutex lock to write log message or thread name into vector
	 */
	pthread_mutex_t* m_WRITELOOP;
};

}

#endif /* CONNECTIONCHECKER_H_ */
