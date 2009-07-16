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
#ifndef OUTSIDECLIENTTRANSACTION_H_
#define OUTSIDECLIENTTRANSACTION_H_

#include <string>
#include <vector>
#include <deque>

#include "hearingthread.h"

#include "../portserver/owserver.h"

#include "../pattern/server/NoCommunicateTransactionAdapter.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * initialication transaction from from client to server from outside
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class OutsideClientTransaction : public NoCommunicateTransferAdapter
	{
		public:
			/**
			 * constructor for transaction from outside
			 */
			OutsideClientTransaction()
			:	m_bAccess(false),
				m_bHold(false)
				{ };
			/**
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return whether initialization was correct
			 */
			virtual bool init(IFileDescriptorPattern& descriptor)
			{ return true; };
			/**
			 * set command from outside the transaction object
			 *
			 * @param command string which should send to server
			 * @param hold whether connection should holding after sending
			 */
			void setCommand(const string& command, const bool& hold)
			{ m_sCommand= command; m_bHold= hold; };
			/**
			 * return answer from server
			 *
			 * @return answer
			 */
			string getAnswer()
			{ return m_sAnswer; }
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * destructor of client transaction
			 */
			virtual ~OutsideClientTransaction() {};

		private:
			/**
			 * whether have correct connection
			 */
			bool m_bAccess;
			/**
			 * whether connection should hold after sending command
			 */
			bool m_bHold;
			/**
			 * command which should send to server
			 */
			string m_sCommand;
			/**
			 * answer from server
			 */
			string m_sAnswer;

	};

}

#endif /*OUTSIDECLIENTTRANSACTION_H_*/
