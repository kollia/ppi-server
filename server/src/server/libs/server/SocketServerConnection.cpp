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

#include "../../../database/logger/lib/logstructures.h"

#include "SocketServerConnection.h"

#include "../FileDescriptor.h"

namespace server
{
	EHObj SocketServerConnection::accept()
	{
		char ip_address[INET6_ADDRSTRLEN];
		string msg;
		FILE* fp;
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;
		POS("#server#wait-client");
		m_kSocket.bindSocket = ::accept(m_kSocket.serverSocket,
						(struct sockaddr *) &m_kSocket.rec_addres, &m_kSocket.adrlaenge);
		if (m_kSocket.bindSocket < 0)
		{
			int error(errno);
			ostringstream decl;

			decl << m_sHost << "@" << m_nPort << "@" << m_kSocket.serverSocket;
			m_pSocketError->setErrnoError("SocketServerConnection",
							"accept", error, decl.str());
			if(error == EBADF)
				glob::stopMessage("SocketServerConneciton::accept(): "
								"The socket argument is not a valid file descriptor, "
								"maybe server will be ending");
			return m_pSocketError;
		}
		fp = fdopen (m_kSocket.bindSocket, "w+");
		if(fp == NULL)
		{
			int error(errno);
			ostringstream decl;

			decl << m_sHost << "@" << m_nPort;
			m_pSocketError->setErrnoError("SocketServerConnection", "fdopen", error, decl.str());
			return m_pSocketError;
		}
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
		m_pSocketError= descriptor->init();
		if(m_pSocketError->hasError())
		{
			descriptor= SHAREDPTR::shared_ptr<IFileDescriptorPattern>();
			return m_pSocketError;
		}
		m_pDescriptor= descriptor;
		return m_pSocketError;
	}

	string SocketServerConnection::getLastDescriptorAddress()
	{
		char ip_address[INET6_ADDRSTRLEN];

		inet_ntop(m_kSocket.ss_family, &m_kSocket.rec_addres.sin_addr, ip_address, INET6_ADDRSTRLEN);
		return ip_address;
	}

	bool SocketServerConnection::initType(addrinfo* ai)
	{
		int reuse;
		ostringstream smsg;

		reuse = 1;
		if (setsockopt(m_kSocket.serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
		{
			int error(errno);
			ostringstream decl;

			decl << m_sHost << "@" << m_nPort;
			m_pSocketError->setErrnoError("SocketConnection", "reuse",
							error, decl.str());
			return false;
		}

		memset(&m_kSocket.rec_addres, 0, sizeof(m_kSocket.rec_addres));
		m_kSocket.rec_addres.sin_family= m_kSocket.ss_family;
		m_kSocket.rec_addres.sin_port= htons(m_nPort);
		m_kSocket.rec_addres.sin_addr.s_addr= htonl( INADDR_ANY ); /* wildcard */
		m_kSocket.adrlaenge= sizeof(m_kSocket.rec_addres);
		//if (bind(m_kSocket.serverSocket, ai->ai_addr, ai->ai_addrlen) != 0)
		if(	bind(m_kSocket.serverSocket,
						(struct sockaddr*)&m_kSocket.rec_addres,
						m_kSocket.adrlaenge						) != 0	)
		{
			int error(errno);
			ostringstream decl;

			decl << m_sHost << "@" << m_nPort;
			m_pSocketError->setErrnoError("SocketServerConnection", "bind",
							error, decl.str());
			return false;
		}
		if (listen(m_kSocket.serverSocket, 5) < 0)
		{
			int error(errno);
			ostringstream decl;

			decl << m_sHost << "@" << m_nPort;
			m_pSocketError->setErrnoError("SocketServerConnection", "bind",
							error, decl.str());
			return -1;
		}
		smsg << "listen on port " << m_nPort << " by host ";
		smsg << m_sHost;
		cout << smsg.str() << endl;
		return true;
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
