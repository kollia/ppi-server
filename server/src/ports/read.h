/**
 *   This file 'read.h' is part of ppi-server.
 *   Created on: 16.04.2014
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

#ifndef READ_H_
#define READ_H_

#include <string>

#include "../util/debug.h"
#include "../util/smart_ptr.h"
#include "../util/URL.h"


#include "switch.h"
#include "ReadWorker.h"

namespace ports
{
	using namespace std;

	class Read : public switchClass
	{
	public:
		/**
		 * creating object of extended class
		 *
		 * @param type type of the current subroutine (extended class)
		 * @param folder name of the folder in which this subroutine running
		 * @param subroutine name from this subroutine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		Read(const string& folderName, const string& subroutineName, unsigned short objectID)
		: switchClass("READ", folderName, subroutineName, objectID),
		  m_bFirstRun(true),
		  m_bTimerStart(false),
		  m_oReader(folderName, subroutineName, this)
		{};
		/**
		 * creating object of extended class
		 *
		 * @param type type of the current subroutine (extended class)
		 * @param folder name of the folder in which this subroutine running
		 * @param subroutine name from this subroutine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		Read(const string& type, const string& folderName, const string& subroutineName,
						unsigned short objectID)
		: switchClass(type, folderName, subroutineName, objectID),
		  m_bFirstRun(true),
		  m_bTimerStart(false),
		  m_oReader(folderName, subroutineName, this)
		{};
		/**
		 * initial extended object to check whether write into database and define range of value.<br />
		 * Before called this method all parameters for method range have be set.
		 *
		 * @param properties the properties of subroutine from file measure.conf
		 * @param pStartFolder reference to all folder
		 * @return whether initalization was ok
		 */
		virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return measured value with last changing time when not changed by self
		 */
		virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue);
		/**
		 * check whether subroutine has possibility to start
		 * any action per time
		 *
		 * @return null string when subroutine can start per time, otherwise an error message string
		 */
		virtual string checkStartPossibility();
		/**
		 * start behavior to starting subroutine per time
		 *
		 * @param tm time to starting subroutine action
		 * @return whether starting was successful
		 */
		virtual bool startingBy(const ppi_time& tm)
		{ return m_oReader.startingBy(tm); };
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug);
		/**
		 *  external command to stop ReadWorker thread when running
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void stop(const bool bWait)
		{ Read::stop(&bWait); };
		/**
		 *  external command to stop ReadWorker thread when running
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void stop(const bool *bWait= NULL)
		{ m_oReader.stop(bWait); };

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
		 * whether measure methode running as first time
		 */
		bool m_bFirstRun;
		/**
		 * whether reading should starting
		 * by timer subroutine request
		 */
		bool m_bTimerStart;
		/**
		 * whether parameter begin/while/end
		 * be set inside subroutine
		 */
		bool m_bWhileSet;
		/**
		 * call back thread of do some readings
		 */
		ReadWorker m_oReader;
		/**
		 * value of last switch action
		 */
		ppi_value m_nDo;
	};

} /* namespace ports */
#endif /* READ_H_ */
