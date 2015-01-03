/**
 *   This file 'IClientTransactionPatterrn.h' is part of ppi-server.
 *   Created on: 27.12.2014
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

#ifndef ICLIENTTRANSACTIONPATTERN_H_
#define ICLIENTTRANSACTIONPATTERN_H_

#include "../../pattern/util/IPPIValuesPattern.h"

#include "../../pattern/server/NoCommunicateTransactionAdapter.h"

namespace design_pattern_world
{
	namespace util_pattern
	{
		using namespace design_pattern_world::server_pattern;

		class IClientTransactionPattern : public NoCommunicateTransferAdapter
		{
		public:
			/**
			 * structure definition of output direction
			 */
			enum direction_e
			{
				all= 0,
				first,
				previous,
				current,
				next,
				last
			};

			/**
			 * write current prompt
			 * when no hearing- or user-transaction running
			 *
			 * @param str string of new prompt when set, otherwise take default from last written
			 */
			virtual void prompt(const string& str= "")= 0;
			/**
			 * set whether user transaction running
			 *
			 * @param run whether running
			 */
			virtual void runUserTransaction(bool run)= 0;
			/**
			 * set which folder subroutine should be hold
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 */
			virtual void setHoldingFolder(const string& folder, const string& subroutine)= 0;
			/**
			 * set all incoming folder:subroutines from server
			 * to hold inside queue
			 */
			virtual void allFolderHolding()= 0;
			/**
			 * check whether folder:subroutine
			 * existing currently inside debugging queue
			 * getting from server
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine, when not exist check only for folder
			 * @return 1 when exist, 0 when not exist but inside queue of want to hold
			 *         or -1 when not exist and not in queue of wan to hold
			 */
			virtual short exist(const string& folder, const string& subroutine)= 0;
			/**
			 * clear all folder subroutines which before defined to holding.
			 * when now folder or subroutine given, clear all
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 */
			virtual void clearHoldingFolder(const string& folder, const string& subroutine)= 0;
			/**
			 * clear all content of debug session
			 */
			virtual void clearDebugSessionContent()= 0;
			/**
			 * create folder ID
			 *
			 * @param folder name of folder
			 */
			virtual string getFolderID(const string& folder)= 0;
			/**
			 * count from getting debug session info
			 * how often all folders are running.<br />
			 * WARNING: method is not thread-safe
			 * has to lock m_DEBUGSESSIONCHANGES outside
			 *
			 * @return map of count for all folders
			 */
			virtual map<string, unsigned long> getRunningFolderList()= 0;
			/**
			 * write content of debug session.<br />
			 * when no subroutines be set write hole folder,
			 * or by no folder write hole content
			 *
			 * @param folder name of folder
			 * @param subroutines vector of subroutines inside folder
			 * @param show whether should show all, current or new content of folder:subroutines
			 * @param curTime last time from folder output starting
			 * @param nr number of shown folder when show was next or previous and curTime is 0
			 * @return start time of shown folder or null time by not shown or direction all
			 */
			virtual IPPITimePattern* writeDebugSession(const string& folder, vector<string>& subroutines,
													const direction_e& show, const IPPITimePattern* curTime,
													const unsigned long nr= 0)= 0;
		};

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* ICLIENTTRANSACTIONPATTERN_H_ */
