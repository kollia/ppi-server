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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/regex.hpp>

#include <cstdio>
#include <cstring>
#include <cmath>

#include "../database/logger/lib/logstructures.h"
#include "../pattern/util/LogHolderPattern.h"

#include "debugsubroutines.h"
#include "exception.h"

#include "stream/ppivalues.h"

#include "CommandExec.h"

using namespace boost;
using namespace design_pattern_world::util_pattern;

int CommandExec::init(void* args)
{
	char* cCommand;

	if(args != NULL)
	{
		try{
			cCommand= static_cast<char*>(args);
			m_sCommand= string(cCommand);

		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("make static cast from void *args value to command string");
			err= ex.getTraceString();
			cerr << endl << err << endl;
			LOG(LOG_ERROR, err);
			return -2;

		}catch(std::exception& ex)
		{
			string err;

			err=  "ERROR: STD exception by make static cast from void *args value to command string\n";
			err+= "what(): " + string(ex.what());
			cerr << err << endl;
			try{
				LOG(LOG_ERROR, err);
			}catch(...)
			{
				cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			}
			return -2;

		}catch(...)
		{
			string error;

			error+= "ERROR: catching UNKNOWN exception by make static cast from void *args value to command string";
			cerr << error << endl;
			try{
				LOG(LOG_ALERT, error);
			}catch(...)
			{
				cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			}
			return -2;
		}
	}
/*	if(m_sCommand == "")
	{
		TIMELOGEX(LOG_ERROR, "msCommandInitial", "CommandExec object will be create without command string", m_pSendLog);
		return -1;
	}*/
	return 0;
}

void CommandExec::setFor(const string& folder, const string& subroutine)
{
	m_sFolder= folder;
	m_sSubroutine= subroutine;
}

