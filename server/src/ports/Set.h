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
#ifndef SET_H_
#define SET_H_

#include <iostream>

#include "switch.h"

#include "../util/smart_ptr.h"
#include "../util/structures.h"

namespace ports
{
	class Set : public switchClass
	{
	private:
		/**
		 * reference to all folder
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;
		/**
		 * calculating string which value should be set
		 */
		string m_sFrom;
		/**
		 * folder:subroutine to be set with value
		 */
		string m_sSet;
		/**
		 * minimal value of holdet value
		 */
		double m_nMin;
		/**
		 * maximal value of holdet value
		 */
		double m_nMax;
		/**
		 * whether value can be an float
		 */
		bool m_bFloat;
		/**
		 * whether parameter begin/while/end for set new value will be done before<br />
		 * 0 is false and 1 is true
		 */
		double m_dSwitch;
	public:
		/**
		 * create object of class Set
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		Set(const string& folderName, const string& subroutineName)
		: switchClass("SET", folderName, subroutineName),
		  m_dSwitch(0)
		{ };
		/**
		 * create object of class ValueHolder.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		Set(const string& type, const string& folderName, const string& subroutineName)
		: switchClass(type, folderName, subroutineName),
		  m_dSwitch(0)
		{ };
		/**
		 * initialing object of ValueHolder
		 *
		 * @param properties the properties in file measure.conf
		 * @param pStartFolder reference to all folder
		 * @return whether initalization was ok
		 */
		virtual bool init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual double measure(const double actValue);

	protected:
		/**
		 * set min and max parameter to the range which can be set for this subroutine.<br />
		 * If the subroutine is set from 0 to 1 and no float, the set method sending only 0 and 1 to the database.
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
	};
}

#endif /*SET_H_*/
