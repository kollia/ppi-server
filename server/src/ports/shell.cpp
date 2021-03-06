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
#include <boost/regex.hpp>

#include <stdlib.h>
#include <sys/types.h>

#include <iostream>

#include "../util/structures.h"
#include "../util/exception.h"
#include "../util/properties/PPIConfigFileStructure.h"
#include "../util/stream/ErrorHandling.h"
#include "../util/thread/Terminal.h"

#include "../database/logger/lib/logstructures.h"
#include "../pattern/util/LogHolderPattern.h"

#include "shell.h"

using namespace boost;

bool Shell::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bRv= true;
	string user, folder(getFolderName()), subroutine(getSubroutineName());
	PPIConfigFiles configFiles;

	configFiles= PPIConfigFileStructure::instance();
	properties->notAllowedParameter("default");
	properties->notAllowedParameter("perm");
	properties->notAllowedAction("binary");
	if(!switchClass::init(properties, pStartFolder))
		bRv= false;
	m_bInfo= !properties->haveAction("noinfo");
	m_bLogOubput= properties->haveAction("debug");
	m_bLastValue= 0;
	m_bStarting= false;
	m_sWhileCom= properties->getValue("command", /*warning*/false);
	trim(m_sWhileCom);
	if(m_sWhileCom != "")
	{
		m_bCommandSet= true;
		m_sWhileCom= configFiles->createCommand(folder, subroutine, "command", m_sWhileCom);
	}else
	{
		m_bCommandSet= false;
		m_bWait= properties->haveAction("wait");
		if(!m_bWait)
			m_bLastRes= properties->haveAction("last");
		else
			m_bLastRes= false;
		m_bBlock= properties->haveAction("block");
		if(!m_bBlock)
		{
			m_sBeginCom= properties->getValue("begincommand", /*warning*/false);
			trim(m_sBeginCom);
		}
		m_sWhileCom= properties->getValue("whilecommand", /*warning*/false);
		trim(m_sWhileCom);
		if(!m_bBlock)
		{
			m_sEndCom= properties->getValue("endcommand", /*warning*/false);
			trim(m_sEndCom);
		}
		if(m_sBeginCom != "")
			m_sBeginCom= configFiles->createCommand(folder, subroutine, "begincommand", m_sBeginCom);
		if(m_sWhileCom != "")
			m_sWhileCom= configFiles->createCommand(folder, subroutine, "whilecommand", m_sWhileCom);
		if(m_sEndCom != "")
			m_sEndCom= configFiles->createCommand(folder, subroutine, "endcommand", m_sEndCom);
	}


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
	m_bLogError= !properties->haveAction("noerrorlog");
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
	if(bRv)
	{
		if(m_sUserAccount == "")
		{
			short count(0);
			SHAREDPTR::shared_ptr<CommandExec> thread;

			if(m_sBeginCom != "")
				++count;
			if(m_sWhileCom != "")
				++count;
			if(m_sEndCom != "")
				++count;
			for(short n= 0; n < count; ++n)
			{
				EHObj res;

				thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec(this, m_bLogError, m_bInfo,
												getRunningThread()->getExternSendDevice()));
				thread->setFor(folder, subroutine);
				res= thread->start();
				if(res->fail())
				{
					int log;
					string msg;

					res->addMessage("Shell", "CommandExecStartInit", folder + "@" + subroutine);
					msg= res->getDescription();
					if(res->hasError())
					{
						log= LOG_ERROR;
						cerr << glob::addPrefix("### ERROR: ", msg) << endl;
					}else
					{
						log= LOG_WARNING;
						cout << glob::addPrefix("### WARNING: ", msg) << endl;
						m_vCommandThreads.push_back(thread);
					}
					LOG(log, msg);
				}else
					m_vCommandThreads.push_back(thread);
			}
		}else
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
	}
	return bRv;
}

string Shell::checkStartPossibility()
{
	ostringstream sRv;

	if(!m_bCommandSet)
	{
		sRv << "subroutine " << getFolderName() << ":" << getSubroutineName();
		sRv << " from type " << getType() << endl;
		sRv << "is not designed to start any action per time." << endl;
		sRv << "Inside subroutine has to be only defined parameter 'command'";
	}else
	{
		LOCK(m_WRITTENVALUES);
		m_bStarting= true;
		UNLOCK(m_WRITTENVALUES);
	}
	return sRv.str();
}

