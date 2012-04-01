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
#include "limits.h"

#include <iostream>

#include <sys/time.h>

#include "timer.h"
#include "measureThread.h"

#include "../util/Calendar.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"


using namespace util;

bool timer::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bsec, bmicro, bOk= true;
	double dDefault;
	string prop, smtime, sSetNull;

	//Debug info to stop by right subroutine
	/*string stopfolder("TRANSMIT_SONY");
	string stopsub("after");
	if(	getFolderName() == stopfolder &&
		getSubroutineName() == stopsub	)
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << stopfolder << ":" << getSubroutineName() << endl;
	}*/
	m_bSeconds= true;
	m_bTimeMeasure= false;
	m_tmStart.tv_sec= 0;
	m_tmStart.tv_usec= 0;
	m_tmStop.tv_sec= 0;
	m_tmStop.tv_usec= 0;
	m_bTime= properties->haveAction("time");
	if(!m_bTime)
	{
		if(properties->haveAction("seconds"))
		{
			m_eWhich= seconds;
			m_bTime= true;

		}else if(properties->haveAction("minutes"))
		{
			m_eWhich= minutes;
			m_bTime= true;

		}else if(properties->haveAction("hours"))
		{
			m_eWhich= hours;
			m_bTime= true;

		}else if(properties->haveAction("days"))
		{
			m_eWhich= days;
			m_bTime= true;

		}else if(properties->haveAction("months"))
		{
			m_eWhich= months;
			m_bTime= true;

		}else if(properties->haveAction("years"))
		{
			m_eWhich= years;
			m_bTime= true;
		}else
		{
			m_eWhich= notype;
			m_bTime= false;
			m_bActivate= properties->haveAction("activate");
		}
	}else
	{
		m_eWhich= notype;
		m_bTime= true;
	}
	if(!m_bTime)
	{
		m_bTimeMeasure= true;
		bsec= properties->haveAction("sec");
		bmicro= properties->haveAction("micro");
		if(bsec && bmicro)
		{
			prop= properties->getMsgHead(/*ERROR*/true);
			prop+= "action sec and micro can not be set both";
			LOG(LOG_ERROR, prop);
			cerr << prop << endl;
			bOk= false;
		}else if(bmicro)
			m_bSeconds= false;
	}

	// check whether should do count down of time
	if(m_bTime)
	{
		if(properties->getValue("begin", /*warning*/false) != "")
			m_bSwitchbyTime= true;
		else if(properties->getValue("while", /*warning*/false) != "")
			m_bSwitchbyTime= true;
		else if(properties->getValue("end", /*warning*/false) != "")
		{
			string msg(properties->getMsgHead(/*error*/false));

			msg+= "no begin or while property be set, so do not wait for beginning and measure all the time";
			LOG(LOG_WARNING, msg);
			tout << msg << endl;
		}
	}
	smtime= properties->getValue("mtime", /*warning*/false);
	m_tmSec= 0;
	if(smtime == "")
	{
		if(m_bSeconds)
		{
			m_oYears.init(pStartFolder, properties->getValue("year", /*warning*/false));
			if(!m_oYears.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				m_bTimeMeasure= false;
			}
			m_oMonths.init(pStartFolder, properties->getValue("month", /*warning*/false));
			if(!m_oMonths.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				m_bTimeMeasure= false;
			}
		}
		m_oDays.init(pStartFolder, properties->getValue("day", /*warning*/false));
		if(!m_oDays.isEmpty())
		{
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
			m_bTimeMeasure= false;
		}
		m_oHours.init(pStartFolder, properties->getValue("hour", /*warning*/false));
		if(!m_oHours.isEmpty())
		{
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
			m_bTimeMeasure= false;
		}
		m_oMinutes.init(pStartFolder, properties->getValue("min", /*warning*/false));
		if(!m_oMinutes.isEmpty())
		{
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
			m_bTimeMeasure= false;
		}
		m_oSeconds.init(pStartFolder, properties->getValue("sec", /*warning*/false));
		if(!m_oSeconds.isEmpty())
		{
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
			m_bTimeMeasure= false;
		}
		if(!m_bSeconds)
		{
			m_oMilliseconds.init(pStartFolder, properties->getValue("millisec", /*warning*/false));
			if(!m_oMilliseconds.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				m_bTimeMeasure= false;
			}
			m_oMicroseconds.init(pStartFolder, properties->getValue("microsec", /*warning*/false));
			if(!m_oMicroseconds.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				m_bTimeMeasure= false;
			}
		}
	}else
	{
		m_bTimeMeasure= false;
		m_omtime.init(pStartFolder, smtime);
	}
	if(m_bTimeMeasure)
	{ // measure time
		if(m_bSeconds)
			m_bReadTime= properties->haveAction("time");
		if(!m_bReadTime)
		{
			sSetNull= properties->getValue("setnull", /*warning*/false);
			m_oSetNull.init(pStartFolder, sSetNull);
		}
	}else
	{
		smtime= properties->getValue("direction", /*warning*/false);
		if(smtime != "")
		{
			m_oDirect.init(pStartFolder, smtime);
			m_nDirection= -1;
		}else
			m_nDirection= -2;
	}


	if(properties->getPropertyCount("link"))
		m_bHasLinks= true;
	if(!initLinks("TIMER", properties, pStartFolder))
		bOk= false;
	properties->notAllowedAction("binary");
	if(!switchClass::init(properties, pStartFolder))
		bOk= false;
	prop= "default";
	dDefault= properties->getDouble(prop, /*warning*/false);
	if(prop == "#ERROR")
	{// no default be set in configuration files for subroutine
		if(	!m_bTimeMeasure &&
			m_oDirect.isEmpty()	)
		{
			setValue(-1, "i:"+getFolderName()+":"+getSubroutineName());
			m_dTimeBefore= -1;

		}//else default value is 0
	}else
		m_dTimeBefore= dDefault;
	return bOk;
}

