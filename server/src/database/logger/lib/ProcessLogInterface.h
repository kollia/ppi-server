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
#ifndef PROCESSLOGINTERFACE_H_
#define PROCESSLOGINTERFACE_H_

//#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#include <string>
#include <vector>

#include "logstructures.h"
#include "LogInterface.h"

#include "../../../server/libs/client/ProcessInterfaceTemplate.h"

using namespace std;
using namespace util;

namespace logger
{
	class ProcessLogInterface :	public LogInterface,
								public ProcessInterfaceTemplate
	{
		public:
			/**
			 * creating instance of LogInterface
			 *
			 * @param process name of process in which the log-interface running
			 * @param connection to which server this interface should connect to send messages
			 * @param identifwait how many seconds an log with an identif string should wait to write again into the logfile
			 * @param wait whether connection should wait for correct access
			 */
			ProcessLogInterface(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait);
			/**
			 * Instantiate class of log-server.<br/>
			 * By cancel this LogInterface object, second parameter object will be also delete in parent class.
			 *
			 * @param process name of process in which the log-interface running
			 * @param connection to which server this interface should connect to send messages
			 * @param identifwait how many seconds an log with an identif string should wait to write again into the logfile
			 * @param wait whether connection should wait for correct access
			 * @return whether connection to LogProccess was correct
			 */
			static bool initial(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait);
			/**
			 * delete object of LogInterface
			 */
			static void deleteObj();
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
			 * @param toopen string for open question, otherwise by null the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @return error number
			 */
			virtual int openConnection(string toopen= "");
			/**
			 * destructor of log interface
			 */
			virtual ~ProcessLogInterface();

		protected:
			/**
			 * send message to given server in constructor
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param answer whether client should wait for answer
			 * @return backward send return value from server if answer is true, elsewhere returning null string
			 */
			virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true)
			{ return ExternClientInputTemplate::sendMethod(toProcess, method, answer); };

		private:
			/**
			 * instance of single logging interface
			 */
			static ProcessLogInterface* _instance;


	}; // class LogInterface
} // namespace logger

#ifndef LOG
#define LOG(type, message) logger::ProcessLogInterface::instance()->log(__FILE__, __LINE__, type, message)
#endif // LOG
#ifndef TIMELOG
#define TIMELOG(type, identif, message) logger::ProcessLogInterface::instance()->log(__FILE__, __LINE__, type, message, identif)
#endif // TIMELOG

#endif /*PROCESSLOGINTERFACE_H_*/
