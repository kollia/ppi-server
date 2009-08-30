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

#include "../../../util/process.h"

#include "../../../ports/measureThread.h"

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
			 * Object delete by ending instance of ServerCommunciationStarter
			 *
			 * @param processName new name of process
			 * @param starter object of IServerCommunicationStarterPattern to starter communication threads
			 * @param connect art of server connection
			 * @param extcon on which connection from outside the process is reachable
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ServerProcess(string processName, const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect, IClientConnectArtPattern* extcon= NULL, const bool wait= true);
			/**
			 * initialization of class ServerThread.<br />
			 * Object delete by ending instance of ServerCommunciationStarter
			 *
			 * @param starter object of IServerCommunicationStarterPattern to starter communication threads
			 * @param connect art of server connection
			 * @param extcon on which connection from outside the process is reachable
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ServerProcess(const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect, IClientConnectArtPattern* extcon= NULL, const bool wait= true);
			/**
			 * return name of server
			 *
			 * @return name
			 */
			virtual string getName() const;
			/**
			 * return factory of ServerCommunicationStarter
			 *
			 * @return actual ServerCommunicationStarter
			 */
			IServerCommunicationStarterPattern* getCommunicationFactory() const
			{ return m_pStarterPool; };
			void close();
			virtual ~ServerProcess();

		protected:
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
			 * pool to start communication threads
			 */
			IServerCommunicationStarterPattern *m_pStarterPool;
			/**
			 * holder IServerConnectArtPattern to verify connection to client
			 */
			IServerConnectArtPattern* m_pConnect;

	};
}

#endif /*SERVERPROCESS_H_*/
