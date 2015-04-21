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
#ifndef SOCKETSERVERCONNECTION_H_
#define SOCKETSERVERCONNECTION_H_

#include "../client/SocketClientConnection.h"

#include "../../../util/thread/Thread.h"

#include "../../../pattern/server/IServerConnectArtPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * Initialization of socket connection for server over TCP or UDP
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class SocketServerConnection :	public virtual IServerConnectArtPattern,
									public SocketClientConnection
	{
		public:
			/**
			 * constructor to initial object.<br/>
			 * By cancel this SocketServerConnection object, 5. parameter object will be also delete in parent class.
			 *
			 * @param type protocol type of connection
			 * @param host string of host as ip4, ip6 or name
			 * @param port number of port
			 * @param timeout waiting seconds if no second client thread waiting for answers
			 * @param transfer pattern of ITransferPattern to cumunicate with some clients
			 */
			SocketServerConnection(int type, const string host, const unsigned short port, const unsigned int timeout,
					ITransferPattern* transfer)
			:	SocketClientConnection(type, host, port, timeout, transfer),
				m_pServer(NULL)
				{ };
			/**
			 * set an instnace of the called server
			 *
			 * @param server instance of calling server
			 */
			virtual void setServerInstance(IServerPattern* server)
			{ m_pServer= server; };
			/**
			 * initial connection for server
			 *
			 * @param ai address info from socket
			 * @return whether command was correct
			 */
			OVERWRITE bool initType(addrinfo* ai);
			/**
			 * listen on device for new connection
			 * and initial descriptor which getting with <code>getDescriptor()</code>
			 *
			 * @return object of error handling
			 */
			virtual EHObj accept();
			/**
			 * return the address witch the comunication have reached after listen
			 *
			 * @return name of adress
			 */
			virtual string getLastDescriptorAddress();
			/**
			 * close all connections of server
			 */
			virtual void close();
			/**
			 * close only binded socket to server
			 */
			virtual void closeBind();
			/**
			 * destructor of SocketServerConnection
			 */
			virtual ~SocketServerConnection();

		protected:
			/**
			 * calling server instance
			 */
			IServerPattern* m_pServer;
	};

}

#endif /*SOCKETSERVERCONNECTION_H_*/
