/**
 *   This file 'Switch.h' is part of ppi-server.
 *   Created on: 27.01.2011
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

#ifndef SWITCH_H_
#define SWITCH_H_

#include "Subroutine.h"

namespace subroutines
{

	class Switch : public Subroutine
	{
	public:
		Switch(ofstream& out, const string& name);
		using Subroutine::pbegin;
		using Subroutine::pwhile;
		using Subroutine::pend;
		using Subroutine::plwhile;
		using Subroutine::plink;
		using Subroutine::pdefault;
		using Subroutine::action;
	};

}

#endif /* SWITCH_H_ */
