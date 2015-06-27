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
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>

#include "../../../pattern/util/LogHolderPattern.h"

#include "SocketClientConnection.h"

#include "../FileDescriptor.h"

using namespace std;

namespace server
{

	SocketClientConnection::SocketClientConnection(int type, const string host, const int port, const unsigned int timeout, ITransferPattern* transfer)
	:	m_bCorrectAddr(false),
	 	m_pTransfer(transfer),
		m_sHost(host),
		m_nPort(port),
		m_nTimeout(timeout),
		m_bNeedConnectionCheck(false),
		m_pSocketError(EHObj(new SocketErrorHandling)),
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

	EHObj SocketClientConnection::init()
	{
		int lasterrno;
		addrinfo hints;
		addrinfo *ai= NULL;
		addrinfo *aptr= NULL;
		char ip_address[INET6_ADDRSTRLEN];
		string host;
		sockaddr_in	*ipv4addr;
		sockaddr_in6 *ipv6addr;
		ostringstream oPort;

		m_pSocketError->clear();
		if(m_pDescriptor)
		{
			if(m_pDescriptor->hasError())
			{
				m_pSocketError= m_pDescriptor->getErrorObj();
				close();
			}else
			{
				bool hold;

				hold= m_pDescriptor->transfer();
				if(!hold)
				{
					m_pSocketError= m_pDescriptor->getErrorObj();
					if(m_pSocketError->fail())
					{
						m_pSocketError->addMessage("SocketClientConnection",
										"transfer", m_pDescriptor->getHostAddressName());
						close();
					}
				}
				return m_pSocketError;
			}
		}

		oPort << m_nPort;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_flags= AI_CANONNAME;
		hints.ai_family= AF_UNSPEC;
		hints.ai_socktype= m_nSocketType;
		m_kSocket.serverSocket= 0;
		m_bCorrectAddr= false;

		if(m_sHost == "")
			m_sHost= "::*";
		host= m_sHost;
		if(m_sHost == "*")
			host= "localhost";
		else if(m_sHost == "::*")
			host= "ip6-localhost";
		lasterrno= getaddrinfo(host.c_str(), oPort.str().c_str(), &hints, &ai);
		if(lasterrno != 0)
		{
			SocketErrorHandling handle;

			handle.setAddrError("SocketConnection", "getaddrinfo",
							lasterrno, errno, host + "@" + oPort.str());
			(*m_pSocketError)= handle;
			return m_pSocketError;
		}
		m_bCorrectAddr= true;
		for(aptr= ai; aptr != NULL; aptr= aptr->ai_next)
		{
			m_sHostName= aptr->ai_canonname;
			if(ai->ai_family == AF_INET)
			{
				ipv4addr= (struct sockaddr_in *)aptr->ai_addr;
				inet_ntop(ai->ai_family, &ipv4addr->sin_addr, ip_address, INET6_ADDRSTRLEN);
			}else
			{
				ipv6addr= (struct sockaddr_in6 *)aptr->ai_addr;
				inet_ntop(ai->ai_family, &ipv6addr->sin6_addr, ip_address, INET6_ADDRSTRLEN);
			}
			m_sHostAddress= ip_address;

			m_kSocket.ss_family= aptr->ai_family;
			m_kSocket.serverSocket = socket(ai->ai_family, aptr->ai_socktype, aptr->ai_protocol);
			if (m_kSocket.serverSocket <= 0)
			{
				lasterrno= errno;
				continue;
			}
			if(m_bNeedConnectionCheck)
			{
				int alive(1);

				if(	setsockopt(m_kSocket.serverSocket, SOL_SOCKET, SO_KEEPALIVE,
								&alive, sizeof( int ))		< 0					)
				{
					m_pSocketError->setErrnoWarning("SocketConnection", "setsockopt",
									errno, m_sHost + "@" + oPort.str());
				}
			}
			initType(aptr);
			break;// socket was set correctly
		}
		if(m_kSocket.serverSocket <= 0)
		{
			m_pSocketError->setErrnoError("SocketConnection", "socket",
							lasterrno, m_sHost + "@" + oPort.str());
		}
		if(ai != NULL)
			freeaddrinfo(ai);
		return m_pSocketError;
	}

	bool SocketClientConnection::connected()
	{
		int res;
		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];
		int port;

		m_bNeedConnectionCheck= true;
		if(m_pDescriptor == NULL)
			return false;

		len = sizeof addr;
		res= getpeername(m_kSocket.serverSocket, (struct sockaddr*)&addr, &len);
		if(res != 0)
			return false;

		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET)
		{
		    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		    port = ntohs(s->sin_port);
		    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
		}else
		{ // AF_INET6
		    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		    port = ntohs(s->sin6_port);
		    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
		}

		//printf("Peer IP address: %s\n", ipstr);
		//printf("Peer port      : %d\n", port);
		if(	ipstr == m_sHostAddress &&
			port == m_nPort				)
		{
			return true;
		}
		return false;
	}

	bool SocketClientConnection::initType(addrinfo* ai)
	{
		int con;
		int errsv;
		string msg;
		time_t t, nt;

		time(&t);
		con= connect(m_kSocket.serverSocket, ai->ai_addr, ai->ai_addrlen);
		errsv= errno;
		if(	con < 0 &&
			errsv != EISCONN	)
		{
			time(&nt);
			while((nt - t) < (time_t)m_nTimeout)
			{
				con= connect(m_kSocket.serverSocket, ai->ai_addr, ai->ai_addrlen);
				errsv= errno;
				if(con == 0)
					break;
				usleep(2000);
				time(&nt);
			}
		}
		if(con < 0)
		{
			ostringstream decl;

			decl << m_nTimeout << "@" << m_sHost << "@" << m_nPort;
			m_pSocketError->setErrnoError("SocketClientConnection",
							"connect", errsv, decl.str());
			return false;
		}
		m_pDescriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>(new FileDescriptor(	NULL,
																							m_pTransfer,
																							m_kSocket.serverSocket,
																							m_sHost,
																							m_nPort,
																							m_nTimeout	));
		m_pSocketError= m_pDescriptor->init();
		if(m_pSocketError->hasError())
		{
			close();
			return false;
		}
		if(m_pTransfer)
		{
			bool hold;

			hold= m_pDescriptor->transfer();
			if(!hold)
				m_pSocketError= m_pDescriptor->getErrorObj();
			if(m_pSocketError->hasError())
			{
				m_pSocketError->addMessage("SocketClientConnection", "transfer",
								m_pDescriptor->getHostAddressName());
				close();
				return false;
			}
			if(!hold)
				m_pSocketError->setError("SocketClientConnection", "transfer",
								m_pDescriptor->getHostAddressName());
		}
		return true;
	}

	SHAREDPTR::shared_ptr<IFileDescriptorPattern> SocketClientConnection::getDescriptor()
	{
		//SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

		//descriptor= m_pDescriptor;
		//m_pDescriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>();
		return m_pDescriptor;
	}

	void SocketClientConnection::close()
	{
		::close(m_kSocket.serverSocket);
//		if(m_bCorrectAddr)
//			freeaddrinfo(m_pAddrInfo);
		m_bCorrectAddr= false;
		m_pDescriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>();
	}

	SocketClientConnection::~SocketClientConnection()
	{
		if(m_pTransfer)
			delete m_pTransfer;
	}
}
