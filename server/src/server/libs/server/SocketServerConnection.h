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
	 * initialication of socket connection for server over TCP or UDP<br />
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
	 * 			20
	 * 		</td>
	 * 		<td>
	 * 			Undefined socket error.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			socket options errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			61
	 * 		</td>
	 * 		<td>
	 * 			The argument sockfd is not a valid descriptor.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			62
	 * 		</td>
	 * 		<td>
	 * 			The  address  pointed  to  by optval is not in a valid part of the process address space.
	 * 			For getsockopt(), this error may also be returned if optlen is not in a valid part of the process address space.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			63
	 * 		</td>
	 * 		<td>
	 * 			optlen invalid in setsockopt().  In some cases this error can also occur for an invalid value in optval
	 * 			(e.g., for the IP_ADD_MEMBERSHIP option described in ip(7)).
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			64
	 * 		</td>
	 * 		<td>
	 * 			The option is unknown at the level indicated.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			65
	 * 		</td>
	 * 		<td>
	 * 			The argument sockfd is a file, not a socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			70
	 * 		</td>
	 * 		<td>
	 * 			Undefined set socket option error
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			binding errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			71
	 * 		</td>
	 * 		<td>
	 * 			The specified address is already in use.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			72
	 * 		</td>
	 * 		<td>
	 * 			The specified address is not available from the local machine.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			73
	 * 		</td>
	 * 		<td>
	 * 			The specified address is not a valid address for the address family of the specified socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			74
	 * 		</td>
	 * 		<td>
	 * 			The socket argument is not a valid file descriptor.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			75
	 * 		</td>
	 * 		<td>
	 * 			The socket is already bound to an address, and the protocol does not support binding to a new address;
	 * 			or the socket has been shut down.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			76
	 * 		</td>
	 * 		<td>
	 * 			The socket argument does not refer to a socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			77
	 * 		</td>
	 * 		<td>
	 * 			The socket type of the specified socket does not support binding to an address.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			78
	 * 		</td>
	 * 		<td>
	 * 			A component of the path prefix denies search permission, or the requested name requires writing
	 * 			in a directory with a mode that denies write per‐mission.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			79
	 * 		</td>
	 * 		<td>
	 * 			The address argument is a null pointer.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			80
	 * 		</td>
	 * 		<td>
	 * 			An I/O error occurred.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			81
	 * 		</td>
	 * 		<td>
	 * 			A loop exists in symbolic links encountered during resolution of the pathname in address.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			82
	 * 		</td>
	 * 		<td>
	 * 			A component of a pathname exceeded {NAME_MAX} characters, or an entire pathname exceeded {PATH_MAX} characters.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			83
	 * 		</td>
	 * 		<td>
	 * 			A component of the pathname does not name an existing file or the pathname is an empty string.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			84
	 * 		</td>
	 * 		<td>
	 * 			A component of the path prefix of the pathname in address is not a directory.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			85
	 * 		</td>
	 * 		<td>
	 * 			The name would reside on a read-only file system.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			86
	 * 		</td>
	 * 		<td>
	 * 			The socket is already connected.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			87
	 * 		</td>
	 * 		<td>
	 * 			Insufficient resources were available to complete the call.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			100
	 * 		</td>
	 * 		<td>
	 * 			Undefined Binding error.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			listen errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			101
	 * 		</td>
	 * 		<td>
	 * 			The socket argument is not a valid file descriptor.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			102
	 * 		</td>
	 * 		<td>
	 * 			The socket is not bound to a local address, and the protocol does not support listening on an unbound socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			103
	 * 		</td>
	 * 		<td>
	 * 			The socket is already connected,
	 * 			or the socket has been shut down.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			104
	 * 		</td>
	 * 		<td>
	 * 			The socket argument does not refer to a socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			105
	 * 		</td>
	 * 		<td>
	 * 			The socket protocol does not support listen().
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			106
	 * 		</td>
	 * 		<td>
	 * 			The calling process does not have the appropriate privileges.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			108
	 * 		</td>
	 * 		<td>
	 * 			Insufficient resources are available in the system to complete the call.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			120
	 * 		</td>
	 * 		<td>
	 * 			Undefined listen error.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			accept errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			121
	 * 		</td>
	 * 		<td>
	 * 			O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			122
	 * 		</td>
	 * 		<td>
	 * 			The socket argument is not a valid file descriptor.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			123
	 * 		</td>
	 * 		<td>
	 * 			A connection has been aborted.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			124
	 * 		</td>
	 * 		<td>
	 * 			The accept() function was interrupted by a signal that was caught before a valid connection arrived.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			125
	 * 		</td>
	 * 		<td>
	 * 			The socket is not accepting connections.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			126
	 * 		</td>
	 * 		<td>
	 * 			{OPEN_MAX} file descriptors are currently open in the calling process.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			127
	 * 		</td>
	 * 		<td>
	 * 			The maximum number of file descriptors in the system are already open.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			128
	 * 		</td>
	 * 		<td>
	 * 			The socket argument does not refer to a socket.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			129
	 * 		</td>
	 * 		<td>
	 * 			The socket type of the specified socket does not support accepting connections.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			130
	 * 		</td>
	 * 		<td>
	 * 			No buffer space is available.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			131
	 * 		</td>
	 * 		<td>
	 * 			There was insufficient memory available to complete the operation.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			132
	 * 		</td>
	 * 		<td>
	 * 			A protocol error has occurred; for example, the STREAMS protocol stack has not been initialized.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			140
	 * 		</td>
	 * 		<td>
	 * 			Undefined accept error.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="2">
	 * 			descriptor initial errors:
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			141
	 * 		</td>
	 * 		<td>
	 * 			The extended ITransferPattern given in the constructor can not be initializet correctly.
	 * 		</td>
	 * 	</tr>
	 * </table>
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
			virtual int initType(addrinfo* ai);
			/**
			 * listen on device for new connection
			 * and initial descriptor which getting with <code>getDescriptor()</code>
			 * if return code was 0
			 *
			 * @return error code
			 */
			virtual int accept();
			/**
			 * return the address witch the comunication have reached after listen
			 *
			 * @return name of adress
			 */
			virtual string getLastDescriptorAddress();
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error) const;
			/**
			 * get maximal error or warning number in positive values from own class
			 *
			 * @param byerror whether needs error number (true) or warning number (false)
			 * @return maximal error or warning number
			 */
			virtual unsigned int getMaxErrorNums(const bool byerror) const;
			/**
			 * close all connections of server
			 */
			virtual void close();
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
