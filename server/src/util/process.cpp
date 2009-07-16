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

#include "Thread.h"
#include "process.h"

#include "../logger/LogThread.h"

namespace util
{

	Process::Process(const string& processName, IClientConnectArtPattern* connection/*= NULL*/, const bool wait/*= true*/) :
		m_bWaitInit(wait),
		m_oConnect(connection),
		m_nProcessID(0),
		m_sProcessName(processName),
		m_bRun(false),
		m_bStop(false),
		m_pvStartRv(NULL),
		m_pvStopRv(NULL),
		m_pSendTransaction(NULL),
		m_pGetTransaction(NULL)
	{
		m_PROCESSRUNNING= Thread::getMutex("PROCESSRUNNING");
		m_PROCESSSTOPPING= Thread::getMutex("PROCESSSTOPPING");
	}

	int Process::stop(const bool bWait/*= true*/)
	{
		if(m_nProcessID == 0)
		{
			int err;
			string answer;

			answer= sendMethod(m_sProcessName, "stop", "", /*hold*/false);
			if(error(answer, err))
				return err;
			if(answer != "done")
				return 4;
			return 0;
		}
		LOCK(m_PROCESSSTOPPING);
		m_bStop= true;
		UNLOCK(m_PROCESSSTOPPING);
		return 0;
	}

	int Process::start(void *args/*= NULL*/, bool bHold/*= false*/)
	{
		string debugProcessName("CommunicationServerProcess");
		pid_t nProcess;
		ostringstream stream;

		if((nProcess= fork()) < 0)
		{
			ostringstream msg;

			msg << "### ALERT: cannot start process of " << getProcessName() << endl;
			msg << "           errno(" << errno << "): " << strerror(errno);
			cerr << msg.str() << endl;
			LOG(LOG_ALERT, msg.str());
			return 2;
		}
		cout << "run on ";
		if(nProcess > 0)
			cout << "father process with child " << nProcess << endl;
		else
			cout << "child process with no id" << endl;
		if(	(	debugProcessName == getProcessName()
				&&
				nProcess > 0						)
			||
			(	debugProcessName != getProcessName()
				&&
				nProcess == 0						)	)
		{
			m_bRun= true;
			m_nProcessID= getpid();
			setThreadLogName(getProcessName());
			cout << "initial communication server with id:" << nProcess << endl;
			if(!init(args))
			{
				string msg("### ERROR: cannot inital server ");
				msg+= getProcessName();
				cerr << msg << endl;
				LOG(LOG_ALERT, msg);
				exit(EXIT_FAILURE);
			}
			while(!stopping())
				execute();
			exit(EXIT_SUCCESS);
		}
		if(	m_bWaitInit	)
		{
			int err;
			string answer;

			answer= sendMethod(m_sProcessName, "init", "", /*hold*/false);
			if(error(answer, err))
				return err;
			if(answer != "done")
				return 3;
		}
		return 0;
	}

	bool Process::error(const string& input, int& err)
	{
		bool exist= false;
		int number;
		string answer;
		istringstream out(input);

		err= 0;
		out >> answer >> number;
		if(answer == "###ERROR") // inside error
		{
			exist= true;
			err= number;
		}else if(answer == "ERROR")
		{
			exist= true;
			err= number + 10;
		}else if(answer == "###WARNING") // inside warning
		{
			exist= true;
			err= number * -1;
		}else if(answer == "WARNING")
		{
			exist= true;
			err= (number * -1) -10;
		}
		return exist;
	}

	string Process::sendMethod(const string& toProcess, const string& method, const string& params, const bool& hold/*= true*/)
	{
		bool bExist;
		bool bHold= false;
		string command;
		string answer;

		if(m_oConnect == NULL)
			return "###WARNING 001";
		if(m_pSendTransaction == NULL)
		{
			bExist= false;
			m_pSendTransaction= new OutsideClientTransaction();
			m_oConnect->newTranfer(m_pSendTransaction, true);
			if(hold == true)
				bHold= true;
			m_pSendTransaction->setCommand(m_sProcessName + " SEND", true);
			if(!m_oConnect->init())
				return "ERROR 003";
			answer= m_pSendTransaction->getAnswer();
			if(	answer.substr(0, 5) == "ERROR"
				||
				answer.substr(0, 7) == "WARNING"	)
			{
				return answer;
			}
		}else
		{
			bExist= true;
			bHold= true;
		}
		command= toProcess + " " + method;
		if(params != "")
		{
			command+= " ";
			command+= params;
		}
		m_pSendTransaction->setCommand(command, bHold);
		m_oConnect->init();
		answer= m_pSendTransaction->getAnswer();
		if(bHold == false)
		{
			m_oConnect->close();
			m_oConnect->newTranfer(NULL, /*delete old*/true);
			m_pSendTransaction= NULL;
		}
		return answer;
	}

	bool Process::running()
	{
		bool bRv;

		if(m_nProcessID == 0)
		{
			if(sendMethod(m_sProcessName, "running", "", /*hold*/false) != "true")
				return false;
			return true;
		}
		LOCK(m_PROCESSRUNNING);
		bRv= m_bRun;
		UNLOCK(m_PROCESSRUNNING);
		return bRv;
	}

	bool Process::stopping()
	{
		bool bRv;

		if(m_nProcessID == 0)
		{
			//toDo: asking over connection
			//return false;
			if(sendMethod(m_sProcessName, "stopping", "", /*hold*/false) != "true")
				return false;
			return true;
		}
		LOCK(m_PROCESSSTOPPING);
		bRv= m_bStop;
		UNLOCK(m_PROCESSSTOPPING);
		return bRv;
	}

	Process::~Process()
	{
	}

}
