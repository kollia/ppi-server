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

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>

#include <list>
#include <algorithm>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../util/debug.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/smart_ptr.h"
#include "../util/URL.h"

#include "../util/stream/ErrorHandling.h"

#include "../server/libs/client/ExternClientInputTemplate.h"
#include "../server/libs/client/SocketClientConnection.h"

#include "../server/libs/server/ServerThread.h"

#include "lib/ClientTransaction.h"

#include "Client.h"

using namespace boost;
using namespace boost::algorithm;
using namespace util;
using namespace server;
using namespace std;



bool Client::execute(const string& workdir, vector<string> options, string command)
{
	unsigned short nPort;
	string fileName, prop;
	string confpath, logpath, sLogLevel, property;
	auto_ptr<SocketClientConnection> clientCon;
	vector<string>::iterator opIt;
	istringstream icommand(command);

	m_sWorkdir= workdir;
	opIt= ::find(options.begin(), options.end(), "-f");
	if(opIt != options.end())
	{
		fileName= opIt->substr(3);
		options.erase(opIt);
	}
	confpath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(confpath, "server.conf");
	m_oServerFileCasher.setDelimiter("owreader", "[", "]");
	m_oServerFileCasher.modifier("owreader");
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

	if(command == "STOP " || command == "stop ")
		command= "stop-server";
	else if(command == "STATUS ")
		command= "status";
	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		return false;

	bool askServer= true;
	ClientTransaction* pClient;
	string co, ask;
	ErrorHandling errHandle;

	command= ConfigPropertyCasher::trim(command);
	pClient= new ClientTransaction(options, command); // insert client into SocketClientConnection, need no auto_ptr
	clientCon= auto_ptr<SocketClientConnection>(new SocketClientConnection(SOCK_STREAM, "127.0.0.1", nPort, 10, pClient));
	icommand >> co;
	if(co == "GETERRORSTRING")
	{
		icommand >> ask;
		errHandle.setErrorStr(ask);
		if(!errHandle.fail())
		{
			/*
			 * toDo: implement error descriptions
			 *       from internet server ('WARNING xxx' / 'ERROR xxx')
			 *       into error Handling
			 */
			if(	ask == "ERROR " ||
				ask == "WARNING"	)
			{
				errHandle.setError("Client", "getClientErrorString");
			}else
				errHandle.setError("Client", "getNoErrorString", ask);
		}
		cout << errHandle.getDescription() << endl;
		errHandle.clear();
		askServer= false;
	}
	if(askServer)
	{
		if(	command == "status" ||
			command == "stop-server"	)
		{
			int s;

			s= ServerThread::connectAsClient("127.0.0.1", nPort, false);
			if(s == 0)
			{
				cerr << "ppi-server does not running" << endl;
				return false;
			}
			close(s);
		}
		errHandle= clientCon->init();
		if(errHandle.fail())
		{
			string output;

			if(!pClient->wasErrorWritten())
			{
				if(	//clres == 25 && toDo: check for old code
					//                     before implement new error handling
					co == "status"	)
				{
					errHandle.setError("Client", "noRun");

				}
				opIt= ::find(options.begin(), options.end(), "-e");
				if(opIt != options.end())
					cout << errHandle.getDescription() << endl;
				else
					cout << errHandle.getErrorStr();
			}
		}
	}
	return true;
}

