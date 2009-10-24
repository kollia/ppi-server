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
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <map>

#include "util/termmacro.h"
#include "util/debug.h"
#include "util/configpropertycasher.h"
#include "util/Thread.h"

#include "portserver/LircClient.h"

//#include "util/Calendar.cpp"

#include "starter.h"

using namespace std;
using namespace util;
using namespace ports;

void usage(void);
void help(char* cpSelf);

int main(int argc, char* argv[])
{
	bool result;
	bool bWait= false;
	int nArcPos= 1;
	string param;
	Starter* server;
	string workdir(argv[0]);
	vector<string> directorys;
	vector<string>::size_type dirlen;
	// access to Lirc device
	//LircClient lirc("lirc", "run");
	//lirc.measure();
	//exit(0);

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

	directorys= ConfigPropertyCasher::split(workdir, "/");
	dirlen= directorys.size();
	workdir= "";

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
	if(argc<2)
	{
		usage();
		return 1;
	}else
		param= argv[1];

	try
	{
		vector<string> vOptions;

		if(param.substr(0, 1) == "-")
		{
			do{
				if(	param == "-w"
					||
					param == "-d"	)
				{
					bWait= true;
				}
				if(	param == "-f"
					||
					param == "-u"	)
				{
					param+= " ";
					++nArcPos;
					param+= argv[nArcPos];
				}
				vOptions.push_back(param);
				++nArcPos;
				if(nArcPos >= argc)
					break;
				param= argv[nArcPos];
			}while(	nArcPos < argc
					&&
					param.substr(0, 1) == "-"	);
		}

		if(	param == "start"
			||
			param == "restart"
			||
			param == "stop"		)
		{
			if(getuid() != 0)
			{
				cerr << "process has to start as root" << endl;
				return EXIT_FAILURE;
			}
		}
		server= new Starter(workdir);
		if(param == "start")
		{
			pthread_mutex_init(&g_READMUTEX, NULL);
			result= server->execute(vOptions);
			cout << "### ppi-server starter process was stopped ";
			if(!result)
			{
				cout << "with an ERROR" << endl;
				return EXIT_FAILURE;
			}
			cout << "successfully" << endl;
			return EXIT_SUCCESS;

		}else if(param == "stop")
		{
			result= server->stop(vOptions);
		}else if(param == "restart")
		{
			server->stop(vOptions);
			cout << "### restart ppi-server" << endl;
			result= server->execute(vOptions);
		}else if(param == "-?")
		{
			help(argv[0]);
			return 0;
		}else
		{
			string command("");

			if(nArcPos < argc)
				param= argv[nArcPos];
			else
				param= "";
			if(	param == "SET"
				||
				param == "GET"
				||
				param == "PROP"
				||
				param == "DEBUG"
				||
				param == "DIR"
				||
				param == "CONTENT"
				||
				param == "status"
				||
				(	bWait
					&&
					param == ""	)	)
			{
				while(nArcPos < argc)
				{
					param= argv[nArcPos];
					command+= param + " ";
					++nArcPos;
				}

				if(	bWait
					||
					command != ""	)
				{
					bool bRes;

					//cout << "command: " << command << endl;
					bRes= server->command(vOptions, command);
					if(bRes)
						return EXIT_SUCCESS;
					return EXIT_FAILURE;
				}
			}

			usage();
			//log->stop();
			return 1;
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
		cout << endl << "server was stopped" << endl;
		return EXIT_SUCCESS;

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

void usage()
{
	printf("no correct command be set\n");
	printf("type -? for help\n");
}

void help(char* cpSelf)
{
	printf("\n");
	printf("syntax:  %s [options] [command]\n", cpSelf);
	printf("\n");
	printf("       options:\n");
	printf("            -?    - show this help\n");
	printf("            -u    - set user for client,\n");
	printf("                    if no user be set, client ask for user when command isn't start/stop/restart\n");
	printf("                    password will be always asked and cannot insert in command\n");
	printf("            -e    - show only errornumbers\n");
	printf("            -w    - hold transaction to server\n");
	printf("                    after them user only must type commands\n");
	printf("                    ending with 'quit' or 'exit'\n");
	printf("            -d    - same as befor (-w) but also client starts\n");
	printf("                    an second connection to server,\n");
	printf("                    where the client get changes which are set\n");
	printf("                    with the command 'HEAR'\n");
	printf("            -t    - showes by status info all threads with running information,\n");
	printf("                    otherwise the command status tell only how much threads are running\n");
	printf("            -c    - like -t but also showes all communication-threads with wich client-ID they are connected,\n");
	printf("                    or they hanging on no client\n");
	printf("\n");
	printf("       command:\n");
	printf("                start    -     starting server\n");
	printf("                stop     -     stopping server\n");
	printf("                restart  -     restarting the hole server\n");
	printf("                status   -     show how much threads for process are running.\n");
	printf("                               see also for options -t or -c\n");
	printf("                CHANGE <username>:<password>\n");
	printf("                         -     changing user, username and password is seperated with an colon\n");
	printf("                PERMISSION <groupnames>\n");
	printf("                         -     ask permission for group.\n");
	printf("                               also more than one groups can be ask, seperated with an colon\n");
	printf("                GET <folder>:<subroutine>\n");
	printf("                         -     get the current value from the subroutines in the folder\n");
	printf("                               folder and subroutine are seperated with an colon\n");
	printf("                SET <folder>:<subroutine> <value>\n");
	printf("                         -     set the given value from given subroutine in given folder\n");
	printf("                HEAR <folder>:<subroutine>\n");
	printf("                         -     if the client has set an second connection with -d,\n");
	printf("                               client can order with this command to hear on the given folder:subroutine's\n");
	printf("                               for changes\n");
	printf("                NEWENTRYS\n");
	printf("                         -     clearing all entrys which are set with the command HEAR\n");
	printf("                DIR <filter>\n");
	printf("                         -     shows all files in directory ${workdir}/client which are suitable to given filter\n");
	printf("                CONTENT <filename>\n");
	printf("                         -     send the file content of the given filename under ${workdir}/client\n");
	printf("                DEBUG <folder> <sleeptime>\n");
	printf("                         -     show by running server debugging messages for given folder\n");
	printf("                               on end of loop measurethread sleeping sleeptime\n");
	printf("                               if folder is -ow client get DEBUG info for bechmark OWServer\n");
	printf("                               and sleeptime should be number of OWServer.\n");
	printf("                               if folder set as string 'null' debugging is ending\n");
	printf("\n");
}
