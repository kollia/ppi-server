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
#ifndef SERVERDBTRANSACTION_H_
#define SERVERDBTRANSACTION_H_

#include <list>

#include "../server/libs/client/ppi_server_clients.h"

#include "../server/libs/server/ServerMethodTransaction.h"

#include "../util/thread/Thread.h"

namespace server
{

	using namespace util;

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
	class ServerDbTransaction : public ServerMethodTransaction
	{
		public:
			/**
			 * constructor to create instance
			 */
			ServerDbTransaction();
			/**
			 * set log object for logging from extern processes
			 *
			 * @param logObj logging object <code>LogThread</code>
			 */
			void setLogObject(LogThread* logObj)
			{ m_pLogObject= logObj; };
			/**
			 * method transaction protocol between server to client.<br />
			 * This method is called from main method of transfer,
			 * if the sending string is for actual server
			 *
			 * @param descriptor file handle to send answer or get more commands from client
			 * @param method object of method with parameters, which want server call
			 * @return whether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor, IMethodStringStream& method);
			/*
			 * this method will be called when any connection was broken.
			 * By this method the database will be informe to arouse the CHANGINGPOOLCONDITION
			 * to end also this connection.
			 *
			 * @param ID unique connection ID
			 * @param process name of process from witch be connected
			 * @param client name of client to witch connected
			 */
			virtual void connectionEnding(const unsigned int ID, const string& process, const string& client);
			/**
			 * destructor of server method-transaction
			 */
			virtual ~ServerDbTransaction();

		protected:
			/**
			 * this method will be called if any connection allocate to server
			 *
			 * @param ID client id
			 * @param client name of client witch allocate
			 */
			virtual void allocateConnection(IFileDescriptorPattern& descriptor);
			/**
			 * this method will be called, if any connection disolve to server
			 *
			 * @param ID client id
			 * @param client name of client witch allocate
			 */
			virtual void dissolveConnection(IFileDescriptorPattern& descriptor);
			/**
			 * return count of how much one wire clients are connected
			 *
			 * @return number of one wire clients
			 */
			unsigned short getOwClientCount();

		private:
			/**
			 * how much one wire clients are connected
			 */
			unsigned short m_nOwClients;
			/**
			 * mutex to count one wire clients
			 */
			pthread_mutex_t* m_ONEWIRECLIENTSMUTEX;
			/**
			 * opened one wire server
			 */
			map<unsigned short, list<unsigned int> > m_msiOpenedOWServer;
			/**
			 * logging object to log from extern processes
			 */
			LogThread* m_pLogObject;
	};

}

#endif /*SERVERDBTRANSACTION_H_*/
