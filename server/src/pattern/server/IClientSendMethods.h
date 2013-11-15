/**
 *   This file 'IClientSendMethods.h' is part of ppi-server.
 *   Created on: 12.11.2013
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

#ifndef ICLIENTSENDMETHODS_H_
#define ICLIENTSENDMETHODS_H_

#include <string>
#include <vector>

#include "../../util/stream/OMethodStringStream.h"

using namespace std;
using namespace util;

namespace design_pattern_world
{
	namespace client_pattern
	{

		class IClientSendMethods
		{
		public:
			/**
			 * send message to given server in constructor
			 * or write into queue when no answer be needed
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param answer whether client should wait for answer
			 * @return backward send return value from server if answer is true, elsewhere returning null string
			 */
			virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true)= 0;
			/**
			 * send message to given server in constructor
			 * or write into queue when no answer be needed
			 *
			 * @param toProcess for which process the method should be
			 * @param method object of method which is sending to server
			 * @param done on which getting string the answer should ending. Ending also when an ERROR or warning occurs
			 * @param answer whether client should wait for answer
			 * @return backward send return string vector from server if answer is true, elsewhere returning vector with no size
			 */
			virtual vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true)= 0;
			/**
			 * dummy destructor
			 */
			virtual ~IClientSendMethods()
			{};
		};

	} /* namespace client_pattern */
} /* namespace design_pattern_world */
#endif /* ICLIENTSENDMETHODS_H_ */
