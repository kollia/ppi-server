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
	/**********************************************************************************************\
		SIGHUP     │    1 │ A      │ Verbindung beendet (aufgehängt)
		SIGINT     │    2 │ A      │ Interrupt-Signal von der Tastatur
		SIGQUIT    │    3 │ A      │ Quit-Signal von der Tastatur
		SIGILL     │    4 │ A      │ Falsche Instruktion
		SIGTRAP    │    5 │ CG     │ Überwachung/Stopp-Punkt
		SIGABRT    │    6 │ C      │ Abbruch
		SIGUNUSED  │    7 │ AG     │ Nicht verwendet
		SIGFPE     │    8 │ C      │ Fließkomma-Überschreitung
		SIGKILL    │    9 │ AEF    │ Beendigungssignal
		SIGUSR1    │   10 │ A      │ Benutzer-definiertes Signal 1
		SIGSEGV    │   11 │ C      │ Ungültige Speicherreferenz
		SIGUSR2    │   12 │ A      │ Benutzer-definiertes Signal 2
		SIGPIPE    │   13 │ A      │ Schreiben in eine Pipeline ohne Lesen
		SIGALRM    │   14 │ A      │ Zeitsignal von alarm(1).
		SIGTERM    │   15 │ A      │ Beendigungssignal
		SIGSTKFLT  │   16 │ AG     │ Stack-Fehler im Coprozessor
		SIGCHLD    │   17 │ B      │ Kindprozess beendet
		SIGCONT    │   18 │        │ Weiterfahren, wenn gestoppt
		SIGSTOP    │   19 │ DEF    │ Prozessstopp
		SIGTSTP    │   20 │ D      │ Stopp getippt an einem TTY
		SIGTTIN    │   21 │ D      │ TTY-Eingabe für Hintergrundprozesse
		SIGTTOU    │   22 │ D      │ TTY-Ausgabe für Hintergrundprozesse
		SIGIO      │   23 │ AG     │ E/A-Fehler
		SIGXCPU    │   24 │ AG     │ CPU-Zeitlimite überschritten
		SIGXFSZ    │   25 │ AG     │ Dateien Größenlimite überschritten
		SIGVTALRM  │   26 │ AG     │ Virtueller Zeitalarm (???)
		SIGPROF    │   27 │ AG     │ Profile Signal
		SIGWINCH   │   29 │ BG     │ Fenstergrößenänderung
	\**********************************************************************************************/

	if(signal(SIGINT, signalconverting) == SIG_ERR)
		printSigError("SIGINT", process);
	if(signal(SIGHUP, signalconverting) == SIG_ERR)
		printSigError("SIGHUP", process);
	if(signal(SIGSEGV, signalconverting) == SIG_ERR)
		printSigError("SIGSEGV", process);
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		printSigError("SIGPIPE", process);

#if 0
	// set all signals for debugging
	//signal(SIGHUP, signalconverting);
	//signal(SIGINT, signalconverting);
	signal(SIGQUIT, signalconverting);
	signal(SIGILL, signalconverting);
	signal(SIGTRAP, signalconverting);
	signal(SIGABRT, signalconverting);
	signal(SIGUNUSED, signalconverting);
	signal(SIGFPE, signalconverting);
	signal(SIGKILL, signalconverting);
	signal(SIGUSR1, signalconverting);
	//signal(SIGSEGV, signalconverting);
	signal(SIGUSR2, signalconverting);
	//signal(SIGPIPE, signalconverting);
	signal(SIGALRM, signalconverting);
	signal(SIGTERM , signalconverting);
	signal(SIGSTKFLT, signalconverting);
	signal(SIGCHLD , signalconverting);
	signal(SIGCONT, signalconverting);
	signal(SIGSTOP, signalconverting);
	signal(SIGTSTP, signalconverting);
	signal(SIGTTIN, signalconverting);
	signal(SIGTTOU , signalconverting);
	signal(SIGIO, signalconverting);
	signal(SIGXCPU , signalconverting);
	signal(SIGXFSZ , signalconverting);
	signal(SIGVTALRM, signalconverting);
	signal(SIGPROF, signalconverting);
	signal(SIGWINCH, signalconverting);
#endif // 0
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
	//LogHolderPattern *log= LogHolderPattern::instance();

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
			LOG(LOG_INFO, msg);
			cout << endl << msg << endl;
			break;*/

		case SIGSEGV:
			cout << "SIGSEGV: \"" << m_sProcessName << "\" close from system" << endl;
			exit(EXIT_FAILURE);
			break;
		// definition of all other signals
		default:
			cout << "system sending signal (" << nSignal << ") to process " << m_sProcessName << endl;
			break;
	}
}

bool GlobalStaticMethods::replaceName(string& name, const string& type)
{
	bool fault= false;
	string::size_type p;

	p= name.find("+");
	if(p != string::npos)
		fault= true;
	p= name.find("-");
	if(p != string::npos)
		fault= true;
	p= name.find("/");
	if(p != string::npos)
		fault= true;
	p= name.find("*");
	if(p != string::npos)
		fault= true;
	p= name.find("<") ;
	if(p != string::npos)
		fault= true;
	p= name.find(">");
	if(p != string::npos)
		fault= true;
	p= name.find("=");
	if(p != string::npos)
		fault= true;
	p= name.find("(");
	if(p != string::npos)
		fault= true;
	p= name.find(")");
	if(p != string::npos)
		fault= true;
	p= name.find("!");
	if(p != string::npos)
		fault= true;
	p= name.find(":");
	if(p != string::npos)
		fault= true;
	p= name.find("&");
	if(p != string::npos)
		fault= true;
	p= name.find("|");
	if(p != string::npos)
		fault= true;
	p= name.find("?");
	if(p != string::npos)
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

