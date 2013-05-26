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
#ifndef SERVER_H_
#define SERVER_H_

#include <vector>
#include <map>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "util/smart_ptr.h"
#include "util/structures.h"

#include "pattern/util/ICommandStructPattern.h"

#include "util/properties/interlacedproperties.h"

#include "server/libs/server/ServerThread.h"

using namespace std;
using namespace util;

extern server::ServerThread* gInternetServer;

void signalconverting(int nSignal);

class Starter
{
	private:
		//string m_sHost;
		//unsigned short m_nDefaultPort;
		//unsigned short m_nDefaultSleep;
		string m_sWorkdir;
		string m_sConfPath;
		//string m_sLogFile;
		//string m_sMeasureFile;
		//int m_nLogLevel;
		//int m_nLogAllSec;
		//int m_nWriteLogDays;
		unsigned short m_nMeasuredness;
		unsigned short m_nMeasurednessCount;
		unsigned short m_nMicrosecCount;
		unsigned short m_nMeasureTimes;
		SHAREDPTR::shared_ptr<measurefolder_t> m_tFolderStart;
		vector<ohm> m_vOhm;
		vector<correction_t> m_vCorrection;
		/**
		 * user id for defaultuser setting in server.conf
		 */
		uid_t m_tDefaultUser;
		/**
		 * user id for loguser setting in server.conf
		 */
		uid_t m_tLogUser;
		/*
		 * casher of defined variables in file server.conf
		 */
		InterlacedProperties m_oServerFileCasher;
		/**
		 * all types of owreader defined in server.conf
		 */
		set<string> m_vOWReaderTypes;
		/**
		 * all types of owreader defined in <code>m_vOWReaderTypes</code>
		 * and used in any subroutine of one measure.conf file
		 */
		set<string> m_vOWReaderNeed;

		/**
		 * create defined folder list beginning by measure.conf
		 *
		 * @param shellstarter list of system user which should starting an command inside specific user account on system
		 */
		void createFolderLists(set<string>& shellstarter);
		/**
		 * configure all subroutine classes which are created in method <code>createFolderLists()</code>
		 *
		 * @param bShowConf whether should shown on command line which folder will be configured
		 * @param bSubs whether should shown also subroutines
		 */
		void configurePortObjects(bool bShowConf, bool bSubs);
		void isNoPathDefinedStop();
		/**
		 * write error or warning from server out to command line
		 *
		 * @param err error/warning number getting from server
		 * @param fp handle to open connection
		 * @return whether string was an error (true) or warning (false)
		 */
		bool writeErrString(const string& err, FILE *fp) const;
		/**
		 * search entry in an vector of used ports
		 *
		 * @param vec vector of used ports
		 * @param port which port should be find
		 * @return pointer to entry, or point of last
		 */
		vector<pair<string, PortTypes> >::iterator find(vector<pair<string, PortTypes> >& vec, string port);

	protected:
		//bool openPort(unsigned long nPort, int nBaud, char cParitaetsbit, unsigned short nDatabits, unsigned short nStopbit);
		bool checkServer();
		/**
		 * read etc/passwd to find user id for setting
		 * defaultuser and loguser in server.conf
		 */
		void readPasswd();
#if 0
		void checkAfterContact();
#endif
		/**
		 * read measure procedures from measure.conf
		 *
		 * @param fileName file name with path from which should read
		 * @return vector with all needet ports
		 */
		vector<unsigned long> readMeasureConf(string path);
		/**
		 * write signal error
		 *
		 * @param cpSigValue error code
		 */
		void printSigError(const string cpSigValue);

	public:
		Starter() :
			m_oServerFileCasher()
			{ };
		/**
		 * start server
		 *
		 * @param commands object of commands to know which options be set
		 * @return whether server running correctly
		 */
		bool execute(const IOptionStructPattern* commands);
		/**
		 * ask server for running status
		 *
		 * @return whether can be connect to server correctly
		 */
		bool status();
		/**
		 * stop server
		 *
		 * @param config show on command line to read configuration
		 * @return whether server can be sopped correctly
		 */
		bool stop(bool config);
		//which ports as string are needeD. Second pair object bool is whether the port is defined for pin reading with ioperm()
		void readFile(vector<pair<string, PortTypes> > &vlRv, string fileName);
		/**
		 * set directory of working
		 *
		 * @param workdir working directory
		 */
		void setWorkingDirectory(const string& workdir)
		{ m_sWorkdir= workdir; };
		~Starter();
};

#endif /*SERVER_H_*/
