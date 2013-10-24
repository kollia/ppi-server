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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		bool bDebugOutput(descriptor.getBoolean("output"));
		ostringstream debugSendMsg;

		debugSendMsg << descriptor.getString("process") << "::";
		debugSendMsg << descriptor.getString("client") << " ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'changed' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			descriptor << "changed";

		}else if(method == "writeIntoDb")
		{
			string folder, subroutine, identif;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> folder;
			object >> subroutine;
			object >> identif;
			db->writeIntoDb(folder, subroutine, identif);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			switch(nRv)
			{
			case 5:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "exist";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "exist";
				break;
			case 4:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "noaccess";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "noaccess";
				break;
			case 3:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "novalue";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "novalue";
				break;
			case 2:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "noidentif";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "noidentif";
				break;
			case 1:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "nosubroutine";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "nosubroutine";
				break;
			default: // inherit 0
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
					debugSendMsg << "nofolder";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "nofolder";
				break;
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
			IMethodStringStream sendMeth(command.str());

			sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
			{
				string::size_type nLen(0);

				nLen= descriptor.getString("process").size() + 3;
				nLen+= descriptor.getString("client").size() - 12;
				cout << descriptor.getString("process") << "::";
				cout << descriptor.getString("client") << " ";
				cout << "(1-2.) get question '" << method << "'" << endl;
				if(nLen > 0)
					cout << string("").append(nLen, ' ');
				cout << "and send as (2-1.) question '" << sendMeth.str(true)
								<<"' to other client ProcessChecker" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			descriptor.sendToOtherClient("ProcessChecker", sendMeth, false, "");
			if(object.getSyncID() > 0)
				sendMeth.removeSyncID();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send nothing back to client, was only to set inside ProcessChecker" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "clearFolderDebug")
		{
			IMethodStringStream sendMeth("clearFolderDebug");

			sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
			{
				string::size_type nLen(0);

				nLen= descriptor.getString("process").size() + 3;
				nLen+= descriptor.getString("client").size() - 12;
				cout << descriptor.getString("process") << "::";
				cout << descriptor.getString("client") << " ";
				cout << "(1-2.) get question '" << method << "'" << endl;
				if(nLen > 0)
					cout << string("").append(nLen, ' ');
				cout << "and send as (2-1.) question '" << sendMeth.str(true)
								<<"' to other client ProcessChecker" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			descriptor.sendToOtherClient("ProcessChecker", sendMeth, false, "");
			if(object.getSyncID() > 0)
				sendMeth.removeSyncID();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send nothing back to client, was only to set inside ProcessChecker" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '" << od.str() << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}else
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'NULL' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "NULL";
			}

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			for(vector<convert_t>::iterator it= vtRv.begin(); it != vtRv.end(); ++it)
			{
				ostringstream parameters;

				parameters << it->be;
				parameters << it->bSetTime;
				parameters << it->nMikrosec;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "'" << parameters.str() << "', ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << parameters.str();
				descriptor.endl();
				descriptor.flush();
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '" ;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			if(bRv == true)
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "true";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "true";
			}else
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "false";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "false";
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "getChangedEntrys")
		{
			unsigned long connection;
			vector<string> vsRv;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> connection;
			vsRv= db->getChangedEntrys(connection);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			for(vector<string>::iterator it= vsRv.begin(); it != vsRv.end(); ++it)
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "'" << *it << "', ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << *it;
				descriptor.endl();
				descriptor.flush();
				descriptor >> method;
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			descriptor << "done";

		}else if(method == "changeNeededIds")
		{
			unsigned long oldId, newId;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> oldId;
			object >> newId;
			db->changeNeededIds(oldId, newId);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "chipsDefined")
		{
			bool defined;

			object >> defined;
			reader->chipsDefined(defined);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "define")
		{
			string server, config;

			object >> server;
			object >> config;
			reader->define(server, config);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "registerSubroutine")
		{
			string subroutine, folder, server, chip;

			object >> subroutine;
			object >> folder;
			object >> server;
			object >> chip;
			reader->registerSubroutine(subroutine, folder, server, chip);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send nothing back to client, was only to set inside ppi-db-server" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "getRegisteredDefaultChipCache")
		{
			const double* cache;
			string server, chip;

			object >> server;
			object >> chip;
			cache= reader->getRegisteredDefaultChipCache(server, chip);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			if(cache != NULL)
			{
				ostringstream oRv;

				oRv << *cache;
				descriptor << oRv.str();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << oRv.str();
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}else
			{
				descriptor << "NULL";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "NULL";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method.substr(0, 24) == "getRegisteredDefaultChip")
		{
			bool bAll= false;
			const chips_t* chip;
			vector<double> errorcodes;
			string server, family, type, schip, pin;

			if(method.substr(24, 1) == "5")
				bAll= true;
			object >> server;
			if(bAll)
			{
				object >> family;
				object >> type;
			}
			object >> schip;
			if(bAll)
				chip= reader->getRegisteredDefaultChip(server, family, type, schip, pin);
			else
				chip= reader->getRegisteredDefaultChip(server, schip);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << oRv.str();
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}else
			{
				descriptor << "NULL";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "NULL";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '" << oRv.str() << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "existOWServer")
		{
			string res;
			unsigned short max;
			unsigned short owServer;
			IMethodStringStream ires;
			IMethodStringStream sendMeth("getOWMaxCount");

			sendMeth.createSyncID(object.getSyncID());
			object >> owServer;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
			{
				string::size_type nLen(0);

				nLen= descriptor.getString("process").size() + 3;
				nLen+= descriptor.getString("client").size() - 12;
				cout << descriptor.getString("process") << "::";
				cout << descriptor.getString("client") << " ";
				cout << "(1-2.) get question '" << method << "'" << endl;
				if(nLen > 0)
					cout << string("").append(nLen, ' ');
				cout << "and send as (2-1.) question '" << sendMeth.str(true)
								<<"' to other client ProcessChecker" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			ires= descriptor.sendToOtherClient("ProcessChecker", sendMeth, true, "");
			res= ires.str();
			if(object.getSyncID() > 0)
				sendMeth.removeSyncID();

			istringstream smax(res);

			smax >> max;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			if(owServer > 0 && owServer <= max)
			{
				descriptor << "true";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "true";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}else
			{
				descriptor << "false";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "false";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "getStatusInfo")
		{
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
			{
				send= auto_ptr<ostringstream>(new ostringstream);
				(*send) << "getStatusInfo";
				if(param != "")
					(*send) << " \"" << param << "\"";
				IMethodStringStream ires;
				IMethodStringStream sendMeth(send->str());

				sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
				{
					string::size_type nLen(0);

					nLen= descriptor.getString("process").size() + 3;
					nLen+= descriptor.getString("client").size() - 12;
					cout << descriptor.getString("process") << "::";
					cout << descriptor.getString("client") << " ";
					cout << "(1-2.) get question '" << method << "'" << endl;
					if(nLen > 0)
						cout << string("").append(nLen, ' ');
					cout << "and send as (2-1.) question '" << sendMeth.str(true)
									<<"' to other client ProcessChecker" << endl;
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				ires= descriptor.sendToOtherClient("ProcessChecker", sendMeth, true, "");
				msg= ires.str();
				trim(msg);
				if(object.getSyncID() > 0)
					sendMeth.removeSyncID();
				if(msg != "done")
				{
					if(ExternClientInputTemplate::error(msg))
					{
						msg= "no communication to ppi-server " + msg;
						++step;
					}
					//cout << "send: " << msg << endl;
					descriptor << msg;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << msg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					descriptor.endl();
					break;
				}
				++step;
			}
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << *pos << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
			{
				IMethodStringStream ires;
				IMethodStringStream sendMeth("getOWMaxCount");

				sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
				{
					string::size_type nLen(0);

					nLen= descriptor.getString("process").size() + 3;
					nLen+= descriptor.getString("client").size() - 12;
					cout << descriptor.getString("process") << "::";
					cout << descriptor.getString("client") << " ";
					cout << "(1-2.) get question '" << method << "'" << endl;
					if(nLen > 0)
						cout << string("").append(nLen, ' ');
					cout << "and send as (2-1.) question '" << sendMeth.str(true)
									<<"' to other client ProcessChecker" << endl;
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				ires= descriptor.sendToOtherClient("ProcessChecker", sendMeth, true, "");
				msg= ires.str();
				if(object.getSyncID() > 0)
					sendMeth.removeSyncID();
				piOWReader= new istringstream(msg);
				*piOWReader >> nMaxOWReader;
				delete piOWReader;
				++step;
			}
			case 4:// get status info from all one wire reader
				while(nOWReader <= nMaxOWReader)
				{
					IMethodStringStream ires;
					send= auto_ptr<ostringstream>(new ostringstream);
					(*send) << "getStatusInfo";
					if(param != "")
						(*send) << " \"" << param << "\"";
					poOWReader= auto_ptr<ostringstream>(new ostringstream);
					(*poOWReader) << "OwServerQuestion-" << nOWReader;
					IMethodStringStream sendMeth(send->str());

					sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client " << poOWReader->str() << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					ires= descriptor.sendToOtherClient(poOWReader->str(), sendMeth, true, "");
					msg= ires.str();
					trim(msg);
					if(object.getSyncID() > 0)
						sendMeth.removeSyncID();
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << msg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << od.str() << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				nOWReader= 1;
				step= 0;
				break;
			}
		}else if(method == "getOWDebugInfo")
		{
			unsigned short ow;
			ostringstream def;
			string res;
			IMethodStringStream ires;
			IMethodStringStream sendMeth("getDebugInfo");

			sendMeth.createSyncID(object.getSyncID());
			object >> ow;
			def << "OwServerQuestion-" << ow;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
			{
				string::size_type nLen(0);

				nLen= descriptor.getString("process").size() + 3;
				nLen+= descriptor.getString("client").size() - 12;
				cout << descriptor.getString("process") << "::";
				cout << descriptor.getString("client") << " ";
				cout << "(1-2.) get question '" << method << "'" << endl;
				if(nLen > 0)
					cout << string("").append(nLen, ' ');
				cout << "and send as (2-1.) question '" << sendMeth.str(true)
								<<"' to other client " << def.str() << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			ires= descriptor.sendToOtherClient(def.str(), sendMeth, true, "");
			res= ires.str();
			trim(res);
			if(object.getSyncID() > 0)
				sendMeth.removeSyncID();
			descriptor << res;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << od.str() << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
				OMethodStringStream sendMeth("setDebug");

				sendMeth.createSyncID(object.getSyncID());
				server << "OwServerQuestion-" << serverID;
				sendMeth << set;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
				{
					string::size_type nLen(0);

					nLen= descriptor.getString("process").size() + 3;
					nLen+= descriptor.getString("client").size() - 12;
					cout << descriptor.getString("process") << "::";
					cout << descriptor.getString("client") << " ";
					cout << "(1-2.) get question '" << method << "'" << endl;
					if(nLen > 0)
						cout << string("").append(nLen, ' ');
					cout << "and send as (2-1.) question '" << sendMeth.str(true)
									<<"' to other client " << server.str() << endl;
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				IMethodStringStream ires;
				IMethodStringStream method(sendMeth);

				ires= descriptor.sendToOtherClient(server.str(), method, false, "");
				res= ires.str();
				if(object.getSyncID() > 0)
					sendMeth.removeSyncID();
				descriptor << res;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << res << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}else
			{
				descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}

		}else if(method == "clearOWDebug")
		{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			bool bDo(false);
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
					IMethodStringStream sendMeth("setDebug false");

					sendMeth.createSyncID(object.getSyncID());
					server << "OwServerQuestion-" << it->first;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					bDo= true;
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client " << server.str() << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					descriptor.sendToOtherClient(server.str(), sendMeth, false, "");
					if(object.getSyncID() == 0)
						sendMeth.removeSyncID();
					if(connectionID > 0)
						break;
				}
			}
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
			{
				debugSendMsg << "(1-";
				if(bDo)
					debugSendMsg << "3";
				else
					debugSendMsg << "2";
				debugSendMsg << ".) send answer 'done' from ppi-db-server back to client" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
					OMethodStringStream sendMeth(fit->first);

					sendMeth << *sit;
					sendMeth << value;
					sendMeth << device;
					sendMeth << onServer+" "+chip;
					sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client ProcessChecker" << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					descriptor.sendToOtherClient("ProcessChecker", sendMeth, false, "");
					if(object.getSyncID() == 0)
						sendMeth.removeSyncID();
				}
			}

			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "checkUnused")
		{
			string res;
			unsigned short max;

			object >> max;
			for(unsigned short n= 1; n <= max; ++n)
			{
				ostringstream server;
				IMethodStringStream sendMeth("checkUnused");

				sendMeth.createSyncID(object.getSyncID());
				server << "OwServerQuestion-" << n;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client " << server.str() << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor.sendToOtherClient(server.str(), sendMeth, true, "");
				if(object.getSyncID() == 0)
					sendMeth.removeSyncID();
			}
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "endOfInitialisation")
		{
			bool out;
			unsigned short max;
			OMethodStringStream sendMeth("endOfInitialisation");

			object >> max;
			object >> out;
			sendMeth << out;
			for(unsigned short n= 1; n <= max; ++n)
			{
				ostringstream server;

				server << "OwServerQuestion-" << n;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
				{
					int nLen(0);

					nLen= descriptor.getString("process").size() + 3;
					nLen+= descriptor.getString("client").size() - 12;
					cout << descriptor.getString("process") << "::";
					cout << descriptor.getString("client") << " ";
					cout << "(1-2.) get question '" << method << "'" << endl;
					if(nLen > 0)
						cout << string("").append(nLen, ' ');
					cout << "and send as (2-1.) question '" << sendMeth.str(true)
									<<"' to other client " << server.str() << endl;
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				sendMeth.createSyncID(object.getSyncID());
				descriptor.sendToOtherClient(server.str(), sendMeth, true, "");
				if(object.getSyncID() == 0)
					sendMeth.removeSyncID();
			}
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
			object >> log.tmnow;
			m_pLogObject->log(log);
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "setThreadName")
		{
			string name;
			pthread_t id;

			object >> name;
			object >> id;
			m_pLogObject->setThreadName(name, id);
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "getThreadName")
		{
			string name;
			pthread_t id;

			object >> id;
			m_pLogObject->getThreadName(id);
			descriptor << name;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '" << name << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "isDbLoaded")
		{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			dbTh= DatabaseThread::instance();
			if(dbTh->isDbLoaded())
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "1";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "1";
			}else
			{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "0";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				descriptor << "0";
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "allOwreadersInitialed")
		{
			static unsigned short existClients(0);
			static unsigned short actClient(0);
			IMethodStringStream ires;
			string res;

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "'done', ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			}
			if(existClients > 0)
			{
				do{
					ostringstream owclient;
					IMethodStringStream sendMeth("finishedInitial");

					sendMeth.createSyncID(object.getSyncID());
					owclient << "OwServerQuestion-";
					owclient << actClient;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client " << owclient.str() << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					ires= descriptor.sendToOtherClient(owclient.str(), sendMeth, true, "");
					res= ires.str();
					trim(res);
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(2-4.) get answer '" << ires.str(true) << "' to send back as '";
						cout << res << "' in (1-3.)" << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					if(object.getSyncID() == 0)
						sendMeth.removeSyncID();
					if(	res == "done" ||
						res == "false"	)
					{
						if(res == "false")
						{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
							if(bDebugOutput)
								debugSendMsg << "'false', ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
							descriptor << "false";
						}
						++actClient;
					}else
					{
						descriptor << res;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
						if(bDebugOutput)
							debugSendMsg << "'" << res << "', ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					}

				}while(	res == "done" &&
						actClient <= existClients	);
				if(	actClient > existClients &&
					res != "false"				)
				{
					descriptor << "done";
					existClients= 0;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
						debugSendMsg << "'done' ";
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				}
			}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << " from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "isServerConfigured")
		{
			bool bConf;
			short nPercent;
			string sProcess;
			ostringstream result;

			db= DatabaseThread::instance()->getDatabaseObj();
			bConf= db->isServerConfigured(sProcess, nPercent);
			result << boolalpha << bConf << " ";
			result << sProcess << " " << nPercent;
			descriptor << result.str();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer '" << result.str() << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else if(method == "setServerConfigureStatus")
		{
			short nPercent;
			string sProcess;

			db= DatabaseThread::instance()->getDatabaseObj();
			object >> sProcess;
			object >> nPercent;
			db->setServerConfigureStatus(sProcess, nPercent);
			descriptor << "done";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'done' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

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
					IMethodStringStream sendMeth("stop-owclient");

					sendMeth.createSyncID(object.getSyncID());
					oldclient= client;
					owclient << "OwServerQuestion-";
					owclient << client;
					glob::stopMessage("ServerDbTransaction::transfer(): send stop message to owreader process " + owclient.str());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client " << owclient.str() << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					descriptor.sendToOtherClient(owclient.str(), sendMeth, false, "");
					if(object.getSyncID() == 0)
						sendMeth.removeSyncID();
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
			{
				IMethodStringStream ires;
				IMethodStringStream sendMeth("stop-all");

				sendMeth.createSyncID(object.getSyncID());
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(bDebugOutput)
				{
					string::size_type nLen(0);

					nLen= descriptor.getString("process").size() + 3;
					nLen+= descriptor.getString("client").size() - 12;
					cout << descriptor.getString("process") << "::";
					cout << descriptor.getString("client") << " ";
					cout << "(1-2.) get question '" << method << "'" << endl;
					if(nLen > 0)
						cout << string("").append(nLen, ' ');
					cout << "and send as (2-1.) question '" << sendMeth.str(true)
									<<"' to other client ProcessChecker" << endl;
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				ires= descriptor.sendToOtherClient("ProcessChecker", sendMeth, true, "");
				sRv= ires.str();
				trim(sRv);
				if(object.getSyncID() == 0)
					sendMeth.removeSyncID();
				if(sRv == "done")
				{
					IMethodStringStream sendMeth("OK");

					sendMeth.createSyncID(object.getSyncID());
					++stopdb;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(bDebugOutput)
					{
						string::size_type nLen(0);

						nLen= descriptor.getString("process").size() + 3;
						nLen+= descriptor.getString("client").size() - 12;
						cout << descriptor.getString("process") << "::";
						cout << descriptor.getString("client") << " ";
						cout << "(1-2.) get question '" << method << "'" << endl;
						if(nLen > 0)
							cout << string("").append(nLen, ' ');
						cout << "and send as (2-1.) question '" << sendMeth.str(true)
										<<"' to other client ProcessChecker" << endl;
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
					descriptor.sendToOtherClient("ProcessChecker", sendMeth, false, "");
					if(object.getSyncID() == 0)
						sendMeth.removeSyncID();
					sRv= "stop measure threads";
				}
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
				//exit(EXIT_SUCCESS);
				glob::stopMessage("ServerDbTransaction::transfer(): stopping was performed, ending with no new transaction");
				return false;
				break;
			}
			descriptor << sRv;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-3.) send answer '" << sRv << "' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		}else
		{
			// undefined command was sending
			LOG(LOG_WARNING, "get undefined question with method name '" + method + "'\n"
							"from client " + descriptor.getString("client"));
			descriptor << "ERROR 011";
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				debugSendMsg << "(1-2.) send answer 'ERROR 011' from ppi-db-server back to client" << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		}
		descriptor.endl();
		descriptor.flush();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(bDebugOutput)
				cout << debugSendMsg.str() << endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
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
