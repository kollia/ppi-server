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

#ifndef EXTERNCLIENTINPUTTEMPLATE_H_
#define EXTERNCLIENTINPUTTEMPLATE_H_

#include <string>

#include "../../../pattern/util/IErrorHandlingPattern.h"

#include "../../../pattern/server/IExternClientInput.h"
#include "../../../pattern/server/IClientConnectArtPattern.h"

#include "../../../util/stream/OMethodStringStream.h"

#include "OutsideClientTransaction.h"
#include "NoAnswerSender.h"


namespace util
{
	using namespace std;
	using namespace server;
	using namespace design_pattern_world::util_pattern;
	using namespace design_pattern_world::client_pattern;
	using namespace design_pattern_world::server_pattern;

	class ExternClientInputTemplate : public IExternClientInput
	{
		public:
			/**
			 * constructor of ExternClientInputTemplate.
			 * By cancel this ExternClientInputTemplate object, third and fourth parameter will be also delete.
			 *
			 * @param process name of process to identify by server
			 * @param client name of client to identify by server
			 * @param sendConnection on which connection from outside the server to answer is reachable
			 * @param getConnection on which connection from outside the server is reachable to get questions
			 */
			ExternClientInputTemplate(const string& process, const string& client,
							IClientConnectArtPattern* sendConnection,
							IClientConnectArtPattern* getConnection);
			/**
			 * open the connection to server for sending questions
			 *
			 * @param toopen string for open question,
			 *               otherwise by null string the connection will be open
			 *               with '<process>:<client> SEND' for connect
			 *               with an ServerMethodTransaction
			 * @return object of error handling
			 */
			EHObj openSendConnection(string toopen= "");
			/**
			 * open the connection to server for sending questions
			 *
			 * @param toopen string for open question,
			 *               otherwise by null string
			 *               the connection will be open
			 *               with '<process>:<client> SEND'
			 *               for connect with an ServerMethodTransaction
			 * @param timeout timeout in seconds by finding no connection in first step
			 * @return object of error handling
			 */
			EHObj openSendConnection(const unsigned int timeout, string toopen= "");
			/**
			 * open the connection to server to get questions and answer this
			 *
			 * @return object of error handling
			 */
			EHObj openGetConnection();
			/**
			 * sending commands to open and ending the sending connection.<br />
			 * By default this method not be called, by open the connection it would be sending
			 * <processName:clientName> set in constructor with following the word SEND.
			 * Also as default before close the connection it will be sending the word 'ending'.<br />
			 * If this method called but no ending or open parameter be set, the developer have to perform
			 * this command self and the object do not sending any commands
			 *
			 * @param open first command after open connection
			 * @param ending command before closing connection
			 */
			void openendSendConnection(const string& open= "", const string& ending= "")
			{ m_sOpenSendCommand= open; m_sEndingSendCommand= ending; };
			/**
			 * sending commands to open and ending the get question connection.<br />
			 * By default this method not be called, by open the connection it would be sending
			 * <processName:clientName> set in constructor with following the word GET.
			 * Also as default before close the connection it will be sending the word 'ending'.<br />
			 * If this method called but no ending or open parameter be set, the developer have to perform
			 * this command self and the object do not sending any commands
			 *
			 * @param open first command after open connection
			 * @param ending command before closing connection
			 */
			void openendGetConnection(const string& open= "", const string& ending= "")
			{ m_sOpenGetCommand= open; m_sEndingGetCommand= ending; };
			/**
			 * whether object has open connection to send questions to server
			 *
			 * @return whether connection is open
			 */
			bool hasOpenSendConnection()
			{ return m_pSendTransaction == NULL ? false : true; };
			/**
			 * whether object has open connection to get questions from server
			 *
			 * @return whether connection is open
			 */
			bool hasOpenGetConnection()
			{ return m_pGetTransaction == NULL ? false : true; };
			/**
			 * return used connection for sending requests
			 *
			 * @return connection object
			 */
			IClientConnectArtPattern* getSendConnection()
			{ return m_oSendConnect; };
			/**
			 * return used connection for getting requests
			 *
			 * @return connection object
			 */
			IClientConnectArtPattern* getGetConnection()
			{ return m_oGetConnect; };
			/**
			 * send message to given server in constructor
			 * or write into queue when no answer be needed
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param answer whether client should wait for answer
			 * @return backward send return value from server if answer is true, elsewhere returning null string
			 */
			virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true);
			/**
			 * send message to given server in constructor
			 * or write into queue when no answer be needed
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param done on which getting string the answer should ending.
			 *             Ending also when an ERROR or warning occurs
			 * @param answer whether client should wait for answer
			 * @return backward send return string vector from server if answer is true,
			 *         elsewhere returning vector with no size
			 */
			virtual vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method,
							const string& done, const bool answer= true);
			virtual vector<string> sendMethodD(const string& toProcess, const OMethodStringStream& method,
							const string& done, const bool answer= true);
			/*
			 * close sending connection to server
			 *
			 * @return object of error handling
			 */
			EHObj closeSendConnection();
			/**
			 * close get connection for questions from server (other client)
			 *
			 * @return object of error handling
			 */
			EHObj closeGetConnection();
			/**
			 * return address of connecting host for sending
			 *
			 * @return host address
			 */
			const string getSendHostAddress()
			{ return m_oSendConnect->getHostAddress(); };
			/**
			 * return address of connecting host from witch should get questions
			 *
			 * @return host address
			 */
			const string getGetHostAddress()
			{ return m_oGetConnect->getHostAddress(); };
			/**
			 * return address of connecting port for sending
			 *
			 * @return port address
			 */
			unsigned short getSendPortAddress() const
			{ return m_oSendConnect->getPortAddress(); };
			/**
			 * return address of connecting port from witch should get questions
			 *
			 * @return port address
			 */
			unsigned short getGetPortAddress() const
			{ return m_oGetConnect->getPortAddress(); };
			/**
			 * last answer from send question.<br />
			 * There also the answer by open connection.
			 *
			 * @return vector of answers
			 */
			vector<string> lastSendAnswer()
			{ return m_vSendAnswer; };
			/**
			 * returning name of actual process in which running
			 *
			 * @return process name
			 */
			const string getProcessName()
			{ return m_sProcess; };
			/**
			 * returning name of client to which is connected or should be
			 *
			 * @return client name
			 */
			const string getClientName()
			{ return m_sName; };
			/**
			 * returning current error handling object
			 *
			 * @return object of error handling
			 */
			OVERWRITE EHObj getErrorHandlingObj() const
			{ return m_pSocketError; };
			/**
			 * destructor of ExternClientInputTemplate
			 */
			virtual ~ExternClientInputTemplate();

		protected:
			/**
			 * SocketErrorHandling object for error/warning,
			 * usable for all depend classes
			 */
			EHObj m_pSocketError;

			/**
			 * ask server for question from any client
			 *
			 * @param lastAnswer answer for last question
			 * @return guestion from client
			 */
			string getQuestion(const string& lastAnswer);
			/**
			 * ask server for question from any client
			 *
			 * @param lastAnswer vector of answer for last question with more rows
			 * @return guestion from client
			 */
			string getQuestion(const vector<string>& lastAnswer);
			/**
			 * send answer back to server
			 *
			 * @param answer string of answer for server
			 */
			void sendAnswer(const string& answer);

		private:
			/**
			 * name of process to identify by server
			 */
			const string m_sProcess;
			/**
			 * name of client to identify by server
			 */
			const string m_sName;
			/**
			 * connection to process which should sending questions
			 */
			IClientConnectArtPattern* m_oSendConnect;
			/**
			 * connection to process which get questions
			 */
			IClientConnectArtPattern* m_oGetConnect;
			/**
			 * with witch command the sending connection should open.
			 * If not set the open command is <processName:processName>
			 */
			string m_sOpenSendCommand;
			/**
			 * with witch command the sending connection should ending.
			 * If m_sOpenSendCommand not be set the m_sEndingSendCommand is 'ending'.
			 * By set m_sOpenSendCommand and m_sEndingSendCommand not be set, no ending command will be sending
			 * and the connection is only cutting
			 */
			string m_sEndingSendCommand;
			/**
			 * with witch command the get question connection should open.
			 * If not set the open command is <processName:processName>
			 */
			string m_sOpenGetCommand;
			/**
			 * with witch command the get question connection should ending.
			 * If m_sOpenGetCommand not be set the m_sEndingGetCommand is 'ending'.
			 * By set m_sOpenGetCommand and m_sEndingGetCommand not be set, no ending command will be sending
			 * and the connection is only cutting
			 */
			string m_sEndingGetCommand;
			/**
			 * when the question need's an answer over more rows
			 * inside this string is set the ending string
			 */
			string m_sAnswerEndString;
			/**
			 * transaction to server to send methods.<br />
			 * If not exist create an new one
			 */
			OutsideClientTransaction* m_pSendTransaction;
			/**
			 * transaction to server to get methods for questions from other processes.<br />
			 * If not exist create an new one
			 */
			OutsideClientTransaction* m_pGetTransaction;
			/**
			 * last answer from sending question
			 */
			vector<string> m_vSendAnswer;
			/**
			 * running thread which sending questions which need no answers
			 */
			NoAnswerSender m_oAnswerSender;
			/**
			 * whether sending questions with no answer inside extern thread
			 * is possible
			 */
			bool m_bNoAnswerSend;
			/**
			 * lock for sending method
			 */
			pthread_mutex_t* m_SENDMETHODLOCK;
			/*
			 * mutex for get questions from server
			 */
			pthread_mutex_t* m_GETQUESTIONLOCK;
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			// this part of writing messages to output gives some ERROR
			// Although the variable m_boutput in the constructor is set to false, it is true in the processing
			// of changes in this variable some problems of connection are shown
			bool m_boutput;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			/**
			 * close given sending or get connection to server
			 *
			 * @param connection ClientConnectArtPattern from sending or get question
			 * @param transaction own created transaction object
			 * @param command command sending before close connection
			 * @return whether closing was successful, save error into member of error handling
			 */
			bool closeConnection(IClientConnectArtPattern* connection, OutsideClientTransaction* transaction, const string& command);
	};
}

#endif /* EXTERNCLIENTINPUTTEMPLATE_H_ */
