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

#include "../logger/LogThread.h"

#include "SocketClientConnection.h"
#include "FileDescriptor.h"

using namespace std;

namespace server
{

	SocketClientConnection::SocketClientConnection(int type, const string host, const unsigned short port, ITransferPattern* transfer)
	{
		string::size_type nLen= host.length();

		m_nSocketType= type;
		m_psHost= new char[nLen + 2];
		strncpy(m_psHost, host.c_str(), nLen + 1);
		m_nPort= port;
		m_pTransfer= transfer;
	}

	bool SocketClientConnection::init()
	{
		int pf; // type of connection
		char  *ip_address= new char[INET6_ADDRSTRLEN];
		string msg;
		sockaddr_in	address;
		sockaddr_in6 address6;
		sockaddr* reference;

		m_kSocket.ss_family= AF_INET;
		m_kSocket.adrlaenge= sizeof(m_kSocket.rec_addres);

		address.sin_family = AF_INET;
		address6.sin6_family= AF_INET6;
		address.sin_port   = htons(m_nPort);
		address6.sin6_port= address.sin_port;
		memset(&address.sin_addr, 0, sizeof(address.sin_addr));
		memset(&address6.sin6_addr, 0, sizeof(address6.sin6_addr));

		if(*m_psHost != '\0')
		{
			m_kSocket.ss_family= AF_INET;
			if(!inet_pton(m_kSocket.ss_family, m_psHost, &address.sin_addr))
			{
				m_kSocket.ss_family= AF_INET6;
				if(!inet_pton(m_kSocket.ss_family, m_psHost, &address6.sin6_addr))
				{
					m_kSocket.ss_family= AF_INET;
					msg=  "### WARNING: host address '";
					msg+=               m_psHost;
					msg+=               "'is not valid\n";
					msg+= "             so listen only on localhost";
					inet_pton(m_kSocket.ss_family, "127.0.0.1", &address.sin_addr);
					LOG(AKWARNING, msg);
#ifndef DEBUG
					cout << msg << endl;
#endif // DEBUG
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

		delete[] m_psHost;
		m_psHost= ip_address;
		if(m_kSocket.ss_family == AF_INET)
			pf= PF_INET;
		else
			pf= PF_INET6;
		m_kSocket.serverSocket = socket(pf, m_nSocketType, 0);
		if (m_kSocket.serverSocket < 0)
		{
			msg=  "ERROR: server cannot connect to socket!\n       ";
			msg+= strerror(errno);
			msg+= "\n       server does not start\n";
			LOG(AKALERT, msg);
			cout << msg << endl;
			return false;
		}

		return initType(reference);
	}

	bool SocketClientConnection::initType(sockaddr* address)
	{
		string msg;
		string host(m_psHost);
		FILE* fp;
		IFileDescriptorPattern* descriptor;

		if(connect(m_kSocket.serverSocket, address, sizeof(*address)) < 0)
		{
			msg= "ERROR: connection failed\n       ";
			msg+= strerror(errno);
			msg+= "\n       maybe found no running server";
			msg+= "\n       client does not start\n";
			LOG(AKALERT, msg);
			cout << msg << endl;
			return false;
		}
		fp= fdopen (m_kSocket.serverSocket, "w+");
		descriptor= new FileDescriptor(m_pTransfer, fp, host, m_nPort);
		if(!descriptor->init())
			return false;
		descriptor->transfer();
		return true;
	}

	void SocketClientConnection::close()
	{
		::close(m_kSocket.serverSocket);
	}

	SocketClientConnection::~SocketClientConnection()
	{
		delete[] m_psHost;
	}

}
