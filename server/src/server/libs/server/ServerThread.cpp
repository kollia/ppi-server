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
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include "../../../util/XMLStartEndTagReader.h"

#include "../../../ports/measureThread.h"

#include "../../../logger/lib/LogInterface.h"

#include "../../../pattern/server/IFileDescriptorPattern.h"

#include "ServerThread.h"
#include "Communication.h"
#include "communicationthreadstarter.h"

using namespace logger;

namespace server
{
	ServerThread::ServerThread(IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect) :
	Thread("ServerThread", /*defaultSleep*/0),
	m_pStarterPool(starter),
	m_pConnect(connect)
	{
		m_pConnect->setServerInstance(this);
	}

	ServerThread::ServerThread(string processName, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect) :
	Thread("ServerThread", /*defaultSleep*/0),
	m_pStarterPool(starter),
	m_pConnect(connect)
	{
		m_pConnect->setServerInstance(this);
	}

	string ServerThread::getName() const
	{
		return getThreadName();
	}

	string ServerThread::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
	{
		ostringstream id;
		string param, sRv;

		if(params != "")
		{
			ostringstream oRv;

			oRv << "[";
			oRv.width(6);
			oRv << dec << pos.tid << "] ";
			oRv << pos.threadname << " ";

			if(pos.identif == "#server#wait-client")
			{
				oRv << "wait of any next client since " << lasttime;
				sRv= oRv.str();

			}else if(pos.identif == "#server#has-client")
			{
				oRv << "server has client from host '" << pos.info2 << "' since " << lasttime;
				sRv= oRv.str();

			}else if(pos.identif == "#client#send-hearing")
			{
				oRv << "send chanched value '" << pos.info2 << "' to client since " << lasttime;
				sRv= oRv.str();

			}else
				sRv= Thread::getStatusInfo(params, pos, elapsed, lasttime);
		}
		return sRv;
	}

	int ServerThread::connectAsClient(const char *ip, unsigned short port, bool print/*=true*/)
	{
		char buf[20];
		string msg;
		struct sockaddr_in	adresse;
		struct in_addr	inadr;

		int	clientsocket;
		string result;

		inet_aton(ip, &inadr);
		adresse.sin_family = AF_INET;
		adresse.sin_port   = htons(port);

		memcpy(&adresse.sin_addr, &inadr.s_addr, sizeof(adresse.sin_addr));

		sprintf(buf, "%d", port);
		msg= "### checking socket on IP:";
		msg+= inet_ntoa(adresse.sin_addr);
		msg+= " port:";
		msg+= buf;

		if(print)
			cout << msg << endl;
		if(LogInterface::instance())
			LOG(LOG_INFO, msg);

		clientsocket = socket(PF_INET, SOCK_STREAM, 0);

		if (clientsocket < 0)
		{
			logger::LogInterface *log= logger::LogInterface::instance();

			msg= "ERROR: app cannot connect to socket!";
			if(LogInterface::instance())
			{
				LOG(LOG_ALERT, msg);
				log->stop();
			}
			cerr << msg << endl;
			exit(1);
		}

		if (connect(clientsocket, (struct sockaddr *) &adresse, sizeof(adresse)) != 0)
		{
			// no Server is running
			return 0;
		}
		return clientsocket;
	}

	int ServerThread::init(void *args)
	{
		m_pStarterPool->start(args);
		return m_pConnect->init();
	}

	int ServerThread::execute()
	{
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> fp;

		if(m_pConnect->accept() <= 0)
		{
			fp= m_pConnect->getDescriptor();
			m_pStarterPool->setNewClient(fp);
		}
		return 0;
	}

	void ServerThread::close()
	{
		m_pConnect->close();
		LOG(LOG_DEBUG, "Server-Socket was closed");
	}
	ServerThread::~ServerThread()
	{
		m_pStarterPool->stopCommunicationThreads(/*wait*/true);
		m_pStarterPool->stop(/*wait*/true);
		delete m_pConnect;
		delete m_pStarterPool;
	}
}