int CommandExec::command_exec(SHAREDPTR::shared_ptr<CommandExec> thread, string command, vector<string>& result,
				bool& more, bool wait, bool block, bool debug)
{
	int nRv(0);
	bool thwait;
	/**
	 * incoming more declare whether thread should set subroutine
	 * to 0 by not starting any command (when shell command defined as blocking)
	 */
	bool bSet0(more);
	bool bNoWaitResult(false);
	string folder, subroutine;
	vector<string>::size_type nLen;

	result.clear();
	more= false;
	if(	wait == false &&
		block == false &&
		debug == true		)
	{  // when wait and block is false but subroutine is in debugging mode
		wait= true; // threads are not classifiable for output
	}               // so subroutine have to wait always by debugging
	thwait= wait;
	if(block == true)
		thwait= false;	// routine should'nt wait for thread
						// now not the same as wait action
						// in subroutine

	LOCK(thread->m_WAITMUTEX);
	thread->m_bWait= wait;
	thread->m_bBlock= block;
	thread->m_bDebug= debug;
	UNLOCK(thread->m_WAITMUTEX);
	if(command == "info")
		return 0;
	if(	!thread->running() ||
		thread->wait()			)
	{
		// clear first by starting all old output
		if(block)
			result= thread->getOutput();
		else
			thread->getOutput();
		if(!thread->running())
		{
			//cout << "start thread for this command" << endl;
			if(thread->start() != 0)
			{
				ostringstream msg;

				if(	thread->getErrorType() == Thread::INIT &&
					thread->getErrorCode() == -2				)
				{
					msg << "ERROR: exception caused by writing command ";
					msg << "'" << command << "' into CommandExec object";
					cerr << msg.str() << endl;
					LOG(LOG_ERROR, msg.str());
					return -3;
				}
				msg << "ERROR: by starting unused CommandExec thread for shell command '" << command << "'\n";
				msg << "       ERRORCODE(" << thread->getErrorCode() << ")";
				cerr << msg.str() << endl;
				LOG(LOG_ERROR, msg.str());
				return -2;
			}
		}
		LOCK(thread->m_WAITMUTEX);
		thread->m_sCommand= command;
		AROUSEALL(thread->m_WAITFORRUNGCONDITION);// <- starting thread
		if(thwait)
		{// and wait for answer with the same condition
		 // when subroutine has action wait and no block
			while(	thread->m_sCommand != "" &&
					!thread->stopping()			)
			{
				CONDITION(thread->m_WAITFORRUNGCONDITION, thread->m_WAITMUTEX);
			}
		}
		UNLOCK(thread->m_WAITMUTEX);
		if(	wait == false ||
			block == true	)
		{
			if(block)
				more= true;
			return 0;
		}
	}else if(bSet0)
	{ // set subroutine to 0, because command do not start new
		ostringstream startCommand;
		ppi_time acttime;

		startCommand << "PPI-SET ";
		if(acttime.setActTime())
			startCommand << "-t " << acttime.toString(/*as date*/false) << " ";
		startCommand << thread->m_sFolder << ":" << thread->m_sSubroutine << " 0";
		thread->setValue(startCommand.str(), /*write error into output queue*/true);
	}
	if(!result.empty())
	{
		vector<string> newResult;

		newResult= thread->getOutput();
		result.push_back("");
		result.push_back("-----------------------------------------");
		result.push_back("     **  new starting of command  **");
		result.push_back("~~~                                   ~~~");
		result.push_back("");
		result.insert(result.end(), newResult.begin(), newResult.end());
	}else
		result= thread->getOutput();
	nLen= result.size();
	if(	block == true &&
		thread->running() &&
		!thread->wait() &&
		(	nLen == 0 ||						// when end of output has an ERRORLEVEL command
			result[nLen-1].length() <= 19 ||    // thread should stopping the next time
			result[nLen-1].substr(0, 19) != "PPI-DEF ERRORLEVEL "))
	{
		more= true;
	}else
		more= false;
	if(	more == false ||
		bNoWaitResult == true	)
	{
		if(nLen > 0)
		{
			if(	result[nLen-1].length() > 19 &&
				result[nLen-1].substr(0, 19) == "PPI-DEF ERRORLEVEL "	)
			{
				istringstream errorlevel(result[nLen-1].substr(19));

				more= false;
				errorlevel >> nRv;
				result.pop_back();
				if(	nLen > 1 &&
					result[nLen-2]	== "-"	)
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

int CommandExec::stop(const bool *bWait/*= NULL*/)
{
	bool bFound(false);
	string command;
	ostringstream ocommand;
	vector<string> spl;
	vector<string> result;
	vector<pid_t> vallPids;

	Thread::stop(false);
	LOCK(m_WAITMUTEX);
	if(m_nStopSignal > 0)
	{
		//cout << "stopping commands on bash with signal " << m_nStopSignal << endl;
		split(spl, m_sCommand, is_any_of(";|&"));
		if(m_tScriptPid > 0)
		{
			ostringstream opid;

			//cout << "stopping commands on bash with pid [" << m_tScriptPid << "]" << endl;
			result= grepPS(m_tScriptPid);
			for(vector<string>::iterator res= result.begin(); res != result.end(); ++res)
			{
				//cout << "search in result '" << *res << "' (" << res->length() << " chars)" << endl;
				for(vector<string>::iterator it= spl.begin(); it != spl.end(); ++it)
				{
					//cout << "      for '" << *it << "'" << endl;
					if(res->find(*it) != string::npos)
					{
						bFound= true;
						vallPids.push_back(m_tScriptPid);
						break;
					}
				}
				if(bFound)
					break;
			}
		}
		if(!bFound)
		{
			pid_t lastPid, curPid;
			vector<pid_t> vlastPid;

			lastPid= m_tOwnPid;
			curPid= lastPid;
			vlastPid= getChildProcess(curPid, &spl);
			if(vlastPid.size() > 1)
			{
				string msg;

				msg=  "### ERROR: by searching of started process from thread object CommandExec()\n";
				msg+= "           (for command: '" + m_sCommand + "')\n";
				msg+= "           found ambiguous process id's\n";
				msg+= "           so do not kill any process";
				msg+= "    WARNING: started command be unfortunately alive\n";
				msg+= "    ( please do not start the same command in more than one SHELL subroutine\n";
				msg+= "                                        maybe only with different parameters )";
				cerr << msg << endl;
				LOGEX(LOG_ERROR, msg, m_pSendLog);

			}else if(vlastPid.size() == 1)
			{
				vector<pid_t>::size_type n(0);

				bFound= true;
				vallPids.push_back(vlastPid[0]);
				while(n < vallPids.size())
				{// create list of all child processes
					vlastPid= getChildProcess(vallPids[n], NULL);
					if(!vlastPid.empty())
						vallPids.insert(vallPids.end(), vlastPid.begin(), vlastPid.end());
					++n;
				}
			}
		}
		if(bFound)
		{
			for(vector<pid_t>::iterator it= vallPids.begin(); it != vallPids.end(); ++it)
			{
				//cout << "kill pid " << *it << endl;
				kill(*it, m_nStopSignal);
			}
		}
		m_nStopSignal= 0;
		m_tScriptPid= 0;
	}
	AROUSEALL(m_WAITFORRUNGCONDITION);
	UNLOCK(m_WAITMUTEX);
	if(	bWait != NULL &&
		*bWait			)
	{
		return Thread::stop(bWait);
	}
	//cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	return 0;
}

vector<pid_t> CommandExec::getChildProcess(pid_t actPid, vector<string>* commands) const
{
	string user;
	pid_t pid, ppid;
	vector<string> result;
	vector<pid_t> vRv;

	do{
		if(vRv.size() > 1)
		{
			vector<string>::iterator itDel;

			cerr << "found ambigius processes" << endl;
			cerr << "search again for next command" << endl;
			itDel= commands->begin();
			commands->erase(itDel);
			vRv.clear();
			cerr << "-----------------------------------------------" << endl;
		}
		//cout << "search last pid for '" << m_sCommand << "'" << endl;
		result= grepPS(actPid);
		for(vector<string>::iterator it= result.begin(); it != result.end(); ++it)
		{
			istringstream output(*it);

			output >> user;
			output >> pid;
			output >> ppid;
			//cout << "ps >> " << *it;
			//cout << "   process id:" << pid << endl;
			//cout << "   parent id :" << ppid << endl;
			if(ppid == actPid)
			{
				if(commands != NULL)
				{
					for(vector<string>::const_iterator co= commands->begin(); co != commands->end(); ++co)
					{
						if(it->find(*co) != string::npos)
						{
							//cout << "implement this for kill" << endl;
							vRv.push_back(pid);
							break;
						}
					}
				}else
				{
					//cout << "implement this for kill" << endl;
					vRv.push_back(pid);
				}
			}
		}
	}while(	commands != NULL &&
			commands->size() > 1 &&
			vRv.size() > 1			);
	return vRv;
}

vector<string> CommandExec::grepPS(pid_t pid) const
{
	char line[1024];
	string sline;
	string grepStr;
	FILE *fp;
	vector<string> vsRv;
	ostringstream opid;

	opid << "grep ";
	opid << pid;
	grepStr= "ps -ef --columns 1023 | " + opid.str();
	fp= popen(grepStr.c_str(), "r");
	if(fp == NULL)
	{
		sline=  "ERROR by grep running process with line "+ grepStr + "\n";
		sline+= "         to secure stop process of shell command '" + m_sCommand + "'\n";
		sline+= "ERRNO: ";
		sline+=           *strerror(errno) + "\n";
		sline+= "       >> do not stop process !!! <<";
		cerr << sline << endl;
		LOGEX(LOG_ERROR, sline, m_pSendLog);
		return vsRv;
	};
	sline= "";
	while(fgets(line, sizeof(line), fp))
	{
		sline= line;
		if(sline.find(opid.str()) == string::npos)
			vsRv.push_back(sline);
	}
	pclose(fp);
	return vsRv;
}

bool CommandExec::wait()
{
	bool bWait(false);

	LOCK(m_WAITMUTEX);
	if(m_sCommand == "")
		bWait= true;
	UNLOCK(m_WAITMUTEX);
	return bWait;
}

int CommandExec::execute()
{
	bool bWaitMutex(false), bResultMutex(false), bOpenShell(false);
	bool bWait, bDebug, bCloseError(false);
	char line[1024];
	string sline;
	string command, orgCommand;
	string sLastErrorlevel;
	vector<string> spl, ispl;
	deque<string> qLastRows;
	string::size_type nLen;
	FILE *fp;
	map<string, double>::iterator found;

	LOCK(m_WAITMUTEX);
/*	if(	m_sCommand == "" &&
		m_sFolder == "power_switch" &&
		m_sSubroutine == "space")
	{
		cout << m_sFolder << ":" << m_sSubroutine << " go into wait status" << endl;
	}*/
	while(	m_sCommand == "" &&
			!stopping()			)
	{
		CONDITION(m_WAITFORRUNGCONDITION, m_WAITMUTEX);
	}
	orgCommand= m_sCommand;
	UNLOCK(m_WAITMUTEX);
	if(stopping())
		return 0;
	//cout << " --  start commandExec thread ----------------------------- " << endl;
	try{
		split(spl, orgCommand, is_any_of(";"));
		for(vector<string>::iterator it= spl.begin(); it != spl.end(); ++it)
		{
			split(ispl, *it, is_any_of("|")); // pipe
			for(vector<string>::iterator iit= ispl.begin(); iit != ispl.end(); ++iit)
			{
				command+= *iit;
				if(iit->find('>', 0) == string::npos)
					command+= " 2>&1";
				command+= " | ";
			}
			command= command.substr(0, command.length() - 3);
			command+= ";";
		}
		command+= "ERRORLEVEL=$?;echo;echo ERRORLEVEL $ERRORLEVEL";
		LOCK(m_WAITMUTEX);
		bWaitMutex= true;
		m_tOwnPid= getpid();
		m_nStopSignal= SIGTERM;
		m_tScriptPid= 0;
		UNLOCK(m_WAITMUTEX);
		bWaitMutex= false;
		fp= popen(command.c_str(), "r");
		bOpenShell= true;
		if(fp == NULL)
		{
			sline= "ERROR by writing command on folder subroutine " + orgCommand + " on command line\n";
			sline+= "ERRNO: " + *strerror(errno);
			cerr << sline << endl;
			LOGEX(LOG_ERROR, sline, m_pSendLog);
			generateError(sLastErrorlevel, qLastRows, bWait, bDebug);
			LOCK(m_WAITMUTEX);
			m_sCommand= "";
			UNLOCK(m_WAITMUTEX);
			return 0;
		};
		LOCK(m_WAITMUTEX);
		bWaitMutex= true;
		bWait= m_bWait;
		bDebug= m_bDebug;
		UNLOCK(m_WAITMUTEX);
		bWaitMutex= false;
		if(	!bWait && // when wait not be set and thread do not starting directly
			m_bInfo		) // and the script before running has the same ERRORLEVEL than this now will be have
		{
			ostringstream startCommand;// the subroutine will be not informed for an new value
			ppi_time acttime;

			startCommand << "PPI-SET ";
			if(acttime.setActTime())
				startCommand << "-t " << acttime.toString(/*as date*/false) << " ";
			startCommand << m_sFolder << ":" << m_sSubroutine << " ";
			LOCK(m_externWRITTENVALUES);
			startCommand << m_nStartCommand;
			UNLOCK(m_externWRITTENVALUES);
			setValue(startCommand.str(), bDebug);
		}
		while(fgets(line, sizeof line, fp))
		{
			if(stopping())
				break;
			sline+= line;
			//cout << "script>> " << sline << flush;
			nLen= sline.length();
			if(	nLen > 0 &&
				sline.substr(nLen-1) == "\n")
			{
				qLastRows.push_back(sline);
				if(qLastRows.size() > 10)
					qLastRows.pop_front();
				sline= sline.substr(0, nLen-1);
				if(	sline.length() >  11 &&
					sline.substr(0, 11) == "ERRORLEVEL "	)
				{// write always last error level, because when taken the error level from m_qOutput,
					sLastErrorlevel= sline; // it can be that queue was pulled before from starting thread
				}							// and queue will be empty -> no ERRORLEVEL
				LOCK(m_WAITMUTEX);
				bWaitMutex= true;
				bWait= m_bWait;
				bDebug= m_bDebug;
				UNLOCK(m_WAITMUTEX);
				bWaitMutex= false;
				readLine(bWait, bDebug, sline);
				sline= "";
			}
		}
		if(stopping())
		{
			pclose(fp);
			return 1;
		}
		LOCK(m_WAITMUTEX);
		bWaitMutex= true;
		m_nStopSignal= 0;
		m_tScriptPid= 0;
		UNLOCK(m_WAITMUTEX);
		bWaitMutex= false;
		if( pclose(fp) != 0 &&
			(	sLastErrorlevel == "ERRORLEVEL 0" ||
				sLastErrorlevel == ""				)	)
		{
			bCloseError= true;
		}
		bOpenShell= false;
		if(sline != "")
		{
			qLastRows.push_back(sline);
			if(qLastRows.size() > 10)
				qLastRows.pop_front();
			readLine(bWait, bDebug, sline);
		}
		if(	sLastErrorlevel != "" ||
			bCloseError				)
		{
			generateError(sLastErrorlevel, qLastRows, bWait, bDebug);
		}
	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("running shell command '" + orgCommand + "' for folder '"
						+ m_sFolder + "' and subroutine '" + m_sSubroutine + "'");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}
		if(bWaitMutex)
			UNLOCK(m_WAITMUTEX);
		if(bResultMutex)
			UNLOCK(m_RESULTMUTEX);
		if(bOpenShell)
			pclose(fp);
		generateError(sLastErrorlevel, qLastRows, bWait, bDebug);

	}catch(std::exception& ex)
	{
		string err;

		err=  "ERROR: STD exception by running shell command '" + orgCommand + "' for folder '"
						+ m_sFolder + "' and subroutine '" + m_sSubroutine + "'\n";
		err+= "what(): " + string(ex.what());
		cerr << err << endl;
		try{
			LOG(LOG_ALERT, err);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}
		if(bWaitMutex)
			UNLOCK(m_WAITMUTEX);
		if(bResultMutex)
			UNLOCK(m_RESULTMUTEX);
		if(bOpenShell)
			pclose(fp);
		generateError(sLastErrorlevel, qLastRows, bWait, bDebug);

	}catch(...)
	{
		string error;

		error+= "ERROR: catching UNKNOWN exception running shell command '" + orgCommand + "' for folder '"
						+ m_sFolder + "' and subroutine '" + m_sSubroutine + "'";
		cerr << error << endl;
		try{
			LOG(LOG_ALERT, error);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}
		if(bWaitMutex)
			UNLOCK(m_WAITMUTEX);
		if(bResultMutex)
			UNLOCK(m_RESULTMUTEX);
		if(bOpenShell)
			pclose(fp);
		generateError(sLastErrorlevel, qLastRows, bWait, bDebug);
	}
	LOCK(m_WAITMUTEX);
	m_sCommand= "";
	AROUSEALL(m_WAITFORRUNGCONDITION);
	UNLOCK(m_WAITMUTEX);
	return 0;
}

void CommandExec::generateError(const string& errorLevelString, const deque<string>& qLastErrRows, bool bWait, bool bDebug)
{
	int nRv;
	string sline;
	istringstream oline(errorLevelString);
	ostringstream iline;

	LOCK(m_RESULTMUTEX);
	if(!m_qOutput.empty())
	{
		sline= m_qOutput.back();
		if(	sline.length() >  12 &&
			sline.substr(0, 12) == "-ERRORLEVEL "	)
		{
			m_qOutput.pop_back();
		}
	}
	if(errorLevelString != "")
	{
		oline >> sline; // string of 'ERRORLEVEL' not needed
		oline >> nRv;
		if(	nRv == 0 )
		{
			nRv= -127;
		}else// all error levels be lower than 0
			nRv*= -1;
	}else
		nRv= -127;
	iline << "PPI-DEF ERRORLEVEL " << nRv;
	m_qOutput.push_back(iline.str());
	UNLOCK(m_RESULTMUTEX);
	if(	bWait == false &&		// when subroutine not waiting for result, send error value to subroutine
		nRv != 0			)	// 0 is no error so running command was send by starting of shell script
	{
		ostringstream setErrorlevel;
		ppi_time acttime;

		setErrorlevel << "PPI-SET ";
		if(acttime.setActTime())
			setErrorlevel << "-t " << acttime.toString(/*as date*/false) << " ";
		setErrorlevel << m_sFolder << ":" << m_sSubroutine << " " << nRv;
		setValue(setErrorlevel.str(), bDebug);
	}
	if(	m_bLogError &&
		nRv != 0		)
	{
		ostringstream msg;

		LOCK(m_WAITMUTEX);
		msg << "shell script \"" << m_sCommand << "\"" << endl;
		UNLOCK(m_WAITMUTEX);
		msg << "from folder '" << m_sFolder << "' and subroutine '" << m_sSubroutine << "'";
		msg << " ending with error " << nRv << endl << endl;
		if(!qLastErrRows.empty())
		{
			for(deque<string>::const_iterator it= qLastErrRows.begin(); it != qLastErrRows.end(); ++it)
				msg << "    " << *it;
		}else
			msg << "    no output from script";
		LOGEX(LOG_ERROR, msg.str(), m_pSendLog);
	}
	if(	bWait ||
		bDebug	)
	{
		LOCK(m_RESULTMUTEX);
		m_qOutput.push_back("PPI-DEF " + errorLevelString);
		UNLOCK(m_RESULTMUTEX);
	}
}

void CommandExec::readLine(const bool& bWait, const bool& bDebug, string sline)
{
	bool bFoundDefCommand(false);
	string command;

	if(	bWait ||
		bDebug	)
	{
		LOCK(m_RESULTMUTEX);				// add an '-' as workaround before all strings getting from shell command
		m_qOutput.push_back("-" + sline);	// because this time an null string makes communication problems
		if(m_qOutput.size() > 1000)			// and also when shell gives in the string only 'done' communication
			m_qOutput.pop_front();			// stops to early
		UNLOCK(m_RESULTMUTEX);
	}
	if(m_bLogging)
	{
		m_qLog.push_back(sline);
		if(m_qLog.size() > 1000)
			m_qLog.pop_front();
	}
	if(	sline.length() > 8 &&
		sline.substr(0, 8) == "PPI-DEF ")
	{
		pid_t pid;
		string msg;
		istringstream oline(sline);

		bFoundDefCommand= true;
		oline >> msg;// -> PPI-DEF string
		oline >> msg;
		if(msg == "running-process")
		{
			oline >> pid;
			if(	!oline.fail() &&
				pid > 1			)
			{
				LOCK(m_WAITMUTEX);
				m_tScriptPid= pid;
				UNLOCK(m_WAITMUTEX);
				command= "";
			}else
			{
				LOCK(m_WAITMUTEX);
				m_tScriptPid= 0;
				UNLOCK(m_WAITMUTEX);
				command= " ### WARNING: cannot recognize process-id as correct running process";
				TIMELOGEX(LOG_WARNING, "shell_stopsignal"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
								+ "\ncannot recognize shell-output as correct running process-id\noutput string '"
								+ sline + "'", m_pSendLog        );
			}

		}else if(msg == "stop")
		{
			bool bok= true;

			oline >> msg;
			if(!oline.fail())
			{
				trim(msg);
				LOCK(m_WAITMUTEX);
				if(msg == "SIGHUP")
					m_nStopSignal= SIGHUP;
				else if(msg == "SIGINT")
					m_nStopSignal= SIGINT;
				else if(msg == "SIGQUIT")
					m_nStopSignal= SIGQUIT;
				else if(msg == "SIGTRAP")
					m_nStopSignal= SIGTRAP;
				else if(msg == "SIGABRT")
					m_nStopSignal= SIGABRT;
				else if(msg == "SIGBUS")
					m_nStopSignal= SIGBUS;
				else if(msg == "SIGKILL")
					m_nStopSignal= SIGKILL;
				else if(msg == "SIGUSR1")
					m_nStopSignal= SIGUSR1;
				else if(msg == "SIGUSR2")
					m_nStopSignal= SIGUSR2;
				else if(msg == "SIGPIPE")
					m_nStopSignal= SIGPIPE;
				else if(msg == "SIGALRM")
					m_nStopSignal= SIGALRM;
				else if(msg == "SIGTERM")
					m_nStopSignal= SIGTERM;
				else if(msg == "SIGSTOP")
					m_nStopSignal= SIGSTOP;
				else
				{
					m_nStopSignal= 0;
					bok= false;
				}
				UNLOCK(m_WAITMUTEX);
			}else
				bok= false;

			if(bok == false)
			{
				command= " ### WARNING: cannot recognize output as stopping signal";
				TIMELOGEX(LOG_WARNING, "shell_stopsignal"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
								+ "\ncannot recognize shell-output as stopping signal\noutput string '"
								+ sline + "'", m_pSendLog        );
			}else
				command= "";

		}else if(msg == "cycle-begin")
		{
			LOCK(m_RESULTMUTEX);
			m_qOutput.clear();
			UNLOCK(m_RESULTMUTEX);
			command= "";
		}else if(msg == "log")
		{
			bool bOK(true);

			oline >> msg;
			if(msg =="DEBUG")
				m_nLogLevel= LOG_DEBUG;
			else if(msg =="INFO")
				m_nLogLevel= LOG_INFO;
			else if(msg =="WARNING")
				m_nLogLevel= LOG_WARNING;
			else if(msg =="ERROR")
				m_nLogLevel= LOG_ERROR;
			else if(msg =="ALERT")
				m_nLogLevel= LOG_ALERT;
			else if(msg == "end")
			{
				m_qLog.pop_back();// delete command string: PPI-DEF log end
				if(!m_qLog.empty())
				{
					msg= "";
					for(deque<string>::iterator it= m_qLog.begin(); it != m_qLog.end(); ++it)
						msg+= *it + "\n";
					trim(msg);
					//cout << "sending msg to logger: '" << msg << "'" << endl;
					LOGEX(m_nLogLevel, msg, m_pSendLog);
					m_qLog.clear();
				}
				m_bLogging= false;
				bOK= false;// only set false for do not reading after end command

			}else
			{
				command= "-  ### ERROR cannot recognize log level \"" + command + "\", so do not logging";
				bOK= false;
			}
			if(bOK)
			{
				oline >> msg;
				if(msg == "string:")
				{
					string::size_type npos;
					istringstream::pos_type pos;

					pos= oline.tellg();
					npos= static_cast<string::size_type>(pos);
					msg= oline.str();
					if(	pos > 0 &&
						msg.length() > npos)
					{
						msg= msg.substr(npos);
						LOGEX(m_nLogLevel, msg, m_pSendLog);
						command= "";
					}else
						command= "-  #ERROR: logging command has no string";

				}else if(msg == "begin")
				{
					m_bLogging= true;
					command= "";
				}else
					command= "-  ### ERROR cannot recognize command after log level";

			}

		}else if(msg == "noerrorlog")
		{
			m_bLogError= false;

		}else
		{
			command= " ### WARNING: cannot recognize PPI-DEF line as any correct definition";
			TIMELOGEX(LOG_WARNING, "shell_ppi-def"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
							+ "\ncannot recognize PPI-DEF shell-output as any correct definition\noutput string '"
							+ sline + "'", m_pSendLog );
		}
	}
	if(!bWait)
	{
		if(	sline.length() > 7 &&             // when wait flag is true
			sline.substr(0, 7) == "PPI-SET"	) // all PPI-SET will be do inside SHELL subroutine
		{
			bFoundDefCommand= true;
			setValue(sline, bDebug);
		}
	}
	if(command != "")
	{
		if(	bWait ||
			bDebug	)
		{
			LOCK(m_RESULTMUTEX);
			m_qOutput.push_back(command);
			if(m_qOutput.size() > 1000)
				m_qOutput.pop_front();
			UNLOCK(m_RESULTMUTEX);
		}
		if(m_bLogging)
		{
			m_qLog.push_back(command);
			if(m_qLog.size() > 1000)
				m_qLog.pop_front();
		}
		command= "";
	}
	if(	bFoundDefCommand == false &&
		(	bWait ||
			bDebug ||
			m_bLogging	)					)
	{
		trim(sline);
		if(sline.length() >= 7)
		{
			sline= sline.substr(0, 7);
			transform(sline.begin(), sline.end(), sline.begin(), (int(*)(int)) toupper);
			if(sline == "PPI-DEF")
			{
				command= "-  ### WARNING: should this line be an definition command for ppi-server?";
				if(	bWait ||
					bDebug	)
				{
					LOCK(m_RESULTMUTEX);
					m_qOutput.push_back(command);
					if(m_qOutput.size() > 1000)
						m_qOutput.pop_front();
				}
				if(m_bLogging)
				{
					m_qLog.push_back(command);
					if(m_qLog.size() > 1000)
						m_qLog.pop_front();
				}
				command= "-               command should be only >> PPI-DEF << in big letters with no spaces before";
				if(	bWait ||
					bDebug	)
				{
					m_qOutput.push_back(command);
					if(m_qOutput.size() > 1000)
						m_qOutput.pop_front();
					UNLOCK(m_RESULTMUTEX);
				}
				if(m_bLogging)
				{
					m_qLog.push_back(command);
					if(m_qLog.size() > 1000)
						m_qLog.pop_front();
				}
			}
		}
	}
}

void CommandExec::defSetError(const string& command, const string& msg)
{
	string errstr, logstr;

	errstr= "-  ### ERROR: cannot read correctly PPI-SET command";
	logstr+= errstr +"\n";
	LOCK(m_RESULTMUTEX);
	m_qOutput.push_back(errstr);
	if(m_qOutput.size() > 1000)
		m_qOutput.pop_front();
	UNLOCK(m_RESULTMUTEX);
	if(m_bLogging)
	{
		m_qLog.push_back(errstr);
		if(m_qLog.size() > 1000)
			m_qLog.pop_front();
	}
	errstr= "              for SHELL subroutine " + m_sFolder + ":" + m_sSubroutine;
	logstr+= errstr +"\n";
	LOCK(m_RESULTMUTEX);
	m_qOutput.push_back(errstr);
	if(m_qOutput.size() > 1000)
		m_qOutput.pop_front();
	UNLOCK(m_RESULTMUTEX);
	if(m_bLogging)
	{
		m_qLog.push_back(errstr);
		if(m_qLog.size() > 1000)
			m_qLog.pop_front();
	}
	LOCK(m_WAITMUTEX);
	errstr= m_sCommand;
	UNLOCK(m_WAITMUTEX);
	errstr= "              by command: '" + errstr + "'";
	logstr+= errstr +"\n";
	LOCK(m_RESULTMUTEX);
	m_qOutput.push_back(errstr);
	if(m_qOutput.size() > 1000)
		m_qOutput.pop_front();
	UNLOCK(m_RESULTMUTEX);
	if(m_bLogging)
	{
		m_qLog.push_back(errstr);
		if(m_qLog.size() > 1000)
			m_qLog.pop_front();
	}
	errstr= "           output string: '" + command + "'";
	logstr+= errstr +"\n";
	LOCK(m_RESULTMUTEX);
	m_qOutput.push_back(errstr);
	if(m_qOutput.size() > 1000)
		m_qOutput.pop_front();
	UNLOCK(m_RESULTMUTEX);
	if(m_bLogging)
	{
		m_qLog.push_back(errstr);
		if(m_qLog.size() > 1000)
			m_qLog.pop_front();
	}
	if(msg != "")
	{
		errstr= "                by ERROR:  " + msg;
		logstr+= errstr +"\n";
		LOCK(m_RESULTMUTEX);
		m_qOutput.push_back(errstr);
		if(m_qOutput.size() > 1000)
			m_qOutput.pop_front();
		UNLOCK(m_RESULTMUTEX);
		if(m_bLogging)
		{
			m_qLog.push_back(errstr);
			if(m_qLog.size() > 1000)
				m_qLog.pop_front();
		}

	}
	cerr << logstr << endl;
	TIMELOGEX(LOG_ERROR, "shell_setValue"+errstr+command, logstr, m_pSendLog);
}

bool CommandExec::setValue(const string& command, bool bLog)
{
	bool bwrite(false);
	string outstr;
	ValueHolder value;
	vector<string> spl;
	istringstream icommand(command);
	map<string, double>::iterator it;

	if(m_pPort == NULL)
	{
		defSetError(command, "no setValue interface be set");
		return false;
	}
	icommand >> outstr; // string of PPI-SET (not needed)
	if(	icommand.eof() ||
		icommand.fail() ||
		outstr != "PPI-SET"	)
	{
		defSetError(command, "cannot read correctly first word of command 'PPI-SET'");
		return false;
	}
	icommand >> outstr; // folder:subroutine string or time option -t
	if(	icommand.eof() ||
		icommand.fail()		)
	{
		defSetError(command, "cannot read definition of <folder>:<subroutine> or option -t");
		return false;
	}
	if(outstr == "-t")
	{
		int nLen;
		long int tv_nsec;// nanoseconds

		icommand >> value.lastChanging.tv_sec; // unixtime
		if(	icommand.eof() ||
			icommand.fail() ||
			value.lastChanging.tv_sec == 0	)
		{
			defSetError(command, "cannot read correctly seconds of unixtime after "
							"option -t before decimal point (when set)");
			return false;
		}
		icommand >> outstr;// microseconds, nanoseconds or folder:subroutine definition
		if(	icommand.eof() ||
			icommand.fail() ||
			outstr.length() == 0	)
		{
			defSetError(command, "cannot read string after seconds of unixtime");
			return false;
		}
		if(outstr[0] == '.')
		{
			istringstream nsec;
			stringstream snsec;

			if(outstr.length() > 1)
			{
				int n0Count(0);

				outstr= outstr.substr(1);
				nLen= outstr.length();
				while(outstr[n0Count] == '0')
					++n0Count;
				nsec.str(outstr);
				nsec >> tv_nsec;
				if(nsec.fail())
				{
					defSetError(command, "cannot read correctly milliseconds, microseconds or nanoseconds "
									"after seconds of unixtime");
					return false;
				}
				snsec << tv_nsec;
				snsec >> outstr;
				if(n0Count > 0)
					outstr.insert(0, n0Count, '0');
				if(static_cast<string::size_type>(nLen) != outstr.length())
				{
					defSetError(command, "cannot read correctly second string "
									"after seconds of unixtime");
					return false;
				}
				nLen= outstr.length() - 6;
				if(nLen > 0)
				{
					nLen= static_cast<int>(powf(10, static_cast<float>(nLen)));
					value.lastChanging.tv_usec= tv_nsec / nLen;
					/*cout << "define nanoseconds " << fixed << tv_nsec << " / " << nLen
									<< " = " << value.lastChanging.tv_usec << endl;*/

				}else if(nLen < 0)
				{
					nLen= static_cast<int>(powf(10, static_cast<float>(nLen * -1)));
					value.lastChanging.tv_usec= tv_nsec *  nLen;
					/*cout << "define nanoseconds " << fixed << tv_nsec << " * " << nLen
									<< "= " << value.lastChanging.tv_usec << endl;*/

				}else // tv_nsec has the same length
					value.lastChanging.tv_usec= tv_nsec;
			}
			icommand >> outstr; // folder:subroutine string or time option -t
			if(	icommand.eof() ||
				icommand.fail()		)
			{
				defSetError(command, "cannot read definition of <folder>:<subroutine> string");
				return false;
			}
		}

	}
	split(spl, outstr, is_any_of(":"));
	if(spl.size() != 2)
	{
		defSetError(command, "cannot read definition of <folder>:<subroutine>");
		return false;
	}
	icommand >> value.value;
	if(icommand.fail())
	{
		defSetError(command, "cannot read definition of value");
		return false;
	}
	if(m_bWriteChanged)
	{
		if(	m_sFolder != spl[0] ||		// when subroutine is own,
			m_sSubroutine != spl[1]	)	// write EERRORLEVEL every time again,
		{								// because by passing subroutine normally inside list,
			LOCK(m_externWRITTENVALUES);// list set to running value (1 for begincommand, 2 .. 3)
			it= m_msdWritten->find(outstr);
			if(	it == m_msdWritten->end() ||
				it->second != value.value			)
			{
				(*m_msdWritten)[outstr]= value.value;
				bwrite= true;
			}else
			{
				if(bLog)
				{
					ostringstream oValue;

					oValue << fixed << value.value;
					LOCK(m_RESULTMUTEX);
					outstr= "  ### do not write value " + oValue.str() + " into subroutine because value is same as before";
					m_qOutput.push_back("- " + outstr);
					if(m_qOutput.size() > 1000)
						m_qOutput.pop_front();
					UNLOCK(m_RESULTMUTEX);
				}
				if(m_bLogging)
				{
					m_qLog.push_back(outstr);
					if(m_qLog.size() > 1000)
						m_qLog.pop_front();
				}
			}
			UNLOCK(m_externWRITTENVALUES);

		}else // if(m_sFolder != spl[0] || m_sSubroutine != spl[1])
			bwrite= true;

	}else // if(m_bWriteChanged)
		bwrite= true;
	if(	bwrite &&
		!stopping()	)
	{
#ifdef __followSETbehaviorFrom
		if(	__followSETbehaviorFrom <= 0 &&
			(	(	string(__followSETbehaviorToFolder) == "" ||
					boost::regex_match(spl[0], m_oToFolderExp)		) &&
				(	string(__followSETbehaviorToSubroutine) == "" ||
					boost::regex_match(spl[1], m_oToSubExp)		)	)	)
		{
			cout << "[0] try to SET " << spl[0] << ":" << spl[1] << " " << fixed << value.value
								<< " on " << value.lastChanging.toString(true) << endl;
		}
#endif // __followSETbehaviorFrom
		if(!m_pPort->setValue(spl[0], spl[1], value, "SHELL-command_"+outstr))
		{
			defSetError(command, "cannot write correctly PPI-SET command over interface to folder-list");
			return false;
		}
	}
	return true;
}

void CommandExec::setWritten(map<string, double>* written, pthread_mutex_t* WRITTENVALUES, const short& nCommand)
{
	LOCK(WRITTENVALUES);
	m_msdWritten= written;
	m_externWRITTENVALUES= WRITTENVALUES;
	m_nStartCommand= nCommand;
	UNLOCK(WRITTENVALUES);
}

vector<string> CommandExec::getOutput()
{
	vector<string> vRv;

	LOCK(m_RESULTMUTEX);
	for(deque<string>::iterator it= m_qOutput.begin(); it != m_qOutput.end(); ++it)
		vRv.push_back(*it);
	m_qOutput.clear();
	UNLOCK(m_RESULTMUTEX);
	return vRv;
}
