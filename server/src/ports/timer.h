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

enum which_time
{
	notype= 0,
	seconds,
	minutes,
	hours,
	days,
	months,
	years
};

class timer : public switchClass
{
public:
	/**
	 * constructor of object
	 *
	 * @param folderName name of folder
	 * @param subroutineName name of subroutine
	 */
	timer(string folderName, string subroutineName)
	: switchClass("TIMER", folderName, subroutineName),
	  m_bHasLinks(false),
	  m_bReadTime(false),
	  m_bTime(false),
	  m_bSwitchbyTime(false),
	  m_bActivate(false),
	  m_bTimeMeasure(false),
	  m_bMeasure(false),
	  m_bSeconds(true),
	  m_omtime(folderName, subroutineName, "mtime", false, false),
	  m_oMicroseconds(folderName, subroutineName, "microsec", false, false),
	  m_oMilliseconds(folderName, subroutineName, "millisec", false, false),
	  m_oSeconds(folderName, subroutineName, "sec", false, false),
	  m_oMinutes(folderName, subroutineName, "min", false, false),
	  m_oHours(folderName, subroutineName, "hour", false, false),
	  m_oDays(folderName, subroutineName, "day", false, false),
	  m_oMonths(folderName, subroutineName, "month", false, false),
	  m_oYears(folderName, subroutineName, "year", false, false),
	  m_oDirect(folderName, subroutineName, "direction", false, false),
	  m_tmSec(0),
	  m_tmMicroseconds(0),
	  m_oSetNull(folderName, subroutineName, "setnull", false, true),
	  m_dSwitch(0),
	  m_dTimeBefore(0),
	  m_nDirection(-2)
	  { };
	/**
	 * configuration of object
	 *
	 * @param properties object of properties contains all subroutine parameters from measure.conf
	 * @param pStartFolder full list of folder and subroutines
	 * @return whether configuration was right done
	 */
	virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * measure new value for subroutine
	 *
	 * @param actValue current value
	 * @return return measured value
	 */
	virtual double measure(const double actValue);
	/**
	 * get value from subroutine
	 *
	 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
	 * 				This time only defined for external reading over OwPort's.
	 * @return current value
	 */
	virtual double getValue(const string& who);
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
	 * whether subroutine is linked with an other one
	 */
	bool m_bHasLinks;
	/**
	 * whether should read only actual time
	 */
	bool m_bReadTime;
	/**
	 * whether action time be set
	 */
	bool m_bTime;
	/**
	 * whether should calculate times only when switch is true
	 */
	bool m_bSwitchbyTime;
	/**
	 * whether time should activated at an fix time
	 */
	bool m_bActivate;
	/**
	 * for m_bTime which type of time be shown in result
	 */
	which_time m_eWhich;
	/**
	 * whether subroutine measure time (true)
	 * or makes an count down (false)
	 */
	bool m_bTimeMeasure;
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
	 * defined-value for microseconds
	 */
	ListCalculator m_oMicroseconds;
	/**
	 * defined-value for milliseconds
	 */
	ListCalculator m_oMilliseconds;
	/**
	 * defined-value for seconds
	 */
	ListCalculator m_oSeconds;
	/**
	 * defined-value for minutes
	 */
	ListCalculator m_oMinutes;
	/**
	 * defined-value for hours
	 */
	ListCalculator m_oHours;
	/**
	 * defined-value for days
	 */
	ListCalculator m_oDays;
	/**
	 * defined-value for months
	 */
	ListCalculator m_oMonths;
	/**
	 * defined-value for years
	 */
	ListCalculator m_oYears;
	/**
	 * in which direction the count down should running.<br />
	 * By result of 0 or lower running near to 0.<br />
	 * By result of 1 or higher the other direction to full count.
	 */
	ListCalculator m_oDirect;
	/**
	 * seconds to refresh when m_smtime not be set
	 */
	time_t m_tmSec;
	/**
	 * microseconds to refresh when m_smtime not be set
	 */
	suseconds_t m_tmMicroseconds;
	/**
	 * start on this time the measuring when m_bTimeMeasure set to measure,
	 * otherwise start the hole folder again on this time
	 */
	timeval m_tmStart;
	/**
	 * when m_bTimeMeasure not set, ending the time measure on this time
	 */
	timeval m_tmStop;
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
	 * witch direction be set or none.<br />
	 * <table>
	 *   <tr>
	 *     <td align="center">
	 *       -2
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       no direction be set
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td align="center">
	 *       -1
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       direction be set but unknown
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td align="center">
	 *       0
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       direction should be measured down to 0
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td align="center">
	 *       1
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       direction should be measured up to defined time
	 *     </td>
	 *   </tr>
	 * </table>
	 */
	short m_nDirection;
	/**
	 * by measuring with direction
	 * this is the value by starting measuring
	 */
	double m_dStartValue;

	/**
	 * calculate next needed time
	 *
	 * @param start whether method should define stopping time
	 * @param debug whether subroutine should write out debug strings
	 * @param actTime should be actual value when parameter start is true and come back with new ending time,
	 *                otherwise when parameter start is false it should be actual calculated time and come unchanged back
	 * @return calculated new value
	 */
	double calcNextTime(const bool& start, const bool& debug, timeval* actTime);
	/**
	 * calculating how long time should running for count down or time measure
	 *
	 * @param debug whether subroutine should write out debug strings
	 * @param actValue actual value of subroutine
	 * @param next give back calculated time to stopping
	 */
	double calcStartTime(const bool& debug, const double actValue, timeval* next);
};

#endif /*TIMER_H_*/
