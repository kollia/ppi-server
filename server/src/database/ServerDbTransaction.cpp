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

#include <boost/algorithm/string/split.hpp>

#include "../pattern/util/LogHolderPattern.h"

#include "../pattern/server/IServerPattern.h"

#include "../util/GlobalStaticMethods.h"

#include "../util/stream/OParameterStringStream.h"
#include "../util/stream/OMethodStringStream.h"

#include "../server/libs/client/ExternClientInputTemplate.h"

#include "logger/LogThread.h"

#include "ServerDbTransaction.h"
#include "DatabaseThread.h"
#include "DefaultChipConfigReader.h"


using namespace std;
using namespace util;
using namespace ports;
using namespace ppi_database;
using namespace design_pattern_world::server_pattern;
using namespace boost;

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
		DatabaseThread* dbTh= NULL;
		IPPIDatabasePattern* db= NULL;

		//cout << "work on command: " << method << endl;
		if(method == "isEntryChanged")
		{
			db= DatabaseThread::instance()->getDatabaseObj();
			descriptor.unlock();
			db->isEntryChanged();
			descriptor.lock();
			descriptor << "changed";

		}else if(method == "writeIntoDb")
		{
			string folder, subroutine;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> folder;
			object >> subroutine;
			db->writeIntoDb(folder, subroutine);

		}else if(method == "fillValue")
		{
			bool bNew;
			double value;
			vector<double> values;
			string folder, subroutine, identif;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> folder;
			object >> subroutine;
			object >> identif;
			object >> bNew;
			while(!object.empty())
			{
				object >> value;
				values.push_back(value);
				if(object.fail())
					break;	// an error is occured
							// take only this one fault value (=0)
							// maybe the value was no number for double (perhaps string or boolean)
			}
			db->fillValue(folder, subroutine, identif, values, bNew);

		}else if(method == "existEntry")
		{
			int number= 0;
			unsigned short nRv;
			string folder, subroutine, identif;

			db= DatabaseThread::instance()->getDatabaseObj();
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

		}else if(method == "debugSubroutine")
		{
			bool debug;
			string folder;
			string subroutine;
			OMethodStringStream command("debugSubroutine");

			object >> debug;
			object >> folder;
			object >> subroutine;
			command << debug;
			command << folder;
			if(subroutine != "")
				command << subroutine;
			descriptor.sendToOtherClient("ProcessChecker", command.str(), false, "");

		}else if(method == "clearFolderDebug")
		{
			descriptor.sendToOtherClient("ProcessChecker", "clearFolderDebug", false, "");

		}else if(method == "getActEntry")
		{
			int number= 0;
			auto_ptr<double> spdRv;
			string folder, subroutine, identif;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> folder;
			object >> subroutine;
			object >> identif;
			if(!object.empty())
				object >> number;
			spdRv= db->getActEntry(folder, subroutine, identif, number);

			if(spdRv.get() != NULL)
			{
				od << *spdRv;
				descriptor << od.str();
			}else
				descriptor << "NULL";

		}else if(method == "getNearest")
		{
			double value;
			string subroutine, definition;
			vector<convert_t> vtRv;

			db= DatabaseThread::instance()->getDatabaseObj();
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

			db= DatabaseThread::instance()->getDatabaseObj();
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

			db= DatabaseThread::instance()->getDatabaseObj();
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

			db= DatabaseThread::instance()->getDatabaseObj();
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

		}else if(method == "registerPortID")
		{
			string folder, subroutine, server, chip;

			object >> folder;
			object >> subroutine;
			object >> server;
			object >> chip;
			db= DatabaseThread::instance()->getDatabaseObj();
			db->useChip(folder, subroutine, server, chip);
			reader->registerSubroutine(subroutine, folder, server, chip);
			descriptor << "done";

		}else if(method == "registerSubroutine")
		{
			string subroutine, folder, server, chip;

			object >> subroutine;
			object >> folder;
			object >> server;
			object >> chip;
			reader->registerSubroutine(subroutine, folder, server, chip);

		}else if(method == "useChip")
		{
			string folder, subroutine, onServer, chip;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> folder;
			object >> subroutine;
			object >> onServer;
			object >> chip;
			db->useChip(folder, subroutine, onServer, chip);
			descriptor << "done";

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
			const chips_t* chip;
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
		}else if(method == "existOWServer")
		{
			string res;
			unsigned short max;
			unsigned short owServer;

			object >> owServer;
			res= descriptor.sendToOtherClient("ProcessChecker", "getOWMaxCount", true, "");

			istringstream smax(res);

			smax >> max;
			if(owServer > 0 && owServer <= max)
				descriptor << "true";
			else
				descriptor << "false";

		}else if(method == "getStatusInfo")
		{
			static bool error= false;
			static unsigned short nOWReader= 1;
			static unsigned short nMaxOWReader;
			static unsigned short step= 0;
			static vector<string> status;
			bool bsend;
			vector<string>::iterator pos;
			auto_ptr<ostringstream> send;
			auto_ptr<ostringstream> poOWReader;
			istringstream *piOWReader;
			string param, msg;

			object >> param;
			switch(step)
			{
			case 0: // get status info from main process ppi-server
				send= auto_ptr<ostringstream>(new ostringstream);
				(*send) << "getStatusInfo";
				if(param != "")
					(*send) << " \"" << param << "\"";
				msg= descriptor.sendToOtherClient("ProcessChecker", send->str(), true, "");
				if(msg != "done")
				{
					if(ExternClientInputTemplate::error(msg))
					{
						msg= "no communication to ppi-server " + msg;
						++step;
					}
					//cout << "send: " << msg << endl;
					descriptor << msg;
					descriptor.endl();
					break;
				}
				++step;
			case 1: // read status info from own process ppi-db-server
				msg= Thread::getStatusInfo(param);
				split(status, msg, is_any_of("\n"));
				++step;
			case 2: // send status info lines from own process ppi-db-server
				bsend= false;
				while(status.size() > 0)
				{
					pos= status.begin();
					if(*pos != "")
					{
						bsend= true;
						descriptor << *pos;
						descriptor.endl();
						//cout << "send: " << *pos << endl;
						status.erase(pos);
						break;
					}else
						status.erase(pos);
				}
				if(bsend)
					break;
				++step;
			case 3: // check how much one wire reader does exist
				msg= descriptor.sendToOtherClient("ProcessChecker", "getOWMaxCount", true, "");
				piOWReader= new istringstream(msg);
				*piOWReader >> nMaxOWReader;
				delete piOWReader;
				++step;
				error= false;
			case 4:// get status info from all one wire reader
				while(nOWReader <= nMaxOWReader)
				{
					send= auto_ptr<ostringstream>(new ostringstream);
					(*send) << "getStatusInfo";
					if(param != "")
						(*send) << " \"" << param << "\"";
					poOWReader= auto_ptr<ostringstream>(new ostringstream);
					(*poOWReader) << "OwServerQuestion-" << nOWReader;
					msg= descriptor.sendToOtherClient(poOWReader->str(), send->str(), true, "");
					if(ExternClientInputTemplate::error(msg))
						msg= "no communication to  " + poOWReader->str() + " " + msg;
					if(msg != "done")
					{
						if(ExternClientInputTemplate::error(msg))
						{
							msg= "no communication to ppi-log-client " + msg;
							++nOWReader;
						}
						//cout << "send: " << msg << endl;
						descriptor << msg;
						descriptor.endl();
						break;
					}
					++nOWReader;
				}
				if(nOWReader <= nMaxOWReader)
					break;
				//cout << "all be done" << endl;
				descriptor << "done";
				descriptor.endl();
				nOWReader= 1;
				step= 0;
				break;
			}
		}else if(method == "getOWDebugInfo")
		{
			unsigned short ow;
			ostringstream def;
			string command("getDebugInfo");
			string res;

			object >> ow;
			def << "OwServerQuestion-" << ow;
			res= descriptor.sendToOtherClient(def.str(), command, true, "");
			descriptor << res;

		}else if(method == "setOWDebug")
		{
			bool set;
			bool bDo= false;
			string res;
			unsigned short serverID;
			unsigned int connectionID;
			list<unsigned int>::iterator conn;
			map<unsigned short, list<unsigned int> >::iterator found;

			object >> serverID;
			object >> connectionID;
			object >> set;
			if(set)
			{
				bool bInsert= false;

				found= m_msiOpenedOWServer.find(serverID);
				if(found != m_msiOpenedOWServer.end())
				{
					conn= find(found->second.begin(), found->second.end(), connectionID);
					if(conn == found->second.end())
						bInsert= true;
				}else
					bInsert= true;
				if(bInsert)
				{
					m_msiOpenedOWServer[serverID].push_back(connectionID);
					bDo= true;
				}
			}else
			{
				found= m_msiOpenedOWServer.find(serverID);
				if(found != m_msiOpenedOWServer.end())
				{
					conn= find(found->second.begin(), found->second.end(), connectionID);
					if(conn != found->second.end())
						found->second.erase(conn);
					if(found->second.empty())
						bDo= true;
				}
			}
			if(bDo)
			{
				ostringstream server;
				ostringstream command;

				server << "OwServerQuestion-" << serverID;
				command << "setDebug " << set;
				res= descriptor.sendToOtherClient(server.str(), command.str(), false, "");
				descriptor << res;
			}else
				descriptor << "done";

		}else if(method == "clearOWDebug")
		{
			bool bSend= false;
			unsigned int connectionID;
			list<unsigned int>::iterator conn;

			object >> connectionID;
			for(map<unsigned short, list<unsigned int> >::iterator it= m_msiOpenedOWServer.begin(); it != m_msiOpenedOWServer.end(); ++it)
			{
				bSend= false;
				if(connectionID > 0)
				{
					conn= find(it->second.begin(), it->second.end(), connectionID);
					if(conn != it->second.end())
					{
						it->second.erase(conn);
						if(it->second.empty())
							bSend= true;
					}
				}else
					bSend= true;
				if(bSend)
				{
					ostringstream server;

					server << "OwServerQuestion-" << it->first;
					descriptor.sendToOtherClient(server.str(), "setDebug false", false, "");
					if(connectionID > 0)
						break;
				}
			}
			descriptor << "done";

		}else if(method == "changedChip")
		{
			bool device;
			double value;
			string onServer, chip;
			map<string, set<string> >* mvFSubs;
			OMethodStringStream command("changedChip");

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> onServer;
			object >> chip;
			object >> value;
			object >> device;
			mvFSubs= db->getSubroutines(onServer, chip);
			for(map<string, set<string> >::iterator fit= mvFSubs->begin(); fit != mvFSubs->end(); ++fit)
			{
				for(set<string>::iterator sit= fit->second.begin(); sit != fit->second.end(); ++sit)
				{
					//cout << "  on server '" << onServer << "'  " << fit->first << ":" << *sit <<
					//				" has new value " << value << " access " << boolalpha << device << endl;
					command << fit->first;
					command << *sit;
					command << value;
					command << device;
					command << onServer+" "+chip;
					descriptor.sendToOtherClient("ProcessChecker", command.str(), false, "");
				}
			}

			descriptor << "done";

		}else if(method == "checkUnused")
		{
			string res;
			unsigned short max;

			object >> max;
			for(unsigned short n= 1; n <= max; ++n)
			{
				ostringstream server;

				server << "OwServerQuestion-" << n;
				descriptor.sendToOtherClient(server.str(), "checkUnused", true, "");
			}
			descriptor << "done";

		}else if(method == "endOfInitialisation")
		{
			bool out;
			unsigned short max;
			OMethodStringStream command("endOfInitialisation");

			object >> max;
			object >> out;
			command << out;
			for(unsigned short n= 1; n <= max; ++n)
			{
				ostringstream server;

				server << "OwServerQuestion-" << n;
				descriptor.sendToOtherClient(server.str(), command.str(), true, "");
			}
			descriptor << "done";

		}else if(method == "log")
		{
			log_t log;

			object >> log.file;
			object >> log.line;
			object >> log.type;
			object >> log.message;
			object >> log.pid;
			object >> log.tid;
			object >> log.thread;
			object >> log.identif;
			m_pLogObject->log(log);
			descriptor << "done";

		}else if(method == "setThreadName")
		{
			string name;
			pthread_t id;

			object >> name;
			object >> id;
			m_pLogObject->setThreadName(name, id);
			descriptor << "done";

		}else if(method == "getThreadName")
		{
			string name;
			pthread_t id;

			object >> id;
			m_pLogObject->getThreadName(id);
			descriptor << name;

		}else if(method == "isDbLoaded")
		{
			dbTh= DatabaseThread::instance();
			if(dbTh->isDbLoaded())
				descriptor << "1";
			else
				descriptor << "0";
		}else if(method == "allOwreadersInitialed")
		{
			static unsigned short existClients(0);
			static unsigned short actClient(0);
			string res;

			if(existClients == 0)
			{
				existClients= getOwClientCount();
				actClient= 1;

			}
			if(actClient > existClients)
			{// last owreader was fault initialed
			 // so call ending of initialization
				descriptor << "done";
				existClients= 0;
			}
			if(existClients > 0)
			{
				do{
					ostringstream owclient;
					owclient << "OwServerQuestion-";
					owclient << actClient;
					res= descriptor.sendToOtherClient(owclient.str(), "finishedInitial", true, "");
					if(	res == "done" ||
						res == "false"	)
					{
						if(res == "false")
							descriptor << "false";
						++actClient;
					}else
						descriptor << res;

				}while(	res == "done" &&
						actClient <= existClients	);
				if(	actClient > existClients &&
					res != "false"				)
				{
					descriptor << "done";
					existClients= 0;
				}
			}

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
			//ILogPattern* log;

			dbTh= DatabaseThread::instance();
			if(stopdb == 0)
			{
				glob::stopMessage("ServerDbTransaction::transfer(): stop database thread");
				dbTh->stop(false);
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
					glob::stopMessage("ServerDbTransaction::transfer(): send stop message to owreader process " + owclient.str());
					descriptor.sendToOtherClient(owclient.str(), "stop-owclient", false, "");
					usleep(500000);
					client= getOwClientCount();
					client-= minus;
				}
				if(client == 0)
					++stopdb;
				++count;
				sRv= "stop one wire clients";
				break;

			case 2:
				glob::stopMessage("ServerDbTransaction::transfer(): send stop message to main process ProcessChecker");
				++stopdb;
			case 3:
				sRv= descriptor.sendToOtherClient("ProcessChecker", "stop-all", true, "");
				if(sRv == "done")
				{
					++stopdb;
					descriptor.sendToOtherClient("ProcessChecker", "OK", false, "");
					sRv= "stop measure threads";
				}
				break;

			case 4:
				glob::stopMessage("ServerDbTransaction::transfer(): wait for ending of database thread");
				sRv= "stop database";
				dbTh->stop(false);
				if(!dbTh->running())
					++stopdb;
				else
					usleep(500000);
				break;

			case 5:
#if 0
				glob::stopMessage("ServerDbTransaction::transfer(): send stop message to logging server");
				sRv= descriptor.sendToOtherClient("LogServer", "stop", true);
				if(sRv == "done")
				{
					++stopdb;
					descriptor.sendToOtherClient("LogServer", "stop-OK", false);
					sRv= "stop logging client";
					//log= LogInterface::instance();
					//delete log;

				}
				break;
#else
				++stopdb;
#endif
			case 6:
				glob::stopMessage("ServerDbTransaction::transfer(): all stopping be done, send finished to client");
				descriptor << "done";
				descriptor.endl();
				descriptor.flush();
				//usleep(500000);// waiting for ending connections from internet server
				server= descriptor.getServerObject();
				holder= server->getCommunicationFactory();
				++stopdb;

			case 7:
				descriptor.unlock();
				glob::stopMessage("ServerDbTransaction::transfer(): send stop message to exiting database communiction clients");
				while(!holder->stopCommunicationThreads(descriptor.getClientID(), /*wait*/true))
						glob::stopMessage("ServerDbTransaction::transfer(): send stop message to exiting database communiction clients");
				//LogInterface::instance()->closeSendConnection();
				glob::stopMessage("ServerDbTransaction::transfer(): send stop message to own ppi-db-server thread");
				server->stop(false);
				// toDo: server do not stop always correctly
#				//exit(EXIT_SUCCESS);
				glob::stopMessage("ServerDbTransaction::transfer(): stopping was performed, ending with no new transaction");
				return false;
				break;
			}
			descriptor << sRv;

		}else
		{
			// undefined command was sending
			LOG(LOG_WARNING, "get undefined question with method name '" + method + "'\n"
							"from client " + descriptor.getString("client"));
			descriptor << "ERROR 011";
		}
		descriptor.endl();
		descriptor.flush();
		//cout << "finish work on command: " << method << endl;
		return true;
	}

	void ServerDbTransaction::connectionEnding(const unsigned int ID, const string& process, const string& client)
	{
		IPPIDatabasePattern* db;

		if(process == "ppi-internet-server")
		{
			db= DatabaseThread::instance()->getDatabaseObj();
			db->arouseChangingPoolCondition();
		}
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
			if(	error >= (static_cast<int>(ServerMethodTransaction::getMaxErrorNums(false)) * -1) &&
				error <= static_cast<int>(ServerMethodTransaction::getMaxErrorNums(true))						)
			{
				str= ServerMethodTransaction::strerror(error);
			}
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
