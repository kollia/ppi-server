/**
 *   This file 'IHearingThreadPattern.h' is part of ppi-server.
 *   Created on: 27.12.2014
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

#ifndef IHEARINGTHREADPATTERN_H_
#define IHEARINGTHREADPATTERN_H_

#include "../../util/thread/Thread.h"

#include "IClientTransactionPattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{

		class IHearingThreadPattern : public Thread
		{
		public:
			/**
			 * constructor for thread
			 */
			IHearingThreadPattern(const string& threadName)
			: Thread(threadName)
			{};
			/**
			 * returning object of transaction
			 * which set for hearing thread
			 *
			 * @return transaction object
			 */
			virtual IClientTransactionPattern* transObj()= 0;
		};

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* IHEARINGTHREADPATTERN_H_ */
