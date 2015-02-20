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
#include <climits>

#include <iostream>
#include <sys/time.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "timer.h"

#include "../util/Calendar.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"


using namespace util;
using namespace ppi_database;

bool timer::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	DbInterface *db= DbInterface::instance();
	bool bBegin, bWhile, bEnd, bOk= true, bAlwaysBegin= false, bTimeMeasure= false;
	double dDefault, dTimerStat;
	string prop, smtime, sSetNull;
	string folder(getFolderName()), subroutine(getSubroutineName());

	//Debug info to stop by right subroutine
/*	if(	getFolderName() == "Raff1_port" &&
		getSubroutineName() == "grad_time"					)
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
	if(properties->haveAction("unixtime"))
	{
		m_eWhich= pass;
		m_bPassSecs= true;
	}
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
				ValueHolder nullValue;
				nullValue.value= -1;
				setValue(nullValue,
								InformObject(InformObject::INTERNAL,
												getFolderName()+":"+getSubroutineName()));
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
			out() << msg << endl;
			bOk= false;
		}
	}

	if(properties->getPropertyCount("link"))
	{
		m_bHasLinks= true;
		if(!initLinks("TIMER", properties, pStartFolder))
			bOk= false;
	}
	//if(bOk)
	{
		m_bExactTime= false;
		m_bWaitTime= false;
		m_bFinished= true;
		if(	m_nCaseNr == 3 ||	// case 3: count the time down to 0
			m_nCaseNr == 4	)	// case 4: count the time down to 0, or up to full time
		{

			m_bExactTime= properties->haveAction("exact");
			if(m_bExactTime)
			{
				bool bStarting(false);
				string starting;

				starting= properties->getValue("start", /*warning*/false);
				if(starting != "")
				{
					vector<string> spl;

					boost::split(spl, starting, boost::is_any_of(":"));
					if(	spl.size() &&
						spl.size() <= 2	)
					{
						m_pStartObj= m_oDirect.getSubroutine(&starting, getObjectFolderID(), true);
						if(m_pStartObj != NULL)
							bStarting= true;
					}
					if(!bStarting)
					{
						string msg(properties->getMsgHead(/*error*/true));

						msg+= "cannot localize subroutine '" + starting + "' inside ";
						msg+= " starting parameter start";
						LOG(LOG_ERROR, msg);
						out() << msg << endl;
					}
				}else // if(starting != "")
					m_bWaitTime= properties->haveAction("wait");
				if(	!bStarting &&
					!m_bWaitTime	)
				{
					getRunningThread()->calculateLengthTime();
				}
				if(m_bLogPercent)
				{
					db->writeIntoDb(folder, subroutine, "wanttime");
					db->writeIntoDb(folder, subroutine, "informlate");
					db->writeIntoDb(folder, subroutine, "startlate");
					db->writeIntoDb("folder", folder, "runpercent");
					db->fillValue(folder, subroutine, "wanttime", 0, /*new*/true);
					db->fillValue(folder, subroutine, "informlate", 0, /*new*/true);
					db->fillValue(folder, subroutine, "startlate", 0, /*new*/true);
					db->fillValue("folder", folder, "runpercent", 0, /*new*/true);
				}
				m_tReachedTypes.runlength= false;
				m_tReachedTypes.folder= folder;
				m_tReachedTypes.subroutine= subroutine;
				m_tReachedTypes.maxVal= 5;
				m_tReachedTypes.inPercent= m_nFinishedCPUtime;
				m_tReachedTypes.log= m_bLogPercent;
				if(m_nCaseNr == 3) // case 3: count the time down to 0
				{
					smtime= properties->getValue("finished", /*warning*/false);
					if(smtime != "")
					{
						bool bAll1;
						long cur, n, s;
						size_t bit, set(0);
						string specID;

						if(m_bLogPercent)
						{// allow database writing for start option --timerdblog
							db->writeIntoDb(folder, subroutine, "reachpercent");
							db->writeIntoDb(folder, subroutine, "reachlate");
							db->writeIntoDb(folder, subroutine, "wrongreach");
							db->fillValue(folder, subroutine, "reachpercent", 0, /*new*/true);
							db->fillValue(folder, subroutine, "reachlate", 0, /*new*/true);
							db->fillValue(folder, subroutine, "wrongreach", 0, /*new*/true);
						}
						properties->notAllowedAction("noinfo");
						m_oFinished.init(pStartFolder, smtime);

						// define bitmask for all possibility's of folder running
						bit= sizeof(long) * 8;
						specID= getFolderRunningID();
						//cout << "   ID for " << subroutine << ":" << folder << " is " << specID << endl;
						if(specID.size() > bit)
						{
							vector<string> specs;
							ostringstream out1, out2, out3;

							specs= getRunningThread()->getAllSpecs();
							out1 << "for folder " << folder << " and subroutine " << subroutine << " with specification '";
							for(vector<string>::iterator it= specs.begin(); it != specs.end(); ++it)
								out1 << *it << " ";
							out1 << "'" << endl;
							out2 << "same folder specifications are defined for " << specID.size() << " folders" << endl;
							out3 << "but for database writing only " << bit << " folders allowed";
							cerr << "WARNING: " << out1.str();
							cerr << "         " << out2.str();
							cerr << "         " << out3.str() << endl;
							LOG(LOG_WARNING, out1.str()+out2.str()+out3.str());
						}else
							bit= specID.size();

						// search on which position is own folder
						for(string::reverse_iterator i= specID.rbegin(); i != specID.rend(); ++i)
						{
							if(*i == '1')
								break;
							++set;
						}

						// define bitmask
						n= 0;
						set= (1 << set);
						do{
							specID= "";
							cur= 0b01;
							bAll1= true;
							if((n & set) || bit <= 1)
							{
								if(bit > 1)
								{
									for(size_t c= 0; c < bit; ++c)
									{
										s= n;
										s|= set;
										if(s & cur)
											specID= "1" + specID;
										else
										{
											specID= "0" + specID;
											bAll1= false;
										}
										cur<<= 1;
									}
								}else
									specID= "none";
					// -------------------------------------------------------------------------------------
					// -------------------------------------------------------------------------------------
								// differ also between CPU time percents
								for(short n= m_nFinishedCPUtime; n <= 100; n+= m_nFinishedCPUtime)
								{
									bool exist;
									// drop set of maxcount
									// because after new starting of hardware
									// and new starting of ppi-server
									// counting should begin from start
									// maybe the application running in other better time
									ostringstream dbstr;

									dbstr << "reachend";
									//maxcount << "maxcount";
									if(bit > 1)
									{
										dbstr << specID;
										//maxcount << specID;
									}
									if(m_nFinishedCPUtime < 100)
									{
										if(bit > 1)
										{
											dbstr << "-";
											//maxcount << "-";
										}
										dbstr << n;
										//maxcount << n;
									}
									db->writeIntoDb(folder, subroutine, dbstr.str());
									//db->writeIntoDb(folder, subroutine, maxcount.str());
									if(!m_bNoDbRead)
									{
										dDefault= db->getActEntry(exist, folder, subroutine, dbstr.str());
										if(	exist &&
											dDefault > 0	)
										{
											m_tReachedTypes.percentSyncDiff[specID][n].dbValue= dDefault;
											(*m_tReachedTypes.percentSyncDiff[specID][n].reachedPercent)[0]= pair<short, double>(1, dDefault);
											//dDefault= db->getActEntry(exist, folder, subroutine, maxcount.str());
											//if(!exist)
											//	dDefault= 1;
											m_tReachedTypes.percentSyncDiff[specID][n].maxCount= 1;//static_cast<short>(dDefault);
											m_tReachedTypes.percentSyncDiff[specID][n].stype= dbstr.str();
											//m_tReachedTypes.percentSyncDiff[specID][n].scount= maxcount.str();
										}
									}else
									{
										db->fillValue(folder, subroutine, dbstr.str(), 0, /*new*/true);
										//db->fillValue(folder, subroutine, maxcount.str(), 0, /*new*/true);
									}
								}//for(short n= m_nFinishedCPUtime; n <= 100; n+= m_nFinishedCPUtime)
					// -------------------------------------------------------------------------------------
					// -------------------------------------------------------------------------------------
							}else
								bAll1= false;
							if(bAll1)
								break;
							if(n == LONG_MAX)
								break;
							++n;
						}while(!bAll1);
					}// end if(smtime != "")
				}// end if(m_nCaseNr == 3)
			} // end if(m_bExactTime)
		}// end if(m_nCaseNr == 3 or 4)
	}
	/*
	 * values should be hold synchronized
	 * with t_reachend.timerstat
	 * inside header file MinMaxTimes.h
	 * to create time statistics
	 */
	dTimerStat= 0;// normally
	if(m_bExactTime)
		dTimerStat= 1;// exact stopping
	if(m_bWaitTime)
		dTimerStat= 2;// every wait
	if(m_pStartObj != NULL)
		dTimerStat= 3;// external starting
	db->writeIntoDb(folder, subroutine, "timerstat");
	db->fillValue(folder, subroutine, "timerstat", dTimerStat, /*new*/true);
	m_dStartValue= getValue(InformObject(InformObject::INTERNAL, getFolderName()))->getValue();
	return bOk;
}

