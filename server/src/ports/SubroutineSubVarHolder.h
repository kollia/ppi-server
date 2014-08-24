/**
 *   This file 'SubroutineSubVarHolder.h' is part of ppi-server.
 *   Created on: 06.04.2014
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

#ifndef SUBROUTINESUBVARHOLDER_H_
#define SUBROUTINESUBVARHOLDER_H_

#include "../pattern/util/IListObjectPattern.h"

#include "../util/stream/ppivalues.h"

#include "../util/debug.h"

using namespace std;
using namespace design_pattern_world::util_pattern;

namespace ports
{
	class SubroutineSubVarHolder : public IListObjectPattern
	{
	public:
		SubroutineSubVarHolder(IListObjectPattern* oSubroutine, const string& sSubVar)
		: m_oSubroutine(oSubroutine),
		  m_sSubVar(sSubVar)
		{};

		/**
		 * check whether subroutine need an external owreader server
		 *
		 * @return whether need an server
		 */
		virtual bool needServer() const
		{ return m_oSubroutine->needServer(); };
		/**
		 * check whether object found for chip in subroutine correct server.<br />
		 * Only when server needed.
		 *
		 * @return whether server found
		 */
		virtual bool hasServer() const
		{ return m_oSubroutine->hasServer(); };
		/**
		 * check whether subroutine has possibility to start
		 * any action per time
		 *
		 * @return null string when subroutine can start per time, otherwise an error message string
		 */
		virtual string checkStartPossibility()
		{ return m_oSubroutine->checkStartPossibility(); };
		/**
		 * start behavior to starting subroutine per time
		 *
		 * @param tm time to starting subroutine action
		 * @return whether starting was successful
		 */
		virtual bool startingBy(const ppi_time& tm)
		{ return m_oSubroutine->startingBy(tm); };
		/**
		 * whether subroutine has the incoming sub-variable
		 *
		 * @subvar name of sub-variable
		 * @return whether subroutine has this varibale
		 */
		virtual bool hasSubVar(const string& subvar) const
		{ return m_oSubroutine->hasSubVar(subvar); };
		/**
		 * return content of sub-variable from aktual subroutine
		 *
		 * @subvar name of sub-variable
		 * @return value of sub-var
		 */
		virtual ppi_value getSubVar(const string& subvar) const
		{ return m_oSubroutine->getSubVar(subvar); };
		/**
		 * return count of subroutine in folder
		 *
		 * @return count of subroutine
		 */
		virtual unsigned short getActCount()
		{ return m_oSubroutine->getActCount(); };
		/**
		 * return true when subroutine need information from other subroutines by changing.<br />
		 * otherwise false.
		 *
		 * @return whether subroutine need information from other subroutines
		 */
		virtual bool needObserver() const
		{ return m_oSubroutine->needObserver(); };
		/**
		 * this method will be called from any measure thread to set as observer
		 * for starting own folder to get value from foreign folder
		 * if there the value was changing
		 *
		 * @param observer measure thread which containing the own folder
		 */
		virtual void setObserver(IMeasurePattern* observer)
		{ m_oSubroutine->setObserver(observer); };
		/**
		 * fill observer vector to inform other folder if value changed
		 *
		 * @param observer measure thread which containing the own folder
		 * @param folder name of folder which should be informed
		 * @param subroutine name of subroutine which should be informed
		 * @param parameter name of parameter where the own subroutine is occured
		 */
		virtual void informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
		{ m_oSubroutine->informObserver(observer, folder, subroutine, parameter); };
		/**
		 * remove from observer from information when value changed.<br />
		 * This method remove the observer only when all 'folder:subroutine parameter' values
		 * be removed.
		 *
		 * @param observer measure thread which containing the own folder
		 * @param folder name of folder which should be removed from information
		 * @param subroutine name of subroutine which should be removed from information
		 * @param parameter name of parameter where the own subroutine is occured
		 */
		virtual void removeObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
		{ m_oSubroutine->removeObserver(observer, folder, subroutine, parameter); };
		/**
		 * return string of defined observer links
		 * for current subroutine
		 */
		OVERWRITE string getObserversString() const
		{ return m_oSubroutine->getObserversString(); };
		/**
		 * set whether subroutine has correct access to device
		 *
		 * @param access whether own object have correct access to device
		 */
		virtual void setDeviceAccess(const bool access)
		{ m_oSubroutine->setDeviceAccess(access); };
		/**
		 * ask subroutine whether she has an correct access to device
		 *
		 * @return whether the access is correct
		 */
		virtual bool hasDeviceAccess() const
		{ return m_oSubroutine->hasDeviceAccess(); };
		/**
		 * returning all groups, seperatly with colons
		 *
		 * @return string of groups
		 */
		virtual string getPermissionGroups()
		{ return m_oSubroutine->getPermissionGroups(); };
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug)
		{ m_oSubroutine->setDebug(bDebug); };
		/**
		 * return whether is subroutine set for output doing actions
		 *
		 * @return whether subroutine do output
		 */
		virtual bool isDebug()
		{ return m_oSubroutine->isDebug(); };
		/**
		 * returning ostringstream object which should written on right time
		 * by next pass into Terminal for output on command line
		 *
		 * @return string stream for writing by next pass
		 */
		virtual ostringstream& out()
		{ return m_oSubroutine->out(); };
		/**
		 * writing into string stream into terminal
		 * when definition WRITEDEBUGALLLINES not be set
		 */
		virtual void writeDebugStream()
		{ m_oSubroutine->writeDebugStream(); };
		/**
		 * returning the type of the current object
		 *
		 * @return name of type of the subroutine
		 */
		virtual string getSubroutineType() const
		{ return m_oSubroutine->getSubroutineType(); };
		/**
		 * returning the name of the folder
		 * in which this subroutine running
		 *
		 * @return name of folder
		 */
		virtual string getFolderName() const
		{ return m_oSubroutine->getFolderName(); };
		/**
		 * returning the name of this subroutine
		 *
		 * @return name of the subroutine
		 */
		virtual string getSubroutineName() const
		{ return m_oSubroutine->getSubroutineName(); };
		/**
		 * return true if subroutine is only for switching between 0 and 1
		 *
		 * @return whether subroutine is for switching
		 */
		virtual bool onlySwitch()
		{ return m_oSubroutine->onlySwitch(); };
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return measured value with last changing time when not changed by self
		 */
		virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue)
		{ return m_oSubroutine->measure(actValue); };
		/**
		 * get value from subroutine
		 *
		 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
		 * 				This time only defined for external reading over OwPort's.
		 * @return current value with last changing time
		 */
		virtual auto_ptr<IValueHolderPattern> getValue(const string& who);
		/**
		 * set value in subroutine.<br />
		 * All strings from parameter 'from' beginning with an one character type,
		 * followed from an colon 'r:' by ppi-reader, 'e:' by an account connected over Internet
		 * or 'i:' by intern folder:subroutine.
		 *
		 * @param value value which should be set with last changing time when set, otherwise method create own time
		 * @param from which folder:subroutine or account changing the value
		 */
		virtual void setValue(const IValueHolderPattern& value, const string& from)
		{ m_oSubroutine->setValue(value, from); };
		/**
		 * informing that variable wasn't change.<br />
		 * for better performance, measure-thread do not set
		 * always value when not changed. But for sub-variable
		 * .changed need subroutine to know when value not be changed
		 */
		virtual void noChange()
		{ m_oSubroutine->noChange(); };
		/**
		 * set measure thread which run this object with method <code>measure()</code>
		 *
		 * @param thread measure thread
		 */
		virtual void setRunningThread(IMeasurePattern* thread)
		{ m_oSubroutine->setRunningThread(thread); };
		/**
		 * return measure thread which run this object with method <code>measure()</code>
		 *
		 * @return measure thread
		 */
		virtual IMeasurePattern* getRunningThread()
		{ return m_oSubroutine->getRunningThread(); };
		/**
		 * return type of subroutine
		 *
		 * @return type name
		 */
		virtual string getType() const
		{ return m_oSubroutine->getType(); };
		/**
		 * return info whether subroutine need last changing time
		 *
		 * @return whether need time
		 */
		virtual bool needChangingTime() const
		{ return m_oSubroutine->needChangingTime(); };
		/**
		 * set min and max parameter to the range which can be set for this subroutine.<br />
		 * If the subroutine is set from 0 to 1 and float false, the set method sending only 0 and 1 to the database.
		 * Also if the values defined in an bit value 010 (dec:2) set 0 and for 011 (dec:3) set 1 in db.
		 * Otherwise the range is only for calibrate the max and min value if set from client outher range.
		 * If pointer of min and max parameter are NULL, the full range of the double value can be used
		 *
		 * @param bfloat whether the values can be float variables
		 * @param min the minimal value
		 * @param max the maximal value
		 * @return whether the range is defined or can set all
		 */
		virtual bool range(bool& bfloat, double* min, double* max)
		{ return m_oSubroutine->range(bfloat, min, max); };
		/**
		 *  external command to send stopping to all subroutines when needed
		 *
		 * @param bWait calling routine should wait until the thread is stopping
		 */
		virtual void stop(const bool bWait)
		{ m_oSubroutine->stop(bWait); };
		/**
		 *  external command to send stopping to all subroutines.<br />
		 *  dummy routine can be overloaded when need
		 *
		 * @param bWait calling routine should wait until the thread is stopping
		 */
		virtual void stop(const bool *bWait= NULL)
		{ m_oSubroutine->stop(bWait); };
		/**
		 * dummy destructor for pattern
		 */
		virtual ~SubroutineSubVarHolder() {};

	private:
		/**
		 * real subroutine object
		 */
		IListObjectPattern* m_oSubroutine;
		/**
		 * definition of sub-variable
		 */
		string m_sSubVar;
	};
}

#endif /* SUBROUTINESUBVARHOLDER_H_ */
