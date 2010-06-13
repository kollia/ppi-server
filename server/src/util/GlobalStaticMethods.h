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

#include <string>

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
	 */
	static void stopMessage(const string& message);
	/**
	 * message output on command line by get stop command
	 * for threads and processes
	 * and definition of _APPLICATIONTHREADSTOPMESSAGE be set
	 *
	 * @param message content of string message
	 */
	static void threadStopMessage(const string& message);

private:
	/**
	 * Name of Process which current running
	 */
	static string m_sProcessName;
};

typedef GlobalStaticMethods glob;

#endif /* GLOBALSTATICMETHODS_H_ */