/**
 *   This file 'Value.h' is part of ppi-server.
 *   Created on: 28.01.2011
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

#ifndef VALUE_H_
#define VALUE_H_

#include "Subroutine.h"

namespace subroutines
{

	class Value : public Subroutine
	{
	public:
		Value(ofstream& out, const string& name);
		using Subroutine::pmin;
		using Subroutine::pmax;
		using Subroutine::pdefault;
		using Subroutine::pwhile;
		using Subroutine::pvalue;
		using Subroutine::plwhile;
		using Subroutine::plink;
		using Subroutine::action;
	};

}

#endif /* VALUE_H_ */
