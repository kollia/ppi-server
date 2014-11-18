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
#ifndef SERVERTRANSACTION_H_
#define SERVERTRANSACTION_H_

#include "../pattern/server/NoCommunicateTransactionAdapter.h"

#include "libs/server/ServerThread.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * initialication transaction from server to client
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ServerTransaction : public NoCommunicateTransferAdapter
	{
		public:
			/**
			 * constructor to create ServerTransaction object
			 *
			 * @param uid default uid to transfer process after reading all access parameters
			 */
			ServerTransaction(const uid_t uid);
			/**
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return object of error handling
			 */
			virtual EHObj init(IFileDescriptorPattern& descriptor);
			/**
			 * transaction protocol between Server and Client<br /><br />
			 * <table>
			 *   <tr>
			 *     <th colspan="3">
			 *       WARNING protocol sending to client:
			 *     </th>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       001
			 *     </td>
			 *     <td>
			 *       server is busy by starting
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <th colspan="3">
			 *       ERROR protocol sending to client:
			 *     </th>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       001
			 *     </td>
			 *     <td>
			 *       client beginning fault transaction
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       002
			 *     </td>
			 *     <td>
			 *       no correct command given
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       003
			 *     </td>
			 *     <td>
			 *       command parameter is incorrect,<br />
			 *       parameter after errornumber is position of error
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       004
			 *     </td>
			 *     <td>
			 *       cannot found given folder for operation
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       005
			 *     </td>
			 *     <td>
			 *       cannot found given subroutine in folder for operation
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       006
			 *     </td>
			 *     <td>
			 *       unknow value to set in subroutine
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       007
			 *     </td>
			 *     <td>
			 *       no filter be set for read directory
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       008
			 *     </td>
			 *     <td>
			 *       cannot read any directory
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       009
			 *     </td>
			 *     <td>
			 *       cannot found given file for read content
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       010
			 *     </td>
			 *     <td>
			 *       given ID from client do not exist
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       011
			 *     </td>
			 *     <td>
			 *       given user do not exist
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       012
			 *     </td>
			 *     <td>
			 *       wrong password for given user
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       013
			 *     </td>
			 *     <td>
			 *       user has no permission
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       014
			 *     </td>
			 *     <td>
			 *       [not set]
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       015
			 *     </td>
			 *     <td>
			 *       user cannot login as first
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       016
			 *     </td>
			 *     <td>
			 *       subroutine has no correct access to device
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       017
			 *     </td>
			 *     <td>
			 *       cannot find OWServer for debugging
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       018
			 *     </td>
			 *     <td>
			 *       no communication thread is free for answer<br /Y
			 *       (this case can behavior when the mincommunicationthreads parameter be 0)
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       019
			 *     </td>
			 *     <td>
			 *       server will be stopping from administrator
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       020
			 *     </td>
			 *     <td>
			 *       cannot load UserManagement correctly
			 *     </td>
			 *   </tr>
			 * </table>
			 *
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(const int error) const;
			/**
			 * destructor of server transaction
			 */
			virtual ~ServerTransaction();

		private:
			/**
			 * uid in which process should running
			 */
			const uid_t m_uid;
			/**
			 * whether server has loaded all content and waiting for client transactions
			 */
			bool m_bFinished;
			/**
			 * whether server will be stopping, do not connect again hearing port
			 */
			bool m_bStopServer;
			/**
			 * transaction protocol version number for communication<br />
			 * Latest version number be defined in file util/debug.h under PPI_SERVER_PROTOCOL
			 */
			float m_fProtocol;
			/**
			 * mutex lock whether server is finished by loading
			 */
			pthread_mutex_t* m_FINISHEDSERVERMUTEX;
			/**
			 * mutex lock for StopServer variable
			 */
			pthread_mutex_t* m_SERVERISSTOPPINGMUTEX;

			/**
			 * send to client all changed messages from database
			 * witch be requested
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			bool hearingPort(IFileDescriptorPattern& descriptor);
			/**
			 * handling all requested commands from client
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			bool clientCommands(IFileDescriptorPattern& descriptor);
			/**
			 * read directory from path and subdirectorys set in with the variable workdir + subdirectory client in file server.conf
			 *
			 * @param filter endstring of files. exp.: '.layout'
			 * @param verz additional path for rekursive searching
			 * @param list epmty vector list which will be filled with strings of /path/file and date
			 * @return bool true wehn workdir + verz exists otherwise false
			 */
			bool getDirectory(string filter, string verz, vector<string> &list);
			/**
			 * write logging message as LOG_SERVERDEBUG and return error string to send back on client
			 *
			 * @param file name from witch source file the method is called, specified with <code>__FILE__</code>
			 * @param line number of line in the source file, specified with <code>__LINE__</code>
			 * @param type defined type of log-message (<code>LOG_DEBUG, LOG_INFO, ...</code>)
			 * @param desc descriptor to write in log message user, host and client ID
			 * @param num number of error
			 * @param input in comming question from client
			 * @param add additional string for logging
			 * @return error string to send back on client
			 */
			string senderror(const string& file, const int line, const int type, const IFileDescriptorPattern& desc,
							const int num, string input, const string& add);
	};

}

#define ERROR(descriptor, num, input, add) senderror(__FILE__, __LINE__, LOG_SERVERERROR, descriptor, num, input, add)
#define INFOERROR(descriptor, num, input, add) senderror(__FILE__, __LINE__, LOG_SERVERINFO, descriptor, num, input, add)
#define DEBUGERROR(descriptor, num, input, add) senderror(__FILE__, __LINE__, LOG_SERVERDEBUG, descriptor, num, input, add)

#endif /*SERVERTRANSACTION_H_*/
