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

#include "debugtransaction.h"
#include "ExternClientInputTemplate.h"
#include "Thread.h"

namespace util
{
	ExternClientInputTemplate::ExternClientInputTemplate(const string& process, const string& client, IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection)
	:	m_sProcess(process),
		m_sName(client),
		m_oSendConnect(sendConnection),
		m_oGetConnect(getConnection),
		m_sOpenSendCommand(process + ":" + client + " SEND"),
		m_sEndingSendCommand("ending"),
		m_sOpenGetCommand(process + ":" + client + " GET"),
		m_sEndingGetCommand("ending"),
		m_pSendTransaction(NULL),
		m_pGetTransaction(NULL)
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		,m_boutput(false)
#endif // __FOLLOWSERVERCLIENTTRANSACTION
	{
		m_SENDMETHODLOCK= Thread::getMutex("SENDMETHODLOCK");
		m_GETQUESTIONLOCK= Thread::getMutex("GETQUESTIONLOCK");
	}

	int ExternClientInputTemplate::openSendConnection(string toopen/*= ""*/)
	{
		if(m_oSendConnect == NULL)
			return 1;
		return openSendConnection(m_oSendConnect->getTimeout(), toopen);
	}

	int ExternClientInputTemplate::openSendConnection(const unsigned int timeout, string toopen/*= ""*/)
	{
		int nRv= 0, err;
		unsigned int old;

		LOCK(m_SENDMETHODLOCK);
		if(m_pSendTransaction)
		{
			UNLOCK(m_SENDMETHODLOCK);
			return -1;
		}
		if(m_oSendConnect == NULL)
		{
			LOCK(m_SENDMETHODLOCK);
			return 1;
		}
		if(toopen == "")
			toopen= m_sOpenSendCommand;
		old= m_oSendConnect->getTimeout();
		if(old != timeout)
			m_oSendConnect->setTimeout(timeout);
		m_pSendTransaction= new OutsideClientTransaction();
		if(toopen != "")
		{
			m_oSendConnect->newTranfer(m_pSendTransaction, true);
			m_pSendTransaction->setCommand(toopen);
			nRv= m_oSendConnect->init();
			if(old != timeout)
				m_oSendConnect->setTimeout(old);
			if(nRv > 0)
			{
				m_oSendConnect->newTranfer(NULL, /*delete old*/true);
				m_pSendTransaction= NULL;
				UNLOCK(m_SENDMETHODLOCK);
				return nRv += getMaxErrorNums(/*error*/true);
			}
			err= error(m_pSendTransaction->getReturnedString()[0]);
			if(err != 0)
			{
				err+= (err > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
				err+= (err > 0 ? m_oSendConnect->getMaxErrorNums(true) : (m_oSendConnect->getMaxErrorNums(false) * -1));
				UNLOCK(m_SENDMETHODLOCK);
				return err;
			}
			if(nRv < 0)
				nRv-= getMaxErrorNums(false);
			UNLOCK(m_SENDMETHODLOCK);
			return nRv;
		}
		return 0;
	}

	string ExternClientInputTemplate::sendMethod(const string& toProcess, const OMethodStringStream& method,
													const bool answer/*= true*/)
	{
		vector<string> result;

		result= sendMethod(toProcess, method, "", answer);
		return result[0];
	}

	vector<string> ExternClientInputTemplate::sendMethod(const string& toProcess, const OMethodStringStream& method,
															const string& done, const bool answer/*= true*/)
	{
		int ret;
		string command;
		vector<string> result;
		vector<string>::iterator it;


		LOCK(m_SENDMETHODLOCK);
		if(m_oSendConnect == NULL)
		{
			result.push_back("ERROR 001");
			UNLOCK(m_SENDMETHODLOCK);
			return result;
		}
		if(m_pSendTransaction == NULL)
		{
			result.push_back("ERROR 002");
			UNLOCK(m_SENDMETHODLOCK);
			return result;
		}
		command= toProcess + " ";
		if(answer)
			command+= "true ";
		else
			command+= "false ";
		command+= method.str();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			bool boutput= true;

#ifndef __FOLLOW_FROMPROCESS
#ifndef __FOLLOW_FROMCLIENT
#ifndef __FOLLOW_SENDMESSAGE
			boutput= false;
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_FROMCLIENT
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_FROMPROCESS
			if(m_sProcess != __FOLLOW_FROMPROCESS)
				boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_FROMCLIENT
			if(m_sName != __FOLLOW_FROMCLIENT)
				boutput= false;
#endif // __FOLLOW_FROMCLIENT
#ifdef __FOLLOW_SENDMESSAGE
			string sendmsg(__FOLLOW_SENDMESSAGE);

			if(method.str().substr(0, sendmsg.length()) != sendmsg)
				boutput= false;
#endif // __FOLLOW_SENDMESSAGE
			if(boutput)
			{ // DEBUG display
				cout << "Interface " << m_sProcess << "::" << m_sName;
				cout << " sending method '" << method.str() <<"'" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		m_pSendTransaction->setCommand(command, done);
		ret= m_oSendConnect->init();
		if(ret > 0)
		{
			UNLOCK(m_SENDMETHODLOCK);
			result.push_back(error(ret + getMaxErrorNums(true)));
			return result;
		}
		result= m_pSendTransaction->getReturnedString();
		UNLOCK(m_SENDMETHODLOCK);
		if(ret == -2)
			closeSendConnection();
		it= result.begin();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		if(boutput)
		{ // DEBUG display
			cout << "Interface " << m_sProcess << "::" << m_sName;
			cout << " get answer:" << endl;
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		while(it != result.end())
		{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(boutput)
			{ // DEBUG display
				cout << "             '" << *it << "'" << endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			ret= error(*it);
			if(ret != 0)
			{
				it= result.erase(it);
				ret+= (ret > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
				ret+= (ret > 0 ? m_oSendConnect->getMaxErrorNums(true) : (m_oSendConnect->getMaxErrorNums(false) * -1));
				result.insert(it, error(ret));
			}
			++it;
		}
		return result;
	}

	int ExternClientInputTemplate::closeSendConnection()
	{
		int nRv;

		LOCK(m_SENDMETHODLOCK);
		nRv= closeConnection(m_oSendConnect, m_pSendTransaction, m_sEndingSendCommand);
		m_pSendTransaction= NULL;
		UNLOCK(m_SENDMETHODLOCK);
		return nRv;
	}

	int ExternClientInputTemplate::closeConnection(IClientConnectArtPattern* connection, OutsideClientTransaction* transaction, const string& command)
	{
		int nRv= 0;

		if(connection == NULL)
			return 1;
		if(transaction == NULL)
			return 2;
		transaction->closeConnection(command);
		if(!connection->init())
			nRv= 3;
		connection->close();
		connection->newTranfer(NULL, /*delete old*/true);
		return nRv;
	}

	int ExternClientInputTemplate::openGetConnection()
	{
		int nRv;
		vector<string> answer;

		LOCK(m_GETQUESTIONLOCK);
		if(m_oGetConnect == NULL)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			return 1;
		}
		if(m_pGetTransaction == NULL)
		{
			m_pGetTransaction= new OutsideClientTransaction();
			if(m_sOpenGetCommand != "")
			{
				m_oGetConnect->newTranfer(m_pGetTransaction, true);
				m_pGetTransaction->setCommand(m_sOpenGetCommand);
				nRv= m_oGetConnect->init();
				if(nRv > 0)
				{
					m_oGetConnect->newTranfer(NULL, /*delete old*/true);
					m_pGetTransaction= NULL;
					UNLOCK(m_GETQUESTIONLOCK);
					return nRv + getMaxErrorNums(true);
				}
				answer= m_pGetTransaction->getReturnedString();
				if(answer[0] != "done")
				{
					int err;

					UNLOCK(m_GETQUESTIONLOCK);
					err= error(answer[0]);
					if(err != 0)
					{
						err+= (err > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
						err+= (err > 0 ? m_oGetConnect->getMaxErrorNums(true) : (m_oGetConnect->getMaxErrorNums(false) * -1));
						return err;
					}
					if(nRv < 0)
						return nRv - getMaxErrorNums(false);
				}
			}
		}
		UNLOCK(m_GETQUESTIONLOCK);
		return 0;
	}
	string ExternClientInputTemplate::getQuestion(const string& lastAnswer)
	{
		int err;
		vector<string> answer;

		LOCK(m_GETQUESTIONLOCK);
		if(m_oGetConnect == NULL)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			return "ERROR 001";
		}
		if(m_pGetTransaction == NULL)
		{
			err= openGetConnection();
			if(err != 0)
			{
				UNLOCK(m_GETQUESTIONLOCK);
				return error(err);
			}
		}
		m_pGetTransaction->setCommand(lastAnswer);
		err= m_oGetConnect->init();
		if(err != 0)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			err+= (err > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
			return error(err);
		}

	// this part of writing messages to output gives some ERROR
	// Although the variable m_boutput in the constructor is set to false, it is true in the processing
	// of changes in this variable some problems of connection are shown
#if 0
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		if(m_boutput)
		{ // DEBUG display
			cout << "Reached client " << m_sProcess << "::" << m_sName;
			cout << " give Answer '" << lastAnswer << "'" << endl;
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
#endif
		answer= m_pGetTransaction->getReturnedString();
#if 0
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		m_boutput= true;
#ifndef __FOLLOW_TOPROCESS
#ifndef __FOLLOW_TOCLIENT
#ifndef __FOLLOW_SENDMESSAGE
		m_boutput= false;
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_TOCLIENT
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOPROCESS
		if(m_sProcess != __FOLLOW_TOPROCESS)
			m_boutput= false;
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOCLIENT
		if(m_sName != __FOLLOW_TOCLIENT)
			m_boutput= false;
#endif // __FOLLOW_TOCLIENT
#ifdef __FOLLOW_SENDMESSAGE
		string sendmsg(__FOLLOW_SENDMESSAGE);

		if(answer[0].substr(0, sendmsg.length()) != sendmsg)
			m_boutput= false;
#endif // __FOLLOW_SENDMASSAGE
		if(m_boutput)
		{ // DEBUG display
			cout << "Reached client " << m_sProcess << "::" << m_sName;
			cout << " get question '" << answer[0] << "'" << endl;
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
#endif
		UNLOCK(m_GETQUESTIONLOCK);
		err= error(answer[0]);
		if(err != 0)
		{
			err+= (err > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
			err+= (err > 0 ? m_oGetConnect->getMaxErrorNums(true) : (m_oGetConnect->getMaxErrorNums(false) * -1));
			return error(err);
		}
		return answer[0];
	}

	int ExternClientInputTemplate::closeGetConnection()
	{
		int nRv;

		LOCK(m_GETQUESTIONLOCK);
		nRv= closeConnection(m_oGetConnect, m_pGetTransaction, m_sEndingGetCommand);
		m_pGetTransaction= NULL;
		UNLOCK(m_GETQUESTIONLOCK);
		return nRv;
	}

	inline int ExternClientInputTemplate::error(const string& input)
	{
		int number;
		string answer;
		istringstream out(input);

		out >> answer >> number;
		if(answer == "ERROR")
			return number;
		if(answer == "WARNING")
			return number * -1;
		return 0;
	}

	inline string ExternClientInputTemplate::error(int err)
	{
		ostringstream in;

		if(err < 0)
		{
			err*= -1;
			in << "WARNING ";
		}else
			in << "ERROR ";
		in.width(3);
		in.fill('0');
		in << err;
		return in.str();
	}

	string ExternClientInputTemplate::strerror(int err, const bool bSend/*= true*/)
	{
		int min, max;
		string str;

		switch(err)
		{
		case 0:
			str= "no error occurred";
			break;
		case -1:
		str= "WARNING: connection exist before";
			break;
		case 1:
			str= "ERROR: no <code>IClientConnectArtPattern</code> be given for sending";
			break;
		case 2:
			str= "cannot connect with server, or initialization was fail";
			break;
		default:
			min= getMaxErrorNums(false) * -1;
			max= getMaxErrorNums(true);
			if(	err > max
				||
				err < min	)
			{
				err-= (err > 0 ? max : min);
				min= 0;
				max= 0;
				if(bSend && m_oSendConnect)
				{
					min= m_oSendConnect->getMaxErrorNums(/*error*/false) * -1;
					max= m_oSendConnect->getMaxErrorNums(/*error*/true);

				}else if(m_oGetConnect)
				{
					min= m_oGetConnect->getMaxErrorNums(/*error*/false) * -1;
					max= m_oGetConnect->getMaxErrorNums(/*error*/true);

				}
				if(err >= min && err <= max)
				{
					if(bSend && m_oSendConnect)
					{
						str= m_oSendConnect->strerror(err);
						break;

					}else if(m_oGetConnect)
					{
						str= m_oGetConnect->strerror(err);
						break;
					}
				}else
				{
					bool open= false;
					string answer;
					OMethodStringStream command("getMinMaxErrorNums");

					err-= (err > 0 ? max : min);
					min= 0;
					max= 0;
					if(bSend && m_oSendConnect)
					{
						if(!m_pSendTransaction)
						{
							openSendConnection();
							open= true;
						}
						min= m_pSendTransaction->getMaxErrorNums(/*error*/false) * -1;
						max= m_pSendTransaction->getMaxErrorNums(/*error*/true);
						if(err >= min && err <= max)
						{
							str= m_pSendTransaction->strerror(err);
							break;
						}
						answer= sendMethod(m_sName, command);
						if(open)
							closeSendConnection();

					}else if(m_oGetConnect)
					{
						if(!m_pGetTransaction)
						{
							openGetConnection();
							open= true;
						}
						min= m_pGetTransaction->getMaxErrorNums(/*error*/false) * -1;
						max= m_pGetTransaction->getMaxErrorNums(/*error*/true);
						if(err >= min && err <= max)
						{
							str= m_pGetTransaction->strerror(err);
							break;
						}
						answer= sendMethod(m_sName, command, true);
						if(open)
							closeGetConnection();
					}
					err-= (err > 0 ? max : min);
					if(error(answer) == 0)
					{
						int err2;
						OMethodStringStream command("getErrorString");
						istringstream ianswer(answer);

						ianswer >> min;
						ianswer >> max;
						if(err >= min && err <= max)
						{
							min= 0;
							max= 0;
							command << err;
							if(bSend && m_oSendConnect)
							{
								if(!m_pSendTransaction)
								{
									openSendConnection();
									open= true;
								}
								str= sendMethod(m_sName, command, /*answer*/true);
								if(open)
									closeSendConnection();
								err2= error(str);
								if(err2 == 0 && str != "")
									break;

							}if(m_oGetConnect)
							{
								if(!m_pGetTransaction)
								{
									openGetConnection();
									open= true;
								}
								str= getQuestion(command.str());
								if(open)
									closeGetConnection();
								err2= error(str);
								if(err2 == 0 && str != "")
									break;
							}
						}
					}
				}
			}
			if(err > 0)
				str= "undefined error occurred";
			else
				str= "undefined warning occurred";
		}
		return str;
	}

	unsigned int ExternClientInputTemplate::getMaxErrorNums(const bool byerror) const
	{
		unsigned int nRv;

		if(byerror)
			nRv= 10;
		else
			nRv= 10;
		return nRv;
	}

	ExternClientInputTemplate::~ExternClientInputTemplate()
	{
		closeSendConnection();
		closeGetConnection();
		if(m_oSendConnect)
			delete m_oSendConnect;
		if(m_oGetConnect)
			delete m_oGetConnect;
		DESTROYMUTEX(m_SENDMETHODLOCK);
		DESTROYMUTEX(m_GETQUESTIONLOCK);
	}
}
