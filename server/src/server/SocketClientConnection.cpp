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

	SocketClientConnection::SocketClientConnection(int type, const string host, const unsigned short port, const unsigned int timeout, ITransferPattern* transfer)
	:	m_pTransfer(transfer),
		m_sHost(host),
		m_nPort(port),
		m_nSocketType(type),
		m_nTimeout(timeout),
		m_pDescriptor(NULL)
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

	bool SocketClientConnection::init()
	{
		int pf; // type of connection
		char  *ip_address= new char[INET6_ADDRSTRLEN];
		string msg;
		sockaddr_in	address;
		sockaddr_in6 address6;
		sockaddr* reference;

		if(!m_pTransfer)
			return false;
		if(m_pDescriptor)
		{
			m_pDescriptor->transfer();
			return true;
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
					msg=  "### WARNING: host address '";
					msg+=               m_sHost;
					msg+=               "'is not valid\n";
					msg+= "             so listen only on localhost";
					inet_pton(m_kSocket.ss_family, "127.0.0.1", &address.sin_addr);
					LOG(LOG_WARNING, msg);
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

		m_sHost= ip_address;
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
			LOG(LOG_ALERT, msg);
			cout << msg << endl;
			return false;
		}

		return initType(reference);
	}

	bool SocketClientConnection::initType(sockaddr* address)
	{
		int con;
		int nsize= sizeof(*address);
		unsigned int count= 0;
		string msg;
		FILE* fp;

		do{
			con= connect(m_kSocket.serverSocket, address, nsize);
			if(con >= 0)
				break;
			++count;
			sleep(1);
		}while(count <= m_nTimeout);
		if(con < 0)
		{
			msg= "ERROR: connection failed\n       ";
			msg+= strerror(errno);
			msg+= "\n       maybe found no running server";
			msg+= "\n       client does not start\n";
			LOG(LOG_ALERT, msg);
			cout << msg << endl;
			return false;
		}
		fp= fdopen (m_kSocket.serverSocket, "w+");
		m_pDescriptor= new FileDescriptor(NULL, m_pTransfer, fp, m_sHost, m_nPort);
		if(!m_pDescriptor->init())
			return false;
		m_pDescriptor->transfer();
		return true;
	}

	void SocketClientConnection::close()
	{
		::close(m_kSocket.serverSocket);
		if(m_pDescriptor)
		{
			delete m_pDescriptor;
			m_pDescriptor= NULL;
		}
	}

	SocketClientConnection::~SocketClientConnection()
	{
		if(m_pTransfer)
			delete m_pTransfer;
	}

}
