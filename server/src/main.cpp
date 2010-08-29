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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <map>

#include "util/termmacro.h"
#include "util/debug.h"

#include "portserver/LircClient.h"

#if 0
// only for simple server client communication
#include "server/libs/client/SocketClientConnection.h"
#include "server/libs/server/TcpServerConnection.h"
#include "util/smart_ptr.h"
// for testing on streams
#include "util/IParameterStringStream.h"
#endif

#include "starter.h"

using namespace std;
using namespace util;
using namespace ports;

void usage(char* cpSelf);

int main(int argc, char* argv[])
{
// simple server client communication
#if 0
	string command;
	string value;
    SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

    if (argc == 2)
    {
    	command= argv[1];
    	if(command == "client")
    	{
    		server::SocketClientConnection client(	SOCK_STREAM,
													"127.0.0.1",
													20000,
													10			);

    		//connection.newTranfer(new ClientTransaction());
    		if(!client.init())
    		{
    			descriptor= client.getDescriptor();
    			cout << "client send: \"Hallo Du!\"" << endl;
    			(*descriptor) << "Hallo Du!\nWie geht es Dir\nden";
    			(*descriptor) << " heute?";
    			descriptor->endl();
    			descriptor->flush();
    			(*descriptor) >> value;
    			cout << value << endl;
    			return EXIT_SUCCESS;
    		}
    		return EXIT_FAILURE;

    	}else if(command == "server")
    	{
    		server::TcpServerConnection connection(	"127.0.0.1",
													20000,
													10,
													NULL	);

    		if(!connection.init())
    		{
    			if(!connection.accept())
    			{
    				descriptor= connection.getDescriptor();
    				while(!descriptor->eof())
    				{
    					(*descriptor) >> value;
    					cout << "Server get message " << value << endl;
    				}
    				(*descriptor) << "Server got message";
    				descriptor->endl();
    				descriptor->unlock();
    				descriptor->closeConnection();
    				return EXIT_SUCCESS;
    			}
    		}
    		return EXIT_FAILURE;
    	}
    }

    fprintf(stderr,"usage %s [server|client]\n", argv[0]);
    return EXIT_FAILURE;
#endif
#if 0
	IParameterStringStream reader("truego");
	bool val;

	reader >> val;
	cout << "value is " << val << endl;
	if(reader.fail())
		cout << "an error is occured" << endl;
	if(reader.empty())
		cout << "reading of string is finished" << endl;
#endif
#if 0
	bool newer= false;
	time_t t;
	time(&t);
	Calendar::calcDate(newer, t, 1, 'h');
	Calendar::calcDate(newer, t, 3, 'h');
	Calendar::calcDate(newer, t, 1, 'D');
	Calendar::calcDate(newer, t, 5, 'D');
	Calendar::calcDate(newer, t, 1, 'W');
	Calendar::calcDate(newer, t, 4, 'W');
	Calendar::calcDate(newer, t, 1, 'M');
	Calendar::calcDate(newer, t, 4, 'M');
	Calendar::calcDate(newer, t, 1, 'Y');
	Calendar::calcDate(newer, t, 2, 'Y');
#endif

	bool result;
	bool bWait= false;
	int nArcPos= 1;
	string param;
	auto_ptr<Starter> server;
	string workdir(argv[0]);
	vector<string> directorys;
	vector<string>::size_type dirlen;
	// access to Lirc device
	//LircClient lirc("lirc", "run");
	//lirc.measure();
	//exit(0);

	directorys= ConfigPropertyCasher::split(workdir, "/");
	dirlen= directorys.size();
	workdir= "";

#if 0
	// auto_ptr test
	SHAREDPTR::shared_ptr<vector<string> > plv, plv2;
	while(1)
	{
		plv2= std::auto_ptr<vector<string> >(new vector<string>());
		plv= std::auto_ptr<vector<string> >(new vector<string>());
		plv->push_back("0-string");
		plv->push_back("1-string");
		plv->push_back("2-string");
		plv->push_back("3-string");
		plv->push_back("4-string");
		plv->push_back("5-string");
		plv->push_back("6-string");
		plv->push_back("7-string");
		plv->push_back("8-string");
		plv->push_back("9-string");
		plv2= plv;
		plv= SHAREDPTR::shared_ptr<vector<string> >();
		if(!plv)
			cout << "shared_ptr plv is NULL" << endl;
		if(plv2)
			cout << "shared_ptr plv2 is not NULL" << endl;
		for(vector<string>::iterator vit= plv2->begin(); vit != plv2->end(); ++vit)
			cout << *vit << endl;
		cout << endl;
	}
#endif
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
	if(argc != 2)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}else
		param= argv[1];

	try
	{
		if(	param == "start" ||
			param == "stop" ||
			param == "restart"	)
		{
			if(getuid() != 0)
			{
				cerr << "process has to start as root" << endl;
				return EXIT_FAILURE;
			}
		}

		server= auto_ptr<Starter>(new Starter(workdir));
		if(param == "start")
		{
			pthread_mutex_init(&g_READMUTEX, NULL);
			result= server->execute();
			cout << "### ppi-server was stopped ";
			if(!result)
			{
				cout << "with an ERROR" << endl;
				return EXIT_FAILURE;
			}
			cout << "successfully" << endl;
			return EXIT_SUCCESS;

		}else if(param == "status")
		{
			result= server->status();

		}else if(param == "stop")
		{
			result= server->stop();

		}else if(param == "restart")
		{
			server->stop();
			cout << "### restart ppi-server" << endl;
			result= server->execute();
		}else
		{
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		//delete server;
	}catch(...)
	{
		cout << "unknown exception" << endl;
		return EXIT_FAILURE;
	}


	if(param == "start")
		cout << "LogServer" << endl;
	if(param == "stop")
	{
		if(result)
		{
			cout << "server was stopped" << endl;
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;

	}else if(param == "start")
	{
		if(result)
		{
			cout << endl << "#### server was stopping correctly" << endl;
			//system("ps -eLf | grep ppi-server");
			return EXIT_SUCCESS;
		}else
		{
			cerr << endl << "#### server stopping with ERROR" << endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_FAILURE;
}

void usage(char* cpSelf)
{
	printf("no correct command be set\n");
	printf("syntax:  %s <command>\n", cpSelf);
	printf("\n");
	printf("       command:\n");
	printf("                start    -     starting server\n");
	printf("                stop     -     stopping server\n");
	printf("                restart  -     restarting the hole server\n");
	printf("                status   -     get feedback whether server is running\n");
	printf("\n");
}
