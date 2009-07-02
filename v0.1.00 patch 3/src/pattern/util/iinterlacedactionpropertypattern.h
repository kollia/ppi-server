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
 * iinterlacedactionpropertypattern.h
 *
 *  Created on: 22.03.2009
 *      Author: Alexander Kolli
 */

#ifndef IINTERLACEDACTIONPROPERTYPATTERN_H_
#define IINTERLACEDACTIONPROPERTYPATTERN_H_

#include "iinterlacedpropertypattern.h"
#include "iactionpropertypattern.h"

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for properties with one tag for options
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IInterlacedActionPropertyPattern :	virtual public IInterlacedPropertyPattern,
												virtual public IActionPropertyPattern
	{
		public:
			/**
			 * virtual destructor of pattern
			 */
			virtual ~IInterlacedActionPropertyPattern() {};
	};
}  // namespace design_pattern_world

#endif /* IINTERLACEDACTIONPROPERTYPATTERN_H_ */
