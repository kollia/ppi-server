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

#include "../../../util/debugtransaction.h"
#include "../../../util/exception.h"
#include "../../../util/GlobalStaticMethods.h"
#include "../../../util/thread/Thread.h"

#include "../../../pattern/util/LogHolderPattern.h"
#include "../../../database/logger/lib/logstructures.h"

#include "ExternClientInputTemplate.h"

namespace util
{
	ExternClientInputTemplate::ExternClientInputTemplate(const string& process, const string& client, IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection)
	:	m_pSocketError(EHObj(new SocketErrorHandling)),
	 	m_sProcess(process),
		m_sName(client),
		m_oSendConnect(sendConnection),
		m_oGetConnect(getConnection),
		m_sOpenSendCommand(process + ":" + client + " SEND"),
		m_sEndingSendCommand("ending"),
		m_sOpenGetCommand(process + ":" + client + " GET"),
		m_sEndingGetCommand("ending"),
		m_pSendTransaction(NULL),
		m_pGetTransaction(NULL),
		m_oAnswerSender(client, this)
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		,m_boutput(false)
#endif // __FOLLOWSERVERCLIENTTRANSACTION
	{
		m_SENDMETHODLOCK= Thread::getMutex("SENDMETHODLOCK");
		m_GETQUESTIONLOCK= Thread::getMutex("GETQUESTIONLOCK");
		m_pSocketError= m_oAnswerSender.start();
		if(m_pSocketError->hasError())
		{
			string err;

			m_bNoAnswerSend= false;
			m_pSocketError->addMessage("ExternClientInputTemplate",
							"NoAnswerSender_start", process + "@" + client);
			err+= m_oAnswerSender.getThreadName() + "'\n";
			cerr << "### WARNING: " << err;
			cerr << "              so send this messages directly which has more bad performance" << endl;
			LOG(LOG_WARNING, err +"so send this messages directly which has more bad performance");
		}else
			m_bNoAnswerSend= true;
		//m_bNoAnswerSend= false;
	}

	EHObj ExternClientInputTemplate::openSendConnection(string toopen/*= ""*/)
	{
		if(m_oSendConnect == NULL)
		{
			m_pSocketError->setError("ExternClientInputTemplate",
							"noSendClass");
			return m_pSocketError;
		}
		return openSendConnection(m_oSendConnect->getTimeout(), toopen);
	}

	EHObj ExternClientInputTemplate::openSendConnection(
					const unsigned int timeout, string toopen/*= ""*/)
	{
		unsigned int old;

		LOCK(m_SENDMETHODLOCK);
		if(m_pSendTransaction)
		{
			UNLOCK(m_SENDMETHODLOCK);
			m_pSocketError->setWarning("ExternClientInputTemplate",
									"noTransactionClass");
			return m_pSocketError;
		}
		if(m_oSendConnect == NULL)
		{
			LOCK(m_SENDMETHODLOCK);
			m_pSocketError->setError("ExternClientInputTemplate",
									"noSendClass");
			return m_pSocketError;
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
			m_pSocketError= m_oSendConnect->init();
			if(old != timeout)
				m_oSendConnect->setTimeout(old);
			if(m_pSocketError->hasError())
			{
				m_oSendConnect->newTranfer(NULL, /*delete old*/true);
				m_pSendTransaction= NULL;
			}
		}
		UNLOCK(m_SENDMETHODLOCK);
		return m_pSocketError;
	}

	string ExternClientInputTemplate::sendMethod(const string& toProcess, const OMethodStringStream& method,
													const bool answer/*= true*/)
	{
		vector<string> result;

		result= sendMethod(toProcess, method, "", answer);
		if(result.size() == 0)
			return "";
		return result[0];
	}

