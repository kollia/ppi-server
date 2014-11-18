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
#ifndef NEEDDBCHANGES_H_
#define NEEDDBCHANGES_H_

#include <string>

#include "../../util/thread/Thread.h"

#include "../../pattern/server/IClientConnectArtPattern.h"

#include "DbInterface.h"

namespace ppi_database {

using namespace util;
using namespace server_pattern;

class NeedDbChanges : public Thread {
public:
	/**
	 * initial single pattern instance of object to know
	 * which time any changes of values in database exist
	 * to refresh client.<br/>
	 * By cancel this LogInterface object, second parameter object will be also delete in parent class.
	 *
	 * @param process name of process in which created
	 * @param connection to which server the database interface should connect to send question for changing
	 * @param bWait if flag is true (default), starting thread waiting until this thread initial with method init()
	 */
	static bool initial(const string& process, IClientConnectArtPattern* connection, const bool bWait= true);
	/**
	 * delete instance of object
	 */
	static void deleteObj();
	/**
	 * returning single instance of object
	 *
	 * return instance
	 */
	static NeedDbChanges* instance()
	{ return _instance; }
	/**
	 * asking whether any value is changed in database
	 *
	 * @param actualized actual state of getting changes (first value should be 0)
	 * @return new actual state of changes
	 */
	unsigned short isEntryChanged(unsigned short actualized);
	/**
	 * virtual destructor of object
	 */
	virtual ~NeedDbChanges()
	{ delete m_oConnection; };

protected:
	/**
	 * Constructor to create instance of object to know
	 * which time any changes of values in database exist
	 * to refresh client.<br/>
	 * By cancel this LogInterface object, second parameter object will be also delete in parent class.
	 *
	 * @param process name of process in which created
	 * @param connection to which server the database interface should connect to send question for changing
	 * @param bWait if flag is true, starting thread waiting until this thread initial with method init()
	 */
	NeedDbChanges(const string& process, IClientConnectArtPattern* connection, const bool bWait);
	/**
	 * method to initial the thread in the class.<br />
	 * this method will be called before running
	 * the method execute
	 *
	 * @param args user defined parameter value or array,<br />
	 * 				comming as void pointer from the external call
	 * 				method start(void *args).
	 * @return object of error handling
	 */
	virtual EHObj init(void *args);
	/**
	 * method to running thread in the extended class.<br />
	 * This method starting again when ending without an sleeptime
	 * if the method stop() isn't call.
	 *
	 * @return whether should start thread again
	 */
	virtual bool execute();
	/**
	 * dummy method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() { delete m_oDb; };

private:
	/**
	 * single instance of object
	 */
	static NeedDbChanges* _instance;
	/**
	 * name of process in which running
	 */
	string m_sProcess;
	/**
	 * to which server the database interface should connect to send question for changing
	 */
	IClientConnectArtPattern* m_oConnection;
	/**
	 * instance of interface to database
	 */
	DbInterface* m_oDb;
	/**
	 * counter for changing in database.<br />
	 * all time if thread coming back from database process
	 * this counter will become one higher.
	 */
	unsigned short m_nChanged;
	/**
	 * lock running count
	 */
	pthread_mutex_t* m_CHANGEQUESTION;
	/**
	 * condition to waiting for changes in database
	 */
	pthread_cond_t* m_CHANGEQUESTIONCOND;
};

}

#endif /* NEEDDBCHANGES_H_ */
