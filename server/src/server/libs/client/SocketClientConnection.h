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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory>

#include "../../../pattern/server/IClientConnectArtPattern.h"
#include "../../../pattern/server/IFileDescriptorPattern.h"
#include "../../../pattern/server/ITransferPattern.h"
#include "../../../pattern/server/IServerPattern.h"

#include "../../../util/smart_ptr.h"

#include "../SocketErrorHandling.h"

using namespace std;
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
			 * constructor to initial object.<br />
			 * By cancel this object, fourth parameter object will be also delete.
			 *
			 * @param type protocol type of connection
			 * @param host string of host as ip4, ip6 or name
			 * @param port number of port
			 * @param timeout if client reach no server, try all seconds to reconnect and ending after timeout
			 * @param transfer pattern of ITransferPattern to cumunicate with some server
			 */
			SocketClientConnection(int type, const string host, const int port, const unsigned int timeout, ITransferPattern* transfer= NULL);
			/**
			 * set new transfer object and delete the old one if exist
			 *
			 * @param transfer pattern of ITransferPattern to cumunicate with server
			 * @param delOld whether method should delete the old transfer object. default is true
			 */
			virtual void newTranfer(ITransferPattern* transfer, const bool delOld= true);
			/**
			 * initial connection.<br />
			 * when any connection exists
			 * check first whether descriptor is correct
			 * and make an transaction.
			 * Otherwise close fault connection and open an new one
			 *
			 * @return error handling object
			 */
			OVERWRITE EHObj init();
			/**
			 * initial connection for client
			 *
			 * @param ai address info from socket
			 * @return whether command was correct
			 */
			virtual bool initType(addrinfo* ai);
			/**
			 * check whether instance is connected
			 *
			 * @return whether connected
			 */
			virtual bool connected();
			/**
			 * get host address to which client connect
			 *
			 * @return host address
			 */
			virtual const string getHostAddress() const
			{ return m_sHost; };
			/**
			 * return port address on which client connect
			 *
			 * @return port address
			 */
			virtual unsigned short getPortAddress() const
			{ return m_nPort; };
			/**
			 * returning descriptor created with <code>accept()</code>
			 *
			 * @return object of ITransferPattern for communicate with client
			 */
			virtual SHAREDPTR::shared_ptr<IFileDescriptorPattern> getDescriptor();
			/**
			 * return timeout by finding no connection
			 *
			 * @return timeout in seconds
			 */
			virtual unsigned int getTimeout() const
			{ return m_nTimeout; };
			/**
			 * set timeout by finding no connection
			 *
			 * @param time timeout in seconds
			 */
			virtual void setTimeout(const unsigned int time)
			{ m_nTimeout= time; };
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
			 * read correct address info from other peer
			 */
			bool m_bCorrectAddr;
			/**
			 * transaction from server to client or backward
			 */
			ITransferPattern* m_pTransfer;
			/**
			 * char string of host.<br />
			 * can be an IP-Address or IP-Name
			 * given from constructor
			 */
			string m_sHost;
			/**
			 * character string of canonical
			 * IP-Name of host
			 */
			string m_sHostName;
			/**
			 * character string of IP-Address
			 */
			string m_sHostAddress;
			/**
			 * number of port
			 */
			const int m_nPort;
			/**
			 * how ofthen the client should try to connect
			 */
			unsigned int m_nTimeout;
			/**
			 * descriptor for transaction
			 */
			SHAREDPTR::shared_ptr<IFileDescriptorPattern> m_pDescriptor;
			/**
			 * whether client need to know
			 * whether connection is alive
			 */
			bool m_bNeedConnectionCheck;
			/**
			 * socket error handling
			 */
			EHObj m_pSocketError;

		private:
			/**
			 * protocl type of connection
			 */
			int m_nSocketType;
	};

}

#endif /*SOCKETCLIENTCONNECTION_H_*/