bool timer::hasSubVar(const string& subvar) const
{
	if(	subvar == "run" ||
		subvar == "startvalue"	)
	{
		return true;
	}
	return portBase::hasSubVar(subvar);
}

ppi_value timer::getSubVar(const InformObject& who, const string& subvar) const
{
	short used(0);
	ppi_value dRv(0);

	if(subvar == "run")
		used= 1;
	else if(subvar == "startvalue")
		used= 2;
	else
		return portBase::getSubVar(who, subvar);
	if(used)
	{
		LOCK(m_SUBVARLOCK);
		switch(used)
		{
		case 1:
			if(m_bRunTime)
				dRv= 1;
			else
				dRv= 0;
			break;
		case 2:
			dRv= m_dStartValue;
			break;
		default:
			dRv= 0;
			break;
		}
		UNLOCK(m_SUBVARLOCK);
	}
	return dRv;
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
	if(!m_oFinished.isEmpty())
	{
		m_oFinished.activateObserver(observer);
	}
	switchClass::setObserver(observer);
}

auto_ptr<IValueHolderPattern> timer::measure(const ppi_value& actValue)
{
	bool bswitch;
	bool bEndCount(false);
	bool debug= isDebug();
	/**
	 * result of TIMER subroutine
	 */
	double nRv;
	double nBeginTime(0);
	string subroutine(getSubroutineName()), folder(getFolderName());
	switchClass::setting set= NONE;
	ppi_time tmLastSwitchChanged;
	auto_ptr<IValueHolderPattern> oMeasureValue;

	//Debug info to stop by right subroutine
	/*if(	folder == "power_switch" &&
		subroutine == "port2_start"					)
	{
		cout << folder << ":" << subroutine << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
	if(!m_oActTime.setActTime())
	{
		string msg("ERROR: cannot get time of day,\n");

		msg+= "       so cannot measure time for TIMER function in folder ";
		msg+= folder + " and subroutine " + subroutine + ".";
		TIMELOG(LOG_ALERT, "gettimeofday", msg);
		if(debug)
			out() << msg << endl;
		if(m_nCaseNr == 5) // measure time inside case of begin/while/end
			nRv= 0;
		else
			nRv= -1;
		oMeasureValue->setValue(nRv);
		LOCK(m_SUBVARLOCK);
		m_bRunTime= false;
		UNLOCK(m_SUBVARLOCK);
		return oMeasureValue;
	}
	if(m_bExactTime)
	{
		if(	m_pStartObj != NULL &&
			m_nAllowStarting == 0	)
		{
			string errStr;
			string err(m_pStartObj->checkStartPossibility());
			vector<string> spl;
			vector<string>::size_type nLen;

			if(err != "")
			{
				m_nAllowStarting= -1;
				boost::split(spl, err, boost::is_any_of("\n"));
				nLen= spl.size();
				errStr= "### ERROR: inside " + getFolderName() + ":" + getSubroutineName();
				errStr+= " by parameter 'start'\n";
				for(vector<string>::size_type n= 0; n < nLen; ++n)
					errStr+= "           " + spl[n] + "\n";
				errStr+= "           so count only the time inside ";
				errStr+= getSubroutineName() + " down to 0";
				LOG(LOG_ERROR, errStr);
				cerr << errStr << endl;
				getRunningThread()->calculateLengthTime();

			}else
				m_nAllowStarting= 1;
		}
	}
	if(	!m_oFinished.isEmpty() &&
		!m_bFinished				)
	{// m_bFinished is only false when waiting for other subroutines in case 3
		if(debug)
			out() << "subroutine wait only for finishing of other subroutines" << endl;
		m_oFinished.calculate(nRv);
		if(nRv != 0)
		{
			ppi_time changed;

			m_bFinished= true;
			changed= m_oFinished.getLastChanging();
			if(	debug ||
				m_bLogPercent	)
			{
				bool less, wless;
				ppi_time diff, wrong;

				if(debug)
				{
					out() << "      start time: " << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << endl;
					if(m_nAllowStarting)
						out() << "external starting ";
					else
						out() << "        end time: ";
					out() << MeasureThread::getTimevalString(m_tmExactStop, /*as date*/true, debug) << endl;
					out() << "   real finished: " << MeasureThread::getTimevalString(changed, /*as date*/true, debug) << endl;
					diff= changed - m_tmStart;
				}
				if(m_tmWantFinish > changed)
				{
					wrong= m_tmWantFinish - changed;
					wless= true;
				}else
				{
					wrong= changed - m_tmWantFinish;
					wless= false;
				}
				if(m_tmExactStop > changed)
				{
					changed= m_tmExactStop - changed;
					less= true;
				}else
				{
					changed-= m_tmExactStop;
					less= false;
				}
				if(debug)
				{
					out() << "          need " << MeasureThread::getTimevalString(diff, /*as date*/false, debug)
										<< " seconds from starting to finished end"<< endl;
					out() << "          need " << MeasureThread::getTimevalString(changed, /*as date*/false, debug);
					if(less)
						out() << " less ";
					else
						out() << " more ";
					out() << "seconds after want exact stopping" << endl;
					out() << "          need " << MeasureThread::getTimevalString(wrong, /*as date*/false, debug);
					if(wless)
						out() << " less ";
					else
						out() << " more ";
					out() << "seconds than estimated" << endl;
					out() << flush;// need when __showStatistic be set for __WRITEDEBUGALLLINES
				}
				if(m_bLogPercent)
				{
					double seconds(MeasureThread::calcResult(changed, /*seconds*/false));

					if(less)
						seconds*= -1;
					getRunningThread()->fillValue(folder, subroutine, "reachlate", seconds);
					seconds= MeasureThread::calcResult(wrong, /*seconds*/false);
					if(wless)
						seconds*= -1;
					getRunningThread()->fillValue(folder, subroutine, "wrongreach", seconds);
				}
				m_sSyncID= "";
			}else
				changed-= m_tmExactStop;// <- subtraction made also inside debug mode
			if(	m_nAllowStarting != 1 ||
				m_bStartExtern			)
			{//when no external starting was done, or external starting was OK
				getRunningThread()->calcLengthDiff(&m_tReachedTypes, changed, debug);
			}
		}else
		{
			oMeasureValue->setValue(0);
			if(debug)
				out() << "result of time is 0 seconds" << endl;
			return oMeasureValue;
		}
	}
	if(debug)
	{
		ostringstream sout;

		sout << "routine running in case of " << m_nCaseNr << " (";
		switch(m_nCaseNr)
		{
		case 1:
			if(m_eWhich != pass)
			{
				sout << "folder polling all ";
				switch(m_eWhich)
				{
				case seconds:
					sout << "seconds";
					break;
				case minutes:
					sout << "minutes";
					break;
				case hours:
					sout << "hours";
					break;
				case days:
					sout << "days";
					break;
				case months:
					sout << "months";
					break;
				case years:
					sout << "years";
					break;
				default:
					sout << "no correct ACTION be set";
					break;
				}
				if(m_bPassSecs)
					sout << " and ";
			} // end of if(m_eWhich != pass)
			if(m_bPassSecs)
				sout << "writing by every pass current seconds since 1970.01.01";
			break;
		case 2:
			char timeString[21];

			strftime(timeString, 20, "%Y.%m.%d %H:%M:%S", gmtime(&m_tmStop.tv_sec));
			sout << "time count down to setting date time " << timeString;
			break;
		case 3:
			sout << "count the time down to 0";
			break;
		case 4:
			sout << "count the time down to 0, or up to full time";
			break;
		case 5:
			sout << "measure time inside case of begin/while/end";
			break;
		}
		out() << sout.str() << ")" << endl;
	}
	if(m_bSwitchbyTime)
	{
		if(	!m_bMeasure ||
			(	m_nCaseNr != 3 &&
				m_nCaseNr != 4		)	)
		{	// for case 3 (count down to 0) and
			// 4 (count down to 0 or up to full time)
			// switch is making inside working branch
			// for ending or during on
			ValueHolder oval;

			oval= switchClass::measure(m_dSwitch, set, /*can be outside changed*/false);
			m_dSwitch= oval.value;
			if(oval.lastChanging.isSet())
				tmLastSwitchChanged= oval.lastChanging;
			else
				tmLastSwitchChanged= m_oActTime;
		}

	}else if(debug)
	{
		out() << "WHILE parameter from measured before is ";
		if(m_dSwitch > 0)
			out() << "true" << endl;
		else
			out() << "false" << endl;
	}
	if(m_dSwitch > 0)
		bswitch= true;
	else
		bswitch= false;
	if(	m_nCaseNr == 1 || // folder should polling all seconds, minutes, hours, ...
		m_nCaseNr == 2	) // time count down to setting date time
	{
		oMeasureValue->setValue(polling_or_countDown(bswitch, m_oActTime, debug));
		return oMeasureValue;
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
			ppi_time next;

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
				if(debug)
				{
					if(direct > 0)
						out() << "direction is set to count the time up to full time" << endl;
					else
						out() << "direction is set to count the time down to 0" << endl;
				}
				if(	m_bMeasure == true &&
					m_nDirection != oldDirection &&
					oldDirection > -1				)
				{
					ValueHolder oval;

					bNewDirection= true;
					if(bswitch)
					{
						if(debug)
							out() << "new direction set, check whether TIMER running should start new" << endl;
						oval= switchClass::measure(m_dSwitch, set, &nRv, /*can be outside changed*/false);
						m_dSwitch= oval.value;
						if(m_dSwitch == 0)
						{
							if(oval.lastChanging.isSet())
								tmLastSwitchChanged= oval.lastChanging;
							else
								tmLastSwitchChanged= m_oActTime;
						}
						if(m_dSwitch > 0)
							bswitch= true;
						else
							bswitch= false;
					}
					if(!bswitch)
					{// when direction be changed, but running should'nt during on
					 // set direction back to old, because otherwise method calcNextTime
					 // calculate wrong time.
					 // (when running should during on, method calcStartTime make right)
						m_nDirection= oldDirection;
					}
				}
			}
			if(	m_bMeasure == false ||
				(	bNewDirection &&
					bswitch			) ||
				(	bswitch &&
					set == BEGIN		)	)
			{// BEGIN to measure
				ppi_time tv;

				if(debug)
				{
					out() << "BEGIN time measuring ";
					if(m_nDirection > -2)
					{
						out() << "to specific time";
						if(	bswitch &&
							set == BEGIN	)
						{
							out() << " because new begin was calculated";

						}else if(bNewDirection)
							out() << " because direction of count was changed";
						out() << endl;
					}else
						out() << "for count down" << endl;
				}
				next= tmLastSwitchChanged;
				if(!next.isSet())
				{
					if(debug)
						out() << "get no starting time (from begin/while/end parameters"
								<< ") take TIMER subroutine start" << endl;
					next= m_oActTime;
				}
				if(debug)
				{
					out() << "start at time " << MeasureThread::getTimevalString(next, /*as date*/true, debug) << endl;
					out() << "  actual time " << MeasureThread::getTimevalString(m_oActTime, /*as date*/true, debug) << endl;
				}
				m_tmStart= next;
				LOCK(m_SUBVARLOCK);
				m_bRunTime= true;
				m_dStartValue= actValue;
				UNLOCK(m_SUBVARLOCK);
				tv= m_oActTime;
				nRv= calcStartTime(debug, actValue, &tv);
				if(	debug &&
					m_sSyncID != ""	)
				{
					out() << "calculated BY RUNNING FOLDER ID " << m_sSyncID << endl;
				}
			/* alex 29/8/2013
			 * set m_bMeasure now inside calcStartTime()
				if(nRv == -1)
				{
					if(m_nDirection > -2)
						nRv= dbValue;
					m_bMeasure= false;
				}else
					m_bMeasure= true;*/
				if(m_nAllowStarting == 1)
				{
					m_bMeasure= false;
					if(m_bStartExtern)
					{
						m_bFinished= false;
						nRv= 0;
					}else
					{// error occurred
						m_bFinished= true;
						nRv= -2;
					}

				}else //if(m_nAllowStarting == 1)
				{
					if(	nRv == -1 &&
						m_nDirection > -2	)
					{
						nRv= actValue;
					}
				}//else if(m_nAllowStarting == 1)

			}else if(m_tmStop <= m_oActTime)
			{ // reaching end of count down
			  // now end polling, or begin with new time
				ppi_time was;

				//Debug info to stop by right subroutine
			/*	if(	folder == "Raff2_Zeit_grad" &&
					subroutine == "grad_time"					)
				{
					cout << folder << ":" << subroutine << endl;
					cout << __FILE__ << __LINE__ << endl;
				}*/
				if(m_bExactTime)
				{
					if(m_tmExactStop > m_oActTime)
					{// waiting microseconds/seconds for exact time
						ppi_time tvWait;

						tvWait= m_tmExactStop - m_oActTime;
						if(debug)
						{
							out() << "subroutine waiting ";
							out() << MeasureThread::getTimevalString(tvWait, /*as date*/false, debug);
							out() << " seconds for exact time" << endl;
						}
						if(!getRunningThread()->usleep(tvWait))
						{
							oMeasureValue->setValue(0);
							LOCK(m_SUBVARLOCK);
							m_bRunTime= false;
							UNLOCK(m_SUBVARLOCK);
							return oMeasureValue;// folder should be stopping
						}
						m_oActTime= m_tmExactStop;

					}else
					{
						if(	debug ||
							m_bLogPercent	)
						{
							ppi_time tvWait;

							tvWait= m_oActTime - m_tmExactStop;
							if(debug)
							{
								out() << "subroutine do not need to wait for exact time," << endl;
								out() << " because exact time was overrun for ";
								out() << MeasureThread::getTimevalString(tvWait, /*as date*/false, debug);
								out() << " seconds" << endl;
							}
							if(m_bLogPercent)
							{
								getRunningThread()->fillValue(folder, subroutine, "startlate",
												MeasureThread::calcResult(tvWait, /*seconds*/false));
							}
						}
						m_tmExactStop= m_oActTime;
					}
					tmLastSwitchChanged.clear();

				}else // if(m_bExactTime)
				{
					ValueHolder oval;

					if(m_nDirection >= 0)
					{
						if(debug)
						{
							string o("check whether while/end ending before and take this end time when set\n");

							o+= "because no exact action be set";
							out() << o << endl;
						}
						oval= switchClass::measure(m_dSwitch, set, &nRv, /*can be outside changed*/false);
						if(	oval.value == 0 &&
							oval.lastChanging.isSet() &&
							oval.lastChanging < m_tmExactStop	)
						{
							tmLastSwitchChanged= oval.lastChanging;
							m_tmExactStop= tmLastSwitchChanged;
							if(debug)
								out() << "take ending time from current while/end state" << endl;
						}
					}
				} // else end  from if(m_bExactTime)
				if(debug)
				{
					string syncID;
					ostringstream outStr;
					ppi_time diff;

					diff= m_tmExactStop - m_tmStart;
					was.tv_sec= m_tmSec;
					was.tv_usec= m_tmMicroseconds;
					outStr << "reach END of time measuring\n";
					if(m_sSyncID != "")
						outStr << "stop time BY RUNNING FOLDER ID " + m_sSyncID + "\n";
					outStr << "folder was refreshed because time of ";
					outStr << MeasureThread::calcResult(was, m_bSeconds);
					outStr << " seconds was reached\n";
					outStr << "  start time (" << m_tmStart.toString(/*as date*/true) << ")\n";
					outStr << "refresh time (" << m_tmStop.toString(/*as date*/true) << ")\n";
					outStr << "current time (" << m_oActTime.toString(/*as date*/true) << ")\n";
					outStr << "    want end (" << m_tmExactStop.toString(/*as date*/true) << ")\n";
					outStr << "         need " << diff.toString(/*as date*/false) << " seconds";
					out() << outStr.str() << endl;
				}
				if(	m_bPoll &&
					m_oFinished.isEmpty()	)
				{
					ppi_time next(m_oActTime);

					if(m_nDirection == 0)
						nRv= 0;
					else
						nRv= calcNextTime(/*start*/false, debug, &next);
					if(	m_bSwitchbyTime &&
						!bNewDirection		)// when new direction set,
					{						 // asking for during on made also there
						ValueHolder oval;

						if(debug)
							out() << "look whether should polling time again" << endl;
						oval= switchClass::measure(m_dSwitch, set, &nRv, /*can be outside changed*/false);
						m_dSwitch= oval.value;
						if(oval.lastChanging.isSet())
							tmLastSwitchChanged= oval.lastChanging;
						else
							tmLastSwitchChanged= m_oActTime;
					}
					if(m_dSwitch > 0)
						bswitch= true;
					else
						bswitch= false;
					if(bswitch)
					{// if end parameter with own subroutine as 0 is true,
					 // do not begin count down again
						if(debug)
						{
							out() << "end of ";
							if(m_nDirection > -2)
								out() << "time ";
							else
								out() << "count down ";
							out() << "is reached, polling again" << endl;
						}
						if( m_nDirection == 0 ||
							m_nDirection == -2	)
						{
							was.tv_sec= m_tmSec;
							was.tv_usec= m_tmMicroseconds;
							nRv= MeasureThread::calcResult(was, m_bSeconds);
						}else
							nRv= 0;
						next= m_oActTime;
						m_tmStart= m_oActTime;
						LOCK(m_SUBVARLOCK);
						m_dStartValue= nRv;
						UNLOCK(m_SUBVARLOCK);
						nRv= calcStartTime(debug, nRv, &next);
						if(nRv == -1)
						{
							if(m_nDirection > -2)
								nRv= actValue;
							m_bMeasure= false;
							bEndCount= true;
						}else
							m_bMeasure= true;

					}else // if(bswitch)
					{
						m_bMeasure= false;
						bEndCount= true;
						if(m_nDirection == -2)
							nRv= 0;
					}// else end of if(bswitch)

				}else // if(m_bPoll)
				{
					if(!m_oFinished.isEmpty())
						m_bFinished= false;
					m_bMeasure= false;
					bEndCount= true;
					if(tmLastSwitchChanged.isSet())
					{
						if(m_nDirection == 0)
							was= was - (m_tmExactStop - m_tmStart);
						else
							was= m_tmExactStop - m_tmStart;
						nRv= calcNextTime(/*start*/false, debug, &tmLastSwitchChanged);
					}else
					{
						if(m_nDirection == 1)
						{
							was.tv_sec= m_tmSec;
							was.tv_usec= m_tmMicroseconds;
							nRv= MeasureThread::calcResult(was, m_bSeconds);
						}else
							nRv= 0;
					}
					/*
					 * run ending
					 * so set switch parameter
					 * to false
					 */
					m_dSwitch= 0;
				} // else end of if(m_bPoll)

			}else // if(m_tmStop <= m_oActTime)
			{ // count down is running
				if(debug)
					out() << endl;
				if(	bswitch &&
					m_bSwitchbyTime	)
				{
					ppi_time next(m_oActTime);
					ValueHolder oval;

					nRv= calcNextTime(/*start*/false, debug, &next);
					if(debug)
					{
						out() << "check whether ";
						if(m_nDirection > -2)
							out() << "time run ";
						else
							out() << "count down ";
						out() << "should during on" << endl;
					}
					oval= switchClass::measure(m_dSwitch, set, &nRv, /*can be outside changed*/false);
					m_dSwitch= oval.value;
					if(oval.lastChanging.isSet())
						tmLastSwitchChanged= oval.lastChanging;
					else
						tmLastSwitchChanged= m_oActTime;
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
					if(debug)
						out() << "erase old time " << m_tmStop.toString(/*as date*/true)
							<< " for folder starting" << endl;
					getRunningThread()->eraseActivateTime(folder, m_tmStop);
					m_bMeasure= false;
					next= tmLastSwitchChanged;
					if(next.isSet())
						m_oActTime= next;
					if(m_nDirection == -2)
						nRv= 0;
					else
						nRv= calcNextTime(/*start*/false, debug, &next);
				}else//if(!bswitch)
				{
					if(	m_bExactTime &&
						(	m_tReachedTypes.inPercent < 100	||
							m_sSyncID != "" 					)	)	// syncID is not null
					{													// so nRv new reachend
						ppi_time refreshTime;

						if(m_sSyncID != "") // search only for new time when synchronization ID was changed
							m_tReachedTypes.synchroID= getFolderRunningID(); // or differ between more percents
						if(	m_sSyncID != m_tReachedTypes.synchroID ||
							m_tReachedTypes.inPercent < 100				)
						{
							if(debug)
							{
								ostringstream sout;

								sout << "check whether exact stop ";
								if(!m_oFinished.isEmpty())
									sout << "and or reach finish ";
								sout << "will be changed since starting" << endl;
								if(m_tReachedTypes.inPercent < 100)
								{
									sout << "because split percent is ";
									sout << m_tReachedTypes.inPercent << " lower then 100" << endl;
								}
								if(m_sSyncID != m_tReachedTypes.synchroID)
								{
									if(m_tReachedTypes.inPercent < 100)
										sout << "and";
									else
										sout << "because";
									sout << " running folders ID " << m_sSyncID;
									sout << " was changed to " << m_tReachedTypes.synchroID << endl;
								}
								out() << sout.str();
							}
							m_sSyncID= m_tReachedTypes.synchroID;
							next= m_tmWantFinish - m_tmStart;
							refreshTime= m_tmStop;
							substractExactFinishTime(&next, debug);
							if(m_tmStop != refreshTime)
							{
								getRunningThread()->changeActivationTime(folder, refreshTime, m_tmStop);
								if(debug)
									out() << "changing of folder refresh time will be done" << endl;

							}else if(debug)
								out() << "no changing have to do" << endl;
						}
					}
					next= m_oActTime;
					nRv= calcNextTime(/*start*/false, debug, &next);
				}//else if(!bswitch)
				if(debug)
				{
					if(bswitch)
					{
						out() << "WHILE: measuring of time to specific end time is running" << endl;
						out() << "routine should stop at " << MeasureThread::getTimevalString(m_tmExactStop, /*as date*/true, debug) << endl;
						out() << "      actually we have " << MeasureThread::getTimevalString(m_oActTime, /*as date*/true, debug) << endl;
						out() << "folder should start again in " << nRv << " seconds" << endl;
						out() << "    by time (" << MeasureThread::getTimevalString(m_tmStop, /*as date*/true, debug) << ")" << endl;
					}else
					{
						ppi_time newtime;

						newtime= m_tmExactStop - m_oActTime;
						out() << "WHILE: reach END of time measuring before finished" << endl;
						out() << "subroutine of timer stops " << MeasureThread::getTimevalString(newtime, /*as date*/false, debug) << " seconds before" << endl;
						out() << "  start time (" << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << ")" << endl;
						out() << "    end time (" << MeasureThread::getTimevalString(m_oActTime, /*as date*/true, debug) << ")" << endl;
						newtime= m_oActTime - m_tmStart;
						out() << "         need " << MeasureThread::getTimevalString(newtime, /*as date*/false, debug) << " seconds "<< endl;
					}
				}
			} // else end of if(m_tmStop <= m_oActTime)
		}else
		{ // m_nCaseNr is 5: measure time inside case of begin/while/end
			if(m_bMeasure == false)
			{ // begin measuring
				m_tmStart= m_oActTime;
				if(debug)
				{
					out() << "actual time : " << MeasureThread::getTimevalString(m_oActTime, /*as date*/true, debug) << endl;
					out() << "starting at : " << MeasureThread::getTimevalString(m_tmStart, /*as date*/true, debug) << endl;
				}
				nRv= 0;
				m_bMeasure= true;
				if(debug)
				{
					out() << "subroutine begin to measure time in ";
					if(m_bSeconds)
						out() << "seconds";
					else
						out() << "microseconds";
					out() << endl;
				}
			}else
			{ // while or end measure
				ppi_time needTime;
				ValueHolder oval;

				needTime-= m_tmStart;
				nRv= MeasureThread::calcResult(needTime, m_bSeconds);
				oval= switchClass::measure(m_dSwitch, set, &nRv, /*can be outside changed*/false);
				m_dSwitch= oval.value;
				if(oval.lastChanging.isSet())
					tmLastSwitchChanged= oval.lastChanging;
				else
					tmLastSwitchChanged= m_oActTime;
				if(m_dSwitch > 0)
				{ // while measure
					if(debug)
						out() << "actually measured time is ";
				}else
				{ // end measure

					m_bMeasure= false;
					if(debug)
						out() << "ending time measure by ";
				}
				if(debug)
				{
					out()  << dec << nRv << " ";
					if(!m_bSeconds)
						out() << "micro";
					out() << "seconds" << endl;
				}
			}

		}
		// bswitch was before also true
		// or some measuring was done
		bswitch= true;
	}else
	{
		if(debug)
			out() << "no measuring should be done" << endl;
		nRv= actValue;
		LOCK(m_SUBVARLOCK);
		m_dStartValue= actValue;
		UNLOCK(m_SUBVARLOCK);
	}

	if(	m_nCaseNr == 5 && // measure time inside case of begin/while/end
		!m_oSetNull.isEmpty()	)
	{
		double res;

		if(m_oSetNull.calculate(res))
		{
			if(res > 0 || res < 0)
			{
				nRv= 0;
				m_bMeasure= false;
			}
		}
	}
	if(	m_nCaseNr != 5 && // do not measure time inside case of begin/while/end
		m_nCaseNr != 4 && // do not count the time down to 0, or up to full time
		!m_bMeasure &&
		!bEndCount &&
		!(m_dTimeBefore > 0 || m_dTimeBefore < 0) &&
		!(nRv > 0 || nRv < 0)						)
	{
		nRv= -1;
	}

	oMeasureValue->setValue(nRv);
	if(m_bHasLinks)
	{
		if(getLinkedValue("TIMER", oMeasureValue, nBeginTime))
		{
			m_oActTime= oMeasureValue->getTime();
			nRv= oMeasureValue->getValue();
			if(debug)
				out() << "result of time from linked subroutine is " << dec << nRv << " seconds" << endl;

		}else if(debug)
				out() << "result of time is " << dec << nRv << " seconds" << endl;
	}else if(debug)
		out() << "result of time is " << dec << nRv << " seconds" << endl;
	if(m_dTimeBefore != nRv)
		oMeasureValue->setTime(m_oActTime);
	m_dTimeBefore= nRv;
	LOCK(m_SUBVARLOCK);
	m_bRunTime= m_bMeasure;
	UNLOCK(m_SUBVARLOCK);
	return oMeasureValue;
}

