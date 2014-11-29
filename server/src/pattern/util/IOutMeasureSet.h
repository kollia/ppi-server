/**
 *   This file 'IOutMeasureSet.h' is part of ppi-server.
 *   Created on: 07.10.2014
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

#ifndef IOUTMEASURESET_H_
#define IOUTMEASURESET_H_

#include "IMeasureSet.h"

class IOutMeasureSet : public IMeasureSet
{
public:
	/**
	 * lock object inside working list
	 * to make value from begin running consistent
	 * to end running.<br />
	 * this method is same like method <code>lockObject()</code>
	 * inside <code>IListObjectPattern</code>
	 */
	virtual void lockMObject() const= 0;
	/**
	 * unlock object inside working list.<br />
	 * this method is same like method <code>unlockObject()</code>
	 * inside <code>IListObjectPattern</code>
	 */
	virtual void unlockMObject() const= 0;
	/**
	 * returning ostringstream object which should written on right time
	 * by next pass into Terminal for output on command line
	 *
	 * @return string stream for writing by next pass
	 */
	virtual ostringstream& out()= 0;
	/**
	 * dummy destructor for pattern
	 */
	virtual ~IOutMeasureSet() {};
};

#endif /* IOUTMEASURESET_H_ */
