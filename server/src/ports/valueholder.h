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
#ifndef OTHERFOLDER_H_
#define OTHERFOLDER_H_

#include <iostream>

#include "portbaseclass.h"

#include "../util/smart_ptr.h"
#include "../util/structures.h"

namespace ports
{
	class ValueHolder : public portBase
	{
	private:
		/**
		 * @param min minimal value of holdet value
		 */
		double m_nMin;
		/**
		 * @param max maximal value of holdet value
		 */
		double m_nMax;
		/**
		 * whether value can be an float
		 */
		bool m_bFloat;
		/**
		 * all values which can be set as content
		 */
		vector<string> m_vdValues;
		/**
		 * while expression to set values from m_vdValues
		 */
		string m_sWhile;
		/**
		 * default value for beginning and when calculated while expression
		 * higher or lower then value count of m_vdValues
		 */
		double m_ddefaultValue;
		/**
		 * reference to all folder
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;

	public:
		/**
		 * create object of class ValueHolder
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		ValueHolder(string folderName, string subroutineName)
		: portBase("VALUE", folderName, subroutineName)
		{ };
		/**
		 * create object of class ValueHolder.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		ValueHolder(string type, string folderName, string subroutineName)
		: portBase(type, folderName, subroutineName)
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
		 * this method will be called from any measure thread to set as observer
		 * for starting own folder to get value from foreign folder
		 * if there the value was changing
		 *
		 * @param observer measure thread which containing the own folder
		 */
		virtual void setObserver(IMeasurePattern* observer);
		/**
		 * method measure do nothing
		 *
		 * @return always true;
		 */
		virtual bool measure();
		/**
		 * calculate while string and set to value result or content of parameter content if exist.<br/>
		 * Method write error or warning string into log-file and on command line if debug flag be set
		 *
		 * @param pStartFolder reference of first Folder
		 * @param folder name of actual folder
		 * @param subroutine name of actual subroutine
		 * @param whileStr defined while string in subroutine
		 * @param content vector of defined-values to replace with number of while string
		 * @param defaultVal default value when vector of content be set but number of calculated while string ist out of range
		 * @param value result of method
		 * @param debug whether should write debug messages on command line
		 * @return true if value will be calculated
		 */
		static bool getWhileStringResult(const SHAREDPTR::shared_ptr<measurefolder_t> pStartFolder, const string& folder, const string& subroutine, const string& whileStr, const vector<string>& content, const double defaultVal, double& value, const bool debug);

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

#endif /*OTHERFOLDER_H_*/
