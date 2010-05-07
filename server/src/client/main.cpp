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

#include "../util/termmacro.h"
#include "../util/debug.h"
#include "../util/configpropertycasher.h"
#include "../util/Thread.h"

#include "Client.h"

using namespace std;
using namespace util;

void usage(void);
void help(char* cpSelf);

int main(int argc, char* argv[])
{
	bool result;
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

		if(nArcPos < argc)
			param= argv[nArcPos];
		else if(param == "-?")
		{
			help(argv[0]);
			return EXIT_SUCCESS;
		}else
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
			param == "STATUS"
			||
			param == "status"
			||
			param == "GETMINMAXERRORNUMS"
			||
			param == "GETERRORSTRING"
			||
			param == "STOP"
			||
			param == "stop"
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
	printf("                STOP     -     stopping server\n");
	printf("                STATUS   -     show how much threads for process are running.\n");
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
	printf("                GETMINMAXERRORNUMS\n");
	printf("                         -     return two integer for maximal warning- and error-number.\n");
	printf("                               The first number is the higest negative warning number (from number to -1) or 0,\n");
	printf("                               the second the highest positive error number (from 1 to number) or 0\n");
	printf("                GETERRORSTRING <errornumber>\n");
	printf("                         -     return for positive errornuber or negative warning number as an string definition\n");
	printf("\n");
}
