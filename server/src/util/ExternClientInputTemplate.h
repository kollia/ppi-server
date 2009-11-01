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

#include "../pattern/server/IClientConnectArtPattern.h"

#include "../server/libs/client/OutsideClientTransaction.h"

#include "OMethodStringStream.h"

namespace util
{
	using namespace std;
	using namespace server;
	using namespace design_pattern_world::util_pattern;
	using namespace design_pattern_world::server_pattern;

	class ExternClientInputTemplate
	{
		public:
			/**
			 * constructor of ExternClientInputTemplate
			 *
			 * @param process name of process to identify by server
			 * @param client name of client to identify by server
			 * @param sendConnection on which connection from outside the server to answer is reachable
			 * @param getConnection on which connection from outside the server is reachable to get questions
			 */
			ExternClientInputTemplate(const string& process, const string& client, IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection);
			/**
			 * calculate the error code given back from server as string.<br />
			 * the return error codes from server should be ERROR or WARNING.
			 * If the returned string was an warning, the number will be multiplied with -1 (become negative)
			 * Elsewhere the number is 0
			 *
			 * @param input the returned string from server
			 * @return error number
			 */
			static int error(const string& input);
			/**
			 * calculate from an error, warning code an string
			 *
			 * @param nr error code
			 * @return string of this error code
			 */
			static string error(const int nr);
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @param bSend whether need error string for sending connections (true is default) or get question connection (false).<br />
			 *              If no sending connection is set, but bSend is true, method ask even by get question connection
			 * @return error string
			 */
			virtual string strerror(const int error, const bool bSend= true);
			/**
			 * get maximal error or warning number in positive values
			 * from own class and all imply run through classes
			 *
			 * @param byerror whether needs error number (true) or warning number (false)
			 * @return maximal error or warning number
			 */
			virtual unsigned int getMaxErrorNums(const bool byerror) const;
			/**
			 * open the connection to server for sending questions
			 * <b>errorcodes:</b>
			 * <table>
			 * 	<tr>
			 * 		<td>
			 * 			0
			 * 		</td>
			 * 		<td>
			 * 			no error occurred
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			-1
			 * 		</td>
			 * 		<td>
			 * 			WARNING: connection exist before
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			1
			 * 		</td>
			 * 		<td>
			 * 			ERROR: no <code>IClientConnectArtPattern</code> be given for sending
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			2
			 * 		</td>
			 * 		<td>
			 * 			cannot connect with server, or initialization was fail
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td colspan="2">
			 * 			all other ERRORs or WARNINGs see in <code>IClientConnectArtPattern</code>
			 * 			for beginning connection by sending
			 * 		</td>
			 * 	</tr>
			 * </table>
			 *
			 * @param toopen string for open question, otherwise by null string the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @return error number
			 */
			int openSendConnection(string toopen= "");
			/**
			 * open the connection to server for sending questions
			 * <b>errorcodes:</b>
			 * <table>
			 * 	<tr>
			 * 		<td>
			 * 			0
			 * 		</td>
			 * 		<td>
			 * 			no error occurred
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			-1
			 * 		</td>
			 * 		<td>
			 * 			WARNING: connection exist before
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			1
			 * 		</td>
			 * 		<td>
			 * 			ERROR: no <code>IClientConnectArtPattern</code> be given for sending
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td>
			 * 			2
			 * 		</td>
			 * 		<td>
			 * 			cannot connect with server, or initialization was fail
			 * 		</td>
			 * 	</tr>
			 * 	<tr>
			 * 		<td colspan="2">
			 * 			all other ERRORs or WARNINGs see in <code>IClientConnectArtPattern</code>
			 * 			for beginning connection by sending
			 * 		</td>
			 * 	</tr>
			 * </table>
			 *
			 * @param toopen string for open question, otherwise by null string the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @param timeout timeout in seconds by finding no connection in first step
			 * @return error number
			 */
			int openSendConnection(const unsigned int timeout, string toopen= "");
			/**
			 * open the connection to server to get questions and answer this
			 *
			 * @return error code
			 */
			int openGetConnection();
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
			/*
			 * close sending connection to server
			 *
			 * @return error code
			 */
			int closeSendConnection();
			/**
			 * close get connection for questions from server (other client)
			 *
			 * @return error code
			 */
			int closeGetConnection();
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
			const unsigned short getSendPortAddress()
			{ return m_oSendConnect->getPortAddress(); };
			/**
			 * return address of connecting port from witch should get questions
			 *
			 * @return port address
			 */
			const unsigned short getGetPortAddress()
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
			 * destructor of ExternClientInputTemplate
			 */
			virtual ~ExternClientInputTemplate();

		protected:
			/**
			 * send message to given server in constructor
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param answer whether client should wait for answer
			 * @return backward send return value from server if answer is true, elsewhere returning null string
			 */
			string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true);
			/**
			 * send message to given server in constructor
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param done on which getting string the answer should ending. Ending also when an ERROR or warning occurs
			 * @param answer whether client should wait for answer
			 * @return backward send return string vector from server if answer is true, elsewhere returning vector with no size
			 */
			vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true);
			/**
			 * ask server for question from any client
			 *
			 * @param lastAnswer answer for last question
			 * @return guestion from client
			 */
			string getQuestion(const string& lastAnswer);
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
			 * lock for sending method
			 */
			pthread_mutex_t* m_SENDMETHODLOCK;
			/*
			 * mutex for get questions from server
			 */
			pthread_mutex_t* m_GETQUESTIONLOCK;

			/**
			 * close given sending or get connection to server
			 *
			 * @param connection ClientConnectArtPattern from sending or get question
			 * @param transaction own created transaction object
			 * @param command command sending before close connection
			 * @return error code
			 */
			int closeConnection(IClientConnectArtPattern* connection, OutsideClientTransaction* transaction, const string& command);
	};
}

#endif /* EXTERNCLIENTINPUTTEMPLATE_H_ */
