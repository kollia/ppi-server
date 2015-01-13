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

#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <string>
#include <vector>
#include <deque>

#include "IHearingThreadPattern.h"

#include "../../util/stream/ppivalues.h"

#include "../../pattern/util/IErrorHandlingPattern.h"
#include "../../pattern/util/IDbFillerPattern.h"


using namespace std;
using namespace design_pattern_world::util_pattern;

namespace server
{
	/**
	 * initialication transaction from server to client
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ClientTransaction : public IClientTransactionPattern
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
			 * @return object of error handling
			 */
			OVERWRITE EHObj init(IFileDescriptorPattern& descriptor);
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * set warning and error number which can occur outside of ClientTransaction
			 *
			 * @param warning highest warning number as positive integer
			 * @param error highes error number
			 */
			void setErrors(const unsigned int warning, const unsigned int error)
			{ m_nOutsideWarn= warning; m_nOutsideErr= error; };
			/**
			 * to now whether ClientTransaction has written an error
			 *
			 * @return whether error was written from client
			 */
			bool wasErrorWritten()
			{ return m_bErrWritten; }
			/**
			 * write current prompt
			 * when no hearing- or user-transaction running
			 *
			 * @param str string of new prompt when set, otherwise take default from last written
			 */
			OVERWRITE void prompt(const string& str= "");
			/**
			 * write last line of prompt with result
			 *
			 * @param lock whether need an lock to have access to prompting values
			 * @param cursor current cursor position inside result
			 * @param str string of new result after prompt
			 * @param end whether should write new line after last string for ending (default= false)
			 */
			OVERWRITE void writeLastPromptLine(bool lock,
							string::size_type cursor= string::npos, const string& str= "", bool end= false);
			/**
			 * set history of written command
			 *
			 * @param command current command
			 * @param pos old position when changed inside history
			 */
			OVERWRITE void setHistory(const string& command, vector<string>::size_type pos= 0);
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
			OVERWRITE string getHistory(vector<string>::size_type& count, history_get_e pos);
			/**
			 * writing all history commands
			 */
			OVERWRITE void writeHistory();
			/**
			 * set whether user transaction running
			 *
			 * @param run whether running
			 */
			OVERWRITE void runUserTransaction(bool run);
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
			OVERWRITE short exist(const string& folder, const string& subroutine);
			/**
			 * set which folder subroutine should be hold
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 */
			OVERWRITE void setHoldingFolder(const string& folder, const string& subroutine);
			/**
			 * set all incoming folder:subroutines from server
			 * to hold inside queue
			 */
			OVERWRITE void allFolderHolding();
			/**
			 * clear all folder subroutines which before defined to holding.
			 * when now folder or subroutine given, clear all
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 */
			OVERWRITE void clearHoldingFolder(const string& folder, const string& subroutine);
			/**
			 * clear all content of debug session
			 */
			OVERWRITE void clearDebugSessionContent();
			/**
			 * create folder ID
			 *
			 * @param folder name of folder
			 */
			OVERWRITE string getFolderID(const string& folder);
			/**
			 * count from getting debug session info
			 * how often all folders are running.<br />
			 * WARNING: method is not thread-safe
			 * has to lock m_DEBUGSESSIONCHANGES outside
			 *
			 * @return map of count for all folders
			 */
			OVERWRITE map<string, unsigned long> getRunningFolderList();
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
			OVERWRITE IPPITimePattern* writeDebugSession(const string& folder, vector<string>& subroutines,
													const direction_e& show, const IPPITimePattern* curTime,
													const unsigned long nr= 0);
			/**
			 * destructor of server transaction
			 */
			virtual ~ClientTransaction();

		private:
			typedef map<ppi_time, vector<IDbFillerPattern::dbgSubroutineContent_t> > debugSessionTimeMap;

			/**
			 * whether was reading
			 * of terminal interface correct
			 */
			bool m_bCorrectTC;
			/**
			 * whether ppi-client was started
			 * with an file shifted in it
			 * like 'ppi-client < file'<br />
			 * by this case <code>tcgetattr</code>
			 * give errno ENOTTY
			 */
			bool m_bScriptState;
			/**
			 * curently changed terminal interface
			 */
			//static struct termios m_tCurrentTermios;
			/**
			 * terminal interface for backup to reset
			 */
			struct termios m_tTermiosBackup;
			/**
			 * prompt string to display
			 * locked by PROMPTMUTEX
			 */
			string m_sPrompt;
			/**
			 * last line of prompt to display
			 * locked by PROMPTMUTEX
			 */
			string m_sLastPromptLine;
			/**
			 * current result after prompt
			 * locked by PROMPTMUTEX
			 */
			string m_sPromptResult;
			/**
			 * current cursor position
			 * inside result
			 * locked by PROMPTMUTEX
			 */
			string::size_type m_nResultPos;
			/**
			 * result length after prompt
			 * from result written before
			 * locked by PROMPTMUTEX
			 */
			string::size_type m_nOldResultLength;
			/**
			 * whether user transaction running
			 * locked by PROMPTMUTEX
			 */
			bool m_bRunUserTrans;
			/**
			 * whether hearing transaction running
			 * locked by PROMPTMUTEX
			 */
			bool m_bRunHearTran;
			/**
			 * vector of all options be set on the shell
			 */
			vector<string> m_vOptions;
			/**
			 * vector of history,
			 * written commands before
			 */
			vector<string> m_vHistory;
			/**
			 * numbers of changed history
			 */
			vector<vector<string>::size_type > m_vChangedHistory;
			/**
			 * command string from shell
			 * for initialization of ClientTransaction object
			 */
			string m_sCommand;
			/**
			 * transaction protocol version number for communication<br />
			 * Latest version number be defined in file util/debug.h under PPI_SERVER_PROTOCOL
			 */
			float m_fProtocol;
			/**
			 * whether an error was written from ClientTransaction
			 */
			bool m_bErrWritten;
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
			auto_ptr<IHearingThreadPattern> m_o2Client;
			/**
			 * warning number which can occur outside of ClientTransaction as positive integer
			 */
			unsigned int m_nOutsideWarn;
			/**
			 * error number which can occur outside of ClientTransaction
			 */
			unsigned int m_nOutsideErr;
			/**
			 * all folder IDs
			 */
			map<string, string> m_mFolderId;
			/**
			 * whether should holding all folders
			 * inside queue
			 */
			bool m_bHoldAll;
			/**
			 * all folder subroutines
			 * for debug session
			 * which should holding before showing
			 * with current output position
			 */
			map<string, map<string, unsigned long> > m_vsHoldFolders;
			/**
			 * all hold debug session content
			 */
			debugSessionTimeMap m_mmDebugSession;
			/**
			 * time returned from method <code>writeDebugSession()</code>
			 */
			ppi_time m_dbgSessTime;
			/**
			 * mutex to write clear hold variables
			 */
			pthread_mutex_t* m_DEBUGSESSIONCHANGES;
			/**
			 * mutex of changing prompt
			 */
			pthread_mutex_t* m_PROMPTMUTEX;

