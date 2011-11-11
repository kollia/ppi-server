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
#ifndef IACTIONPROPERTYPATTERN_H_
#define IACTIONPROPERTYPATTERN_H_

#include <string>

#include "ipropertypattern.h"

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for properties with one tag for options
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IActionPropertyPattern : virtual public IPropertyPattern
	{
		public:
			/**
			 * define actions
			 *
			 * @param spez name of action
			 */
			virtual void action(const string& spez)= 0;
			/**
			 * ask whether spezific name is an action paramter
			 *
			 * @param spez whether name is an action
			 */
			virtual bool isAction(const string& spez) const= 0;
			/**
			 * set the default parameter name for reading actions.<br />
			 * Default set is </code>action</code>
			 *
			 * @param name which name the parameter should have
			 */
			virtual void setDefaultActionName(const string& name)= 0;
			/**
			 * question to have defined an string for default action in configuration file
			 *
			 * @param defined whether string is defined in action
			 * @return true if action be set
			 */
			virtual bool haveAction(const string& defined) const= 0;
			/**
			 * question to have defined an string for action in configuration file
			 *
			 * @param action name of action
			 * @param defined whether string is defined in action
			 * @return true if action be set
			 */
			virtual bool haveAction(const string& action, const string& defined) const= 0;
			/**
			 * set action parameter.<br />
			 * Add action only when allowed.
			 *
			 * @param name action which should be set
			 * @return whether action was allowed to set
			 */
			virtual bool setAction(const string& name)= 0;
			/**
			 * set action parameter.<br />
			 * Add action only when allowed.
			 *
			 * @param action name of action
			 * @param name action which should be set
			 * @return whether action was allowed to set
			 */
			virtual bool setAction(const string& action, const string& name)= 0;
			/**
			 * delete action parameter
			 *
			 * @param name action which should be set
			 */
			virtual void delAction(const string& name)= 0;
			/**
			 * delete action parameter
			 *
			 * @param action name of action
			 * @param name action which should be set
			 */
			virtual void delAction(const string& action, const string& name)= 0;
			/**
			 * if this method be set, the parameter is not allowed.<br />
			 * When the parameter will be fetched in an base class,
			 * it get's the second parameter as default value
			 *
			 * @param defined the definition in action which should not be allowed
			 * @param set this parameter can be set if an base class ask for this action to see true or false
			 * @return whether the action be set (not the defined action)
			 */
			virtual bool notAllowedAction(const string& defined, const bool set= false)= 0;
			/**
			 * if this method be set, the parameter is not allowed.<br />
			 * When the parameter will be fetched in an base class,
			 * it get's the second parameter as default value
			 *
			 * @param action name of action
			 * @param defined the definition in action which should not be allowed
			 * @param set this parameter can be set if an base class ask for this action to see true or false
			 * @return whether the action be set (not the defined action)
			 */
			virtual bool notAllowedAction(const string& action, const string& defined, const bool set= false)= 0;
			/**
			 * virtual destructor of pattern
			 */
			virtual ~IActionPropertyPattern() {};
	};
}  // namespace pattern-world

#endif /*IACTIONPROPERTYPATTERN_H_*/
