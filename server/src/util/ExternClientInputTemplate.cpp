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
			toopen= m_sProcess + ":" + m_sName + " SEND";
		old= m_oSendConnect->getTimeout();
		if(old != timeout)
			m_oSendConnect->setTimeout(timeout);
		m_pSendTransaction= new OutsideClientTransaction();
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
		while(it != result.end())
		{
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
		nRv= closeConnection(m_oSendConnect, m_pSendTransaction);
		m_pSendTransaction= NULL;
		UNLOCK(m_SENDMETHODLOCK);
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

		LOCK(m_GETQUESTIONLOCK);
		if(m_oGetConnect == NULL)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			return 1;
		}
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
		}else
			m_pGetTransaction->setCommand(lastAnswer);
		err= m_oGetConnect->init();
		if(err != 0)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			err+= (err > 0 ? getMaxErrorNums(true) : (getMaxErrorNums(false) * -1));
			return error(err);
		}
		answer= m_pGetTransaction->getReturnedString();
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
		nRv= closeConnection(m_oGetConnect, m_pGetTransaction);
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

	string ExternClientInputTemplate::strerror(int err)
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
				if(m_oGetConnect)
				{
					min= m_oGetConnect->getMaxErrorNums(/*error*/false) * -1;
					max= m_oGetConnect->getMaxErrorNums(/*error*/true);

				}else if(m_oSendConnect)
				{
					min= m_oSendConnect->getMaxErrorNums(/*error*/false) * -1;
					max= m_oSendConnect->getMaxErrorNums(/*error*/true);
				}
				if(err >= min && err <= max)
				{
					if(m_oGetConnect)
					{
						str= m_oGetConnect->strerror(err);
						break;
					}else if(m_oSendConnect)
					{
						str= m_oSendConnect->strerror(err);
						break;
					}
				}else
				{
					err-= (err > 0 ? max : min);
					min= 0;
					max= 0;
					if(m_pGetTransaction)
					{
						min= m_pGetTransaction->getMaxErrorNums(/*error*/false) * -1;
						max= m_pGetTransaction->getMaxErrorNums(/*error*/true);

					}else if(m_pSendTransaction)
					{
						min= m_pSendTransaction->getMaxErrorNums(/*error*/false) * -1;
						max= m_pSendTransaction->getMaxErrorNums(/*error*/true);
					}
					if(err >= min && err <= max)
					{
						if(m_pGetTransaction)
						{
							str= m_pGetTransaction->strerror(err);
							break;
						}else if(m_pSendTransaction)
						{
							str= m_pSendTransaction->strerror(err);
							break;
						}
					}else
					{
						err-= (err > 0 ? max : min);
						min= 0;
						max= 0;
						if(m_pSendTransaction)
						{
							int err2;
							OMethodStringStream command("getErrorString");

							command << err;
							str= sendMethod(m_sName, command, /*answer*/true);
							err2= error(str);
							if(err2 == 0 && str != "")
								break;
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
		DESTROYMUTEX(m_SENDMETHODLOCK);
		DESTROYMUTEX(m_GETQUESTIONLOCK);
	}
}
