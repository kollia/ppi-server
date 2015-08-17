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

/**
 * display counting
 * of external subroutines
 * and folder running
 * by method writeDebugSession()
 * when set by compiling
 * this parameter to one
 */
#define __DISPLAY_COUNTING 0

#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <string>
#include <vector>
#include <set>
#include <deque>
#include <iostream>
#include <fstream>

#include "IHearingThreadPattern.h"

#include "../../util/stream/ppivalues.h"

#include "../../pattern/util/IErrorHandlingPattern.h"
#include "../../pattern/util/IDbFillerPattern.h"


using namespace std;
using namespace design_pattern_world::util_pattern;

namespace server
{
	/**
	 * Initialization transaction from server to client
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
			 * return error / warning object
			 *
			 * @return error handling object
			 */
			virtual EHObj getErrorObj() const
			{ return m_pError; };
			/**
			 * transaction protocol between client to server
			 *
			 * @param descriptor file handle to get command's and send answer
			 * @return wether need to hold the connection
			 */
			virtual bool transfer(IFileDescriptorPattern& descriptor);
			/**
			 * remove hearing transaction
			 * to reach from user transaction
			 */
			void removeHearThread()
			{ m_o2Client= SHAREDPTR::shared_ptr<IHearingThreadPattern>(); };
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
			 * set prompt string to writing with prompt
			 *
			 * @param str string of new prompt
			 */
			void setPromptString(const string& str= "");
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
			 * print string by next call of method <code>prompt()</code> or <code>ask()</code>
			 *
			 * @param str string which should be printed
			 */
			OVERWRITE void cout(const string& str);
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
			 * writing help usage
			 *
			 * @param sfor which helping should write (?, ?value or ?debug)
			 * @param editor whether application was started with option --hear
			 */
			static void writeHelpUsage(const string& sfor, bool editor);
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
			 * @return whether after clearing all holding of debug session queue is empty
			 */
			OVERWRITE bool clearHoldingFolder(const string& folder, const string& subroutine);
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
			 * how often all folders are running.
			 *
			 * @param locked whether DEBUGSESSIONCHANGES locked before
			 * @param outside whether should be included external running subroutines
			 * @return map of count for all folders
			 */
			OVERWRITE map<string, unsigned long> getRunningFolderList(bool locked, bool outside= false);
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
			OVERWRITE vector<string> getUsableFolders(const string& str);
			/**
			 * search in subroutine list from given folder
			 * fro results beginning with given string
			 *
			 * @param folder name of folder from which subroutines should be searched
			 * @param str subroutine names should beginning with this string
			 * @return vector of all possible subroutines
			 */
			OVERWRITE vector<string> getUsableSubroutines(const string& folder, const string& str);
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
													const direction_t show, IPPITimePattern* curTime,
													const unsigned long nr= 0);
			/**
			 * check whether getting debug session queue from server
			 * is empty
			 *
			 * @return whether queue is empty
			 */
			OVERWRITE bool emptyDbgQueue() const;
			/**
			 * save current or follow debug session queue with ending <code>.dbgsession</code>
			 * into file on current file system where client started
			 *
			 * @param file name of file where should stored
			 * @return whether saving was correct done
			 */
			OVERWRITE bool saveFile(const string& file);
			/**
			 * close opened file to store debug session content
			 */
			OVERWRITE void closeFile();
			/**
			 * load before saved debug session from file system
			 * where client was started
			 *
			 * @param file name of file which should loaded
			 * @return whether loading was correct done
			 */
			OVERWRITE bool loadFile(const string& file);
			/**
			 * destructor of server transaction
			 */
			virtual ~ClientTransaction();

		protected:
			/**
			 * error handling object
			 */
			EHObj m_pError;

		private:
			typedef map<ppi_time,
						SHAREDPTR::shared_ptr<
							IDbFillerPattern::dbgSubroutineContent_t> > dbSubroutineInfoType;
			/**
			 * parameter of command
			 * with possible follow parameters
			 */
			struct params_t
			{
				string param;
				vector<SHAREDPTR::shared_ptr<params_t> > follow;
			};
			struct get_params_vec_t
			{
				vector<SHAREDPTR::shared_ptr<params_t> >* content;
			};
			struct sorted_debugSessions_t
			{
				/**
				 * time of starting folder
				 */
				ppi_time tm_start;
				/**
				 * time of ending folder
				 */
				ppi_time tm_end;
				/**
				 * all trying inform folder to start
				 */
				dbSubroutineInfoType inform;
				/**
				 * all external running subroutines
				 * before starting folder
				 */
				dbSubroutineInfoType external;
				/**
				 * one folder running from
				 * start to end
				 */
				dbSubroutineInfoType folder;
			};
			/**
			 * structure by finding position of subroutine
			 */
			struct found_subroutine_t
			{
				/**
				 * inside which vector
				 * subroutine found
				 */
				action_e vector;
				/**
				 * first subroutine
				 * inside vector
				 */
				dbSubroutineInfoType::iterator first;
				/**
				 * previous subroutine
				 */
				dbSubroutineInfoType::iterator previous;
				/**
				 * founded subroutine
				 */
				dbSubroutineInfoType::iterator found;
				/**
				 * count of subroutine
				 * inside inform or external vector
				 */
				size_t count;
				/**
				 * last subroutine
				 * from vector
				 */
				dbSubroutineInfoType::iterator last;
			};

			/**
			 * full debug session content map
			 * sorted by time
			 */
			typedef map<ppi_time, vector<
							SHAREDPTR::shared_ptr<
								IDbFillerPattern::dbgSubroutineContent_t> > > debugSessionTimeMap;
			typedef vector<
							SHAREDPTR::shared_ptr<
								IDbFillerPattern::dbgSubroutineContent_t> > sharedDebugSessionVec;
			/**
			 * sorted debug session content per folder
			 * spited to inform external and running subroutines inside time
			 */
			typedef map<string, map<ppi_time, sorted_debugSessions_t> > sortedDebugSessionMap;
			typedef map<string, map<ppi_time, sorted_debugSessions_t> >::iterator sortedFolderSessionIterator;
			typedef map<ppi_time, sorted_debugSessions_t>::iterator folderSessionIterator;
			typedef map<ppi_time, sorted_debugSessions_t>::reverse_iterator folderSessionReverseIterator;
			/**
			 * type definition for an parameter pointer
			 */
			typedef SHAREDPTR::shared_ptr<params_t> parameter_type;
			/**
			 * type definition for an vector of parameter pointer
			 */
			typedef vector<parameter_type> parameter_types;

			/**
			 * whether client is connected
			 * with server
			 */
			bool m_bConnected;
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
			 * additional output string
			 * before prompt
			 */
			string m_sAddPromptString;
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
			 * current folder name in which searching
			 * with CURDEBUG
			 */
			string m_sCurrentFolder;
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
			 * object of an second client running inside thread
			 */
			SHAREDPTR::shared_ptr<IHearingThreadPattern> m_oHearObj;
			/**
			 * second pointer to hearing object
			 * to reach from user transaction
			 */
			SHAREDPTR::shared_ptr<IHearingThreadPattern> m_o2Client;
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
			 * hold debug sessions
			 * inside an sorted map<br />
			 * <code>map&lt; 'folder', map&lt; 'end time',
			 *      'debug content splitting as informed/external/subroutines' &gt; &gt;</code>
			 */
			sortedDebugSessionMap m_mSortedSessions;
			/**
			 * time returned from method <code>writeDebugSession()</code>
			 */
			ppi_time m_dbgSessTime;
			/**
			 * all possible commands with possible parameters
			 * to write inside command line editor
			 */
			map<string, parameter_types > m_mUserInteraction;
			/**
			 * all folders with subroutines
			 * getting as debug session from server
			 */
			map<string, set<string> > m_mFolderSubs;
			/**
			 * file to store debug session content
			 * getting from server
			 */
			ofstream m_oStoreFile;
			/**
			 * whether should be shown
			 * content loading from server
			 */
			bool m_bServerLoad;
			/**
			 * steps of loading content from server
			 */
			unsigned short m_sServerLoadStep;
			/**
			 * last time of new step setting
			 */
			ppi_time m_tLastLoad;
			/**
			 * current set password
			 */
			string m_sPassword;
			/**
			 * mutex by change user/password or first login
			 */
			pthread_mutex_t* m_PASSWORDCHECK;
			/**
			 * mutex to write clear hold variables
			 */
			pthread_mutex_t* m_DEBUGSESSIONCHANGES;
			/**
			 * mutex of changing prompt
			 */
			pthread_mutex_t* m_PROMPTMUTEX;
			/**
			 * mutex to set handle for loading
			 * content from server
			 */
			pthread_mutex_t* m_LOADMUTEX;

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
			 * set new loading step from server
			 * or signal end of load
			 *
			 * @param get whether client get content form server
			 */
			void load(bool get);
			/**
			 * reading one character
			 * or specialcharacter sequence
			 * from command line
			 *
			 * @return integer number of character
			 */
			int getch();
			/**
			 * set handle for termios reading
			 * whether was written ok.<br />
			 * this only be useful when second connection
			 * for hearing be started
			 *
			 * @param read whether was written ok
			 */
			OVERWRITE void correctTC(bool read);
			/**
			 * set TC backup also for hearing client
			 *
			 * @param current TC
			 */
			OVERWRITE void setTcBackup(const struct termios& backup);
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
			 * whether inside folder vector
			 * exist any subroutine from vector
			 *
			 * @param folder vector of subroutines
			 * @param subroutines vector where one of them should exist
			 * @return whether exist
			 */
			bool subroutineSet(const dbSubroutineInfoType& folder,
							const vector<string>& subroutines);
			/**
			 * implement subroutine getting from server
			 * or loaded from hard disk
			 * inside an sorted array of subroutines
			 *
			 * @param content structure of info about the read subroutine
			 */
			void implementFolderSorting(
							SHAREDPTR::shared_ptr<
								IDbFillerPattern::dbgSubroutineContent_t> content);
			/**
			 * search subroutine with current time
			 * and return an structure with found pointer
			 * and inside which vector found (inform, external or folders)
			 *
			 * @param curTime currently time for searching
			 * @param folderObj iterator for currently folder object where searching
			 * @param subroutines for which subroutines searching
			 * @param pos which position should be searching, when action 'none' and direction all'
			 *            searching for current time
			 * @return structure where found
			 */
			found_subroutine_t currentSubroutineTyp(IPPITimePattern* curTime,
													folderSessionIterator folderObj,
													vector<string>& subroutines		);
			/**
			 * go to next subroutine which should be displayed
			 *
			 * @param show which type of subroutine should be shown as next.
			 * @param nCurFolder current folder count of polling
			 * @param curContent current folder of polling,
			 *                   which contain inform-, external- and folder-subroutines.
			 *                   Changing to content which should be shown
			 * @param endContent end of polling folder iterator
			 * @param foundSubroutine current founded subroutine inside curContent
			 * @param endSubroutine last subroutine defined for non exist
			 * @param subroutines all subroutines which should be able to display
			 * @return subroutine which should be shown
			 */
			dbSubroutineInfoType::iterator
					getNextSubroutine(	direction_t& show,
										size_t& nCurFolder,
										folderSessionIterator& curContent,
										const folderSessionIterator& endContent,
										const found_subroutine_t& foundSubroutine,
										const dbSubroutineInfoType::iterator endSubroutine,
										const vector<string>& subroutines					);
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
			 * read password and user when not inserted than from
			 * from command line and compare with server
			 *
			 * @param descriptor file handle to compare with server
			 * @param user name of user or null string
			 * @param pwd password or null string
			 * @return whether entries was correct
			 */
			OVERWRITE bool compareUserPassword(IFileDescriptorPattern& descriptor, string& user, string& pwd, bool* bHear= NULL);
			/**
			 * get current setting password
			 *
			 * @return password string
			 */
			string getCurrentPassword();
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
			/**
			 * add command to allowed list.<br />
			 * when second parameter 'params' be set
			 * there will be returned an vector for all parameter
			 * after command, which can be filled with method <code>addParam()</code>.
			 * After filling this parameter can be used for other commands
			 * to implement the filled vector into other commands
			 * which should doing the same
			 *
			 * @param command name of command
			 * @param params vector of parameters to fill, or when filled add to command
			 */
			void addCommand(const string& command, get_params_vec_t* params= NULL);
			/**
			 * add parameter into an existing command.<br />
			 * when third parameter 'params' be set
			 * this can also use like by method <code>addCommand()</code>
			 * for more parameters after the current whitch will be set.<br />
			 * the name of parameter can be an special string
			 * whitch describe directly or an parameter beginning with an '#'<br />
			 * '#string' for an undefined string cannot be filled with tabulator<br />
			 * '#folder' for all 'folders' getting by HOLDDEBUG<br />
			 * '#subroutine' for all 'subroutines' from current folder<br />
			 * '#folderSub' for all 'folder:subroutine' strings
			 *
			 * @param into vector of parameter where should be filled into
			 * @param param name of parameter
			 * @param params vector of followed parameters
			 */
			void addParam(get_params_vec_t* into, const string& param, get_params_vec_t* params= NULL);
			/**
			 * set current folder in which searching
			 * with CURDEBUG
			 *
			 * @param folder current folder name
			 */
			void setCurrentFolder(const string& folder);
			/**
			 * select current folder in which searching
			 * with CURDEBUG
			 *
			 * @return current folder name
			 */
			string getCurrentFolder() const;
			/**
			 * fill command with parameters
			 * to array of tabulator completion
			 *
			 * @param command string of command with parameters
			 */
			void command(string command);
			/**
			 * send command to server
			 *
			 * @param descriptor open file descriptor to server
			 * @param command string of command sending
			 * @param bWaitEnd whether answer of server is for more rows and ending with done
			 * @return whether an error occurred
			 */
			bool send(IFileDescriptorPattern& descriptor, const string& command, bool bWaitEnd);
	};

}

#endif /*CLIENTTRANSACTION_H_*/
