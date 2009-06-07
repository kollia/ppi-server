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

#include "hearingthread.h"
#include "ServerThread.h"
#include "SocketClientConnection.h"
#include "ClientTransaction.h"

namespace server
{

	HearingThread::HearingThread(string host, unsigned short port, string communicationID,
																	string user, string pwd, bool bOwDebug)
	: Thread("ClientHearingThread", /*defaultSleep*/0)
	{
		m_shost= host;
		m_nPort= port;
		m_sCommunicationID= "-id ";
		m_sCommunicationID+= communicationID;
		m_sUser= "-u ";
		m_sUser+= user;
		m_sPwd= "-p ";
		m_sPwd+= pwd;
		m_bOwDebug= bOwDebug;
	}

	bool HearingThread::init(void* args)
	{
		return true;
	}

	void HearingThread::execute()
	{
		vector<string> options;
		SocketClientConnection* clientCon;

		options.push_back(m_sCommunicationID);
		options.push_back(m_sUser);
		options.push_back(m_sPwd);
		if(m_bOwDebug)
			options.push_back("-ow");
		clientCon= new SocketClientConnection(SOCK_STREAM, m_shost, m_nPort,
											new ClientTransaction(options, ""));
		clientCon->init();
		delete clientCon;
		stop();
	}

	void HearingThread::ending()
	{
	}

	HearingThread::~HearingThread()
	{
	}

}
