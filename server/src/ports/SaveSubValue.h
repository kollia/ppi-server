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
#ifndef SAVESUBVALUE_H_
#define SAVESUBVALUE_H_

#include <string>

#include "../util/properties/configpropertycasher.h"

#include "ListCalculator.h"
#include "switch.h"

namespace ports
{

	/**
	 * class representing to save any subroutines
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class SaveSubValue : public switchClass
	{
	public:
		/**
		 * instanciate class of saving
		 *
		 * @param folderName name of folder in whitch procedures are running
		 * @param subroutineName name of subroutine inside the folder
		 */
		SaveSubValue(string folderName, string subroutineName)
		: switchClass(folderName, subroutineName),
		  m_dSwitch(0)
		{ };
		/**
		 * initial class of saving
		 *
		 * @param properties property casher for this subroutine with type SAVE
		 * @param pStartFolder first pointer to all defined folders
		 */
		bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual valueHolder_t measure(const double actValue);
		/**
		 * destructor to delete all ListContainer for saving
		 */
		virtual ~SaveSubValue();

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
		 * identification string for database entry
		 */
		string m_sIdentif;
		/**
		 * subroutine which value should be saved
		 */
		vector<ListCalculator*> m_vpSave;
		/**
		 * whether parameter begin/while/end for save sub value will be done before<br />
		 * 0 is false and 1 is true
		 */
		double m_dSwitch;
	};

}

#endif /*SAVESUBVALUE_H_*/
