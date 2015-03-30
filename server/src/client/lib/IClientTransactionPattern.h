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
			 * currently action of direction
			 */
			enum action_e
			{
				none= 0,
				inform,
				external,
				folder,
				changed, // also folder
				unchanged // same
			};
			/**
			 * structure of direction with action
			 */
			struct direction_t
			{
				direction_e direction;
				action_e action;
			};
			/**
			 * structure of which history entry
			 * want get back
			 */
			enum history_get_e
			{
				Older= 0,
				Current,
				Newer
			};

			/**
			 * write current prompt
			 * when no hearing- or user-transaction running
			 *
			 * @param str string of new prompt when set, otherwise take default from last written
			 */
			virtual void prompt(const string& str= "")= 0;
			/**
			 * write last line of prompt with result
			 *
			 * @param lock whether need an lock to have access to prompting values
			 * @param cursor current cursor position inside result
			 * @param str string of new result after prompt
			 * @param end whether should write new line after last string for ending (default= false)
			 */
			virtual void writeLastPromptLine(bool lock,
							string::size_type cursor= string::npos, const string& str= "", bool end= false)= 0;
			/**
			 * set handle for termios reading
			 * whether was written ok.<br />
			 * this only be useful when second connection
			 * for hearing be started
			 *
			 * @param read whether was written ok
			 */
			virtual void correctTC(bool read)= 0;
			/**
			 * print string by next call of method <code>prompt()</code> or <code>ask()</code>
			 *
			 * @param str string which should be printed
			 */
			virtual void cout(const string& str)= 0;
			/**
			 * set history of written command
			 *
			 * @param command current command
			 * @param pos old position when changed inside history
			 */
			virtual void setHistory(const string& command, vector<string>::size_type pos= 0)= 0;
			/**
			 * return current history command.<br />
			 * when count is 0, it will be return the last written command
			 *
			 * @param count last getting history before or 0 by none.<br />
			 *              give back number of history
			 * @param pos which history entry want to get back
			 *
			 * @return history command
			 */
			virtual string getHistory(vector<string>::size_type& count, history_get_e pos)= 0;
			/**
			 * writing all history commands
			 */
			virtual void writeHistory()= 0;
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
			 * @return whether after clearing all holding of debug session queue is empty
			 */
			virtual bool clearHoldingFolder(const string& folder, const string& subroutine)= 0;
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
			 * how often all folders are running.
			 *
			 * @param locked whether DEBUGSESSIONCHANGES locked before
			 * @param outside whether should be included external running subroutines
			 * @return map of count for all folders
			 * @return map of count for all folders
			 */
			virtual map<string, unsigned long> getRunningFolderList(bool locked, bool outside= false)= 0;
			/**
			 * complete given result with an new tabulator string
			 * and giving result in same parameter back
			 *
			 * @param result current result which will be completed
			 * @param nPos current position of cursor
			 * @param count currently count of pressed tabulator before
			 */
			void createTabResult(string& result, string::size_type& nPos, short& count);
			/**
			 * search in folder list getting from debug session
			 * for results beginning with given string
			 *
			 * @param str folder names should beginning with this string
			 * @return vector of all possible folders
			 */
			virtual vector<string> getUsableFolders(const string& str)= 0;
			/**
			 * search in subroutine list from given folder
			 * fro results beginning with given string
			 *
			 * @param folder name of folder from which subroutines should be searched
			 * @param str subroutine names should beginning with this string
			 * @return vector of all possible subroutines
			 */
			virtual vector<string> getUsableSubroutines(const string& folder, const string& str)= 0;
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
													direction_t show, IPPITimePattern* curTime,
													const unsigned long nr= 0)= 0;
			/**
			 * hear on terminal input
			 *
			 * @param yesno whether should ask only for yes or no
			 * @param promptStr prompt string before waiting for input
			 * @return released word, or by yes/no question only 'Y' or 'N' in big letters
			 */
			virtual string ask(bool yesno, string promptStr)= 0;
			/**
			 * check whether getting debug session queue from server
			 * is empty
			 *
			 * @return whether queue is empty
			 */
			virtual bool emptyDbgQueue() const= 0;
			/**
			 * save current or follow debug session queue with ending <code>.dbgsession</code>
			 * into file on current file system where client started
			 *
			 * @param file name of file where should stored
			 * @return whether saving was correct done
			 */
			virtual bool saveFile(const string& file)= 0;
			/**
			 * close opened file to store debug session content
			 */
			virtual void closeFile()= 0;
			/**
			 * load before saved debug session from file system
			 * where client was started
			 *
			 * @param file name of file which should loaded
			 * @return whether loading was correct done
			 */
			virtual bool loadFile(const string& file)= 0;
		};

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* ICLIENTTRANSACTIONPATTERN_H_ */
