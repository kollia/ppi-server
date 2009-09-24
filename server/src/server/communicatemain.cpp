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

#include "libs/client/SocketClientConnection.h"
#include "libs/server/TcpServerConnection.h"
#include "libs/server/communicationthreadstarter.h"
#include "libs/server/ServerProcess.h"

#include "libs/server/ServerMethodTransaction.h"

using namespace std;
using namespace boost;
using namespace util;
using namespace server;
using namespace logger;

int main(int argc, char* argv[])
{
	uid_t defaultuserID;
	string defaultuser;
	string workdir;
	string commhost;
	string property;
	string sConfPath, fileName;
	unsigned short commport;
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

	// initial Log Interface
	property= "timelogSec";
	nLogAllSec= oServerProperties.getInt(property, /*warning*/false);// warning be written in starter.cpp
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	LogInterface::initial(	"ppi-communicate-server",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														0								),
							/*identif log wait*/nLogAllSec,
							/*wait*/true													);


	ServerProcess communicate(	"CommunicationServerProcess", defaultuserID,
								new CommunicationThreadStarter(0, 4),
								new TcpServerConnection(	commhost,
															commport,
															10,
															new ServerMethodTransaction()	),
								new SocketClientConnection(	SOCK_STREAM,
															commhost,
															commport,
															10			)						);

	err= communicate.run(&oServerProperties);
	if(err != 0)
	{
		if(err > 0)
			cerr << "### ERROR: for ";
		else
			cerr << "### WARNING: by ";
		cerr << "initial process communicate server" << endl;
		cerr << "             " << communicate.strerror(err) << endl;
		return err;
	}

	return 0;
}