double timer::polling_or_countDown(const bool bswitch, ppi_time tv, const bool debug)
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
			if(m_eWhich != pass)
				out() << "subroutine is defined to measure time of date" << endl;
			else
				out() << "subroutine is only defined to write seconds every passing" << endl;
		}else
			out() << "subroutine is this time inside begin/while/end not defined for measure" << endl;
	}
	if(	m_bSwitchbyTime &&
		!bswitch			)
	{
		if(m_tmStop.tv_sec <= actTime)
		{
			LOCK(m_SUBVARLOCK);
			m_bRunTime= false;
			UNLOCK(m_SUBVARLOCK);
			return -1;
		}
	}
	actTime= tv.tv_sec;
	if(m_eWhich > pass)
		if(localtime_r(&actTime, &local) == NULL)
		{
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			LOCK(m_SUBVARLOCK);
			m_bRunTime= false;
			UNLOCK(m_SUBVARLOCK);
			return -1;
		}
	if(debug)
		out() << "measuring is defined for ";
	if(	m_tmStop.tv_sec > actTime ||
		m_eWhich == pass			)
	{
		switch(m_eWhich)
		{
		case notype:
			if(debug)
				out() << "seconds" << endl;
			break;
		case pass:
			if(debug)
				out() << " write seconds since 1970.01.01" << endl;
			break;
		case seconds:
			if(debug)
				out() << "seconds" << endl;
			if(!m_bPassSecs)
				actTime= local.tm_sec;
			break;
		case minutes:
			if(debug)
				out() << "minutes" << endl;
			if(!m_bPassSecs)
				actTime= local.tm_min;
			break;
		case hours:
			if(debug)
				out() << "hours" << endl;
			if(!m_bPassSecs)
				actTime= local.tm_hour;
			break;
		case days:
			if(debug)
				out() << "days" << endl;
			if(!m_bPassSecs)
				actTime= local.tm_mday;
			break;
		case months:
			if(debug)
				out() << "months" << endl;
			if(!m_bPassSecs)
				actTime= local.tm_mon + 1;
			break;
		case years:
			if(debug)
				out() << "years" << endl;
			if(!m_bPassSecs)
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
			out() << "              actual time is " << asctime(&l);
			if(m_eWhich != pass)
			{
				if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
				{
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				}
				out() << "folder was set to refresh at " << asctime(&l);
			}
			out() << "result of subroutine is " << actTime << endl;
		}
		LOCK(m_SUBVARLOCK);
		m_bRunTime= true;
		UNLOCK(m_SUBVARLOCK);
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
			out() << "seconds" << endl;
		if(m_tmSec == 0)
			m_tmStop.tv_sec+= 1;
		break;
	case pass:
	case seconds:
		if(debug)
			out() << "seconds" << endl;
		if(m_tmSec == 0)
			m_tmStop.tv_sec+= 1;
		actTime= local.tm_sec;
		break;
	case minutes:
		if(debug)
			out() << "minutes" << endl;
		thisTime= local.tm_min;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextMinute(actTime, &local);
		actTime= thisTime;
		break;
	case hours:
		if(debug)
			out() << "hours" << endl;
		thisTime= local.tm_hour;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextHour(actTime, &local);
		actTime= thisTime;
		break;
	case days:
		if(debug)
			out() << "days" << endl;
		thisTime= local.tm_mday;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextDay(actTime, &local);
		actTime= thisTime;
		break;
	case months:
		if(debug)
			out() << "months" << endl;
		thisTime= local.tm_mon + 1;
		if(m_tmSec == 0)
			m_tmStop.tv_sec= Calendar::nextMonth(actTime, &local);
		actTime= thisTime;
		break;
	case years:
		if(debug)
			out() << "years" << endl;
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
		out() << "               actual time is " << asctime(&l);
		if(localtime_r(&m_tmStop.tv_sec, &l) == NULL)
		{
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct local time");
		}
		out() << "folder should be refreshed at " << asctime(&l);
		out() << "result of subroutine is " << actTime << endl;
	}
	LOCK(m_SUBVARLOCK);
	m_bRunTime= true;
	UNLOCK(m_SUBVARLOCK);
	// toDo: measure to correct time
	getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
	return static_cast<double>(actTime);
}

