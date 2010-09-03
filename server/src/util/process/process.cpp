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

#include "../stream/OMethodStringStream.h"

#include "../../pattern/util/LogHolderPattern.h"
//#include "../../logger/lib/LogInterface.h"

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

	int Process::stop(const bool bWait/*= true*/)
	{
		if(m_nProcessID == 0)
		{
			OMethodStringStream stop("stop");
			int err, err2= 0;
			string answer;

			err= openSendConnection();
			if(err > 0)
				return err;
			answer= sendMethod(m_sToClient, stop, bWait);
			if(err == 0)
				err2= closeSendConnection();
			err= error(answer);
			if(err != 0)
				return err;
			if(err2 > 0)
				return err2;
			if(answer != "done")
				return 4;
			return 0;
		}
		LOCK(m_PROCESSSTOPPING);
		m_bStop= true;
		if(bWait)
			CONDITION(m_PROCESSSTOPCOND, m_PROCESSSTOPPING);
		UNLOCK(m_PROCESSSTOPPING);
		return 0;
	}

	int Process::start(void *args/*= NULL*/, bool bHold/*= false*/)
	{
		string processName(getProcessName());
		string debugProcessName("");//("LogServer");//("CommunicationServerProcess");//
		pid_t nProcess;
		ostringstream stream;

		if((nProcess= fork()) < 0)
			return 2;
		if(	(	debugProcessName == processName
				&&
				nProcess > 0						)
			||
			(	debugProcessName != processName
				&&
				nProcess == 0						)	)
		{
			int ret;
			ret= runprocess(args, bHold);
			exit(ret);
		}
		//if(getProcessName() == "LogServer")
		//	sleep(5);
		if(	m_bWaitInit	)
		{
			int ret;

			ret= check();
			if(ret > 0)
				return ret;
		}
		if(bHold)
		{
			int err, err2= 0;
			string answer;
			OMethodStringStream waiting("waiting");

			answer= sendMethod(m_sToClient, waiting, true);
			err= error(answer);
			if(err == 0)
				err2= closeSendConnection();
			if(err != 0)
				return err;
			if(err2 > 0)
				return err2;
			if(answer != "done")
				return 3;
		}
		return 0;
	}

	int Process::check()
	{
		int err, err2= 0;
		string answer;
		OMethodStringStream init("init");

		err= openSendConnection();
		if(err > 0)
			return err;
		answer= sendMethod(m_sToClient, init, true);
		if(err == 0)
			err2= closeSendConnection();
		err= error(answer);
		if(err != 0)
			return err;
		if(err2 > 0)
			return err2;
		if(answer != "done")
			return 3;
		return 0;
	}

	int Process::run(void *args/*= NULL*/)
	{
		return runprocess(args, false);
	}

	int Process::runprocess(void* args, bool bHold)
	{
		int ret= 0;

		m_bRun= true;
		m_nProcessID= getpid();
		initstatus(getProcessName(), this);
		ret= init(args);
		while(	ret <= 0
				&&
				!stopping()	)
		{
			POS("###THREAD_execute_start");
			ret= execute();
		}
		AROUSEALL(m_PROCESSSTOPCOND);
		ending();
		removestatus(m_nProcessID);
		return ret;
	}

	string Process::strerror(int error)
	{
		string str;

		switch(error)
		{
		case 0:
			str= "no error occurred";
			break;
		case 2:
			str= "cannot fork process";
			break;
		case 3:
			str= "cannot correctly check initialization from new process, "
		  			"maybe connection was failed or server give back wrong answer (not 'done')";
			break;
		case 4:
			str= "cannot correctly check stopping from process, "
		  			"maybe connection was failed or server give back wrong answer (not 'done')";
			break;
		default:
			str= ExternClientInputTemplate::strerror(error);
			break;
		}
		return str;
	}

	int Process::running()
	{
		int nRv= 0;
		string answer;

		if(m_nProcessID == 0)
		{
			OMethodStringStream running("running");

			answer= sendMethod(m_sToClient, running, true);
			if(answer == "true")
				return 1;
			if(answer == "false")
				return 0;
			return error(answer);
		}
		LOCK(m_PROCESSRUNNING);
		if(m_bRun)
			nRv= 1;
		UNLOCK(m_PROCESSRUNNING);
		return nRv;
	}

	int Process::stopping()
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
			return error(answer);
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
