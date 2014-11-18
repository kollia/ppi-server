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

#include "../../../pattern/util/IErrorHandlingPattern.h"

#include "../../../pattern/server/NoCommunicateTransactionAdapter.h"

#include "../SocketErrorHandling.h"

using namespace design_pattern_world::util_pattern;
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
			:	m_pSocketError(EHObj(new SocketErrorHandling)),
			 	m_bAccess(false),
				m_bHold(true)
				{ };
			/**
			 * initial all values for transaction
			 *
			 * @param descriptor file handle to set start values
			 * @return object for error handling
			 */
			virtual EHObj init(IFileDescriptorPattern& descriptor);
			/**
			 * set command from outside the transaction object
			 *
			 * @param command string which should send to server
			 * @param ending on which string the answer should ending
			 */
			void setCommand(const string& command, const string& ending= "")
			{ m_sCommand= command; m_sAnswerEnding= ending; };
			/**
			 * set answer block from outside the transaction object
			 *
			 * @param answer vector which should send to server
			 * @param ending on which string the answer should ending
			 */
			void setAnswer(const vector<string>& answer);
			/**
			 * client transaction should ending by next <code>init()</code>
			 *
			 * @param endcommand send before ending this command
			 */
			void closeConnection(const string& endcommand)
			{ m_bHold= false; m_sEnding= endcommand; };
			/**
			 * return answer or question from server (other client)
			 *
			 * @return answer
			 */
			vector<string> getReturnedString()
			{ return m_vAnswer; }
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return whether should hold connection
			 */
			OVERWRITE bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * destructor of client transaction
			 */
			virtual ~OutsideClientTransaction() {};

		protected:
			/**
			 * socket error handling
			 */
			EHObj m_pSocketError;

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
			 * answer block sending back when set
			 * otherwise when m_sCommand not null, sending command
			 */
			vector<string> m_vsAnswerBlock;
			/**
			 * on which string the answer should ending
			 */
			string m_sAnswerEnding;
			/**
			 * answer from server
			 */
			vector<string> m_vAnswer;
			/**
			 * send before ending this command
			 */
			string m_sEnding;

	};

}

#endif /*OUTSIDECLIENTTRANSACTION_H_*/
