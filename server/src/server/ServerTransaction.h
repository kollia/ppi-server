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

#include "ServerThread.h"

#include "../pattern/server/NoCommunicateTransactionAdapter.h"

extern server::ServerThread* gInternetServer;

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
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return whether initialization was correct
			 */
			virtual bool init(IFileDescriptorPattern& descriptor);
			/**
			 * transaction protocol between Server and Client<br /><br />
			 * <table>
			 *   <tr>
			 *     <th colspan="3">
			 *       ERROR protokol sending to client:
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
			 *       subrutine isn't correct defined by the settings of config file
			 *     </td>
			 *   </tr>
			 *   <tr>
			 *     <td width="30">
			 *     </td>
			 *     <td align="right">
			 *       015
			 *     </td>
			 *     <td>
			 *       root cannot login as first user
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
			virtual string strerror(int error) const;
			/**
			 * destructor of server transaction
			 */
			virtual ~ServerTransaction();

		private:
			/**
			 * search measure port for given folder
			 *
			 * @param folder name of folder
			 * @return measure port if exist, otherwise null
			 */
			meash_t* getMeasurePort(string folder);
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
	};

}

#endif /*SERVERTRANSACTION_H_*/