bool timer::range(bool& bfloat, double* min, double* max)
{
	if(m_bSeconds)
		bfloat= false;
	else
		bfloat= true;
	if(m_bTimeMeasure)
		*min= 0;
	else
		*min= -1;
	*max= (double)LONG_MAX;
	return true;
}

double timer::measure(const double actValue)
{
	bool bswitch;
	bool bEndCount(false);
	bool debug= isDebug();
	double need, nBeginTime(0);
	timeval tv;
	switchClass::setting set;

	if(	!m_bTime ||
		m_bSwitchbyTime ||
		m_bTimeMeasure ||
		!m_bMeasure			)
	{
		m_dSwitch= switchClass::measure(m_dSwitch, set);
	}
	//Debug info to stop by right subroutine
	/*if(!m_oDirect.isEmpty())
	//if(	getFolderName() == "TRANSMIT_SONY" &&
	//	getSubroutineName() == "new_activate")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << getFolderName() << ":" << getSubroutineName() << endl;
	}*/
	if(m_dSwitch > 0)
		bswitch= true;
	else
		bswitch= false;
	if(	bswitch ||
		m_bMeasure ||
		m_bTimeMeasure ||
		(	m_bTime &&
			m_omtime.isEmpty()	)	)
	{
		if(gettimeofday(&tv, NULL))
		{
			string msg("ERROR: cannot get time of day,\n");

			msg+= "       so cannot measure time for TIMER function in folder ";
			msg+= getFolderName() + " and subroutine " + getSubroutineName() + ".";
			TIMELOG(LOG_ALERT, "gettimeofday", msg);
			if(debug)
				tout << msg << endl;
			if(m_bTimeMeasure)
			{
				if(m_nDirection > -2)
					need= actValue;
				else
					need= 0;
			}else
				need= -1;
			return need;
		}
	}
	if(m_bTime)
	{
		time_t actTime;
		tm local;

		if(!m_omtime.isEmpty())
		{
			m_omtime.calculate(need);
			tv.tv_sec= static_cast<time_t>(need);
		}
		if(debug)
		{
			if(	!m_bSwitchbyTime ||
				bswitch				)
			{
				tout << "subroutine is defined to measure time of date" << endl;
			}else
				tout << "subroutine is this time iside begin/while/end not defined for measure" << endl;
		}
		if(	m_bSwitchbyTime &&
			!bswitch			)
		{
			if(m_tmStop.tv_sec <= actTime)
				return -1;
		}
		actTime= tv.tv_sec;
		if(m_eWhich > notype)
			if(localtime_r(&actTime, &local) == NULL)
			{
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				return -1;
			}
		if(debug)
			tout << "measuring is defined for ";
		if(m_tmStop.tv_sec > actTime)
		{
			switch(m_eWhich)
			{
			case notype:
				if(debug)
					tout << "seconds" << endl;
				break;
			case seconds:
				if(debug)
					tout << "seconds" << endl;
				actTime= local.tm_sec;
				break;
			case minutes:
				if(debug)
					tout << "minutes" << endl;
				actTime= local.tm_min;
				break;
			case hours:
				if(debug)
					tout << "hours" << endl;
				actTime= local.tm_hour;
				break;
			case days:
				if(debug)
					tout << "days" << endl;
				actTime= local.tm_mday;
				break;
			case months:
				if(debug)
					tout << "months" << endl;
				actTime= local.tm_mon + 1;
				break;
			case years:
				if(debug)
					tout << "years" << endl;
				actTime= local.tm_year + 1900;
				break;
			}
			if(debug)
			{
				tm l;

				if(localtime_r(&tv.tv_sec, &l) == NULL)
				{
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				}
				tout << "              actual time is " << asctime(&l);
				if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
				{
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				}
				tout << "folder was set to refresh at " << asctime(&l);
			}
			return static_cast<double>(actTime);
		}
		if(m_tmSec > 0)
		{
			double res;

			m_tmSec= 0;
			if(!m_oSeconds.isEmpty())
			{
				m_oSeconds.calculate(res);
				m_tmSec+= static_cast<time_t>(res);
			}
			if(!m_oMinutes.isEmpty())
			{
				m_oMinutes.calculate(res);
				m_tmSec+= static_cast<time_t>(res) * 60;
			}
			if(!m_oHours.isEmpty())
			{
				m_oHours.calculate(res);
				m_tmSec+= static_cast<time_t>(res) * 60 * 60;
			}
			if(!m_oDays.isEmpty())
			{
				m_oDays.calculate(res);
				m_tmSec+= static_cast<time_t>(res) * 24 * 60 * 60;
			}
		}
		m_tmStop.tv_sec= actTime + m_tmSec;
		switch(m_eWhich)
		{
		case notype:
			if(debug)
				tout << "seconds" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec+= 1;
			break;
		case seconds:
			if(debug)
				tout << "seconds" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec+= 1;
			actTime= local.tm_sec;
			break;
		case minutes:
			if(debug)
				tout << "minutes" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec= Calendar::nextMinute(actTime, &local);
			actTime= local.tm_min;
			break;
		case hours:
			if(debug)
				tout << "hours" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec= Calendar::nextHour(actTime, &local);
			actTime= local.tm_hour;
			break;
		case days:
			if(debug)
				tout << "days" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec= Calendar::nextDay(actTime, &local);
			actTime= local.tm_mday;
			break;
		case months:
			if(debug)
				tout << "months" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec= Calendar::nextMonth(actTime, &local);
			actTime= local.tm_mon + 1;
			break;
		case years:
			if(debug)
				tout << "years" << endl;
			if(m_tmSec == 0)
				m_tmStop.tv_sec= Calendar::nextYear(actTime, &local);
			actTime= local.tm_year + 1900;
			break;
		}
		if(debug)
		{
			tm l;

			if(localtime_r(&tv.tv_sec, &l) == NULL)
			{
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			}
			tout << "               actual time is " << asctime(&l);
			if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
			{
				TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			}
			tout << "folder should be refreshed at " << asctime(&l);
		}
		getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
		return static_cast<double>(actTime);
	}
	if(	bswitch ||
		m_bMeasure	)
	{
		if(!m_bTimeMeasure)
		{ // measure count down or time
			timeval next;

			if(	m_bMeasure == false ||
				(	bswitch &&
					set == BEGIN	)	)
			{// BEGIN to measure

				if(!m_oDirect.isEmpty())
					m_nDirection= -1; // no direction (-2) was set in init() method
				if(debug)
				{
					tout << "BEGIN time measuring ";
					if(m_nDirection == -2)
						tout << "for count down" << endl;
					else
						tout << "to specific time" << endl;
				}
				next= tv;
				m_tmStart= tv;
				m_dStartValue= actValue;
				need= calcStartTime(debug, actValue, &next);
				if(need == -1)
				{
					if(m_nDirection > -2)
						need= actValue;
					m_bMeasure= false;
				}else
					m_bMeasure= true;

			}else if( timercmp(&m_tmStop, &tv, <) )
			{ // reaching end of count down
			  // now polling ending, or begin with new time
				double direct;
				timeval was;

				if(debug)
				{
					char stime[18];
					tm l;

					tout << "reach END of time measuring" << endl;
					tout << "folder was refreshed because time of " << need;
					tout << " seconds was reached" << endl;
					if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
						TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
					strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
					tout << "refresh time (" << stime << " ";
					tout << MeasureThread::getUsecString(m_tmStop.tv_usec) << ")" << endl;
					if(localtime_r(&tv.tv_sec, &l) == NULL)
						TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
					strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
					tout << " actual time (" << stime << " ";
					tout << MeasureThread::getUsecString(tv.tv_usec) << ")" << endl;
					tout << "look whether should polling time again" << endl;
				}
				if(m_nDirection == -1)
				{
					m_oDirect.calculate(direct);
					if(direct < 1)
						m_nDirection= 0;
					else
						m_nDirection= 1;
				}
				if(m_nDirection == 0)
					need= 0;
				else
					need= calcNextTime(/*start*/false, debug, &tv);
				m_dSwitch= switchClass::measure(m_dSwitch, set, &need);
				if(m_dSwitch > 0)
					bswitch= true;
				else
					bswitch= false;
				if(bswitch)
				{// if end parameter with own subroutine as 0 is true,
				 // do not begin count down again
					if(debug)
					{
						tout << "end of ";
						if(m_nDirection > -2)
							tout << "time ";
						else
							tout << "count down ";
						tout << "is reached, polling again" << endl;
					}

					timeval next;

					if( m_nDirection == 0 ||
						m_nDirection == -2	)
					{
						was.tv_sec= m_tmSec;
						was.tv_usec= m_tmMicroseconds;
						need= calcResult(was, m_bSeconds);
					}else
						need= 0;
					next= tv;
					m_tmStart= tv;
					m_dStartValue= need;
					need= calcStartTime(debug, need, &next);
					if(need == -1)
					{
						if(m_nDirection > -2)
							need= actValue;
						m_bMeasure= false;
						bEndCount= true;
					}else
						m_bMeasure= true;
					if(m_nDirection == -2)
					{// toDo: after checking behavior with transmitter, remove this if-sentence
						need= 0;
					}

				}else
				{
					m_bMeasure= false;
					bEndCount= true;
					if(m_nDirection == -2)
						need= 0;
				}

			}else
			{ // count down is running
				if(debug)
				{
					timeval newtime;

					timersub(&m_tmStop, &tv, &newtime);
					tout << "WHILE: measuring of time to specific end time is running" << endl;
					tout << "routine should stop at " << m_tmStop.tv_sec << " seconds" << endl;
					tout << "actualy we have " << tv.tv_sec << " seconds" << endl;
					tout << "this means time stopping in " << newtime.tv_sec << " seconds" << endl;
					tout << endl;
				}
				need= calcNextTime(/*start*/false, debug, &tv);
				if(debug)
				{
					tout << "check whether ";
					if(m_nDirection > -2)
						tout << "time run ";
					else
						tout << "count down ";
					tout << "should during on" << endl;
				}
				m_dSwitch= switchClass::measure(m_dSwitch, set, &need);
				if(m_dSwitch > 0)
					bswitch= true;
				else
					bswitch= false;
				if(!bswitch)
				{
					if(m_nDirection == -2)
						need= 0;
					m_bMeasure= false;
				}

				if(debug)
				{
					char stime[18];
					tm l;

					if(bswitch)
					{
						tout << "folder should start again in " << need << " seconds" << endl;
						if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
							TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
						strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
						tout << "    by time (" << stime << " ";
						tout << MeasureThread::getUsecString(m_tmStop.tv_usec) << ")" << endl;
					}else
					{
						double nTime;
						timeval newtime;

						timersub(&m_tmStop, &tv, &newtime);
						nTime= calcResult(newtime, m_bSeconds);
						tout << "subroutine of timer stops " << nTime << " seconds before" << endl;
					}
					if(localtime_r(&tv.tv_sec, &l) == NULL)
						TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
					strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
					tout << "actual time (" << stime << " ";
					tout << MeasureThread::getUsecString(tv.tv_usec) << ")" << endl;
				}
			}
		}else
		{ // measure time up
			if(m_bMeasure == false)
			{ // begin measuring
				m_tmStart= tv;
				if(debug)
				{
					tout << "actual microsec : " << MeasureThread::getUsecString(tv.tv_usec) << endl;
					tout << "starting at     : " << MeasureThread::getUsecString(m_tmStart.tv_usec) << endl;
				}
				need= 0;
				m_bMeasure= true;
				if(debug)
				{
					tout << "subroutine begin to measure time in ";
					if(m_bSeconds)
						tout << "seconds";
					else
						tout << "microseconds";
					tout << endl;
				}
			}else
			{ // while or end measure

				timersub(&tv, &m_tmStart, &tv);
				need= calcResult(tv, m_bSeconds);
				if(bswitch)
				{ // while measure
					if(debug)
						tout << "actually measured time is ";
				}else
				{ // end measure

					m_bMeasure= false;
					if(debug)
						tout << "ending time measure by ";
				}
				if(debug)
				{
					tout  << dec << need << " ";
					if(!m_bSeconds)
						tout << "micro";
					tout << "seconds" << endl;
				}
			}

		}
		// bswitch was before also true
		// or some measuring was done
		bswitch= true;
	}else
	{
		if(debug)
			tout << "no measuring should be done" << endl;
		need= actValue;
	}

	if(	m_bTimeMeasure &&
		!m_oSetNull.isEmpty()	)
	{
		double res;

		if(m_oSetNull.calculate(res))
		{
			if(res > 0 || res < 0)
			{
				need= 0;
				m_bMeasure= false;
			}
		}
	}
	if(	!m_bTimeMeasure &&
		m_nDirection == -2 &&
		!m_bMeasure &&
		!bEndCount &&
		!(m_dTimeBefore > 0 || m_dTimeBefore < 0) &&
		!(need > 0 || need < 0)						)
	{
		need= -1;
	}

	if(m_bHasLinks)
	{
		if(getLinkedValue("TIMER", need, nBeginTime))
		{
			if(debug)
				tout << "result of time from linked subroutine is " << dec << need << " seconds" << endl;

		}else if(debug)
				tout << "result of time is " << dec << need << " seconds" << endl;
	}else if(debug)
		tout << "result of time is " << dec << need << " seconds" << endl;
	m_dTimeBefore= need;
	return need;
}

