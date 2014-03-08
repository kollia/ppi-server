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
#include <iostream>
#include <string.h>

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../pattern/util/LogHolderPattern.h"

#include "../pattern/server/IServerCommunicationStarterPattern.h"

#include "../server/libs/client/SocketClientConnection.h"

#include "../util/debugtransaction.h"
#include "../util/structures.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/XMLStartEndTagReader.h"
#include "../util/usermanagement.h"
#include "../util/URL.h"

#include "../util/stream/ppivalues.h"

#include "../util/properties/configpropertycasher.h"

#include "../database/logger/lib/logstructures.h"

#include "../database/lib/DbInterface.h"
#include "../database/lib/NeedDbChanges.h"

#include "libs/server/ServerThread.h"
#include "libs/server/communicationthreadstarter.h"

#include "ServerTransaction.h"

extern string global_clientpath;

using namespace std;
using namespace user;
using namespace util;
using namespace server;
using namespace ppi_database;
using namespace design_pattern_world::server_pattern;
using namespace boost;

//ServerThread* gInternetServer= NULL;

namespace server
{
	ServerTransaction::ServerTransaction(const uid_t uid)
	:	m_uid(uid),
	 	m_bFinished(false),
	 	m_bStopServer(false),
	 	m_fProtocol(0)
	{
		m_FINISHEDSERVERMUTEX= Thread::getMutex("FINISHEDSERVERMUTEX");
		m_SERVERISSTOPPINGMUTEX= Thread::getMutex("SERVERISSTOPPINGMUTEX");
	}

	bool ServerTransaction::init(IFileDescriptorPattern& descriptor)
	{
		descriptor.setString("username", "");
		descriptor.setBoolean("speaker", false);
		descriptor.setBoolean("access", false);
		descriptor.setBoolean("wait", false);
		descriptor.setUShort("actualized", 0);
		descriptor.setBoolean("readdebuginfo", false);
		descriptor.setBoolean("nextconnection", true);
		descriptor.setBoolean("finishedloading", false);
		return true;
	}

	bool ServerTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool hold;
		bool bServerStops;

		if(descriptor.getBoolean("nextconnection"))
		{
			ostringstream omsg;

			omsg << "server get new connection from host '" << descriptor.getHostAddressName() << "' ";
			omsg << " ID:" << descriptor.getClientID();
			LOG(LOG_SERVERINFO, omsg.str());
#ifdef SERVERDEBUG
			cout << omsg << endl;
#endif
			descriptor.setBoolean("nextconnection", false);
		}
		LOCK(m_SERVERISSTOPPINGMUTEX);
		bServerStops= m_bStopServer;
		UNLOCK(m_SERVERISSTOPPINGMUTEX);
		if(bServerStops)
		{
			string in;

			descriptor >> in;
			if(!descriptor.eof())
			{
				descriptor << "ERROR 019";
				descriptor.endl();
				descriptor.flush();
			}
			hold= false;

		}else if(	descriptor.getBoolean("access")
					&&
					descriptor.getBoolean("speaker")	)
		{
			hold= hearingPort(descriptor);
			if(!hold) // wait for next connection
				descriptor.setBoolean("nextconnection", true);

		}else
		{
			hold= clientCommands(descriptor);
			if(!hold) // wait for next connection
				descriptor.setBoolean("nextconnection", true);
		}

