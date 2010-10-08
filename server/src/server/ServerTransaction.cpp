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

#include "../pattern/server/IServerCommunicationStarterPattern.h"

#include "../server/libs/client/SocketClientConnection.h"

#include "../util/structures.h"

#include "../logger/lib/LogInterface.h"

#include "../ports/measureThread.h"

#include "../portserver/lib/OWInterface.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/XMLStartEndTagReader.h"
#include "../util/usermanagement.h"
#include "../util/URL.h"

#include "../util/properties/configpropertycasher.h"

#include "../database/lib/DbInterface.h"
#include "../database/lib/NeedDbChanges.h"

#include "libs/server/ServerThread.h"
#include "libs/server/communicationthreadstarter.h"

#include "ServerTransaction.h"

extern string global_clientpath;

using namespace std;
using namespace user;
using namespace util;
using namespace logger;
using namespace server;
using namespace ppi_database;
using namespace design_pattern_world::server_pattern;

//ServerThread* gInternetServer= NULL;

namespace server
{
	ServerTransaction::ServerTransaction()
	:	m_bStopServer(false)
	{
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
		return true;
	}

	bool ServerTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool hold;
		bool bServerStops;

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

		}else
			hold= clientCommands(descriptor);

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

		POS("#client#wait-forQuestion");
		descriptor >> input;
		if(descriptor.eof())
		{
			DbInterface* db= DbInterface::instance();

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

		}else if(input == "ending")
		{
			DbInterface* db= DbInterface::instance();

			db->clearOWDebug(descriptor.getClientID());
			db->needSubroutines(descriptor.getClientID(), "stopclient");
#ifdef SERVERDEBUG
			cout << "client stop connection" << endl;
#endif
			return false;

		}else if(	input == "init"
					||
					input == "ppi-internet-server true init"	)
		{
			descriptor << "done";
			descriptor.endl();
			descriptor.flush();

		}else if(	input == "GETMINMAXERRORNUMS"
					||
					input == "ppi-internet-server true getMinMaxErrorNums"	)
		{
			ostringstream output;

			output << getMaxErrorNums(false) * -1;
			output << " ";
			output << getMaxErrorNums(true);
			descriptor << output.str();
			descriptor.endl();
			descriptor.flush();

		}else if(	input.substr(0, 15) == "GETERRORSTRING "
					||
					input.substr(0, 40) == "ppi-internet-server true getErrorString "	)
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

		}else if(	!descriptor.getBoolean("access")
					||
					(	length > 6
						&&
						input.substr(0, 6) == "CHANGE"	)	)
		{
			bool ok= false;

			if(	input == "GET"
				||
				input == "GET wait"
				||
				(	length > 7
					&&
					input.substr(0, 7) == "GET ID:"	)
				||
				(	length > 12
					&&
					input.substr(0, 12) == "GET wait ID:"	)	)
			{
				short nPos= 0;
				char cID[21];
				unsigned int ID;

				if(	length > 7
					&&
					input.substr(0, 7) == "GET ID:"	)
				{
					nPos= 7;
				}else if(	length > 12
							&&
							input.substr(0, 12) == "GET wait ID:"	)
				{
					nPos= 12;
				}
				if(nPos)
				{
					string sID;

					sID= input.substr(nPos);
					ID= atoi(sID.c_str());
					if(ID)
					{
						descriptor.setClientID(ID);
						descriptor.setBoolean("speaker", true);
					}else
					{
						string msg("ERROR: client givs no correct ID. Send ERROR code 010");

						LOG(LOG_SERVERERROR, msg);
						sendmsg= "ERROR 010";
					}
				}

				if(sendmsg == "")
				{
					cID[20]= '\0';
					sendmsg= "port-server:";
					snprintf(cID, 20, "%i", descriptor.getClientID());
					sendmsg+= cID;
					if(input.substr(0, 8) == "GET wait")
						descriptor.setBoolean("wait", true);
				}
		#ifdef SERVERDEBUG
					cout << "send: " << sendmsg << endl;
		#endif
				sendmsg+= "\n";
				descriptor << sendmsg;
				return true;

			}else if(	(	!descriptor.getBoolean("access")
							&&
							length > 2
							&&
							input.substr(0, 2) == "U:"	)
						||
						(	descriptor.getBoolean("access")
							&&
							length > 7
							&&
							input.substr(0, 6) == "CHANGE"	)	)
			{
				bool login= true;
				short first= 1;

				vector<string> split;
				UserManagement* user= UserManagement::instance();

				if(	length > 7
					&&
					input.substr(0, 6) == "CHANGE"	)
				{
					input= input.substr(7);
					first= 0;
					login= false;
				}
				split= ConfigPropertyCasher::split(input, ":");
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
						string msg("### ERROR: user ");

						msg+= split[first];
						sendmsg= "ERROR ";
						if(	!descriptor.getBoolean("access")
							&&
							!user->rootLogin()
							&&
							user->isRoot(split[first]))
						{
							msg+= " is root and cannot login as first user";
							sendmsg+= "015";
						}else
						{
							if(user->isUser(split[first]))
							{
								msg+= " have no correct password";
								sendmsg+= "012";
							}else
							{
								msg+= " is no correct user";
								sendmsg+= "011";
							}
							if(!descriptor.getBoolean("access"))
								msg+= ", so permission denied";
						}
						LOG(LOG_SERVERERROR, msg);
						sleep(2);
	#ifdef SERVERDEBUG
						cerr << "send: " << sendmsg << endl;
						cerr << msg << endl;
	#endif // SERVERDEBUG
						sendmsg+= "\n";
						descriptor << sendmsg;
						if(descriptor.getBoolean("access"))
							return descriptor.getBoolean("wait");
						return false;
					}
					descriptor.setString("username", split[first]);
					descriptor.setBoolean("access", true);
					ok= true;
					sendmsg= "OK\n";
					descriptor << sendmsg;
					descriptor.flush();
					return true;
				}
			}
			if(!ok)
			{
				string msg("can not identify command '");

				msg+= input;
				msg+= "'";
				msg+= "\nsend ERROR 002";
				LOG(LOG_SERVERERROR, msg);
				sendmsg= "ERROR 002\n";
				descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cerr << msg << endl;
		#endif
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
				LogInterface* logger= LogInterface::instance();
				UserManagement* user= UserManagement::instance();
				NeedDbChanges* dbchanges= NeedDbChanges::instance();
				IServerPattern* server= descriptor.getServerObject();
				IClientHolderPattern* starter= server->getCommunicationFactory();

				glob::stopMessage("get stop or restart command from outside");
				if(!user->isRoot(descriptor.getString("username")))
				{
					string msg;

					msg+= "client want to stop server with no root user '";
					msg+= descriptor.getString("username") + "'\n";
					msg+= "permisson denied, send ERROR 013";
					LOG(LOG_SERVERERROR, msg);
					glob::stopMessage(msg);
					sendmsg= "ERROR 013\n";
					descriptor << sendmsg;
	#ifdef SERVERDEBUG
					cerr << msg << endl;
	#endif
					return descriptor.getBoolean("wait");
				}
				LOG(LOG_SERVERINFO, "user stop server with foreign application");
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
				logger->closeSendConnection();
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

			}else if(input.substr(0, 5) == "DEBUG")
			{
				bool bWait= false;
				unsigned short nSleep;
				//unsigned int nFolderPos;
				string sCommand, sFolder, sSleep;
				stringstream ss(input);
				vector<string> values;
				DbInterface* db= DbInterface::instance();

				ss >> sCommand;
				ss >> sFolder;
				ss >> sSleep;
				nSleep= atoi(sSleep.c_str());
				if(sFolder == "-ow")
				{
					unsigned short ID= nSleep;
					string sID= sSleep;

					if(sID == "null")
					{
						ID= 0;
						db->clearOWDebug(descriptor.getClientID());
					}else
					{
						if(!db->existOWServer(ID))
						{
							string msg;

							ID= 0;
							msg+= "client ask for '";
							msg+= input + "'\ncannot found OWServer with ID ";
							msg+= sID;
							msg+= "\nsend ERROR 017";
							LOG(LOG_SERVERERROR, msg);
							sendmsg= "ERROR 017\n";
							descriptor << sendmsg;
#ifdef SERVERDEBUG
							cerr << msg << endl;
#endif
						}else
							db->setOWDebug(ID, descriptor.getClientID(), true);
					}
					if(ID != 0)
					{
						if(!db->needSubroutines(descriptor.getClientID(), "owserver-" + sID))
						{
							sendmsg= "ERROR 017\n";
#ifdef SERVERDEBUG
							cerr << "send: ERROR 017" << endl;
							cerr << "      undifined Error in DbInterface::needSubroutine()" << endl;
#endif
							descriptor << sendmsg;
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
					if(sFolder == "null")
					{
						db->clearFolderDebug();
						sendmsg= "done\n";
						descriptor << sendmsg;

					}else if(db->existFolder(sFolder))
					{
						db->debugFolder(sFolder);
						sendmsg= "done\n";
						descriptor << sendmsg;
					}else
					{
						string msg;

						msg+= "client ask for '";
						msg+= input + "'\ncannot found folder";
						msg+= "\nsend ERROR 004";
						LOG(LOG_SERVERERROR, msg);
						sendmsg= "ERROR 004\n";
						descriptor << sendmsg;
			#ifdef SERVERDEBUG
						cerr << msg << endl;
			#endif
					}
				}
				bWait= false;

			}else if(input.substr(0, 3) == "DIR")
			{
				typedef vector<string>::iterator iter;
				vector<string> list;
				string subroutine;
				string filter;
				string sChar;

				if(input.length() <= 5)
				{
					string msg;

					msg+= "client ask for '";
					msg+= input + "'\n";
					msg+= "\nsend ERROR 007";
					LOG(LOG_SERVERERROR, msg);
					sendmsg= "ERROR 007\n";
					descriptor << sendmsg;
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
					{
						string msg;

						msg+= "client ask for '";
						msg+= input + "'\n";
						msg+= "\nsend ERROR 008";
						LOG(LOG_SERVERERROR, msg);
						sendmsg= "ERROR 008\n";
						descriptor << sendmsg;
		#ifdef SERVERDEBUG
						cerr << msg << endl;
		#endif
					}
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
						}
						if(reader.end())
							break;
					}
					line= reader.endTag();
					if(line != "")
						descriptor << line;
					file.close();
				}else
				{
					string msg;

					msg+= "client ask for '";
					msg+= input + "'\n";
					msg+= "send ERROR 009";
					LOG(LOG_SERVERERROR, msg);
					sendmsg= "<error number=\"009\" />\n";
					descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cerr << msg << endl;
		#endif
				}


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
				meash_t *pCurMeas= NULL;
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
							LOG(LOG_SERVERERROR, msg);
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
						pCurMeas= NULL;
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
							LOG(LOG_SERVERERROR, msg);
							sendmsg= "ERROR 003 1\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cout << "send: ERROR 003 1" << endl;
	#endif
							bWait= false;
						}else
							nExist= db->existEntry(values[0], values[1], "value", 0);

						if(nExist == 5)
						{
							bool bCorrect;
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

														// bCorrect must be always true
									value= db->getActEntry(bCorrect, values[0], values[1], "value");
									//value= port->getValue("e:"+descriptor.getString("username"));
									ovalue << value;
									sendmsg= ovalue.str();
#ifdef SERVERDEBUG
									cout << "send: " << sendmsg << endl;
#endif
									sendmsg+= "\n";
									descriptor << sendmsg;
									bWait= false;
								}else
								{
									double value;

									ss >> value;
									db->setValue(values[0], values[1], value, account);
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

								msg+= "client ask for '";
								msg+= input + "'\n";
								msg+= "but user '";
								msg+= descriptor.getString("username") + "' has no permisson to subroutine\n";
								msg+= "so permisson denied, send ERROR 013";
								LOG(LOG_SERVERERROR, msg);
								sendmsg= "ERROR 013\n";
								descriptor << sendmsg;
								bWait= false;
#ifdef SERVERDEBUG
								cerr << msg << endl;
#endif
							}
						}else
						{
							descriptor << getNoExistErrorCode(nExist, values[0], values[1]);
							if(nExist != 4)
								bWait= false;
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
					LOG(LOG_SERVERERROR, msg);
					sendmsg= "ERROR 003 1\n";
					descriptor << sendmsg;
	#ifdef SERVERDEBUG
					cout << "send: ERROR 003 1" << endl;
	#endif
				}else
				{
					unsigned short nExist;

					nExist= db->existEntry(split[0], split[1], "value", 0);
					if(nExist > 3)
					{
						if(user->hasPermission(descriptor.getString("username"), split[0], split[1], "read"))
						{
							if(!db->needSubroutines(descriptor.getClientID(), entry))
							{
								sendmsg= "ERROR 005\n";
#ifdef SERVERDEBUG
								cerr << "send: ERROR 005" << endl;
								cerr << "      cannot found given folder or subroutine" << endl;
#endif
								descriptor << sendmsg;
							}else
							{
								sendmsg= "done\n";
								descriptor << sendmsg;
							}

						}else
						{
							string msg;

							msg+= "client ask for '";
							msg+= input + "'\n";
							msg+= "user '";
							msg+= descriptor.getString("username") + "' but has no permisson to subroutine\n";
							msg+= "so permisson denied, send ERROR 013";
							LOG(LOG_SERVERERROR, msg);
							sendmsg= "ERROR 013\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cerr << msg << endl;
	#endif
						}
					}else
						descriptor << getNoExistErrorCode(nExist, split[0], split[1]);
				}
			}else
			{
				string msg("can not identify command '");

				msg+= input;
				msg+= "'";
				msg+= "\nsend ERROR 002";
				LOG(LOG_SERVERERROR, msg);
				sendmsg= "ERROR 002\n";
				descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cerr << msg << endl;
		#endif
				return false; // server not wait
			}
		}
		return descriptor.getBoolean("wait");
	}

	string ServerTransaction::getNoExistErrorCode(const unsigned short err, const string& folder, const string& subroutine)
	{
		string sRv;
		string msg;

		switch(err)
		{
		case 4:
			msg+= "client ask for '";
			msg+= folder + ":" + subroutine + "'\n";
			msg+= "but subroutine has no correct acces to device\n";
			msg+= "send ERROR 016";
			LOG(LOG_SERVERERROR, msg);
#ifdef SERVERDEBUG
			cerr << msg << endl;
#endif
			sRv= "ERROR 016\n";
			break;

		case 1:
			msg+= "client ask for '";
			msg+= folder + ":" + subroutine + "'\n";
			msg+= "cannot find subroutine ";
			msg+= subroutine;
			msg+= " in folder ";
			msg+= folder;
			msg+= "\nsend ERROR 005";
			LOG(LOG_SERVERERROR, msg);
			sRv= "ERROR 005\n";
#ifdef SERVERDEBUG
			cerr << msg << endl;
#endif
			break;

		case 0:
			msg+= "client ask for >> ";
			msg+= folder + ":" + subroutine;
			msg+= "cannot find folder ";
			msg+= folder;
			msg+= "\nsend ERROR 004";
			LOG(LOG_SERVERERROR, msg);
	#ifdef SERVERDEBUG
			cout << "send: ERROR 004" << endl;
	#endif
			sRv= "ERROR 004\n";
			break;
		}
		return sRv;
	}

	string ServerTransaction::strerror(int error) const
	{
		string str;

		switch(error)
		{
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
			str= "unknow value to set in subroutine";
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
			str= "given user do not exist";
			break;
		case 12:
			str= "wrong password for given user";
			break;
		case 13:
			str= "user has no permission";
			break;
		case 14:
			str= "subrutine isn't correct defined by the settings of config file";
			break;
		case 15:
			str= "root cannot login as first user";
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
		return 0;
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
						struct tm *tm;
						char ctime[21];

						tm= localtime(&fileStat.st_mtime);
						strftime(ctime, 20, "%x %X", tm);
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
