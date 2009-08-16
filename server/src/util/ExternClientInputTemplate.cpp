/*
 * ExternClientInputTemplate.cpp
 *
 *  Created on: 20.07.2009
 *      Author: root
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
		return error(m_pSendTransaction->getReturnedString());
	}

	string ExternClientInputTemplate::sendMethod(const string& toProcess, const string& methodString,
													const bool answer/*= true*/)
	{
		string command;
		string result;

		if(m_oSendConnect == NULL)
			return "ERROR 001";
		if(m_pSendTransaction == NULL)
			return "ERROR 002";
		command= toProcess + " ";
		if(answer)
			command+= "true ";
		else
			command+= "false ";
		command+= methodString;
		LOCK(m_SENDMETHODLOCK);
		m_pSendTransaction->setCommand(command);
		if(!m_oSendConnect->init())
		{
			UNLOCK(m_SENDMETHODLOCK);
			return "ERROR 003";
		}
		result= m_pSendTransaction->getReturnedString();
/*		if(result != "")
		{
			cout << "get result: " << result << endl;
			cout << "fail command: " << command << endl;
		}*/
		UNLOCK(m_SENDMETHODLOCK);
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
		string answer;

		if(m_oGetConnect == NULL)
			return 1;
		if(m_pGetTransaction == NULL)
		{
			m_pGetTransaction= new OutsideClientTransaction();
			m_oGetConnect->newTranfer(m_pGetTransaction, true);
			m_pGetTransaction->setCommand(m_sProcess + ":" + m_sName + " GET");
			nRv= m_oGetConnect->init();
			if(nRv > 0)
				return nRv + 10;
			answer= m_pGetTransaction->getReturnedString();
			if(answer != "done")
				return error(answer);
		}
		return 0;
	}
	string ExternClientInputTemplate::getQuestion(const string& lastAnswer)
	{
		string answer;

		if(m_oGetConnect == NULL)
			return "ERROR 001";
		LOCK(m_GETQUESTIONLOCK);
		if(m_pGetTransaction == NULL)
		{
			int err;

			err= openGetConnection();
			if(err != 0)
			{
				UNLOCK(m_GETQUESTIONLOCK);
				return error(err);
			}
		}else
			m_pGetTransaction->setCommand(lastAnswer);
		if(!m_oGetConnect->init())
		{
			UNLOCK(m_GETQUESTIONLOCK);
			return "ERROR 003";
		}
		answer= m_pGetTransaction->getReturnedString();
		UNLOCK(m_GETQUESTIONLOCK);
		return answer;
	}

	int ExternClientInputTemplate::closeGetConnection()
	{
		int nRv;

		nRv= closeConnection(m_oGetConnect, m_pGetTransaction);
		m_pGetTransaction= NULL;
		return nRv;
	}

	int ExternClientInputTemplate::error(const string& input)
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

	string ExternClientInputTemplate::error(int err)
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
