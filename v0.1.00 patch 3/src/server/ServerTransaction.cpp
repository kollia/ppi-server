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

#include "../logger/LogThread.h"

#include "../ports/measureThread.h"

#include "../portserver/owserver.h"

#include "../util/XMLStartEndTagReader.h"
#include "../util/usermanagement.h"
#include "../util/configpropertycasher.h"

#include "../database/Database.h"

#include "ServerThread.h"
#include "ServerTransaction.h"
#include "communicationthreadstarter.h"

using namespace std;
using namespace user;
using namespace util;
using namespace ppi_database;


namespace server
{
	bool ServerTransaction::init(IFileDescriptorPattern& descriptor)
	{
		descriptor.setString("username", "");
		descriptor.setBoolean("speaker", false);
		descriptor.setBoolean("access", false);
		descriptor.setBoolean("wait", false);
		return true;
	}

	bool ServerTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool hold;

		if(	descriptor.getBoolean("access")
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
		Database* db= Database::instance();
		vector<string> messages;
		string sendmsg, msg;

		messages= db->getChangedEntrys(descriptor.getClientID());
		for(vector<string>::iterator i= messages.begin(); i != messages.end(); ++i)
		{
			sendmsg= *i;
#ifdef SERVERDEBUG
			if(sendmsg == "stopclient")
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
			sendmsg+= "\n";
			descriptor << sendmsg;
			descriptor.flush();
			//if(descriptor.eof())	// for asking eof() after connection is broken
									// and server only sending messages kernel throw an exception
			if(sendmsg == "stopclient\n")
				return false;
		}
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
			Database* db= Database::instance();

			db->needSubroutines(descriptor.getClientID(), "stopclient");
			msg= "connection to client:";
			msg+=  descriptor.getHostAddressName();
			msg+= " is brocken";
			LOG(LOG_SERVER, msg);

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

		if(input.substr(0, 6) == "status")
		{
			string param;

			if(input.length() > 6)
				param= ConfigPropertyCasher::trim(input.substr(6));
			sendmsg= Thread::getStatusInfo(param);
#ifdef SERVERDEBUG
			cout << "send: " << sendmsg << endl;
#endif
			descriptor << sendmsg;
#ifdef SERVERDEBUG
			cout << "send: done" << endl;
#endif
			descriptor << "done\n";

		}else if(	!descriptor.getBoolean("access")
					||
					(	length > 6
						&&
						input.substr(0, 6) == "CHANGE"	)
					||
					input == "ending"						)
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

						LOG(LOG_SERVER, msg);
						sendmsg= "ERROR 010";
					}
				}/*else
				{
					if(!m_bConnected)
					{// if client send in second time an GET command
					 // do not give the connection an new ID
						ID= m_poStarter->nextClientID();
						mutex_lock(m_CONNECTIONIDACCESS);
						m_unConnID= ID;
						mutex_unlock(m_CONNECTIONIDACCESS);
						mutex_lock(m_SPEAKERVARACCESS);
						m_bSpeakerThread= false;
						mutex_unlock(m_SPEAKERVARACCESS);
						m_bConnected= true;
					}
				}*/

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
						LOG(LOG_ERROR, msg);
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
			}else if(input == "ending")
			{
				Database* db= Database::instance();

				db->needSubroutines(descriptor.getClientID(), "stopclient");
	#ifdef SERVERDEBUG
				cout << "client stop connection" << endl;
	#endif
				return false;
			}
			if(!ok)
			{
				string msg("can not identify command '");

				msg+= input;
				msg+= "'";
				msg+= "\nsend ERROR 002";
				LOG(LOG_ERROR, msg);
				sendmsg= "ERROR 002\n";
				descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cerr << msg << endl;
		#endif
				return false;
			}
		}else
		{
			if(input == "stop-server")
			{
				meash_t *pCurrent= meash_t::firstInstance;
				ServerThread *server= ServerThread::instance();
				UserManagement* user= UserManagement::instance();
				CommunicationThreadStarter* starter= CommunicationThreadStarter::instance();

				if(!user->isRoot(descriptor.getString("username")))
				{
					string msg;

					msg+= "client want to stop server with no root user '";
					msg+= descriptor.getString("username") + "'\n";
					msg+= "permisson denied, send ERROR 013";
					LOG(LOG_ERROR, msg);
					sendmsg= "ERROR 013\n";
					descriptor << sendmsg;
	#ifdef SERVERDEBUG
					cerr << msg << endl;
	#endif
					return descriptor.getBoolean("wait");
				}
				cout << endl;
				cout << "stopping:" << endl;
				cout << "measureThreads " << flush;
				LOG(LOG_INFO, "user stop server with foreign application");
				while(pCurrent)
				{ // stopping all measure threads
					pCurrent->pMeasure->stop(/*wait*/false);
					pCurrent= pCurrent->next;
				}
				starter->stopCommunicationThreads();
				pCurrent= meash_t::firstInstance;
				while(pCurrent)
				{ // waiting for threads are Stopping
					sendmsg= "wait\n";
					cout << "." << flush;
					while(pCurrent->pMeasure->running())
					{
						descriptor << sendmsg;
						descriptor.flush();
		#ifdef DEBUG
						cout << "wait for MeasureThread" << endl;
		#endif
		#ifdef SERVERDEBUG
					cout << "send: wait" << endl;
		#endif
						sleep(1);
					}
					pCurrent= pCurrent->next;
				}
				cout << endl << "server " << flush;
				server->stop(false);
				// sending any command to server for stopping
				ServerThread::connectAsClient("127.0.0.1", 20004, false);
				cout << "." << endl;
				sendmsg= "OK\n";
				descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cout << "send: OK" << endl;
					cout << "MeasureThreads are be stopping" << endl;
		#endif
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
				bool bFoundFolder= false;
				unsigned short nSleep;
				//unsigned int nFolderPos;
				string sCommand, sFolder, sSleep;
				stringstream ss(input);
				vector<string> values;
				meash_t *pCurMeas= NULL;

				ss >> sCommand;
				ss >> sFolder;
				ss >> sSleep;
				nSleep= atoi(&sSleep[0]);
				if(sFolder == "-ow")
				{
					unsigned short ID= nSleep;
					string sID= sSleep;
					OWServer* server;
					Database* db= Database::instance();

					if(sID == "null")
						ID= 0;
					else
					{
						server= OWServer::getServer(ID);
						if(server == NULL)
						{
							string msg;

							ID= 0;
							msg+= "client ask for '";
							msg+= input + "'\ncannot found OWServer with ID ";
							msg+= sID;
							msg+= "\nsend ERROR 017";
							LOG(LOG_ERROR, msg);
							sendmsg= "ERROR 017\n";
							descriptor << sendmsg;
#ifdef SERVERDEBUG
							cerr << msg << endl;
#endif
						}
					}
					OWServer::setDebug(ID);
					if(ID != 0)
					{
						if(!db->needSubroutines(descriptor.getClientID(), "owserver-" + sID))
						{
							sendmsg= "ERROR 017\n";
#ifdef SERVERDEBUG
							cerr << "send: ERROR 017" << endl;
							cerr << "      undifined Error in Database::needSubroutine()" << endl;
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
					if(nSleep == 0)
						nSleep= 3;
					if(sFolder == "null")
						bFoundFolder= true;

					pCurMeas= meash_t::firstInstance;
					while(pCurMeas)
					{
						//cout << "'" << pCurMeas->pMeasure->getThreadName() << "' == '" << values[0] << "'" << endl;
						if(pCurMeas->pMeasure->getThreadName() == sFolder)
						{
							bFoundFolder= true;
							pCurMeas->pMeasure->setDebug(true, nSleep);
						}else
							pCurMeas->pMeasure->setDebug(false, 0);
						pCurMeas= pCurMeas->next;
					}
					if(bFoundFolder)
					{
						sendmsg= "done\n";
						descriptor << sendmsg;
					}
					else
					{
						string msg;

						msg+= "client ask for '";
						msg+= input + "'\ncannot found folder";
						msg+= "\nsend ERROR 004";
						LOG(LOG_ERROR, msg);
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
					LOG(LOG_ERROR, msg);
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
						LOG(LOG_ERROR, msg);
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
				string path(meash_t::clientPath);
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
					LOG(LOG_ERROR, msg);
					sendmsg= "<error number=\"009\" />\n";
					descriptor << sendmsg;
		#ifdef SERVERDEBUG
					cerr << msg << endl;
		#endif
				}


			}else if(	input.substr(0, 3) == "SET"
						||
						input.substr(0, 3) == "GET"	)
			{
				bool bGet= true;
				bool bWait= false;
				unsigned short nCycle= 0;
				//unsigned int nFolderPos;
				string buffer;
				string sSubroutine;
				stringstream ss(input);
				vector<string> values;
				meash_t *pCurMeas= NULL;
				portBase* port= NULL;

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
							LOG(LOG_ERROR, msg);
							sendmsg= "ERROR 003\n";
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
							LOG(LOG_ERROR, msg);
							sendmsg= "ERROR 003 1\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cout << "send: ERROR 003 1" << endl;
	#endif
							bWait= false;
						}else
							pCurMeas= getMeasurePort(values[0]);

						if(pCurMeas)
						{
							bool bCorrect;
							double value;
							char cValue[500];
							string groups, command;
							UserManagement* user= UserManagement::instance();

							if(pCurMeas)
							{
								port= pCurMeas->pMeasure->getPortClass(values[1], bCorrect);
								if(port)
								{
									if(bCorrect)
									{
										groups= port->getPermissionGroups();
										if(bGet)
											command= "read";
										else
											command= "write";
										if(user->hasPermission(descriptor.getString("username"), groups, command))
										{
											if(port->hasDeviceAccess())
											{
												if(bGet)
												{
													value= port->getValue("e:"+descriptor.getString("username"));
													sprintf(cValue, "%lf", value);
													sendmsg= cValue;
#ifdef SERVERDEBUG
													cout << "send: " << sendmsg << endl;
#endif
													sendmsg+= "\n";
													descriptor << sendmsg;
													bWait= false;
												}else
												{
													sSubroutine= values[1];
													bWait= true;
												}
											}else
											{
												string msg;

												msg+= "client ask for '";
												msg+= input + "'\n";
												msg+= "but subroutine has no correct acces to device\n";
												msg+= "send ERROR 016";
												LOG(LOG_ERROR, msg);
#ifdef SERVERDEBUG
												cerr << msg << endl;
#endif
												descriptor << "ERROR 016\n";
											}

										}else
										{
											string msg;

											msg+= "client ask for '";
											msg+= input + "'\n";
											msg+= "but user '";
											msg+= descriptor.getString("username") + "' has no permisson to subroutine\n";
											msg+= "so permisson denied, send ERROR 013";
											LOG(LOG_ERROR, msg);
											sendmsg= "ERROR 013\n";
											descriptor << sendmsg;
											bWait= false;
#ifdef SERVERDEBUG
											cerr << msg << endl;
#endif
										}
									}else
									{
										string msg;

										msg+= "client ask for '";
										msg+= input + "'\n";
										msg+= "user '";
										msg+= descriptor.getString("username") + "' but server has no correct chip contact\n";
										msg+= "send ERROR 014";
										LOG(LOG_ERROR, msg);
										sendmsg= "ERROR 014\n";
										descriptor << sendmsg;
										bWait= false;
				#ifdef SERVERDEBUG
										cerr << msg << endl;
				#endif
									}
								}else
								{
									string msg;

									msg+= "client ask for '";
									msg+= input + "'\n";
									msg+= "cannot find subroutine ";
									msg+= values[1];
									msg+= " in folder ";
									msg+= values[0];
									msg+= "\nsend ERROR 005";
									LOG(LOG_ERROR, msg);
									sendmsg= "ERROR 005\n";
									descriptor << sendmsg;
									bWait= false;
			#ifdef SERVERDEBUG
									cerr << msg << endl;
			#endif
								}
							}
						}else
						{
							string msg;

							msg+= "client ask for >> ";
							msg+= input;
							msg+= "cannot found folder ";
							msg+= values[0];
							msg+= "\nsend ERROR 004";
							LOG(LOG_ERROR, msg);
			#ifdef SERVERDEBUG
							cout << "send: ERROR 004" << endl;
			#endif
							sendmsg= "ERROR 004\n";
							descriptor << sendmsg;
							bWait= false;
						}
						break;

					case 2:
						double nValue;

						nValue= atof(buffer.c_str());
						port->setValue(nValue);
						sendmsg= "done";
	#ifdef SERVERDEBUG
						cout << "send: " << sendmsg << endl;
	#endif
						sendmsg+= "\n";
						descriptor << sendmsg;
						bWait= false;
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

				Database* db= Database::instance();

				db->needSubroutines(descriptor.getClientID(), "newentrys");
				sendmsg= "done\n";
				descriptor << sendmsg;
	#ifdef SERVERDEBUG
				cout << "send: done" << endl;
	#endif

			}else if(input.substr(0, 5) == "HEAR ")
			{
				bool bCorrect;
				string entry;
				string groups;
				vector<string> split;
				meash_t* pCurMeas;
				portBase* port;
				UserManagement* user= UserManagement::instance();
				Database* db= Database::instance();

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
					LOG(LOG_ERROR, msg);
					sendmsg= "ERROR 003 1\n";
					descriptor << sendmsg;
	#ifdef SERVERDEBUG
					cout << "send: ERROR 003 1" << endl;
	#endif
				}else
				{
					pCurMeas= getMeasurePort(split[0]);
					if(pCurMeas == NULL)
					{
						string msg;

						msg+= "client ask for >> ";
						msg+= input;
						msg+= "cannot found folder ";
						msg+= split[0];
						msg+= "\nsend ERROR 004";
						LOG(LOG_ERROR, msg);
	#ifdef SERVERDEBUG
						cout << "send: ERROR 004" << endl;
	#endif
						sendmsg= "ERROR 004\n";
						descriptor << sendmsg;
					}else
					{
						port= pCurMeas->pMeasure->getPortClass(split[1], bCorrect);
						if(port == NULL)
						{
							string msg;

							msg+= "client ask for '";
							msg+= input + "'\n";
							msg+= "cannot find subroutine ";
							msg+= split[1];
							msg+= " in folder ";
							msg+= split[0];
							msg+= "\nsend ERROR 005";
							LOG(LOG_ERROR, msg);
							sendmsg= "ERROR 005\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cerr << msg << endl;
	#endif
						}else if(!bCorrect)
						{
							string msg;

							msg+= "client ask for '";
							msg+= input + "'\n";
							msg+= "user '";
							msg+= descriptor.getString("username") + "' but server has no correct chip contact\n";
							msg+= "send ERROR 014";
							LOG(LOG_ERROR, msg);
							sendmsg= "ERROR 014\n";
							descriptor << sendmsg;
	#ifdef SERVERDEBUG
							cerr << msg << endl;
	#endif
						}else
						{
							groups= port->getPermissionGroups();
							if(user->hasPermission(descriptor.getString("username"), groups, "read"))
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
								LOG(LOG_ERROR, msg);
								sendmsg= "ERROR 013\n";
								descriptor << sendmsg;
		#ifdef SERVERDEBUG
								cerr << msg << endl;
		#endif
							}
						}
					}
				}
			}else
			{
				string msg("can not identify command '");

				msg+= input;
				msg+= "'";
				msg+= "\nsend ERROR 002";
				LOG(LOG_ERROR, msg);
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

	meash_t* ServerTransaction::getMeasurePort(string folder)
	{
			meash_t* pCurMeas= meash_t::firstInstance;

			while(pCurMeas)
			{
				if(pCurMeas->pMeasure->getThreadName() == folder)
					break;
				pCurMeas= pCurMeas->next;
			}
			/*if(!bFoundFolder)
			{
				pCurrentFolder= m_ptFolderStart;
				while(pCurrentFolder != NULL)
				{
					if(pCurrentFolder->name == folder[0])
					{
						bFoundFolder= true;
						break;
					}
					pCurrentFolder= pCurrentFolder->next;
				}
				if(pCurrentFolder == NULL)
				}
			}
		}else*/

		return pCurMeas;
	}

	bool ServerTransaction::getDirectory(string filter, string verz, vector<string> &list)
	{
		//struct dirent **namelist;
		struct dirent *dirName;
		struct stat fileStat;
		string path(meash_t::clientPath);
		string folder;
		//int result;
		size_t filterLen= filter.length();
		DIR *dir;

		if(	path.substr(path.length()-1, 1) != "/"
			&&
			verz.length() > 0
			&&
			verz.substr(0, 1) != "/"				)
		{
			path+= "/";
		}
		path+= verz;
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
