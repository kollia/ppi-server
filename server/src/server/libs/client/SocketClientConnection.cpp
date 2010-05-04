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
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>

#include "../../../logger/lib/LogInterface.h"

#include "SocketClientConnection.h"

#include "../FileDescriptor.h"

using namespace std;

namespace server
{

	SocketClientConnection::SocketClientConnection(int type, const string host, const unsigned short port, const unsigned int timeout, ITransferPattern* transfer)
	:	m_pTransfer(transfer),
		m_sHost(host),
		m_nPort(port),
		m_nTimeout(timeout),
		m_nSocketType(type)
	{
	}

	void SocketClientConnection::newTranfer(ITransferPattern* transfer, const bool delOld/*= true*/)
	{
		if(	delOld
			&&
			m_pTransfer	)
		{
			delete m_pTransfer;
			m_pTransfer= NULL;
		}
		m_pTransfer= transfer;
	}

	int SocketClientConnection::init()
	{
		int nRv= 0;
		int nRvw= 0;
		//int pf; // type of connection
		char ip_address[INET6_ADDRSTRLEN];
		//string msg;
		sockaddr_in	address;
		sockaddr_in6 address6;
		sockaddr* reference;

		if(m_pDescriptor)
		{
			if(m_pDescriptor->transfer())
				return 0;
			return -2;
		}

		m_kSocket.ss_family= AF_INET;
		m_kSocket.adrlaenge= sizeof(m_kSocket.rec_addres);

		address.sin_family = AF_INET;
		address6.sin6_family= AF_INET6;
		address.sin_port   = htons(m_nPort);
		address6.sin6_port= address.sin_port;
		memset(&address.sin_addr, 0, sizeof(address.sin_addr));
		memset(&address6.sin6_addr, 0, sizeof(address6.sin6_addr));

		if(m_sHost != "")
		{
			m_kSocket.ss_family= AF_INET;
			if(!inet_pton(m_kSocket.ss_family, m_sHost.c_str(), &address.sin_addr))
			{
				m_kSocket.ss_family= AF_INET6;
				if(!inet_pton(m_kSocket.ss_family, m_sHost.c_str(), &address6.sin6_addr))
				{
					m_kSocket.ss_family= AF_INET;
			/*		msg=  "### WARNING: host address '";
					msg+=               m_sHost;
					msg+=               "'is not valid\n";
					msg+= "             so listen only on localhost";*/
					inet_pton(m_kSocket.ss_family, "127.0.0.1", &address.sin_addr);
			/*		if(logger::LogInterface::instance())
						LOG(LOG_WARNING, msg);
					cout << msg << endl;*/
					nRvw= -1;
				}
			}
		}
		if(m_kSocket.ss_family == AF_INET)
		{
			inet_ntop(m_kSocket.ss_family, &address.sin_addr, ip_address, INET6_ADDRSTRLEN);
			reference= (sockaddr *)&address;
		}else
		{
			inet_ntop(m_kSocket.ss_family, &address6.sin6_addr, ip_address, INET6_ADDRSTRLEN);
			reference= (sockaddr *)&address6;
		}

		m_sHost= ip_address;
		/*if(m_kSocket.ss_family == AF_INET)
			pf= PF_INET;
		else
			pf= PF_INET6;
		m_kSocket.serverSocket = socket(pf, m_nSocketType, 0);*/
		m_kSocket.serverSocket = socket(m_kSocket.ss_family, m_nSocketType, 0);
		if (m_kSocket.serverSocket < 0)
		{
			switch(errno)
			{
			case EAFNOSUPPORT:
				nRv= 1;
				break;
			case EMFILE:
				nRv= 2;
				break;
			case ENFILE:
				nRv= 3;
				break;
			case EPROTONOSUPPORT:
				nRv= 4;
				break;
			case EPROTOTYPE:
				nRv= 5;
				break;
			case EACCES:
				nRv= 6;
				break;
			case ENOBUFS:
				nRv= 7;
				break;
			case ENOMEM:
				nRv= 8;
				break;
			default:
				nRv= 20;
				break;
			}
			return nRv;
		}

		nRv= initType(reference);
		if(nRv == 0)
			nRv= nRvw;
		return nRv;
	}

