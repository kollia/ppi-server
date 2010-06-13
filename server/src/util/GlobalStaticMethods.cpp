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

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>

#include "GlobalStaticMethods.h"
#include "Thread.h"

#include "../logger/lib/LogInterface.h"


string GlobalStaticMethods::m_sProcessName("unknown process");

using namespace logger;

void GlobalStaticMethods::stopMessage(const string& message)
{
#ifdef _APPLICATIONSTOPMESSAGES
	ostringstream msg;

	msg << "[" << m_sProcessName << "(" << Thread::gettid() << ")] " << message << endl;
	cout << msg.str();
#endif //_APPLICATIONSTOPMESSAGES
}

void GlobalStaticMethods::threadStopMessage(const string& message)
{
#ifdef _APPLICATIONTHREADSTOPMESSAGES
	ostringstream msg;

	msg << "[" << m_sProcessName << "(" << Thread::gettid() << ")] " << message << endl;
	cout << msg.str();
#endif //_APPLICATIONTHREADSTOPMESSAGES
}

void GlobalStaticMethods::setSignals(const string& process)
{
	if(signal(SIGINT, signalconverting) == SIG_ERR)
		printSigError("SIGINT", process);
	if(signal(SIGHUP, signalconverting) == SIG_ERR)
		printSigError("SIGHUP", process);
	if(signal(SIGSEGV, signalconverting) == SIG_ERR)
		printSigError("SIGSEGV", process);
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		printSigError("SIGPIPE", process);
}

void GlobalStaticMethods::printSigError(const string& cpSigValue, const string& process)
{
	string msg;

	msg= "cannot initial signal \"";
	msg+= cpSigValue;
	msg+= "\" for process " + process;
	msg+= "\"\nSystem-ERROR: ";
	msg+= strerror(errno);
	cerr << msg << endl;
	LOG(LOG_ERROR, msg);
}

void GlobalStaticMethods::signalconverting(int nSignal)
{
	string msg;
	LogInterface *log= LogInterface::instance();

	if(	log
		&&
		!log->running()	)
	{
		log= NULL;
	}
	switch(nSignal)
	{
		case SIGINT:
			cout << "SIGINT: \"" << m_sProcessName << "\"" << " terminated by user" << endl;
			exit(0);
			break;

		case SIGHUP:
			msg= StatusLogRoutine::getStatusInfo("clients");
			if(log)
				LOG(LOG_INFO, msg);
			cout << endl << msg << endl;
			break;

		case SIGSEGV:
			cout << "SIGSEGV: \"" << m_sProcessName << "\" close from system" << endl;
			exit(0);
			break;
	}
}

