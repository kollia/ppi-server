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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <stdlib.h>

#include <iostream>

#include "../util/structures.h"
#include "../util/thread/Terminal.h"

#include "../database/lib/DbInterface.h"

#include "../pattern/util/LogHolderPattern.h"

#include "shell.h"

using namespace ppi_database;
using namespace boost;

bool Shell::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bRv= true;
	string user;

	properties->notAllowedParameter("default");
	properties->notAllowedParameter("perm");
	properties->notAllowedAction("binary");
	if(!switchClass::init(properties, pStartFolder))
		bRv= false;
	m_bLastValue= 0;
	m_sBeginCom= properties->getValue("begincommand", /*warning*/false);
	m_sWhileCom= properties->getValue("whilecommand", /*warning*/false);
	m_sEndCom= properties->getValue("endcommand", /*warning*/false);
	m_bWait= properties->haveAction("wait");
	m_bBlock= properties->haveAction("block");
	m_oYears.init(pStartFolder, properties->getValue("year", /*warning*/false));
	if(!m_oYears.isEmpty())
		m_bFixTimePoll= true;
	m_oMonths.init(pStartFolder, properties->getValue("month", /*warning*/false));
	if(!m_oMonths.isEmpty())
		m_bFixTimePoll= true;
	m_oDays.init(pStartFolder, properties->getValue("day", /*warning*/false));
	if(!m_oDays.isEmpty())
		m_bFixTimePoll= true;
	m_oHours.init(pStartFolder, properties->getValue("hour", /*warning*/false));
	if(!m_oHours.isEmpty())
		m_bFixTimePoll= true;
	m_oMinutes.init(pStartFolder, properties->getValue("min", /*warning*/false));
	if(!m_oMinutes.isEmpty())
		m_bFixTimePoll= true;
	m_oSeconds.init(pStartFolder, properties->getValue("sec", /*warning*/false));
	if(!m_oSeconds.isEmpty())
		m_bFixTimePoll= true;
	if(	m_oYears.isEmpty() &&
		m_oMonths.isEmpty()		)
	{
		m_oMilliseconds.init(pStartFolder, properties->getValue("millisec", /*warning*/false));
		if(!m_oMilliseconds.isEmpty())
			m_bFixTimePoll= true;
		m_oMicroseconds.init(pStartFolder, properties->getValue("microsec", /*warning*/false));
		if(!m_oMicroseconds.isEmpty())
			m_bFixTimePoll= true;
	}
	m_bMore= false;
	//m_sGUI= properties->getValue("gui", /*warning*/false);
	m_sUserAccount= properties->getValue("runuser", /*warning*/false);
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
	if(bRv && m_sUserAccount != "")
	{
		m_pOWServer= OWInterface::getServer("SHELL", m_sUserAccount);
		if(!m_pOWServer)
		{
			string msg;

			msg= properties->getMsgHead(/*error message*/false);
			msg+= "cannot find OWServer for user account " + m_sUserAccount;
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			setDeviceAccess(false);
			bRv= false;
		}
	}
	return bRv;
}

double Shell::measure(const double actValue)
{
	bool bDebug(isDebug());
	bool bswitch(false);
	int res;
	double dRv(actValue);
	double dLastSwitch;

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "configure" &&
		getSubroutineName() == "ppi_server"	)
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << getFolderName() << ":" << getSubroutineName() << endl;
	}*/
	if(m_bLastValue)
		dLastSwitch= 1;
	else
		dLastSwitch= 0;
	if(switchClass::measure(dLastSwitch))
		bswitch= true;
	if(m_bMore)
	{// command was sending in the last pass
	 // and should get now only results
	 // no command be necessary
		res= system("read", "need_no_command");
		m_bLastValue= bswitch;
		if(	m_sUserAccount == "" ||
			m_bWait ||
			bDebug		)
		{
			dRv= static_cast<double>(res);
		}

	}else
	{
		if(bswitch)
		{
			bool bMaked= false;

			if(!m_bLastValue)
			{
				if(m_sBeginCom != "")
				{
					res= system("begincommand", m_sBeginCom);
					bMaked= true;
				}
			}
			if(!bMaked)
			{
				if(m_sWhileCom != "")
				{
					res= system("whilecommand", m_sWhileCom);
				}
			}
			m_bLastValue= true;
			if(	m_sUserAccount == "" ||
				m_bWait ||
				bDebug		)
			{
				dRv= static_cast<double>(res);
			}

		}else
		{
			if(m_bLastValue)
			{
				if(m_sEndCom != "")
				{
					res= system("endcommand", m_sEndCom);
				}
			}
			m_bLastValue= false;
			if(	m_sUserAccount == "" ||
				m_bWait ||
				bDebug		)
			{
				dRv= static_cast<double>(res);
			}
		}
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
				break;
			}
		}else
		{
			if(dRv == -1 || dRv == 127)
				tout << " for ERROR - take a look in LOG file!";
			tout << endl;
		}
	}
	return dRv;
}

