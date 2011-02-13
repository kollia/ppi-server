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

#include "util/MainParams.h"
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
	string command;
	auto_ptr<Starter> server;
	string workdir;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	const ICommandStructPattern* commands;

	params.setDescription("start or stop ppi-server to reading external sensors and devices");

	params.option("configure", "c", "display which folder configure by starting\n"
							"(can be set by longer starting time to know what server is doing)");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);

	params.command("start", "starting server");
	params.command("stop", "stopping server");
	params.command("restart", "restarting the hole server");
	params.command("status", "get feedback whether server is running");

	params.execute();
	workdir= params.getPath();
	commands= params.getCommands();
	command= commands->command();

	try
	{
		if(	command == "start" ||
			command == "stop" ||
			command == "restart"	)
		{
			if(getuid() != 0)
			{
				cerr << "process has to start as root" << endl;
				return EXIT_FAILURE;
			}
		}

		server= auto_ptr<Starter>(new Starter(workdir));
		if(command == "start")
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

		}else if(command == "status")
		{
			result= server->status();

		}else if(command == "stop")
		{
			result= server->stop();

		}else if(command == "restart")
		{
			server->stop();
			cout << "### restart ppi-server" << endl;
			result= server->execute();
		}else
		{
			cout << "no correct command be set" << endl;
			cout << "  type -? for help" << endl;
			return EXIT_FAILURE;
		}
		//delete server;
	}catch(...)
	{
		cout << "unknown exception" << endl;
		return EXIT_FAILURE;
	}


	if(command == "start")
		cout << "LogServer" << endl;
	if(command == "stop")
	{
		if(result)
		{
			cout << "server was stopped" << endl;
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;

	}else if(command == "start")
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
