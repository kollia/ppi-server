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
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <map>

#include "../util/debug.h"
#include "../util/exception.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/stream/ErrorHandling.h"
#include "../util/thread/ThreadErrorHandling.h"
#include "../server/libs/SocketErrorHandling.h"

#include "Client.h"
#include "lib/ClientTransaction.h"

using namespace std;
using namespace util;
using namespace server;

void usage(void);
void help(char* cpSelf);

int main(int argc, char* argv[])
{
	bool bWait= false;
	int nArcPos= 1;
	string param;
	string command("");
	Client client;
	string workdir(argv[0]);
	vector<string> directorys;
	vector<string>::size_type dirlen;
	ErrorHandling errHandle;
	thread::ThreadErrorHandling thErrHandle;
	SocketErrorHandling sockErrHandle;

	glob::processName("ppi-client");
	errHandle.read();
	thErrHandle.read();
	sockErrHandle.read();

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
				if(param == "--help")
					param= "-?";
				else if(param == "--user")
					param= "-u";
				else if(param == "--errornums")
					param= "-e";
				else if(param == "--wait")
					param= "-w";
				else if(param == "--hear")
					param= "-h";
				else if(param == "--thread")
					param= "-t";
				else if(param == "--client")
					param= "-c";
				else if(param == "--pid")
					param= "-p";
				if(	param == "-w" ||
					param == "-h"	)
				{
					bWait= true;
				}
				if(	param == "-f" ||
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

		if(nArcPos < argc)
			param= argv[nArcPos];
		else if(param == "-?")
		{
			help(argv[0]);
			return EXIT_SUCCESS;
		}else
			param= "";

		if(	param == "STOP" ||
			param == "stop"||
			param == "STATUS" ||
			param == "status" ||
			param == "PERMISSION" ||
			param == "GET" ||
			param == "SET" ||
			param == "CONTENT" ||
			param == "DIR" ||
			param == "SHOW" ||
			param == "STOPDEBUG" ||
			param == "GETMINMAXERRORNUMS" ||
			param == "GETERRORSTRING" ||
			(	bWait
				&&
				(	param == ""	||
					param == "CHANGE" ||
					param == "HEAR"||
					param == "NEWENTRYS"	)	)	)
		{
			bool bRes;

			while(nArcPos < argc)
			{
				param= argv[nArcPos];
				command+= param + " ";
				++nArcPos;
			}

			try{
				//cout << "command: " << command << endl;
				bRes= client.execute(workdir, vOptions, command);
			}catch(SignalException& ex)
			{
				ex.printTrace();
				return EXIT_FAILURE;

			}catch(std::exception& ex)
			{
				cout << string(ex.what()) << endl;
				return EXIT_FAILURE;
			}
			if(bRes > 0)
				return EXIT_FAILURE;

		}else if(param == "CHANGE")
		{
			cerr << "this command is only when you start ppi-server with option 'wait' or 'hear' (./ppi-server -w)" << endl;
			cerr << "type -? for helping" << endl;
			return EXIT_FAILURE;

		}else if(	param == "HEAR"||
					param == "NEWENTRYS"	)
		{
			cerr << "this command is only when you start ppi-server with option 'hear' (./ppi-server -h)" << endl;
			cerr << "type -? for helping" << endl;
			return EXIT_FAILURE;

		}else
		{
			usage();
			return EXIT_FAILURE;
		}

	}catch(...)
	{
		cout << "unknown exception" << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void usage()
{
	std::cout << "no correct command be set\n";
	std::cout << "type -? for help\n";
}

void help(char* cpSelf)
{
	std::cout << "" << endl;
	std::cout << "syntax:  " << *cpSelf << " [options] [command]\n";
	std::cout << endl;
	std::cout << "       options:" << endl;
	std::cout << "            -?  --help         - show this help." << endl;
	std::cout << "            -u  --user         - set user for client," << endl;
	std::cout << "                                 if no user be set, client ask for user when command isn't start/stop/restart" << endl;
	std::cout << "                                 password will be always asked and cannot insert in command" << endl;
	std::cout << "            -e  --errornums    - show only error numbers by get an error or warning from server," << endl;
	std::cout << "                                 otherwise the client display an error or warning description" << endl;
	std::cout << "            -w  --wait         - hold transaction to server, where the user can write more than one command like an command line" << endl;
	std::cout << "                                 after them user only must type commands" << endl;
	std::cout << "                                 ending with 'quit' or 'exit'" << endl;
	std::cout << "            -h  --hear         - same as before (--wait) but also client starts" << endl;
	std::cout << "                                 an second connection to server," << endl;
	std::cout << "                                 where the client get changes which are set before" << endl;
	std::cout << "                                 with the command 'HEAR'" << endl;
//	std::cout << "            -t  --thread       - display by status info all threads with running information," << endl;
//	std::cout << "                                 otherwise the command status tell only how much threads on an process running" << endl;
//	std::cout << "            -c  --client       - like --thread but also display all communication-threads with which client-ID they are connected," << endl;
//	std::cout << "                                 or it displays for no client" << endl;
//	std::cout << "            -p  --pid          - display by status info all process id's separated in own rows" << endl;
	std::cout << endl;
	std::cout << "       command:" << endl;
	std::cout << "                STOP     -     stopping server" << endl;
//	std::cout << "                STATUS   -     show how much threads for process are running." << endl;
//	std::cout << "                               see also for options -t or -c" << endl;
	std::cout << "                CHANGE [username]" << endl;
	std::cout << "                         -     changing user, user name and password" << endl;
	std::cout << "                               this command is only when ppi-server is started with option --wait or --hear" << endl;
	std::cout << "                               (better description will be inside the editor started with describt options)" << endl;
	std::cout << "                PERMISSION <groupnames>" << endl;
	std::cout << "                         -     ask permission for group." << endl;
	std::cout << "                               also more than one groups can be ask, separated with an colon" << endl;
	std::cout << "                               RESULT:  write - user has permission to read and write" << endl;
	std::cout << "                                        read  - user has only permission to read" << endl;
	std::cout << "                                        noSubroutine  - given subroutine do not exist inside folder" << endl;
	std::cout << "                                        noFolder      - given Folder do not exist" << endl;
	std::cout << "                                        ERROR 003     - fault insert of folder:subroutine" << endl;
	std::cout << "                GET <folder>:<subroutine>" << endl;
	std::cout << "                         -     get the current value from the subroutines in the folder" << endl;
	std::cout << "                               folder and subroutine are separated with an colon" << endl;
	std::cout << "                SET <folder>:<subroutine> <value>" << endl;
	std::cout << "                         -     set the given value from given subroutine in given folder" << endl;
	std::cout << "                HEAR <folder>:<subroutine>" << endl;
	std::cout << "                         -     if the client has set an second connection with option --hear," << endl;
	std::cout << "                               client can order with this command to hear on the given folder:subroutine's" << endl;
	std::cout << "                               for changes" << endl;
	std::cout << "                NEWENTRYS" << endl;
	std::cout << "                         -     clearing all entry's which are set with the command HEAR" << endl;
	std::cout << "                               this command is only when ppi-server is started with option --hear" << endl;
	std::cout << "                DIR <filter>" << endl;
	std::cout << "                         -     shows all files in directory ${workdir}/client which are suitable to given filter" << endl;
	std::cout << "                CONTENT <filename>" << endl;
	std::cout << "                         -     send the file content of the given filename under ${workdir}/client" << endl;
	std::cout << "                SHOW [-c] <seconds>" << endl;
	std::cout << "                         -     show on command line which folder threads how often running" << endl;
	std::cout << "                               measure the count inside given seconds" << endl;
	std::cout << "                               when option -c be set, wait until given SET command from any client" << endl;
	std::cout << "                DEBUG [-i|-ow] <folder[:subroutine]/owreaderID>" << endl;
	std::cout << "                         -     show by running server debugging messages for given folder and subroutine" << endl;
	std::cout << "                               when no subroutine given, the hole folder is set for debugging" << endl;
	std::cout << "                               by add option -i, when for folder defined an inform parameter" << endl;
	std::cout << "                               show this calculation also by debug output." << endl;
	std::cout << "                               if option -ow be set client get DEBUG info for benchmark owreader" << endl;
	std::cout << "                               and folder have to be the OWServer ID (owreaderID)." << endl;
	std::cout << "                               (the OWServer ID you can see by starting ppi-server on command line after '### starting OWServer)" << endl;
	std::cout << "                STOPDEBUG [-ow] <folder[:subroutine]/owreaderID>" << endl;
	std::cout << "                         -     same as option DEBUG" << endl;
	std::cout << "                               but option stopping given debugging from hole folder (<folder) or given subroutine (<folder>:<subroutine>)" << endl;
	std::cout << "                GETERRORSTRING <errornumber>" << endl;
	std::cout << "                         -     return for positive errornuber or negative warning number as an string definition" << endl;
	std::cout << "                load     -     load before saved file with extension .dbgsession" << endl;
	std::cout << "                               see command below inside editor use" << endl;
	std::cout << endl;
	std::cout << "        __________________________________________________________________________________________________________________________________" << endl;
	std::cout << "        follow commands are only usable" << endl;
	std::cout << "        inside editor, stared with option --hear:" << endl;
	std::cout << endl;
	ClientTransaction::writeHelpUsage("?debug", /*editor*/true);
	std::cout << endl;
}