	vector<string> ExternClientInputTemplate::sendMethod(const string& toProcess, const OMethodStringStream& method,
															const string& done, const bool answer/*= true*/)
	{
		if(	!answer &&
			m_bNoAnswerSend	)
		{
#ifdef __WRONGPPISERVERSENDING
			if(glob::getProcessName() == "ppi-server")
			{
				ostringstream out;
				out << glob::getProcessName() << " send over NoAnswerSender pool" << endl;
				out << "  '" << method.str() << "'" << endl;
				cerr << out.str();
			}
#endif // __WRONGPPISERVERSENDING
			return m_oAnswerSender.sendMethod(toProcess, method, done);
		}
#ifdef __WRONGPPISERVERSENDING
		if(glob::getProcessName() == "ppi-server")
		{
			ostringstream out;
			out << "Sending directly from " << glob::getProcessName() << "(" << m_sName << ") command " << endl;
			out << "  '" << method.str() << "'" << endl;
			cerr << out.str();
		}
#endif // __WRONGPPISERVERSENDING
		return sendMethodD(toProcess, method, done, answer);
	}

	vector<string> ExternClientInputTemplate::sendMethodD(const string& toProcess, const OMethodStringStream& method,
															const string& done, const bool answer/*= true*/)
	{
		bool bsend;
		int tryrecon= 0;
		string command;
		vector<string> result, fresult;
		vector<string>::iterator it;

		LOCK(m_SENDMETHODLOCK);
		if(m_oSendConnect == NULL)
		{
			SocketErrorHandling error;

			error.setError("ExternClientInputTemplate",
									"noConSendMethod");
			result.push_back(error.getErrorStr());
			UNLOCK(m_SENDMETHODLOCK);
			return result;
		}
		if(m_pSendTransaction == NULL)
		{
			SocketErrorHandling error;

			error.setError("ExternClientInputTemplate",
									"noTransSendMethod");
			result.push_back(error.getErrorStr());
			UNLOCK(m_SENDMETHODLOCK);
			return result;
		}
		command= toProcess + " ";
		if(answer)
			command+= "true ";
		else
			command+= "false ";
		if(done != "")
			command+= "true " + done + " ";
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
		do{
			try{
				m_pSendTransaction->setCommand(command, done);
			}catch(SignalException& ex)
			{
				ostringstream msg;

				msg << "try to set command '" + command + "' with end string '";
				msg << done << "' on OutsideClientTransaction object ";
				msg << m_pSendTransaction;
				ex.addMessage(msg.str());
				throw(ex);
			}
			m_pSocketError= m_oSendConnect->init();
			bsend= true;
			if(m_pSocketError->hasGroupError("IClientConnectArtPattern"))
			{
				UNLOCK(m_SENDMETHODLOCK);
				result.push_back(m_pSocketError->getErrorStr());
				return result;
			}
			result= m_pSendTransaction->getReturnedString();
			UNLOCK(m_SENDMETHODLOCK);
			if(m_pSocketError->hasError())
			{
				/*
				 * this error will be now
				 * only from transaction or higher,
				 * because group of IClientConnectArtPattern
				 * was asked before
				 */
				closeSendConnection();
				if(tryrecon <= 5)
				{
					bool bdone= false;

					for(vector<string>::iterator er= result.begin(); er != result.end(); ++er)
					{
						if(	done == "" ||
							*er == done		)
						{ // after bdone just asked if one result was returned
							bdone= true;

						}
						if(*er == "ERROR 001")
						{ // try to reconnect
							openSendConnection();
							bsend= false;
							if(result.size() > 1)
							{
								result.erase(er);
								fresult.insert(fresult.end(), result.begin(), result.end());
								if(bdone)
									bsend= true;
								break;
							}
							++tryrecon;
							break;
						}
					}
				}
			}
		}while(bsend == false);
		if(!fresult.empty())
			result.insert(result.begin(), fresult.begin(), fresult.end());
		it= result.begin();
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		if(boutput)
		{ // DEBUG display
			cout << "Interface " << m_sProcess << "::" << m_sName;
			cout << " get answer:" << endl;
		}
		while(it != result.end())
		{
			if(boutput)
			{ // DEBUG display
				cout << "             '" << *it << "'" << endl;
			}
			++it;
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		return result;
	}

	EHObj ExternClientInputTemplate::closeSendConnection()
	{
		LOCK(m_SENDMETHODLOCK);
		try{
			if(m_pSendTransaction == NULL)
			{
				/*
				 * no transaction is open
				 * or be closed before
				 * so nothing to do
				 */
				UNLOCK(m_SENDMETHODLOCK);
				return m_pSocketError;
			}
			closeConnection(m_oSendConnect, m_pSendTransaction, m_sEndingSendCommand);
		}catch(SignalException& ex)
		{
			ex.addMessage("try to close send connection for Interface " + m_sProcess + "::" + m_sName);
			ex.printTrace();
			LOG(LOG_WARNING, ex.getTraceString());
			if(!m_pSocketError->hasError())
				m_pSocketError->setError("ExternClientInputTemplate", "closeSendConnection",
								m_sProcess + "@" + m_sName);
		}
		m_pSendTransaction= NULL;
		UNLOCK(m_SENDMETHODLOCK);
		return m_pSocketError;
	}

	bool ExternClientInputTemplate::closeConnection(IClientConnectArtPattern* connection,
					OutsideClientTransaction* transaction, const string& command)
	{
		int bRv(true);
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> descriptor;

		try{
			if(transaction)
				transaction->closeConnection(command);
			m_pSocketError= connection->init();
			if(m_pSocketError->fail())
				bRv= false;
			if(	bRv &&
				!transaction	)
			{
				descriptor= connection->getDescriptor();
				(*descriptor) << command;
				if(command.substr(command.length()-1) != "\n")
					descriptor->endl();
				descriptor->flush();
			}
			connection->close();
			connection->newTranfer(NULL, /*delete old*/true);

		}catch(SignalException& ex)
		{
			ex.addMessage("try to close transaction connection for Interface " + m_sProcess + "::" + m_sName);
			ex.printTrace();
			LOG(LOG_WARNING, ex.getTraceString());
			m_pSocketError->setError("ExternClientInputTemplate", "closeConnection",
							m_sProcess + "@" + m_sName);
			bRv= false;
		}
		return bRv;
	}

	EHObj ExternClientInputTemplate::openGetConnection()
	{
		vector<string> answer;

		LOCK(m_GETQUESTIONLOCK);
		if(m_oGetConnect == NULL)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			m_pSocketError->setError("ExternClientInputTemplate", "noGetClass");
			return m_pSocketError;
		}
		if(m_pGetTransaction == NULL)
		{
			m_pGetTransaction= new OutsideClientTransaction();
			if(m_sOpenGetCommand != "")
			{
				SocketErrorHandling handling;

				m_oGetConnect->newTranfer(m_pGetTransaction, true);
				m_pGetTransaction->setCommand(m_sOpenGetCommand);
				m_pSocketError= m_oGetConnect->init();
				if(m_pSocketError->hasGroupError("IClientConnectArtPattern"))
				{
					m_oGetConnect->newTranfer(NULL, /*delete old*/true);
					m_pGetTransaction= NULL;
					UNLOCK(m_GETQUESTIONLOCK);
					return m_pSocketError;
				}
				answer= m_pGetTransaction->getReturnedString();
				handling.searchResultError(answer);
				if(handling.fail())
				{
					m_pGetTransaction= NULL;
					(*m_pSocketError)= handling;
				}
			}
		}
		UNLOCK(m_GETQUESTIONLOCK);
		return m_pSocketError;
	}

