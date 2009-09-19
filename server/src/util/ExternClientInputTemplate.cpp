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

#include "ExternClientInputTemplate.h"
#include "Thread.h"

namespace util
{
	ExternClientInputTemplate::ExternClientInputTemplate(const string& process, const string& client, IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection)
	:	m_sProcess(process),
		m_sName(client),
		m_oSendConnect(sendConnection),
		m_oGetConnect(getConnection),
		m_pSendTransaction(NULL),
		m_pGetTransaction(NULL)
	{
		m_SENDMETHODLOCK= Thread::getMutex("SENDMETHODLOCK");
		m_GETQUESTIONLOCK= Thread::getMutex("GETQUESTIONLOCK");
	}

	int ExternClientInputTemplate::openSendConnection()
	{
		int nRv= 0;

		if(m_pSendTransaction)
			return -1;
		if(m_oSendConnect == NULL)
			return 1;
		m_pSendTransaction= new OutsideClientTransaction();
		m_oSendConnect->newTranfer(m_pSendTransaction, true);
		m_pSendTransaction->setCommand(m_sProcess + ":" + m_sName + " SEND");
		nRv= m_oSendConnect->init();
		if(nRv > 0)
		{
			m_oSendConnect->newTranfer(NULL, /*delete old*/true);
			m_pSendTransaction= NULL;
			return nRv + 10;
		}
		return error(m_pSendTransaction->getReturnedString()[0]);
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

		if(m_oSendConnect == NULL)
		{
			result.push_back("ERROR 001");
			return result;
		}
		if(m_pSendTransaction == NULL)
		{
			result.push_back("ERROR 002");
			return result;
		}
		command= toProcess + " ";
		if(answer)
			command+= "true ";
		else
			command+= "false ";
		command+= method.str();
		LOCK(m_SENDMETHODLOCK);
		m_pSendTransaction->setCommand(command, done);
		ret= m_oSendConnect->init();
		if(ret > 0)
		{
			UNLOCK(m_SENDMETHODLOCK);
			result.push_back(error(ret + 10));
			return result;
		}
		result= m_pSendTransaction->getReturnedString();
		UNLOCK(m_SENDMETHODLOCK);
		if(ret == -2)
			closeSendConnection();
		return result;
	}

	int ExternClientInputTemplate::closeSendConnection()
	{
		int nRv;

		nRv= closeConnection(m_oSendConnect, m_pSendTransaction);
		m_pSendTransaction= NULL;
		return nRv;
	}

	int ExternClientInputTemplate::closeConnection(IClientConnectArtPattern* connection, OutsideClientTransaction* transaction)
	{
		int nRv= 0;

		if(connection == NULL)
			return 1;
		if(transaction == NULL)
			return 2;
		transaction->closeConnection();
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

		if(m_oGetConnect == NULL)
			return 1;
		if(m_pGetTransaction == NULL)
		{
			m_pGetTransaction= new OutsideClientTransaction();
			m_oGetConnect->newTranfer(m_pGetTransaction, true);
			m_pGetTransaction->setCommand(m_sProcess + ":" + m_sName + " GET");
			nRv= m_oGetConnect->init();
			if(nRv > 0)
			{
				m_oGetConnect->newTranfer(NULL, /*delete old*/true);
				m_pGetTransaction= NULL;
				return nRv + 10;
			}
			answer= m_pGetTransaction->getReturnedString();
			if(answer[0] != "done")
				return error(answer[0]);
		}
		return 0;
	}
	string ExternClientInputTemplate::getQuestion(const string& lastAnswer)
	{
		int err;
		vector<string> answer;

		if(m_oGetConnect == NULL)
			return "ERROR 001";
		LOCK(m_GETQUESTIONLOCK);
		if(m_pGetTransaction == NULL)
		{
			err= openGetConnection();
			if(err != 0)
			{
				UNLOCK(m_GETQUESTIONLOCK);
				return error(err);
			}
		}else
			m_pGetTransaction->setCommand(lastAnswer);
		err= m_oGetConnect->init();
		if(err != 0)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			return error(err + 10);
		}
		answer= m_pGetTransaction->getReturnedString();
		UNLOCK(m_GETQUESTIONLOCK);
		return answer[0];
	}

	int ExternClientInputTemplate::closeGetConnection()
	{
		int nRv;

		nRv= closeConnection(m_oGetConnect, m_pGetTransaction);
		m_pGetTransaction= NULL;
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

	string ExternClientInputTemplate::strerror(int error) const
	{
		string str;

		switch(error)
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
			if(	error > 10
				||
				error < -10	)
			{
				error= (error > 0 ? error - 10 : error + 10);
				if(m_oGetConnect)
				{
					str= m_oGetConnect->strerror(error);
					break;
				}else if(m_oSendConnect)
				{
					str= m_oSendConnect->strerror(error);
					break;
				}
			}
			if(error > 0)
				str= "undefined error occurred";
			else
				str= "undefined warning occurred";
		}
		return str;
	}

	ExternClientInputTemplate::~ExternClientInputTemplate()
	{
		closeSendConnection();
		closeGetConnection();
		DESTROYMUTEX(m_SENDMETHODLOCK);
		DESTROYMUTEX(m_GETQUESTIONLOCK);
	}
}
