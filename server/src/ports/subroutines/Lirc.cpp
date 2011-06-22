/**
 *   This file 'Lirc.cpp' is part of ppi-server.
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

#include <iostream>
#include <sstream>

#include "Lirc.h"


namespace subroutines
{

	Lirc::Lirc(ofstream& out, const string& name)
	:	Subroutine(out, name, "LIRC")
	{
	}

	void Lirc::premote(const string& content, const string& desc)
	{
		writeParam("ID", content, desc);
	}

	void Lirc::pcode(const string& content, const string& desc)
	{
		writeParam("pin", content, desc);
	}

	void Lirc::pcount(const double& content, const string& desc)
	{
		ostringstream ocont;

		ocont << content;
		pcount(ocont.str(), desc);
	}

	void Lirc::pcount(const string& content, const string& desc)
	{
		writeParam("count", content, desc);
	}

	void Lirc::ppriority(const string& content, const string& desc)
	{
		writeParam("priority", content, desc);
	}

}
