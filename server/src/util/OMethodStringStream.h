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

#ifndef OMETHODSTRINGSTREAM_H_
#define OMETHODSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

#include "OParameterStringStream.h"

using namespace std;

namespace util {

class OMethodStringStream : public OParameterStringStream {
public:
	/**
	 * constructor to create OMethodStringStream object
	 */
	explicit OMethodStringStream(const string& method);
	/**
	 * constructor to create OMethodStringStream object from new object.<br>
	 * New object overwrite method name and parameter stream
	 *
	 * @param method name of method in object
	 */
	//OMethodStringStream(const OMethodStringStream& method);
	/**
	 * output of hole method string with parameters
	 *
	 * @return hole string
	 */
	string str() const;
	/**
	 * output of all parameters in object
	 *
	 * @return parameter string
	 */
	string parameters() const;
	/**
	 * virtual destructor of object
	 */
	virtual ~OMethodStringStream();

private:
	/**
	 * method of object
	 */
	string m_sMethod;

};

}

#endif /* OMETHODSTRINGSTREAM_H_ */
