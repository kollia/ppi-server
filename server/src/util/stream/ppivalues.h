/**
 *   This file 'ppivalues.h' is part of ppi-server.
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

#ifndef PPIVALUES_H_
#define PPIVALUES_H_

#include <sys/time.h>

#include <string>

#include "../smart_ptr.h"

#include "../../pattern/util/IPPIValuesPattern.h"

using namespace std;

class ppi_valTime
{

};

/**
 * nearly equal for double comparison.<br />
 * Compare only first 15 digits of expression
 *
 * @param a first value to compare
 * @param b with second value
 * @return whether expressions are equal
 */
bool nearly_equal(const ppi_value& a, const ppi_value& b);

/**
 * double comparison whether first value will be nearly lower than second.<br />
 * Compare only first 15 digits of expression
 *
 * @param lower first value which should be lower or nearly equal
 * @param higher second value which should be higher or nearly equal
 * @return whether expressions are lower
 */
bool nearly_lower(const ppi_value& lower, const ppi_value& higher);

/**
 * double comparison whether first value will be nearly higher than second.<br />
 * Compare only first 15 digits of expression
 *
 * @param higher first value which should be higher or nearly equal
 * @param lower second value which should be lower or nearly equal
 * @return whether expressions are higher
 */
bool nearly_higher(const ppi_value& higher, const ppi_value& lower);

/**
 * type of handled time inside ppi-server
 * source:LogHolderPattern.cpp
 */
class ppi_time : public IPPITimePattern
{
public:
	/**
	 * constructor to initial timeval with 0
	 */
	ppi_time()
	{ tv_sec= 0; tv_usec= 0; m_nErrno= 0; };
	/**
	 * copy constructor to initial object with new timeval
	 */
	ppi_time(const timeval& time)
	{ tv_sec= time.tv_sec; tv_usec= time.tv_usec; m_nErrno= 0; };
	/**
	 * copy operator for time pattern
	 */
	virtual IPPITimePattern& operator = (const IPPITimePattern& time);
	/**
	 * copy operator for time object
	 */
	virtual IPPITimePattern& operator = (const ppi_time& time);
	/**
	 * copy operator for normal timeval value
	 */
	virtual IPPITimePattern& operator = (const timeval& time);
	/**
	 * equal operator
	 */
	virtual bool operator == (const timeval& time) const;
	/**
	 * unequal operator
	 */
	virtual bool operator != (const timeval& time) const;
	/**
	 * lower operator
	 */
	virtual bool operator < (const timeval& time) const;
	/**
	 * greater operator
	 */
	virtual bool operator > (const timeval& time) const;
	/**
	 * lower equal operator
	 */
	virtual bool operator <= (const timeval& time) const;
	/**
	 * greater equal operator
	 */
	virtual bool operator >= (const timeval& time) const;
	/**
	 * addition operator
	 */
	virtual timeval operator + (const timeval& time);
	/**
	 * addition operator to own object
	 */
	virtual IPPITimePattern& operator += (const timeval& time);
	/**
	 * subtraction operator
	 */
	virtual timeval operator - (const timeval& time);
	/**
	 * subtraction operator to own object
	 */
	virtual IPPITimePattern& operator -= (const timeval& time);
	/**
	 * fill object time from an normaly ppi_value type
	 *
	 * @param value ppi_value type stored inside object
	 */
	virtual IPPITimePattern& operator = (ppi_value value);
	/**
	 * write own holded time value inside an normal ppi_value type
	 *
	 * @param value ppi_value into which should writing
	 */
	virtual void operator >> (ppi_value& value) const;
	/**
	 * allocation time from object
	 * to an double value
	 *
	 * @return result of current inherit time
	 */
	virtual operator ppi_value() const
	{
		double val;

		val= tv_usec;
		val/= 1000000;
		val+= tv_sec;
		return val;
	}
	/**
	 * set actually time into own object
	 *
	 * @return true for success, or -1 for failure (in which case errno is set appropriately).
	 */
	bool setActTime();
	/**
	 * check whether time is set
	 */
	bool isSet() const;
	/**
	 * set own timer object to 0
	 */
	void clear();
	/**
	 * write ppi_time as date with microseconds or as second value
	 * with also microseconds after decimal point into an string.<br />
	 * When by date writing an error occurs, the date will be written as
	 * xx.xx.xxxx xx:xx:xx with correct microseconds
	 *
	 * @param bDate whether should write as date (true) or seconds (false)
	 * @param bDebug whether should output error on console
	 * @return string object as date or seconds
	 */
	string toString(const bool& bDate) const;
	/**
	 * read a string representation of time.<br />
	 * formating see man pages from strptime.<br />
	 * on format string is possible to write %N
	 * to read milliseconds and microseconds
	 * on ending of time string
	 *
	 * @param str string of time definition
	 * @param format formating of string
	 */
	bool read(const string& str, string format);
	/**
	 * sending back ERRNO definition for <code>setActTime()</code>
	 * or <code>toString()</code> when failed
	 *
	 * @return ERRNO integer or 0
	 */
	int error() const;
	/**
	 * create error string from ERRNO number
	 *
	 * @return error string
	 */
	string errorStr() const;

private:
	/**
	 * errno number when <code>localtime_r()</code> inside <code>toString()</code>
	 * or <code>gettimeofday()</code> inside <code>setActTime()</code> fail
	 */
	mutable int m_nErrno;
	/**
	 * self setting error string when an error occurred but mo errno be set
	 */
	mutable string m_sError;
};

/**
 * structure to define handled value
 * with last changing time
 */
struct ValueHolder : public IValueHolderPattern
{
public:
	/**
	 * constructor to set time as to 0
	 */
	ValueHolder()
	{ value= 0; };
	/**
	 * copy constructor
	 *
	 * @param str other object
	 * @return own object
	 */
	virtual IValueHolderPattern& operator = (const IValueHolderPattern& str)
	{ setTimeValue(str); return *this; };
	/**
	 * copy constructor
	 *
	 * @param str other object
	 * @return own object
	 */
	virtual IValueHolderPattern& operator = (const auto_ptr<IValueHolderPattern> str)
	{ setTimeValue(*str.get()); return *this; };
	/**
	 * copy constructor
	 *
	 * @param str other object
	 * @return own object
	 */
	virtual IValueHolderPattern& operator = (const SHAREDPTR::shared_ptr<IValueHolderPattern> str)
	{ setTimeValue(*str.get()); return *this; };
	/**
	 * setter method for value with time
	 *
	 * @param str setting value
	 */
	virtual void setTimeValue(const IValueHolderPattern& str)
	{ value= str.getValue(); lastChanging= str.getTime(); };
	/**
	 * setter method for value
	 *
	 * @param value setting value
	 */
	virtual void setValue(const ppi_value& value)
	{ this->value= value; };
	/**
	 * getter method for value
	 *
	 * @return defined value
	 */
	virtual ppi_value getValue() const
	{ return value; };
	/**
	 * setter method for time
	 *
	 * @param time setting time
	 */
	virtual void setTime(const IPPITimePattern& time)
	{ lastChanging= time; };
	/**
	 * getter method for time
	 *
	 * @return defined time
	 */
	virtual const IPPITimePattern& getTime() const
	{ return lastChanging; };

	/**
	 * main handled value inside ppi-server
	 */
	ppi_value value;
	/**
	 * last changed time of value
	 */
	ppi_time lastChanging;

};

#endif /* PPIVALUES_H_ */
