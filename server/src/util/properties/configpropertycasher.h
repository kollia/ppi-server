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
#ifndef CONFIGPROPERTYCASHER_H_
#define CONFIGPROPERTYCASHER_H_

#include <string>
#include <map>
#include <vector>

#include "interlacedactionproperties.h"

#include "../../pattern/util/iactionpropertymsgpattern.h"

using namespace std;
using namespace design_pattern_world;


namespace util
{

	/**
	 * Class representing all propertys under one subroutine name
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class ConfigPropertyCasher : virtual public ActionProperties,
								 virtual public IActionPropertyMsgPattern
	{
	public:
		/**
		 * default constructor of ConfigPropertyCasher
		 */
		ConfigPropertyCasher();
		/**
		 * read line and save into variables
		 *
		 * @param character line
		 * @return whether line was an correct parameter with value
		 */
		bool readLine(const string& line);
		/**
		 * delete characters before the string and behind
		 *
		 * @param str string on whitch should execute
		 * @param chars characters whitch should be extracted
		 * @return extracted string
		 */
		static string trim(string str, const string chars= "");
		/**
		 * split the given string on all characters in the delimiter
		 *
		 * @param str string whitch should split
		 * @param delimiter string with characters on which should split
		 * @return vector of splited strings
		 */
		static vector<string> split(const string str, const string delimiter);
		/**
		 * if this method be set, the parameter is not allowed.<br />
		 * When the parameter will be fetched in an base class,
		 * it get's the second parameter as default value
		 *
		 * @param defined the definition in action which should not be allowed
		 * @param set this parameter can be set if an base class ask for this action to see true or false (default false)
		 *
		 * @return whether the action be set (not the defined action)
		 */
		virtual bool notAllowedAction(const string& defined, const bool set= false);
		/**
		 * whether found an new modifier defined on constructor
		 *
		 * @return string of modifier name, elsewhere an null string ("")
		 */
		virtual Properties::param_t newSubroutine() const;
		/**
		 * if this method be set, the parameter is not allowed.<br />
		 * When the parameter will be fetched in an base class,
		 * it get's the second parameter as default value
		 *
		 * @param property the parameter which is not allowed
		 * @param sDefault the value which gets the method how fetch this parameter
		 * @return true if the parameter was set
		 */
		virtual bool notAllowedParameter(const string& property, const string& sDefault= "");
		/**
		 * destructor of class
		 */
		virtual ~ConfigPropertyCasher() {};

	private:
		string m_sCurrent;
		vector<string> m_vsModifier;
	};

}

#endif /*CONFIGPROPERTYCASHER_H_*/
