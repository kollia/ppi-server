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

#include "../logger/lib/LogInterface.h"

#include "../util/properties.h"
#include "../util/URL.h"
#include "../util/usermanagement.h"

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
	uid_t defaultuserID;
	string defaultuser;
	string workdir;
	string host, commhost;
	string property;
	string sConfPath, fileName;
	unsigned short port, commport, minThreads, maxThreads;
	int err;
	int nLogAllSec;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	Properties oServerProperties;

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
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	oServerProperties.readLine("workdir= " + workdir);

	// start logging server
	defaultuser= oServerProperties.getValue("defaultuser", /*warning*/false);
	if(defaultuser == "")
	{
		cerr << "### WARNING: defaultuser is not defined" << endl;
		cerr << "             so process run under 'nobody'" << endl;
		defaultuser= "nobody";
	}

	defaultuserID= URL::getUserID(defaultuser);
	commhost= oServerProperties.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= oServerProperties.needUShort(property);
	host= oServerProperties.getValue("listen", /*warning*/false);
	if(host == "")
		host= "127.0.0.1";
	property= "port";
	port= oServerProperties.needUShort(property);

	global_clientpath= URL::addPath(workdir, PPICLIENTPATH, /*always*/true);

	// initial Log Interface
	property= "timelogSec";
	nLogAllSec= oServerProperties.getInt(property, /*warning*/false);// warning be written in starter.cpp
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	LogInterface::initial(	"ppi-internet-server",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														0								),
							/*identif log wait*/nLogAllSec,
							/*wait*/true													);


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

	// initial main connection to database to send questions
	DbInterface::initial("ppi-internet-server", new SocketClientConnection(	SOCK_STREAM,
																			commhost,
																			commport,
																			5				)	);

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
	if(!UserManagement::initial(URL::addPath(sConfPath, "access.conf", /*always*/true),
								URL::addPath(sConfPath, "measure.conf", /*always*/true)))
	{
		cerr << "### ERROR: cannot read correctly 'access.conf'" << endl;
		return false;
	}

	ServerProcess internetserver(	"ppi-internet-server", defaultuserID,
									new CommunicationThreadStarter(minThreads, maxThreads),
									new TcpServerConnection(	host,
																port,
																5,
																new ServerTransaction()	),
									new SocketClientConnection(	SOCK_STREAM,
																host,
																port,
																5			),
									/*open connection with*/"GET"									);

	err= internetserver.run(); //&oServerProperties);
	if(err != 0)
	{
		if(err > 0)
			cerr << "### ERROR: for ";
		else
			cerr << "### WARNING: by ";
		cerr << "initial process internet server" << endl;
		cerr << "             " << internetserver.strerror(err) << endl;
		return err;
	}

	return 0;
}
