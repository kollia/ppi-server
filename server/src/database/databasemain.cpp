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

#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>

#include <boost/algorithm/string/split.hpp>

#include "../pattern/util/LogHolderPattern.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/URL.h"
#include "../util/debug.h"

#include "../util/properties/interlacedproperties.h"

#include "../server/libs/client/SocketClientConnection.h"
#include "../server/libs/server/TcpServerConnection.h"
#include "../server/libs/server/ServerProcess.h"
#include "../server/libs/server/communicationthreadstarter.h"

#include "logger/LogThread.h"

#include "ServerDbTransaction.h"
#include "DatabaseThread.h"

using namespace std;
using namespace boost;
using namespace util;
using namespace ppi_database;

int main(int argc, char* argv[])
{
	/**
	 * log file name for logging process
	 */
	string logpath;
	/**
	 * minimal log level to write inside log file
	 * as string from server configuration file
	 */
	string sLogLevel;
	/**
	 * minimal log level to write inside log file
	 * as integer
	 */
	int nLogLevel;
	/**
	 * how often TIMELOG is logging the same log message
	 */
	int nLogAllSec;
	/**
	 * after how much days logging process should create an new logging file
	 */
	int nLogDays;
	/**
	 * after how much logging files should be deleted.<br />
	 * -1 is never
	 */
	unsigned short nDelete;

	string defaultuser;
	string workdir;
	string commhost;
	string property;
	string sConfPath, fileName;
	string dbpath;
	unsigned short commport;
	unsigned short nDbConnectors= 0;
	int err;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	InterlacedProperties oServerProperties;
	DatabaseThread* db;
	CommunicationThreadStarter* starter;
	LogThread logObj(/*check*/true, /*waitFirst*/true, /*asServer*/true);
	ServerDbTransaction* pDbTransaction;
	map<string, uid_t> users;

	glob::processName("ppi-db-server");
	glob::setSignals("ppi-db-server");
	if(argc >= 2)
	{ // read how much clients db-server for communication thre
		istringstream oConnT(argv[1]);

		oConnT >> nDbConnectors;
	}
	if(nDbConnectors == 0)
	{
		cerr << "database server need an valid integer variable to know how much communication clients should be started" << endl;
		cerr << " syntax: ppi-db-server <communication threads>" << endl << endl;
		return EXIT_FAILURE;
	}

	// create working directory
	directorys= split(directorys, argv[0], is_any_of("/"));
	dirlen= directorys.size();
	for(vector<string>::size_type c= 0; c < dirlen; ++c)
	{
		if(c == dirlen-2)
		{// directory is bin, Debug or Release
			if(directorys[c] == ".")
				workdir+= "../";
			break;
		}
		workdir+= directorys[c] + "/";
	}
	sConfPath= URL::addPath(workdir, PPICONFIGPATH, /*always*/false);
	dbpath= URL::addPath(workdir, PPIDATABASEPATH, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	oServerProperties.setDelimiter("owreader", "[", "]");
	oServerProperties.modifier("owreader");
	oServerProperties.readLine("workdir= " + workdir);
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: db-server cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}

	defaultuser= oServerProperties.getValue("defaultuser", /*warning*/false);
	if(defaultuser == "")
	{
		cerr << "### WARNING: defaultuser is not defined" << endl;
		cerr << "             so process run under 'nobody'" << endl;
		defaultuser= "nobody";
	}
	users[defaultuser]= 0;

	commhost= oServerProperties.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= oServerProperties.needUShort(property);

	// ------------------------------------------------------------------------------
	// initial logging service
	//
	logpath= URL::addPath(workdir, PPILOGPATH, /*always*/false);
	logpath= URL::addPath(logpath, "ppi-server_");
	property= "log";
	sLogLevel= oServerProperties.getValue(property, false);
	if(sLogLevel == "DEBUG")
		nLogLevel= LOG_DEBUG;
	else if(sLogLevel == "INFO")
		nLogLevel= LOG_INFO;
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
	nLogAllSec= oServerProperties.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	property= "newfileAfterDays";
	nLogDays= oServerProperties.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogDays= 30;
	}
	property= "deleteLogFiles";
	nDelete= oServerProperties.getUShort(property);

	logObj.setProperties(logpath, nLogLevel, nLogAllSec, nLogDays, nDelete);
	LogHolderPattern::init(dynamic_cast<ILogPattern*>(&logObj));
	logObj.start();
	LogHolderPattern::instance()->setThreadName(glob::getProcessName());
	//
	// ------------------------------------------------------------------------------

	bool bGlbCondSet(false), bGlbMutSet(false);

#ifdef CONDITIONSDEBUG
	bGlbCondSet= true;
#endif
#ifdef MUTEXLOCKDEBUG
	bGlbMutSet= true;
#endif
	if(	bGlbCondSet ||
		bGlbMutSet		)
	{
		string msg("\n");

		if(bGlbCondSet)
			msg+= "Globally lock for CONDITIONSDEBUG be defined\n";
		if(bGlbMutSet)
			msg+= "Globally lock for MUTEXLOCKDEBUG be defined\n";
		msg+= "\nThis locking cost performance time";
		LOG(LOG_WARNING, msg);
	}

	LOG(LOG_DEBUG, "starting database");
	starter= new CommunicationThreadStarter(0, nDbConnectors);
	// start initialization from database
	DatabaseThread::initial(dbpath, sConfPath, &oServerProperties);
	db= DatabaseThread::instance();
	db->setCommunicator(starter);

	//*********************************************************************************
	// change user id of process after creating DatabaseThread object
	// because database starting thread of ThinningDatabase with lower priority
	// and need by starting root rights
	if(!glob::readPasswd(oServerProperties.getValue("passwd"), users))
	{
		string msg;

		msg=  "### WARNING: do not found default user " + defaultuser + " inside passwd\n";
		msg+= "             so internet server running as root";
		LOG(LOG_ALERT, msg);
		cerr << msg << endl;
	}else
	{
		if(setuid(users[defaultuser]) != 0)
		{
			string err;

			err=   "### ERROR: cannot set process to default user " + defaultuser + "\n";
			err+=  "    ERRNO: " + *strerror(errno);
			err+= "\n          so internet server running as root";
			LOG(LOG_ALERT, err);
			cerr << err << endl;
		}
	}

	logObj.beginLogging();
	pDbTransaction= new ServerDbTransaction();
	pDbTransaction->setLogObject(&logObj);
	ServerProcess database(	"ppi-db-server", 0, starter,
							new TcpServerConnection(	commhost,
														commport,
														10,
														pDbTransaction	),
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														10			)						);

	//*********************************************************************************
	// starting database process
	err= database.run(&oServerProperties);
	if(err != 0)
	{
		logObj.stop();
		if(err > 0)
			cerr << "### ERROR: for ";
		else
			cerr << "### WARNING: by ";
		cerr << "initial database server" << endl;
		cerr << "             " << database.strerror(err) << endl;
		return err;
	}
	glob::stopMessage("### ending database process with all threads", /*all process names*/true);
	logObj.stop();
	return EXIT_SUCCESS;
}
