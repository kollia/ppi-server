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
#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "../util/structures.h"

#include "../util/properties/configpropertycasher.h"

#include "switch.h"
#include "ListCalculator.h"

namespace ports
{
	class Output : public switchClass
	{
		public:
			/**
			 * create object of class Output.
			 *
			 * @param folder in which folder the routine running
			 * @param subroutine name of the routine
			 * @param objectID count of folder when defined inside an object, otherwise 0
			 */
			Output(string folderName, string subroutineName, unsigned short objectID);
			/**
			 * create object of class Output.<br />
			 * Constructor for an extendet object
			 *
			 * @param type type of object from extendet class
			 * @param folder in which folder the routine running
			 * @param subroutine name of the routine
			 * @param objectID count of folder when defined inside an object, otherwise 0
			 */
			Output(string type, string folderName, string subroutineName, unsigned short objectID);
			/**
			 * initialing object of Output
			 *
			 * @param properties the properties in file measure.conf
			 * @param pStartFolder reference to all folder
			 * @return whether initalization was ok
			 */
			virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
			/**
			 * set subroutine for output doing actions
			 *
			 * @param whether should write output
			 */
			virtual void setDebug(bool bDebug);
			/**
			 * return whether is subroutine set for output doing actions
			 *
			 * @return whether subroutine do output
			 */
			virtual bool isDebug();
			/**
			 * this method will be called from any measure thread to set as observer
			 * for starting own folder to get value from foreign folder
			 * if there the value was changing
			 *
			 * @param observer measure thread which containing the own folder
			 */
			virtual void setObserver(IMeasurePattern* observer);
			/**
			 * measure whether switch value is set or not
			 *
			 * @param actValue current value
			 * @return return measured value
			 */
			virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue);
			/**
			 * destructor to delete all value ListCalculator
			 */
			virtual ~Output();

		protected:
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
			virtual bool range(bool& bfloat, double* min, double* max);

		private:
			/**
			 * mutex lock for debug
			 */
			pthread_mutex_t *m_DEBUG;
			/**
			 * whether set DEBUG over server for this folder
			 */
			bool m_bDebug;
			/**
			 * log level when output string defined for logging
			 */
			int m_nLogLevel;
			/**
			 * whether output string should displayed on command line
			 */
			bool m_bCL;
			/**
			 * whether output calculating over SWITCH type
			 * or by define subroutine for debugging
			 */
			bool m_bNeedSwitch;
			/**
			 * text for output before value
			 */
			vector<string> m_vsStrings;
			/**
			 * calculating value for output
			 */
			vector<ListCalculator*> m_voVal;
	};
}

#endif /*OUTPUT_H_*/
