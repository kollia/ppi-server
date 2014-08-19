/**
 *   This file 'IInformerCachePattern.h' is part of ppi-server.
 *   Created on: 13.08.2014
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

#ifndef IINFORMERCACHEPATTERN_H_
#define IINFORMERCACHEPATTERN_H_

#include "IPPIValuesPattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{
		class IInformerCachePattern
		{
		public:
			/**
			 * information by changed value in any subroutine
			 *
			 * @param folder which folder should be informed
			 * @param from from which folder comes information
			 */
			virtual void changedValue(const string& folder, const string& from)= 0;
			/**
			 * dummy virtual destructor
			 */
			virtual ~IInformerCachePattern()
			{};
		};
	} /* namespace util_pattern */

} /* namespace design_pattern_world */

#endif /* IINFORMERCACHEPATTERN_H_ */
