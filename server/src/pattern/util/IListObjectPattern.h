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

using namespace std;


namespace design_pattern_world
{
	namespace util_pattern
	{
		/**
		 * pattern class for all threads
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
			 * return count of subroutine in folder
			 *
			 * @return count of subroutine
			 */
			virtual unsigned short getActCount()= 0;
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
			 * returning the type of the current object
			 *
			 * @return name of type of the subroutine
			 */
			virtual string getSubroutineType()= 0;
			/**
			 * returning the name of the folder
			 * in which this subroutine running
			 *
			 * @return name of folder
			 */
			virtual string getFolderName()= 0;
			/**
			 * returning the name of this subroutine
			 *
			 * @return name of the subroutine
			 */
			virtual string getSubroutineName()= 0;
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
			 * @return return measured value
			 */
			virtual double measure(const double actValue)=0;
			/**
			 * get value from subroutine
			 *
			 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
			 * 				This time only defined for external reading over OwPort's.
			 * @return current value
			 */
			virtual double getValue(const string& who)= 0;
			/**
			 * set value in subroutine.<br />
			 * All strings from parameter 'from' beginning with an one character type,
			 * followed from an colon 'r:' by ppi-reader, 'e:' by an account connected over Internet
			 * or 'i:' by intern folder:subroutine.
			 *
			 * @param value value which should be set
			 * @param from which folder:subroutine or account changing the value
			 */
			virtual void setValue(const double value, const string& from)= 0;
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
			virtual string getType()= 0;
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
			 * dummy destructor for pattern
			 */
			virtual ~IListObjectPattern() {};

		};
	}
}

#endif /*ILISTOBJECTPATTERN_H_*/
