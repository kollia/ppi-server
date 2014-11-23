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
#ifndef ILISTOBJECTPATTERN_H_
#define ILISTOBJECTPATTERN_H_

#include <string>

#include "iactionpropertypattern.h"
#include "imeasurepattern.h"
#include "IPPIValuesPattern.h"

#include "../../util/stream/ppivalues.h"

using namespace std;


namespace design_pattern_world
{
	namespace util_pattern
	{
		/**
		 * pattern class for all list objects
		 *
		 * @autor Alexander Kolli
		 * @version 1.0.0
		 */
		class IListObjectPattern
		{
			public:
			/**
			 * check whether subroutine need an external owreader server
			 *
			 * @return whether need an server
			 */
			virtual bool needServer() const= 0;
			/**
			 * check whether object found for chip in subroutine correct server.<br />
			 * Only when server needed.
			 *
			 * @return whether server found
			 */
			virtual bool hasServer() const= 0;
			/**
			 * check whether subroutine has possibility to start
			 * any action per time
			 *
			 * @return null string when subroutine can start per time, otherwise an error message string
			 */
			virtual string checkStartPossibility()= 0;
			/**
			 * start behavior to starting subroutine per time
			 *
			 * @param tm time to starting subroutine action
			 * @return whether starting was successful
			 */
			virtual bool startingBy(const ppi_time& tm)= 0;
			/**
			 * whether subroutine has the incoming sub-variable
			 *
			 * @subvar name of sub-variable
			 * @return whether subroutine has this varibale
			 */
			virtual bool hasSubVar(const string& subvar) const= 0;
			/**
			 * return content of sub-variable from aktual subroutine
			 *
			 * @subvar name of sub-variable
			 * @return value of sub-var
			 */
			virtual ppi_value getSubVar(const string& subvar) const= 0;
			/**
			 * return count of subroutine in folder
			 *
			 * @return count of subroutine
			 */
			virtual unsigned short getActCount()= 0;
			/**
			 * return true when subroutine need information from other subroutines by changing.<br />
			 * otherwise false.
			 *
			 * @return whether subroutine need information from other subroutines
			 */
			virtual bool needObserver() const= 0;
			/**
			 * this method will be called from any measure thread to set as observer
			 * for starting own folder to get value from foreign folder
			 * if there the value was changing
			 *
			 * @param observer measure thread which containing the own folder
			 */
			virtual void setObserver(IMeasurePattern* observer)= 0;
			/**
			 * fill observer vector to inform other folder if value changed
			 *
			 * @param observer measure thread which containing the own folder
			 * @param folder name of folder which should be informed
			 * @param subroutine name of subroutine which should be informed
			 * @param parameter name of parameter where the own subroutine is occured
			 */
			virtual void informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)= 0;
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
			virtual void removeObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)= 0;
			/**
			 * return string of defined observer links
			 * for current subroutine
			 */
			virtual string getObserversString() const= 0;
			/**
			 * set whether subroutine has correct access to device
			 *
			 * @param access whether own object have correct access to device
			 */
			virtual void setDeviceAccess(const bool access)= 0;
			/**
			 * ask subroutine whether she has an correct access to device
			 *
			 * @return whether the access is correct
			 */
			virtual bool hasDeviceAccess() const= 0;
			/**
			 * returning all groups, seperatly with colons
			 *
			 * @return string of groups
			 */
			virtual string getPermissionGroups()= 0;
			/**
			 * set subroutine for output doing actions
			 *
			 * @param whether should write output
			 */
			virtual void setDebug(bool bDebug)= 0;
			/**
			 * return whether is subroutine set for output doing actions
			 *
			 * @return whether subroutine do output
			 */
			virtual bool isDebug()= 0;
			/**
			 * returning ostringstream object which should written on right time
			 * by next pass into Terminal for output on command line
			 *
			 * @return string stream for writing by next pass
			 */
			virtual ostringstream& out()= 0;
			/**
			 * writing into string stream into terminal
			 * when definition WRITEDEBUGALLLINES not be set
			 */
			virtual void writeDebugStream()= 0;
			/**
			 * returning the type of the current object
			 *
			 * @return name of type of the subroutine
			 */
			virtual string getSubroutineType() const = 0;
			/**
			 * returning the name of the folder
			 * in which this subroutine running
			 *
			 * @return name of folder
			 */
			virtual string getFolderName() const = 0;
			/**
			 * returning the name of this subroutine
			 *
			 * @return name of the subroutine
			 */
			virtual string getSubroutineName() const = 0;
			/**
			 * return true if subroutine is only for switching between 0 and 1
			 *
			 * @return whether subroutine is for switching
			 */
			virtual bool onlySwitch()= 0;
			/**
			 * measure new value for subroutine
			 *
			 * @param actValue current value
			 * @return measured value with last changing time when not changed by self
			 */
			virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue)=0;
			/**
			 * get value from subroutine
			 *
			 * @param who declare who need the value information
			 * @return current value with last changing time
			 */
			virtual auto_ptr<IValueHolderPattern> getValue(const InformObject& who)= 0;
			/**
			 * set value in subroutine
			 *
			 * @param value value which should be set with last changing time when set,
			 *              otherwise method create own time
			 * @param from which folder:subroutine or account changing the value
			 */
			virtual void setValue(const IValueHolderPattern& value,
							const InformObject& from)= 0;
			/**
			 * informing that variable wasn't change.<br />
			 * for better performance, measure-thread do not set
			 * always value when not changed. But for sub-variable
			 * .changed need subroutine to know when value not be changed
			 */
			virtual void noChange()= 0;
			/**
			 * set measure thread which run this object with method <code>measure()</code>
			 *
			 * @param thread measure thread
			 */
			virtual void setRunningThread(IMeasurePattern* thread)= 0;
			/**
			 * return measure thread which run this object with method <code>measure()</code>
			 *
			 * @return measure thread
			 */
			virtual IMeasurePattern* getRunningThread()= 0;
			/**
			 * return type of subroutine
			 *
			 * @return type name
			 */
			virtual string getType() const= 0;
			/**
			 * return info whether subroutine need last changing time
			 *
			 * @return whether need time
			 */
			virtual bool needChangingTime() const= 0;
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
			virtual bool range(bool& bfloat, double* min, double* max)= 0;
			/**
			 *  external command to send stopping to all subroutines when needed
			 *
			 * @param bWait calling routine should wait until the thread is stopping
			 */
			virtual void stop(const bool bWait)= 0;
			/**
			 *  external command to send stopping to all subroutines.<br />
			 *  dummy routine can be overloaded when need
			 *
			 * @param bWait calling routine should wait until the thread is stopping
			 */
			virtual void stop(const bool *bWait= NULL)= 0;
			/**
			 * dummy destructor for pattern
			 */
			virtual ~IListObjectPattern() {};

		};
	}
}

#endif /*ILISTOBJECTPATTERN_H_*/
