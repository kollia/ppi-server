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

#include "../../../pattern/util/LogHolderPattern.h"

#include "../../../pattern/server/IFileDescriptorPattern.h"

#include "../../../util/GlobalStaticMethods.h"
#include "../../../util/XMLStartEndTagReader.h"

#include "../../../database/logger/lib/logstructures.h"

#include "../client/SocketClientConnection.h"
#include "../client/ExternClientInputTemplate.h"

#include "ServerProcess.h"


namespace server
{
	using namespace design_pattern_world::server_pattern;

	ServerProcess::ServerProcess(string processName, const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect,
										IClientConnectArtPattern* extcon/*= NULL*/, const string& open/*= ""*/, const bool wait/*= true*/)
	:	Process(processName, processName, extcon, NULL, wait),
		m_uid(uid),
		m_nAcceptThread(0),
		m_bNewConnections(true),
		m_pStarterPool(starter),
		m_pConnect(connect),
		m_sOpenConnection(open)
	{
		m_pConnect->setServerInstance(this);
		m_NEWCONNECTIONS= Thread::getMutex("NEWCONNECTIONS");
		m_NOCONWAITCONDITION= Thread::getCondition("NOCONWAITCONDITION");
	}

	ServerProcess::ServerProcess(const uid_t uid, IServerCommunicationStarterPattern* starter, IServerConnectArtPattern* connect,
											IClientConnectArtPattern* extcon/*= NULL*/, const string& open/*= ""*/, const bool wait/*= true*/)
	:	Process("CommunicationServerProcess", "CommunicationServerProcess", extcon, NULL, wait),
		m_uid(uid),
		m_bNewConnections(true),
		m_pStarterPool(starter),
		m_pConnect(connect),
		m_sOpenConnection(open)
	{
		m_pConnect->setServerInstance(this);
		m_NEWCONNECTIONS= Thread::getMutex("NEWCONNECTIONS");
	}

	string ServerProcess::getName() const
	{
		return getProcessName();
	}

