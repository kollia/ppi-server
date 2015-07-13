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
#ifndef DATABASETHREAD_H_
#define DATABASETHREAD_H_

#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "../util/thread/Thread.h"
#include "../util/structures.h"
#include "../util/smart_ptr.h"

#include "../pattern/util/IPPIDatabasePattern.h"

#include "../pattern/server/IServerCommunicationStarterPattern.h"

#include "../server/libs/client/ppi_server_clients.h"

using namespace std;
using namespace design_pattern_world;
using namespace design_pattern_world::server_pattern;

namespace ppi_database
{

	/**
	 * class representig connection to internal
	 * or an mySql database
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class DatabaseThread : public Thread
	{
	public:
		/**
		 * Instantiate class of database
		 *
		 * @param dbDir working directory of database where write the files
		 * @param confDir path of configuration files for default chips settings
		 * @param properties defined properties from server.conf
		 */
		static void initial(string dbDir, string confDir, IPropertyPattern* properties);
		/**
		 * return object of database
		 *
		 * @return database object
		 */
		IPPIDatabasePattern* getDatabaseObj()
		{ return m_pDatabase; };
		/**
		 * method to ask database whether database file is loaded
		 *
		 * @return wheter file is loaded
		 */
		bool isDbLoaded() const;
		/**
		 * return instance of database object
		 *
		 * @return database object
		 */
		static DatabaseThread* instance()
					{ return _instance; };
		/**
		 * get debug info for benchmark
		 *
		 * @param server ID from one wire server
		 * @return vector of string for all devices
		 */
		vector<string> getDebugInfo(const unsigned short server);
		/**
		 * set client starter for communication with any client
		 *
		 * @param starter pointer to an IServerCommunicationStarterPattern
		 */
		void setCommunicator(IServerCommunicationStarterPattern* starter)
		{ m_pStarter= starter; };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return DatabaseThread::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling routine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * delete single instance of DatabaseThread object
		 */
		static void deleteObj();

	protected:
		/**
		 * method to initial the thread .<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		OVERWRITE EHObj init(void *args= NULL);
		/**
		 * method to running thread .<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return whether should start thread again
		 */
		OVERWRITE bool execute()
		{ return m_pDatabase->execute(); };
		/**
		 * method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending()
		{ m_pDatabase->ending(); };

	private:
		/**
		 * single instance of database
		 */
		static DatabaseThread* _instance;
		/**
		 * Hardware database
		 */
		IPPIDatabasePattern* m_pDatabase;
		/**
		 * mutex lock to know whether database file is loaded
		 */
		pthread_mutex_t* m_DBLOADED;
		/**
		 * whether database file is loaded
		 */
		bool m_bDbLoaded;
		/**
		 * starter pool for communication with any client
		 */
		IServerCommunicationStarterPattern* m_pStarter;

		/**
		 * private initialization of Database
		 *
		 * @param dbDir working directory of database where write the files
		 * @param confDir path of configuration files for default chips settings
		 * @param measureName setting measure-name in file mesure.conf
		 * @param mbyte write after MB an new database file
		 */
		DatabaseThread(string dbdir, string confDir, IPropertyPattern* properties);
		/**
		 * destruct of Database
		 */
		virtual ~DatabaseThread();
	};

}

#endif /*DATABASETHREAD_H_*/
