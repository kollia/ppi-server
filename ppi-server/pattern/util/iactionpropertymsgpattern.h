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
#ifndef IACTIONPROPERTYMSGPATTERN_H_
#define IACTIONPROPERTYMSGPATTERN_H_

#include <string>

#include "iactionpropertypattern.h"

using namespace std;

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for properties with one tag for options on parent interface
	 * and message issue (ERROR/WARNING) for defined categorys
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IActionPropertyMsgPattern : virtual public IActionPropertyPattern
	{
	public:
		/**
		 * question to have given action
		 *
		 * @param action name of action
		 * @return true if action be set
		 */
		virtual bool haveAction(const string& action) const= 0;
		/**
		 * whether found an new modifier defined on constructor
		 *
		 * @return string of modifier name, elsewhere an null string ("")
		 */
		virtual param_t newSubroutine() const= 0;
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
		virtual bool notAllowedAction(const string& defined, const bool set= false)= 0;
		//virtual string getMsgHead(const bool error) const =0;
		/**
		 * virtual destructor of pattern
		 */
		virtual ~IActionPropertyMsgPattern() {};
	};
}  // namespace pattern-world

#endif /*IACTIONPROPERTYMSGPATTERN_H_*/
