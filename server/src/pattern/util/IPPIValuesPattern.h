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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sys/time.h>

#include <string>
#include <vector>

using namespace std;
using namespace boost;

/**
 * type of handled value inside ppi-server
 */
typedef double ppi_value;

class InformObject
{
public:
	/**
	 * position type from where the value comes
	 */
	enum posPlace_e
	{
		/**
		 * enum for null object
		 */
		NOSET= 0,
		/**
		 * inside from
		 * working list
		 * description should be folder:subroutine
		 */
		INTERNAL,
		/**
		 * time condition
		 * inside working list
		 * description should be date time
		 */
		TIMECONDITION,
		/**
		 * only informed while
		 * to check whether
		 * external port is reachable
		 */
		SEARCHSERVER,
		/**
		 * external from
		 * client over internet
		 * description should be user account
		 */
		EXTERNAL,
		/**
		 * from an shell command
		 * description should be folder:subroutine
		 */
		SHELL,
		/**
		 * from TIMER started ReadWorker thread
		 * description should be folder:subroutine
		 */
		READWORKER,
		/**
		 * from any external port
		 * description should be folder:subroutine
		 */
		READER
	};

	/**
	 * constructor to set no defined object
	 */
	InformObject()
	: m_eDirection(NOSET)
	{};
	/**
	 * constructor to define informing object
	 *
	 * @param place from which direction be informed
	 * @param from who does inform
	 */
	InformObject(const posPlace_e place, const string& from)
	: m_eDirection(place),
	  m_sDescription(from)
	{};
	/**
	 * return from where object be informed
	 *
	 * @return direction place type
	 */
	 posPlace_e getDirection() const
	 { return m_eDirection; };
	 /**
	  * return from description from where
	  * object be informed.<br />
	  * maybe folder or folder:subroutine
	  *
	  * @return who does inform
	  */
	 string getWhoDescription() const
	 { return m_sDescription; };
	 /**
	  * return an string combination
	  * of direction from where object coming
	  * and who does inform
	  *
	  * @return description of object
	  */
	 string toString() const
	 {
		 string sRv;

		switch(m_eDirection)
		{
		case INTERNAL:
			sRv= "INTERNAL '";
			break;
		case EXTERNAL:
			sRv= "internet connection account '";
			break;
		case TIMECONDITION:
			sRv= "time condition at ";
			break;
		case SHELL:
			sRv= "SHELL script '";
			break;
		case READWORKER:
			sRv= "TIMER external started reading '";
			break;
		case READER:
			sRv= "external physical port '";
			break;
		default:
			sRv= "UNKNOWN direction '";
			break;
		}
		sRv+= m_sDescription;
		if(m_eDirection != TIMECONDITION)
			sRv+= "'";
		return sRv;
	 };

	 /**
	  * return short definition of hole object
	  * to convert after transfer back into object
	  *
	  * @return short definition
	  */
	 string getDefString() const
	 {
		 string sRv;

		switch(m_eDirection)
		{
		case INTERNAL:
			sRv= "INTERNAL|";
			break;
		case EXTERNAL:
			sRv= "EXTERNAL|";
			break;
		case TIMECONDITION:
			sRv= "TIMECONDITION|";
			break;
		case SHELL:
			sRv= "SHELL|'";
			break;
		case READWORKER:
			sRv= "READWORKER|";
			break;
		case READER:
			sRv= "READER|";
			break;
		default:
			sRv= "NOSET|";
			break;
		}
		sRv+= m_sDescription;
		return sRv;
	 }

	 /**
	  * read after transaction
	  * InformObjct string from getDefString
	  * to create again an Object
	  */
	 void readDefString(const string& defString)
	 {
		 vector<string> spl;

		 m_sDescription= "";
		 if(defString == "")
		 {
			 m_eDirection= NOSET;
			 return;
		 }
		 split(spl, defString, is_any_of("|"));
		 if(spl[0] == "INTERNAL")
			 m_eDirection= INTERNAL;
		 else if(spl[0] == "EXTERNAL")
			 m_eDirection= EXTERNAL;
		 else if(spl[0] == "TIMECONDITION")
			 m_eDirection= TIMECONDITION;
		 else if(spl[0] == "SHELL")
			 m_eDirection= SHELL;
		 else if(spl[0] == "READWORKER")
			 m_eDirection= READWORKER;
		 else if(spl[0] == "READER")
			 m_eDirection= READER;
		 else if(spl[0] == "NOSET")
			 m_eDirection= NOSET;
		 if(spl.size() > 1)
			 m_sDescription= spl[1];
	 }

	 /**
	  * operator for container to know
	  * by counting order
	  * whether object is same
	  */
	 bool operator == (const InformObject& other) const
	 {
		if(	m_eDirection == other.m_eDirection &&
			m_sDescription == other.m_sDescription	)
		{
			return true;
		}
		return false;
	 }
	 /**
	  * operator for container to know
	  * by counting order
	  * whether object is lower
	  */
	 bool operator < (const InformObject& other) const
	 {
		 if(m_eDirection < other.m_eDirection)
			 return true;
		 if(m_sDescription < other.m_sDescription)
			 return true;
		 return false;
	 }

private:
	/**
	 * from which direction
	 * the value coming
	 */
	posPlace_e m_eDirection;
	/**
	 * description from where
	 * the value coming
	 */
	string m_sDescription;
};

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
	 * fill object time from an normaly ppi_value type
	 *
	 * @param value ppi_value type stored inside object
	 */
	virtual IPPITimePattern& operator = (ppi_value value)= 0;
	/**
	 * write own holded time value inside an normal ppi_value type
	 *
	 * @param value ppi_value into which should writing
	 */
	virtual void operator >> (ppi_value& value) const= 0;
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
	 * read a string representation of time.<br />
	 * formating see man pages from strptime.<br />
	 * on format string is possible to write %N
	 * to read milliseconds and microseconds
	 * on ending of time string
	 *
	 * @param str string of time definition
	 * @param format formating of string, see strptime
	 */
	bool read(const string& str, string format);
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
