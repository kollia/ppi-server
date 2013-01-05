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
#ifndef GLOBALSTATICMETHODS_H_
#define GLOBALSTATICMETHODS_H_

#include <unistd.h>

#include <string>
#include <map>

#include "debug.h"

using namespace std;

class GlobalStaticMethods {
public:
	/**
	 * return defined process name
	 *
	 * @return name of process
	 */
	static string getProcessName()
	{ return m_sProcessName; };
	/**
	 * signal procedure for set signal
	 *
	 * @param nSignal signal getting from system
	 */
	static void signalconverting(int nSignal);
	/**
	 * set all needed signals for process
	 *
	 * @param process name of running process
	 */
	static void setSignals(const string& process);
	/**
	 * write error for signals on command line and into log-file
	 *
	 * @param cpSigValue used signal
	 * @param process name of running process
	 */
	static void printSigError(const string& cpSigValue, const string& process);
	/**
	 * set by beginning name of process for any message output
	 *
	 * @param process name of process
	 */
	static void processName(const string& process)
	{ m_sProcessName= process; };
	/**
	 * message output on command line by get stop command
	 * and definition of _APPLICATIONSTOPMESSAGE be set
	 *
	 * @param message content of string message
	 * @param all display stop message by all predefined process names
	 */
	static void stopMessage(const string& message, bool all= false);
	/**
	 * message output on command line by get stop command
	 * for threads and processes
	 * and definition of _APPLICATIONTHREADSTOPMESSAGE be set
	 *
	 * @param message content of string message
	 */
	static void threadStopMessage(const string& message);
	/**
	 * replace + - / * < > = ( ) ! in the given name when exist
	 * and make also an underline before if the name beginns with an number
	 *
	 * @param name string to replace
	 * @param type how the string name calls for write out on command line.<br/>if no type given, method do not display anything
	 * @param change whether should change string and write description for problem or not
	 * @return whether the name was changed
	 */
	static bool replaceName(string& name, const string& type= "", bool change= true);
	/**
	 * read for given user ID from passwd
	 *
	 * @param passwd file with path where finding entrys
	 * @param users map of users as key, by returning procedure the value should has user uid
	 * @return whether procedure found all users
	 */
	static bool readPasswd(const string& passwd, map<string, uid_t>& users);
	/**
	 * define binary string
	 *
	 * @param value pin which should set as binary string
	 * @param bits count of bits set in string
	 * @return binary string
	 */
	static string getBinString(const long value, const size_t bits);

private:
	/**
	 * Name of Process which current running
	 */
	static string m_sProcessName;
};

typedef GlobalStaticMethods glob;

#endif /* GLOBALSTATICMETHODS_H_ */
