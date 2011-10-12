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

#include "../util/smart_ptr.h"
#include "../util/thread/Thread.h"

class CommandExec : public Thread
{
public:
	/**
	 * constructor of CommandExec to run some commands on shell
	 */
	CommandExec()
	: Thread("CommandExec", 0),
	  m_bStarted(false)
	{ m_RESULTMUTEX= getMutex("RESULTMUTEX"); };
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
	 * @return returning error level of command or script
	 */
	static int command_exec(SHAREDPTR::shared_ptr<CommandExec> thread, string command, vector<string>& result, bool& more, bool wait, bool block);
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
	 * whether process was started any time before
	 *
	 * @return whether porcess was started
	 */
	bool started()
	{ return m_bStarted; };
	/**
	 * returning current output of command
	 */
	vector<string> getOutput();
	/**
	 * destructor
	 */
	virtual ~CommandExec()
	{ DESTROYMUTEX(m_RESULTMUTEX); };

private:
	/**
	 * mutex of locking output result
	 */
	pthread_mutex_t* m_RESULTMUTEX;
	/**
	 * whether thread was started
	 */
	bool m_bStarted;
	/**
	 * command to execute
	 */
	string m_sCommand;
	/**
	 * output result of command
	 */
	vector<string> m_vOutput;

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
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() {};
};

#endif /* COMMANDEXEC_H_ */
