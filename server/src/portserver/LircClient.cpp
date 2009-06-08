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

#include "LircClient.h"
#ifdef _LIRCCLIENTLIBRARY

#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace ports
{
	LircClient::LircClient(string folderName, string subroutineName) :
		portBase("LIRC", folderName, subroutineName)
	{
		init();
	}

	LircClient::LircClient(string type, string folderName, string subroutineName) :
		portBase(type, folderName, subroutineName)
	{
		init();
	}

	void LircClient::init()
	{
	#ifdef _LIRCCLIENTLIBRARY

		if(lirc_init("irexec",1) == -1)
		{
			cout << "### ERROR: cannot initial lirc" << endl;
			cout << "    ERRNO: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}
		if(lirc_readconfig(NULL, &m_ptLircConfig, NULL) != 0)
		{
			cout << "### ERROR: cannot read lirc configuration" << endl;
			cout << "    ERRNO: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}

	#endif //_LIRCCLIENTLIBRARY
	}

	bool LircClient::measure()
	{
	#ifdef _LIRCCLIENTLIBRARY

		char *code;
		char *c;
		int ret;

		while(lirc_nextcode(&code) == 0)
		{
			if(code == NULL)
				continue;
			while(	(ret= lirc_code2char(m_ptLircConfig, code, &c)) == 0
					&&
					c != NULL										)
			{
	#ifdef DEBUG
				printf("Execing code \"%s\"\n", code);
				printf("Execing command \"%s\"\n", c);
	#endif
				system(c);
			}
			free(code);
			if(ret==-1)
				break;
		}


	#endif //_LIRCCLIENTLIBRARY
		return true;
	}

	LircClient::~LircClient()
	{
	#ifdef _LIRCCLIENTLIBRARY

		lirc_freeconfig(m_ptLircConfig);
		lirc_deinit();

	#endif //_LIRCCLIENTLIBRARY
	}
}
#endif //_LIRCCLIENTLIBRARY
