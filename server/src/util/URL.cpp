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
 * URL.cpp
 *
 *  Created on: 22.05.2009
 *      Author: Alexander Kolli
 */

#include "URL.h"

namespace util {

	string URL::addPath(string first, string second, bool always/*= true*/)
	{
		string sRv;

		if(always)
		{
			int fLen= first.length();

			if(	first[fLen-1] != '/'
				&&
				second[0] != '/'				)
			{
				first+= "/";
			}else if(	first[fLen-1] == '/'
						&&
						second[0] == '/'				)
			{
				first= first.substr(0, fLen - 1);
			}
			sRv= first + second;
		}else
		{
			if(second.substr(0, 1) != "/")
			{
				if(first.substr(first.length()-1) != "/")
					first+= "/";
				sRv= first + second;
			}else
				sRv= second;
		}
		return sRv;
	}
}