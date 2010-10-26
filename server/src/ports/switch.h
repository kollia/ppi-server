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

#include "../util/properties/configpropertycasher.h"

#include "portbaseclass.h"
#include "ListCalculator.h"

class switchClass : public portBase
{
public:
		/**
		 * enum whether value be set
		 */
		enum setting
		{
			NONE= 0,
			BEGIN,
			WHILE,
			END
		};
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
		/**
		 * initialing object of switchClass
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
		 * find object of port which should be inform
		 *
		 * @param pStratFolder address of the first folder
		 * @param folder name of current folder
		 * @param subroutine name of current subroutine
		 * @param cCurrent string of subroutines with folder
		 * @param own whether need own foldeer as result
		 * @param addinfo info string to add when any error occurs
		 * @return base object of ports
		 */
		static portBase* getPort(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& folder,
										const string& subroutine, const string& cCurrent, const bool own, const string& addinfo);
		/**
		 * measure whether switch value is set or not
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual double measure(const double actValue);
		/**
		 * measure also whether switch value is set or not,
		 * but show in second parameter, maybe for decided object need,
		 * whether value set for BEGIN, WHILE or END
		 *
		 * @param actValue current value
		 * @param set whether value set begins <code>switchClass::BEGIN</code>, during <code>switchClass::WHILE</code>, ending <code>switchClass::END</code> or not set <code>switchClass::NONE</code>
		 * @return return measured value
		 */
		double measure(const double actValue, setting& set);
		/**
		 * destructor
		 */
		virtual ~switchClass();
		/**
		 * returns the address of the given folder name
		 *
		 * @param name name of the folder
		 * @param pStratFolder address of the first folder
		 * @return address of the folder in given parameter name
		 */
		static const SHAREDPTR::shared_ptr<measurefolder_t>  getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug);

	protected:
		/**
		 * value from last pass
		 */
		bool m_bLastValue;
		/**
		 * begin calculation string
		 */
		ListCalculator m_oBegin;
		/**
		 * while calculation string
		 */
		ListCalculator m_oWhile;
		/**
		 * end calculation string
		 */
		ListCalculator m_oEnd;

		/**
		 * write the value from the given folder:subroutine or comparison as string
		 * into the parameter dResult
		 *
		 * @param cCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string all found
		 */
//		bool calculateResult(const string &cCurrent, double &dResult);
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
//		bool getResult(const string &from, bool& result);
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
//		static bool getResult(string& str, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result);
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
//		static bool getSubResult(const string &from, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result);
		/**
		 * write the current value from the given folder:subroutine
		 * into the parameter dResult
		 *
		 * @param pcCurrent string of subroutine or folder:subroutine
		 * @param dResult outcomming double result of the subroutine
		 * @return whether the subroutines in the character string was found
		 */
//		bool searchResult(const char* pcCurrent, double &dResult);
};

#endif /*SWITCHCLASS_H_*/
