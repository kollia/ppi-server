/**
 *   This file 'IPPIValuesPattern.h' is part of ppi-server.
 *   Created on: 02.01.2014
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

#ifndef IPPIVALUESPATTERN_H_
#define IPPIVALUESPATTERN_H_

#include <sys/time.h>

#include <string>

using namespace std;

/**
 * type of handled value inside ppi-server
 */
typedef double ppi_value;

class IPPITimePattern : public timeval
{
public:
	/**
	 * copy operator
	 */
	virtual IPPITimePattern& operator = (const timeval& time)= 0;
	/**
	 * equal operator
	 */
	virtual bool operator == (const timeval& time) const= 0;
	/**
	 * unequal operator
	 */
	virtual bool operator != (const timeval& time) const= 0;
	/**
	 * lower operator
	 */
	virtual bool operator < (const timeval& time) const= 0;
	/**
	 * greater operator
	 */
	virtual bool operator > (const timeval& time) const= 0;
	/**
	 * lower equal operator
	 */
	virtual bool operator <= (const timeval& time) const= 0;
	/**
	 * greater equal operator
	 */
	virtual bool operator >= (const timeval& time) const= 0;
	/**
	 * addition operator
	 */
	virtual timeval operator + (const timeval& time)= 0;
	/**
	 * addition operator to own object
	 */
	virtual IPPITimePattern& operator += (const timeval& time)= 0;
	/**
	 * subtraction operator
	 */
	virtual timeval operator - (const timeval& time)= 0;
	/**
	 * subtraction operator to own object
	 */
	virtual IPPITimePattern& operator -= (const timeval& time)= 0;
	/**
	 * set actually time into own object
	 *
	 * @return true for success, or -1 for failure (in which case errno is set appropriately).
	 */
	virtual bool setActTime()= 0;
	/**
	 * check whether time is set
	 */
	virtual bool isSet() const= 0;
	/**
	 * set own timer object to 0
	 */
	virtual void clear()= 0;
	/**
	 * write IPPITimePattern as date with microseconds or as second value
	 * with also microseconds after decimal point into an string.<br />
	 * When by date writing an error occurs, the date will be written as
	 * xx.xx.xxxx xx:xx:xx with correct microseconds
	 *
	 * @param bDate whether should write as date (true) or seconds (false)
	 * @return string object as date or seconds
	 */

	virtual string toString(const bool& bDate) const= 0;
	/**
	 * dummy destructor
	 */
	virtual ~IPPITimePattern() {};
};

/**
 * template class to define handled value
 * with last changing time
 */
class IValueHolderPattern
{
public:
	/**
	 * setter method for value
	 *
	 * @param value setting value
	 */
	virtual void setValue(const ppi_value& value)= 0;
	/**
	 * setter method for value with time
	 *
	 * @param str setting value
	 */
	virtual void setTimeValue(const IValueHolderPattern& str)= 0;
	/**
	 * getter method for value
	 *
	 * @return defined value
	 */
	virtual ppi_value getValue() const= 0;
	/**
	 * setter method for time
	 *
	 * @param time setting time
	 */
	virtual void setTime(const IPPITimePattern& time)= 0;
	/**
	 * getter method for time
	 *
	 * @return defined time
	 */
	virtual const IPPITimePattern& getTime() const= 0;
	/**
	 * dummy destructor for template class
	 */
	virtual ~IValueHolderPattern() {};
};

#endif /* IPPIVALUESPATTERN_H_ */
