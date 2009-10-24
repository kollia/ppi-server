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
#include <vector>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <string.h>

#include <boost/algorithm/string/replace.hpp>

#include "LogProcess.h"
#include "lib/LogInterface.h"

#include "../util/URL.h"
#include "../util/IMethodStringStream.h"

#include "../pattern/util/ipropertypattern.h"

LogProcess::LogProcess(const uid_t uid, IClientConnectArtPattern* getConnection, IClientConnectArtPattern* sendConnection/*= NULL*/, const bool wait/*= true*/)
:	Process("LogServer", sendConnection, getConnection, wait)
{
	m_uid= uid;
}

int LogProcess::init(void* arg)
{
	int ret, nLogLevel, nLogAllSec, nLogDays;
	unsigned short nDelete;
	string logpath, workdir, property, sLogLevel;
	IPropertyPattern* oServerFileCasher= static_cast<IPropertyPattern*>(arg);

	if(logger::LogInterface::instance() != NULL)
		delete logger::LogInterface::instance();
	setuid(m_uid);
	ret= openGetConnection();
	if(	ret > 0
		&&
		ret != 35	)
	{ // by error number 35
	  // no communication server is listening on port
	  // try again later
		return ret;
	}

	/***************************************************************************************
	 **  read log properties															****/

	workdir= oServerFileCasher->getValue("workdir", false);
	logpath= URL::addPath(workdir, PPILOGPATH, /*always*/false);
	logpath= URL::addPath(logpath, "ppi-server_");
	property= "log";
	sLogLevel= oServerFileCasher->getValue(property, false);
	if(sLogLevel == "DEBUG")
		nLogLevel= LOG_DEBUG;
	else if(sLogLevel == "INFO")
		nLogLevel= LOG_INFO;
	else if(sLogLevel == "SERVER")
		nLogLevel= LOG_SERVER;
	else if(sLogLevel == "WARNING")
		nLogLevel= LOG_WARNING;
	else if(sLogLevel == "ERROR")
		nLogLevel= LOG_ERROR;
	else if(sLogLevel == "ALERT")
		nLogLevel= LOG_ALERT;
	else
	{
		cerr << "### WARNING: undefined log level '" << sLogLevel << "' in config file server.conf" << endl;
		cerr << "             set log-level to DEBUG" << endl;
		nLogLevel= LOG_DEBUG;
	}
	property= "timelogSec";
	nLogAllSec= oServerFileCasher->getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	property= "newfileAfterDays";
	nLogDays= oServerFileCasher->getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogDays= 30;
	}
	property= "deleteLogFiles";
	nDelete= oServerFileCasher->getUShort(property);

	/***************************************************************************************/

	m_pLogThread= new LogThread(/*check identif logs*/false);
	if(!m_pLogThread)
		return 1;
	m_pLogThread->setProperties(logpath, nLogLevel, nLogAllSec, nLogDays, nDelete);
	if(m_pLogThread->start() > 0)
	{
		closeGetConnection();
		return 2;
	}
	m_pLogThread->setThreadName(getProcessName());
	return 0;
}

int LogProcess::execute()
{
	int err;
	string question, command;
	string::size_type pos;

	question= getQuestion(m_sAnswer);
	err= error(question);
	if(err != 0)
	{
		if(err < 0 || err == 35)
		{ 	// by error number 35
			// no communication server is listening on port
			// or err is an warning (under 0)
			// try again later
			return 0;
		}
		return err;
	}
	IMethodStringStream stream(question);

	command= stream.getMethodName();
	//cout << "LogServer get command '" << question << "'" << endl;
	if(command == "init")
		m_sAnswer= "done";
	else if(command == "setThreadName")
	{
		string threadName;
		pthread_t threadID;

		stream >> threadName;
		stream >> threadID;
		m_pLogThread->setThreadName(threadName, threadID);
		m_sAnswer= "";
	}else if(command == "log")
	{
		log_t tMessage;

		tMessage.line= -1;
		tMessage.pid= 0;
		tMessage.thread= 0;
		tMessage.tid= 0;
		tMessage.tmnow= 0;
		tMessage.type= -1;
		stream >> tMessage.file;
		stream >> tMessage.line;
		stream >> tMessage.type;
		stream >> tMessage.message;
		stream >> tMessage.pid;
		stream >> tMessage.tid;
		stream >> tMessage.thread;
		stream >> tMessage.identif;
		m_pLogThread->log(tMessage);
		m_sAnswer= "";
	}else if(command == "stop")
	{
		m_pLogThread->stop(false);
		if(m_pLogThread->running())
		{
			m_sAnswer= "stop logging client";
			usleep(500000);
		}else
			m_sAnswer= "done";

	}else if(command == "stop-OK")
	{
		closeGetConnection();
		stop(false);

	}else
	{
		log_t msg;

		msg.file= __FILE__;
		msg.line= __LINE__;
		msg.message= " LogServer get unknown command '" + question + "'\n";
		msg.message+=" sending back WARNING 001";
		msg.pid= getpid();
		msg.tid= Thread::gettid();
		msg.thread= pthread_self();
		time(&msg.tmnow);
		msg.type= LOG_WARNING;
		m_pLogThread->log(msg);
		m_sAnswer= "WARNING 001";
	}
	return 0;
}

void LogProcess::ending()
{
	m_pLogThread->stop(true);
	delete m_pLogThread;
}