bool Shell::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= -1;
	return true;
}

int Shell::system(const string& action, string command)
{
	bool wait(m_bWait);
	bool bDebug(isDebug());
	int res(0);
	vector<string> result;
	string msg, nocorrread, folder(getFolderName()), subroutine(getSubroutineName());

	if(	m_bWait == false &&
		bDebug == true		)
	{// when subroutine is set for debugging
		wait= true; // always wait for output script strings
	}
	if(action != "read")
	{
		msg= "make " + action + " '" + command + "'";
	}else
		msg= "read command results from one of last pass";
	TIMELOG(LOG_DEBUG, folder+":"+subroutine, msg);
	if(bDebug)
		tout << msg << endl;
	if(m_sGUI != "")
	{
		// toDo: sending command to client with X-Server
		TIMELOG(LOG_ERROR, folder+":"+subroutine, "sending to client with X-Server not implemented now");
		if(bDebug)
			tout << "sending to client with X-Server not implemented now" << endl;
		res= -1;

	}
	if(m_sUserAccount != "")
	{
		command= folder + " " + subroutine;
		command+= " " + action;
		if(m_bWait)
			command+= " wait";
		if(m_bBlock)
			command+= " block";
		if(bDebug)
		{
			command+= " debug";
			if(action == "read")
				tout << "execute reading ";
			else
				tout << "execute command ";
			tout << "by foreign process with user account " << m_sUserAccount << endl;
		}
		res= m_pOWServer->command_exec(wait, command, result, m_bMore);

	}else
	{
		bool bchangedVec(false);
		SHAREDPTR::shared_ptr<CommandExec> thread;
		typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;


		if(	action == "read" &&
			!m_vCommandThreads.empty()	)
		{// when action is defined with read, m_bBlock is true
		 // and variable m_vCommandThread has only one thread inside
			thread= m_vCommandThreads[0];

		}else
		{
			thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec(this));
			thread->setFor(folder, subroutine);
		}

		if(	m_bBlock &&
			!thread->running())
		{
			LOCK(m_WRITTENVALUES);
			m_msdWritten.clear();
			UNLOCK(m_WRITTENVALUES);
		}
		thread->setWritten(&m_msdWritten, m_WRITTENVALUES);
		LOCK(m_EXECUTEMUTEX);
		res= CommandExec::command_exec(thread, command, result, m_bMore, m_bWait, m_bBlock, bDebug);
		do{// remove all not needed threads from vector
			bchangedVec= false;
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
			{
				if(!(*it)->running())
				{
					m_vCommandThreads.erase(it);
					bchangedVec= true;
					break;
				}
			}
		}while(bchangedVec);
		if(	m_bWait == false ||
			m_bBlock == true ||
			thread->running()	)
		{// give CommandExec inside queue and delete object by next pass
		 // when he was stopping between
			if(action != "read")
				m_vCommandThreads.push_back(thread);
		}
		UNLOCK(m_EXECUTEMUTEX);
	}
	if(m_bWait)
	{
		if(bDebug)
		{
			if(	action != "read" &&
				m_bMore &&
				result.empty()		)
			{
				tout << "~~~~~~~~" << endl;
				tout << "no output exist by begin of blocking, get by next pass" << endl;
				tout << "~~~~~~~~" << endl;

			}else
			{
				tout << "output of command:" << endl;
				tout << "~~~~~~~~" << endl;
			}
		}
		for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
		{
			if(bDebug)
				tout << *it << endl;
			if(	it->length() > 8 &&
				it->substr(0, 8) == "-PPI-SET"	)
			{
				if(!setValue(true, *it))
				{
					if(bDebug)
						tout << " ### ERROR: cannot read correctly PPI-SET command" << endl;
					TIMELOG(LOG_WARNING, "shell_setValue"+command+*it, "SHELL subroutine " + folder + ":" + subroutine
									+ "\nby command: " + command + "\noutput string '" + *it
									+ "'\n               ### ERROR: cannot read correctly PPI-SET command"               );
				}
			}
		}
		if(bDebug)
			tout << "~~~~~~~~" << endl;
	}
	if(bDebug)
	{
		if(	action != "read" &&
			m_bMore &&
			result.empty()		)
		{
			tout << "~~~~~~~~" << endl;
			tout << "no output exist by begin of blocking, get by next pass" << endl;
			tout << "~~~~~~~~" << endl;

		}else
		{
			tout << "output of command:" << endl;
			tout << "~~~~~~~~" << endl;
			for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
			{
				tout << *it << endl;
			}
			tout << "~~~~~~~~" << endl;
		}
	}
	return res;
}