	int SocketClientConnection::initType(sockaddr* address)
	{
		int nRv;
		int con;
		int nsize= sizeof(*address);
		int errsv;
		string msg;
		FILE* fp;
		time_t t, nt;

		time(&t);
		con= connect(m_kSocket.serverSocket, address, nsize);
		if(con < 0)
		{
			errsv= errno;
			time(&nt);
			while((nt - t) < (time_t)m_nTimeout)
			{
				con= connect(m_kSocket.serverSocket, address, nsize);
				if(con == 0)
					break;
				errsv= errno;
				usleep(2000);
				time(&nt);
			}
		}
		if(con < 0)
		{
			switch(errsv)
			{
			case EADDRNOTAVAIL:
				nRv= 21;
				break;
			case EAFNOSUPPORT:
				nRv= 22;
				break;
			case EALREADY:
				nRv= 23;
				break;
			case EBADF:
				nRv= 24;
				break;
			case ECONNREFUSED:
				nRv= 25;
				break;
			case EINPROGRESS:
				nRv= 26;
				break;
			case EINTR:
				nRv= 27;
				break;
			case EISCONN:
				nRv= 28;
				break;
			case ENETUNREACH:
				nRv= 29;
				break;
			case ENOTSOCK:
				nRv= 30;
				break;
			case EPROTOTYPE:
				nRv= 31;
				break;
			case ETIMEDOUT:
				nRv= 32;
				break;
			case EIO:
				nRv= 33;
				break;
			case ELOOP:
				nRv= 34;
				break;
			case ENAMETOOLONG:
				nRv= 35;
				break;
			case ENOENT:
				nRv= 36;
				break;
			case ENOTDIR:
				nRv= 37;
				break;
			case EACCES:
				nRv= 38;
				break;
			case EADDRINUSE:
				nRv= 39;
				break;
			case ECONNRESET:
				nRv= 40;
				break;
			case EHOSTUNREACH:
				nRv= 41;
				break;
			case EINVAL:
				nRv= 42;
				break;
			case ENETDOWN:
				nRv= 43;
				break;
			case ENOBUFS:
				nRv= 44;
				break;
			case EOPNOTSUPP:
				nRv= 45;
				break;
			default:
				nRv= 60;
				break;
			}
			return nRv;
		}
		fp= fdopen (m_kSocket.serverSocket, "w+");
		m_pDescriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>(new FileDescriptor(	NULL,
																							m_pTransfer,
																							fp,
																							m_sHost,
																							m_nPort,
																							m_nTimeout	));
		if(!m_pDescriptor->init())
			return 10;
		if(m_pTransfer)
		{
			if(!m_pDescriptor->transfer())
				return -2;
		}
		return 0;
	}

	SHAREDPTR::shared_ptr<IFileDescriptorPattern> SocketClientConnection::getDescriptor()
	{
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

		descriptor= m_pDescriptor;
		m_pDescriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>();
		return descriptor;
	}

	void SocketClientConnection::close()
	{
		::close(m_kSocket.serverSocket);
	}

