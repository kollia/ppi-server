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
#ifndef TCPSERVERCONNECTION_H_
#define TCPSERVERCONNECTION_H_

#include "SocketServerConnection.h"

namespace server
{

	/**
	 * initialication of socket connection to server over TCP/IPv4/IPv6
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class TcpServerConnection : public SocketServerConnection
	{
		public:
			/**
			 * constructor to initial object
			 *
			 * @param host string of host as ip4, ip6 or name
			 * @param port number of port
			 * @param transfer pattern of ITransferPattern to cumunicate with some clients
			 */
			TcpServerConnection(const string host, const unsigned short port, ITransferPattern* transfer)
			:SocketServerConnection(SOCK_STREAM, host, port, transfer)  { };
	};

}

#endif /*TCPSERVERCONNECTION_H_*/
