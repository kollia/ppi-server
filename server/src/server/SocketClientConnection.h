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
#include "../pattern/server/IServerPattern.h"

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
	 * initialication of client connection over TCP or UDP<br />
	 * <br />
	 * error codes:<br />
	 * (all codes over 0 be errors and under warnings)
	 * <table>
	 * 	<tr>
	 * 		<td>
	 * 			0
	 * 		</td>
	 * 		<td>
	 * 			no error occured
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			-2
	 * 		</td>
	 * 		<td>
	 * 			transaction to server will get stop command from ITransferPattern
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			-1
	 * 		</td>
	 * 		<td>
	 * 			no valid address for host be set,
	 * 			so connect only to localhost
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			socket errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			1
	 * 		</td>
	 * 		<td>
	 * 			The implementation does not support the specified address family.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			2
	 * 		</td>
	 * 		<td>
	 * 			No more file descriptors are available for this process.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			3
	 * 		</td>
	 * 		<td>
	 * 			No more file descriptors are available for the system.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			4
	 * 		</td>
	 * 		<td>
	 * 			The protocol is not supported by the address family, or the protocol is not supported by the implementation.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			5
	 * 		</td>
	 * 		<td>
	 * 			The socket type is not supported by the protocol.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			6
	 * 		</td>
	 * 		<td>
	 * 			The process does not have appropriate privileges.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			7
	 * 		</td>
	 * 		<td>
	 * 			Insufficient resources were available in the system to perform the operation.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			8
	 * 		</td>
	 * 		<td>
	 * 			Insufficient memory was available to fulfill the request.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			9
	 * 		</td>
	 * 		<td>
	 * 			no ITransferPattern be set for transaction
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			10
	 * 		</td>
	 * 		<td>
	 * 			can not initial correct new descriptor in given ITransactionPattern
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			20
	 * 		</td>
	 * 		<td>
	 * 			Undefined socket error.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			connect errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			21
	 * 		</td>
	 * 		<td>
	 * 			The specified address is not available from the local machine.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			22
	 * 		</td>
	 * 		<td>
	 * 			The specified address is not a valid address for the address family of the specified socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			23
	 * 		</td>
	 * 		<td>
	 * 			A connection request is already in progress for the specified socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			24
	 * 		</td>
	 * 		<td>
	 * 			The socket argument is not a valid file descriptor.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			25
	 * 		</td>
	 * 		<td>
	 * 			The target address was not listening for connections or refused the connection request.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			26
	 * 		</td>
	 * 		<td>
	 * 			O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established;
	 * 			the connection shall be established asynchronously.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			27
	 * 		</td>
	 * 		<td>
	 * 			The attempt to establish a connection was interrupted by delivery of a signal that was caught;
	 * 			the connection shall be established asynchronously.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			28
	 * 		</td>
	 * 		<td>
	 * 			The specified socket is connection-mode and is already connected.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			29
	 * 		</td>
	 * 		<td>
	 * 			No route to the network is present.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			30
	 * 		</td>
	 * 		<td>
	 * 			The socket argument does not refer to a socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			31
	 * 		</td>
	 * 		<td>
	 * 			The specified address has a different type than the socket bound to the specified peer address.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			32
	 * 		</td>
	 * 		<td>
	 * 			The attempt to connect timed out before a connection was made.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			33
	 * 		</td>
	 * 		<td>
	 * 			An I/O error occurred while reading from or writing to the file system.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			34
	 * 		</td>
	 * 		<td>
	 * 			A loop exists in symbolic links encountered during resolution of the pathname in address.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			35
	 * 		</td>
	 * 		<td>
	 * 			A component of a pathname exceeded {NAME_MAX} characters, or an entire pathname exceeded {PATH_MAX} characters.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			36
	 * 		</td>
	 * 		<td>
	 * 			A component of the pathname does not name an existing file or the pathname is an empty string.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			37
	 * 		</td>
	 * 		<td>
	 * 			A component of the pathname does not name an existing file or the pathname is an empty string.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			38
	 * 		</td>
	 * 		<td>
	 * 			Search permission is denied for a component of the path prefix; or write access to the named socket is denied.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			39
	 * 		</td>
	 * 		<td>
	 * 			Attempt to establish a connection that uses addresses that are already in use.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			40
	 * 		</td>
	 * 		<td>
	 * 			Remote host reset the connection request.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			41
	 * 		</td>
	 * 		<td>
	 * 			The destination host cannot be reached (probably because the host is down or a remote router cannot reach it).
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			42
	 * 		</td>
	 * 		<td>
	 * 			The address_len argument is not a valid length for the address family; or invalid address family in the sockaddr structure.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			43
	 * 		</td>
	 * 		<td>
	 * 			The local network interface used to reach the destination is down.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			44
	 * 		</td>
	 * 		<td>
	 * 			No buffer space is available.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			45
	 * 		</td>
	 * 		<td>
	 * 			The socket is listening and cannot be connected.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			60
	 * 		</td>
	 * 		<td>
	 * 			Undefined connect error.
	 * 		</td>
	 * 	</tr>
	 * </table>
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class SocketClientConnection : public virtual IClientConnectArtPattern
	{
		public:
			/**
			 * constructor to initial object.<br />
			 * object delete by ending ITransferPattern
			 *
			 * @param type protocol type of connection
			 * @param host string of host as ip4, ip6 or name
			 * @param port number of port
			 * @param timeout if client reach no server, try all seconds to reconnect and ending after timeout
			 * @param transfer pattern of ITransferPattern to cumunicate with some server
			 */
			SocketClientConnection(int type, const string host, const unsigned short port, const unsigned int timeout, ITransferPattern* transfer= NULL);
			/**
			 * set new transfer object and delete the old one if exist
			 *
			 * @param transfer pattern of ITransferPattern to cumunicate with server
			 * @param delOld whether method should delete the old transfer object. default is true
			 */
			virtual void newTranfer(ITransferPattern* transfer, const bool delOld= true);
			/**
			 * initial connection
			 *
			 * @return whether command was correct
			 */
			virtual int init();
			/**
			 * initial connection for client
			 *
			 * @return whether command was correct
			 */
			virtual int initType(sockaddr* address);
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
			virtual const unsigned short getPortAddress() const
			{ return m_nPort; };
			/**
			 * close all connections of client
			 */
			virtual void close();
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error) const;
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
			string m_sHost;
			/**
			 * number of port
			 */
			const unsigned short m_nPort;
			/**
			 * how ofthen the client should try to connect
			 */
			const unsigned int m_nTimeout;

		private:
			/**
			 * protocl type of connection
			 */
			int m_nSocketType;
			/**
			 * descriptor for transaction
			 */
			IFileDescriptorPattern* m_pDescriptor;
	};

}

#endif /*SOCKETCLIENTCONNECTION_H_*/
