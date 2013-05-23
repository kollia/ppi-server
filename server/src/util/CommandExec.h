/**
 *   This file 'CommandExec.h' is part of ppi-server.
 *   Created on: 08.10.2011
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

#ifndef COMMANDEXEC_H_
#define COMMANDEXEC_H_

#include <sys/types.h>

#include <deque>

#include "../pattern/util/IMeasureSet.h"

#include "../util/smart_ptr.h"
#include "../util/thread/Thread.h"

class CommandExec : public Thread
{
public:
	/**
	 * constructor of CommandExec to run some commands on shell
	 *
	 * @param port Interface to set value in an subroutine
	 */
	CommandExec(IMeasureSet* port, bool logError)
	: Thread("CommandExec", true),
	  m_bStarted(false),
	  m_bLogging(false),
	  m_bLogError(logError),
	  m_pPort(port),
	  m_tScriptPid(0),
	  m_nStopSignal(0),
	  m_bWait(false),
	  m_bBlock(false)
	{ m_RESULTMUTEX= getMutex("RESULTMUTEX");
	  m_WAITMUTEX= getMutex("WAITMUTEX");     };
	/**
	 * writing command on shell
	 *
	 * @param thread command object to provide command on shell.<br />
	 *               when variable block is true, thread should be the same
	 *               object from the last pass
	 * @param command script or command to writing on shell
	 * @param result output result of command
	 * @param more returning whether method has more result content for the next time
	 * @param wait whether method should wait for result of command
	 * @param block whether method should'nt wait for result of command,
	 *              but shown every next pass when not ending between
	 * @param debug whether subroutine is inside debugging mode
	 * @return returning error level of command or script
	 */
	static int command_exec(SHAREDPTR::shared_ptr<CommandExec> thread, string command, vector<string>& result,
					bool& more, bool wait, bool block, bool debug);
	/**
	 * start method to running the thread paralell
	 *
	 * @param args arbitary optional defined parameter to get in initialisation method init
	 * @param bHold should the caller wait of thread by ending.<br />
	 * 				default is false
	 * @return NULL when all ok if bHold is false, otherwise the returnvalue of the thread in an void pointer
	 */
	virtual int start(void *args= NULL, bool bHold= false)
	{ m_bStarted= true; return Thread::start(args, bHold); };
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool bWait)
	{ return CommandExec::stop(&bWait); };
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool *bWait= NULL);
	/**
	 * whether process was started any time before
	 *
	 * @return whether porcess was started
	 */
	bool started()
	{ return m_bStarted; };
	/**
	 * set folder and subroutine name
	 * for which shell command running
	 *
	 * @param folder name of folder for which started
	 * @param subroutine name of subroutine for which started
	 */
	void setFor(const string& folder, const string& subroutine)
	{ m_sFolder= folder; m_sSubroutine= subroutine; };
	/**
	 * set map of all last written values inside folder list
	 *
	 * @param pointer of map
	 * @param WRITTENVALUES extern defined mutex for all written values
	 */
	void setWritten(map<string, double>* written, pthread_mutex_t* WRITTENVALUES);
	/**
	 * get child process from given process
	 *
	 * @param actPid current pid from searching child process
	 * @return one ore more child processes
	 */
	vector<pid_t> getChildProcess(pid_t actPid, vector<string>* commands) const;
	/**
	 * make over shell command an egrep over all processes
	 *
	 * @param pid searching pid in run processes
	 * @return vector string of egrep result
	 */
	vector<string> grepPS(pid_t pid) const;
	/**
	 * returning current output of command
	 */
	vector<string> getOutput();
	/**
	 * destructor
	 */
	virtual ~CommandExec()
	{ stop(true);
	  DESTROYMUTEX(m_RESULTMUTEX);
	  DESTROYMUTEX(m_WAITMUTEX);   };

private:
	/**
	 * mutex of locking output result
	 */
	pthread_mutex_t* m_RESULTMUTEX;
	/**
	 * mutex for wait flag ->
	 * whether folder-list waiting for result
	 */
	pthread_mutex_t* m_WAITMUTEX;
	/**
	 * extern mutex for last written values
	 */
	pthread_mutex_t* m_externWRITTENVALUES;
	/**
	 * whether thread was started
	 */
	bool m_bStarted;
	/**
	 * whether should write shell output into log-file
	 */
	bool m_bLogging;
	/**
	 * whether ending script should write error return value (!= 0)
	 * into log file
	 */
	bool m_bLogError;
	/**
	 * defined current log level from shell script
	 */
	int m_nLogLevel;
	/**
	 * Interface to set value in an subroutine
	 */
	IMeasureSet* m_pPort;
	/**
	 * command to execute
	 */
	string m_sCommand;
	/**
	 * for which folder shell command running
	 */
	string m_sFolder;
	/**
	 * for which subroutine shell command running
	 */
	string m_sSubroutine;
	/**
	 * output result of command
	 */
	deque<string> m_qOutput;
	/**
	 * output for logging
	 */
	deque<string> m_qLog;
	/**
	 * process id of own current pid
	 */
	pid_t m_tOwnPid;
	/**
	 * process id from running shell script
	 */
	pid_t m_tScriptPid;
	/**
	 * signal type to send by stopping<br />
	 * allowed signals from shell script set with 'running-process <pid> <signal>' are:<br />
	 * <code>SIGHUP, SIGINT, SIGQUIT, SIGTRAP, SIGABRT, SIGBUS,
	 * SIGKILL, SIGUSR1, SIGUSR2, SIGPIPE, SIGALRM, SIGTERM, SIGSTOP</code>
	 */
	int m_nStopSignal;
	/**
	 * whether subroutine should waiting for result of command
	 */
	bool m_bWait;
	/**
	 * whether shell script will be blocking
	 */
	bool m_bBlock;
	/**
	 * whether subroutine is in debugging mode
	 */
	bool m_bDebug;
	/**
	 * map container of all last written values inside folder list
	 */
	map<string, double>* m_msdWritten;

	/**
	 * initialization of starting command
	 *
	 * @args name of command in an string
	 * @return whether initialization was successful
	 */
	virtual int init(void *args);
	/**
	 * execute of command on shell
	 *
	 * @return defined error code from extended class
	 */
	virtual int execute();
	/**
	 * read output line from script
	 *
	 * @param bWait whether starting thread wait for output result
	 * @param bDebug whether subroutine is in debugging mode
	 * @param sline one output line from script
	 */
	void readLine(const bool& bWait, const bool& bDebug, string sline);
	/**
	 * set new value inside folder list
	 *
	 * @param command string beginning with 'PPI-SET' getting from output on SHELL
	 * @param bLog whether errors should be written inside output queue
	 * @return whether method can reading correctly given command from SHELL
	 */
	void setValue(const string& command, bool bLog);
	/**
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() {};
};

#endif /* COMMANDEXEC_H_ */
