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

#include "../database/lib/DbInterface.h"


using namespace util;
using namespace ppi_database;

bool timer::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bBegin, bWhile, bEnd, bOk= true, bAlwaysBegin= false, bTimeMeasure= false;
	double dDefault;
	string prop, smtime, sSetNull;

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "display_settings" &&
		getSubroutineName() == "activate"					)
	{
		cout << getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	m_nCaseNr= 0;
	m_bSeconds= true;
	m_tmSec= 0;
	m_tmMicroseconds= 0;
	m_tmStart.tv_sec= 0;
	m_tmStart.tv_usec= 0;
	m_tmStop.tv_sec= 0;
	m_tmStop.tv_usec= 0;

	// -----------------------------------------------------------------------
	// case 1: folder should polling all seconds, minutes, hours, ...
	if(properties->haveAction("seconds"))
		m_eWhich= seconds;
	else if(properties->haveAction("minutes"))
		m_eWhich= minutes;
	else if(properties->haveAction("hours"))
		m_eWhich= hours;
	else if(properties->haveAction("days"))
		m_eWhich= days;
	else if(properties->haveAction("months"))
		m_eWhich= months;
	else if(properties->haveAction("years"))
		m_eWhich= years;
	else
		m_eWhich= notype;

	if(m_eWhich != notype)
		m_nCaseNr= 1;

	if(	m_nCaseNr == 0 &&
		properties->haveAction("activate")	)
	{
		// -----------------------------------------------------------------------
		// case 2: time count down to setting date time
		m_nCaseNr= 2;
		m_oYears.init(pStartFolder, properties->getValue("year", /*warning*/false));
		if(!m_oYears.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
		m_oMonths.init(pStartFolder, properties->getValue("month", /*warning*/false));
		if(!m_oMonths.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
		m_oDays.init(pStartFolder, properties->getValue("day", /*warning*/false));
		if(!m_oDays.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
		m_oHours.init(pStartFolder, properties->getValue("hour", /*warning*/false));
		if(!m_oHours.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
		m_oMinutes.init(pStartFolder, properties->getValue("min", /*warning*/false));
		if(!m_oMinutes.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
		m_oSeconds.init(pStartFolder, properties->getValue("sec", /*warning*/false));
		if(!m_oSeconds.isEmpty())
			m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
	}
	if(m_nCaseNr == 0)
	{
		bTimeMeasure= true;// will be the case of 5
		m_bSeconds= true;//default
		if(!properties->haveAction("sec"))
			if(properties->haveAction("micro"))
				m_bSeconds= false;

		// -----------------------------------------------------------------------
		// case 3 or 4: count the time down to 0, or up to full time
		// case 5: measure time inside case of begin/while/end
		smtime= properties->getValue("mtime", /*warning*/false);
		m_omtime.init(pStartFolder, smtime);
		if(m_omtime.isEmpty())
		{
			if(m_bSeconds)
			{
				m_oYears.init(pStartFolder, properties->getValue("year", /*warning*/false));
				if(!m_oYears.isEmpty())
				{
					m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
					bTimeMeasure= false;
				}
				m_oMonths.init(pStartFolder, properties->getValue("month", /*warning*/false));
				if(!m_oMonths.isEmpty())
				{
					m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
					bTimeMeasure= false;
				}
			}
			m_oDays.init(pStartFolder, properties->getValue("day", /*warning*/false));
			if(!m_oDays.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				bTimeMeasure= false;
			}
			m_oHours.init(pStartFolder, properties->getValue("hour", /*warning*/false));
			if(!m_oHours.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				bTimeMeasure= false;
			}
			m_oMinutes.init(pStartFolder, properties->getValue("min", /*warning*/false));
			if(!m_oMinutes.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				bTimeMeasure= false;
			}
			m_oSeconds.init(pStartFolder, properties->getValue("sec", /*warning*/false));
			if(!m_oSeconds.isEmpty())
			{
				m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
				bTimeMeasure= false;
			}
			if(!m_bSeconds)
			{
				m_oMilliseconds.init(pStartFolder, properties->getValue("millisec", /*warning*/false));
				if(!m_oMilliseconds.isEmpty())
				{
					m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
					bTimeMeasure= false;
				}
				m_oMicroseconds.init(pStartFolder, properties->getValue("microsec", /*warning*/false));
				if(!m_oMicroseconds.isEmpty())
				{
					m_tmSec= 1;// calculate time from parameter millisec, sec, min and so on
					bTimeMeasure= false;
				}
			}
		}else
			bTimeMeasure= false;

		if(!bTimeMeasure)
		{
			// -----------------------------------------------------------------------
			// case 3 or 4: count the time down to 0, or up to full time
			m_bPoll= properties->haveAction("poll");
			smtime= properties->getValue("direction", /*warning*/false);
			m_oDirect.init(pStartFolder, smtime);
			if(m_oDirect.isEmpty())
			{
				// -----------------------------------------------------------------------
				// case 3: count the time down to 0
				m_nCaseNr= 3;
				m_nDirection= -2;
				bAlwaysBegin= properties->haveAction("alwaysbegin");

			}else
			{
				// -----------------------------------------------------------------------
				// case 4: count the time down to 0, or up to full time
				m_nCaseNr= 4;
				m_nDirection= -1;
			}
		}else
		{
			// -----------------------------------------------------------------------
			// case 5: measure time inside case of begin/while/end
			m_nCaseNr= 5;
			sSetNull= properties->getValue("setnull", /*warning*/false);
			m_oSetNull.init(pStartFolder, sSetNull);
		}
	}

	// check whether has begin, while or end parameters
	bBegin= false;
	bWhile= false;
	bEnd= false;
	if(properties->getValue("begin", /*warning*/false) != "")
	{
		bBegin= true;
		m_bSwitchbyTime= true;
	}
	if(properties->getValue("while", /*warning*/false) != "")
	{
		bWhile= true;
		m_bSwitchbyTime= true;
	}
	if(properties->getValue("end", /*warning*/false) != "")
	{
		bEnd= true;
		m_bSwitchbyTime= true;
	}
	m_bExactTime= false;
	m_bFinished= true;
	if(	m_nCaseNr == 3 ||	// case 3: count the time down to 0
		m_nCaseNr == 4	)	// case 4: count the time down to 0, or up to full time
	{
		string folder(getFolderName()), subroutine(getSubroutineName());
		DbInterface *db= DbInterface::instance();

		m_bExactTime= properties->haveAction("exact");
		if(	m_bExactTime &&
			m_bLogPercent	)
		{
			db->writeIntoDb("folder", folder, "runpercent");
			db->writeIntoDb(folder, subroutine, "startlate");
			db->fillValue("folder", folder, "runpercent", 0, /*new*/true);
			db->fillValue("folder", folder, "startlate", 0, /*new*/true);
			m_tReachedTypes.runlength= false;
			m_tReachedTypes.folder= folder;
			m_tReachedTypes.subroutine= subroutine;
			m_tReachedTypes.maxVal= 5;
			m_tReachedTypes.inPercent= m_nFinishedCPUtime;
		}
		if(	m_bExactTime &&
			m_nCaseNr == 3	) // case 3: count the time down to 0
		{
			smtime= properties->getValue("finished", /*warning*/false);
			if(smtime != "")
			{
				if(m_bLogPercent)
				{
					db->writeIntoDb(folder, subroutine, "reachpercent");
					db->writeIntoDb(folder, subroutine, "reachlate");
					db->writeIntoDb(folder, subroutine, "wrongreach");
					db->fillValue(folder, subroutine, "reachpercent", 0, /*new*/true);
					db->fillValue(folder, subroutine, "reachlate", 0, /*new*/true);
					db->fillValue(folder, subroutine, "wrongreach", 0, /*new*/true);
				}
				properties->notAllowedAction("noinfo");
				m_oFinished.init(pStartFolder, smtime);
				for(short n= m_nFinishedCPUtime; n <= 100; n+= m_nFinishedCPUtime)
				{
					bool exist;
					ostringstream dbstr, maxcount;

					dbstr << "reachend";
					maxcount << "maxcount";
					if(m_nFinishedCPUtime < 100)
					{
						dbstr << n;
						maxcount << n;
					}
					db->writeIntoDb(folder, subroutine, dbstr.str());
					db->writeIntoDb(folder, subroutine, maxcount.str());
					if(!m_bNoDbRead)
					{
						dDefault= db->getActEntry(exist, folder, subroutine, dbstr.str());
						if(	exist &&
							dDefault > 0	)
						{
							m_tReachedTypes.percentDiff[n].actValue= dDefault;
							m_tReachedTypes.percentDiff[n].reachedPercent[0]= pair<short, double>(1, dDefault);
							dDefault= db->getActEntry(exist, folder, subroutine, maxcount.str());
							if(!exist)
								dDefault= 1;
							m_tReachedTypes.percentDiff[n].maxCount= static_cast<short>(dDefault);
							m_tReachedTypes.percentDiff[n].stype= dbstr.str();
							m_tReachedTypes.percentDiff[n].scount= maxcount.str();
						}
					}else
					{
						db->fillValue(folder, subroutine, dbstr.str(), 0, /*new*/true);
						db->fillValue(folder, subroutine, maxcount.str(), 0, /*new*/true);
					}
				}
			}
		}
	}
	if(	(bBegin && bEnd) ||
		bWhile				)
	{
		properties->notAllowedAction("binary");
		if(!switchClass::init(properties, pStartFolder, bAlwaysBegin))
			bOk= false;

		prop= "default";
		dDefault= properties->getDouble(prop, /*warning*/false);
		if(prop == "#ERROR")
		{// no default be set in configuration files for subroutine
			if(	m_nCaseNr != 5 && // do not measure time inside case of begin/while/end
				m_oDirect.isEmpty()	)
			{
				setValue(-1, "i:"+getFolderName()+":"+getSubroutineName());
				m_dTimeBefore= -1;

			}//else default value is 0
		}else
			m_dTimeBefore= dDefault;

	}else
	{
		if(!portBase::init(properties, pStartFolder))
			bOk= false;
		if(	m_nCaseNr == 3 ||
			m_nCaseNr == 4 ||
			m_nCaseNr == 5		)
		{
			string msg(properties->getMsgHead(/*error*/true));

			msg+= "no right case of begin, while, end parameters be set, so do not start subroutine";
			LOG(LOG_ERROR, msg);
			tout << msg << endl;
			bOk= false;
		}
	}

	if(properties->getPropertyCount("link"))
	{
		m_bHasLinks= true;
		if(!initLinks("TIMER", properties, pStartFolder))
			bOk= false;
	}
	return bOk;
}

void timer::setRunningThread(IMeasurePattern* thread)
{
	if(m_bExactTime)
		thread->calculateLengthTime();
	portBase::setRunningThread(thread);
}

bool timer::range(bool& bfloat, double* min, double* max)
{
	if(m_bSeconds)
		bfloat= false;
	else
		bfloat= true;
	if(	m_nCaseNr == 1 || // folder should polling all seconds, minutes, hours, ...
		m_nCaseNr == 5	) // measure time inside case of begin/while/end
		*min= 0;
	else
		*min= -1;
	*max= (double)LONG_MAX;
	return true;
}

void timer::setObserver(IMeasurePattern* observer)
{
	string subroutine(getSubroutineName());
	vector<string> vars;
	IMeasurePattern* measureThread= getRunningThread();

	if(!m_oFinished.isEmpty())
	{
		m_oFinished.activateObserver(observer);
		vars= m_oFinished.getVariables();
		for(vector<string>::iterator it= vars.begin(); it != vars.end(); ++it)
			measureThread->needChangingTime(subroutine, *it);
	}
	if(!m_oBegin.isEmpty())
	{
		vars= m_oBegin.getVariables();
		for(vector<string>::iterator it= vars.begin(); it != vars.end(); ++it)
			measureThread->needChangingTime(subroutine + "+BEGIN", *it);
	}
	if(!m_oWhile.isEmpty())
	{
		vars= m_oWhile.getVariables();
		for(vector<string>::iterator it= vars.begin(); it != vars.end(); ++it)
			measureThread->needChangingTime(subroutine + "+WHILE", *it);
	}
	if(!m_oEnd.isEmpty())
	{
		vars= m_oEnd.getVariables();
		for(vector<string>::iterator it= vars.begin(); it != vars.end(); ++it)
			measureThread->needChangingTime(subroutine + "+END", *it);
	}
	switchClass::setObserver(observer);
}

double timer::measure(const double actValue)
{
	bool bswitch;
	bool bEndCount(false);
	bool debug= isDebug();
	double need, nBeginTime(0);
	string subroutine(getSubroutineName()), folder(getFolderName());
	timeval tv;
	switchClass::setting set= NONE;

	//Debug info to stop by right subroutine
	/*if(	folder == "display_settings" &&
		subroutine == "activate"					)
	{
		cout << folder << ":" << subroutine << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	if(gettimeofday(&tv, NULL))
	{
		string msg("ERROR: cannot get time of day,\n");

		msg+= "       so cannot measure time for TIMER function in folder ";
		msg+= folder + " and subroutine " + subroutine + ".";
		TIMELOG(LOG_ALERT, "gettimeofday", msg);
		if(debug)
			tout << msg << endl;
		if(m_nCaseNr == 5) // measure time inside case of begin/while/end
			need= 0;
		else
			need= -1;
		return need;
	}
	if(m_bExactTime)
		getRunningThread()->setCpuMeasureBegin(&m_tReachedTypes);
	if(!m_bFinished)
	{// m_bFinished is only false when waiting for other subroutines in case 3
		if(debug)
			tout << "subroutine wait only for finishing of other subroutines" << endl;
		m_oFinished.calculate(need);
		if(need != 0)
		{
			string chtime;
			timeval changed;

			if(debug)
				chtime= "FINISHED";
			m_bFinished= true;
			changed= getRunningThread()->getMaxChangingTime(subroutine, chtime);
			if(	debug ||
				m_bLogPercent	)
			{
				bool less, wless;
				timeval diff, wrong;

				if(debug)
				{
					tout << "   start time: " << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << endl;
					tout << "     end time: " << MeasureThread::getTimevalString(m_tmExactStop, /*as date*/true, debug) << endl;
					tout << "real finished: " << MeasureThread::getTimevalString(changed, /*as date*/true, debug) << endl;
					timersub(&changed, &m_tmStart, &diff);
				}
				if(timercmp(&m_tmWantFinish, &changed, >))
				{
					timersub(&m_tmWantFinish, &changed, &wrong);
					wless= true;
				}else
				{
					timersub(&changed, &m_tmWantFinish, &wrong);
					wless= false;
				}
				if(timercmp(&m_tmExactStop, &changed, >))
				{
					timersub(&m_tmExactStop, &changed, &changed);
					less= true;
				}else
				{
					timersub(&changed, &m_tmExactStop, &changed);
					less= false;
				}
				if(debug)
				{
					tout << "          need " << MeasureThread::getTimevalString(diff, /*as date*/false, debug) << " seconds "<< endl;
					tout << "          need " << MeasureThread::getTimevalString(changed, /*as date*/false, debug);
					if(less)
						tout << " less seconds" << endl;
					else
						tout << " more seconds " << endl;
					tout << "          need " << MeasureThread::getTimevalString(wrong, /*as date*/false, debug);
					if(wless)
						tout << " less than want" << endl;
					else
						tout << " more than want" << endl;
				}
				if(m_bLogPercent)
				{
					double seconds(MeasureThread::calcResult(changed, /*seconds*/false));
					DbInterface *db= DbInterface::instance();

					if(less)
						seconds*= -1;
					db->fillValue(folder, subroutine, "reachlate", seconds, /*new*/true);
					seconds= MeasureThread::calcResult(wrong, /*seconds*/false);
					if(wless)
						seconds*= -1;
					db->fillValue(folder, subroutine, "wrongreach", seconds, /*new*/true);
				}
			}else
				timersub(&changed, &m_tmExactStop, &changed);// <- timersub made also inside debug mode
			MeasureThread::calcLengthDiff(&m_tReachedTypes, changed, debug);
		}else
			return 0;
	}
	if(debug)
	{
		tout << "routine running in case of " << m_nCaseNr << " (";
		switch(m_nCaseNr)
		{
		case 1:
			tout << "folder polling all ";
			switch(m_eWhich)
			{
			case seconds:
				tout << "seconds";
				break;
			case minutes:
				tout << "minutes";
				break;
			case hours:
				tout << "hours";
				break;
			case days:
				tout << "days";
				break;
			case months:
				tout << "months";
				break;
			case years:
				tout << "years";
				break;
			default:
				tout << "(no correct ACTION be set)";
				break;
			}
			break;
		case 2:
			char timeString[21];

			strftime(timeString, 20, "%Y.%m.%d %H:%M:%S", gmtime(&m_tmStop.tv_sec));
			tout << "time count down to setting date time " << timeString;
			break;
		case 3:
			tout << "count the time down to 0";
			break;
		case 4:
			tout << "count the time down to 0, or up to full time";
			break;
		case 5:
			tout << "measure time inside case of begin/while/end";
			break;
		}
		tout << ")" << endl;
	}
	if(	m_bSwitchbyTime )
	{
		m_dSwitch= switchClass::measure(m_dSwitch, set);

	}else if(debug)
	{
		tout << "WHILE parameter from measured before is ";
		if(m_dSwitch > 0)
			tout << "true" << endl;
		else
			tout << "false" << endl;
	}
	if(m_dSwitch > 0)
		bswitch= true;
	else
		bswitch= false;
	if(	m_nCaseNr == 1 || // folder should polling all seconds, minutes, hours, ...
		m_nCaseNr == 2	) // time count down to setting date time
	{
		return polling_or_countDown(bswitch, tv, debug);
	}
	// case 3: count the time down to 0
	// case 4: count the time down to 0, or up to full time
	// case 5: measure time inside case of begin/while/end
	if(	bswitch ||
		m_bMeasure	)
	{
		if(m_nCaseNr != 5) // do not measure time inside case of begin/while/end
		{ // case 3: count the time down to 0
		  // case 4: count the time down to 0, or up to full time
			bool bNewDirection= false;
			timeval next;

			if(	bswitch &&
				m_nCaseNr == 4	) // count the time down to 0, or up to full time
			{
				double direct;
				short oldDirection= m_nDirection;

				m_oDirect.calculate(direct);
				if(direct > 0)
					m_nDirection= 1;
				else
					m_nDirection= 0;
				if(	m_nDirection != oldDirection &&
					oldDirection > -1				)
				{
					bNewDirection= true;
				}
			}
			if(	m_bMeasure == false ||
				(	bNewDirection &&
					bswitch			) ||
				(	bswitch &&
					set == BEGIN		)	)
			{// BEGIN to measure

				string identifier(subroutine), chtime;

				if(debug)
				{
					tout << "BEGIN time measuring ";
					if(m_nDirection > -2)
					{
						tout << "to specific time";
						if(	bswitch &&
							set == BEGIN	)
						{
							tout << " because new begin was calculated";

						}else if(bNewDirection)
							tout << " because direction of count was changed";
						tout << endl;
					}else
						tout << "for count down" << endl;
				}
				switch(set)
				{
				case BEGIN:
					identifier+= "+BEGIN";
					if(debug)
						chtime= "BEGIN to start";
					break;
				case WHILE:
					identifier+= "+WHILE";
					if(debug)
						chtime= "WHILE to start";
					break;
				default:// get always unset time from getMaxChangingTime()
					identifier+= "+wrong";
					if(set == END)
						identifier+= "_END";
					if(debug)
						chtime= "(wrong definition inside timer.cpp) for start";
					break;
				}
				next= getRunningThread()->getMaxChangingTime(identifier, chtime);
				if(!timerisset(&next))
				{
					if(debug)
						tout << "get no starting time (from " << identifier
								<< ") take TIMER subroutine start" << endl;
					next= tv;
				}
				if(debug)
				{
					tout << "start at time " << MeasureThread::getTimevalString(next, /*as date*/true, debug) << endl;
					tout << "  actual time " << MeasureThread::getTimevalString(tv, /*as date*/true, debug) << endl;
				}
				m_tmStart= next;
				m_dStartValue= actValue;
				need= calcStartTime(debug, actValue, &tv);
			/* alex 29/8/2013
			 * set m_bMeasure now inside calcStartTime()
				if(need == -1)
				{
					if(m_nDirection > -2)
						need= actValue;
					m_bMeasure= false;
				}else
					m_bMeasure= true;*/
				if(	need == -1 &&
					m_nDirection > -2	)
				{
					need= actValue;
				}

			}else if( timercmp(&m_tmStop, &tv, <) )
			{ // reaching end of count down
			  // now end polling, or begin with new time
				timeval was;

				if(	m_bExactTime)
				{
					timeval tv2;

					tv2= tv;
					if(gettimeofday(&tv, NULL))
					{
						string msg("ERROR: cannot get time of day,\n");

						msg+= "       cannot measure second time for TIMER function in folder ";
						msg+= folder + " and subroutine " + subroutine + ".\n";
						msg+= "       so take first time measure on begin of subroutine";
						TIMELOG(LOG_ALERT, "getsecondtimeofday", msg);
						if(debug)
							tout << msg << endl;
						tv= tv2;
					}
					if(timercmp(&m_tmExactStop, &tv, >))
					{// waiting microseconds/seconds for exact time
						timeval tvWait;

						timersub(&m_tmExactStop, &tv, &tvWait);
						if(debug)
						{
							tout << "subroutine waiting ";
							tout << MeasureThread::getTimevalString(tvWait, /*as date*/false, debug);
							tout << " seconds for exact time" << endl;
						}
						if(!getRunningThread()->usleep(tvWait))
							return 0;// folder should be stopping
						tv.tv_sec= m_tmExactStop.tv_sec;
						tv.tv_usec= m_tmExactStop.tv_usec;

					}else
					{
						if(	debug ||
							m_bLogPercent	)
						{
							timeval tvWait;

							timersub(&tv, &m_tmExactStop, &tvWait);
							if(debug)
							{
								tout << "subroutine do not need to wait for exact time," << endl;
								tout << " because exact time was overrun for ";
								tout << MeasureThread::getTimevalString(tvWait, /*as date*/false, debug);
								tout << " seconds" << endl;
							}
							if(m_bLogPercent)
							{
								DbInterface *db= DbInterface::instance();

								db->fillValue(folder, subroutine, "startlate",
												MeasureThread::calcResult(tvWait, /*seconds*/false));
							}
						}
						m_tmExactStop.tv_sec= tv.tv_sec;
						m_tmExactStop.tv_usec= tv.tv_usec;
					}
				}
				if(debug)
				{
					timeval diff, tv2;

					tv2= tv;
					if(gettimeofday(&tv, NULL))
					{
						tv= tv2;
					}

					timersub(&m_tmExactStop, &m_tmStart, &diff);
					was.tv_sec= m_tmSec;
					was.tv_usec= m_tmMicroseconds;
					tout << "reach END of time measuring" << endl;
					tout << "folder was refreshed because time of " << MeasureThread::calcResult(was, m_bSeconds);
					tout << " seconds was reached" << endl;
					tout << "  start time (" << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << ")" << endl;
					tout << "refresh time (" << MeasureThread::getTimevalString(m_tmStop, /*as date*/true, debug) << ")" << endl;
					tout << "  actual end (" << MeasureThread::getTimevalString(tv, /*as date*/true, debug) << ")" << endl;
					tout << "    want end (" << MeasureThread::getTimevalString(m_tmExactStop, /*as date*/true, debug) << ")" << endl;
					tout << "         need " << MeasureThread::getTimevalString(diff, /*as date*/false, debug) << " seconds "<< endl;
				}
				if(	m_bPoll &&
					m_oFinished.isEmpty()	)
				{
					if(m_nDirection == 0)
						need= 0;
					else
						need= calcNextTime(/*start*/false, debug, &tv);
					if(debug)
						tout << "look whether should polling time again" << endl;
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
							need= MeasureThread::calcResult(was, m_bSeconds);
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

					}else
					{
						m_bMeasure= false;
						bEndCount= true;
						if(m_nDirection == -2)
							need= 0;
					}
				}else
				{
					if(!m_oFinished.isEmpty())
						m_bFinished= false;
					m_bMeasure= false;
					bEndCount= true;
					if(m_nDirection == 1)
					{
						was.tv_sec= m_tmSec;
						was.tv_usec= m_tmMicroseconds;
						need= MeasureThread::calcResult(was, m_bSeconds);
					}else
						need= 0;
				}

			}else
			{ // count down is running
				switchClass::setting oldSet(set);

				if(debug)
				{
					timeval newtime;

					timersub(&m_tmStop, &tv, &newtime);
					tout << endl;
				}
				if(bswitch)
				{
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
				}
				if(!bswitch)
				{
					string identifier(subroutine), chtime;

					// reach END of time measuring before finished
					// erase folder starting because folder run in this case needless
					getRunningThread()->eraseActivateTime(folder, m_tmStop);
					m_bMeasure= false;
					switch(oldSet)
					{
					case WHILE:
						identifier+= "+WHILE";
						if(debug)
							chtime= "WHILE for ending";
						break;
					case END:
						identifier+= "+END";
						if(debug)
							chtime= "END for ending";
						break;
					default:// get always unset time from getMaxChangingTime()
						identifier+= "+wrong";
						if(oldSet == BEGIN)
							identifier+= "_BEGIN";
						if(debug)
							chtime= "(wrong definition inside timer.cpp) for ending";
						break;
					}
					next= getRunningThread()->getMaxChangingTime(identifier, chtime);
					if(debug)
						tout << "get for identifier '" << identifier << "' " <<
							MeasureThread::getTimevalString(next, /*as date*/true, debug) << " seconds" << endl;
					if(timerisset(&next))
						tv= next;
					else if(debug)
							tout << "get no ending time (from " << identifier
								<< ") take TIMER subroutine start" << endl;
					if(m_nDirection == -2)
						need= 0;
					else
						need= calcNextTime(/*start*/false, debug, &tv);
					if(debug)
						tout << "get for identifier '" << identifier << "' " <<
							MeasureThread::getTimevalString(tv, /*as date*/true, debug) << " seconds" << endl;
				}else
				{
					if(	m_bExactTime &&
						m_tReachedTypes.inPercent < 100	)
					{
						timeval refreshTime;

						if(debug)
						{
							tout << "check whether exact stop ";
							if(!m_oFinished.isEmpty())
								tout << "and or reach finish ";
							tout << "will be changed since starting" << endl;
						}
						timersub(&m_tmWantFinish, &m_tmStart, &next);
						substractExactFinishTime(&next, &refreshTime, &m_tmExactStop, debug);
						if(timercmp(&m_tmStop, &refreshTime, !=))
						{
							getRunningThread()->changeActivationTime(folder, m_tmStop, refreshTime);
							m_tmStop= refreshTime;
							if(debug)
								tout << "changing of folder refresh time will be done" << endl;

						}else if(debug)
							tout << "no changing have to do" << endl;
					}
					need= calcNextTime(/*start*/false, debug, &tv);
				}
				if(debug)
				{
					if(bswitch)
					{
						tout << "WHILE: measuring of time to specific end time is running" << endl;
						tout << "routine should stop at " << MeasureThread::getTimevalString(m_tmExactStop, /*as date*/true, debug) << endl;
						tout << "      actually we have " << MeasureThread::getTimevalString(tv, /*as date*/true, debug) << endl;
						tout << "folder should start again in " << need << " seconds" << endl;
						tout << "    by time (" << MeasureThread::getTimevalString(m_tmStop, /*as date*/true, debug) << ")" << endl;
					}else
					{
						timeval newtime;

						timersub(&m_tmExactStop, &tv, &newtime);
						tout << "WHILE: reach END of time measuring before finished" << endl;
						tout << "subroutine of timer stops " << MeasureThread::getTimevalString(newtime, /*as date*/false, debug) << " seconds before" << endl;
						tout << "  start time (" << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << ")" << endl;
						tout << "    end time (" << MeasureThread::getTimevalString(tv, /*as date*/true, debug) << ")" << endl;
						timersub(&tv, &m_tmStart, &newtime);
						tout << "         need " << MeasureThread::getTimevalString(newtime, /*as date*/false, debug) << " seconds "<< endl;
					}
				}
			}
		}else
		{ // m_nCaseNr is 5: measure time inside case of begin/while/end
			if(m_bMeasure == false)
			{ // begin measuring
				m_tmStart= tv;
				if(debug)
				{
					tout << "actual time : " << MeasureThread::getTimevalString(tv, /*as date*/true, debug) << endl;
					tout << "starting at : " << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << endl;
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
				need= MeasureThread::calcResult(tv, m_bSeconds);
				m_dSwitch= switchClass::measure(m_dSwitch, set, &need);
				if(m_dSwitch > 0)
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

	if(	m_nCaseNr == 5 && // measure time inside case of begin/while/end
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
	if(	m_nCaseNr != 5 && // do not measure time inside case of begin/while/end
		m_nCaseNr != 4 && // do not count the time down to 0, or up to full time
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

double timer::polling_or_countDown(const bool bswitch, timeval tv, const bool debug)
{
	double need= -1;
	time_t actTime, thisTime;
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
			tout << "subroutine is this time inside begin/while/end not defined for measure" << endl;
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
		thisTime= local.tm_min;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextMinute(actTime, &local);
		actTime= thisTime;
		break;
	case hours:
		if(debug)
			tout << "hours" << endl;
		thisTime= local.tm_hour;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextHour(actTime, &local);
		actTime= thisTime;
		break;
	case days:
		if(debug)
			tout << "days" << endl;
		thisTime= local.tm_mday;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextDay(actTime, &local);
		actTime= thisTime;
		break;
	case months:
		if(debug)
			tout << "months" << endl;
		thisTime= local.tm_mon + 1;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextMonth(actTime, &local);
		actTime= thisTime;
		break;
	case years:
		if(debug)
			tout << "years" << endl;
		thisTime= local.tm_year + 1900;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextYear(actTime, &local);
		actTime= thisTime;
		break;
	}
	if(debug)
	{
		tm l;

		if(localtime_r(&tv.tv_sec, &l) == NULL)
		{
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct local time");
		}
		tout << "               actual time is " << asctime(&l);
		if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
		{
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct local time");
		}
		tout << "folder should be refreshed at " << asctime(&l);
	}
	// toDo: measure to correct time
	getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
	return static_cast<double>(actTime);
}

double timer::calcStartTime(const bool& debug, const double actValue, timeval* next)
{
	bool bneed(false);
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
	if(m_nCaseNr != 2)
	{ // when subroutine do not count down time to setting date time
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
			}
		}else
		{
			string msg("cannot read time in subroutine ");

			msg+= getFolderName() + ":" + getSubroutineName();
			msg+= " from given mtime parameter " + m_omtime.getStatement();
			TIMELOG(LOG_WARNING, "mtimemeasure", msg);
			if(debug)
				tout << msg << endl;
			next->tv_sec= 0;
			next->tv_usec= 0;
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
			if(m_nCaseNr == 2)
				nSeconds= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res);
		}
		if(!m_oMinutes.isEmpty())
		{
			m_oMinutes.calculate(res);
			if(m_nCaseNr == 2)
				nMinutes= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 60;
		}
		if(!m_oHours.isEmpty())
		{
			m_oHours.calculate(res);
			if(m_nCaseNr == 2)
				nHours= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 60 * 60;
		}
		if(!m_oDays.isEmpty())
		{
			m_oDays.calculate(res);
			if(m_nCaseNr == 2)
				nDays= static_cast<int>(res);
			else
				m_tmSec+= static_cast<time_t>(res) * 24 * 60 * 60;
		}
		if(m_nCaseNr == 2)
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
		bneed= true;
	}
	m_bMeasure= false;
	if(bneed)
	{
		//var next is for starting defined as actual value
		need= calcNextTime(/*start*/true, debug, next);
		if(m_bExactTime)
			calc= substractExactFinishTime(next, &m_tmStop, &m_tmExactStop, debug);
		else
		{
			calc= MeasureThread::calcResult(*next, m_bSeconds);
			timeradd(&m_tmStart, next, &m_tmExactStop);
			m_tmStop= m_tmExactStop;
		}
		if(calc > 0)
		{
			getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
			m_bMeasure= true;
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
			}else
			{
				if(debug)
					tout << "do not refresh folder, because calculated time was 0" << endl;
			}
			m_dSwitch= 0;
			m_bMeasure= false;
			need= actValue;
		}
	}
	return need;
}

double timer::substractExactFinishTime(timeval* nextTime, timeval* refreshTime, timeval* exactStop, const bool& debug)
{
	timeval tvLength, finishedNext, tmReachEnd;

	if(!m_oFinished.isEmpty())
	{

		timeradd(&m_tmStart, nextTime, &m_tmWantFinish);
		tmReachEnd= MeasureThread::getLengthedTime(m_tReachedTypes, &m_nLengthPercent,
													m_bLogPercent, debug);
		if(debug)
		{
			finishedNext.tv_sec= nextTime->tv_sec;
			finishedNext.tv_usec= nextTime->tv_usec;
		}
		if(timerisset(&tmReachEnd))
		{
			// subtract calculated finished time
			if(timercmp(nextTime, &tmReachEnd, >))
			{
				timersub(nextTime, &tmReachEnd, nextTime);

			}else
			{
				nextTime->tv_sec= 0;
				nextTime->tv_usec= 000001;
			}
			if(debug)
			{
				tout << "      - calculating   " << MeasureThread::calcResult(tmReachEnd, /*seconds*/false)
						<< " seconds maybe should finished" << endl;
			}
		}
	}

	timeradd(&m_tmStart, nextTime, exactStop);
	// Subtract length of folder run, because subroutine should reached before
	// to wait inside this subroutine for exact time
	tvLength= getRunningThread()->getLengthedTime(m_bLogPercent, debug);
	if(timercmp(nextTime, &tvLength, >))
	{
		timersub(exactStop, &tvLength, refreshTime);
		timersub(nextTime, &tvLength, nextTime);

	}else
	{
		nextTime->tv_sec= 0;
		nextTime->tv_usec= 000001;
		timeradd(&m_tmStart, nextTime, refreshTime);
	}
	if(debug)
	{
		tout << "      - folder length " << MeasureThread::getTimevalString(tvLength, /*as date*/false, debug)
				<< " seconds for exact time starting" << endl;
		if(	nextTime->tv_sec == 0 &&
			nextTime->tv_usec == 000001	)
		{
			tout << "        subtracted times are to much for starting again" << endl;
			tout << "        please optimize folder running length (maybe split behavior)" << endl;
			tout << "        and start new server session with option -n (--nodbbegintime)" << endl;
		}
		tout << "    folder should start again in ";
		tout <<  MeasureThread::getTimevalString(*nextTime, /*as date*/false, debug) << " seconds by (";
		tout << MeasureThread::getTimevalString(*refreshTime, /*as date*/true, debug) << ")" << endl;
		if(m_bExactTime)
		{
			if(!m_oFinished.isEmpty())
				timersub(&finishedNext, &tmReachEnd, &tvLength);
			else
				tvLength= finishedNext;
			tout << "          to reach subroutine after ";
			tout << MeasureThread::getTimevalString(tvLength, /*as date*/false, debug);
			tout << " seconds, by ";
			tout <<  MeasureThread::getTimevalString(*exactStop, /*as date*/true, debug);
			tout << endl;
		}
		if(!m_oFinished.isEmpty())
		{
			tout << "          which should be finished after ";
			tout << MeasureThread::getTimevalString(finishedNext, /*as date*/false, debug);
			tout << " seconds, by ";
			tout << MeasureThread::getTimevalString(m_tmWantFinish, /*as date*/true, debug);
			tout << endl;
		}
	}
	return MeasureThread::calcResult(*nextTime, m_bSeconds);
}

double timer::calcNextTime(const bool& start, const bool& debug, timeval* actTime)
{
	double newTime;
	timeval endTime;

	if(debug)
	{
		tout << "-- calculating next time";
		if(start)
			tout << " for begin measuring";
		tout << endl;
	}
	if(start)
	{
		if(m_nDirection == -2)
		{// starting for count down
			actTime->tv_sec= m_tmSec;
			actTime->tv_usec= m_tmMicroseconds;
			newTime= MeasureThread::calcResult(*actTime, m_bSeconds);

		}else
		{
			endTime.tv_sec= m_tmSec;
			endTime.tv_usec= m_tmMicroseconds;
			if(timercmp(actTime, &endTime, >))
			{
				actTime->tv_sec= m_tmSec;
				actTime->tv_usec= m_tmMicroseconds;
			}

			newTime= MeasureThread::calcResult(*actTime, m_bSeconds);
			if(newTime < 0)
			{
				newTime= 0;
				actTime->tv_sec= 0;
				actTime->tv_usec= 0;
			}
			if(m_nDirection == 1)
			{ // direction defined to count to full time
				timersub(&endTime, actTime, actTime);
//				if(m_bExactTime)
//					newTime+= MeasureThread::calcResult(gone, m_bSeconds);
			}else
			{ // direction is defined to calculating to 0 or count down
			 // actTime has the right values
//				if(m_bExactTime)
//					newTime-= MeasureThread::calcResult(gone, m_bSeconds);
			}
		}
		if(debug)
		{
			tout << "end time should be in " << MeasureThread::calcResult(*actTime, m_bSeconds) << " seconds"<< endl;
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
		timersub(&m_tmExactStop, actTime, &endTime);
	}
	newTime= MeasureThread::calcResult(endTime, m_bSeconds);
	if(debug)
		tout << "measured actual time is " << newTime << endl;
	if(	m_nDirection == 1)
		newTime+= m_dStartValue;
	return newTime;
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
		need= MeasureThread::calcResult(next);
		getRunningThread()->nextActivateTime(getFolderName(), m_tmStart);
	}
}
#endif

void timer::setDebug(bool bDebug)
{
	m_omtime.doOutput(bDebug);
	m_oSetNull.doOutput(bDebug);
	m_oDirect.doOutput(bDebug);
	m_oFinished.doOutput(bDebug);
	switchClass::setDebug(bDebug);
}

timer::~timer()
{

}
