/**
 *   This file 'IExternClientInput.h' is part of ppi-server.
 *   Created on: 02.11.2013
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

#ifndef IEXTERNCLIENTINPUT_H_
#define IEXTERNCLIENTINPUT_H_

#include <string>

#include "IClientSendMethods.h"

using namespace std;

namespace design_pattern_world
{
	namespace client_pattern
	{
		class IExternClientInput : public IClientSendMethods
		{
		public:

			virtual vector<string> sendMethodD(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true)= 0;
			/**
			 * dummy destructor
			 */
			virtual ~IExternClientInput()
			{};
		};

	} /* namespace client_pattern */
} /* namespace design_pattern_world */
#endif /* IEXTERNCLIENTINPUT_H_ */
