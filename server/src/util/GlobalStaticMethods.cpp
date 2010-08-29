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

#include <boost/algorithm/string/replace.hpp>

#include "GlobalStaticMethods.h"

#include "../pattern/util/LogHolderPattern.h"


string GlobalStaticMethods::m_sProcessName("unknown process");

using namespace boost;

void GlobalStaticMethods::stopMessage(const string& message, bool all/*= false*/)
{
#ifdef _APPLICATIONSTOPMESSAGES
	ostringstream msg;

	if(	all ||
		_APPLICATIONSTOPMESSAGES == m_sProcessName ||
		_APPLICATIONSTOPMESSAGES == ""					)
	{
		msg << "[" << m_sProcessName << "(" << Thread::gettid() << ")] " << message << endl;
		cout << msg.str();
	}
#endif //_APPLICATIONSTOPMESSAGES
}

void GlobalStaticMethods::threadStopMessage(const string& message)
{
#ifdef _APPLICATIONTHREADSTOPMESSAGES
	ostringstream msg;

	if(	_APPLICATIONSTOPMESSAGES == m_sProcessName ||
		_APPLICATIONSTOPMESSAGES == ""					)
	{
		msg << "[" << m_sProcessName << "(" << Thread::gettid() << ")] " << message << endl;
		cout << msg.str();
	}
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
	LogHolderPattern *log= LogHolderPattern::instance();

/*	if(	log
		&&
		!log->running()	)
	{
		log= NULL;
	}*/
	switch(nSignal)
	{
		case SIGINT:
			cout << "SIGINT: \"" << m_sProcessName << "\"" << " terminated by user" << endl;
			exit(0);
			break;

// 2010/08/18 ppi@magnificat.at:	remove getStatusInfo for signal SIGHUP
//									because cycle dependency not allowed for shared library
//									libppiutil.so <-> libppithreadutil.so
/*		case SIGHUP:
			msg= StatusLogRoutine::getStatusInfo("clients");
			if(log)
				LOG(LOG_INFO, msg);
			cout << endl << msg << endl;
			break;*/

		case SIGSEGV:
			cout << "SIGSEGV: \"" << m_sProcessName << "\" close from system" << endl;
			exit(0);
			break;
	}
}

bool GlobalStaticMethods::replaceName(string& name, const string& type)
{
	bool fault= false;
	string::size_type p;
	string::size_type len= name.length();

	p= name.find("+");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("-");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("/");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("*");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("<") ;
	if(p >= 0 && p < len)
		fault= true;
	p= name.find(">");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("=");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("(");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find(")");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("!");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find(":");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("&");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("|");
	if(p >= 0 && p < len)
		fault= true;
	p= name.find("?");
	if(p >= 0 && p < len)
		fault= true;
	if(isdigit(name[0]))
		name= "_" + name;
	if(fault)
	{
		if(type != "")
			cout << "### WARNGING: in " << type << " '" << name << "' do not use + - / * < > = ( ) ! : & | ?" << endl;
		replace_all(name, "+", "_PLUS_");
		replace_all(name, "-", "_MINUS_");
		replace_all(name, "/", "_THRU_");
		replace_all(name, "*", "_MULTI_");
		replace_all(name, "<", "_LT_");
		replace_all(name, ">", "_GT_");
		replace_all(name, "=", "_IS_");
		replace_all(name, "(", "_BREAKON_");
		replace_all(name, ")", "_BREAKOFF_");
		replace_all(name, "!", "_EXMARK_");
		replace_all(name, ":", "_COLON_");
		replace_all(name, "&", "_AND_");
		replace_all(name, "|", "_OR_");
		replace_all(name, "?", "_QMARK_");
		if(type != "")
		{
			cout << "              actual " << type << " is now '" << name << "'" << endl;
			cout << "              the problem is when you refer in an begin, while, or end property with the wrong name, it find no result." << endl;
		}

	}
	return fault;
}

