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

#include "../pattern/server/IServerPattern.h"

#include "../util/OParameterStringStream.h"
#include "../util/OMethodStringStream.h"

#include "../logger/lib/LogInterface.h"

#include "ServerDbTransaction.h"
#include "DatabaseThread.h"
#include "DefaultChipConfigReader.h"

using namespace std;
using namespace util;
using namespace logger;
using namespace ppi_database;
using namespace design_pattern_world::server_pattern;

namespace server
{
	ServerDbTransaction::ServerDbTransaction()
	:	m_nOwClients(0)
	{
		m_ONEWIRECLIENTSMUTEX= Thread::getMutex("ONEWIRECLIENTSMUTEX");
	}

	bool ServerDbTransaction::transfer(IFileDescriptorPattern& descriptor, IMethodStringStream& object)
	{
		string method(object.getMethodName());
		ostringstream od;
		DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();
		DatabaseThread *db= DatabaseThread::instance();

		//cout << "work on command: " << method << endl;
		if(method == "isEntryChanged")
		{
			descriptor.unlock();
			db->isEntryChanged();
			descriptor.lock();
			descriptor << "changed";

		}else if(method == "isDbLoaded")
		{
			if(db->isDbLoaded())
				descriptor << "1";
			else
				descriptor << "0";
		}else if(method == "writeIntoDb")
		{
			string folder, subroutine;

			object >> folder;
			object >> subroutine;
			db->writeIntoDb(folder, subroutine);

		}else if(method == "fillValue")
		{
			bool bNew;
			double value;
			vector<double> values;
			string folder, subroutine, identif;
			OMethodStringStream command("fillValue");

			object >> folder;
			command << folder;
			object >> subroutine;
			command << subroutine;
			object >> identif;
			object >> bNew;
			while(!object.empty())
			{
				object >> value;
				values.push_back(value);
				command << value;
			}
			db->fillValue(folder, subroutine, identif, values, bNew);

		}else if(method == "existEntry")
		{
			int number= 0;
			unsigned short nRv;
			string folder, subroutine, identif;

			object >> folder;
			object >> subroutine;
			object >> identif;
			if(!object.empty())
				object >> number;
			nRv= db->existEntry(folder, subroutine, identif, number);
			switch(nRv)
			{
			case 5:
				descriptor << "exist";
				break;
			case 4:
				descriptor << "noaccess";
				break;
			case 3:
				descriptor << "novalue";
				break;
			case 2:
				descriptor << "noidentif";
				break;
			case 1:
				descriptor << "nosubroutine";
				break;
			default: // inherit 0
				descriptor << "nofolder";
			}

		}else if(method == "getActEntry")
		{
			int number= 0;
			double* pdRv;
			string folder, subroutine, identif;

			object >> folder;
			object >> subroutine;
			object >> identif;
			if(!object.empty())
				object >> number;
			pdRv= db->getActEntry(folder, subroutine, identif, number);

			if(pdRv != NULL)
			{
				od << *pdRv;
				descriptor << od.str();
			}else
				descriptor << "NULL";

		}else if(method == "getNearest")
		{
			double value;
			string subroutine, definition;
			vector<convert_t> vtRv;

			object >> subroutine;
			object >> definition;
			object >> value;
			vtRv= db->getNearest(subroutine, definition, value);
			for(vector<convert_t>::iterator it= vtRv.begin(); it != vtRv.end(); ++it)
			{
				ostringstream parameters;

				parameters << it->be;
				parameters << it->bSetTime;
				parameters << it->nMikrosec;
				descriptor << parameters.str();
				descriptor.endl();
				descriptor.flush();
			}
			descriptor << "done";

		}else if(method == "needSubroutines")
		{
			bool bRv;
			string name;
			unsigned long connection;

			object >> connection;
			object >> name;
			bRv= db->needSubroutines(connection, name);
			if(bRv == true)
				descriptor << "true";
			else
				descriptor << "false";

		}else if(method == "getChangedEntrys")
		{
			unsigned long connection;
			vector<string> vsRv;

			object >> connection;
			vsRv= db->getChangedEntrys(connection);
			for(vector<string>::iterator it= vsRv.begin(); it != vsRv.end(); ++it)
			{
				descriptor << *it;
				descriptor.endl();
				descriptor.flush();
				descriptor >> method;
			}
			descriptor << "done";

		}else if(method == "changeNeededIds")
		{
			unsigned long oldId, newId;

			object >> oldId;
			object >> newId;
			db->changeNeededIds(oldId, newId);

		}else if(method == "chipsDefined")
		{
			bool defined;

			object >> defined;
			reader->chipsDefined(defined);

		}else if(method == "define")
		{
			string server, config;

			object >> server;
			object >> config;
			reader->define(server, config);

		}else if(method == "registerChip")
		{
			bool bFloat;
			bool *pbFloat= &bFloat;
			double dmin, dmax, dCache;
			double *pdmin= &dmin;
			double *pdmax= &dmax;
			double *pdCache= &dCache;
			string server, chip, pin, type, family;

			object >> server;
			object >> chip;
			object >> pin;
			object >> type;
			object >> family;
			object >> dmin;
			if(object.null())
				pdmin= NULL;
			object >> dmax;
			if(object.null())
				pdmax= NULL;
			object >> bFloat;
			if(object.null())
				pbFloat= NULL;
			object >> dCache;
			if(object.null())
				pdCache= NULL;
			reader->registerChip(server, chip, pin, type, family, pdmin, pdmax, pbFloat, pdCache);

		}else if(method == "registerSubroutine")
		{
			string subroutine, folder, server, chip;

			object >> subroutine;
			object >> folder;
			object >> server;
			object >> chip;
			reader->registerSubroutine(subroutine, folder, server, chip);

		}else if(method == "getRegisteredDefaultChipCache")
		{
			const double* cache;
			string server, chip;

			object >> server;
			object >> chip;
			cache= reader->getRegisteredDefaultChipCache(server, chip);
			if(cache != NULL)
			{
				ostringstream oRv;

				oRv << *cache;
				descriptor << oRv.str();
			}else
				descriptor << "NULL";

		}else if(method.substr(0, 24) == "getRegisteredDefaultChip")
		{
			bool bAll= false;
			double v;
			const DefaultChipConfigReader::chips_t* chip;
			vector<double> errorcodes;
			string server, family, type, schip;

			if(method.substr(24, 1) == "4")
				bAll= true;
			object >> server;
			if(bAll)
			{
				object >> family;
				object >> type;
			}
			object >> schip;
			if(bAll)
				chip= reader->getRegisteredDefaultChip(server, family, type, schip);
			else
				chip= reader->getRegisteredDefaultChip(server, schip);
			if(chip != NULL)
			{
				OParameterStringStream oRv;

				oRv << chip->server;
				oRv << chip->family;
				oRv << chip->type;
				oRv << chip->id;
				oRv << chip->pin;
				oRv << chip->dmin;
				oRv << chip->dmax;
				oRv << chip->bFloat;
				oRv << chip->dCache;
				oRv << chip->bWritable;
				errorcodes= chip->errorcode;
				for(vector<double>::iterator it= errorcodes.begin(); it != errorcodes.end(); ++it)
					oRv << *it;
				descriptor << oRv.str();
			}else
				descriptor << "NULL";

		}else if(method == "getDefaultCache")
		{
			bool bFloat;
			double min, max, dRv;
			string folder, subroutine;

			object >> min;
			object >> max;
			object >> bFloat;
			object >> folder;
			object >> subroutine;
			dRv= reader->getDefaultCache(min, max, bFloat, folder, subroutine);

			ostringstream oRv;

			oRv << dRv;
			descriptor << oRv.str();
		}else if(method == "stop-all")
		{
			string sRv;
			static short minus= 0;
			static short count= 0;
			static unsigned short oldclient= 0;
			static unsigned short stopdb= 0;
			unsigned short client;
			ostringstream owclient;
			IServerPattern* server;
			IClientHolderPattern* holder;
			LogInterface* log;

			if(stopdb == 0)
			{
				db->stop(false);
				++stopdb;
			}
			switch(stopdb)
			{
			case 1:

				client= getOwClientCount();
				client-= minus;
				if(oldclient != client)
					count= 0;
				else if(count > 5)
				{
					cerr << "### ERROR: cannot stop one wire client with ID " << client << endl;
					++minus;
					--client;
					count= 0;
				}
				if(client > 0)
				{
					oldclient= client;
					owclient << "OwServerQuestion-";
					owclient << client;
					descriptor.sendToOtherClient(owclient.str(), "stop-owclient", false);
					usleep(500000);
					client= getOwClientCount();
					client-= minus;
				}
				if(client == 0)
					++stopdb;
				++count;
				sRv= "one wire clients";
				break;

			case 2:
				sRv= descriptor.sendToOtherClient("ProcessChecker", "stop-all", true);
				if(sRv == "done")
				{
					++stopdb;
					descriptor.sendToOtherClient("ProcessChecker", "OK", false);
					sRv= "stop measure threads";
				}
				break;

			case 3:
				sRv= "stop database";
				if(!db->running())
					++stopdb;
				else
					usleep(500000);
				break;

			case 4:
				sRv= descriptor.sendToOtherClient("LogServer", "stop", true);
				if(sRv == "done")
				{
					++stopdb;
					descriptor.sendToOtherClient("LogServer", "stop-OK", false);
					sRv= "stop logging client";
					log= LogInterface::instance();
					//log->stop(false);
					delete log;

				}
				break;

			case 5:
				descriptor << "done";
				descriptor.endl();
				descriptor.flush();
				usleep(500000);// waiting for ending connections from internet server
				server= descriptor.getServerObject();
				holder= server->getCommunicationFactory();
				holder->stopCommunicationThreads(descriptor.getClientID(), /*wait*/false);
				//LogInterface::instance()->closeSendConnection();
				server->stop(false);
				// toDo: server do not stop always correctly
				exit(EXIT_SUCCESS);
				return false;
				break;
			}
			descriptor << sRv;

		}else
		{
			// undefined command was sending
			descriptor << "ERROR 011";
		}
		descriptor.endl();
		descriptor.flush();
		//cout << "finish work on command: " << method << endl;
		return true;
	}

