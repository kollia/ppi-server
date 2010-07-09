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

#include "../util/GlobalStaticMethods.h"
#include "../util/properties.h"
#include "../util/URL.h"

#include "../server/libs/client/SocketClientConnection.h"
#include "../server/libs/server/TcpServerConnection.h"
#include "../server/libs/server/ServerProcess.h"
#include "../server/libs/server/communicationthreadstarter.h"

#include "ServerDbTransaction.h"
#include "DatabaseThread.h"

using namespace std;
using namespace boost;
using namespace util;
using namespace ppi_database;
using namespace logger;

int main(int argc, char* argv[])
{
	uid_t defaultuserID;
	int nLogAllSec;
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
	Properties oServerProperties;
	DatabaseThread* db;
	CommunicationThreadStarter* starter;

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
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	oServerProperties.readLine("workdir= " + workdir);

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

	// initial interface to log client

	property= "timelogSec";
	nLogAllSec= oServerProperties.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	LogInterface::initial(	"ppi-db-server",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														0			),
							/*identif log*/nLogAllSec,
							/*wait*/true								);



	starter= new CommunicationThreadStarter(0, nDbConnectors);
	// start initialitation from database
	DatabaseThread::initial(dbpath, sConfPath, &oServerProperties);
	db= DatabaseThread::instance();
	db->setCommunicator(starter);

	ServerProcess database(	"ppi-db-server", defaultuserID, starter,
							new TcpServerConnection(	commhost,
														commport,
														10,
														new ServerDbTransaction()	),
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														10			)						);

	err= database.run(&oServerProperties);
	if(err != 0)
	{
		if(err > 0)
			cerr << "### ERROR: for ";
		else
			cerr << "### WARNING: by ";
		cerr << "initial database server" << endl;
		cerr << "             " << database.strerror(err) << endl;
		return err;
	}
	glob::stopMessage("### ending database process with all threads", /*all process names*/true);
	return EXIT_SUCCESS;
}
