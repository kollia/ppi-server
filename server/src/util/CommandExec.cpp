/**
 *   This file 'CommandExec.cpp' is part of ppi-server.
 *   Created on: 08.10.2011
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

#include <cstdio>
#include <cstring>

#include "../pattern/util/LogHolderPattern.h"

#include "CommandExec.h"

int CommandExec::init(void* args)
{
	if(args != NULL)
		m_sCommand= *static_cast<string*>(args);
	if(m_sCommand == "")
		return -1;
	return 0;
}

int CommandExec::command_exec(SHAREDPTR::shared_ptr<CommandExec> thread, string command, vector<string>& result, bool& more, bool wait, bool block)
{
	int nRv;
	bool thwait(wait);
	vector<string>::size_type nLen;

	result.clear();
	more= false;
	if(block == true)
		thwait= false;	// routine should'nt wait for thread
						// now not the same as wait action
						// in subroutine

	if(!thread->started())
	{
		thread->start(&command, thwait);
		if(	wait == false ||
			block == true	)
		{
			if(block)
				more= true;
			return 0;
		}
	}
	if(	block == true &&
		thread->running()	)
	{
		more= true;
	}
	result= thread->getOutput();
	if(more == false)
	{
		nLen= result.size();
		if(nLen > 0)
		{
			if(result[nLen-1].substr(0, 11) == "ERRORLEVEL ")
			{
				istringstream errorlevel(result[nLen-1].substr(11));

				errorlevel >> nRv;
				result.pop_back();
				if(	nLen > 1 &&
					result[nLen-2]	== ""	)
				{
					result.pop_back();
				}
			}else
				nRv= -1;
		}else
			nRv= -1;
	}
	return nRv;
}

int CommandExec::execute()
{
	char line[130];
	string sline;
	string command;
	vector<string>::size_type nLen;
	FILE *fp;

	command= m_sCommand;
	command+= ";ERRORLEVEL=$?; echo; echo ERRORLEVEL $ERRORLEVEL";
	fp= popen(command.c_str(), "r");
	if(fp == NULL)
	{
		sline= "ERROR by writing command on folder subroutine " + m_sCommand + " on command line\n";
		sline+= "ERRNO" + *strerror(errno);
		LOG(LOG_ERROR, sline);
		return -1;
	}
	while(fgets(line, sizeof line, fp))
	{
		sline+= line;
		//cout << sline << flush;
		nLen= sline.length();
		if(	nLen > 0 &&
			sline.substr(nLen-1) == "\n")
		{
			sline= sline.substr(0, nLen-1);
			LOCK(m_RESULTMUTEX);
			m_vOutput.push_back(sline);
			UNLOCK(m_RESULTMUTEX);
			sline= "";
		}
	}
	pclose(fp);
	if(sline != "")
	{
		LOCK(m_RESULTMUTEX);
		m_vOutput.push_back(sline);
		UNLOCK(m_RESULTMUTEX);
	}
	stop(false);
	return 0;
}

vector<string> CommandExec::getOutput()
{
	vector<string> vRv;

	LOCK(m_RESULTMUTEX);
	vRv= m_vOutput;
	m_vOutput.clear();
	UNLOCK(m_RESULTMUTEX);
	return vRv;
}
