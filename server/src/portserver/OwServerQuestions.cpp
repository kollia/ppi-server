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

#include "OwServerQuestions.h"

#include "../logger/lib/LogInterface.h"

#include "../util/IMethodStringStream.h"

namespace server {

using namespace logger;

int OwServerQuestions::init(void* args)
{
	return openGetConnection();
}

int OwServerQuestions::execute()
{
	int err;
	string question, command;

	question= getQuestion(m_sAnswer);
	err= error(question);
	if(err != 0)
	{
		if(err < 0 || err == 35)
		{ 	// by error number 35
			// no communication server is listening on port
			// or err is an warning (under 0)
			// try again later
			return 0;
		}
		command=  "### ERROR: ending OWServer question client for server " + m_oServer->getServerName() + "\n";
		command+= "           " + strerror(err);
		cerr << command << endl;
		LOG(LOG_ERROR, command);
		return err;
	}
	IMethodStringStream stream(question);

	command= stream.getMethodName();
	if(command == "exist")
	{
		m_vAnswer.push_back("true");
	}else if(command == "setDebug")
	{
		bool set;

		stream >> set;
		m_oServer->setDebug(set);
	}else if(command == "getDebugInfo")
	{
		vector<string>::size_type size= m_vAnswer.size();

		++m_nPos;
		if(size <= m_nPos)
		{
			m_vAnswer.clear();
			m_vAnswer= m_oServer->getDebugInfo();
			m_vAnswer.push_back("done");
			m_nPos= 0;
			m_sAnswer= m_vAnswer[m_nPos];
		}else
			m_sAnswer= m_vAnswer[m_nPos];
	}else if(command == "stop-owclient")
	{
		closeGetConnection();
		stop(false);
	}else
	{
		string msg("### ERROR: undefined command '");

		msg+= command + "' was send to one wire server";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		m_sAnswer= "ERROR 001";
	}
	return 0;
}

void OwServerQuestions::ending()
{
	closeGetConnection();
}

}
