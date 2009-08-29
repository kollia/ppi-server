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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../pattern/server/IClientPattern.h"
#include "../pattern/server/IClientHolderPattern.h"
#include "../pattern/server/IServerPattern.h"

#include "../util/structures.h"

#include "../logger/LogThread.h"

#include "../ports/measureThread.h"

#include "../portserver/owserver.h"

#include "../util/XMLStartEndTagReader.h"
#include "../util/usermanagement.h"
#include "../util/configpropertycasher.h"

#include "../database/Database.h"

#include "ServerThread.h"
#include "ServerMethodTransaction.h"
#include "communicationthreadstarter.h"

using namespace std;
using namespace user;
using namespace util;
using namespace ppi_database;
using namespace design_pattern_world::server_pattern;
using namespace boost::algorithm;

namespace server
{
	bool ServerMethodTransaction::init(IFileDescriptorPattern& descriptor)
	{
		descriptor.setString("process", "");
		descriptor.setString("client", "");
		descriptor.setBoolean("asker", true);
		descriptor.setBoolean("access", false);
		descriptor.setBoolean("own", false);
		return true;
	}

	bool ServerMethodTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool bRun= true;
		bool bwait= true;
		int pos;
		string input, wasinput;
		string process;
		string client;
		IServerPattern* server= NULL;

