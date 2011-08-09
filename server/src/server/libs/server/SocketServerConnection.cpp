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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>

#include "../../../pattern/util/LogHolderPattern.h"

#include "../../../util/GlobalStaticMethods.h"

#include "SocketServerConnection.h"

#include "../FileDescriptor.h"

namespace server
{
	bool SocketServerConnection::socketWait()
	{
		bool bRv;

		LOCK(m_WAITACCEPT);
		bRv= m_bWaitAccept;
		UNLOCK(m_WAITACCEPT);
		return bRv;
	}

	int SocketServerConnection::accept()
	{
		int nRv= 0;
		char ip_address[INET6_ADDRSTRLEN];
		string msg;
		FILE* fp;
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

		LOCK(m_WAITACCEPT);
		m_bWaitAccept= true;
		POS("#server#wait-client");
		UNLOCK(m_WAITACCEPT);
		m_kSocket.bindSocket = ::accept(m_kSocket.serverSocket, (struct sockaddr *) &m_kSocket.rec_addres, &m_kSocket.adrlaenge);
		LOCK(m_WAITACCEPT);
		m_bWaitAccept= false;
		UNLOCK(m_WAITACCEPT);
		if (m_kSocket.bindSocket < 0)
		{
			switch(errno)
			{
			case EAGAIN:
			//case EWOULDBLOCK: < is the same value
				nRv= 121;
				break;
			case EBADF:
				nRv= 122;
				glob::stopMessage("SocketServerConneciton::accept(): The socket argument is not a valid file descriptor, maybe server will be ending");
				break;
			case ECONNABORTED:
				nRv= 123;
				break;
			case EINTR:
				nRv= 124;
				break;
			case EINVAL:
				nRv= 125;
				break;
			case EMFILE:
				nRv= 126;
				break;
			case ENFILE:
				nRv= 127;
				break;
			case ENOTSOCK:
				nRv= 128;
				break;
			case EOPNOTSUPP:
				nRv= 129;
				break;
			case ENOBUFS:
				nRv= 130;
				break;
			case ENOMEM:
				nRv= 131;
				break;
			case EPROTO:
				nRv= 132;
				break;
			default:
				nRv= 140;
				break;
			}
			return nRv;
		}
		fp = fdopen (m_kSocket.bindSocket, "w+");
		inet_ntop(m_kSocket.ss_family, &m_kSocket.rec_addres.sin_addr, ip_address, INET6_ADDRSTRLEN);
		POSS("#server#has-client", ip_address);
	#ifdef SERVERDEBUG
		msg= "connect to client with IP-address ";
		msg+= ip_address;
		cout << msg << endl;
	#endif // SERVERDEBUG
		descriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>(new FileDescriptor(	m_pServer,
																						m_pTransfer,
																						fp,
																						ip_address,
																						m_nPort,
																						m_nTimeout	));
		if(!descriptor->init())
		{
			descriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>();
			return 141;
		}
		m_pDescriptor= descriptor;
		return 0;
	}

	string SocketServerConnection::getLastDescriptorAddress()
	{
		char ip_address[INET6_ADDRSTRLEN];

		inet_ntop(m_kSocket.ss_family, &m_kSocket.rec_addres.sin_addr, ip_address, INET6_ADDRSTRLEN);
		return ip_address;
	}

