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

#include "SocketClientConnection.h"

#include "../pattern/server/IServerConnectArtPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * initialication of socket connection over TCP or UDP
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class SocketServerConnection :	public virtual IServerConnectArtPattern,
									public SocketClientConnection
	{
		public:
			/**
			 * constructor to initial object
			 *
			 * @param type protocol type of connection
			 * @param host string of host as ip4, ip6 or name
			 * @param port number of port
			 * @param transfer pattern of ITransferPattern to cumunicate with some clients
			 */
			SocketServerConnection(int type, const string host, const unsigned short port, ITransferPattern* transfer)
			:SocketClientConnection(type, host, port, transfer) { };

			/**
			 * initial connection for server
			 */
			virtual bool initType(sockaddr* address);
			/**
			 * listen on socket for new connection
			 *
			 * @return object of IFileDescriptorPattern for communicate with client
			 */
			virtual IFileDescriptorPattern* listen();
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
			 * destructor of SocketServerConnection
			 */
			virtual ~SocketServerConnection();
	};

}

#endif /*SOCKETSERVERCONNECTION_H_*/
