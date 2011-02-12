/**
 *   This file 'action.h' is part of ppi-server.
 *   Created on: 26.01.2011
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

#ifndef ACTION_H_
#define ACTION_H_

#include <string>

#include "../../pattern/subroutines/IActionPattern.h"

using namespace std;

namespace subroutine
{

	class action : virtual public IActionPattern
	{
	public:
		/**
		 * constructor of action to create type
		 *
		 * @param type for which type the class is defined
		 */
		action(const string type)
		: m_stype(type) {};
		/**
		 * return type of action
		 *
		 * @return type of action
		 */
		virtual string type() const
		{ return m_stype; };

	private:
		/**
		 * type of action object
		 */
		const string m_stype;
	};

}

#endif /* ACTION_H_ */