bool Shell::startingBy(const ppi_time& tm, const InformObject& from)
{
	bool bRv(false);
	bool bDebug(isDebug());
	vector<string> result;

	LOCK(m_WRITTENVALUES);
	bRv= m_bStarting;
	if(!bRv)
		m_oExternalStarting= from;
	UNLOCK(m_WRITTENVALUES);
	if(bRv)
	{
		if(m_sUserAccount != "")
		{
			string command;

			command= getFolderName() + " " + getSubroutineName();
			command+= " starting " + tm.toString(/*as date*/true);
			command+= " " + from.getDefString();
			if(m_pOWServer->command_exec(false, command, result, m_bMore))
				bRv= false;
		}else
		{
			if(!inlineStarting("starting", m_sWhileCom, result, tm, bDebug))
				bRv= false;
		}
	}
	return bRv;
}

auto_ptr<IValueHolderPattern> Shell::measure(const ppi_value& actValue)
{
	bool bDebug(isDebug());
	bool bswitch(false);
	bool bMaked(false);
	int res(0);
	double dRv(actValue);
	double dLastSwitch;
	auto_ptr<IValueHolderPattern> oMeasureValue;

	oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
	//Debug info to stop by right subroutine
/*	if(	getFolderName() == "log_weather_starter" &&
		getSubroutineName() == "logging"	)
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << getFolderName() << ":" << getSubroutineName() << endl;
	}*/
	if(	!m_bLastRes &&
		dRv >= 0		)
	{
		dRv= 0;
	}
	if(m_bLastValue)
		dLastSwitch= 1;
	else
		dLastSwitch= 0;
	if(!m_bStarting)
	{//check begin/while/end only when subroutine is not defined for external starting
		if(switchClass::measure(dLastSwitch)->getValue())
			bswitch= true;
	}
	if(bswitch)
	{

		if(!m_bLastValue)
		{
			if(m_sBeginCom != "")
			{
				res= system("begincommand", m_sBeginCom);
				dRv= 1;	// doing begin command
				bMaked= true;
			}
		}
		if(!bMaked)
		{
			if(m_sWhileCom != "")
			{
				res= system("whilecommand", m_sWhileCom);
				dRv= 2;	// doing while command
				bMaked= true;
			}
		}
		m_bLastValue= true;
		if(	m_bWait ||
			res < 0		)
		{
			dRv= static_cast<double>(res);
		}

	}else // if(bswitch)
	{
		if(m_bLastValue)
		{
			if(m_sEndCom != "")
			{
				res= system("endcommand", m_sEndCom);
				dRv= 3; // doing end command
				bMaked= true;
			}
		}
		m_bLastValue= false;
		if(	m_bWait ||
			res < 0 	)
		{
			dRv= static_cast<double>(res);
		}
	} // else branch if(bswitch)
	if(bDebug)
	{
		ostringstream outStr;

		if(m_bStarting)
		{
			m_bBlock= true;
			system("read", m_sWhileCom);
			outStr << "SHELL routine is defined for external starting" << endl;
		}
		outStr << "result of subroutine is " << dRv << " for ";
		switch(static_cast<int>(dRv))
		{
		case 0:
			outStr << "do nothing";
			break;
		case 1:
			if(bMaked)
				outStr << "making ";
			else
				outStr << "last command ";
			outStr << "'begincommand'";
			break;
		case 2:
			if(bMaked)
				outStr << "making ";
			else
				outStr << "last command ";
			outStr << "'whilecommand'";
			break;
		case 3:
			if(bMaked)
				outStr << "making ";
			else
				outStr << "last command ";
			outStr << "'endcommand'";
			break;
		default:
			if(bMaked)
				outStr << "ERROR - take a look into LOG file!";
			else
				outStr << "ERROR of one of last passing";
			break;
		}
		if(	bMaked &&
			!bDebug &&
			!m_bWait &&
			dRv > 0		)
		{
			outStr << " will be set external from shell command" << endl;
			outStr << "current value is " << actValue;
		}
		out() << outStr.str() << endl;
	}
	if(	!bDebug &&
		!m_bWait &&
		m_bInfo &&
		dRv > 0		)
	{// when wait, debug is false and info true, subroutine will be set from external
		oMeasureValue->setValue(actValue);// thread before starting shell command
	}else                   // so do not make any changes now
		oMeasureValue->setValue(dRv);
	return oMeasureValue;
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
	ErrorHandling errHandle;

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
	TIMELOGEX(LOG_DEBUG, folder+":"+subroutine+action, msg, getRunningThread()->getExternSendDevice());
	if(bDebug)
		out() << msg << endl;
	if(m_sGUI != "")
	{
		// toDo: sending command to client with X-Server
		TIMELOG(LOG_ERROR, folder+":"+subroutine, "sending to client with X-Server not implemented now");
		if(bDebug)
			out() << "sending to client with X-Server not implemented now" << endl;
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
				out() << "execute reading ";
			else
				out() << "execute command ";
			out() << "by foreign process with user account " << m_sUserAccount << endl;
		}
		res= m_pOWServer->command_exec(wait, command, result, m_bMore);

	}else
	{
		ppi_time nullTime;

		res= inlineStarting(action, command, result, nullTime, bDebug);
	}

	if(	m_bWait ||
		bDebug		)
	{
		if(bDebug)
		{
			if(	action != "read" &&
				m_bMore &&
				result.empty()		)
			{
				out() << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
				out() << "no output exist by begin of blocking, or since last pass of subroutine." << endl;
				out() << "maybe get by next pass or one follow" << endl;

			}else
			{
				out() << "output of command:" << endl;
				out() << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
			}
		}
		for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
		{
			if(bDebug)
			{
				errHandle.clear();
				errHandle.setErrorStr(*it);
				if(errHandle.fail())
				{
					string level("### ");

					if(errHandle.hasError())
						level+= "ERROR: ";
					else
						level+= "WARNING: ";
					out() << glob::addPrefix(level, errHandle.getDescription()) << endl;
				}else
					out() << *it << endl;
			}
			if(	it->length() > 8 &&
				it->substr(0, 8) == "-PPI-SET"	)
			{
				if(!setValue(true, command, it->substr(1)))
				{
					if(bDebug)
						out() << " ### ERROR: cannot read correctly PPI-SET command (look into log file)" << endl;
				}
			}
		}
		if(bDebug)
			out() << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	}