	string ExternClientInputTemplate::getQuestion(const string& lastAnswer)
	{
		static string lastQuestion;
		static vector<string> answer;

		answer.push_back(lastAnswer);
		if(	m_sAnswerEndString != "" &&
			m_sAnswerEndString != lastAnswer	)
		{
			return lastQuestion;
		}
		lastQuestion= getQuestion(answer);
		answer.clear();
		return lastQuestion;
	}

	string ExternClientInputTemplate::getQuestion(const vector<string>& lastAnswer)
	{
		string question;
		vector<string> answer;

		LOCK(m_GETQUESTIONLOCK);
		if(m_oGetConnect == NULL)
		{
			UNLOCK(m_GETQUESTIONLOCK);
			m_pSocketError->setError("ExternClientInputTemplate", "noGetClass");
			return m_pSocketError->getErrorStr();
		}
		if(m_pGetTransaction == NULL)
		{
			m_pSocketError= openGetConnection();
			if(m_pSocketError->hasError())
			{
				UNLOCK(m_GETQUESTIONLOCK);
				return m_pSocketError->getErrorStr();
			}
		}
		m_pGetTransaction->setAnswer(lastAnswer);
		m_pSocketError= m_oGetConnect->init();
		if(m_pSocketError->hasError())
			return m_pSocketError->getErrorStr();

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
		string warnstr;
		SocketErrorHandling handling;

		if(m_pSocketError->hasWarning())
		{
			warnstr= m_pSocketError->getErrorStr();
			m_pSocketError->clear();
		}
		answer= m_pGetTransaction->getReturnedString();
		if(	handling.searchResultError(answer) &&
			handling.hasError()						)
		{
			return handling.getErrorStr();
		}
		(*m_pSocketError)= handling;
		if(answer.size())
		{
			IMethodStringStream oQuestion(answer.front());

			if(oQuestion.getMethodName() == "blockA")
			{
				OMethodStringStream setingBlockCommand("blockA");
				bool block;

				oQuestion >> block;
				setingBlockCommand << block;
				if(block)
				{// question need's an answer over more rows

					oQuestion >> m_sAnswerEndString;
					setingBlockCommand << m_sAnswerEndString;
				}else
					m_sAnswerEndString= "";
				question= answer.front().substr(setingBlockCommand.str().length());

			}else
			{
				question= answer.front();
				m_sAnswerEndString= "";
			}
		}else
		{
			question= "";
			m_sAnswerEndString= "";
		}
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
		return question;
	}

