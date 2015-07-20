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

//#include <string.h>
//#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>


#include <boost/algorithm/string/split.hpp>


#include "../pattern/util/LogHolderPattern.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/URL.h"
#include "../util/usermanagement.h"

#include "../util/thread/ThreadErrorHandling.h"
#include "../util/properties/interlacedproperties.h"

#include "../database/lib/NeedDbChanges.h"

#include "libs/client/SocketClientConnection.h"
#include "libs/server/TcpServerConnection.h"
#include "libs/server/communicationthreadstarter.h"
#include "libs/server/ServerProcess.h"

#include "ServerTransaction.h"

using namespace std;
using namespace boost;
using namespace util;
using namespace server;
using namespace user;
using namespace logger;
using namespace ppi_database;

string global_clientpath;

int main(int argc, char* argv[])
{
	int nRv;
	uid_t defaultuserID;
	string defaultuser;
	string workdir;
	string host, commhost;
	string property;
	string sConfPath, fileName;
	unsigned short port, commport, minThreads, maxThreads;
	int nLogAllSec;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	InterlacedProperties oServerProperties;
	map<string, uid_t> users;
	ErrorHandling errHandle;
	thread::ThreadErrorHandling thErrHandle;
	SocketErrorHandling sockErrHandle;

	glob::processName("ppi-internet-server");
	glob::setSignals("ppi-internet-server");

	errHandle.read();
	thErrHandle.read();
	sockErrHandle.read();

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
	fileName= URL::addPath(sConfPath, "server.conf");
	oServerProperties.setDelimiter("owreader", "[", "]");
	oServerProperties.modifier("owreader");
	oServerProperties.readLine("workdir= " + workdir);
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: internet server cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}

	// start logging server
	defaultuser= oServerProperties.getValue("defaultuser", /*warning*/false);
	if(defaultuser == "")
	{
		cerr << "### WARNING: defaultuser is not defined" << endl;
		cerr << "             so process run under 'nobody'" << endl;
		defaultuser= "nobody";
	}

	users[defaultuser]= 0;
	if(!glob::readPasswd(oServerProperties.getValue("passwd"), users))
	{
		string msg;

		defaultuserID= 0;
		msg=  "### WARNING: do not found default user " + defaultuser + " inside passwd\n";
		msg+= "             so internet server running as root";
		LOG(LOG_ALERT, msg);
		cerr << msg << endl;
	}else
		defaultuserID= users[defaultuser];
	commhost= oServerProperties.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= oServerProperties.needUShort(property);
	host= oServerProperties.getValue("listen", /*warning*/false);
	property= "port";
	port= oServerProperties.needUShort(property);
	// initial Log Interface
	property= "timelogSec";
	nLogAllSec= oServerProperties.getInt(property, /*warning*/false);// warning be written in starter.cpp
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}

	global_clientpath= URL::addPath(workdir, PPICLIENTPATH, /*always*/false);

	// initial main connection to database to send questions and logs
	DbInterface::initial("ppi-internet-server", new SocketClientConnection(	SOCK_STREAM,
																			commhost,
																			commport,
																			5				),
												/*identif log wait*/nLogAllSec					);


	if(!UserManagement::initial(URL::addPath(sConfPath, "access.conf", /*always*/true),
								URL::addPath(sConfPath, "measure.conf", /*always*/true)))
	{
		cerr << "### ERROR: cannot read correctly 'access.conf'" << endl;
		return false;
	}

	property= "minconnectionthreads";
	minThreads= oServerProperties.getUShort(property, /*warning*/true);
	property= "maxconnectionthreads";
	maxThreads= oServerProperties.getUShort(property, /*warning*/true);
	if(	property == "#ERROR"
		||
		maxThreads < 4		)
	{
		maxThreads= 4;
	}

	// initial get connection to database server for get questions
	if(!NeedDbChanges::initial("ppi-internet-server", new SocketClientConnection(	SOCK_STREAM,
																					commhost,
																					commport,
																					5				)	)	)
	{
		string msg("### WARNING: cannot start second connection to database,\n");

		msg+= "             so client connection with HEAEING commands cannot be answered";
		cerr << msg << endl;
		LOG(LOG_WARNING, msg);
	}

	ServerProcess internetserver(	"ppi-internet-server", defaultuserID,
									new CommunicationThreadStarter(minThreads, maxThreads),
									new TcpServerConnection(	host,
																port,
																5,
																new ServerTransaction(defaultuserID)	),
									new SocketClientConnection(	SOCK_STREAM,
																"127.0.0.1",
																port,
																5			),
									/*open connection with*/"GET"									);
	errHandle= internetserver.run();
	nRv= EXIT_SUCCESS;
	if(errHandle.fail())
	{
		string msg;

		errHandle.addMessage("internetservermain", "run");
		msg= errHandle.getDescription();
		if(errHandle.hasError())
		{
			cerr << glob::addPrefix("### ERROR: ", msg) << endl;
			nRv= EXIT_FAILURE;
		}else
			cout << glob::addPrefix("### WARNING: ", msg) << endl;
	}
	glob::stopMessage("### process of ppi-internet-server was ending correctly", /*all process names*/true);
	return nRv;
}
