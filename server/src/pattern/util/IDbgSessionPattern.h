/**
 *   This file 'IDbgSessionPattern.h' is part of ppi-server.
 *   Created on: 05.01.2015
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

#ifndef IDBGSESSIONPATTERN_H_
#define IDBGSESSIONPATTERN_H_

#include <string>

#include "../../util/smart_ptr.h"

#include "IPPIValuesPattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{
		using namespace std;

		class IDbgSessionPattern
		{
		public:
			/**
			 * structure of data fields
			 * writing by folder:subroutine
			 * for debug session
			 */
			struct dbgSubroutineContent_t
			{
				/**
				 * name of folder
				 */
				string folder;
				/**
				 * name of subroutine
				 */
				string subroutine;
				/**
				 * current value
				 * by debugging subroutine
				 */
				ppi_value value;
				/**
				 * current time
				 * where debugging proceed
				 */
				SHAREDPTR::shared_ptr<IPPITimePattern> currentTime;
				/**
				 * debug output string
				 * written inside subroutine
				 * only by debug session
				 */
				string content;
			};
		};

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* IDBGSESSIONPATTERN_H_ */
