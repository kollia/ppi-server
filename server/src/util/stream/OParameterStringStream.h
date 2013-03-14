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

#ifndef OPARAMETERSTRINGSTREAM_H_
#define OPARAMETERSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace util {

class OParameterStringStream {
public:
	/**
	 * output of hole method string with parameters
	 *
	 * @return hole string
	 */
	virtual string str() const;
	/**
	 * operator for an second object.<br />
	 * New object do not overwrite method and append stream from new object
	 *
	 * @param value variable to an OMethodStringStream object
	 * @return own object
	 */
	OParameterStringStream* operator << (const OParameterStringStream& value);
	/**
	 * operator for an second object.<br />
	 * New object do not overwrite method and append stream from new object
	 *
	 * @param value pointer to an OParameterStringStream object
	 * @return own object
	 */
	OParameterStringStream* operator << (const OParameterStringStream* value);
	/**
	 * operator for bool
	 *
	 * @param value bool variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const bool value);
	/**
	 * operator for bool pointer
	 *
	 * @param value pointer to bool variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const bool* value);
	/**
	 * operator for short
	 *
	 * @param value short variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const short value);
	/**
	 * operator for unsigned short
	 *
	 * @param value unsigned short variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned short value);
	/**
	 * operator for short pointer
	 *
	 * @param value pointer to short variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const short* value);
	/**
	 * operator for unsigned short pointer
	 *
	 * @param value pointer to unsigned short variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned short* value);
	/**
	 * operator for integer
	 *
	 * @param value integer variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const int value);
	/**
	 * operator for unsignet integer
	 *
	 * @param value unsigned integer variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned int value);
	/**
	 * operator for integer pointer
	 *
	 * @param value pointer to integer variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const int* value);
	/**
	 * operator for unsignet integer pointer
	 *
	 * @param value pointer to unsigned integer variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned int* value);
	/**
	 * operator for long
	 *
	 * @param value long variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const long value);
	/**
	 * operator for unsigned long
	 *
	 * @param value unsigned long variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned long value);
	/**
	 * operator for long pointer
	 *
	 * @param value pointer to long variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const long* value);
	/**
	 * operator for unsigned long pointer
	 *
	 * @param value pointert to unsigned long variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const unsigned long* value);
	/**
	 * operator for double
	 *
	 * @param value double variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const double value);
	/**
	 * operator for double pointer
	 *
	 * @param value pointer to double variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const double* value);
	/**
	 * operator for string
	 *
	 * @param value string variable
	 * @return own object
	 */
	OParameterStringStream* operator << (const char* const value);
	/**
	 * operator for string
	 *
	 * @param value string variable
	 * @return own object
	 */
	OParameterStringStream* operator << (string value);
	/**
	 * operator for string pointer
	 *
	 * @param value pointer to string variable
	 * @return own object
	 */
	OParameterStringStream* operator << (string* value);
	/**
	 * virtual destructor of object
	 */
	virtual ~OParameterStringStream();

private:
	/**
	 * created string stream
	 */
	ostringstream m_sStream;

};

}

#endif /* OPARAMETERSTRINGSTREAM_H_ */