	string SocketClientConnection::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 0:
			str= "no connection error occurred";
			break;
		case -2:
			str="transaction to server will get stop command from ITransferPattern";
			break;
		case -1:
			str= "WARNING: no valid address for host be set, so connect only to localhost";
			break;
		case 1:
			str= "ERROR: The implementation does not support the specified address family.";
			break;
		case 2:
			str= "ERROR: No more file descriptors are available for this process.";
			break;
		case 3:
			str= "ERROR: No more file descriptors are available for the system.";
			break;
		case 4:
			str= "ERROR: The protocol is not supported by the address family, or the protocol is not supported by the implementation.";
			break;
		case 5:
			str= "ERROR: The socket type is not supported by the protocol.";
			break;
		case 6:
			str= "ERROR: The process does not have appropriate privileges.";
			break;
		case 7:
			str= "ERROR: Insufficient resources were available in the system to perform the operation.";
			break;
		case 8:
			str= "ERROR: Insufficient memory was available to fulfill the request.";
			break;
		case 9:
			str= "ERROR: no ITransferPattern be set for transaction";
			break;
		case 10:
			str= "ERROR: can not initial correct new descriptor in given ITransactionPattern";
			break;
		case 20:
			str= "ERROR: Undefined socket error.";
			break;
		 // connect errors:
		case 21:
			str= "ERROR: The specified address is not available from the local machine.";
			break;
		case 22:
			str= "ERROR: The specified address is not a valid address for the address family of the specified socket.";
			break;
		case 23:
			str= "ERROR: A connection request is already in progress for the specified socket.";
			break;
		case 24:
			str= "ERROR: The socket argument is not a valid file descriptor.";
			break;
		case 25:
			str= "ERROR: The target address was not listening for connections or refused the connection request.";
			break;
		case 26:
			str= "ERROR: O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established; "
		  			"the connection shall be established asynchronously.";
			break;
		case 27:
			str= "ERROR: The attempt to establish a connection was interrupted by delivery of a signal that was caught; "
					"the connection shall be established asynchronously.";
			break;
		case 28:
			str= "ERROR: The specified socket is connection-mode and is already connected.";
			break;
		case 29:
			str= "ERROR: No route to the network is present.";
			break;
		case 30:
			str= "ERROR: The socket argument does not refer to a socket.";
			break;
		case 31:
			str= "ERROR: The specified address has a different type than the socket bound to the specified peer address.";
			break;
		case 32:
			str= "ERROR: The attempt to connect timed out before a connection was made.";
			break;
		case 33:
			str= "ERROR: An I/O error occurred while reading from or writing to the file system.";
			break;
		case 34:
			str= "ERROR: A loop exists in symbolic links encountered during resolution of the pathname in address.";
			break;
		case 35:
			str= "ERROR: A component of a pathname exceeded {NAME_MAX} characters, or an entire pathname exceeded {PATH_MAX} characters.";
			break;
		case 36:
			str= "ERROR: A component of the pathname does not name an existing file or the pathname is an empty string.";
			break;
		case 37:
			str= "ERROR: A component of the pathname does not name an existing file or the pathname is an empty string.";
			break;
		case 38:
			str= "ERROR: Search permission is denied for a component of the path prefix; or write access to the named socket is denied.";
			break;
		case 39:
			str= "ERROR: Attempt to establish a connection that uses addresses that are already in use.";
			break;
		case 40:
			str= "ERROR: Remote host reset the connection request.";
			break;
		case 41:
			str= "ERROR: The destination host cannot be reached (probably because the host is down or a remote router cannot reach it).";
			break;
		case 42:
			str= "ERROR: The address_len argument is not a valid length for the address family; or invalid address family in the sockaddr structure.";
			break;
		case 43:
			str= "ERROR: The local network interface used to reach the destination is down.";
			break;
		case 44:
			str= "ERROR: No buffer space is available.";
			break;
		case 45:
			str= "ERROR: The socket is listening and cannot be connected.";
			break;
		default:
			if(error > 0)
				str= "Undefined socket connection error";
			else
				str= "Undefined socket connection warning";
		}
		return str;
	}

	inline unsigned int SocketClientConnection::getMaxErrorNums(const bool byerror) const
	{
		unsigned int nRv;

		if(byerror)
			nRv= 50;
		else
			nRv= 10;
		return nRv;
	}

	SocketClientConnection::~SocketClientConnection()
	{
		if(m_pTransfer)
			delete m_pTransfer;
	}
}
