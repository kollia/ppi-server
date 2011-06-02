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
#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>
#include <sys/time.h>

#include <string>

#include "../util/structures.h"

#include "../util/properties/configpropertycasher.h"

#include "ListCalculator.h"
#include "switch.h"

class timer : public switchClass
{
public:
	timer(string folderName, string subroutineName)
	: switchClass("TIMER", folderName, subroutineName),
	  m_bTime(false),
	  m_bMeasure(false),
	  m_bSeconds(true),
	  m_omtime(folderName, subroutineName, "mtime", false, false),
	  m_tmSec(0),
	  m_tmMicroseconds(0),
	  m_oSetNull(folderName, subroutineName, "setnull", false, true),
	  m_dSwitch(0),
	  m_dTimeBefore(0),
	  m_oEnd(folderName, subroutineName, "end", false, true)
	  { };
	virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * measure new value for subroutine
	 *
	 * @param actValue current value
	 * @return return measured value
	 */
	virtual double measure(const double actValue);
	/**
	 * set subroutine for output doing actions
	 *
	 * @param whether should write output
	 */
	virtual void setDebug(bool bDebug);
	/**
	 * calculate double result whether should measure in second range
	 * or microsecond range
	 *
	 * @param tv current time
	 * @param secondcalc whether should calculate in seconds (true) or microseconds (false)
	 * @return result of subroutine
	 */
	static double calcResult(timeval tv, bool secondcalc);
	/**
	 * destructor
	 */
	virtual ~timer();

protected:
	/**
	 * whether subroutine measure time (true)
	 * or makes an count down (false)
	 */
	bool m_bTime;
	/**
	 * whether measure actually the time
	 */
	bool m_bMeasure;
	/**
	 * whether measuring time from seconds to years (true)
	 * or from microseconds to minutes (false)
	 */
	bool m_bSeconds;
	/**
	 * defined-value where get the time for count down
	 */
	ListCalculator m_omtime;
	/**
	 * seconds to refresh when m_smtime not be set
	 */
	time_t m_tmSec;
	/**
	 * microseconds to refresh when m_smtime not be set
	 */
	suseconds_t m_tmMicroseconds;
	/**
	 * start on this time the measuring when m_bTime set to measure,
	 * otherwise start the hole folder again on this time
	 */
	timeval m_tmStart;
	/**
	 * string of options whether set value to 0
	 */
	ListCalculator m_oSetNull;

	/**
	 * set min and max parameter to the range which can be set for this subroutine.<br />
	 * If the subroutine is set from 0 to 1 and float false, the set method sending only 0 and 1 to the database.
	 * Also if the values defined in an bit value 010 (dec:2) set 0 and for 011 (dec:3) set 1 in db.
	 * Otherwise the range is only for calibrate the max and min value if set from client outher range.
	 * If pointer of min and max parameter are NULL, the full range of the double value can be used
	 *
	 * @param bfloat whether the values can be float variables
	 * @param min the minimal value
	 * @param max the maximal value
	 * @return whether the range is defined or can set all
	 */
	virtual bool range(bool& bfloat, double* min, double* max);

private:
	/**
	 * whether parameter begin/while/end for set new value will be done before<br />
	 * 0 is false and 1 is true
	 */
	double m_dSwitch;
	/**
	 * Time measured by last pass
	 */
	double m_dTimeBefore;
	/**
	 * calculation for end string
	 */
	ListCalculator m_oEnd;
};

#endif /*TIMER_H_*/
