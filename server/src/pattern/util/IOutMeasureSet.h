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
	 * returning ostringstream object which should written on right time
	 * by next pass into Terminal for output on command line
	 *
	 * @return string stream for writing by next pass
	 */
	virtual ostringstream& out()= 0;
	/**
	 * fill debug session for sending to client
	 *
	 * @param folder on which folder execute
	 * @param subroutine on which subroutine execute
	 * @param value by which value execute
	 * @param content which content execute
	 */
	virtual void fillDebugSession(const string& folder, const string& subroutine,
					const ppi_value& value, const string& content)= 0;
	/**
	 * dummy destructor for pattern
	 */
	virtual ~IOutMeasureSet() {};
};

#define LOCKMOBJECT() lockMObject(__FILE__, __LINE__)
#define UNLOCKMOBJECT(locked) unlockMObject(locked, __FILE__, __LINE__)

#endif /* IOUTMEASURESET_H_ */