		return hold;
	}

	bool ServerTransaction::hearingPort(IFileDescriptorPattern& descriptor)
	{
		NeedDbChanges* changes= NeedDbChanges::instance();
		DbInterface* db= DbInterface::instance();
		vector<string> messages, debuginfo;
		string sendmsg, msg;
		unsigned short actualized= descriptor.getUShort("actualized");

		if(descriptor.getBoolean("readdebuginfo") == false)
		{
			actualized= changes->isEntryChanged(actualized);
			descriptor.setUShort("actualized", actualized);
		}
		messages= db->getChangedEntrys(descriptor.getClientID());
		for(vector<string>::iterator i= messages.begin(); i != messages.end(); ++i)
		{
			sendmsg= *i;
			if(sendmsg != "done")
			{
#ifdef SERVERDEBUG
				if( sendmsg == "stopclient"
					||
					sendmsg == "serverisstopping"	)
				{
					msg= "server stop HEARing connection to client ";
					msg+=  descriptor.getHostAddressName();
					cout << msg << endl;
				}else
					cout << "send: " << sendmsg << endl;
#endif

				if(sendmsg.size() > 10)
					POSS("#client#send-hearing", sendmsg.substr(0, 10) + " ...");
				else
					POSS("#client#send-hearing", sendmsg);
				if(sendmsg.substr(0, 23) == "read_owserver_debuginfo")
				{
					unsigned short server;
					istringstream info(sendmsg);

					descriptor.setBoolean("readdebuginfo", true);
					info >> msg >> server;
					debuginfo= db->getOWDebugInfo(server);
					for(vector<string>::iterator it= debuginfo.begin(); it != debuginfo.end(); ++it)
					{
						descriptor << *it;
						descriptor.endl();
						descriptor.flush();
					}
					return true;
				}
				if(sendmsg == "serverisstopping")
					sendmsg= "ERROR 019";
				sendmsg+= "\n";
				descriptor << sendmsg;
				descriptor.flush();
				//if(descriptor.eof())	// for asking eof() after connection is broken
										// and server only sending messages kernel throw an exception
				if( sendmsg == "stopclient\n"
					||
					sendmsg == "serverisstopping"	)
				{

					return false;
				}
			}
		}
		descriptor.setBoolean("readdebuginfo", false);
		return true;

	}

	bool ServerTransaction::clientCommands(IFileDescriptorPattern& descriptor)
	{
		string::size_type length;
		string input;
		string sendmsg, msg;
		ostringstream omsg;
		string username(descriptor.getString("username"));

		POS("#client#wait-forQuestion");
#ifdef SERVERDEBUG
			cout << "Server waiting for new command ..." << endl;
#endif // SERVERDEBUG
		descriptor >> input;
		if(descriptor.eof())
		{
			DbInterface* db= DbInterface::instance();

			m_fProtocol= 0;
			db->clearOWDebug(descriptor.getClientID());
			db->needSubroutines(descriptor.getClientID(), "stopclient");
			msg= "connection to client:";
			msg+=  descriptor.getHostAddressName();
			msg+= " is brocken";
			LOG(LOG_SERVERINFO, msg);

#ifndef DEBUG
#ifdef SERVERDEBUG
			cout << msg << endl;
#endif // SERVERDEBUG
#else // DEBUG
			cout << msg << endl;
#endif // DEBUG

			return false;
		}
		input= ConfigPropertyCasher::trim(input, " \t\r\n");
		length= input.size();
		if(length > 10)
			POSS("#client#answer-question", input.substr(0, 10) + " ...");
		else
			POSS("#client#answer-question", input);

#ifdef SERVERDEBUG
		cout << "get: " << input << endl;
#endif // SERVERDEBUG

// *************************************************************************************************
// *** checking whether hole ppi-server
// *** is finished by loading all content
// ***
		if(!descriptor.getBoolean("finishedloading"))
		{
			LOCK(m_FINISHEDSERVERMUTEX);
			if(!m_bFinished)
			{
				if(	input.substr(0, 3) == "GET" &&
					input != "GET wait"				)
				{
					short res, nPercent;
					string sProcess;
					IUserManagementPattern* user;
					DbInterface* db;

					user= UserManagement::instance();
					res= user->finished();
					if(	res == 0 &&
						m_fProtocol <= 1.0	)
					{
#ifdef SERVERDEBUG
						cout << "server not finished by loading user-management" << endl;
						cout << " waiting until finished" << endl;
#endif // SERVERDEBUG
						res= user->finished();
						if(res != 0)
						{
							descriptor << "SERVERBUSY";
							descriptor.endl();
							descriptor.flush();
							UNLOCK(m_FINISHEDSERVERMUTEX);
							return false;
						}
					}
					if(res == 0)// server loading
					{
#ifdef SERVERDEBUG
					cout << "server not finished by loading user-management" << endl;
					cout << "ending connection" << endl;
					cout << "send: 'ppi-server: WARNING 001 starting -1'" << endl;
#endif // SERVERDEBUG
					cout << "server starting, ending connection with 'WARNING 001 starting -1'" << endl;
						descriptor << "ppi-server: WARNING 001 starting -1";
						descriptor.endl();
						descriptor.flush();
						UNLOCK(m_FINISHEDSERVERMUTEX);
						return true;

					}else if(res == 1)// loding is finished
					{
						if(m_uid != 0)
						{
							if(setuid(m_uid) != 0)
							{
								string err;

								err=   "### ERROR: cannot set process to default user\n";
								err+=  "    ERRNO: " + *::strerror(errno);
								err+= "\n          so internet server running as root";
								LOG(LOG_ALERT, err);
								cerr << err << endl;
							}
						}
					}else
					{// error occurred by loading UserManagement
						descriptor << "ppi-server: ERROR 020";
						descriptor.endl();
						descriptor.flush();
#ifdef SERVERDEBUG
						cout << "send: ERROR 020 - ";
						cout << "Usermanagement cannot be loading correctly" << endl;
#endif
						UNLOCK(m_FINISHEDSERVERMUTEX);
						return false;
					}
					if(input != "GET wait")
					{
						db= DbInterface::instance();
						if(!db->isServerConfigured(sProcess, nPercent))
						{
							ostringstream output;

							output << "ppi-server: WARNING 001 " << sProcess << " " << nPercent;
	#ifdef SERVERDEBUG
							cout << "server not finished by starting, running " << sProcess << " by " << nPercent << "%" << endl;
							cout << "send: " << output.str() << endl;
	#endif // SERVERDEBUG
							descriptor << output.str();
							descriptor.endl();
							descriptor.flush();
							UNLOCK(m_FINISHEDSERVERMUTEX);
							return true;
						}
						m_bFinished= true;
						descriptor.setBoolean("finishedloading", true);
					}
				}
			}else
				descriptor.setBoolean("finishedloading", true);
			UNLOCK(m_FINISHEDSERVERMUTEX);
		}
// *** end of checking
// *************************************************************************************************

		// --------------------------------------------------
		// first all server transaction which do not need an loggin
		if(input.substr(0, 6) == "status")
		{
			DbInterface* db= DbInterface::instance();
			vector<string> status;
			string param;

			if(input.length() > 6)
				param= ConfigPropertyCasher::trim(input.substr(6));
			if(param == "")
				param= "text";
			sendmsg= Thread::getStatusInfo(param);
			status= db->getStatusInfo(param);
			for(vector<string>::iterator it= status.begin(); it != status.end(); ++it)
			{
				if(*it != "done")
				{
#ifdef SERVERDEBUG
					cout << "send: " << *it;
#endif

					if(it->substr(0, 11) == "with client")
						descriptor << "         ";
					descriptor << *it;
					if(	*it == "" ||
						it->substr(it->length()-1, 1) != "\n"	)
					{
						descriptor.endl();
					}
				}
			}
#ifdef SERVERDEBUG
			cout << "send: " << sendmsg;
#endif
			descriptor << sendmsg;
#ifdef SERVERDEBUG
			cout << "send: done" << endl;
#endif
			descriptor << "done\n";
			return true;

		}else if(input == "ending")
		{
			DbInterface* db= DbInterface::instance();

			m_fProtocol= 0;
			db->clearOWDebug(descriptor.getClientID());
			db->needSubroutines(descriptor.getClientID(), "stopclient");
			omsg << "client on host '" << descriptor.getHostAddressName() << "' ";
			if(username != "")
				omsg << "with user '" << username << "' ";
			omsg << "and ID:" << descriptor.getClientID() << " stopping connection to server";
			LOG(LOG_SERVERINFO, omsg.str());
#ifdef SERVERDEBUG
			cout << "client stop connection" << endl;
#endif
			return false;

		}else if(	input == "init"
					||
					input == "ppi-internet-server true false init"	)
		{
			descriptor << "done";
			descriptor.endl();
			descriptor.flush();
#ifdef SERVERDEBUG
				cout << "send: done" << endl;
#endif
			omsg << "client on host '" << descriptor.getHostAddressName() << "' ";
			if(username != "")
				omsg << "with user '" << username << "' ";
			omsg << "and ID:" << descriptor.getClientID() << " asking only for initialization";
			LOG(LOG_SERVERINFO, omsg.str());
			return true;

		}else if(	input == "GETMINMAXERRORNUMS"
					||
					input == "ppi-internet-server true false getMinMaxErrorNums"	)
		{
			ostringstream output;

			output << getMaxErrorNums(false) * -1;
			output << " ";
			output << getMaxErrorNums(true);
			descriptor << output.str();
			descriptor.endl();
			descriptor.flush();
#ifdef SERVERDEBUG
			cout << "send: " << output.str() << endl;
#endif
			return true;

		}else if(	input.substr(0, 15) == "GETERRORSTRING "
					||
					input.substr(0, 40) == "ppi-internet-server true false getErrorString "	)
		{
			istringstream in(input);
			string strings;
			int errnr;

			in >> strings;
			if(strings == "ppi-internet-server")
			{
				in >> strings;// = 'true'
				in >> strings;// = 'getErrorString'
			}
			in >> errnr;
			descriptor << strerror(errnr);
			descriptor.endl();
			descriptor.flush();
#ifdef SERVERDEBUG
			cout << "send: " << strerror(errnr);
#endif
			return true;

		}

		if(	!descriptor.getBoolean("access") ||
			(	input.length() > 6 &&
				input.substr(0, 6) == "CHANGE"	)	)
		{
			bool ok(false);
			string spinput;
			istringstream oinput(input);

			oinput >> spinput;
			if(spinput == "GET")
			{
				unsigned int ID(0);

				oinput >> spinput;
				if(spinput.substr(0, 1) == "v")
				{
					float getnum;
					istringstream num(spinput.substr(1));

					num >> getnum;
					if(getnum > PPI_SERVER_PROTOCOL)
						m_fProtocol= PPI_SERVER_PROTOCOL;
					else
						m_fProtocol= getnum;
					oinput >> spinput;
				}
				if(	spinput == "wait" ||
					m_fProtocol >= 1.0	)
				{
					descriptor.setBoolean("wait", true);
					if(spinput == "wait")
						oinput >> spinput;

				}else
					descriptor.setBoolean("wait", false);
				if(spinput.substr(0, 3) == "ID:")
				{
					istringstream id(spinput.substr(3));

					id >> ID;
					omsg << "client on host '" << descriptor.getHostAddressName() << "' ";
					if(username != "")
						omsg << "with user '" << username << "' ";
					omsg << "and ID:" << descriptor.getClientID() << ",\n";
					if(ID == 0)
					{
						sendmsg= INFOERROR(descriptor, 10, input, "");
					}else
					{
						descriptor.setClientID(ID);
						descriptor.setBoolean("speaker", true);
						omsg << "switch connection to hearing and gets ID:" << descriptor.getClientID();
						LOG(LOG_SERVERINFO, omsg.str());
#ifdef SERVERDEBUG
						cout << omsg.str() << endl;
#endif
					}
				}else
					descriptor.setBoolean("speaker", false);

				if(sendmsg == "")
				{
					ostringstream out;

					if(m_fProtocol >= 1.0)
					{
						out << "ppi-server:" << descriptor.getClientID() << " v";
						out.setf(ios_base::fixed, ios_base::floatfield);
						out.precision(2);
						out << m_fProtocol;

					}else
						out << "port-server:" << descriptor.getClientID();
					sendmsg= out.str();
				}
		#ifdef SERVERDEBUG
					cout << "send: " << sendmsg << endl;
		#endif
				sendmsg+= "\n";
				descriptor << sendmsg;
				return true;

			}else if(	descriptor.getBoolean("finishedloading") &&
						(	(	!descriptor.getBoolean("access") &&
								length > 2 &&
								spinput.substr(0, 2) == "U:"	) ||
							(	descriptor.getBoolean("access")
								&&
								spinput == "CHANGE"				)	)	)
			{
				bool login= true;
				short first= 1;

				vector<string> split;
				UserManagement* user= UserManagement::instance();

				if(spinput == "CHANGE"	)
				{
					oinput >> spinput;
					first= 0;
					login= false;
				}
				split= ConfigPropertyCasher::split(spinput, ":");
				if(	(	first == 0
						&&
						split.size() == 2	)
					||
					(	first == 1
						&&
						split.size() == 3	)	)
				{
					if(!user->hasAccess(split[first], split[first+1], login))
					{
						int error;
						string msg("for user ");

						msg+= split[first];
						if(	!descriptor.getBoolean("access")
							&&
							!user->canLoginFirst(split[first])	)
						{
							error= 15;
							sendmsg= INFOERROR(descriptor, error, input, msg);
						}else
						{
							if(!descriptor.getBoolean("access"))
								msg+= ", permission denied";
							sendmsg= ERROR(descriptor, 11, input, msg);
						}
						sleep(2);
						descriptor << sendmsg;
						if(	descriptor.getBoolean("access") ||
							error == 15							)
						{
							return descriptor.getBoolean("wait");
						}
						sendmsg= ERROR(descriptor, error, input, msg);
						return false;
					}
					omsg << "client on host '" << descriptor.getHostAddressName() << "' ";
					if(username != "")
						omsg << "with user '" << username << "' ";
					omsg << "and ID:" << descriptor.getClientID();
					descriptor.setString("username", split[first]);
					descriptor.setBoolean("access", true);
					ok= true;
					sendmsg= "OK\n";
					descriptor << sendmsg;
					descriptor.flush();
					if(login)
						omsg << "\nget access to server with user name '" << split[first] << "'";
					else
						omsg << "\nchange user correctly to user '" << split[first] << "'";
					LOG(LOG_SERVERINFO, omsg.str());
#ifdef SERVERDEBUG
					cout << omsg.str() << endl;
#endif
					return true;
				}
			}
			if(!ok)
			{
				descriptor << DEBUGERROR(descriptor, 2, input, "");
				return false;
			}
		}else
		{
			if(	input == "stop-server"
				||
				input == "restart-server"	)
			{
				string action;
				DbInterface* db= DbInterface::instance();
				//LogInterface* logger= LogInterface::instance();
				UserManagement* user= UserManagement::instance();
				NeedDbChanges* dbchanges= NeedDbChanges::instance();
				IServerPattern* server= descriptor.getServerObject();
				IClientHolderPattern* starter= server->getCommunicationFactory();

				db->fillValue("ppi-server", "starting", "starting", 0);
				glob::stopMessage("get stop or restart command from outside");
				if(user->getRootUser() != descriptor.getString("username"))
				{
					string msg;

					msg+= "client want to stop server with no root user '";
					msg+= descriptor.getString("username") + "'\n";
					msg+= "permisson denied";
					sendmsg= INFOERROR(descriptor, 13, input, msg);
					glob::stopMessage(msg);
					descriptor << sendmsg;
	#ifdef SERVERDEBUG
					cerr << msg << endl;
	#endif
					return descriptor.getBoolean("wait");
				}
				omsg << "user '" << username << "' with ID:" << descriptor.getClientID() << " stop hole server application";
				LOG(LOG_SERVERINFO, omsg.str());
				glob::stopMessage("do not allow new connections");
				server->allowNewConnections(false);
				glob::stopMessage("ending all Debug message output from owreader to any client");
				db->clearOWDebug(0);
				glob::stopMessage("do not start again any new hearingPort transactions");
				LOCK(m_SERVERISSTOPPINGMUTEX);
				m_bStopServer= true;
				UNLOCK(m_SERVERISSTOPPINGMUTEX);
				glob::stopMessage("send stop command to CLASS NeedDbChanges");
				dbchanges->stop(false);
				db->needSubroutines(0, "serverisstopping");
				glob::stopMessage("send first stopping message to all existing clients of server");
				starter->stopCommunicationThreads(descriptor.getClientID(), /*wait*/false);
				glob::stopMessage("send stop command to process ppi-db-server");
				do{
					//cout << "send stop-all to database" << endl;
					action= db->stopall();
					if(action != "done")
					{
						descriptor << action;
						descriptor.endl();
						descriptor.flush();
					}
				}while(action != "done");
				glob::stopMessage("stopping of ppi-db-server process and all other member processes should be done");
				do{
					glob::stopMessage("send stopping message to existing internet clients");
					descriptor << "stop Internet clients";
					descriptor.endl();
					descriptor.flush();
				}while(!starter->stopCommunicationThreads(descriptor.getClientID(), /*wait*/true));
				descriptor << "### stoping internet clients are performed";
				descriptor.endl();
				descriptor.flush();
				glob::stopMessage("ending connection to LogInterface");
				//logger->closeSendConnection();
				sendmsg= "OK";
				descriptor << sendmsg;
				descriptor.endl();
				descriptor.flush();
		#ifdef SERVERDEBUG
					cout << "send: OK" << endl;
					cout << "MeasureThreads are be stopping" << endl;
		#endif
				glob::stopMessage("send stop message to own ppi-internet-server thread");
				server->stop(false);
				glob::stopMessage("stopping was performed, ending with no new transaction");
				return false;

			}else if(	length > 10
						&&
						input.substr(0, 10) == "PERMISSION"	)
			{
				string username(descriptor.getString("username"));
				UserManagement* user= UserManagement::instance();

				input= input.substr(10);
				input= ConfigPropertyCasher::trim(input);
				// set third parameter to 'read'
				// because method ask only to have any permission
				if(user->hasPermission(username, input, "write"))
					sendmsg= "write";
				else if(user->hasPermission(username, input, "read"))
					sendmsg= "read";
				else
					sendmsg= "none";
	#ifdef SERVERDEBUG
				cout << "send: " << sendmsg << endl;
	#endif // SERVERDEBUG
				sendmsg+= "\n";
				descriptor << sendmsg;

			}else if(input.substr(0, 4) == "SHOW")
			{
				bool bFail(false), bClient(false);
				int seconds;
				istringstream action(input);
				DbInterface* db= DbInterface::instance();

				action >> sendmsg; // first output is 'SHOW' (do not need)
				action >> seconds;
				if(action.fail())
				{
					action.clear();
					action >> sendmsg;
					if(	action.eof() ||
						action.fail() ||
						sendmsg != "c"	)
					{
						sendmsg= INFOERROR(descriptor, 21, input, "");
						bFail= true;
					}else
					{
						bClient= true;
						action >> seconds;
						if(	action.fail() ||
							seconds <= 0	)
						{
							sendmsg= INFOERROR(descriptor, 21, input, "");
							bFail= true;
						}
					}
				}
				if(!bFail)
				{
					db->showThreads(seconds, bClient);
					sendmsg= "done\n";
#ifdef SERVERDEBUG
					cout << "send: " << sendmsg;
#endif // SERVERDEBUG
				}
				descriptor << sendmsg;

			}else if(	input.substr(0, 5) == "DEBUG" ||
						input.substr(0, 9) == "STOPDEBUG"	)
			{
				bool bDebug= true;
				//unsigned int nFolderPos;
				string sCommand, sFolderSub, sFolder, sSubroutine;
				stringstream ss(input);
				vector<string> values;
				DbInterface* db= DbInterface::instance();

				if(input.substr(0, 9) == "STOPDEBUG")
					bDebug= false;
				ss >> sCommand;
				ss >> sFolderSub;
				if(sFolderSub == "-ow")
				{
					unsigned short ID;

					if(ss.eof())
						ID= 0;
					else
						ss >> ID;
					if(ID == 0)
					{
						ID= 0;
						db->clearOWDebug(descriptor.getClientID());
					}else
					{
						if(!db->existOWServer(ID))
						{
							ostringstream msg;

							msg << "client ask for '";
							msg << input << "'\ncannot found OWServer with ID ";
							msg << ID;
							descriptor << INFOERROR(descriptor, 17, input, msg.str());
							ID= 0;
						}else
							db->setOWDebug(ID, descriptor.getClientID(), true);
					}
					if(ID != 0)
					{
						ostringstream server;

						server << "owserver-" << ID;
						if(!db->needSubroutines(descriptor.getClientID(), server.str()))
						{
							descriptor << INFOERROR(descriptor, 17, input, "Undefined Error in DbInterface::needSubroutine()");
						}else
						{
							sendmsg= "done\n";
							descriptor << sendmsg;
						}
					}else
					{
						db->needSubroutines(descriptor.getClientID(), "stopclient");
						sendmsg= "done\n";
						descriptor << sendmsg;
					}
				}else
				{
					vector<string> spl;

					trim(sFolderSub);
					if(sFolderSub == "")
					{
						if(bDebug == false)
						{
							db->clearFolderDebug();
							sendmsg= "done\n";
							descriptor << sendmsg;
						}else
							descriptor << DEBUGERROR(descriptor, 4, input, "no folder or folder:subroutine be given");
					}else
					{
						bool bInform(false);

						if(sFolderSub == "-i")
						{
							bInform= true;
							ss >> sFolderSub;
							trim(sFolderSub);
						}
						split(spl, sFolderSub, is_any_of(":"));
						sFolder= spl[0];
						if(spl.size() > 1)
							sSubroutine= spl[1];
						if(db->existSubroutine(sFolder, sSubroutine))
						{
							db->debugSubroutine(bDebug, bInform, sFolder, sSubroutine);
							sendmsg= "done\n";
							descriptor << sendmsg;
						}else
							descriptor << INFOERROR(descriptor, 5, input, "");
					}
				}

			}else if(input.substr(0, 3) == "DIR")
			{
				typedef vector<string>::iterator iter;
				vector<string> list;
				string subroutine;
				string filter;
				string sChar;

				if(input.length() <= 5)
				{
					descriptor << DEBUGERROR(descriptor, 7, input, "");
				}else
				{
					int begin= 3, len= 0;

					sChar= input[begin];
					while(sChar == " ")
					{
						++begin;
						sChar= input[begin];
					}
					sChar= input[begin + len];
					while(	sChar != " "
							&&
							sChar != "\n"	)
					{
						++len;
						sChar= input[begin + len];
					}
					filter= input.substr(begin, len);
					if(getDirectory(filter, "", list))
					{
						for(iter i= list.begin(); i != list.end(); ++i)
						{
							subroutine= *i;
							subroutine+= "\n";
		#ifdef SERVERDEBUG
							cout << "send: " << subroutine << flush;
		#endif
							descriptor << subroutine;
							//descriptor.flush();
						}
					}else
						descriptor << ERROR(descriptor, 8, input, "with this filter '" + filter + "'");
				}

		#ifdef SERVERDEBUG
					cout << "send: done" << endl;
		#endif
				sendmsg= "done\n";
				descriptor << sendmsg;

			}else if(input.substr(0, 7) == "CONTENT")
			{
				string line;
				std::ifstream file;
				string path(global_clientpath);
				string fileName(ConfigPropertyCasher::trim(input.substr(7)));
				XMLStartEndTagReader reader;

				if(	path.substr(path.length()-1, 1) != "/"
					&&
					fileName.substr(0, 1) != "/"				)
				{
					path+= "/";
				}
				path+= fileName;

#ifdef SERVERDEBUG
				int nContent(0);
#endif
				file.open(path.c_str());
				if(file.is_open())
				{
					while(!file.eof())
					{
						getline(file, line);
						line= reader.readLine(line);
						if(reader.read())
						{
							line+= "\n";
							descriptor << line;
#ifdef SERVERDEBUG
							++nContent;
#endif
						}
						if(reader.end())
							break;
					}
					line= reader.endTag();
					if(line != "")
						descriptor << line;
					file.close();
#ifdef SERVERDEBUG
					cerr << "sending content of " << nContent << "rows is finished" << endl;
#endif
				}else
					descriptor << INFOERROR(descriptor, 9, input, "");


			}else if(	input.substr(0, 4) == "SET "
						||
						input.substr(0, 4) == "GET "	)
			{
				bool bGet= true;
				bool bWait= false;
				unsigned short nCycle= 0;
				//unsigned int nFolderPos;
				string buffer;
				string sSubroutine;
				stringstream ss(input);
				vector<string> values;
				unsigned short nExist= 6;
				DbInterface* db= DbInterface::instance();

				while(ss >> buffer)
				{
					bWait= false;
					switch(nCycle)
					{
					case 0:
						// looking of command GET or SET
						if(buffer == "GET")
						{
							bGet= true;
							bWait= true;
						}else if(buffer == "SET")
						{
							bGet= false;
							bWait= true;
						}else
						{
							string msg;

							msg+= "client ask for '";
							msg+= input + "'\n";
							msg+= "first parmeter ";
							msg+= buffer;
							msg+= " is incorrect";
							msg+= "\nsend ERROR 003 0";
							LOG(LOG_SERVERDEBUG, msg);
							sendmsg= "ERROR 003 0\n";
							descriptor << sendmsg;
							bWait= false;
		#ifdef SERVERDEBUG
							cerr << msg << endl;
		#endif
						}
						break;

					case 1:
						// verification of permission by path
						// and if command is GET, write value of path to client
						values= ConfigPropertyCasher::split(buffer, ":");
						if(values.size() < 2)
						{
							string msg;

							msg+= "client ask for '";
							msg+= input + "'\n";
							msg+= "second parmeter ";
							msg+= buffer;
							msg+= " is incorrect";
							msg+= "\nsend ERROR 003 1";
							LOG(LOG_SERVERDEBUG, msg);
							sendmsg= "ERROR 003 1\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cout << "send: ERROR 003 1" << endl;
	#endif
							bWait= false;
						}else
						{
							nExist= db->existEntry(values[0], values[1], "value", 0);
						}

						if(nExist == 5)
						{
							short noexist;
							double value;
							string command;
							string account(descriptor.getString("username"));
							UserManagement* user= UserManagement::instance();

							if(bGet)
								command= "read";
							else
								command= "write";
							if(user->hasPermission(account, values[0], values[1], command))
							{
								if(bGet)
								{
									ostringstream ovalue;

									value= db->getFolderValue(noexist, values[0], values[1], "e:" + account);
									if(noexist == 0)
									{
										ovalue << value;
										sendmsg= ovalue.str();
#ifdef SERVERDEBUG
										cout << "send: " << sendmsg << endl;
#endif
										sendmsg+= "\n";
										descriptor << sendmsg;

									}else if(noexist == -1)
										descriptor << DEBUGERROR(descriptor, 16, input, "");
									else if(noexist == -2)
										descriptor << ERROR(descriptor, 14, input, "");
									else if(noexist == -3)
										descriptor << ERROR(descriptor, 5, input, "");
									else if(noexist == -4)
										descriptor << ERROR(descriptor, 4, input, "");
									bWait= false;
								}else
								{
									ValueHolder oValue;

									ss >> oValue.value;
									if(!oValue.lastChanging.setActTime())
									{
										string err("internet-server cannot create actual time");

										err+= " for folder:" + values[0] + " subroutine:" + values[1] + "\n";
										err+= oValue.lastChanging.errorStr() + "\n";
										err+= "so send 0 time to server";
										TIMELOG(LOG_WARNING, "internet-server_timecreation", err);
									}
									db->setValue(values[0], values[1], oValue, account);
									sendmsg= "done";
#ifdef SERVERDEBUG
									cout << "send: " << sendmsg << endl;
#endif
									sendmsg+= "\n";
									descriptor << sendmsg;
									bWait= false;
								}

							}else
							{
								string msg;

								msg=  "user '";
								msg+= descriptor.getString("username") + "' has no permisson to subroutine";
								descriptor << DEBUGERROR(descriptor, 13, input, msg);
							}
						}else
						{
							switch(nExist)
							{
							case 4:
								// no access to device
								descriptor << DEBUGERROR(descriptor, 16, input, "");
								break;
							case 3:
								// number of value count do not exist
								// but this number is always 0 and cannot be
								// when this error code reached, write error code for 1
							case 2:
								// this identifier is every time 'value' and have to exist
								// when this error code reached, write error code for 1
							case 1:
								// subroutine do not exist
								descriptor << DEBUGERROR(descriptor, 5, input, "");
								break;
							default: // inherit code 0
								// folder do not exist
								descriptor << DEBUGERROR(descriptor, 4, input, "");
								break;
							}
							bWait= descriptor.getBoolean("wait");
						}
						break;

					}
					if(!bWait)
					{
						return descriptor.getBoolean("wait");	// is wait is  false
																// server must not wait for new transaction
					}
					++nCycle;
				}
			}else if(input == "NEWENTRYS")
			{

				DbInterface* db= DbInterface::instance();

				db->needSubroutines(descriptor.getClientID(), "newentrys");
				sendmsg= "done\n";
				descriptor << sendmsg;
	#ifdef SERVERDEBUG
				cout << "send: done" << endl;
	#endif

			}else if(input.substr(0, 5) == "HEAR ")
			{
				string entry;
				string groups;
				vector<string> split;
				UserManagement* user= UserManagement::instance();
				DbInterface* db= DbInterface::instance();


				entry= input.substr(5);
				split= ConfigPropertyCasher::split(entry, ":");
				if(split.size() < 2)
				{
					string msg;

					msg+= "client ask for '";
					msg+= input + "'\n";
					msg+= "second parmeter ";
					msg+= entry;
					msg+= " is incorrect";
					msg+= "\nsend ERROR 003 1";
					LOG(LOG_SERVERDEBUG, msg);
					sendmsg= "ERROR 003 1\n";
					descriptor << sendmsg;
#ifdef SERVERDEBUG
					cout << "send: ERROR 003 1" << endl;
#endif
				}else
				{
					if(user->hasPermission(descriptor.getString("username"), split[0], split[1], "read"))
					{
						if(db->needSubroutines(descriptor.getClientID(), entry))
						{
							sendmsg= "done\n";
#ifdef SERVERDEBUG
							cout << "send: done" << endl;
#endif
							descriptor << sendmsg;
						}else
							descriptor << DEBUGERROR(descriptor, 5, input, "cannot found given folder or subroutine");

					}else
					{
						string msg;

						msg= "by user '";
						msg+= descriptor.getString("username") + "'";
						descriptor << DEBUGERROR(descriptor, 13, input, msg);
					}
#if 0
					unsigned short nExist;

					nExist= db->existEntry(split[0], split[1], "value", 0);
					if(nExist > 3)
					{
						if(user->hasPermission(descriptor.getString("username"), split[0], split[1], "read"))
						{
							if(db->needSubroutines(descriptor.getClientID(), entry))
							{
								sendmsg= "done\n";
#ifdef SERVERDEBUG
								cout << "send: done" << endl;
#endif
								descriptor << sendmsg;
							}else
								descriptor << DEBUGERROR(descriptor, 5, input, "cannot found given folder or subroutine");

						}else
						{
							string msg;

							msg= "by user '";
							msg+= descriptor.getString("username") + "'";
							descriptor << DEBUGERROR(descriptor, 13, input, msg);
						}
					}else
					{
						switch(nExist)
						{
						case 4:
							// no access to device
							// by this error code make also an hearing (block before)
							// because client should wait for access
							break;
						case 3:
							// number of value count do not exist
							// but this number is always 0 and cannot be
							// when this error code reached, write error code for 1
						case 2:
							// this identifier is every time 'value' and have to exist
							// when this error code reached, write error code for 1
						case 1:
							// subroutine do not exist
							descriptor << DEBUGERROR(descriptor, 5, input, "");
							break;
						default: // inherit code 0
							// folder do not exist
							descriptor << DEBUGERROR(descriptor, 4, input, "");
							break;
						}
					}
#endif // if 0
				}
			}else
			{
				descriptor << ERROR(descriptor, 2, input, "");
				return descriptor.getBoolean("wait");
			}
		}
		return descriptor.getBoolean("wait");
	}

	string ServerTransaction::senderror(const string& file, const int line, const int type,
					const IFileDescriptorPattern& descriptor, const int num, string input, const string& add)
	{
		string user(descriptor.getString("username"));
		ostringstream logmsg;
		ostringstream sendmsg;

		sendmsg << "ERROR ";
		if(num < 100)
			sendmsg << "0";
		if(num < 10)
			sendmsg << "0";
		sendmsg << num <<  "\n";
		logmsg << "client from host '" << descriptor.getHostAddressName() << "' ";
		if(user != "")
			logmsg << " with user '" << user << "' ";
		logmsg << "and ID:" << descriptor.getClientID();
		if(	input.substr(0, 2) == "U:" ||
			input.substr(0, 7) == "change "	)
		{// make password irrecognizable with stars
			string::size_type n(1);
			vector<string> spl;

			if(input.substr(0, 2) == "U:")
				n= 2;
			split(spl, input, is_any_of(":"));
			input= spl[0] + ":";
			if( spl.size() > 1 &&
				n == 2				)
			{
				input+=  spl[1] + ":";
			}
			if(spl.size() > n)
				input.append(spl[n].length(), '*');
		}
		logmsg << " ask for '" << input << "'\n";
		logmsg << "send: " << sendmsg.str();
		if(add != "")
			logmsg <<  add << "\n";
		logmsg << strerror(num);
		ppi_database::DbInterface::instance()->log(file, line, type, logmsg.str());
#ifdef SERVERDEBUG
		cout << logmsg.str() << endl;
#endif
		return sendmsg.str();
	}

	string ServerTransaction::strerror(const int error) const
	{
		string str;

		switch(error)
		{
		case -1:
			str= "server is busy by starting";
			break;
		case 0:
			str= "no error occurred";
			break;
		case 1:
			str= "client beginning fault transaction";
			break;
		case 2:
			str= "no correct command given";
			break;
		case 3:
			str= "command parameter is incorrect, "
					"parameter after errornumber is position of error";
			break;
		case 4:
			str= "cannot found given folder for operation";
			break;
		case 5:
			str= "cannot found given subroutine in folder for operation";
			break;
		case 6:
			str= "unknown value to set in subroutine";
			break;
		case 7:
			str= "no filter be set for read directory";
			break;
		case 8:
			str= "cannot read any directory";
			break;
		case 9:
			str= "cannot found given file for read content";
			break;
		case 10:
			str= "given ID from client do not exist";
			break;
		case 11:
			str= "wrong user or password";
			break;
		case 12:
			str= "do not use error number 12 now";
			break;
		case 13:
			str= "user has no permission";
			break;
		case 14:
			str= "subroutine isn't correct defined by the settings of config file";
			break;
		case 15:
			str= "user cannot login as first";
			break;
		case 16:
			str= "subroutine has no correct access to device";
			break;
		case 17:
			str= "cannot find OWServer for debugging";
			break;
		case 18:
			str= "no communication thread is free for answer "
					"(this case can behavior when the mincommunicationthreads parameter be 0)";
			break;
		case 19:
			str= "server will be stopping from administrator";
			break;
		case 20:
			str= "cannot load UserManagement correctly";
			break;
		case 21:
			str= "unknown options after command SHOW";
			break;
		default:
			if(error > 0)
				str= "Undefined transaction error";
			else
				str= "Undefined transaction warning";
			break;
		}
		return str;
	}

	inline unsigned int ServerTransaction::getMaxErrorNums(const bool byerror) const
	{
		if(byerror)
			return 20;
		return 1;
	}

	bool ServerTransaction::getDirectory(string filter, string verz, vector<string> &list)
	{
		//struct dirent **namelist;
		struct dirent *dirName;
		struct stat fileStat;
		string path(global_clientpath);
		string folder;
		//int result;
		size_t filterLen= filter.length();
		DIR *dir;
		//map<string, string> result;

		if(	path.substr(path.length()-1, 1) != "/"
			&&
			verz.length() > 0
			&&
			verz.substr(0, 1) != "/"				)
		{
			path+= "/";
		}
		path+= verz;
		//result= URL::readDirectory(path, "", filter);
		//for(map<string, string>::iterator o= result.begin(); o != result.end(); ++o)
		//	cout << "first:" << o->first << " second:" << o->second << endl;
		//return true;
		dir= opendir(&path[0]);
		if(dir == NULL)
			return false;
		while((dirName= readdir(dir)) != NULL)
		{
			if(dirName->d_type == DT_LNK)
			{
				string msg("no links in filesystem client be supported\n");
				string name(dirName->d_name);

				msg+= "link ";
				msg+= name;
				msg+= " is not showen";
				TIMELOG(LOG_ERROR, name+"client_folder_link", msg);
				cerr << msg << endl;

			}else if(	dirName->d_type == DT_DIR
						&&
						strcmp(dirName->d_name, ".")
						&&
						strcmp(dirName->d_name, "..")	)
			{
				string newPath(verz);

				//printf ("%s\n", dirName->d_name);
				newPath+= "/";
				newPath+= string(dirName->d_name);
				getDirectory(filter, newPath, list);

			}else if(dirName->d_type == DT_REG)
			{
				//printf ("%s\n", dirName->d_name);
				folder= dirName->d_name;
				if(	folder.length() > filterLen
					&&
					folder.substr(folder.length()-filterLen, filterLen) == filter	)
				{
					string file(path + "/");

					file+= folder;
					folder= verz + "/" + folder + " -> ";
					//cout << "read stat from " << file << endl;
					if(stat(&file[0], &fileStat) == 0)
					{
						struct tm l;
						char ctime[21];

						if(localtime_r(&fileStat.st_mtime, &l) == NULL)
							TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
						strftime(ctime, 20, "%x %X", &l);
						//ctime= asctime(tm);
						folder+= ctime;
						//strftime(sTime, 20, , "fileStat->st_ctim;
					}
					list.push_back(folder);
				}
			}
			//free (namelist[result]);
		}
		closedir(dir);
		return true;
	}

	ServerTransaction::~ServerTransaction()
	{
	}

}
