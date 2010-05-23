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
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include <algorithm>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <list>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../util/debug.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/smart_ptr.h"
#include "../util/URL.h"
#include "../util/configpropertycasher.h"
#include "../util/ExternClientInputTemplate.h"

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
	bool bRv;
	int clres;
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
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

	if(command == "STOP " || command == "stop ")
		command= "stop-server ";
	else if(command == "STATUS ")
		command= "status";
	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		return false;

	bool askServer= true;
	unsigned int err, warn, ask;
	ClientTransaction* pClient;
	string co;

	bRv= true;
	command= ConfigPropertyCasher::trim(command);
	pClient= new ClientTransaction(options, command); // insert client into SocketClientConnection, need no auto_ptr
	clientCon= auto_ptr<SocketClientConnection>(new SocketClientConnection(SOCK_STREAM, "127.0.0.1", nPort, 10, pClient));
	err= clientCon->getMaxErrorNums(true);
	warn= clientCon->getMaxErrorNums(false);
	pClient->setErrors(warn, err);
	icommand >> co;
	if(co == "GETERRORSTRING")
	{
		icommand >> ask;
		if(	ask >= (warn*-1)
			&&
			ask <= err		)
		{
			cout << clientCon->strerror(ask) << endl;
			askServer= false;
		}
	}
	if(askServer)
	{
		if(co == "status")
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
		clres= clientCon->init();
		if(clres != 0)
		{
			string output;

			if(	clres == 25 &&
				co == "status"	)
			{
				cerr << "ppi-server does not running" << endl;
				bRv= false;
			}else
			{
				opIt= ::find(options.begin(), options.end(), "-e");
				if(opIt != options.end())
					output= ExternClientInputTemplate::error(clres);
				else
					output= clientCon->strerror(clres);

				if(clres > 0)
				{
					cerr << output << endl;
					bRv= false;
				}else
					cout << output << endl;
			}
		}
	}
	return bRv;
}

