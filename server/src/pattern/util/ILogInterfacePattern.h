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

#ifndef ILOGINTERFACEPATTERN_H_
#define ILOGINTERFACEPATTERN_H_


#include <string>

#include "IErrorHandlingPattern.h"

using namespace std;

namespace design_pattern_world
{
	namespace util_pattern
	{
	/**
	 * pattern class for an logging object
	 *
	 * @autor Alexander Kolli
	 * @version 1.0.0
	 */
		class ILogInterfacePattern
		{
		public:
			/**
			 * write vector log and threadNames which was read
			 * before get connection
			 *
			 * @return whether write any message over connection
			 */
			virtual bool writeVectors()= 0;
			/**
			 * open the connection to server for sending questions
			 *
			 * @param toopen string for open question, otherwise by null the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
			 * @return object of error handling
			 */
			virtual EHObj openConnection(string toopen= "")= 0;
			/**
			 * this method will be called when ConnectionChecker ending
			 */
			virtual void connectionCheckerStops()= 0;
			/**
			 * dummy destructor of design pattern
			 */
			virtual ~ILogInterfacePattern() {};
		};
	}
}

#endif /* ILOGINTERFACEPATTERN_H_ */
