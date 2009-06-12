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

#include "../util/Thread.h"
#include "../util/XMLStartEndTagReader.h"

#include "../ports/measureThread.h"

#include "../logger/LogThread.h"

#include "../pattern/server/IFileDescriptorPattern.h"

#include "ServerThread.h"
#include "Communication.h"
#include "communicationthreadstarter.h"


namespace server
{
	ServerThread* ServerThread::_instance= NULL;

	ServerThread::ServerThread(IServerConnectArtPattern* connect) :
	Thread("ServerThread", /*defaultSleep*/0)
	{
		m_pConnect= connect;
		m_connectionID= 0;
	}

	ServerThread *ServerThread::initial(IServerConnectArtPattern* connect, void* args, bool bHold)
	{
		if(!_instance)
		{
			_instance= new ServerThread(connect);
			_instance->start(args, bHold);
		}
		return _instance;
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
#ifndef DEBUG
		if(print)
			cout << msg << endl;
#endif // DEBUG
		LOG(LOG_INFO, msg);

		clientsocket = socket(PF_INET, SOCK_STREAM, 0);

		if (clientsocket < 0)
		{
			LogThread *log= LogThread::instance();

			LOG(LOG_ALERT, "ERROR: server as client cannot connect to socket!");
			log->stop();
			exit(1);
		}

		if (connect(clientsocket, (struct sockaddr *) &adresse, sizeof(adresse)) != 0)
		{
			// no Server is running
			return 0;
		}
		return clientsocket;
	}

	bool ServerThread::init(void *args)
	{
		int minThreads;
		string param("minconnectionthreads");
		serverArg_t* serverArg= (serverArg_t*)args;
		CommunicationThreadStarter* commThreadStarter;

		//m_pDistributor= serverArg.pDistributor;
		m_pFirstMeasureThread= serverArg->pFirstMeasureThreads;
		m_sClientRoot= serverArg->clientFolder;

		// check whether an CommunicationThreadStarter be needed if minconnectionthreads higher than 0
		// otherwise object only needed for initialization to start CommunicationThreads
		// and order new connections from ServerThread to CommunicationThread
		minThreads= serverArg->pServerConf->getInt(param, /*warning*/false);

		// creating Communication threads
		CommunicationThreadStarter::initial(m_pConnect, serverArg, 0);

		commThreadStarter= CommunicationThreadStarter::instance();
		if(minThreads == 0)
		{
			if(!commThreadStarter->init(&serverArg->clientFolder))
				return false;
		}else
			commThreadStarter->start(&serverArg->clientFolder);

		return m_pConnect->init();
	}

	void ServerThread::execute()
	{
		CommunicationThreadStarter* starter= CommunicationThreadStarter::instance();
		IFileDescriptorPattern* fp;

		fp= m_pConnect->listen();
		starter->setNewClient(fp);
	}

	void ServerThread::close()
	{
		m_pConnect->close();
		LOG(LOG_DEBUG, "Server-Socket was closed");
	}
	ServerThread::~ServerThread()
	{
		CommunicationThreadStarter* starter= CommunicationThreadStarter::instance();

		starter->stopCommunicationThreads(/*wait*/true);
		starter->stop(/*wait*/true);
		//delete m_pConnect;
	}
}