			/**
			 * set whether hearing transaction running
			 *
			 * @param run whether running
			 */
			void runHearingTransaction(bool run);
			/**
			 * hear on terminal input
			 *
			 * @param yesno whether should ask only for yes or no
			 * @param promptStr prompt string before waiting for input
			 * @return released word, or by yes/no question only 'Y' or 'N' in big letters
			 */
			OVERWRITE string ask(bool yesno, string promptStr);
			/**
			 * reading one character
			 * or specialcharacter sequence
			 * from command line
			 *
			 * @return integer number of character
			 */
			int getch();
			/**
			 * read command line terminal interface
			 * to make possible to reset
			 */
			void readTcBackup();
			/**
			 * reset command line terminal interface
			 * to last reading backup
			 */
			void resetTc();
			/**
			 * set terminal interface on command line
			 * to currently stdin file descriptor.<br />
			 * only when reading of <code>readTcBackup()</code> was OK<br />
			 * see man pages of <code>tcsetattr</code>
			 *
			 * @param action like 2. parameter of <code>tcsetattr</code> in man pages
			 * @param teriosp changed termios structure
			 * @return whether setting was done
			 */
			bool tcsetattr(int action, const struct termios *termiosp);
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			bool userTransfer(IFileDescriptorPattern& descriptor);
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			bool hearingTransfer(IFileDescriptorPattern& descriptor);
			/**
			 * whether subroutine exist inside vector of subroutines
			 *
			 * @param subroutine name of subroutine
			 * @param subroutines vector where subroutine should exist
			 * @return whether exist
			 */
			bool subroutineSet(const string& subroutine,
							const vector<string>& subroutines);
			/**
			 * writing help usage
			 *
			 * @param sfor which helping should write (?, ?value or ?debug)
			 */
			void writeHelpUsage(const string& sfor);
			/**
			 * whether folder:subroutine
			 * exist inside ppi-server working list
			 *
			 * @param descriptor descriptor to ask ppi-server
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 * @return whether exist
			 */
			bool existOnServer(IFileDescriptorPattern& descriptor,
							const string& folder, const string& subroutine);
			/**
			 * return all ERROR results as translated strings
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @param result ERROR string with number
			 * @param string from error code
			 */
			string getError(IFileDescriptorPattern& descriptor, const string& error);
			/**
			 * read password and when not given user
			 * from command line and compare with server
			 *
			 * @param descriptor file handle to compare with server
			 * @param user name of user or null string
			 * @return whether entries was correct
			 */
			bool compareUserPassword(IFileDescriptorPattern& descriptor, string user);
			/**
			 * print all ERROR results as translated strings on command line
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @param result ERROR string with number
			 */
			void printError(IFileDescriptorPattern& descriptor, const string& error);
			/**
			 * check whether command has enough parameters
			 * or not to much.<br />
			 * Inside parameter 'descriptions' should be
			 * and description for every parameter after command.
			 * Which will be displayed when inserted command
			 * has not enough parameters.
			 * When an entry inside squared brackets '[]'
			 * entry is optional
			 * or when vector has no entries or is an NULL pointer,
			 * this mean that no parameters are allowed
			 *
			 * @param commands vector of command with parameters
			 * @param errorWritten return back whether an error be written when iserted as false
			 *                     otherwise it do not write an error
			 * @param descriptions description for all parameters
			 * @return whether command count was ok
			 */
			bool checkCommandCount(vector<string> commands, bool& errorWritten,
							vector<string>* descriptions= NULL);
			/**
			 * same as method <code>checkCommandCount()</code>
			 * but check also whether ppi-client was started with option --wait
			 *
			 * @param commands vector of command with parameters
			 * @param errorWritten return back whether an error be written when inserted as false
			 *                     otherwise it do not write an error
			 * @param descriptions description for all parameters
			 * @return whether command count was ok
			 */
			bool checkWaitCommandCount(vector<string> commands, bool& errorWritten,
							vector<string>* descriptions= NULL);
			/**
			 * same as method <code>checkCommandCount()</code>
			 * but check also whether ppi-client was started with option --hear
			 *
			 * @param commands vector of command with parameters
			 * @param error return back whether an error be written when inserted as false
			 *              otherwise it do not write an error
			 * @param descriptions description for all parameters
			 * @return whether command count was ok
			 */
			bool checkHearCommandCount(vector<string> commands, bool& error, vector<string>* descriptions= NULL);
	};

}

#endif /*CLIENTTRANSACTION_H_*/
