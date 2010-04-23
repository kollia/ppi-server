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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "LogProcess.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/properties.h"
#include "../util/URL.h"

#include "../server/libs/client/SocketClientConnection.h"

using namespace std;
using namespace boost::algorithm;
using namespace util;

int main(int argc, char* argv[])
{
	uid_t loguserID;
	string loguser;
	string workdir;
	string commhost;
	string property;
	string sConfPath, fileName;
	unsigned short commport;
	int err;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	Properties oServerProperties;

	glob::processName("ppi-log-client");
	glob::setSignals("ppi-log-client");
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
	loguser= oServerProperties.getValue("loguser", /*warning*/false);
	if(loguser == "")
		loguser= oServerProperties.getValue("loguser", /*warning*/false);
	if(loguser == "")
	{
		cerr << "### WARNING: loguser or defaultuser are not defined" << endl;
		cerr << "             set loguser to 'nobody'" << endl;
		loguser= "nobody";
	}
	loguserID= URL::getUserID(loguser);
	commhost= oServerProperties.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= oServerProperties.needUShort(property);

	LogProcess logger(	loguserID,
						new SocketClientConnection(	SOCK_STREAM,
													commhost,
													commport,
													10			),
						new SocketClientConnection(	SOCK_STREAM,
													commhost,
													commport,
													10			)	);

	err= logger.run(&oServerProperties);
	if(err != 0)
	{
		if(err > 0)
			cerr << "### ERROR: for ";
		else
			cerr << "### WARNING: by ";
		cerr << "initial process LogServer" << endl;
		cerr << "             " << logger.strerror(err) << endl;
		return err;
	}
	cout << "### ending logging process with all threads" << endl;

	return err;
}