double timer::calcStartTime(const bool& debug, const double actValue, timeval* next)
{
	bool bneed;
	double calc, need;

	if(debug)
	{
		tout << "calculate how long ";
		if(m_nDirection > -2)
			tout << "time ";
		else
			tout << "count down ";
		tout << "should running" << endl;
	}
	//var next is for starting defined as actual value
	if(!m_bActivate)
	{
		next->tv_sec= static_cast<time_t>(actValue);
		calc= actValue - static_cast<double>(next->tv_sec);
		calc*= (1000 * 1000);
		next->tv_usec= static_cast<suseconds_t>(calc);
	}
	if(!m_omtime.isEmpty())
	{ // calculate seconds (m_tmSec) and microseconds (m_tmMicroseconds) from other subroutine
		if(m_omtime.calculate(need))
		{
			if(need > 0)
				bneed= true;
			if(!bneed)
			{
				if(debug)
					tout << "no next measure time be set, make no count down" << endl;
				need= -1;
			}else
			{
				double calc;

				// define first m_tmSec and m_tmMicroseconds from parameter mtime
				m_tmSec= static_cast<time_t>(need);
				calc= need - static_cast<double>(m_tmSec);
				calc*= (1000 * 1000);
				m_tmMicroseconds= static_cast<suseconds_t>(calc);
				//var next is for starting defined as actual value
				need= calcNextTime(/*start*/true, debug, next);
			}
		}else
		{
			string msg("cannot read time in subroutine ");

			msg+= getFolderName() + ":" + getSubroutineName();
			msg+= " from given mtime parameter " + m_omtime.getStatement();
			TIMELOG(LOG_WARNING, "mtimemeasure", msg);
			if(debug)
				tout << msg << endl;
			m_tmSec= 0;
			m_tmMicroseconds= 0;
			need= -1;
		}
	}else
	{
		int nSeconds(-1), nMinutes(-1), nHours(-1), nDays(-1), nMonths(-1), nYears(-1);
		double res;

		m_tmSec= 0;
		m_tmMicroseconds= 0;
		if(!m_oMicroseconds.isEmpty())
		{
			m_oMicroseconds.calculate(res);
			m_tmMicroseconds+= static_cast<suseconds_t>(res);
		}
		if(!m_oMilliseconds.isEmpty())
		{
			m_oMilliseconds.calculate(res);
			m_tmMicroseconds+= static_cast<suseconds_t>(res) * 1000;
		}
		if(!m_oSeconds.isEmpty())
		{
			m_oSeconds.calculate(res);
			if(m_bActivate)
				nSeconds= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res);
		}
		if(!m_oMinutes.isEmpty())
		{
			m_oMinutes.calculate(res);
			if(m_bActivate)
				nMinutes= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 60;
		}
		if(!m_oHours.isEmpty())
		{
			m_oHours.calculate(res);
			if(m_bActivate)
				nHours= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 60 * 60;
		}
		if(!m_oDays.isEmpty())
		{
			m_oDays.calculate(res);
			if(m_bActivate)
				nDays= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 24 * 60 * 60;
		}
		if(m_bActivate)
		{
			if(!m_oMonths.isEmpty())
			{
				m_oMonths.calculate(res);
				nMonths= static_cast<int>(res);
			}
			if(!m_oYears.isEmpty())
			{
				m_oYears.calculate(res);
				nYears= static_cast<int>(res);
			}
			m_tmSec= Calendar::setDate(nYears, nMonths, nDays, nHours, nMinutes, nSeconds);
			m_tmSec-= next->tv_sec;
		}
		//var next is for starting defined as actual value
		need= calcNextTime(/*start*/true, debug, next);
		bneed= true;
	}
	if(bneed)
	{
		calc= calcResult(*next, m_bSeconds);
		if(calc > 0)
		{
			//m_tmSec= next->tv_sec;
			//m_tmMicroseconds= next->tv_usec;
			timeradd(&m_tmStart, next, &m_tmStop);
			getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
			m_bMeasure= true;
			if(debug)
			{
				char stime[18];
				tm l;

				calc= calcResult(*next, m_bSeconds);
				if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				strftime(stime, 16, "%Y%m%d:%H%M%S", &l);
				tout << "folder should start again in " << calc << " seconds by (" << stime << " ";
				tout << MeasureThread::getUsecString(m_tmStop.tv_usec) << ")" << endl;
			}
		}else
		{
			if(calc < 0)
			{
				ostringstream err;

				err << "calculated minus time (" << calc << ") to refresh folder by ";
				err << getFolderName() << ":" << getSubroutineName();
				TIMELOG(LOG_ALERT, getSubroutineName(), err.str());
				if(debug)
					tout << "###ERROR: " << err.str() << endl;
			}else if(debug)
				tout << "do not refresh folder, because calculated time was 0" << endl;
		}
	}
	return need;
}