	EHObj ExternClientInputTemplate::closeGetConnection()
	{
		LOCK(m_GETQUESTIONLOCK);
		try{
			if(m_pGetTransaction == NULL)
			{
				/*
				 * no transaction is open
				 * or be closed before
				 * so nothing to do
				 */
				UNLOCK(m_GETQUESTIONLOCK);
				return m_pSocketError;
			}
			closeConnection(m_oGetConnect, m_pGetTransaction, m_sEndingGetCommand);
		}catch(SignalException& ex)
		{
			ex.addMessage("try to close get connection for Interface " + m_sProcess + "::" + m_sName);
			ex.printTrace();
			LOG(LOG_WARNING, ex.getTraceString());
			if(!m_pSocketError->hasError())
				m_pSocketError->setError("ExternClientInputTemplate", "closeSendConnection",
								m_sProcess + "@" + m_sName);
		}
		m_pGetTransaction= NULL;
		UNLOCK(m_GETQUESTIONLOCK);
		return m_pSocketError;
	}

	ExternClientInputTemplate::~ExternClientInputTemplate()
	{
		m_oAnswerSender.stop(true);
		if(m_oSendConnect)
		{
			if(m_oSendConnect->connected())
				closeSendConnection();
			delete m_oSendConnect;
		}
		if(m_oGetConnect)
		{
			if(m_oGetConnect->connected())
				closeGetConnection();
			delete m_oGetConnect;
		}
		DESTROYMUTEX(m_SENDMETHODLOCK);
		DESTROYMUTEX(m_GETQUESTIONLOCK);
	}
}
