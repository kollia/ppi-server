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
#ifndef SERVERPROCESS_H_
#define SERVERPROCESS_H_

#include <iostream>
#include <sys/io.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "../../../util/process/process.h"

//#include "../../../ports/measureThread.h"

#include "../../../pattern/util/ipropertypattern.h"
#include "../../../pattern/server/IServerPattern.h"
#include "../../../pattern/server/IServerCommunicationStarterPattern.h"
#include "../../../pattern/server/IServerConnectArtPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	using namespace util;

	class ServerProcess :	virtual public IServerPattern,
							virtual public Process
	{
		public:
			/**
			 * initialization of class ServerThread.<br />
			 * By cancel this ServerProcess object, second and third parameter objects will be also delete
			 * and third object in parent class.
			 *
			 * @param processName new name of process
			 * @param starter object of IServerCommunicationStarterPattern to starter communication threads
			 * @param connect art of server connection
			 * @param extcon on which connection from outside the process is reachable
			 * @param open string for open connection, otherwise by null string the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ServerProcess(string processName, const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect,
										IClientConnectArtPattern* extcon= NULL, const string& open= "", const bool wait= true);
			/**
			 * initialization of class ServerThread.<br />
			 * Object delete by ending instance of ServerCommunciationStarter
			 *
			 * @param starter object of IServerCommunicationStarterPattern to starter communication threads
			 * @param connect art of server connection
			 * @param extcon on which connection from outside the process is reachable
			 * @param open string for open connection, otherwise by null string the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ServerProcess(const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect, IClientConnectArtPattern* extcon= NULL,
												const string& open= "", const bool wait= true);
			/**
			 * return name of server
			 *
			 * @return name
			 */
			virtual string getName() const;
			/**
			 * returning current error handling object
			 *
			 * @return object of error handling
			 */
			OVERWRITE EHObj getErrorHandlingObj() const
			{ return m_pSocketError; };
			/**
			 * return factory of ServerCommunicationStarter
			 *
			 * @return actual ServerCommunicationStarter
			 */
			IServerCommunicationStarterPattern* getCommunicationFactory() const
			{ return m_pStarterPool; };
			/**
			 * allow new connections from any client
			 *
			 * @param allow whether connections are allowed
			 */
			virtual void allowNewConnections(const bool allow);
			/**
			 * ask for whether new connections are allowed
			 *
			 * @return whether connection allowed
			 */
			bool connectionsAllowed();
			/**
			 * close server connection on port
			 */
			void close();
			/**
			 *  external command to stop process
			 *
			 * @param bWait calling rutine should wait until the process is stopping
			 * @return object of error handling
			 */
			OVERWRITE EHObj stop(const bool bWait= true);
			/**
			 * desstructor to delete created objects
			 */
			virtual ~ServerProcess();

		protected:
			/**
			 * this method will be called before running
			 * the method execute to initial class
			 *
			 * @param args user defined parameter value or array,<br />
			 * 				coming as void pointer from the external call
			 * 				method start(void *args).
			 * @return object of error handling
			 */
			OVERWRITE EHObj init(void *arg);
			/**
			 * This method starting again when ending with code 0 or lower for warnings
			 * and if the method stop() isn't called.
			 *
			 * @param whether process should start again
			 */
			OVERWRITE bool execute();
			/**
			 * This method will be called if any other or own thread
			 * calling method stop().
			 */
			virtual void ending();
			/**
			 * protected initialization for given info points
			 * to write into the status information
			 *
			 * @param params parameter set by call getStatusInfo from main method
			 * @param pos position struct see pos_t
			 * @param elapsed seconds be elapsed since last position time pos_t.time
			 * @param time from last position pos_t.time converted in an string
			 */
			virtual string getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime);

		private:
			/**
			 * under witch user the process should be running
			 */
			const uid_t m_uid;
			/**
			 * thread id from running process
			 * which accept connection
			 */
			pthread_t m_nAcceptThread;
			/**
			 * whether new connections are allowed
			 */
			bool m_bNewConnections;
			/**
			 * pool to start communication threads
			 */
			IServerCommunicationStarterPattern *m_pStarterPool;
			/**
			 * holder IServerConnectArtPattern to verify connection to client
			 */
			IServerConnectArtPattern* m_pConnect;
			/**
			 * string for open connection, otherwise by null string the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 */
			string m_sOpenConnection;
			/**
			 * mutex lock to get new connections
			 */
			pthread_mutex_t* m_NEWCONNECTIONS;
			/**
			 * condition to wait while new connections are not allowed
			 */
			pthread_cond_t* m_NOCONWAITCONDITION;

	};
}

#endif /*SERVERPROCESS_H_*/
