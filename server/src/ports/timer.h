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

#include "../util/thread/Thread.h"

#include "ListCalculator.h"
#include "switch.h"
#include "measureThread.h"

enum which_time
{
	notype= 0,
	/**
	 * set seconds since 1.1.1970 into subroutine value
	 */
	pass,
	/**
	 * set actual second (0-59) into subroutine value
	 */
	seconds,
	/**
	 * set actual minute (0-59) into subroutine value
	 */
	minutes,
	/**
	 * set actual hour (0-59) into subroutine value
	 */
	hours,
	/**
	 * set actual day (1-31) into subroutine value
	 */
	days,
	/**
	 * set actual month (1-12) into subroutine value
	 */
	months,
	/**
	 * set actual year (4 numbers) into subroutine value
	 */
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
	 * @param bTimerLog whether timer routine should write percent into database
	 * @param bNoDbRead whether times for TIMER subroutines should read from database
	 * @param objectID count of folder when defined inside an object, otherwise 0
	 */
	timer(string folderName, string subroutineName, bool bTimerLog, bool bNoDbRead,
					short finishedCPUtime, unsigned short objectID)
	: switchClass("TIMER", folderName, subroutineName, objectID),
	  m_bHasLinks(false),
	  m_nCaseNr(0),
	  m_bLogPercent(bTimerLog),
	  m_nFinishedCPUtime(finishedCPUtime),
	  m_bNoDbRead(bNoDbRead),
	  m_bSwitchbyTime(false),
	  m_bPassSecs(false),
	  m_eWhich(notype),
	  m_bMeasure(false),
	  m_bSeconds(true),
	  m_bFinished(true),
	  m_nAllowStarting(0),
	  m_omtime(folderName, subroutineName, "mtime", false, false, this),
	  m_oFinished(folderName, subroutineName, "finished", false, true, this),
	  m_oMicroseconds(folderName, subroutineName, "microsec", false, false, this),
	  m_oMilliseconds(folderName, subroutineName, "millisec", false, false, this),
	  m_oSeconds(folderName, subroutineName, "sec", false, false, this),
	  m_oMinutes(folderName, subroutineName, "min", false, false, this),
	  m_oHours(folderName, subroutineName, "hour", false, false, this),
	  m_oDays(folderName, subroutineName, "day", false, false, this),
	  m_oMonths(folderName, subroutineName, "month", false, false, this),
	  m_oYears(folderName, subroutineName, "year", false, false, this),
	  m_oDirect(folderName, subroutineName, "direction", false, false, this),
	  m_tmSec(0),
	  m_tmMicroseconds(0),
	  m_oSetNull(folderName, subroutineName, "setnull", false, true, this),
	  m_dSwitch(0),
	  m_bPoll(false),
	  m_dTimeBefore(0),
	  m_nDirection(-2),
	  m_bRunTime(false),
	  m_SUBVARLOCK(Thread::getMutex("SUBVARLOCK"))
	  { };
	/**
	 * configuration of object
	 *
	 * @param properties object of properties contains all subroutine parameters from measure.conf
	 * @param pStartFolder full list of folder and subroutines
	 * @return whether configuration was right done
	 */
	OVERWRITE bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * this method will be called from any measure thread to set as observer
	 * for starting own folder to get value from foreign folder
	 * if there the value was changing
	 *
	 * @param observer measure thread which containing the own folder
	 */
	OVERWRITE void setObserver(IMeasurePattern* observer);
	/**
	 * set value in subroutine.<br />
	 * All strings from parameter 'from' beginning with an one character type,
	 * followed from an colon 'r:' by ppi-reader, 'e:' by an account connected over Internet
	 * or 'i:' by intern folder:subroutine.
	 *
	 * @param value value which should be set with last changing time when set,
	 *              otherwise method create own time
	 * @param from which folder:subroutine or account changing the value
	 */
	OVERWRITE void setValue(const IValueHolderPattern& value, const InformObject& from);
	/**
	 * measure new value for subroutine
	 *
	 * @param actValue current value
	 * @return return measured value
	 */
	OVERWRITE auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue);
	/**
	 * get value from subroutine
	 *
	 * @param who describe who need the value information
	 * @return current value
	 */
	OVERWRITE auto_ptr<IValueHolderPattern> getValue(const InformObject& who);
	/**
	 * whether subroutine has the incoming sub-variable
	 *
	 * @param subvar name of sub-variable
	 * @return whether subroutine has this variable
	 */
	OVERWRITE bool hasSubVar(const string& subvar) const;
	/**
	 * return content of sub-variable from current subroutine
	 *
	 * @param who declare who need the value information
	 * @param subvar name of sub-variable
	 * @return value of sub-var
	 */
	OVERWRITE ppi_value getSubVar(const InformObject& who, const string& subvar) const;
	/**
	 * set subroutine for output doing actions
	 *
	 * @param whether should write output
	 */
	OVERWRITE void setDebug(bool bDebug);
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
	 * which case of subroutine for type TIMER be used.
	 *
	 *        0  -  no case be set at the beginning
	 *   case 1  -  Beside begin/while/end and only one action property seconds, minutes, hours, days, months or years be set.
	 *              The folder will be polling all seconds, minutes, hours, ... in the case of begin while end
	 *              when this parameters be set, otherwise all the time.
	 *              When no action property of seconds, minutes, ... be set but action property time, the folder polling all seconds
	 *              and the actual subroutine value will be filled with all seconds after 01.01.1970
	 *   case 2  -  Beside parameter begin/while/end and action property activate
	 *              also the parameters year, month, day, hour, min and or sec,
	 *              or alone parameter mtime be set
	 *              The time will be count down, after parameter begin while end, from the setting date in seconds
	 *              to time 0. This meaning, when setting time is reached subroutine has value 0.
	 *              With ending of while parameter or reached end parameter before and value was 0
	 *              own value be set to -1. This is also set when no time measure was running
	 *   case 3  -  Beside parameters begin/while/end also parameters of day, hour, min, sec, millisec, microsec
	 *              or alone parameter mtime be set. Differ with action property sec and micro between seconds and microseconds
	 *              count the setting time down to 0 inside the case of begin, while, end.
	 *              previously when a situation arise that parameter while ending or end is achieved (only when set),
     *              the count down to 0 is also reached.
     *              The next pass of this subroutine when 0 was reached before, the value is -1
     *              value -1 is also the case when no count down running.
     *              When the count down finished with 0, but inside while/end the routine found no ending
     *              and action poll be set, the subroutine begins again to run with full time.
	 *   case 4  -  Beside begin/while/end also the parameters from case 2 and action property direction be set.
	 *              Count also the time like case 2, but when stopping the subroutine hold the actual value
     *              this case is given when the property direction be set with 0 (count down to 0)
     *              or width 1 (count up to full setting time)
	 *   case 5  -  Only the active parameters of begin/while/end be set.
	 *              Time will be measured inside this active time
	 *              and can be differ between seconds (default or be set with action property 'sec')
	 *              or microseconds (action property have to be 'micro')
	 */
	unsigned short m_nCaseNr;
	/**
	 * whether activation of folder should be performed with length time of folder run
	 */
	bool m_bExactTime;
	/**
	 * whether soubroutine should always waiting for exact time
	 */
	bool m_bWaitTime;
	/**
	 * pull parameter 'reachend'/'wrongreach' from database with follow
	 * running folder synchronization ID
	 */
	string m_sSyncID;
	/**
	 * log inside database for starting earlier or estimate possible finish time
	 * by which CPU percent time length be taken
	 */
	bool m_bLogPercent;
	/**
	 * on which CPU time should differ by database writing for possible finished time
	 */
	short m_nFinishedCPUtime;
	/**
	 * whether should read possible folder length or finished times from database
	 */
	bool m_bNoDbRead;
	/**
	 * whether should calculate times only when switch is true
	 */
	bool m_bSwitchbyTime;
	/**
	 * whether should shown time since 1.1.1970 inside result
	 */
	bool m_bPassSecs;
	/**
	 * for result, which type of time be shown
	 */
	which_time m_eWhich;
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
	 * whether end time calculation or finished property be reached
	 * when m_bExactTime be set
	 */
	bool m_bFinished;
	/**
	 * whether foreign subroutine is designed to starting
	 * -1 not designed
	 * 0 never checked
	 * 1 correct designed
	 */
	short m_nAllowStarting;
	/**
	 * whether can starting external subroutine
	 */
	bool m_bStartExtern;
	/**
	 * subroutine to start per time
	 */
	SHAREDPTR::shared_ptr<IListObjectPattern> m_pStartObj;
	/**
	 * defined-value where get the time for count down
	 */
	ListCalculator m_omtime;
	/**
	 * for calculating middle length time
	 * when other subroutines has correct results
	 */
	ListCalculator m_oFinished;
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
	ppi_time m_tmStart;
	/**
	 * when m_bTimeMeasure not set, ending the time measure after this time
	 * which is subtracted from length folder time
	 */
	ppi_time m_tmStop;
	/**
	 * when m_bTimeMeasure not set, ending the time measure on this exact time
	 */
	ppi_time m_tmExactStop;
	/**
	 * full ending time when finished should be reached.<br />
	 * only defined when parameter finished in subroutine be set
	 */
	ppi_time m_tmWantFinish;
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
	 * actual time of running subroutine
	 */
	ppi_time m_oActTime;
	/**
	 * whether parameter begin/while/end for set new value will be done before<br />
	 * 0 is false and 1 is true
	 */
	double m_dSwitch;
	/**
	 * whether measure of time or count down
	 * should begin again
	 */
	bool m_bPoll;
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
	 * whether time measuring run
	 */
	bool m_bRunTime;
	/**
	 * percent by getting length, to write into calculation
	 */
	short m_nLengthPercent;
	/**
	 * static types to calculating length of reached finished time
	 */
	MeasureThread::timetype_t m_tReachedTypes;
	/**
	 * mutex lock for sub-variables
	 */
	pthread_mutex_t *m_SUBVARLOCK;

	/**
	 * running case 1 or 2<br />
	 * when folder should polling all seconds, minutes, hours, ...<br />
	 * or time count down to setting date time
	 *
	 * @param bswitch whether subroutine is activated from parameter begin, while or end
	 * @param tv actual time for subroutine
	 * @param debug whether subroutine is inside debug modus
	 * @return new value of subroutine
	 */
	double polling_or_countDown(const bool bswitch, ppi_time tv, const bool debug);
	/**
	 * calculate next needed time
	 *
	 * @param start whether method should define stopping time
	 * @param debug whether subroutine should write out debug strings
	 * @param actTime should be actual value when parameter start is true and come back with new ending time,
	 *                otherwise when parameter start is false it should be actual calculated time and come unchanged back
	 * @return calculated new value
	 */
	double calcNextTime(const bool& start, const bool& debug, ppi_time* actTime);
	/**
	 * calculating how long time should running for count down or time measure
	 *
	 * @param debug whether subroutine should write out debug strings
	 * @param actValue actual value of subroutine
	 * @param next actual time and give back calculated time to stopping
	 */
	double calcStartTime(const bool& debug, const double actValue, ppi_time* next);
	/**
	 * Subtract from time seconds for exact starting
	 *
	 * @param nextTime full waiting time need for ending
	 * @param debug whether actual session is for debugging
	 */
	double substractExactFinishTime(ppi_time* nextTime, const bool& debug);
};

#endif /*TIMER_H_*/
