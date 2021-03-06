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
#ifndef SERVERMETHODTRANSACTION_H_
#define SERVERMETHODTRANSACTION_H_

#include "../../../pattern/server/ITransferPattern.h"

#include "../../../util/stream/IMethodStringStream.h"
#include "../../../util/debugtransaction.h"

#include "../SocketErrorHandling.h"

using namespace design_pattern_world::server_pattern;
using namespace util;

namespace server
{
	/**
	 * initial method transaction from server to client.<br />
	 * <br />
	 * Server Protocol:<br />
	 * <table>
	 * 	<tr>
	 * 		<td colspan="3">
	 * 			<b>registration by server:</b>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td width="30">
	 * 			&#160;
	 * 		</td>
	 * 		<td>
	 * 			&lt;client-name&gt; &lt:SEND|GET&gt;
	 * 		</td>
	 * 		<td>
	 * 			<table>
	 * 				<tr>
	 * 					<td>
	 * 						client-name
	 * 					</td>
	 * 					<td>
	 * 						name of client to identify
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						SEND
	 * 					</td>
	 * 					<td>
	 * 						client want to send questions or actions to an other process
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						GET
	 * 					</td>
	 * 					<td>
	 * 						client should wait for questions or actions from an other client to answer it
	 * 					</td>
	 * 				</tr>
	 * 			</table>
	 * 		</td>
	 * 	</tr>
	 * 	<tr height="50">
	 * 		<td>
	 * 			&#160;
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="3">
	 * 			<b>sending questions to an other client:</b>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td width="30">
	 * 			&#160;
	 * 		</td>
	 * 		<td>
	 * 			&lt;client-name&gt; &lt;string&gt;
	 * 		</td>
	 * 		<td>
	 * 			<table>
	 * 				<tr>
	 * 					<td>
	 * 						client-name
	 * 					</td>
	 * 					<td>
	 * 						name of client to identify to which should send
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						string
	 * 					</td>
	 * 					<td>
	 * 						any string send to to the other client<br />
	 * 						<table>
	 * 							<tr>
	 * 								<td colspan="3">
	 * 									send to this server (client-name is from server process-name)<br />
	 * 									follow strings (commands) can be set:
	 * 								</td>
	 * 							</tr>
	 * 							<tr>
	 * 								<td>
	 * 									init
	 * 								</td>
	 * 								<td>
	 * 									whether initialization of server process is done correctly.<br />
	 * 									server returning always true, because if this question be reached
	 * 									the initialization of server was done correctly, elsewhere the client do not reach the server
	 * 								</td>
	 * 							</tr>
	 * 							<tr>
	 * 								<td>
	 * 									running
	 * 								</td>
	 * 								<td>
	 * 									whether the server is running.<br />
	 * 									server returning always true, because if this question be reached
	 * 									the server always running
	 * 								</td>
	 * 							</tr>
	 * 							<tr>
	 * 								<td>
	 * 									stopping
	 * 								</td>
	 * 								<td>
	 * 									ask the server whether it should stopping.<br />
	 * 									returning true if server have to stop in the next time, elsewhere false
	 * 								</td>
	 * 							</tr>
	 * 							<tr>
	 * 								<td>
	 * 									status [string]
	 * 								</td>
	 * 								<td>
	 *									asking in all process for status of all running threads.<br />
	 *									<div>
	 *										if no parameter after <code>status</code> (string) is given, it returning
	 *										two numbers -&gt; how much process are running as first and how much threads are
	 *										running as second
	 *									</div>
	 *									<div>
	 *										the string can be an various string will be given to all StatusLogRoutine objects.
	 *										in the most case if given the string as thread, it shows the status from all threads
	 *										without the clients state from the two server. Otherwise it shows also the client state
	 *									</div>
	 * 								</td>
	 * 							</tr>
	 * 							<tr>
	 * 								<td>
	 *									stop
	 * 								</td>
	 * 								<td>
	 *									send stop to all clients and ending server
	 * 								</td>
	 * 							</tr>
	 * 						</table>
	 * 					</td>
	 * 				</tr>
	 * 			</table>
	 * 		</td>
	 * 	</tr>
	 * 	<tr height="50">
	 * 		<td>
	 * 			&#160;
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td colspan="3">
	 * 			<b>sign off server-client transaction:</b>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td width="30">
	 * 			&#160;
	 * 		</td>
	 * 		<td>
	 * 			ending
	 * 		</td>
	 * 		<td>
	 * 			ends the server client transaction and wait for an new client
	 * 		</td>
	 * </table>
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ServerMethodTransaction : public ITransferPattern
	{
		public:
			ServerMethodTransaction()
			: m_pSockError(EHObj(new SocketErrorHandling))
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			  ,m_boutput(false)
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			{};
			/**
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return object of error handling
			 */
			OVERWRITE EHObj init(IFileDescriptorPattern& descriptor);
			/**
			 * return error / warning object
			 *
			 * @return error handling object
			 */
			virtual EHObj getErrorObj() const
			{ return m_pSockError; };
			/**
			 * method transaction protocol between Server and Client
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return whether server should hold transaction
			 */
			OVERWRITE bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * method transaction protocol between server to client.<br />
			 * This method is called from main method of transfer,
			 * if the sending string is for actual server
			 *
			 * @param descriptor file handle to send answer or get more commands from client
			 * @param method object of method with parameters, which want server call
			 * @return whether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor, IMethodStringStream& parameter);
			/*
			 * this method will be called when any connection was broken
			 * and the child class should respond with some actions.
			 *
			 * @param ID unique connection ID
			 * @param process name of process from witch be connected
			 * @param client name of client to witch connected
			 */
			virtual void connectionEnding(const unsigned int ID, const string& process, const string& client) {};
			/**
			 * method to get name of transaction
			 *
			 * @param descriptor file handle to get variables
			 * @return name of transaction
			 */
			virtual string getTransactionName(const IFileDescriptorPattern& descriptor) const;
			/**
			 * search whether thread has given defined client name
			 * whitch is not defined for asking the server.
			 * This means the server do not wait for questions from client.
			 *
			 * @param descriptor file handle to get variables
			 * @param process name of process in which client running
			 * @param client defined name to find client, when null string ask only for process
			 * @return whether client is correct with given definition
			 */
			virtual bool hasNonAskingClient(	const IFileDescriptorPattern& descriptor,
												const string& process,
												const string& client= ""		) const;
			/**
			 * destructor of server method-transaction
			 */
			virtual ~ServerMethodTransaction();

		protected:
			/**
			 * socket object for error handling
			 */
			EHObj m_pSockError;

			/**
			 * this method will be called if any connection allocate to server
			 * and is only for overwriting
			 *
			 * @param ID client id
			 * @param client name of client witch allocate
			 */
			virtual void allocateConnection(IFileDescriptorPattern& descriptor) {};
			/**
			 * this method will be called, if any connection dissolve to server
			 * and is only for overwriting
			 *
			 * @param ID client id
			 * @param client name of client witch allocate
			 */
			virtual void dissolveConnection(IFileDescriptorPattern& descriptor) {};

		private:
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			bool m_boutput;
#endif // __FOLLOWSERVERCLIENTTRANSACTION

	};

}

#endif /*SERVERMETHODTRANSACTION_H_*/