/*	if(bDebug)
	{
		if(	action != "read" &&
			m_bMore &&
			result.empty()		)
		{
			out() << "~~~~~~~~" << endl;
			out() << "no output exist by begin of blocking, get by next pass" << endl;
			out() << "~~~~~~~~" << endl;

		}else
		{
			out() << "output of command:" << endl;
			out() << "~~~~~~~~" << endl;
			for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
			{
				out() << *it << endl;
			}
			out() << "~~~~~~~~" << endl;
		}
	}*/
	return res;
}

int Shell::inlineStarting(const string& action, string command, vector<string>& result,
				const ppi_time& tm, bool bDebug)
{
	bool bchangedVec(false);
	int res;
	short nCommand;
	short count(0), runCount(0);
	SHAREDPTR::shared_ptr<CommandExec> thread;
	typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;
	bool bReadInform(false);
	InformObject oExternalStarting;

	if(m_sBeginCom != "")
		++count;
	if(m_sWhileCom != "")
		++count;
	if(m_sEndCom != "")
		++count;
	LOCK(m_EXECUTEMUTEX);
	if(action == "read")
	{// when action is defined with read, m_bBlock is true
		for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
		{
			if(	!(*it)->stopping() &&
				!(*it)->wait()			)
			{
				thread= *it;
				break;
			}
		}
		if(thread == NULL)
		{
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
			{
				if(!(*it)->stopping())
				{
					thread= *it;
					break;
				}
			}
			if(	thread == NULL &&
				!m_vCommandThreads.empty()	)
			{
				thread= m_vCommandThreads[0];
			}
		}

	}else
	{
		for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
		{
			if(	!(*it)->stopping() &&
				(*it)->wait()			)
			{
				thread= *it;
				break;
			}
		}
		if(thread == NULL)
		{
			EHObj res;

			out() << "create new CommandExec Thread for action " << action << endl;
			thread= SHAREDPTR::shared_ptr<CommandExec>(new CommandExec(this, m_bLogError, m_bInfo,
							getRunningThread()->getExternSendDevice()));
			thread->setFor(getFolderName(), getSubroutineName());
			res= thread->start();
			if(res->fail())
			{
				int log;
				string msg;

				res->addMessage("Shell", "CommandExecStart",
								command + "@" + getFolderName() + "@" + getSubroutineName());
				msg= res->getDescription();
				if(res->hasError())
				{
					log= LOG_ERROR;
					cerr << glob::addPrefix("### ERROR: ", msg) << endl;
				}else
				{
					log= LOG_WARNING;
					cout << glob::addPrefix("### WARNING: ", msg) << endl;
					m_vCommandThreads.push_back(thread);
				}
				LOG(log, msg);
				UNLOCK(m_EXECUTEMUTEX);
				return -2;
			}else
				m_vCommandThreads.push_back(thread);
		}
	}
	UNLOCK(m_EXECUTEMUTEX);

	if(	m_bBlock &&
		!thread->running())
	{
		LOCK(m_WRITTENVALUES);
		m_msdWritten.clear();
		if(action == "starting")
		{
			oExternalStarting= m_oExternalStarting;
			bReadInform= true;
		}
		UNLOCK(m_WRITTENVALUES);
	}
	if(action == "begincommand")
		nCommand= 1;
	else if(action == "whilecommand")
		nCommand= 2;
	else if(action == "endcommand")
		nCommand= 3;
	else if(action == "starting")
		nCommand= 2;
	else
		nCommand= 0;
	thread->setWritten(&m_msdWritten, m_WRITTENVALUES, nCommand);
	LOCK(m_EXECUTEMUTEX);
	// incoming more is for set subroutine to 0 when
	// no shell command be starting (no ERROR)
	m_bMore= !m_bLastRes;
	//cout << "--- run '" << command << "' of " << getFolderName() << ":" << getSubroutineName() << endl;
	if(action == "starting")
	{
		if(!bReadInform)
		{
			LOCK(m_WRITTENVALUES);
			oExternalStarting= m_oExternalStarting;
			UNLOCK(m_WRITTENVALUES);
		}
		res= thread->startingBy(tm, command, oExternalStarting);
	}else
	{
		if(m_bLogOubput)
			bDebug= true;
		res= CommandExec::command_exec(thread, command, result, m_bMore, m_bWait, m_bBlock, bDebug);
	}
	do{// remove all not needed threads from vector
		bchangedVec= false;
		for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
		{
			if(	runCount >= count &&
				!(*it)->stopping() &&
				(*it)->wait()			)
			{
				(*it)->stop(false);
			}
			if((*it)->stopping())
			{
				//cout << "remove one CommandExec thread for " << folder << ":" << subroutine;
				//cout << " because " << m_vCommandThreads.size() << " are running" << endl;
				if((*it)->running())
				{// when thread should stopping,
				 // waiting for finished
					(*it)->stop(true);
				}
				//cout << "remove CommandExec Thread inside folder " << folder << ":" << subroutine << endl;
				m_vCommandThreads.erase(it);
				bchangedVec= true;
				break;
			}else
				++runCount;
		}
	}while(bchangedVec);
	UNLOCK(m_EXECUTEMUTEX);
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
	/**
	 * return value for writing command_exec,
	 * but do not need for any case by command info
	 */
	bool bMore;
	/**
	 * command to writing to command_exec
	 */
	string command;
	/**
	 * result strings for writing command_exes,
	 * but do not need for any case by command info
	 */
	vector<string> result;

	switchClass::setDebug(bDebug);
	m_oMicroseconds.doOutput(bDebug);
	m_oMilliseconds.doOutput(bDebug);
	m_oSeconds.doOutput(bDebug);
	m_oMinutes.doOutput(bDebug);
	m_oHours.doOutput(bDebug);
	m_oDays.doOutput(bDebug);
	m_oMonths.doOutput(bDebug);
	m_oYears.doOutput(bDebug);
	if(	m_bBlock &&
		!m_bLogOubput	)
	{
		/*
		 * when subroutine defined for blocking
		 * CommandExec has also to know whether debug
		 * is defined for logging output
		 * but not when output logging is set for all the time
		 * because CommandExec thinking always debug is set
		 */
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
			m_pOWServer->command_exec(false, command, result, bMore);

		}else
		{
			typedef vector<SHAREDPTR::shared_ptr<CommandExec> >::iterator thIt;

			LOCK(m_EXECUTEMUTEX);
			for(thIt it= m_vCommandThreads.begin(); it != m_vCommandThreads.end(); ++it)
				CommandExec::command_exec(*it, "info", result, bMore, m_bWait, m_bBlock, bDebug);
			UNLOCK(m_EXECUTEMUTEX);
		}
	}
}

bool Shell::setValue(bool always, const string& shellcommand, const string& command)
{
	static bool bFirst(true);
	static CommandExec thread(this, /*log*/false,
					getRunningThread()->getExternSendDevice());

	if(bFirst)
	{
		thread.setFor(getFolderName(), getSubroutineName());
		thread.setWritten(&m_msdWritten, m_WRITTENVALUES, /*command not need*/0);
		bFirst= false;
	}
	if(!thread.setValue(command, /*log*/false))
	{
		thread.getOutput();// clear output inside thread
		return false;
	}
	return true;
}

Shell::~Shell()
{
	DESTROYMUTEX(m_EXECUTEMUTEX);
	DESTROYMUTEX(m_WRITTENVALUES);
}
