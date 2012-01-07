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
#include "util/debug.h"
#include "util/thread/Terminal.h"

#include "starter.h"

using namespace std;
using namespace util;

// only used method for some debugging tests
void tests(const string& workdir, int argc, char* argv[]);

int main(int argc, char* argv[])
{
	bool result;
	string command;
	Starter server;
	MainParams params(argc, argv, /*read path for parent dirs*/1);
	const ICommandStructPattern* commands;

	params.setDescription("start or stop ppi-server to reading external sensors and devices");

	params.option("configure", "c", "display which folder configure by starting\n"
							"(can be set by longer starting time to know what server is doing)");
	params.option("firstvalue", "f", "show after configure folder all first values of defined ports from owreader");
	params.option("folderstart", "F", "show all folder on command line which are starting");
	params.option("debug", "d", "show logging messages, deep defined inside server.conf, on screen\n"
									"(only useable for stop command)");
	params.version(PPI_MAJOR_RELEASE, PPI_MINOR_RELEASE, PPI_SUBVERSION, PPI_PATCH_LEVEL,
										/*no build*/0, PPI_REVISION_NUMBER, DISTRIBUTION_RELEASE);
	params.help("help", "?");

	params.command("start", "starting server");
	params.command("stop", "stopping server");
	params.command("restart", "restarting the hole server");
	params.command("status", "get feedback whether server is running");

	params.execute();
	commands= params.getCommands();
	command= commands->command();
	server.setWorkingDirectory(params.getPath());

	// method has only content by debugging when need
	tests(params.getPath(), argc, argv);

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

		if(command == "start")
		{
			if(params.hasOption("debug"))
			{
				cerr << "debug option only allowed by stopping command" << endl;
				cerr << "   for more description read help manual (ppi-server --help)" << endl << endl;
				return EXIT_FAILURE;
			}
			pthread_mutex_init(&g_READMUTEX, NULL);
			result= server.execute(&params);
			cout << "### ppi-server was stopped ";
			if(!result)
			{
				cout << "with an ERROR" << endl;
				Terminal::deleteObj();
				return EXIT_FAILURE;
			}
			cout << "successfully" << endl;
			Terminal::deleteObj();
			return EXIT_SUCCESS;

		}else if(command == "status")
		{
			result= server.status();

		}else if(command == "stop")
		{
			vector<string>::size_type nCount;

			nCount= params.optioncount();
			if(	nCount == 0 ||
				(	nCount == 1 &&
					params.hasOption("debug")	)	)
			{
				result= server.stop(params.hasOption("debug"));
			}else
			{
				cout << "by stopping ppi-server only debug option be allowed" << endl;
				cout << "   for more description read help manual (ppi-server --help)" << endl << endl;
				return EXIT_FAILURE;
			}

		}else if(command == "restart")
		{
			server.stop(params.hasOption("configue"));
			cout << "### restart ppi-server" << endl;
			result= server.execute(commands);
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



#if 0
// some includes needed for tests method
#include "util/URL.h"
// only for simple server client communication
#include "server/libs/client/SocketClientConnection.h"
#include "server/libs/server/TcpServerConnection.h"
#include "util/smart_ptr.h"
// for testing on streams
#include "util/properties/interlacedproperties.h"
#endif

void tests(const string& workdir, int argc, char* argv[])
{
#if 0
	// define configure path
	string sConfPath;

	sConfPath= URL::addPath(workdir, PPICONFIGPATH, /*always*/false);
#endif

#if 0
	// check working of InterlacedProperties
	typedef vector<IInterlacedPropertyPattern*>::iterator secIt;
	string fileName;
	InterlacedActionProperties mainprop(/*check after*/true);
	vector<IInterlacedPropertyPattern*> folderSections;
	vector<IInterlacedPropertyPattern*> subSections;

	fileName= URL::addPath(sConfPath, "measure.conf");
	mainprop.action("action");
	mainprop.modifier("folder");
	mainprop.setMsgParameter("folder");
	mainprop.modifier("name");
	mainprop.setMsgParameter("name", "subroutine");
	mainprop.valueLocalization("\"", "\"", /*remove*/true);
	mainprop.readFile(fileName);
	folderSections= mainprop.getSections();
	for(secIt fit= folderSections.begin(); fit != folderSections.end(); ++fit)
	{
		cout << (*fit)->getMsgHead(false) << endl;
		subSections= (*fit)->getSections();
		for(secIt sit= subSections.begin(); sit != subSections.end(); ++sit)
		{
			cout << (*sit)->getMsgHead(false) << endl;
		}
	}
	exit(EXIT_SUCCESS);
#endif

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

}
