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

#include "unistd.h"

#include <string>
#include <map>

using namespace std;

namespace util
{

	class URL
	{
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
			static string addPath(const string& first, string second, bool always= true);
			/**
			 * read directory in defined path with an filter for begin and end of the files
			 *
			 * @param path subdirectory wich have to read
			 * @param beginfilter char string of beginning
			 * @param endfilter char atring of ending
			 * @return map of all files as value and the date as key
			 */
			static map<string, string> readDirectory(const string& path, const string& beginfilter, const string& endfilter);
			/**
			 * search user id in /etc/passwd
			 *
			 * @param user name of user
			 * @return user id if found, elsewhere -1
			 */
			//static uid_t getUserID(const string& user);
			/**
			 * extract path from given path with file
			 *
			 * @param filestring file with path
			 * @return only path
			 */
			static string getPath(const string& filestring);
	};

}

#endif /* URL_H_ */
