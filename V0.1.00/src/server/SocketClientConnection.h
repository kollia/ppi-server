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
#ifndef SOCKETCLIENTCONNECTION_H_
#define SOCKETCLIENTCONNECTION_H_

#include <sys/io.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../pattern/server/IClientConnectArtPattern.h"
#include "../pattern/server/IFileDescriptorPattern.h"
#include "../pattern/server/ITransferPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{

	struct _SOCKET
	{
		sa_family_t	ss_family;
		int			serverSocket;
		int			bindSocket;
		struct		sockaddr_in	rec_addres;
		socklen_t	adrlaenge;
		//FILE*		fp;

	};

	/**
	 * initialication of client connection over TCP or UDP
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class SocketClientConnection : public virtual IClientConnectArtPattern
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
			SocketClientConnection(int type, const string host, const unsigned short port, ITransferPattern* transfer);
			/**
			 * initial connection
			 */
			virtual bool init();
			/**
			 * initial connection for client
			 */
			virtual bool initType(sockaddr* address);
			/**
			 * close all connections of client
			 */
			virtual void close();
			/**
			 * destructor of SocketClientConnection
			 */
			virtual ~SocketClientConnection();

		protected:
			/**
			 * socket of connection
			 */
			_SOCKET m_kSocket;
			/**
			 * transaction from server to client or backward
			 */
			ITransferPattern* m_pTransfer;
			/**
			 * char string of host
			 */
			char* m_psHost;
			/**
			 * number of port
			 */
			unsigned short m_nPort;

		private:
			/**
			 * protocl type of connection
			 */
			int m_nSocketType;
	};

}

#endif /*SOCKETCLIENTCONNECTION_H_*/