	string ServerProcess::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
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
				sRv= Process::getStatusInfo(params, pos, elapsed, lasttime);
		}
		return sRv;
	}

	EHObj ServerProcess::init(void *args)
	{
		const unsigned short port= getSendPortAddress();
		string host(getSendHostAddress());
		//ExternClientInputTemplate run(getName(), getName(), m_pConnect, NULL);

		if(	host != "127.0.0.1" &&
			host != "localhost" &&
			host != "::1" &&
			host != "ip6-localhost"	)
		{
			cerr << "### " << getProcessName() << " should running on other computer," << endl;
			cerr << "    so do not start any server process" << endl;
			m_pSocketError->setError("ServerProcess", "init", getProcessName() + "@" + host);
			cerr << glob::addPrefix("### ERROR: ", m_pSocketError->getDescription()) << endl;
			LOG(LOG_ERROR, m_pSocketError->getDescription());
			return m_pSocketError;
		}
		cout << "### checking socket on IP:127.0.0.1 port:" << port << endl;
		m_pSocketError= openSendConnection(0, m_sOpenConnection);
		if(!m_pSocketError->fail(IEH::errno_error, ECONNREFUSED, "IClientConnectArtPattern", "connect"))
		{
			closeSendConnection();
			m_pSocketError->addMessage("ServerProcess", "openSendConnection", getProcessName());
			return m_pSocketError;
		}
		LogHolderPattern::instance()->setThreadName(getProcessName());
		m_pSocketError= m_pStarterPool->start(args);
		if(m_pSocketError->fail())
		{
			int log;
			string prefix, msg;

			m_pSocketError->addMessage("ServerProcess", "communicationclient_start", getProcessName());
			if(m_pSocketError->hasError())
			{
				prefix= "### ERROR: ";
				log= LOG_ERROR;
			}else
			{
				prefix= "### WARNING: ";
				log= LOG_WARNING;
			}
			msg= m_pSocketError->getDescription();
			cout << glob::addPrefix(prefix, msg) << endl;
			LOG(log, msg);
		}
		return m_pConnect->init();
	}

	bool ServerProcess::execute()
	{
		bool allowed(true);
		bool bRv(true);
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> fp;

		if(!connectionsAllowed())
		{
			if(!stopping())
			{
				LOCK(m_NEWCONNECTIONS);
				CONDITION(m_NOCONWAITCONDITION, m_NEWCONNECTIONS);
				UNLOCK(m_NEWCONNECTIONS);
			}
			allowed= false;
		}else
		{
			m_nAcceptThread= pthread_self();
			m_pSocketError= m_pConnect->accept();
			allowed= connectionsAllowed();
			if(m_pSocketError->fail())
			{
				string msg;
				ostringstream decl;

				decl << getProcessName();
				decl << "@" << m_pConnect->getHostAddress();
				decl << "@" << m_pConnect->getPortAddress();
				m_pSocketError->addMessage("ServerProcess", "accept", decl.str());
				cout << "write ERROR: '" << m_pSocketError->getErrorStr() << "'" << endl;
				msg= m_pSocketError->getDescription();
				if(m_pSocketError->hasError())
				{
					cerr << glob::addPrefix("### ALERT: ", msg) << endl;
					LOG(LOG_ALERT, msg);
					bRv= false;
				}else
				{
					cout << glob::addPrefix("### WARNING: ", msg) << endl;
					TIMELOG(LOG_WARNING, getProcessName() +
									"@" + m_pConnect->getHostAddress(), msg);
				}
			}
		}

		if(	bRv &&
			!stopping()	)
		{
			if(allowed)
			{
				fp= m_pConnect->getDescriptor();
				m_pStarterPool->setNewClient(fp);

			}else
			{
				glob::stopMessage("server allow no new access!");
				m_pConnect->close();

			}
		}
		return bRv;
	}

	inline bool ServerProcess::connectionsAllowed()
	{
		bool allow;

		LOCK(m_NEWCONNECTIONS);
		allow= m_bNewConnections;
		UNLOCK(m_NEWCONNECTIONS);
		return allow;
	}

	void ServerProcess::allowNewConnections(const bool allow)
	{
		LOCK(m_NEWCONNECTIONS);
		m_bNewConnections= allow;
		UNLOCK(m_NEWCONNECTIONS);
	}

	inline void ServerProcess::close()
	{
		m_pConnect->close();
	}

	EHObj ServerProcess::stop(const bool bWait/*= true*/)
	{
		SocketErrorHandling handle;

		allowNewConnections(false);
		m_pSocketError= m_pStarterPool->stop(false);
		handle= Process::stop(false);
		AROUSE(m_NOCONWAITCONDITION);
		if(	running() &&
			m_nAcceptThread != 0	)
		{
			pthread_kill(m_nAcceptThread, SIGALRM);
		}
		if(	!m_pSocketError->hasError() &&
			bWait							)
		{
			m_pSocketError= m_pStarterPool->stop(true);
		}
		if(!m_pSocketError->hasError())
		{
			close();
			if(	bWait &&
				!handle.hasError()	)
			{
				handle= Process::stop(true);
			}
		}
		if(	!m_pSocketError->hasError() &&
			handle.hasError()				)
		{
			(*m_pSocketError)= handle;

		}else if(	!m_pSocketError->fail() &&
					handle.fail()				)
		{
			(*m_pSocketError)= handle;
		}
		return m_pSocketError;
	}

	void ServerProcess::ending()
	{
		glob::threadStopMessage("ServerProcess::ending(): close connection");
		close();
		glob::threadStopMessage("ServerProcess::ending(): stop communication treads");
		while(!m_pStarterPool->stopCommunicationThreads(0, /*wait*/true))
		{
			glob::threadStopMessage("ServerProcess::ending(): stop all communication treads");
		}
		glob::threadStopMessage("ServerProcess::ending(): stop CommunicationThreadStarter");
		m_pStarterPool->stop(/*wait*/true);
		glob::threadStopMessage("ServerProcess::ending(): all communication threads and also starter pool be stopped");
	}

	ServerProcess::~ServerProcess()
	{
		if(m_pConnect)
			delete m_pConnect;
		if(m_pStarterPool)
			delete m_pStarterPool;
		DESTROYMUTEX(m_NEWCONNECTIONS);
		DESTROYCOND(m_NOCONWAITCONDITION);
		glob::threadStopMessage("ServerProcess::~ServerProcess(): object of ServerProcess was destroyed");
	}
}
