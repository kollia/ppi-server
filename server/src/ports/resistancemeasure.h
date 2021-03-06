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
#ifndef RESISTORPORT_H_
#define RESISTORPORT_H_

#include <vector>

#include "../util/structures.h"

#include "../util/properties/configpropertycasher.h"

#include "ListCalculator.h"
#include "timemeasure.h"

namespace ports
{
	using namespace util;

	/**
	 * class meashuring ohm capacity
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ResistanceMeasure : public TimeMeasure
	{
		public:
			/**
			 * inctanciate class of ohm meashuring
			 *
			 * @param folderName name of folder in whitch procedures are running
			 * @param subroutineName name of subroutine inside the folder
			 * @param objectID count of folder when defined inside an object, otherwise 0
			 */
			ResistanceMeasure(string folderName, string subroutineName, unsigned short objectID)
			: TimeMeasure(folderName, subroutineName, objectID),
			  m_oMeasuredSubroutine(folderName, subroutineName, "mvalue", true, false, this) { };
			/**
			 * initial class of saving
			 *
			 * @param properties property casher for this subroutine with type RESISTANCE
			 * @param pStartFolder first pointer to all defined folders
			 */
			bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
			/**
			 *
			 */
			double getResistance();
			/**
			 * measure new value for subroutine
			 *
			 * @param actValue current value
			 * @return return measured value
			 */
			virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue);
			/**
			 * set subroutine for output doing actions
			 *
			 * @param whether should write output
			 */
			virtual void setDebug(bool bDebug);

		protected:
			unsigned long getCapacitance();

		private:
			/**
			 * whether class should measure own time
			 */
			bool m_bOwnMeasure;
			/**
			 * subroutine from whitch get the measured time
			 */
			ListCalculator m_oMeasuredSubroutine;
			/**
			 * subroutine whitch hold the ohmvalue whitch should be
			 * the curent measured time to write in database
			 * by pressing an button from an SAVE subroutine
			 */
			string m_sOhmValue;
			/**
			 * first pointer to all defined folders
			 */
			SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;

	};
}

#endif /*RESISTORPORT_H_*/
