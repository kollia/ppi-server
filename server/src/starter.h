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

#include "util/properties/configpropertycasher.h"

#include "server/libs/server/ServerThread.h"

using namespace std;
using namespace util;
using namespace ports;

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
		Properties m_oServerFileCasher;
		/**
		 * all types in any subroutine whitch be reached over an Owserver instance
		 */
		vector<string> m_vOWServerTypes;

		/**
		 * create instance of all subroutine classes whitch are set in measure.conf
		 */
		void createPortObjects();
		void isNoPathDefinedStop();
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
		void checkAfterContact();
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
		Starter(string workdir) :
			m_sWorkdir(workdir),
			m_oServerFileCasher()
			{ };
		/**
		 * start server
		 *
		 * @return whether server running correctly
		 */
		bool execute();
		/**
		 * ask server for running status
		 *
		 * @return whether can be connect to server correctly
		 */
		bool status();
		/**
		 * stop server
		 *
		 * @return whether server can be sopped correctly
		 */
		bool stop();
		//which ports as string are needeD. Second pair object bool is whether the port is defined for pin reading with ioperm()
		void readFile(vector<pair<string, PortTypes> > &vlRv, string fileName);
		~Starter();
};

#endif /*SERVER_H_*/
