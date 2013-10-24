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

#ifndef IPARAMETERSTRINGSTREAM_H_
#define IPARAMETERSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace util {

class IParameterStringStream {
public:
	/**
	 * constructor to fill stream
	 */
	IParameterStringStream(const string& stream);
	/**
	 * copy constructor for own object
	 *
	 * @param obj other object to copy inside
	 */
	IParameterStringStream& operator = (const IParameterStringStream& obj);
	/**
	 * operator for bool
	 *
	 * @param value bool variable
	 * @return own object
	 */
	void operator >> ( bool& value);
	/**
	 * operator for short
	 *
	 * @param value short variable
	 * @return own object
	 */
	void operator >> ( short& value);
	/**
	 * operator for unsigned short
	 *
	 * @param value unsigned short variable
	 * @return own object
	 */
	void operator >> ( unsigned short& value);
	/**
	 * operator for integer
	 *
	 * @param value integer variable
	 * @return own object
	 */
	void operator >> ( int& value);
	/**
	 * operator for unsignet integer
	 *
	 * @param value unsigned integer variable
	 * @return own object
	 */
	void operator >> ( unsigned int& value);
	/**
	 * operator for long
	 *
	 * @param value long variable
	 * @return own object
	 */
	void operator >> ( long& value);
	/**
	 * operator for unsigned long
	 *
	 * @param value unsigned long variable
	 * @return own object
	 */
	void operator >> ( unsigned long& value);
	/**
	 * operator for float
	 *
	 * @param value float variable
	 * @return own object
	 */
	void operator >> ( float& value);
	/**
	 * operator for double
	 *
	 * @param value double variable
	 * @return own object
	 */
	void operator >> ( double& value);
	/**
	 * operator for string
	 *
	 * @param value string variable
	 * @return own object
	 */
	void operator >> (string& value);
	/**
	 * get string from stream, like operaor >> (string)
	 *
	 * @param value string variable
	 */
	void getString(string& value);
	/**
	 * return current hole defined string
	 *
	 * @return defined string
	 */
	virtual string str() const
	{ return m_sStream.str(); };
	/**
	 * whether the parameter stream is empty or reach end of string
	 *
	 * @return true if the stream is empty
	 */
	bool empty();
	/**
	 * whether the pulling variable was from the wrong type.<br />
	 * The seek position was set back, so try the right one.
	 * This method return also true if the object was empty.
	 *
	 * @return true if the call was wrong
	 */
	bool fail()
	{ return m_bFail; };
	/**
	 * whether pulled variable was an null pointer and does not exist
	 *
	 * @return true if the variable was null
	 */
	bool null()
	{ return m_bNull; };
	/**
	 * virtual destructor of object
	 */
	virtual ~IParameterStringStream() {};

protected:
	/**
	 * created string stream
	 */
	stringstream m_sStream;

private:
	/**
	 * whether the pulling variable was from the wrong type.<br />
	 * The seek position was set back, so try the right one
	 */
	bool m_bFail;
	/**
	 * whether pulled variable was NULL
	 */
	bool m_bNull;

};

}

#endif /* IPARAMETERSTRINGSTREAM_H_ */
