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

#ifndef IMETHODSTRINGSTREAMPATTERN_H_
#define IMETHODSTRINGSTREAMPATTERN_H_

//#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace design_pattern_world
{

	class IMethodStringStreamPattern
	{
	public:
		/**
		 * output of hole method string with parameters
		 *
		 * @return hole string
		 */
		virtual string getMethodName() const= 0;
		/**
		 * create intern synchronization ID for object
		 *
		 * @param syncID synchronization ID to implement, elsewhere when not set or 0 create one
		 * @return whether new synchronization ID be created or otherwise take before defined
		 */
		virtual bool createSyncID(unsigned long long syncID= 0)= 0;
		/**
		 * return created synchronize ID when exist,
		 * elsewhere 0
		 *
		 * @return synchronize ID
		 */
		virtual unsigned long long getSyncID() const= 0;
		/**
		 * remove synchronization ID for object which is intern set
		 */
		virtual void removeSyncID()= 0;
		/**
		 * return current hole defined string
		 *
		 * @param withSync output string with syncID when exist
		 * @param withMethod whether string should returned with method name
		 * @return defined string
		 */
		virtual string str(bool withSync= false, bool withMethod= true) const= 0;
		/**
		 * dummy destructor
		 */
		virtual ~IMethodStringStreamPattern()
		{};
	};

}

#endif /* IMETHODSTRINGSTREAMPATTERN_H_ */
