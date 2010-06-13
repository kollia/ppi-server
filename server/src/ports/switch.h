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
#ifndef SWITCHCLASS_H_
#define SWITCHCLASS_H_

#include "../util/structures.h"

#include "portbaseclass.h"

class switchClass : public portBase
{
public:
		/**
		 * create object of class switchClass.
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		switchClass(string folderName, string subroutineName);
		/**
		 * create object of class switchClass.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param defaultValue only for derived classes which are no SWITCH type the first value, otherwise it will taken from database or default 0
		 */
		switchClass(string type, string folderName, string subroutineName);
		virtual bool init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const bool* const defaultValue= NULL);
		/**
		 * this method will be called from any measure thread to set as observer
		 * for starting own folder to get value from foreign folder
		 * if there the value was changing
		 *
		 * @param observer measure thread which containing the own folder
		 */
		virtual void setObserver(IMeasurePattern* observer);
		/**
		 * activate observer inside new subroutine
		 *
		 * @param pStratFolder address of the first folder
		 * @param observer measure thread which containing the own folder
		 * @param folder name of current folder
		 * @param subroutine name of current subroutine
		 * @param cCurrent string of defined values to find other folder and subroutines
		 */
		static void activateObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
														const string& folder, const string& subroutine, const string& cCurrent);
		/**
		 * fill observer into founded folder:subroutine from cCurrent string
		 *
		 * @param pStratFolder address of the first folder
		 * @param observer measure thread which containing the own folder
		 * @param folder name of current folder
		 * @param subroutine name of current subroutine
		 * @param cCurrent string of defined values to find other folder and subroutines
		 */
		static void giveObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
														const string& folder, const string& subroutine, const string& cCurrent);
		/**
		 * measure new value for subroutine
		 *
		 * @return return measured value
		 */
		virtual double measure();
		/**
		 * destructor
		 */
		virtual ~switchClass();

		/**
		 * calculate whether the given string is an true value.<br />
		 * string can be set as 'true' or 'false',<br />
		 * or an subroutine where in this case it calculate null or not,<br />
		 * or an comparison of two values, subroutines or numbers.<br />
		 * This string can be also splited with '|' or '&'
		 *
		 * @param str string of comparison
		 * @param pStratFolder address of the first folder
		 * @param sFolder name of folder in which the subroutine running
		 * @param debug whether the debug mode outgoing from server is set
		 * @param result whether the comparison was true
		 * @return whether the comparison string was correct
		 */
		static bool getResult(string& str, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result);
		/**
		 * calculate whether the given string is an true value.<br />
		 * string can be set as 'true' or 'false',<br />
		 * or an subroutine where in this case it calculate null or not,<br />
		 * or an comparison of two values, subroutines or numbers
		 *
		 * @param from string of comparison
		 * @param pStratFolder address of the first folder
		 * @param sFolder name of folder in which the subroutine running
		 * @param debug whether the debug mode outgoing from server is set
		 * @param result whether the comparison was true
		 * @return whether the comparison string was correct
		 */
		static bool getSubResult(const string &from, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result);
		/**
		 * returns the address of the given foldername
		 *
		 * @param name name of the folder
		 * @param pStratFolder address of the first folder
		 * @return address of the folder in given parameter name
		 */
		static const SHAREDPTR::shared_ptr<measurefolder_t>  getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * write the value from the given folder:subroutine or comparison as string
		 * into the parameter dResult
		 *
		 * @param pStartFolder first pointer to all defined folders
		 * @param cCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string all found
		 */
		static bool calculateResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &cCurrent, double &dResult);
		/**
		 * write the current value from the given folder:subroutine
		 * into the parameter dResult
		 *
		 * @param pStartFolder first pointer to all defined folders
		 * @param cCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string was found
		 */
		static bool searchResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &pcCurrent, double &dResult);
		static bool subroutineResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pfolder, const string &cCurrent, double &dResult);

	protected:
		/**
		 * whether 0 and 1 is holding inside of object (by true)
		 * and the object is from an derived class.
		 * Otherwise the routine is only for switching between 0 and 1 (false)
		 * and the value is holding in original parent class <code>portBase</code>
		 */
		bool m_bUseInner;
		/**
		 * inner value if object isn't only for switching
		 */
		bool m_bInner;
		/**
		 * value from last pass
		 */
		bool m_bLastValue;
		SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;
		SHAREDPTR::shared_ptr<measurefolder_t> m_pOwnFolder;
		string m_sOn;
		string m_sWhile;
		string m_sOff;

		/**
		 * calculate whether the given string is an true value.<br />
		 * string can be set as 'true' or 'false',<br />
		 * or an subroutine where in this case it calculate null or not,<br />
		 * or an comparison of two values, subroutines or numbers
		 * This string can be also splited with '|' or '&'
		 *
		 * @param from string of comparison
		 * @param result whether the comparison was true
		 * @return whether the comparison string was correct
		 */
		bool getResult(const string &from, bool& result);
		/**
		 * write the value from the given folder:subroutine or comparison as string
		 * into the parameter dResult
		 *
		 * @param cCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string all found
		 */
		bool calculateResult(const string &cCurrent, double &dResult);
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
		//bool getSubResult(char* str);
		/**
		 * write the current value from the given folder:subroutine
		 * into the parameter dResult
		 *
		 * @param pcCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string was found
		 */
		bool searchResult(const char* pcCurrent, double &dResult);
};

#endif /*SWITCHCLASS_H_*/
