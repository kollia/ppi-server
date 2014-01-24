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
#ifndef MEASUREDNESS_H_
#define MEASUREDNESS_H_

#include <string.h>

#include "../util/structures.h"
#include "../util/properties/configpropertycasher.h"

#include "portbaseclass.h"
#include "ListCalculator.h"


namespace ports
{
	using namespace util;

	/**
	 * class define the range to create an measuredness
	 * by measuring an time
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class Measuredness : public portBase
	{
	public:
		/**
		 * create object of class Measuredness
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		Measuredness(string folder, string subroutine)
		: portBase("MEASUREDNESS", folder, subroutine),
		  m_oMeasuredValue(folder, subroutine, "mavalue", true, false, this),
		  m_oBegin(folder, subroutine, "begin", true, true, this)
		{ };
		/**
		 * create object of class Measuredness.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		Measuredness(string type, string folder, string subroutine)
		: portBase(type, folder, subroutine),
		  m_oMeasuredValue(folder, subroutine, "mavalue", true, false, this),
		  m_oBegin(folder, subroutine, "begin", true, true, this)
		{ };
		/**
		 * initialing object of Measuredness
		 *
		 * @param properties the properties in file measure.conf
		 * @param pStartFolder pointer to first folder of all subroutines
		 */
		bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual IValueHolderPattern& measure(const ppi_value& actValue);


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
		 * pointer to first folder of all subroutines
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;
		/**
		 * subroutine where measure the time
		 */
		ListCalculator m_oMeasuredValue;
		/**
		 * whether measure range should beginning
		 */
		ListCalculator m_oBegin;
	};

}

#endif /*MEASUREDNESS_H_*/