double timer::calcStartTime(const bool& debug, const double actValue, ppi_time* next)
{
	bool bneed(false);
	double calc, need;
	ppi_time actTime(*next);

	if(debug)
	{
		out() << "calculate how long ";
		if(m_nDirection > -2)
			out() << "time ";
		else
			out() << "count down ";
		out() << "should running" << endl;
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
				if(m_nAllowStarting == 1)
				{
					if(debug)
						out() << "no next measure time be set, starting currently external subroutine" << endl;
					m_tmExactStop= m_tmStart;
					m_tmStop= m_tmExactStop;
					m_bStartExtern= m_pStartObj->startingBy(m_tmExactStop);
					if(	!m_bStartExtern &&
						debug										)
					{
						out() << "WARNING: cannot start external subroutine\n"
										"         maybe starting will be running" << endl;
					}
				}else if(debug)
					out() << "no next measure time be set, make no count down" << endl;
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
				out() << msg << endl;
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
		{// only by case of 3 (count the time down to 0)
			m_tReachedTypes.synchroID= getFolderRunningID();
			m_sSyncID= m_tReachedTypes.synchroID;
			if(m_bLogPercent)
			{
				DbInterface* db;

				db= DbInterface::instance();
				db->fillValue(getFolderName(), getSubroutineName(), "wanttime", need, /*new*/true);
			}
			calc= substractExactFinishTime(next, debug);
			need= calc;
		}else
		{
			calc= MeasureThread::calcResult(*next, m_bSeconds);
			m_tmExactStop= m_tmStart + *next;
			m_tmStop= m_tmExactStop;
		}
		if(m_nAllowStarting != 1)
		{
			if(calc > 0)
			{
				if(debug)
					out() << "refresh folder at " << m_tmStop.toString(/*as date*/true) << endl;
				getRunningThread()->nextActivateTime(getFolderName(), m_tmStop);
				m_bMeasure= true;
			}else
			{
				if(calc < 0)
				{
					ostringstream err;

					err << "calculated minus time (" << calc << ") to refresh folder by ";
					err << getFolderName() << ":" << getSubroutineName();
					TIMELOGEX(LOG_WARNING, getSubroutineName(), err.str(),
									getRunningThread()->getExternSendDevice());
					if(debug)
						out() << "###ERROR: " << err.str() << endl;
					calc= 0;
				}else
				{
					if(debug)
						out() << "do not refresh folder, because calculated time was 0" << endl;
				}
				//m_dSwitch= 0;
				m_bMeasure= false;
				if(!m_oFinished.isEmpty())
					m_bFinished= false;
				//need= actValue;
			}// else if(calc > 0)
		}else//if(m_nAllowStarting != 1)
		{
			m_bStartExtern= m_pStartObj->startingBy(m_tmExactStop);
			if(	!m_bStartExtern &&
				debug										)
			{
				out() << "WARNING: cannot start external subroutine\n"
								"         maybe starting will be running" << endl;
			}
		}
	}
	return need;
}