	int SocketServerConnection::initType(sockaddr* address)
	{
		int nRv= 0;
		int reuse;
		string msg;
		ostringstream smsg;

		reuse = 1;
		if (setsockopt(m_kSocket.serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
		{
			switch(errno)
			{
			case EBADF:
				nRv= 61;
				break;
			case EFAULT:
				nRv= 62;
				break;
			case EINVAL:
				nRv= 63;
				break;
			case ENOPROTOOPT:
				nRv= 64;
				break;
			case ENOTSOCK:
				nRv= 65;
				break;
			default:
				nRv= 70;
				break;
			}
			return nRv;
		}

		if (bind(m_kSocket.serverSocket, address, sizeof(*address)) != 0)
		{
			switch(errno)
			{
			case EADDRINUSE:
				nRv= 71;
				break;
			case EADDRNOTAVAIL:
				nRv= 72;
				break;
			case EAFNOSUPPORT:
				nRv= 73;
				break;
			case EBADF:
				nRv= 74;
				break;
			case EINVAL:
				nRv= 75;
				break;
			case ENOTSOCK:
				nRv= 76;
				break;
			case EOPNOTSUPP:
				nRv= 77;
				break;
			case EACCES:
				nRv= 78;
				break;
			case EDESTADDRREQ:
			case EISDIR:
				nRv= 79;
				break;
			case EIO:
				nRv= 80;
				break;
			case ELOOP:
				nRv= 81;
				break;
			case ENAMETOOLONG:
				nRv= 82;
				break;
			case ENOENT:
				nRv= 83;
				break;
			case ENOTDIR:
				nRv= 84;
				break;
			case EROFS:
				nRv= 85;
				break;
			case EISCONN:
				nRv= 86;
				break;
			case ENOBUFS:
				nRv= 87;
				break;
			default:
				nRv= 100;
				break;
			}
			msg= "ERROR: by binding on server\n       ";
			msg+= strerror(errno);
			msg+= "\n       server does not start\n";
			LOG(LOG_ALERT, msg);
			cout << msg << endl;
			return false;
		}
		if (listen(m_kSocket.serverSocket, 5) < 0)
		{
			switch(errno)
			{
			case EBADF:
				nRv= 101;
				break;
			case EDESTADDRREQ:
				nRv= 102;
				break;
			case EINVAL:
				nRv= 103;
				break;
			case ENOTSOCK:
				nRv= 104;
				break;
			case EOPNOTSUPP:
				nRv= 105;
				break;
			case EACCES:
				nRv= 106;
				break;
			case ENOBUFS:
				nRv= 108;
				break;
			default:
				nRv= 120;
				break;
			}
			return nRv;
		}
		smsg << "listen on port " << m_nPort << " with IP ";
		smsg << m_sHost;
		cout << smsg.str() << endl;
		return 0;
	}

	string SocketServerConnection::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 61:
			str= "ERROR: The argument sockfd is not a valid descriptor.";
			break;
		case 62:
			str= "ERROR: The  address  pointed  to  by optval is not in a valid part of the process address space. "
		  			"For getsockopt(), this error may also be returned if optlen is not in a valid part of the process address space.";
			break;
		case 63:
			str= "ERROR: optlen invalid in setsockopt().  In some cases this error can also occur for an invalid value in optval "
		  			"(e.g., for the IP_ADD_MEMBERSHIP option described in ip(7)).";
			break;
		case 64:
			str= "ERROR: The option is unknown at the level indicated.";
			break;
		case 65:
			str= "ERROR: The argument sockfd is a file, not a socket.";
			break;
		case 70:
			str= "ERROR: Undefined set socket option error";
			break;
		 // binding errors:
		case 71:
			str= "ERROR: The specified address is already in use.";
			break;
		case 72:
			str= "ERROR: The specified address is not available from the local machine.";
			break;
		case 73:
			str= "ERROR: The specified address is not a valid address for the address family of the specified socket.";
			break;
		case 74:
			str= "ERROR: The socket argument is not a valid file descriptor.";
			break;
		case 75:
			str= "ERROR: The socket is already bound to an address, and the protocol does not support binding to a new address; "
		  			"or the socket has been shut down.";
			break;
		case 76:
			str= "ERROR: The socket argument does not refer to a socket.";
			break;
		case 77:
			str= "ERROR: The socket type of the specified socket does not support binding to an address.";
			break;
		case 78:
			str= "ERROR: A component of the path prefix denies search permission, or the requested name requires writing"
		  			" in a directory with a mode that denies write per‐mission.";
			break;
		case 79:
			str= "ERROR: The address argument is a null pointer.";
			break;
		case 80:
			str= "ERROR: An I/O error occurred.";
			break;
		case 81:
			str= "ERROR: A loop exists in symbolic links encountered during resolution of the pathname in address.";
			break;
		case 82:
			str= "ERROR: A component of a pathname exceeded {NAME_MAX} characters, or an entire pathname exceeded {PATH_MAX} characters.";
			break;
		case 83:
			str= "ERROR: A component of the pathname does not name an existing file or the pathname is an empty string.";
			break;
		case 84:
			str= "ERROR: A component of the path prefix of the pathname in address is not a directory.";
			break;
		case 85:
			str= "ERROR: The name would reside on a read-only file system.";
			break;
		case 86:
			str= "ERROR: The socket is already connected.";
			break;
		case 87:
			str= "ERROR: Insufficient resources were available to complete the call.";
			break;
		case 100:
			str= "ERROR: Undefined Binding error.";
			break;
		 // listen errors
		case 101:
			str= "ERROR: The socket argument is not a valid file descriptor.";
			break;
		case 102:
			str= "ERROR: The socket is not bound to a local address, and the protocol does not support listening on an unbound socket.";
			break;
		case 103:
			str= "ERROR: The socket is already connected, or the socket has been shut down.";
			break;
		case 104:
			str= "ERROR: The socket argument does not refer to a socket.";
			break;
		case 105:
			str= "ERROR: The socket protocol does not support listen().";
			break;
		case 106:
			str= "ERROR: The calling process does not have the appropriate privileges.";
			break;
		case 108:
			str= "ERROR: Insufficient resources are available in the system to complete the call.";
			break;
		case 120:
			str= "ERROR: Undefined listen error.";
			break;
		 // accept errors:
		case 121:
			str= "ERROR: O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted.";
			break;
		case 122:
			str= "ERROR: The socket argument is not a valid file descriptor.";
			break;
		case 123:
			str= "ERROR: A connection has been aborted.";
			break;
		case 124:
			str= "ERROR: The accept() function was interrupted by a signal that was caught before a valid connection arrived.";
			break;
		case 125:
			str= "ERROR: The socket is not accepting connections.";
			break;
		case 126:
			str= "ERROR: {OPEN_MAX} file descriptors are currently open in the calling process.";
			break;
		case 127:
			str= "ERROR: The maximum number of file descriptors in the system are already open.";
			break;
		case 128:
			str= "ERROR: The socket argument does not refer to a socket.";
			break;
		case 129:
			str= "ERROR: The socket type of the specified socket does not support accepting connections.";
			break;
		case 130:
			str= "ERROR: No buffer space is available.";
			break;
		case 131:
			str= "ERROR: There was insufficient memory available to complete the operation.";
			break;
		case 132:
			str= "ERROR: A protocol error has occurred; for example, the STREAMS protocol stack has not been initialized.";
			break;
		case 140:
			str= "ERROR: Undefined accept error.";
			break;
		default:
			if(error >= 0)
			{
				if(error <= 140)
					str= SocketClientConnection::strerror(error);
				else
					str= m_pDescriptor->strerror(error - 140);
			}else
			{
				if(error < -10)
					str= m_pDescriptor->strerror(error + 10);
				else
					str= SocketClientConnection::strerror(error);
			}
		}
		return str;
	}

	inline unsigned int SocketServerConnection::getMaxErrorNums(const bool byerror) const
	{
		unsigned int nRv;

		if(byerror)
			nRv= 140;
		else
			nRv= 10;
		return nRv;
	}

	void SocketServerConnection::close()
	{
		::close(m_kSocket.bindSocket);
		SocketClientConnection::close();
	}

	SocketServerConnection::~SocketServerConnection()
	{
	}

}
