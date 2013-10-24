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

#include "../../../pattern/util/LogHolderPattern.h"

#include "../../../pattern/server/IClientPattern.h"
#include "../../../pattern/server/IClientHolderPattern.h"
#include "../../../pattern/server/IServerPattern.h"

#include "../../../util/structures.h"

#include "ServerMethodTransaction.h"
#include "ServerThread.h"
#include "communicationthreadstarter.h"

using namespace std;
//using namespace user;
//using namespace util;
using namespace design_pattern_world::server_pattern;
using namespace boost;
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
		// other client wait for answer
		descriptor.setBoolean("clientwait", false);
		// whether own client should send an array, this is the ending string
		descriptor.setString("endstring", "");
		// last question when own client send an answer array
		descriptor.setString("lastquestion", "");
		// set by sending question for read answer same syncID should be set
		descriptor.setULongLong("questionID", 0);

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		descriptor.setBoolean("output", false);
#endif // __FOLLOWSERVERCLIENTTRANSACTION

		return true;
	}

	bool ServerMethodTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool bRun= true;
		bool bwait= true;
		string::size_type pos;
		string input, wasinput;
		string process;
		string client;
		string endString;
		IServerPattern* server= NULL;

		descriptor >> input;
		if(descriptor.eof())
		{
			unsigned int ID;
			string process, client;
			ostringstream msg;
			IMethodStringStream oInit("init");

			ID= descriptor.getClientID();
			process= descriptor.getString("process");
			client= descriptor.getString("client");
			msg << "WARNING: conection " << ID;
			msg << " in " << descriptor.getServerObject()->getName();
			msg << " from client " << client;
			msg << " in process " << process << " is broken by";
			if(descriptor.error())
				msg << " an undefined ERROR";
			else
				msg << " ending of stream";
			msg << endl << "         so close connection";
			LOG(LOG_INFO, msg.str());
#ifdef ALLOCATEONMETHODSERVER
			msg << endl;
			if(descriptor.getServerObject()->getName() == ALLOCATEONMETHODSERVER)
				cerr << msg.str();
#endif // ALLOCATEONMETHODSERVER
			descriptor.setBoolean("access", false);
			oInit.createSyncID();
			input= descriptor.sendToOtherClient(client, oInit, true, "");
			oInit.removeSyncID();
			connectionEnding(ID, process, client);
			dissolveConnection(descriptor);
			return false;
		}
		wasinput= input;
		trim(input);
		//input= ConfigPropertyCasher::trim(input, " \t\r\n");
		//cout << "input string: '" << input << "' from " << descriptor.getString("client") << endl;
		if(descriptor.getBoolean("asker"))
		{
			pos= input.find(' ', 0);
			bwait= false;
			if(pos != string::npos)
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
				if(input.substr(0, 4) == "true")
				{
					input= input.substr(5);
					pos= input.find(' ', 0);
					if(pos > 0)
					{
						endString= input.substr(0, pos);
						input= input.substr(pos + 1);
					}

				}else if(input.substr(0, 5) == "false")
				{
					input= input.substr(6);
				}

			// DEBUG OUTPUT
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
				descriptor << "ERROR 002\n";
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
			allocateConnection(descriptor);
#ifdef ALLOCATEONMETHODSERVER
			if(descriptor.getServerObject()->getName() == ALLOCATEONMETHODSERVER)
			{
				cout << "allocate ";
				if(descriptor.getBoolean("asker"))
					cout << "sending ";
				else
					cout << "answer ";
				cout << "conection " << descriptor.getClientID();
				cout << " in " << descriptor.getServerObject()->getName();
				cout << " from client " << descriptor.getString("client");
				cout << " in process " << descriptor.getString("process") << endl;
			}
#endif // ALLOCATEONMETHODSERVER
			return true;
		}
		if(	client == ""
			&&
			input == "ending"			)
		{
#ifdef ALLOCATEONMETHODSERVER
			if(descriptor.getServerObject()->getName() == ALLOCATEONMETHODSERVER)
			{
				cout << "finish connection with ID " << descriptor.getClientID();
				cout << "  from client " << descriptor.getString("client");
				cout << " in process " << descriptor.getString("process") << endl;
			}
#endif // ALLOCATEONMETHODSERVER
			dissolveConnection(descriptor);
			descriptor.setBoolean("asker", true);
			descriptor.setBoolean("access", false);
			descriptor.setBoolean("own", false);
			descriptor.setString("client", "");
			descriptor << "done\n";
			descriptor.flush();
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
				while(!server->getCommunicationFactory()->stopCommunicationThreads(descriptor.getClientID(), /*wait*/true))
					{};
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
			unsigned long long nSync;
			IMethodStringStream ianswer(input);

			bwait= descriptor.getBoolean("clientwait");
			endString= descriptor.getString("endstring");
			nSync= descriptor.getULongLong("questionID");
			if(nSync > 0)
			{
				ianswer.createSyncID(nSync);
				input= ianswer.str(true);
			}
			descriptor.setULongLong("questionID", 0);

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			m_boutput= true;
#ifndef __FOLLOW_TOPROCESS
#ifndef __FOLLOW_TOCLIENT
#ifndef __FOLLOW_SENDMESSAGE
#ifdef __FOLLOW_FROMPROCESS
			//if(descriptor.getString("process") != __FOLLOW_TOPROCESS)
				m_boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_FROMCLIENT
			//if(descriptor.getString("client") != __FOLLOW_TOCLIENT)
				m_boutput= false;
#endif // __FOLLOW_FROMCLIENT
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_TOCLIENT
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOPROCESS
			if(descriptor.getString("process") != __FOLLOW_TOPROCESS)
				m_boutput= false;
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOCLIENT
			if(descriptor.getString("client") != __FOLLOW_TOCLIENT)
				m_boutput= false;
#endif // __FOLLOW_TOCLIENT
			descriptor.setBoolean("output", m_boutput);
			if(m_boutput)
			{ // DEBUG on command line
			  // (2-3.) answer client give answer with syncID
				cout << descriptor.getString("process") << "::" << descriptor.getString("client");
				cout << " (2-3.) give Answer '" << ianswer.str(true) << "'" << endl;
				if(bwait)
				{
					cout << " and client waiting for ";
					if(endString == "")
						cout << "this normally string" << endl;
					else
						cout << "an array with ending string '" << endString << "'" << endl;
				}else
					cout << " but client do not wait for any answer" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			if(bwait)
				descriptor.sendAnswer(input);
			if(	endString != "" &&
				endString == input	)
			{
				bwait= false;
				endString= "";
				descriptor.setBoolean("clientwait", false);
				descriptor.setString("endstring", "");
				descriptor.setString("lastquestion", "");
			}
			if(	endString == ""	||
				bwait				)
			{// asking only for new question when last asking for no array
			 // or other client waiting for answer from this array question
				input= descriptor.getOtherClientString(bwait, endString, /*wait now*/true);
				descriptor.setBoolean("clientwait", bwait);
				descriptor.setString("endstring", endString);
				pos= input.find(' ', 0);
				if(pos == string::npos)
					descriptor.setString("lastquestion", input);
				else
					descriptor.setString("lastquestion", input.substr(0, pos));
			}else
			{
				input= descriptor.getString("lastquestion");
			}

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
#ifdef __FOLLOW_SENDMESSAGE
			string sendmsg(__FOLLOW_SENDMESSAGE);

			if(input.substr(0, sendmsg.length()) != sendmsg)
				m_boutput= false;
#endif // __FOLLOW_SENDMASSAGE
			descriptor.setBoolean("output", m_boutput);
			if(m_boutput)
			{ // DEBUG display
				  // (2-2.) answer client get question with syncID
				cout << descriptor.getString("process") << "::" << descriptor.getString("client");
				cout << " (2-2.) get question '" << input << "'" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			if(	input == ""
				||
				input.substr(input.size() -1) != "\n"	)
			{
				input+= "\n";
			}
			IMethodStringStream iquestion(input);

			descriptor.setULongLong("questionID", iquestion.getSyncID());
			descriptor << iquestion.str();
			descriptor.flush();
		}else if(client == descriptor.getServerObject()->getName())
		{// question is sending to own object depend from this class
			IMethodStringStream method(input);

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			bool boutput= true;

#ifdef __FOLLOW_FROMPROCESS
			if(descriptor.getString("process") != __FOLLOW_FROMPROCESS)
				boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_FROMCLIENT
			if(descriptor.getString("client") != __FOLLOW_FROMCLIENT)
				boutput= false;
#endif // __FOLLOW_FROMCLIENT
#ifdef __FOLLOW_SENDMESSAGE
			string sendmsg(__FOLLOW_SENDMESSAGE);

			if(input.substr(0, sendmsg.length()) != sendmsg)
				boutput= false;
#endif // __FOLLOW_SENDMESSAGE
			if(boutput)
			{ // DEBUG display
			  // (1-1.) send question to own client and need first no syncID
				cout << descriptor.getString("process") << "::" << descriptor.getString("client");
				cout << " (1-1.) send question '" << method.str(true) <<"' to own client " << client << endl;
			}
			descriptor.setBoolean("output", boutput);
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			bRun= transfer(descriptor, method);

		}else
		{ // else sentence sending first question from process
			IMethodStringStream method(input);

			trim(input);
			if(input != "")
				method.createSyncID();

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			bool boutput= true;
#ifdef __FOLLOW_FROMPROCESS
			if(descriptor.getString("process") != __FOLLOW_FROMPROCESS)
				boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_FROMCLIENT
			if(descriptor.getString("client") != __FOLLOW_FROMCLIENT)
				boutput= false;
#endif // __FOLLOW_FROMCLIENT
#ifdef __FOLLOW_SENDMESSAGE
			string sendmsg(__FOLLOW_SENDMESSAGE);

			if(input.substr(0, sendmsg.length()) != sendmsg)
				boutput= false;
#endif // __FOLLOW_SENDMESSAGE
			if(boutput)
			{ // DEBUG display fghdghhf
			  // (2-1.) send question to other client with created syncID
				cout << descriptor.getString("process") << "::" << descriptor.getString("client");
				cout << " (2-1.) send question '" << method.str(true) <<"' to other client " << client;
				if(!bwait)
					cout << " and need no answer";
				cout << endl;
			}
			descriptor.setBoolean("output", boutput);
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			input= descriptor.sendToOtherClient(client, method, bwait, endString);

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
#ifndef __FOLLOW_FROMPROCESS
#ifndef __FOLLOW_FROMCLIENT
#ifndef __FOLLOW_SENDMESSAGE
#ifdef __FOLLOW_TOCLIENT
			if(client != __FOLLOW_TOCLIENT)
				boutput= false;
#endif
#ifdef __FOLLOW_TOPROCESS
			boutput= false;
#endif
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_FROMCLIENT
#endif // __FOLLOW_FROMPROCESS

			if(boutput)
			{ // DEBUG display
			  // (2-4.) send answer back to asker and remove syncID
				IMethodStringStream oAnswer(input);

				cout << process << "::" << client;
				cout << " (2-4.) send answer '" << oAnswer.str(true) << "' ";
				cout << "as '" << oAnswer.str(false) << "' ";
				cout << "back to " << descriptor.getString("process") << "::";
				cout << descriptor.getString("client") << endl;
				cout << " and remove synchronization ID " << method.getSyncID() << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			IMethodStringStream oAnswer(input);

			// delete now synchronization ID
			// created by send question to own or other client
			oAnswer.removeSyncID();
			input= oAnswer.str();
			trim(input);
			if(	input == "" ||
				input.substr(input.size() -1) != "\n"	)
			{
				input+= "\n";
			}
			descriptor << input;
			descriptor.flush();
		}
		return bRun;
	}

	bool ServerMethodTransaction::transfer(IFileDescriptorPattern& descriptor, IMethodStringStream& method)
	{
		// dummy method to overwrite from any child class
		descriptor << "ERROR 003";
		descriptor.endl();
		descriptor.flush();
		return true;
	}

	string ServerMethodTransaction::getTransactionName(const IFileDescriptorPattern& descriptor) const
	{
		return descriptor.getString("client");
	}

	bool ServerMethodTransaction::isClient(const IFileDescriptorPattern& descriptor, const string& definition) const
	{
	/*	if(!descriptor.getBoolean("asker"))
		{// debug messages
			cout << "is client '" << descriptor.getString("client") << "'" << endl;
			cout << "the same  '" << definition << "'" << endl;
		}else
			cout << "client '" << descriptor.getString("client") << "' is an asker" << endl;*/
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
		case 2:
			str= "ERROR: connection was opened with wrong command";
			break;
		case 3:
			str= "class have no own commands to administered";
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

	inline unsigned int ServerMethodTransaction::getMaxErrorNums(const bool byerror) const
	{
		if(byerror)
			return 10;
		return 0;
	}

	ServerMethodTransaction::~ServerMethodTransaction()
	{
	}

}
