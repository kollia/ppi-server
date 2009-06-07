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
 * URL.h
 *
 *  Created on: 22.05.2009
 *      Author: Alexander Kolli
 */

#ifndef URL_H_
#define URL_H_

#include <string>

using namespace std;

namespace util {

	class URL {
	public:
		/**
		 * add first with second path,
		 * only if the second not beginning
		 * with an slash
		 *
		 * @param first first beginning path (working directory)
		 * @param second second path or file
		 * @param file boolean whether second is an file.<br />
		 * 				If parameter is true (DEFAULT),
		 * 				the first path is only add when the second do not beginning
		 * 				with an slash
		 */
		static string addPath(string first, string second, bool always= true);
	};

}

#endif /* URL_H_ */
