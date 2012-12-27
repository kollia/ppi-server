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

#include "OwServerQuestions.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../util/stream/IMethodStringStream.h"

#include "../util/properties/configpropertycasher.h"

namespace server {

using namespace boost;

int OwServerQuestions::init(void* args)
{
	return openGetConnection();
}

int OwServerQuestions::execute()
{
	int err;
	string question, command;

	question= getQuestion(m_sAnswer);
	m_sAnswer= "";
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
		command=  "### ERROR: ending OWServer question client for server " + m_oServer->getServerDescription() + "\n";
		command+= "           " + strerror(err);
		cerr << command << endl;
		LOG(LOG_ERROR, command);
		return err;
	}
	IMethodStringStream stream(question);

	command= stream.getMethodName();
	if(command == "write")
	{
		bool bRv;
		string chip, addinfo;
		double value;

		stream >> chip;
		stream >> value;
		stream >> addinfo;
		bRv= m_oServer->write(chip, value, addinfo);
		if(bRv)
			m_sAnswer= "true";
		else
			m_sAnswer= "false";

	}else if(command == "read")
	{
		bool bRv;
		string chip;
		double value;
		OParameterStringStream answer;

		stream >> chip;
		stream >> value;
		bRv= m_oServer->read(chip, &value);
		answer << bRv;
		answer << value;
		m_sAnswer= answer.str();

	}else if(command == "command_exec")
	{
		vector<string>::size_type size= m_vAnswer.size();
		bool more;
		int nRv;
		string command;
		ostringstream morecontent;
		ostringstream errorlevel;

		++m_nPos;
		if(size <= m_nPos)
		{
			m_vAnswer.clear();
			stream >> command;
			nRv= m_oServer->command_exec(command, m_vAnswer, more);
			morecontent << "MORECONTENT ";
			morecontent << boolalpha << more;
			m_vAnswer.push_back(morecontent.str());
			errorlevel << "ERRORLEVEL ";
			errorlevel << nRv;
			m_vAnswer.push_back(errorlevel.str());
			m_vAnswer.push_back("done");
			m_nPos= 0;
			//for(vector<string>::iterator it= m_vAnswer.begin(); it != m_vAnswer.end(); ++it)
			//	cout << "want send >> " << *it << endl;
			m_sAnswer= m_vAnswer[m_nPos];
		}else
			m_sAnswer= m_vAnswer[m_nPos];
		//cout << "do send >> " << m_sAnswer << endl;

	}else if(command == "exist")
	{
		m_vAnswer.push_back("true");

	}else if(command == "getServerDescription")
	{
		m_sAnswer= m_oServer->getServerDescription();

	}else if(command == "endOfInitialisation")
	{
		bool out;

		stream >> out;
		m_oServer->endOfInitialisation(out);
		m_sAnswer= "done";

	}else if(command == "checkUnused")
	{
		m_oServer->checkUnused();
		m_sAnswer= "done";

	}else if(command == "isServer")
	{
		bool bIs;
		string type, chipID;

		stream >> type;
		stream >> chipID;
		bIs= m_oServer->isServer(type, chipID);
		if(bIs)
			m_sAnswer= "true";
		else
			m_sAnswer= "false";

	}else if(command == "getChipType")
	{
		string chip;

		stream >> chip;
		m_sAnswer= m_oServer->getChipType(chip);

	}/*else if(command == "getChipFamily")
	{
		string chip, family;

		stream >> chip;
		family= m_oServer->getChipFamily(chip);
		m_vAnswer.push_back(family);

	}*/else if(command == "getChipIDs")
	{
		m_vAnswer= m_oServer->getChipIDs();

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

	}else if(command == "usePropActions")
	{
		string prop;
		ConfigPropertyCasher properties;

		stream >> prop;
		properties.tag(prop);
		m_oServer->usePropActions(&properties);
		m_sAnswer= properties.pulled();

	}else if(command == "range")
	{
		bool bFloat;
		double min, max;
		string pin, res;
		OParameterStringStream answer;

		stream >> pin;
		m_oServer->range(pin, min, max, bFloat);
		answer << min;
		answer << max;
		answer << bFloat;
		m_sAnswer= answer.str();

	}else if(command == "haveToBeRegistered")
	{
		bool bRv;
		string res;

		bRv= m_oServer->haveToBeRegistered();
		if(bRv)
			m_sAnswer= "true";
		else
			m_sAnswer= "false";

	}else if(command == "useChip")
	{
		short res;
		ConfigPropertyCasher properties;
		string prop, unique, folder, subroutine;
		OParameterStringStream answer;

		stream >> prop;
		properties.tag(prop);
		stream >> unique;
		stream >> folder;
		stream >> subroutine;
		res= m_oServer->useChip(&properties, unique, folder, subroutine);
		answer << res;
		answer << properties.pulled();
		answer << unique;
		prop= answer.str();
		m_sAnswer= prop;

	}else if(command == "hasUnusedIDs")
	{
		bool bRv;

		bRv= m_oServer->hasUnusedIDs();
		if(bRv)
			m_sAnswer= "true";
		else
			m_sAnswer= "false";

	}else if(command == "reachAllChips")
	{
		bool bRv;

		bRv= m_oServer->reachAllChips();
		if(bRv)
			m_sAnswer= "true";
		else
			m_sAnswer= "false";

	}else if(command == "init")
	{
		m_sAnswer= "done";

	}else if(command == "finishedInitial")
	{
		static bool first(true);

		if(	!m_oServer->running() ||
			!m_oServer->initialed()	)
		{
			if(	m_oServer->stopping() &&
				first == false			) 	// when error in initialization known by first call
			{								// send back as first answer name of server
				m_sAnswer= "false";
			}else
				m_sAnswer= m_oServer->getServerDescription();
		}else
			m_sAnswer= "done";
		first= false;

	}else if(command == "getMinMaxErrorNums")
	{
		int nums;
		ostringstream errornums;

		nums= getMaxErrorNums(false);
		errornums << nums << " ";
		nums= getMaxErrorNums(true);
		nums+= 1; // for undefined command sending;
		errornums << nums;
		m_sAnswer= errornums.str();

	}else if(command == "stop-owclient")
	{
		closeGetConnection();
		stop(false);
	}else if(command == "getStatusInfo")
	{
		static unsigned short step= 0;
		static vector<string> status;
		vector<string>::iterator pos;
		string param, msg;

		stream >> param;
		switch(step)
		{
		case 0:
			msg= Thread::getStatusInfo(param);
			split(status, msg, is_any_of("\n"));
			pos= status.begin();
			m_sAnswer= *pos;
			//cout << "Log send: " << *pos << endl;
			status.erase(pos);
			++step;
			break;
		case 1:
			if(status.size() > 0)
			{
				do{
					pos= status.begin();
					msg= *pos;
					m_sAnswer= *pos;
					//cout << "Log send: " << *pos << endl;
					status.erase(pos);

				}while(	msg == "" &&
						!status.empty()	);
				if(msg != "")
					break;
			}
			//cout << "Log all be done" << endl;
			m_sAnswer= "done\n";
			step= 0;
			break;
		}

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
