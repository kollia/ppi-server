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
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>

#include "../logger/LogThread.h"

#include "SocketServerConnection.h"
#include "FileDescriptor.h"

namespace server
{

	IFileDescriptorPattern* SocketServerConnection::listen()
	{
		char ip_address[INET6_ADDRSTRLEN];
		string msg;
		FILE* fp;
		IFileDescriptorPattern* descriptor;

		POS("#server#wait-client");
		m_kSocket.bindSocket = accept(m_kSocket.serverSocket, (struct sockaddr *) &m_kSocket.rec_addres, &m_kSocket.adrlaenge);
		if (m_kSocket.bindSocket < 0)
		{
			LOG(LOG_ERROR, "Error by starting new connection\n");
			return NULL;
		}
		fp = fdopen (m_kSocket.bindSocket, "w+");
		inet_ntop(m_kSocket.ss_family, &m_kSocket.rec_addres.sin_addr, ip_address, INET6_ADDRSTRLEN);
		msg= "connect to client with IP-address ";
		msg+= ip_address;
		POSS("#server#has-client", ip_address);
		LOG(LOG_SERVER, msg);
	#ifdef SERVERDEBUG
		cout << msg << endl;
	#endif // SERVERDEBUG
		descriptor= new FileDescriptor(m_pTransfer, fp, ip_address, m_nPort);
		if(!descriptor->init())
		{
			LOG(LOG_ERROR, msg+"\ninitialization fault ");
			//fclose(fp);
			delete descriptor;
			return NULL;
		}
		return descriptor;
	}

	string SocketServerConnection::getLastDescriptorAddress()
	{
		char ip_address[INET6_ADDRSTRLEN];

		inet_ntop(m_kSocket.ss_family, &m_kSocket.rec_addres.sin_addr, ip_address, INET6_ADDRSTRLEN);
		return ip_address;
	}

	bool SocketServerConnection::initType(sockaddr* address)
	{
		int reuse;
		string msg;

		reuse = 1;
		if (setsockopt(m_kSocket.serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
			perror(NULL);

		if (bind(m_kSocket.serverSocket, address, sizeof(*address)) != 0)
		{
			msg= "ERROR: by binding on server\n       ";
			msg+= strerror(errno);
			msg+= "\n       server does not start\n";
			LOG(LOG_ALERT, msg);
			cout << msg << endl;
			return false;
		}
		if (::listen(m_kSocket.serverSocket, 5) < 0)
		{
			LOG(LOG_ALERT, "ERROR: by listen on socket");
			//sleep(60);
			return false;
		}
		msg= "listen on IP ";
		msg+= m_psHost;
		msg+= "\n server is running ...";
		LOG(LOG_INFO, msg);
		cout << msg << endl;
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
