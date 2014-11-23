/**
 *   This file 'IMeasureSet.h' is part of ppi-server.
 *   Created on: 29.12.2012
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


#ifndef IMEASURESET_H_
#define IMEASURESET_H_

#include <string>

#include "IPPIValuesPattern.h"

using namespace std;

class IMeasureSet
{
public:
	/**
	 * set double value into measure list
	 *
	 * @param folder folder name from the running thread
	 * @param subroutine name of the subroutine in the folder
	 * @param value value which should write into database with last changing time when set, otherwise method create own time
	 * @param place from which place value comes
	 * @param account from which account over Internet the value will be set
	 * @return whether subroutine can be set correctly
	 */
	virtual bool setValue(const string& folder, const string& subroutine,
					const IValueHolderPattern& value, const InformObject& account)= 0;
	/**
	 * dummy destructor for pattern
	 */
	virtual ~IMeasureSet() {};

};


#endif /* IMEASURESET_H_ */
