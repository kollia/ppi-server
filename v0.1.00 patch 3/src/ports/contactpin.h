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
#ifndef CONTACTPIN_H_
#define CONTACTPIN_H_

#include "portbaseclass.h"

#include "../util/configpropertycasher.h"

namespace ports
{
	using namespace util;

	class contactPin : public portBase
	{
		protected:
			bool m_bContact;
			Pins m_tIn;
			Pins m_tOut;

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

		public:
			/**
			 * create object of class contactPin.
			 *
			 * @param folder in which folder the routine running
			 * @param subroutine name of the routine
			 */
			contactPin(string folderName, string subroutineName)
			: portBase("GETCONTACT", folderName, subroutineName) { };
			/**
			 * create object of class contactPin.<br />
			 * Constructor for an extendet object
			 *
			 * @param type type of object from extendet class
			 * @param folder in which folder the routine running
			 * @param subroutine name of the routine
			 */
			contactPin(string type, string folderName, string subroutineName)
			: portBase(type, folderName, subroutineName) { };
			virtual bool init(ConfigPropertyCasher &properties);
			virtual bool measure();
			virtual void setValue(const double value);
			virtual double getValue();
			virtual ~contactPin();
	};
}

#endif /*CONTACTPIN_H_*/
