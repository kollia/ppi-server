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
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

//#include "Thread.h"
#include "process.h"

#include "../GlobalStaticMethods.h"

#include "../stream/OMethodStringStream.h"

#include "../../pattern/util/LogHolderPattern.h"
#include "../../database/logger/lib/logstructures.h"

namespace util
{

	Process::Process(const string& ownProcess, const string& toClient, IClientConnectArtPattern* sendConnection/*= NULL*/,
										IClientConnectArtPattern* getConnection/*= NULL*/, const bool wait/*= true*/) :
		ExternClientInputTemplate(ownProcess, toClient, sendConnection, getConnection),
		StatusLogRoutine(),
		m_bWaitInit(wait),
		m_nProcessID(0),
		m_sProcessName(ownProcess),
		m_sToClient(toClient),
		m_bRun(false),
		m_bStop(false),
		m_pvStartRv(NULL),
		m_pvStopRv(NULL)
	{
		m_PROCESSRUNNING= Thread::getMutex("PROCESSRUNNING");
		m_PROCESSSTOPPING= Thread::getMutex("PROCESSSTOPPING");
		m_PROCESSSTOPCOND= Thread::getCondition("PROCESSSTOPCOND");
	}

	EHObj Process::stop(const bool bWait/*= true*/)
	{
		m_pSocketError->clear();
		if(m_nProcessID == 0)
		{
			OMethodStringStream stop("stop");
			string answer;

			m_pSocketError= openSendConnection();
			if(m_pSocketError->hasError())
				return m_pSocketError;
			answer= sendMethod(m_sToClient, stop, bWait);
			m_pSocketError= closeSendConnection();
			/*
			 * when error exist inside answer
			 * and also closeSendConnection()
			 * overwrite close-error
			 * with answer-error
			 */
			m_pSocketError->setErrorStr(answer);
			return m_pSocketError;
		}
		LOCK(m_PROCESSSTOPPING);
		m_bStop= true;
		if(bWait)
			CONDITION(m_PROCESSSTOPCOND, m_PROCESSSTOPPING);
		UNLOCK(m_PROCESSSTOPPING);
		return m_pSocketError;
	}

	EHObj Process::start(void *args/*= NULL*/, bool bHold/*= false*/)
	{
		string processName(getProcessName());
		string debugProcessName("");//("LogServer");//("CommunicationServerProcess");//
		pid_t nProcess;
		ostringstream stream;

		if((nProcess= fork()) < 0)
		{
			m_pSocketError->setErrnoError("process", "fork", errno,
							processName + "@" + m_sToClient);
			return m_pSocketError;
		}
		if(	(	debugProcessName == processName
				&&
				nProcess > 0						)
			||
			(	debugProcessName != processName
				&&
				nProcess == 0						)	)
		{
			m_pSocketError= runprocess(args, bHold);
			if(m_pSocketError->fail())
			{
				int logging;
				string prefix;
				string msg;

				msg= m_pSocketError->getDescription();
				if(m_pSocketError->hasError())
				{
					prefix= "### ALERT: ";
					logging= LOG_ALERT;
				}else
				{
					prefix= "### WARNING: ";
					logging= LOG_WARNING;
				}
				LOG(logging, msg);
				cout << glob::addPrefix(prefix, msg) << endl;
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}
		if(	m_bWaitInit	)
		{
			m_pSocketError= check();
			if(m_pSocketError->hasError())
				return m_pSocketError;
		}
		if(bHold)
		{
			SocketErrorHandling err;
			string answer;
			OMethodStringStream waiting("waiting");

			answer= sendMethod(m_sToClient, waiting, true);
			m_pSocketError->setErrorStr(answer);
			if(m_pSocketError->hasError())
				err= closeSendConnection();
			if(m_pSocketError->fail())
				return m_pSocketError;
			if(err.fail())
			{
				(*m_pSocketError)= err;
				return m_pSocketError;
			}
			if(answer != "done")
				m_pSocketError->setError("process", "waiting",
								processName + "@" + m_sToClient + "@" + answer);
		}
		return m_pSocketError;
	}

	EHObj Process::check()
	{
		SocketErrorHandling errorHandling;
		string answer;
		OMethodStringStream init("init");

		m_pSocketError= openSendConnection();
		if(m_pSocketError->hasError())
			return m_pSocketError;
		answer= sendMethod(m_sToClient, init, true);
		m_pSocketError->setErrorStr(answer);
		if(m_pSocketError->hasError())
			errorHandling= closeSendConnection();
		if(m_pSocketError->fail())
		{
			m_pSocketError->addMessage("process", "check", getProcessName() + "@" + m_sToClient);
			return m_pSocketError;
		}
		if(answer != "done")
		{
			m_pSocketError->setError("process", "wrong_answer",
							getProcessName() + "@" + m_sToClient + "@" + answer);
			return m_pSocketError;
		}
		(*m_pSocketError)= errorHandling;
		m_pSocketError->addMessage("process", "closeSendConnection",
							getProcessName() + "@" + m_sToClient);
		return m_pSocketError;
	}

	EHObj Process::run(void *args/*= NULL*/)
	{
		return runprocess(args, false);
	}

	EHObj Process::runprocess(void* args, bool bHold)
	{
		bool bRun(true);

		m_bRun= true;
		m_nProcessID= getpid();
		initstatus(getProcessName(), this);
		m_pSocketError= init(args);
		if(m_pSocketError->fail())
		{
			m_pSocketError->addMessage("process", "init",
							getProcessName());
		}
		if(!m_pSocketError->hasError())
		{
			while(	bRun &&
					!stopping()	)
			{
				POS("###THREAD_execute_start");
				m_pSocketError->clear();
				bRun= execute();
			}
		}
		AROUSEALL(m_PROCESSSTOPCOND);
		ending();
		removestatus(m_nProcessID);
		return m_pSocketError;
	}

	short Process::running()
	{
		short nRv= 0;
		string answer;

		if(m_nProcessID == 0)
		{
			OMethodStringStream running("running");

			answer= sendMethod(m_sToClient, running, true);
			if(answer == "true")
				return 1;
			if(answer == "false")
				return 0;
			m_pSocketError->setErrorStr(answer);
			if(!m_pSocketError->fail())
			{
				m_pSocketError->setError("process", "running",
								getProcessName() + "@" + m_sToClient + "@" + answer);
			}
			return -1;
		}
		LOCK(m_PROCESSRUNNING);
		if(m_bRun)
			nRv= 1;
		UNLOCK(m_PROCESSRUNNING);
		return nRv;
	}

	short Process::stopping()
	{
		int nRv= 0;
		string answer;

		if(m_nProcessID == 0)
		{
			OMethodStringStream stopping("stopping");

			//toDo: asking over connection
			//return false;
			answer= sendMethod(m_sToClient, stopping, true);
			if(answer == "true")
				return 1;
			if(answer == "false")
				return 0;
			m_pSocketError->setErrorStr(answer);
			if(!m_pSocketError->fail())
			{
				m_pSocketError->setError("process", "stopping",
								getProcessName() + "@" + m_sToClient + "@" + answer);
			}
			return -1;
		}
		LOCK(m_PROCESSSTOPPING);
		if(m_bStop)
			nRv= 1;
		UNLOCK(m_PROCESSSTOPPING);
		return nRv;
	}

	Process::~Process()
	{
	}

}
