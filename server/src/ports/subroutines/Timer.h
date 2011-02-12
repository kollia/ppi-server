/**
 *   This file 'Timer.h' is part of ppi-server.
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

#ifndef TIMER_H_
#define TIMER_H_

#include "Subroutine.h"

namespace subroutines
{

	class Timer : public Subroutine
	{
	public:
		Timer(ofstream& out, const string& name);
		using Subroutine::pmtime;
		using Subroutine::pday;
		using Subroutine::phour;
		using Subroutine::pmin;
		using Subroutine::psec;
		using Subroutine::pmillisec;
		using Subroutine::pmicrosec;
		using Subroutine::pbegin;
		using Subroutine::pwhile;
		using Subroutine::pend;
		using Subroutine::pdefault;
		using Subroutine::psetnull;
		using Subroutine::plwhile;
		using Subroutine::plink;
		using Subroutine::action;
	};

}

#endif /* TIMER_H_ */