double timer::substractExactFinishTime(ppi_time* nextTime, const bool& debug)
{
	ppi_time folderLength, tmReachEnd, lateSec, needTime(*nextTime);

	m_tmWantFinish= m_tmStart + *nextTime;
	m_tmExactStop= m_tmStart + *nextTime;
	lateSec= m_oActTime - m_tmStart;
	if(m_bLogPercent)
	{
		ppi_value dlateSec;

		lateSec >> dlateSec;
		getRunningThread()->fillValue(getFolderName(), getSubroutineName(), "informlate", dlateSec);
	}
	if(	debug &&
		m_oActTime != m_tmStart	)
	{
		out() << "      - time length   " << lateSec.toString(/*as date*/false)
				<< " seconds since the late information" << endl;
	}
	if(*nextTime < lateSec)
		nextTime->clear();
	else
		*nextTime-= lateSec;
	if(m_nAllowStarting != 1)
	{
		// Subtract length of folder run, because subroutine should reached before
		// to wait inside this subroutine for exact time
		folderLength= getRunningThread()->getLengthedTime((m_nFinishedCPUtime<100), debug);
		if(*nextTime > folderLength)
		{
			m_tmStop= m_tmExactStop - folderLength;
			*nextTime-= folderLength;

		}else
		{
			nextTime->tv_sec= 0;
			nextTime->tv_usec= 0;
			m_tmStop= m_oActTime;
		}
		if(debug)
		{
			out() << "      - folder length " << folderLength.toString(/*as date*/false)
					<< " seconds for exact time starting" << endl;
		}
	}
	if(!m_oFinished.isEmpty())
	{
		tmReachEnd= getRunningThread()->getLengthedTime(&m_tReachedTypes, &m_nLengthPercent,
													(m_nFinishedCPUtime<100), debug);
		if(tmReachEnd.isSet())
		{
			// subtract calculated finished time
			if(*nextTime > tmReachEnd)
			{
				*nextTime-= tmReachEnd;
				m_tmExactStop-= tmReachEnd;
				m_tmStop-= tmReachEnd;

			}else
			{
				nextTime->clear();
				m_tmStop= m_oActTime;
				if(ppi_time(*nextTime + folderLength) > tmReachEnd)
					m_tmExactStop-= tmReachEnd;
				else
					m_tmExactStop= m_oActTime;
			}
			if(debug)
			{
				out() << "      - calculated    " << tmReachEnd.toString(/*as date*/false)
						<< " seconds where the end should be achieved" << endl;
			}
		}
	}
	if(debug)
	{
		ppi_time res;

		res= lateSec + folderLength;
		res+= tmReachEnd;
		out() << "                  are ";
		out() << res.toString(/*as date*/false);
		out() << " seconds" << endl;
		if(m_nAllowStarting == -1)
		{
			string errStr;
			string err(m_pStartObj->checkStartPossibility());
			vector<string> spl;

			boost::split(spl, err, boost::is_any_of("\n"));
			errStr= "STARTING-ERROR: " + spl[0] + "\n";
			if(spl.size() > 1)
			{
				vector<string>::size_type nLen= spl.size();
				for(vector<string>::size_type n= 1; n < nLen; ++n)
					errStr+= "                " + spl[n] + "\n";
			}
			out() << errStr;
		}
		if(nextTime->isSet())
		{
			if(m_nAllowStarting != 1)
			{
				out() << "    folder should start again in ";
				out() << nextTime->toString(/*as date*/false) << " seconds, by ";
				out() << m_tmStop.toString(/*as date*/true) << endl;
			}
			if(m_bExactTime)
			{
				if(m_nAllowStarting == 1)
					out() << "external subroutine should start at ";
				else
					out() << "          to reach subroutine after ";
				out() << ppi_time(*nextTime + folderLength).toString(/*as date*/false);
				out() << " seconds, by ";
				out() << m_tmExactStop.toString(/*as date*/true);
				out() << endl;
			}
			if(!m_oFinished.isEmpty())
			{
				res= *nextTime + folderLength;
				res+= tmReachEnd;
				out() << "     which should be finished after ";
				out() << res.toString(/*as date*/false);
				out() << " seconds, by ";
				out() << m_tmWantFinish.toString(/*as date*/true);
				out() << endl;
			}
		} //if(nextTime->isSet())
	}//if(debug)

	if(	nextTime->isSet() &&
		m_bWaitTime == false	)
	{
		if(m_nAllowStarting != 1)
			*nextTime+= folderLength;
	}else
	{
		ppi_time tvWait;

		if(debug)
		{
			if(m_bWaitTime)
				out() << "  wait now until exact stopping time is reached" << endl;
			else
				out() << "  subtracted times are to much for starting again" << endl;
		}

		if(m_tmExactStop > m_oActTime)
		{
			if(tvWait.setActTime())
				tvWait= m_tmExactStop - tvWait;
			else
				tvWait= m_tmExactStop - m_oActTime;
			if(debug)
			{
				ppi_time res;

				res= lateSec + folderLength;
				res+= tmReachEnd;
				out() << "  wait now ";
				out() << tvWait.toString(/*as date*/false);
				out() << " seconds to reach exact time of ";
				out() << m_tmExactStop.toString(/*as date*/true);
				out() << endl;
				if(!m_oFinished.isEmpty())
				{
					out() << "  which should be finished after ";
					out() << res.toString(/*as date*/false);
					out() << " seconds, by ";
					out() << m_tmWantFinish.toString(/*as date*/true);
					out() << endl;
				}
			}
			if(m_bWaitTime)// when bWaitTime set, nextTime can also be set
				nextTime->clear();
			getRunningThread()->usleep(tvWait);
			m_oActTime= m_tmExactStop;

		}else //if(m_tmExactStop > m_oActTime)
		{
			if(	debug ||
				m_bLogPercent	)
			{
				ppi_time res;

				tvWait= lateSec + tmReachEnd;
				tvWait-= needTime;
				if(debug)
				{
					out() << "subroutine do not need to wait for exact time," << endl;
					out() << "because subroutine was informed to late for ";
					out() << tvWait.toString(/*as date*/false);
					out() << " seconds to reach finished time" << endl;
				}
			}
		}
	}//end else if(nextTime->isSet())
	return MeasureThread::calcResult(*nextTime, m_bSeconds);
}

