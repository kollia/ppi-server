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
#ifndef ISERVERPATTERN_H_
#define ISERVERPATTERN_H_

#include <iostream>
#include <sys/io.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "../util/ithreadpattern.h"

#include "IClientHolderPattern.h"

namespace design_pattern_world
{
	using namespace util_pattern;

	namespace server_pattern
	{
		class IServerPattern : virtual public IThreadPattern
		{
			public:
				/**
				 * return name of server
				 *
				 * @return name
				 */
				virtual string getName() const= 0;
				/**
				 * return factory of ServerCommunicationStarter
				 *
				 * @return actual ServerCommunicationStarter
				 */
				virtual IClientHolderPattern* getCommunicationFactory() const= 0;
				/**
				 * allow new connections from any client
				 *
				 * @param allow whether connections are allowed
				 */
				virtual void allowNewConnections(const bool allow)= 0;
				/**
				 * dummy destructor of pattern
				 */
				virtual ~IServerPattern() {};

		};
	}
}

#endif /*ISERVERPATTERN_H_*/
