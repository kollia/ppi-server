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
#ifndef IMEASUREPATTERN_H_
#define IMEASUREPATTERN_H_

//#include <unistd.h>

#include <string>

using namespace std;


namespace design_pattern_world
{
	namespace util_pattern
	{
		/**
		 * pattern class for MeasureThread to activate new pass by changed value
		 *
		 * @autor Alexander Kolli
		 * @version 1.0.0
		 */
		class IMeasurePattern
		{
			public:
				/**
				 * activate new pass of hole folder
				 *
				 * @param folder name of folder
				 */
				virtual void changedValue(const string& folder)= 0;
				/**
				 * dummy destructor for pattern
				 */
				virtual ~IMeasurePattern() {};

		};
	}
}

#endif /*IMEASUREPATTERN_H_*/