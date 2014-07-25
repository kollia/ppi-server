/**
 *   This file 'PPIConfigFileStructure.h' is part of ppi-server.
 *   Created on: 30.05.2014
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

#ifndef PPICONFIGFILESTRUCTURE_H_
#define PPICONFIGFILESTRUCTURE_H_

#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../../pattern/util/LogHolderPattern.h"
#include "../../pattern/util/imeasurepattern.h"

#include "../smart_ptr.h"
#include "../debug.h"
#include "../GlobalStaticMethods.h"
#include "../URL.h"

#include "measureStructures.h"
#include "interlacedproperties.h"
#include "interlacedactionproperties.h"

namespace util
{
	using namespace std;
	using namespace design_pattern_world::util_pattern;

	/**
	 * type definition of PPIConfigFileStructure
	 * in an shared pointer
	 */
	typedef SHAREDPTR::shared_ptr<class PPIConfigFileStructure> PPIConfigFiles;

	/**
	 * object to reading configuration files
	 * of server.conf and measure.conf from hard disk
	 * deploy entries of main server configuration
	 * and also structure of folder subroutine properties
	 *
	 * @author Alexander Kolli
	 * @version 1.0
	 */
	class PPIConfigFileStructure
	{
	public:
		/**
		 * constructor to initial
		 * member variables
		 *
		 * @param workingDir working directory
		 * @param bFirst whether object should write errors by fault reading
		 */
		PPIConfigFileStructure(const string& workingDir, const bool bFirst)
		: m_bFirstReading(bFirst),
		  m_sConfigPath(URL::addPath(workingDir, PPICONFIGPATH, /*always*/false)),
		  m_sServerConf("server.conf"),
		  m_sMeasureConf("measure.conf")
		{};
		/**
		 * creating single pattern instance of object
		 *
		 * @param workingDir working directory
		 * @param bFirst whether object should write errors by fault reading
		 * @return whether creating instance of object was correct
		 */
		static void init(const string& workingDir, const bool bFirst);
		/**
		 * deploying instance of own object
		 *
		 * @return instance of object
		 */
		static SHAREDPTR::shared_ptr<PPIConfigFileStructure> instance()
				{ return __instance; }
		/**
		 * reading main configuration file for server
		 */
		void readServerConfig();
		/**
		 * return directory of configuration path
		 *
		 * @return configuration path
		 */
		string getConfigPath()
		{ return m_sConfigPath; };
		/**
		 * get defined external port interfaces names
		 */
		set<string> getPortIntercfaceNames();
		/**
		 * check whether should database server started
		 *
		 * @return whether start
		 */
		bool startDbServer();
		/**
		 * read how much extra database write threads
		 * needed for all folder to write into database
		 *
		 * @return -1        - not thread, writing directly
		 *          0        - creating for every folder an thread
		 *          <number> - count of threads writing into database
		 */
		short getFolderDbThreads();
		/**
		 * check whether should start any external port interface
		 *
		 * @return whether start
		 */
		bool startPortInterfaces();
		/**
		 * check whether should start internet server
		 *
		 * @return whether start
		 */
		bool startInternetServer();
		/**
		 * return time interval for searching of external ports
		 * when no access is given
		 */
		time_t getPortSearchInterval();
		/**
		 * return communication host
		 * defined inside server.conf
		 *
		 * @return IP address or host name
		 */
		string getCommunicationHost();
		/**
		 * return host for internet communication
		 * defined inside server.conf
		 *
		 * @return IP address or host name
		 */
		string getInternetHost();
		/**
		 * return communication port
		 * defined inside server.conf
		 *
		 * @return port number
		 */
		unsigned short getCommunicationPort();
		/**
		 * return port for internet communication
		 * defined inside server.conf
		 *
		 * @return port number
		 */
		unsigned short getInternetPort();
		/**
		 * read how much time should pass for logging again
		 * when inside source and TIMERLOG macro be set
		 */
		int getTimerLogSeconds();
		/**
		 * read whether should splitting reachend and finishedtime values
		 * for extra CPU time
		 */
		pair<short, short> getCPUtimeSplitting();
		/**
		 * reading server.conf properties for specific external port
		 *
		 * @param port external port name
		 */
		const IPropertyPattern* getExternalPortProperties(const string& port);
		/**
		 * deploy default user for all processes
		 * running for ppi-server
		 *
		 * @return user name
		 */
		string getDefaultUser();
		/**
		 * deploy default user for
		 * all external ports
		 *
		 * @return user name
		 */
		string getExternalPortUser();
		/**
		 * deploy logging level from server.conf
		 *
		 * @return log level
		 */
		int getLogLevel();
		/**
		 * deploy logging level name from server.conf
		 *
		 * @return name of log level
		 */
		string getLogLevelName();
		/**
		 * deploy passwd file with all user names
		 * and ID's from system
		 */
		string getPasswdFile();
		/**
		 * read all measure control lists
		 * of folders with subroutines
		 */
		bool readMeasureConfig();
		/**
		 * deploy read working list folder
		 *
		 * @return working list folder
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> getWorkingList()
				{ return m_tFolderStart; };
		/**
		 * deploy all subroutines from specific folder
		 *
		 * @param folder name of folder
		 * @return folder list
		 */
		vector<sub>* getFolderList(const string& folder);
		/**
		 * deploy specific subroutine
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return subroutine
		 */
		sub* getSubroutine(const string& folder, const string& subroutine);
		/**
		 * deploy properties of specific subroutine
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return properties of subroutine
		 */
		SHAREDPTR::shared_ptr<IActionPropertyPattern> getSubroutineProperties(const string& folder,
																			const string& subroutine);
		/**
		 * create command inside specific folder
		 * when any subvar defined in command string
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param def which command should be formed
		 * @param command string of command with maybe contain an subvar
		 * @return correct command string
		 */
		string createCommand(const string& folder, const string& subroutine, const string& def, string command);

	private:
		/**
		 * instance of object
		 */
		static SHAREDPTR::shared_ptr<PPIConfigFileStructure> __instance;
		/**
		 * whether write errors for first reading
		 */
		const bool m_bFirstReading;
		/**
		 * path in which sub directory configuration files laying
		 */
		const string m_sConfigPath;
		/**
		 * main configuration file name
		 */
		const string m_sServerConf;
		/**
		 * file name of first measure properties
		 */
		const string m_sMeasureConf;
		/*
		 * properties of main server configuration
		 */
		InterlacedProperties m_oServerFileCasher;
		/**
		 * exist external founded serial ports
		 * inside measure configuration files
		 */
		vector<pair<string, PortTypes> > m_vPortTypes;
		/**
		 * hole measure control list
		 * of folders with subroutines
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> m_tFolderStart;
	};


} /* namespace util */



#endif /* PPICONFIGFILESTRUCTURE_H_ */
