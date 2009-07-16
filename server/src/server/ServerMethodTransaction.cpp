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

namespace server
{
	bool ServerMethodTransaction::init(IFileDescriptorPattern& descriptor)
	{
		descriptor.setString("process", "");
		descriptor.setBoolean("asker", true);
		descriptor.setBoolean("access", false);
		descriptor.setBoolean("own", false);
		return true;
	}

	bool ServerMethodTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		int pos;
		string input;
		string process;
		IServerPattern* server= NULL;

		descriptor >> input;
		input= ConfigPropertyCasher::trim(input, " \t\r\n");
		cout << input << endl;
		pos= input.find(' ', 0);
		if(pos > 0)
		{
			process= input.substr(0, pos);
			input= input.substr(pos + 1);
		}
		if(!descriptor.getBoolean("access"))
		{
			if(input == "GET")
				descriptor.setBoolean("asker", false);
			else if(input == "SEND")
				descriptor.setBoolean("asker", true);
			else
			{
				descriptor << "ERROR 005\n";
				descriptor.flush();
				descriptor.setBoolean("asker", true);
				descriptor.setBoolean("access", false);
				descriptor.setBoolean("own", false);
				descriptor.setString("process", "");
				return false;
			}
			descriptor.setBoolean("access", true);
			descriptor.setString("process", process);
			if(process == descriptor.getServerObject()->getName())
				descriptor.setBoolean("own", true);
			descriptor << "done\n";
			descriptor.flush();
			return true;
		}
		if(	process == ""
			&&
			input == "ending"			)
		{
			descriptor.setBoolean("asker", true);
			descriptor.setBoolean("access", false);
			descriptor.setBoolean("own", false);
			descriptor.setString("process", "");
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
			}if(input == "stop")
			{
				server= descriptor.getServerObject();
				server->getCommunicationFactory()->stopCommunicationThreads(/*wait*/true, descriptor.getString("process"));
				server->stop(/*wait*/false);
				descriptor << "done\n";
				descriptor.flush();
				return false;
			}
			if(used)
			{
				descriptor.flush();
				return true;
			}
		}
		input= descriptor.sendToOtherClient(process, input);
		if(input.substr(input.size() -1) != "\n")
			input+= "\n";
		descriptor << input;
		descriptor.flush();
		return true;
	}

	string ServerMethodTransaction::getTransactionName(const IFileDescriptorPattern& descriptor) const
	{
		return descriptor.getString("process");
	}

	bool ServerMethodTransaction::isClient(const IFileDescriptorPattern& descriptor, const string& definition) const
	{
		return (	descriptor.getBoolean("asker") == false
					&&
					descriptor.getString("process") == definition	) ? true : false;
	}

	ServerMethodTransaction::~ServerMethodTransaction()
	{
	}

}
