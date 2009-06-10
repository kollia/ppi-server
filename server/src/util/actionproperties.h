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

/*
 * actionproperties.h
 *
 *  Created on: 21.03.2009
 *      Author: Alexander Kolli
 */

#ifndef ACTIONPROPERTIES_H_
#define ACTIONPROPERTIES_H_

#include <string>
#include <vector>
#include <map>

#include <boost/algorithm/string/trim.hpp>

#include "properties.h"

#include "../pattern/util/iactionpropertypattern.h"

namespace util {

	class ActionProperties :	virtual public Properties,
								virtual public design_pattern_world::IActionPropertyPattern
	{
		public:
			/**
			 * initialization of properties
			 *
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			ActionProperties(const bool byCheck= false)
			:	Properties(byCheck) {};
			/**
			 * read line and save into variables
			 *
			 * @param character line
			 * @return whether line was an correct parameter with value
			 */
			bool readLine(const string& line);
			/**
			 * define actions
			 *
			 * @param spez name of action
			 */
			virtual void action(const string& spez);
			/**
			 * ask whether spezific name is an action paramter
			 *
			 * @param spez whether name is an action
			 */
			virtual bool isAction(const string& spez) const;
			/**
			 * question to have defined an string for action in configuration file
			 *
			 * @param action name of action
			 * @param defined whether string is defined in action
			 * @return true if action be set
			 */
			virtual bool haveAction(const string& action, const string& defined) const;
			/**
			 * to set in casher witch action be allowed
			 *
			 * @param action for witch action
			 * @param defined defined action witch is allowed
			 * @return whether defined is allowed for this action
			 */
			virtual bool allowedAction(const string& action, const string& defined) const;
			/**
			 * if this method be set, the parameter is not allowed.<br />
			 * When the parameter will be fetched in an base class,
			 * it get's the second parameter as default value
			 *
			 * @param action name of action
			 * @param defined the definition in action which should not be allowed
			 * @param set this parameter can be set if an base class ask for this action to see true or false (default false)
			 *
			 * @return whether the action be set (not the defined action)
			 */
			virtual bool notAllowedAction(const string& action, const string& defined, const bool set= false);
			/**
			 * method write WARNINGS on command line if any action not necessary
			 * and if properties be set, but not needed
			 *
			 * @param output method fill this string if set with WARNINGS of parameter and actions which are set but not allowed.<br />
			 * 				 Elsewhere if string not be set (NULL), WARNINGS will be writing on command line
			 * @param head whether should writing WARNING with message parameter defined in setMsgParameter() (default true)
			 */
			virtual void checkProperties(string* output= NULL, const bool head= true) const;
			/**
			 * destructor of object
			 */
			virtual ~ActionProperties();
		protected:
			/**
			 * hold all allowed actions
			 */
			mutable map<string, vector<string> > m_mvAllowed;
			/**
			 * hold all actions which are set
			 */
			map<string, vector<string> > m_mvActions;
			/**
			 * holder of all not allowed actions
			 */
			map<string, map<string, bool> > m_mmNotAllowed;
	};

}

#endif /* ACTIONPROPERTIES_H_ */