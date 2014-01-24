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
	: switchClass("SHELL", folderName, subroutineName),
	  m_oMicroseconds(folderName, subroutineName, "microsec", false, false, this),
	  m_oMilliseconds(folderName, subroutineName, "millisec", false, false, this),
	  m_oSeconds(folderName, subroutineName, "sec", false, false, this),
	  m_oMinutes(folderName, subroutineName, "min", false, false, this),
	  m_oHours(folderName, subroutineName, "hour", false, false, this),
	  m_oDays(folderName, subroutineName, "day", false, false, this),
	  m_oMonths(folderName, subroutineName, "month", false, false, this),
	  m_oYears(folderName, subroutineName, "year", false, false, this),
	  m_bFixTimePoll(false),
	  m_EXECUTEMUTEX(Thread::getMutex("EXECUTEMUTEX")),
	  m_WRITTENVALUES(Thread::getMutex("WRITTENVALUES"))
	{ };
	virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * measure new value for subroutine
	 *
	 * @param actValue current value
	 * @return return measured value
	 */
	virtual IValueHolderPattern& measure(const ppi_value& actValue);
	/**
	 * set subroutine for output doing actions
	 *
	 * @param whether should write output
	 */
	virtual void setDebug(bool bDebug);
	/**
	 *  external command to send stopping to all CommandExec Threads
	 *
	 * @param bWait calling routine should wait until the thread is stopping
	 */
	virtual void stop(const bool bWait)
	{ Shell::stop(&bWait); };
	/**
	 *  external command to send stopping to all CommandExec Threads
	 *
	 * @param bWait calling routine should wait until the thread is stopping
	 */
	virtual void stop(const bool *bWait= NULL);
	/**
	 * destructor
	 */
	virtual ~Shell();

protected:
	/**
	 * defined-value for microseconds
	 */
	ListCalculator m_oMicroseconds;
	/**
	 * defined-value for milliseconds
	 */
	ListCalculator m_oMilliseconds;
	/**
	 * defined-value for seconds
	 */
	ListCalculator m_oSeconds;
	/**
	 * defined-value for minutes
	 */
	ListCalculator m_oMinutes;
	/**
	 * defined-value for hours
	 */
	ListCalculator m_oHours;
	/**
	 * defined-value for days
	 */
	ListCalculator m_oDays;
	/**
	 * defined-value for months
	 */
	ListCalculator m_oMonths;
	/**
	 * defined-value for years
	 */
	ListCalculator m_oYears;
	/**
	 * whether an polling time for shell script be set
	 */
	bool m_bFixTimePoll;

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
	 * mutex lock for last written values
	 */
	pthread_mutex_t* m_WRITTENVALUES;
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
	 * false when result should show actual command
	 * or true for last command.<br />
	 * Only when action 'wait' not be set
	 */
	bool m_bLastRes;
	/**
	 * whether command is blocking the hole process
	 */
	bool m_bBlock;
	/**
	 * whether subroutine get from triggered command later more result
	 */
	bool m_bMore;
	/**
	 * whether ending script should write error return value (!= 0)
	 * into log file
	 */
	bool m_bLogError;
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
	 * map container of all last written values inside folder list
	 */
	map<string, double> m_msdWritten;

	/**
	 * execute shell command on system or send command to client with X-Server
	 *
	 * @param action type of action (begincommand, whilecommand or endcommand)
	 * @param command shell command to execute
	 */
	int system(const string& action, string command);
	/**
	 * set new value inside folder list
	 *
	 * @param always whether should write all values (normal behavior) or only changed (for debugging when not wait)
	 * @param shellcommand shell command for output
	 * @param command string beginning with 'PPI-SET' getting from output on SHELL
	 * @return whether method can reading correctly given command from SHELL
	 */
	bool setValue(bool always, const string& shellcommand, const string& command);
};

#endif /*SHELL_H_*/
