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

#include "../util/termmacro.h"
#include "../util/debug.h"

#include "../util/properties/configpropertycasher.h"

#include "Client.h"

using namespace std;
using namespace util;

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

		if(	param == "SET" ||
			param == "GET" ||
			param == "DEBUG" ||
			param == "STOPDEBUG" ||
			param == "DIR" ||
			param == "CONTENT" ||
			param == "STATUS" ||
			param == "status" ||
			param == "GETMINMAXERRORNUMS" ||
			param == "GETERRORSTRING" ||
			param == "STOP" ||
			param == "stop"||
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
				bRes= client.execute(workdir, vOptions, command);
				if(bRes > 0)
					return EXIT_FAILURE;
			}

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
	printf("no correct command be set\n");
	printf("type -? for help\n");
}

void help(char* cpSelf)
{
	printf("\n");
	printf("syntax:  %s [options] [command]\n", cpSelf);
	printf("\n");
	printf("       options:\n");
	printf("            -?  --help         - show this help.\n");
	printf("            -u  --user         - set user for client,\n");
	printf("                                 if no user be set, client ask for user when command isn't start/stop/restart\n");
	printf("                                 password will be always asked and cannot insert in command\n");
	printf("            -e  --errornums    - show only error numbers by get an error or warning from server,\n");
	printf("                                 otherwise the client display an error or warning description\n");
	printf("            -w  --wait         - hold transaction to server, where the user can write more than one command like an command line\n");
	printf("                                 after them user only must type commands\n");
	printf("                                 ending with 'quit' or 'exit'\n");
	printf("            -h  --hear         - same as before (--wait) but also client starts\n");
	printf("                                 an second connection to server,\n");
	printf("                                 where the client get changes which are set before\n");
	printf("                                 with the command 'HEAR'\n");
	printf("            -t  --thread       - display by status info all threads with running information,\n");
	printf("                                 otherwise the command status tell only how much threads on an process running\n");
	printf("            -c  --client       - like --thread but also display all communication-threads with which client-ID they are connected,\n");
	printf("                                 or it displays for no client\n");
	printf("            -p  --pid          - display by status info all process id's separated in own rows\n");
	printf("\n");
	printf("       command:\n");
	printf("                STOP     -     stopping server\n");
	printf("                STATUS   -     show how much threads for process are running.\n");
	printf("                               see also for options -t or -c\n");
	printf("                CHANGE <username>:<password>\n");
	printf("                         -     changing user, user name and password is separated with an colon\n");
	printf("                PERMISSION <groupnames>\n");
	printf("                         -     ask permission for group.\n");
	printf("                               also more than one groups can be ask, separated with an colon\n");
	printf("                GET <folder>:<subroutine>\n");
	printf("                         -     get the current value from the subroutines in the folder\n");
	printf("                               folder and subroutine are separated with an colon\n");
	printf("                SET <folder>:<subroutine> <value>\n");
	printf("                         -     set the given value from given subroutine in given folder\n");
	printf("                HEAR <folder>:<subroutine>\n");
	printf("                         -     if the client has set an second connection with -d,\n");
	printf("                               client can order with this command to hear on the given folder:subroutine's\n");
	printf("                               for changes\n");
	printf("                NEWENTRYS\n");
	printf("                         -     clearing all entry's which are set with the command HEAR\n");
	printf("                DIR <filter>\n");
	printf("                         -     shows all files in directory ${workdir}/client which are suitable to given filter\n");
	printf("                CONTENT <filename>\n");
	printf("                         -     send the file content of the given filename under ${workdir}/client\n");
	printf("                DEBUG [-ow] <folder[:subroutine]/owreaderID>\n");
	printf("                         -     show by running server debugging messages for given folder and subroutine\n");
	printf("                               when no subroutine given, the hole folder is set for debugging\n");
	printf("                               if option -ow be set client get DEBUG info for bechmark owreader\n");
	printf("                               and folder have to be the OWServer ID (owreaderID).\n");
	printf("                               (the OWServer ID you can see by starting ppi-server on command line after '### starting OWServer)\n");
	printf("                STOPDEBUG [-ow] <folder[:subroutine]/owreaderID>\n");
	printf("                         -     same as option DEBUG\n");
	printf("                               but option stoppig given debugging from hole folder (<folder) or given subroutine (<folder>:<subroutine>)\n");
	printf("                GETMINMAXERRORNUMS\n");
	printf("                         -     return two integer for maximal warning- and error-number.\n");
	printf("                               The first number is the higest negative warning number (from number to -1) or 0,\n");
	printf("                               the second the highest positive error number (from 1 to number) or 0\n");
	printf("                GETERRORSTRING <errornumber>\n");
	printf("                         -     return for positive errornuber or negative warning number as an string definition\n");
	printf("\n");
}