void Shell::stop(const bool* bWait/*= NULL*/)
{
	typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;

	for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
		(*it)->stop(bWait);
}

void Shell::setDebug(bool bDebug)
{
	string command;
	vector<string> result;

	portBase::setDebug(bDebug);
	m_oMicroseconds.doOutput(bDebug);
	m_oMilliseconds.doOutput(bDebug);
	m_oSeconds.doOutput(bDebug);
	m_oMinutes.doOutput(bDebug);
	m_oHours.doOutput(bDebug);
	m_oDays.doOutput(bDebug);
	m_oMonths.doOutput(bDebug);
	m_oYears.doOutput(bDebug);
	if(m_bBlock)
	{
		if(m_sUserAccount != "")
		{
			command= getFolderName() + " " + getSubroutineName() + " info ";
			if(!bDebug)
				command+= "no";
			command+= "debug";
			if(m_bWait)
				command+= " wait";
			if(m_bBlock)
				command+= " block";
			m_pOWServer->command_exec(false, command, result, m_bMore);

		}else
		{
			typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;

			LOCK(m_EXECUTEMUTEX);
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
				CommandExec::command_exec(*it, "info", result, m_bMore, m_bWait, m_bBlock, bDebug);
			UNLOCK(m_EXECUTEMUTEX);
		}
	}
}

bool Shell::setValue(bool always, const string& command)
{
	double value;
	string outstr;
	vector<string> spl;
	istringstream icommand(command);
	map<string, double>::iterator it;

	icommand >> outstr; // string of PPI-SET (not needed)
	if(	icommand.eof() ||
		icommand.fail()		)
	{
		return false;
	}
	icommand >> outstr; // folder:subroutine string
	if(	icommand.eof() ||
		icommand.fail()		)
	{
		return false;
	}
	split(spl, outstr, is_any_of(":"));
	if(spl.size() != 2)
		return false;
	icommand >> value;
	if(icommand.fail())
		return false;
	if(!always)
	{
		LOCK(m_WRITTENVALUES);
		it= m_msdWritten.find(outstr);
		if(	it == m_msdWritten.end() ||
			it->second != value			)
		{
			m_msdWritten[outstr]= value;
			always= true;
		}
		UNLOCK(m_WRITTENVALUES);
	}
	if(always)
		if(!portBase::setValue(spl[0], spl[1], value, "SHELL-command_"+outstr))
			return false;
	return true;
}

Shell::~Shell()
{
	DESTROYMUTEX(m_EXECUTEMUTEX);
	DESTROYMUTEX(m_WRITTENVALUES);
}
