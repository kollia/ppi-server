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

#include <cstdio>
#include <cstring>

#include "../pattern/util/LogHolderPattern.h"

#include "../util/exception.h"

#include "CommandExec.h"

using namespace boost;

int CommandExec::init(void* args)
{
	if(args != NULL)
	{
		try{
			m_sCommand= *static_cast<string*>(args);
		}catch(SignalException& ex)
		{
			string err;
			ostringstream msg;

			msg << "make static cast from void*(";
			msg << args << ") to string";
			ex.addMessage(msg.str());
			err= ex.getTraceString();
			cerr << endl << err << endl;
			LOG(LOG_ERROR, err);
			return -2;
		}
	}
	if(m_sCommand == "")
	{
		TIMELOG(LOG_ERROR, "msCommandInitial", "CommandExec object will be create without command string");
		return -1;
	}
	return 0;
}

int CommandExec::command_exec(SHAREDPTR::shared_ptr<CommandExec> thread, string command, vector<string>& result,
				bool& more, bool wait, bool block, bool debug)
{
	int nRv(0);
	bool thwait;
	bool bNoWaitResult(false);
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
	if(!thread->running())
	{
		result= thread->getOutput();
		if(result.size() == 0)
		{
			if(thread->start(&command, thwait) != 0)
			{
				if(	thread->getErrorType() == Thread::INIT &&
					thread->getErrorCode() == -2				)
				{
					string msg;

					msg= "ERROR: exception caused by writing command ";
					msg+= "'"+command+"' into CommandExec object";
					cerr << msg << endl;
					LOG(LOG_ERROR, msg);
					return -3;
				}
				return -2;
			}
			if(	wait == false ||
				block == true	)
			{
				if(block)
					more= true;
				return 0;
			}
		}else
			bNoWaitResult= true;
	}
	if(!bNoWaitResult)
		result= thread->getOutput();
	nLen= result.size();
	if(	block == true &&
		thread->running() &&
		(	nLen == 0 ||						// when end of output has an ERRORLEVEL command
			result[nLen-1].length() <= 19 ||    // thread should stopping the next time
			result[nLen-1].substr(0, 19) != "PPI-DEF ERRORLEVEL "))
	{
		more= true;
	}
	if(	more == false ||
		bNoWaitResult == true	)
	{
		if(nLen > 0)
		{
			if(	result[nLen-1].length() > 19 &&
				result[nLen-1].substr(0, 19) == "PPI-DEF ERRORLEVEL "	)
			{
				istringstream errorlevel(result[nLen-1].substr(19));

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
	LOCK(m_WAITMUTEX);
	if(m_nStopSignal > 0)
		kill(m_tScriptPid, m_nStopSignal);
	UNLOCK(m_WAITMUTEX);
	return Thread::stop(bWait);
}

int CommandExec::execute()
{
	bool bWait, bDebug;
	char line[130];
	string sline;
	string command;
	vector<string>::size_type nLen;
	FILE *fp;

	command+= m_sCommand;
	command+= ";ERRORLEVEL=$?; echo; echo PPI-DEF ERRORLEVEL $ERRORLEVEL";
	fp= popen(command.c_str(), "r");
	if(fp == NULL)
	{
		sline= "ERROR by writing command on folder subroutine " + m_sCommand + " on command line\n";
		sline+= "ERRNO" + *strerror(errno);
		LOG(LOG_ERROR, sline);
		return -1;
	};
	LOCK(m_WAITMUTEX);
	bWait= m_bWait;
	bDebug= m_bDebug;
	UNLOCK(m_WAITMUTEX);
	if(!bWait) // when wait not be set and thread do not starting the first time, the own subroutine can having
		setValue("PPI-SET " + m_sFolder + ":" + m_sSubroutine + " 0");// the ERRORLEVEL result from the last pass
	while(fgets(line, sizeof line, fp))                               // when now the script fail with the same ERRORLEVEL
	{																  // the subroutine will be not informed
		sline+= line;
		//cout << sline << flush;
		nLen= sline.length();
		if(	nLen > 0 &&
			sline.substr(nLen-1) == "\n")
		{
			sline= sline.substr(0, nLen-1);
			LOCK(m_WAITMUTEX);
			bWait= m_bWait;
			bDebug= m_bDebug;
			UNLOCK(m_WAITMUTEX);
			readLine(bWait, bDebug, sline);
			sline= "";
		}
		if(stopping())
			break;
	}
	pclose(fp);
	if(sline != "")
		readLine(bWait, bDebug, sline);
	LOCK(m_WAITMUTEX);
	m_nStopSignal= 0;
	UNLOCK(m_WAITMUTEX);
	return 1;
}

void CommandExec::readLine(const bool& bWait, const bool& bDebug, string sline)
{
	bool bFoundDefCommand(false);
	string command;

	if(	bWait ||
		bDebug	)
	{
		string insertLine(sline);

		LOCK(m_RESULTMUTEX);
		if(	insertLine.length() <= 19 ||
			insertLine.substr(0, 19) != "PPI-DEF ERRORLEVEL "	)
		{ // add an '-' as workaround before all strings getting from shell command
			insertLine= "-" + insertLine; // because this time an null string makes communication problems
		}                                 // and also when shell gives in the string only 'done' communication
										  // stops to early
		m_qOutput.push_back(insertLine);
		if(m_qOutput.size() > 1000)
			m_qOutput.pop_front();
		UNLOCK(m_RESULTMUTEX);
	}
	if(	sline.length() > 8 &&
		sline.substr(0, 8) == "PPI-DEF ")
	{
		pid_t pid;
		istringstream oline(sline);

		bFoundDefCommand= true;
		oline >> command;// -> PPI-DEF string
		oline >> command;
		if(command == "running-process")
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
				TIMELOG(LOG_WARNING, "shell_stopsignal"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
								+ "\ncannot recognize shell-output as correct running process-id\noutput string '" + sline + "'"        );
			}

		}else if(command == "stop")
		{
			bool bok= true;

			oline >> command;
			if(!oline.fail())
			{
				trim(command);
				LOCK(m_WAITMUTEX);
				if(command == "SIGHUP")
					m_nStopSignal= 1;
				else if(command == "SIGINT")
					m_nStopSignal= 2;
				else if(command == "SIGQUIT")
					m_nStopSignal= 3;
				else if(command == "SIGTRAP")
					m_nStopSignal= 5;
				else if(command == "SIGABRT")
					m_nStopSignal= 6;
				else if(command == "SIGBUS")
					m_nStopSignal= 7;
				else if(command == "SIGKILL")
					m_nStopSignal= 9;
				else if(command == "SIGUSR1")
					m_nStopSignal= 10;
				else if(command == "SIGUSR2")
					m_nStopSignal= 12;
				else if(command == "SIGPIPE")
					m_nStopSignal= 13;
				else if(command == "SIGALRM")
					m_nStopSignal= 14;
				else if(command == "SIGTERM")
					m_nStopSignal= 15;
				else if(command == "SIGSTOP")
					m_nStopSignal= 19;
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
				TIMELOG(LOG_WARNING, "shell_stopsignal"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
								+ "\ncannot recognize shell-output as stopping signal\noutput string '" + sline + "'"        );
			}else
				command= "";

		}else if(command == "cycle-begin")
		{
			LOCK(m_RESULTMUTEX);
			m_qOutput.clear();
			UNLOCK(m_RESULTMUTEX);
			command= "";

		}else if(command == "ERRORLEVEL")
		{
			int nRv;
			ostringstream setErrorlevel;

			command= "";
			if(bWait == false)
			{
				if(!bDebug)
				{
					LOCK(m_RESULTMUTEX);
					m_qOutput.push_back(sline);
					if(m_qOutput.size() > 1000)
						m_qOutput.pop_front();
					UNLOCK(m_RESULTMUTEX);
				}
				oline >> nRv;
				setErrorlevel << "PPI-SET " << m_sFolder << ":" << m_sSubroutine << " ";
				setErrorlevel << nRv;
				if(!setValue(setErrorlevel.str()))
				{
					command= "  ### ERROR: cannot write correctly PPI-SET command for result of subroutine";
					TIMELOG(LOG_WARNING, "shell_setValue"+m_sCommand+setErrorlevel.str(), "for SHELL subroutine "
									+ m_sFolder + ":" + m_sSubroutine
									+ "\nby command: " + command + "\nresult string '" + setErrorlevel.str()
									+ "'\n               ### ERROR: cannot write correctly PPI-SET command for result of subroutine" );
				}
			}
		}else
		{
			command= " ### WARNING: cannot recognize PPI-DEF line as any correct definition";
			TIMELOG(LOG_WARNING, "shell_ppi-def"+sline, "for SHELL command inside subroutine " + m_sFolder + ":" + m_sSubroutine
							+ "\ncannot recognize PPI-DEF shell-output as any correct definition\noutput string '" + sline + "'" );
		}
	}
	if(	!bWait &&
		!bDebug		)
	{
		ostringstream setErrorlevel;

		if(	sline.length() > 7 &&             // when wait or debug is true
			sline.substr(0, 7) == "PPI-SET"	) // all PPI-SET will be do inside SHELL subroutine
		{
			if(!setValue(sline))
			{
				command= " ### ERROR: cannot read correctly PPI-SET command";
				TIMELOG(LOG_WARNING, "shell_setValue"+m_sCommand+sline, "for SHELL subroutine " + m_sFolder + ":" + m_sSubroutine
								+ "\nby command: " + command + "\noutput string '" + sline
								+ "'\n               ### ERROR: cannot read correctly PPI-SET command"               );
			}
		}
	}
	if(command != "")
	{
		LOCK(m_RESULTMUTEX);
		m_qOutput.push_back(command);
		if(m_qOutput.size() > 1000)
			m_qOutput.pop_front();
		UNLOCK(m_RESULTMUTEX);
		command= "";
	}
	if(	bFoundDefCommand == false &&
		(	bWait ||
			bDebug	)					)
	{
		trim(sline);
		if(sline.length() >= 7)
		{
			sline= sline.substr(0, 7);
			transform(sline.begin(), sline.end(), sline.begin(), (int(*)(int)) toupper);
			if(sline == "PPI-DEF")
			{
				LOCK(m_RESULTMUTEX);
				m_qOutput.push_back("-  ### WARNING: should this line be an definition command for ppi-server?");
				if(m_qOutput.size() > 1000)
					m_qOutput.pop_front();
				m_qOutput.push_back("-               command should be only >> PPI-DEF << in big letters with no spaces before");
				if(m_qOutput.size() > 1000)
					m_qOutput.pop_front();
				UNLOCK(m_RESULTMUTEX);
			}
		}
	}
}

bool CommandExec::setValue(const string& command)
{
	bool bwrite(false);
	double value;
	string outstr;
	vector<string> spl;
	istringstream icommand(command);
	map<string, double>::iterator it;

	if(m_pPort == NULL)
		return false;
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
	LOCK(m_externWRITTENVALUES);
	it= m_msdWritten->find(outstr);
	if(	it == m_msdWritten->end() ||
		it->second != value			)
	{
		(*m_msdWritten)[outstr]= value;
		bwrite= true;
	}
	UNLOCK(m_externWRITTENVALUES);
	if(bwrite)
		m_pPort->setValue(spl[0], spl[1], value, "SHELL-command_"+outstr);
	return true;
}

void CommandExec::setWritten(map<string, double>* written, pthread_mutex_t* WRITTENVALUES)
{
	LOCK(WRITTENVALUES);
	m_msdWritten= written;
	m_externWRITTENVALUES= WRITTENVALUES;
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
