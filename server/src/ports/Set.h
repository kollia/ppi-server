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
		 * array of calculating string's which values should be set
		 */
		vector<ListCalculator*> m_vpoFrom;
		/**
		 * array of folder:subroutine strings to be set with value
		 */
		vector<string> m_vsSet;
		/**
		 * whether should set changing time
		 * from 'from' property (true)
		 * into foreign subroutine
		 * or from begin/while/end properties (false)
		 */
		bool m_bFromTime;

	public:
		/**
		 * create object of class Set
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		Set(const string& folderName, const string& subroutineName, unsigned short objectID)
		: switchClass("SET", folderName, subroutineName, objectID)
		{ };
		/**
		 * create object of class ValueHolder.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		Set(const string& type, const string& folderName, const string& subroutineName, unsigned short objectID)
		: switchClass(type, folderName, subroutineName, objectID)
		{ };
		/**
		 * initialing object of ValueHolder
		 *
		 * @param properties the properties in file measure.conf
		 * @param pStartFolder reference to all folder
		 * @return whether initalization was ok
		 */
		virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
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
		/**
		 * destructor to delete all from parameter
		 */
		virtual ~Set();

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
		/**
		 * whether subroutine can be new defined from given other subroutine.<br />
		 * when own subroutine has to set value in this other subroutine
		 * which is also from class Set
		 * and the other subroutine want to set in the same time
		 * an value in the own subroutine,
		 * both objects are locked and it cause an dead-lock.<br />
		 * Except both subroutines are in the same folder.<br />
		 * But WARNING: there is no check whether incoming subroutine
		 * is from class Set
		 *
		 * @param folder name of folder from other subroutine
		 * @param subroutine name of subroutine from other
		 * @return whether possible to set value
		 */
		bool possibleSet(const string& folder, const string& subroutine) const;
	};
}

#endif /*SET_H_*/