double timer::calcNextTime(const bool& start, const bool& debug, ppi_time* actTime)
{
	double newTime;
	ppi_time endTime;

	if(debug)
	{
		out() << "-- calculating next time";
		if(start)
			out() << " for begin measuring";
		out() << endl;
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
			if(*actTime > endTime)
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
				*actTime= endTime - *actTime;
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
			out() << "end time should be in " << MeasureThread::calcResult(*actTime, m_bSeconds) << " seconds"<< endl;
			out() << "measured actual time is " << newTime << " seconds" << endl;
		}
		return newTime;
	}
	endTime.tv_sec= m_tmSec;
	endTime.tv_usec= m_tmMicroseconds;
	if(m_nDirection == 1)
	{//measure up to full time
		endTime= *actTime - m_tmStart;

	}else
	{// direction is defined to calculating to 0 or make count down
		endTime= m_tmExactStop - *actTime;
	}
	newTime= MeasureThread::calcResult(endTime, m_bSeconds);
	if(debug)
		out() << "measured actual time is " << newTime << endl;
	if(	m_nDirection == 1)
	{
		LOCK(m_SUBVARLOCK);
		newTime+= m_dStartValue;
		UNLOCK(m_SUBVARLOCK);
	}
	return newTime;
}

auto_ptr<IValueHolderPattern> timer::getValue(const InformObject& who)
{
	long nval;
	ppi_value value;
	auto_ptr<IValueHolderPattern> oGetValue;

	oGetValue= switchClass::getValue(who);
	value= oGetValue->getValue();
	// secure method to calculate only with seconds
	// or microseconds and no more decimal places behind
	if(!m_bSeconds)
		value*= (1000 * 1000);
	nval= static_cast<long>(value);
	value= static_cast<ppi_value>(nval);
	if(!m_bSeconds)
		value/= (1000 * 1000);
	oGetValue->setValue(static_cast<double>(value));
	return oGetValue;
}

void timer::setValue(const IValueHolderPattern& value, const InformObject& from)
{
	switchClass::setValue(value, from);
	LOCK(m_SUBVARLOCK);
	if(!m_bRunTime)
		m_dStartValue= getValue(InformObject(InformObject::INTERNAL, getFolderName()))->getValue();
	UNLOCK(m_SUBVARLOCK);
}

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
	DESTROYMUTEX(m_SUBVARLOCK);
}