	string ServerDbTransaction::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 0:
			str= "no error occured";
			break;
		case 11:
			str= "undefined command was send to database server";
			break;
		default:
			if(error >= (ServerMethodTransaction::getMaxErrorNums(false) * -1) && error <= ServerMethodTransaction::getMaxErrorNums(true))
				str= ServerMethodTransaction::strerror(error);
			if(error > 0)
				str= "Undefined error for transaction";
			else
				str= "Undefined warning for transaction";
			break;
		}
		return str;
	}

	inline unsigned int ServerDbTransaction::getMaxErrorNums(const bool byerror) const
	{
		if(byerror)
			return 15;
		return 0;
	}

	void ServerDbTransaction::allocateConnection(IFileDescriptorPattern& descriptor)
	{
		if(!descriptor.getBoolean("asker") && descriptor.getString("client").substr(0, 17) == "OwServerQuestion-")
		{
			LOCK(m_ONEWIRECLIENTSMUTEX);
			++m_nOwClients;
			UNLOCK(m_ONEWIRECLIENTSMUTEX);
		}
	}

	void ServerDbTransaction::dissolveConnection(IFileDescriptorPattern& descriptor)
	{
		if(!descriptor.getBoolean("asker") && descriptor.getString("client").substr(0, 17) == "OwServerQuestion-")
		{
			LOCK(m_ONEWIRECLIENTSMUTEX);
			--m_nOwClients;
			UNLOCK(m_ONEWIRECLIENTSMUTEX);
		}
	}

	unsigned short ServerDbTransaction::getOwClientCount()
	{
		unsigned short nRv;

		LOCK(m_ONEWIRECLIENTSMUTEX);
		nRv= m_nOwClients;
		UNLOCK(m_ONEWIRECLIENTSMUTEX);
		return nRv;
	}

	ServerDbTransaction::~ServerDbTransaction()
	{
		DESTROYMUTEX(m_ONEWIRECLIENTSMUTEX);
	}

}