double timer::calcNextTime(const bool& start, const bool& debug, timeval* actTime)
{
	double newTime, direct;
	timeval endTime;
	//time_t actSec, nextSec;
	//suseconds_t actMicrosec, nextMicrosec;

	if(debug)
	{
		tout << "-- calculating next time";
		if(start)
			tout << " for begin measuring";
		tout << endl;
	}
	if(m_nDirection == -1)
	{// direction is defined but unknown
		m_oDirect.calculate(direct);
		if(direct > 0)
			m_nDirection= 1;
		else
			m_nDirection= 0;
	}
	if(start)
	{
		if(m_nDirection == -2)
		{// starting for count down
			actTime->tv_sec= m_tmSec;
			actTime->tv_usec= m_tmMicroseconds;
			newTime= calcResult(*actTime, m_bSeconds);

		}else
		{
			endTime.tv_sec= m_tmSec;
			endTime.tv_usec= m_tmMicroseconds;
			if(timercmp(actTime, &endTime, >))
			{
				actTime->tv_sec= m_tmSec;
				actTime->tv_usec= m_tmMicroseconds;
			}
			newTime= calcResult(*actTime, m_bSeconds);
			if(newTime < 0)
			{
				newTime= 0;
				actTime->tv_sec= 0;
				actTime->tv_usec= 0;
			}
			if(m_nDirection == 1)
			{// direction is defined to calculating to full time
				timersub(&endTime, actTime, actTime);
			}// else
			 // direction is defined to calculating to 0 or count down
			 // actTime has the right values
		}
		if(debug)
		{
			tout << "end time should be in " << calcResult(*actTime, m_bSeconds) << " seconds"<< endl;
			tout << "measured actual time is " << newTime << " seconds" << endl;
		}
		return newTime;
	}
	endTime.tv_sec= m_tmSec;
	endTime.tv_usec= m_tmMicroseconds;
	if(m_nDirection == 1)
	{//measure up to full time
		timersub(actTime, &m_tmStart, &endTime);

	}else
	{// direction is defined to calculating to 0 or make count down
		timersub(&m_tmStop, actTime, &endTime);
	}
	newTime= calcResult(endTime, m_bSeconds);
	if(debug)
		tout << "measured actual time is " << newTime << endl;
	if(m_nDirection == 1)
		newTime+= m_dStartValue;
	return newTime;

#if 0
	if(direction > -2)
	{
		actSec= static_cast<time_t>(actValue);
		newTime= actValue;
		newTime-= static_cast<double>(actSec);
		newTime*= (1000 * 1000);
		actMicrosec= static_cast<suseconds_t>(newTime);
		if(debug)
			tout << "     splitted in " << actSec << " " << actMicrosec << endl;
	}
	if(debug)
		tout << "    new need value is " << need << endl;
	nextSec= static_cast<time_t>(need);
	need-= static_cast<double>(nextSec);
	need*= (1000 * 1000);
	nextMicrosec= static_cast<suseconds_t>(need);
	if(debug)
		tout << "     splitted in " << nextSec << " " << nextMicrosec << endl;
	if(	m_bSeconds &&
		nextMicrosec > 0	)
	{
		string msg("parameter mtime has microseconds, but subroutine ");

		msg+= getSubroutineName() + " defined in folder " + getFolderName();
		msg+= " has no action micro. Do not measure now in microseconds.";
		nextMicrosec= 0;
		actMicrosec= 0;
		TIMELOG(LOG_WARNING, getSubroutineName()+":"+getFolderName(), msg);
		if(debug)
			tout << "### WARNING: " << msg << endl;
	}
	//m_bSeconds= m_tmMicroseconds == 0 ? true : false;
	endTime.tv_sec= nextSec;
	endTime.tv_usec= nextMicrosec;
	if(debug)
		tout << "     set to struct " << endTime.tv_sec << " " << endTime.tv_usec << endl;
	if(direction > -2)
	{
		double direct;
		timeval minus;

		minus.tv_sec= actSec;
		minus.tv_usec= actMicrosec;
		if(debug)
			tout << "    minus actual is " << minus.tv_sec << " " << minus.tv_usec << endl;
		timersub(&endTime, &minus, &minus);
		if(debug)
			tout << "    next - minus is " << minus.tv_sec << " " << minus.tv_usec << endl;

		if(debug)
			tout << "time should running ";
		if(direction > 0)
		{// count up to mtime
			endTime.tv_sec= minus.tv_sec;
			endTime.tv_usec= minus.tv_usec;
			if(debug)
				tout << "    next is now " << endTime.tv_sec << " " << endTime.tv_usec << endl;
			if(debug)
				tout << "up ";
			//m_tmSec= actSec;
			//m_tmMicroseconds= actMicrosec;

		}else
		{// count down to 0
			timersub(&endTime, &minus, &endTime);
			if(debug)
				tout << "down ";
			//m_tmSec-= actSec;
			//m_tmMicroseconds-= actMicrosec;
		}
	}else if(debug)
		tout << "count down should running ";
	if(debug)
	{
		tout << "for " << endTime.tv_sec;
		tout << "." << MeasureThread::getUsecString(endTime.tv_usec);
		tout << " seconds" << endl;
	}
	//next.tv_sec= m_tmSec;
	//next.tv_usec= m_tmMicroseconds;
	if(debug)
		tout << "-------------------------------------------------------" << endl;
	return endTime;
#endif
}

