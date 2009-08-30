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

#include <iostream>

#include "../util/structures.h"

#include "../logger/lib/LogInterface.h"

#include "shell.h"

//void Shell::init(folder *pStartFolder, string sWhile, string beginCommand, string whileCommand, string endCommand)
bool Shell::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
{
	properties.notAllowedParameter("begin");
	properties.notAllowedParameter("end");
	if(!switchClass::init(properties, pStartFolder))
		return false;
	m_sBeginCom= properties.getValue("begincommands", /*warning*/false);
	m_sWhileCom= properties.getValue("whilecommand", /*warning*/false);
	m_sEndCom= properties.getValue("endcommand", /*warning*/false);
	if(	m_sBeginCom == ""
		&&
		m_sWhileCom == ""
		&&
		m_sEndCom == ""		)
	{
		string msg;

		msg= properties.getMsgHead(/*error*/true);
		msg+= "no command (begincommand/whilecommand/endcommand) be set";
		LOG(LOG_ERROR, msg);
		cerr << msg << endl;
		return false;
	}
	return true;
}

bool Shell::measure()
{
	string who("i:" + getFolderName());
	bool bSwitched= switchClass::getValue(who);
	bool bResultTrue;
	string msg("make command '");

	switchClass::measure();
	bResultTrue= switchClass::getValue(who);

	if(bResultTrue)
	{
		bool bMaked= false;

		if(!bSwitched)
		{
			if(m_sBeginCom != "")
			{
				msg+= m_sBeginCom;
				msg+= "' for beginning";
				TIMELOG(LOG_INFO, getFolderName(), msg);
				if(isDebug())
					cout << msg << endl;
				system(m_sBeginCom.c_str());
				bMaked= true;
			}
		}
		if(!bMaked)
		{
			if(m_sWhileCom != "")
			{
				msg+= m_sWhileCom;
				msg+= "' while during contact exists";
				TIMELOG(LOG_INFO, getFolderName(), msg);
				if(isDebug())
					cout << msg << endl;
				system(m_sWhileCom.c_str());
			}
		}
	}else
	{
		if(bSwitched)
		{
			if(m_sEndCom != "")
			{
				msg+= m_sEndCom;
				msg+= "' for ending";
				TIMELOG(LOG_DEBUG, getFolderName(), msg);
				if(isDebug())
					cout << msg << endl;
				system(m_sEndCom.c_str());
			}
		}
	}
	return true;
}

bool Shell::range(bool& bfloat, double* min, double* max)
{
	return false;
}

void Shell::system(const char *command)
{
	pid_t child;
	//int status;

	if((child= fork()) < 0)
	{
		string msg("cannot fork process in subroutine\nfor command '");

		msg+= command;
		msg+= "'";
		LOG(LOG_ALERT, msg);
		return;
	}

	if(child == 0)
	{
		if(::system(command) != 0)
		{
			string msg("cannot make system on commandline in subroutine\nfor command '");

			msg+= command;
			msg+= "'";
			LOG(LOG_ALERT, msg);
		}
		//cout << "ending child witch starting command '" << command << "'" << endl;
		//cout << "------------------------------------------------------------------------------------------" << endl;
		exit(1);
	}
}

Shell::~Shell()
{
}
