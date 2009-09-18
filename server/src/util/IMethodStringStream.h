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

#ifndef IMETHODSTRINGSTREAM_H_
#define IMETHODSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

#include "IParameterStringStream.h"

using namespace std;

namespace util {

class IMethodStringStream : public IParameterStringStream {
public:
	/**
	 * constructor to create IMethodStringStream object
	 */
	IMethodStringStream(const string& method);
	/**
	 * output of hole method string with parameters
	 *
	 * @return hole string
	 */
	string getMethodName() const
	{ return m_sMethod; };
	/**
	 * virtual destructor of object
	 */
	virtual ~IMethodStringStream() {};

private:
	/**
	 * method of object
	 */
	string m_sMethod;

};

}

#endif /* IMETHODSTRINGSTREAM_H_ */
