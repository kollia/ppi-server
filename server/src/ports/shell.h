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
#ifndef SHELL_H_
#define SHELL_H_

#include "switch.h"

#include "../portserver/libs/interface/OWInterface.h"

#include "../util/CommandExec.h"
#include "../util/properties/configpropertycasher.h"

using namespace util;


class Shell : public switchClass
{
public:
	Shell(string folderName, string subroutineName)
	: switchClass(folderName, subroutineName),
	  m_EXECUTEMUTEX(Thread::getMutex("EXECUTEMUTEX"))
	{ };
	virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * measure new value for subroutine
	 *
	 * @param actValue current value
	 * @return return measured value
	 */
	virtual double measure(const double actValue);
	virtual ~Shell();

protected:
	/**
	 * this method is an dummy
	 * because the value can not write into database
	 * and be never set
	 *
	 * @param bfloat whether the values can be float variables
	 * @param min the minimal value
	 * @param max the maximal value
	 * @return whether the range is defined or can set all
	 */
	virtual bool range(bool& bfloat, double* min, double* max);

private:
	/**
	 * mutex lock to execute <code>command_exec()</code>
	 */
	pthread_mutex_t* m_EXECUTEMUTEX;
	/**
	 * begin command when set
	 */
	string m_sBeginCom;
	/**
	 * while command when set
	 */
	string m_sWhileCom;
	/**
	 * end command when set
	 */
	string m_sEndCom;
	/**
	 * value from last pass
	 */
	bool m_bLastValue;
	/**
	 * whether subroutine should waiting for result of command
	 */
	bool m_bWait;
	/**
	 * whether command is blocking the hole process
	 */
	bool m_bBlock;
	/**
	 * whether subroutine get from triggered command later more result
	 */
	bool m_bMore;
	/**
	 * whether commands should be sending to an client with X-Server access
	 * content of variable is environment variable DISPLAY ('0', '0.0', '0.1', '1', ... )
	 */
	string m_sGUI;
	/**
	 * whether shell command should start in an other system account
	 * content of variable is exist account name
	 */
	string m_sUserAccount;
	/**
	 * server which read and write on one wire device
	 */
	OWI m_pOWServer;
	/**
	 * all running shell threads
	 */
	vector<SHAREDPTR::shared_ptr<CommandExec> > m_vCommandThreads;

	/**
	 * execute shell command on system or send command to client with X-Server
	 *
	 * @param action type of action (begincommand, whilecommand or endcommand)
	 * @param command shell command to execute
	 */
	int system(const string& action, string command);
};

#endif /*SHELL_H_*/
