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
#include "../util/thread/Terminal.h"

#include "../database/lib/DbInterface.h"

#include "../pattern/util/LogHolderPattern.h"

#include "shell.h"

using namespace ppi_database;

bool Shell::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bRv= true;

	properties->notAllowedParameter("begin");
	properties->notAllowedParameter("end");
	properties->notAllowedParameter("default");
	properties->notAllowedParameter("perm");
	if(!switchClass::init(properties, pStartFolder))
		bRv= false;
	m_bLastValue= 0;
	m_sBeginCom= properties->getValue("begincommand", /*warning*/false);
	m_sWhileCom= properties->getValue("whilecommand", /*warning*/false);
	m_sEndCom= properties->getValue("endcommand", /*warning*/false);
	m_bWait= properties->haveAction("wait");
	m_sGUI= properties->getValue("gui", /*warning*/false);
	if(	m_sBeginCom == ""
		&&
		m_sWhileCom == ""
		&&
		m_sEndCom == ""		)
	{
		string msg;

		msg= properties->getMsgHead(/*error*/true);
		msg+= "no command (begincommand/whilecommand/endcommand) be set";
		LOG(LOG_ERROR, msg);
		cerr << msg << endl;
		bRv= false;
	}
	return bRv;
}

double Shell::measure(const double actValue)
{
	bool bDebug(isDebug());
	int res;
	double dRv(0);
	string msg("make command '");

	if(m_bLastValue)
		dRv= 1;
	if(switchClass::measure(dRv))
	{
		bool bMaked= false;

		if(!m_bLastValue)
		{
			if(m_sBeginCom != "")
			{
				msg+= m_sBeginCom;
				msg+= "' for beginning";
				LOG(LOG_DEBUG, msg);
				if(bDebug)
					tout << msg << endl;
				res= system(m_sBeginCom.c_str());
				if(	m_bWait ||
					res == -1 ||
					res == 127	)
				{
					dRv= static_cast<double>(res);
				}else
					dRv= 1;
				bMaked= true;
			}
		}
		if(!bMaked)
		{
			if(m_sWhileCom != "")
			{
				msg+= m_sWhileCom;
				msg+= "' while during contact exists";
				TIMELOG(LOG_DEBUG, getFolderName(), msg);
				if(bDebug)
					tout << msg << endl;
				res= system(m_sWhileCom.c_str());
				if(	m_bWait ||
					res == -1 ||
					res == 127	)
				{
					dRv= static_cast<double>(res);
				}else
					dRv= 2;
			}
		}
		m_bLastValue= true;

	}else
	{
		if(m_bLastValue)
		{
			if(m_sEndCom != "")
			{
				msg+= m_sEndCom;
				msg+= "' for ending";
				TIMELOG(LOG_DEBUG, getFolderName(), msg);
				if(bDebug)
					tout << msg << endl;
				res= system(m_sEndCom.c_str());
				if(	m_bWait ||
					res == -1 ||
					res == 127	)
				{
					dRv= static_cast<double>(res);
				}else
					dRv= 3;
			}
		}
		m_bLastValue= false;
	}
	if(bDebug)
	{
		tout << "result of subroutine is " << dRv;
		if(!m_bWait)
		{
			tout << " for ";
			switch(static_cast<int>(dRv))
			{
			case 0:
				tout << "do nothing" << endl;
				break;
			case 1:
				tout << "BEGIN" << endl;
				break;
			case 2:
				tout << "WHILE" << endl;
				break;
			case 3:
				tout << "END" << endl;
				break;
			default:
				tout << "ERROR - take a look in LOG file!" << endl;
			}
		}else
		{
			if(dRv == -1 || dRv == 127)
				tout << "for ERROR - take a look in LOG file!" << endl;
		}
	}
	return dRv;
}

bool Shell::range(bool& bfloat, double* min, double* max)
{
	return false;
}

int Shell::system(const char *command)
{
	int res(0);
	pid_t child;

	if(m_sGUI != "")
	{
		DbInterface* db;

		db= DbInterface::instance();
		// toDo: sending command to client with X-Server
		TIMELOG(LOG_ERROR, getFolderName()+":"+getSubroutineName(), "sending to client with X-Server not implemented now");
		if(isDebug())
			tout << "sending to client with X-Server not implemented now" << endl;
		res= -1;

	}else if(m_bWait)
	{
		res= ::system(command);
		if(res == -1 || res == 127)
		{
			string msg("cannot make system on commandline in subroutine\nfor command '");

			msg+= command;
			msg+= "'\n";
			msg+= "ERRNO: " + string(strerror(errno));
			TIMELOG(LOG_ERROR, getFolderName()+":"+getSubroutineName(), msg);
			if(isDebug())
				tout << msg << endl;
		}
	}else
	{
		if((child= fork()) == EAGAIN)
		{
			string msg("cannot fork process in subroutine\nfor command '");

			msg+= command;
			msg+= "'\n";
			msg+= "ERRNO: " + string(strerror(errno));
			TIMELOG(LOG_ERROR, getFolderName()+":"+getSubroutineName(), msg);
			if(isDebug())
				tout << msg << endl;
			return -1;
		}

		if(child == 0)
		{
			res= ::system(command);
			if(res == -1 || res == 127)
			{
				string msg("cannot make system on commandline in subroutine\nfor command '");

				msg+= command;
				msg+= "'\n";
				msg+= "ERRNO: " + string(strerror(errno));
				LOG(LOG_ERROR, msg);
				if(isDebug())
					cerr << msg << endl;
				exit(EXIT_FAILURE);
			}
			//tout << "ending child witch starting command '" << command << "'" << endl;
			//tout << "------------------------------------------------------------------------------------------" << endl;
			exit(EXIT_SUCCESS);
		}
	}
	return res;
}

Shell::~Shell()
{
}
