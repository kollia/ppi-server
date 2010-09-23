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
#ifndef VALUEHOLDER_H_
#define VALUEHOLDER_H_

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
		 * value from last pass
		 */
		double m_dLastValue;
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
		 * whether an observer for any link be set
		 */
		bool m_bSetLinkObserver;
		/**
		 * whether an observer for any value string be set
		 */
		bool m_bSetValueObserver;
		/**
		 * which link for observer be set
		 */
		vector<string>::size_type m_nLinkObserver;
		/**
		 * which value string for observer be set
		 */
		vector<string>::size_type m_nValueObserver;
		/**
		 * observer handle
		 */
		IMeasurePattern* m_poObserver;
		/**
		 * all values which can be set as content
		 */
		vector<string> m_vdValues;
		/**
		 * all links tho share with other subroutines
		 */
		vector<string> m_vsLinks;
		/**
		 * while expression to set values from m_vdValues
		 */
		string m_sWhile;
		/**
		 * while expession to set link from m_vsLinks
		 */
		string m_sLinkWhile;
		/**
		 * default value for beginning and when calculated while expression
		 * higher or lower then value count of m_vdValues
		 */
		double m_ddefaultValue;
		/**
		 * reference to all folder
		 */
		SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;
		/**
		 * whether while parameter have operator or '|' or and '&' inside of string
		 */
		bool m_bBooleanWhile;

	public:
		/**
		 * create object of class ValueHolder
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		ValueHolder(const string& folderName, const string& subroutineName)
		: portBase("VALUE", folderName, subroutineName),
		  m_dLastValue(0),
		  m_bSetLinkObserver(false),
		  m_bSetValueObserver(false)
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
		: portBase(type, folderName, subroutineName),
		  m_dLastValue(0),
		  m_bSetLinkObserver(false),
		  m_bSetValueObserver(false)
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
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual double measure(const double actValue);
		/**
		 * get value from subroutine
		 *
		 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
		 * 				This time only defined for external reading over OwPort's.
		 * @return current value
		 */
		virtual double getValue(const string& who);
		/**
		 * set value in subroutine
		 *
		 * @param value value which should be set
		 */
		virtual void setValue(const double value);
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
		 * @param readBool whether whileStr have inside an operator of or '|' or and '&'
		 * @param debug whether should write debug messages on command line
		 * @return true if value will be calculated
		 */
		bool getWhileStringResult(const SHAREDPTR::shared_ptr<measurefolder_t> pStartFolder, const string& folder, const string& subroutine, const string& whileStr, const vector<string>& content, const double defaultVal, double& value, const bool readBool, const bool debug);

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

#endif /*VALUEHOLDER_H_*/