double timer::getValue(const string& who)
{
	long nval;
	double val(switchClass::getValue(who));

	// secure method to calculate only with seconds
	// or microseconds and no more decimal places behind
	if(!m_bSeconds)
		val*= (1000 * 1000);
	nval= static_cast<long>(val);
	val= static_cast<double>(nval);
	if(!m_bSeconds)
		val/= (1000 * 1000);
	return val;
}

#if 0
void timer::setValue(double value, const string& from)
{
	if(	!m_bTimeMeasure &&
		value > 0 &&
		from != "i:"+getFolderName()+":"+getSubroutineName()	)
	{

		next.tv_sec= m_tmSec;
		next.tv_usec= m_tmMicroseconds;
		timeradd(&m_tmStart, &next, &m_tmStart);
		need= calcResult(next);
		getRunningThread()->nextActivateTime(getFolderName(), m_tmStart);
	}
}
#endif

void timer::setDebug(bool bDebug)
{
	m_omtime.doOutput(bDebug);
	m_oSetNull.doOutput(bDebug);
	m_oDirect.doOutput(bDebug);
	switchClass::setDebug(bDebug);
}

double timer::calcResult(timeval tv, bool secondcalc)
{
	long val;
	double dRv;

	if(!secondcalc)
	{
		dRv= static_cast<double>(tv.tv_usec) / (1000 * 1000);
		dRv+= static_cast<double>(tv.tv_sec);
	}else
		dRv= static_cast<double>(tv.tv_sec);
	// secure method to calculate only with seconds
	// or microseconds and no more decimal places behind
	if(!secondcalc)
		dRv*= (1000 * 1000);
	val= static_cast<long>(dRv);
	dRv= static_cast<double>(val);
	if(!secondcalc)
		dRv/= (1000 * 1000);
	return dRv;
}

timer::~timer()
{

}
