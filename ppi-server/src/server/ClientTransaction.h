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
#ifndef CLIENTTRANSACTION_H_
#define CLIENTTRANSACTION_H_

#include <string>
#include <vector>
#include <deque>

#include "hearingthread.h"

#include "../portserver/owserver.h"

#include "../pattern/server/ITransferPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * initialication transaction from server to client
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ClientTransaction : public ITransferPattern
	{
		public:
			/**
			 * constructor to initial member variables
			 */
			ClientTransaction(vector<string> options, string command);
			/**
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return whether initialization was correct
			 */
			virtual bool init(IFileDescriptorPattern& descriptor);
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * destructor of server transaction
			 */
			virtual ~ClientTransaction();

		private:
			/**
			 * vector of all options be set on the shell
			 */
			vector<string> m_vOptions;
			/**
			 * command string from shell
			 */
			string m_sCommand;
			/**
			 * whether client should wait for an new command
			 */
			bool m_bWait;
			/**
			 * whether transaction set to be an hearing connection
			 */
			bool m_bHearing;
			/**
			 * whether client should show ERROR number (true) or translated string (false)
			 */
			bool m_bShowENum;
			/**
			 * whether the session is set for debug an OWServer
			 */
			bool m_bOwDebug;
			/**
			 * map of all devices -> id as key and device_debug_t as value
			 */
			map<unsigned short, OWServer::device_debug_t> m_mOwDevices;
			/**
			 * max micro time for all ids
			 */
			map<unsigned short, long> m_mOwMaxTime;
			/**
			 * count of measure from devices
			 */
			map<unsigned short, deque<unsigned short> > m_mOwMaxCount;
			/**
			 * object of an second client running inside and thread
			 */
			HearingThread* m_o2Client;

			/**
			 * print all ERROR results as translated strings on commandline
			 *
			 * @param result ERROR string with number
			 */
			void printError(string error);


	};

}

#endif /*CLIENTTRANSACTION_H_*/