		descriptor >> input;
		if(descriptor.eof())
		{
			client= descriptor.getString("client");
			input=  "ERROR: connection from " + client + " is broken\n";
			input+= "       so close connection";
			if(client == "LogServer")
			{
				cerr << input << endl;
				return false;
			}
			boost::algorithm::replace_all(input, "\n", "\\n");
			input= "LogServer false log 'SereverMethodTransaction.cpp' 93 5 \"" + input +"\"";
			bRun= false;
		}
		wasinput= input;
		//trim(input, is_any_of(" \t\r\n"));
		input= ConfigPropertyCasher::trim(input, " \t\r\n");
		//cout << "input string: '" << input << "' from " << descriptor.getString("client") << endl;
		if(descriptor.getBoolean("asker"))
		{
			pos= input.find(' ', 0);
			if(pos > 0)
			{
				client= input.substr(0, pos);
				input= input.substr(pos + 1);
				if(input.substr(0, 4) == "true")
				{
					bwait= true;
					input= input.substr(5);
				}else if(input.substr(0, 5) == "false")
				{
					bwait= false;
					input= input.substr(6);
				}
			/*	cout << "from client " << descriptor.getString("client");
				cout << " in process " << descriptor.getString("process");
				cout << " toProcess: " << process;
				cout << " command: " << input << endl;
				if(bwait)
					cout << "client wait for answer" << endl;
				else
					cout << "client does not wait for answer" << endl;*/
			}
		}//else
			//cout << descriptor.getString("client") << " wait for any questions" << endl;
		if(!descriptor.getBoolean("access"))
		{
			bool access= true;
			vector<string> SplitVec;

			if(client == "")
				access= false;
			if(access)
			{
				split(SplitVec, client, is_any_of(":"));
				if(SplitVec.size() == 2)
				{
					process= SplitVec[0];
					client= SplitVec[1];
					descriptor.setString("client", client);
					descriptor.setString("process", process);
					if(input == "GET")
						descriptor.setBoolean("asker", false);
					else if(input == "SEND")
						descriptor.setBoolean("asker", true);
					else
						access= false;
				}else
					access= false;
			}
			if(access == false)
			{
				descriptor << "ERROR 005\n";
				descriptor.flush();
				descriptor.setBoolean("asker", true);
				descriptor.setBoolean("access", false);
				descriptor.setBoolean("own", false);
				descriptor.setString("client", "");
				return false;
			}
			descriptor.setBoolean("access", true);
			if(client == descriptor.getServerObject()->getName())
				descriptor.setBoolean("own", true);
			descriptor << "done\n";
			descriptor.flush();
			cout << "allocate ";
			if(descriptor.getBoolean("asker"))
				cout << "sending ";
			else
				cout << "answer ";
			cout << "conection " << descriptor.getClientID();
			cout << " from client " << descriptor.getString("client");
			cout << " in process " << descriptor.getString("process") << endl;
			return true;
		}
		if(	client == ""
			&&
			input == "ending"			)
		{
			descriptor.setBoolean("asker", true);
			descriptor.setBoolean("access", false);
			descriptor.setBoolean("own", false);
			descriptor.setString("client", "");
			descriptor << "done\n";
			descriptor.flush();
			cout << "finish connection with ID " << descriptor.getClientID();
			cout << "  from client " << descriptor.getString("client");
			cout << " in process " << descriptor.getString("process") << endl;
			return false;
		}
		if(descriptor.getBoolean("own"))
		{
			bool used= false;

			if(input == "init")
			{
				descriptor << "done\n";
				used= true;
			}else if(input.substr(0, 6) == "status")
			{
				string param;
				string output;

				if(input.length() > 6)
					param= ConfigPropertyCasher::trim(input.substr(6));
				output= Thread::getStatusInfo(param);
				descriptor << output;
				descriptor << "true\n";
				used= true;

			}else if(input == "running")
			{
				descriptor << "true\n";
				used= true;
			}else if(input == "stopping\n")
			{
				server= descriptor.getServerObject();
				if(server->stopping())
					descriptor << "true\n";
				else
					descriptor << "false\n";
				used= true;
			}else if(input == "stop")
			{
				server= descriptor.getServerObject();
				server->getCommunicationFactory()->stopCommunicationThreads(/*wait*/true, descriptor.getString("client"));
				server->stop(/*wait*/false);
				descriptor << "done\n";
				descriptor.flush();
				return false;
			}else
			{
				descriptor << "ERROR 001\n";
				used= true;
			}
			if(used)
			{
				descriptor.flush();
				return true;
			}
		}else if(!descriptor.getBoolean("asker"))
		{
			descriptor.sendAnswer(input);
			//cout << "log server waits for any questions" << endl;
			input= descriptor.getOtherClientString(true);
			//cout << descriptor.getString("process") << "::" << descriptor.getString("client");
			//cout << " get question " << input << endl;
			if(	input == ""
				||
				input.substr(input.size() -1) != "\n"	)
			{
				input+= "\n";
			}
			descriptor << input;
			descriptor.flush();
		}else
		{
			if(input == "")
			{
				//cout << "get null command from client " << descriptor.getString("client");
				//cout << " in process " << descriptor.getString("process") << "  -------------------" << endl;
				//cout << wasinput << endl;
				descriptor << "WARNING 111";
				descriptor.endl();
				descriptor.flush();
				return true;
			}
			//cout << "send input '" << input <<"' to " << client << endl;
			input= descriptor.sendToOtherClient(client, input, bwait);
			//cout << "send answer '" << input << "' back to client" << endl;
			if(	input == ""
				||
				input.substr(input.size() -1) != "\n"	)
			{
				input+= "\n";
			}
			descriptor << input;
			descriptor.flush();
		}
		return bRun;
	}

	string ServerMethodTransaction::getTransactionName(const IFileDescriptorPattern& descriptor) const
	{
		return descriptor.getString("client");
	}

	bool ServerMethodTransaction::isClient(const IFileDescriptorPattern& descriptor, const string& definition) const
	{
		return (	descriptor.getBoolean("asker") == false
					&&
					descriptor.getString("client") == definition	) ? true : false;
	}

	string ServerMethodTransaction::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 0:
			str= "no error occurred";
			break;
		case 1:
			str= "ERROR: undefined command for own process";
			break;
		default:
			if(error > 0)
				str= "Undefined error for transaction";
			else
				str= "Undefined warning for transaction";
			break;
		}
		return str;
	}

	ServerMethodTransaction::~ServerMethodTransaction()
	{
	}

}
