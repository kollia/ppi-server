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

#include "../logger/lib/LogInterface.h"


using namespace util;

bool timer::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bsec, bmicro, bOk= true;
	int milli= 0, min= 0, hour= 0, days= 0;
	double dDefault;
	time_t sec= 0;
	suseconds_t micro= 0;
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
	prop= properties->getValue("end", /*warning*/false);
	if(prop != "")
	{
		vector<string> vars;
		vector<string>::iterator found;

		m_oEnd.init(pStartFolder, prop);
		vars= m_oEnd.getVariables();
		found= find(vars.begin(), vars.end(), getSubroutineName());
		if(found == vars.end())
		{
			found= find(vars.begin(), vars.end(), getFolderName()+":"+getSubroutineName());
			if(found == vars.end())
				m_oEnd.clear();
		}
	}
	m_bTime= properties->haveAction("measure");
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
	if(!m_bTime)
	{ // make count down of time
		m_bTime= false;
		smtime= properties->getValue("mtime", /*warning*/false);
		m_omtime.init(pStartFolder, smtime);
		if(smtime == "")
		{
			if(m_bSeconds)
			{
				prop= "day";
				days= properties->getInt(prop, /*warning*/false);
				prop= "hour";
				hour= properties->getInt(prop, /*warning*/false);
			}
			prop= "min";
			min= properties->getInt(prop, /*warning*/false);
			prop= "sec";
			sec= static_cast<time_t>(properties->getInt(prop, /*warning*/false));
			if(!m_bSeconds)
			{
				prop= "millisec";
				milli= properties->getInt(prop, /*warning*/false);
				prop= "microsec";
				sec= static_cast<suseconds_t>(properties->getInt(prop, /*warning*/false));
			}
			sec+= (static_cast<time_t>(min) * 60);
			sec+= (static_cast<time_t>(hour) * 60);
			sec+= (static_cast<time_t>(days) * 60);
			m_tmSec= sec;
			micro+= (static_cast<suseconds_t>(milli) * 1000);
			m_tmMicroseconds= micro;
		}
	}else
	{ // measure time
		m_bTime= true;
		sSetNull= properties->getValue("setnull", /*warning*/false);
		m_oSetNull.init(pStartFolder, sSetNull);
	}

	if(!initLinks("TIMER", properties, pStartFolder))
		bOk= false;
	if(!switchClass::init(properties, pStartFolder))
		bOk= false;
	if(	!m_bTime &&
		m_omtime.isEmpty() &&
		m_tmSec == 0 &&
		m_tmMicroseconds == 0	)
	{
		prop= properties->getMsgHead(/*ERROR*/true);
		prop+= "TIMER subroutine set for count down and ";
		if(!m_bSeconds)
			prop+= "micro";
		prop+= "seconds, but no time (";
		if(m_bSeconds)
			prop+= "day/hour/";
		prop+= "min/sec";
		if(!m_bSeconds)
			prop+= "/millisec/microsec";
		prop+= " or mtime) be set";
		LOG(LOG_ERROR, prop);
		cerr << prop << endl;
		bOk= false;
	}
	prop= "default";
	dDefault= properties->getDouble(prop, /*warning*/false);
	if(prop != "default")
	{// no default be set in configuration files for subroutine
		if(!m_bTime)
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
	if(m_bTime)
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
	double need;
	switchClass::setting set;

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "TRANSMIT_SONY" &&
		getSubroutineName() == "new_activate")
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << getFolderName() << ":" << getSubroutineName() << endl;
	}*/
	m_dSwitch= switchClass::measure(m_dSwitch, set);
	if(m_dSwitch > 0)
		bswitch= true;
	else
		bswitch= false;
	if(	bswitch ||
		m_bMeasure	)
	{
		timeval tv;

		if(gettimeofday(&tv, NULL))
		{
			string msg("ERROR: cannot get time of day,\n");

			msg+= "       so cannot measure time for TIMER function in folder ";
			msg+= getFolderName() + " and subroutine " + getSubroutineName() + ".";
			TIMELOG(LOG_WARNING, "gettimeofday", msg);
			if(debug)
				cerr << msg << endl;
			if(m_bTime)
				need= 0;
			else
				need= -1;
		}else
		{
			if(!m_bTime)
			{ // measure count down
				if(	m_bMeasure == false ||
					set == switchClass::BEGIN	)
				{ // starting first count down
					bool bneed= true;

					m_tmStart= tv;
					if(!m_omtime.isEmpty())
					{ // calculate seconds (m_tmSec) and microseconds (m_tmMicroseconds) from other subroutine
						if(m_omtime.calculate(need))
						{
							if(need > 0 || need < 0)
								bneed= true;
							if(!bneed)
							{
								if(debug)
									cout << "no next measure time be set, make no count down" << endl;
								need= -1;
							}else
							{
								m_tmSec= static_cast<time_t>(need);
								need-= static_cast<double>(m_tmSec);
								need*= (1000 * 1000);
								m_tmMicroseconds= static_cast<suseconds_t>(need);
								m_bSeconds= m_tmMicroseconds == 0 ? true : false;
							}
						}else
						{
							string msg("cannot read time in subroutine ");

							msg+= getFolderName() + ":" + getSubroutineName();
							msg+= " from given mtime parameter " + m_omtime.getStatement();
							TIMELOG(LOG_WARNING, "mtimemeasure", msg);
							if(debug)
								cerr << msg << endl;
							m_tmSec= 0;
							m_tmMicroseconds= 0;
							need= -1;
							bneed= false;
						}
					}
					if(bneed)
					{
						timeval next;

						next.tv_sec= m_tmSec;
						next.tv_usec= m_tmMicroseconds;
						timeradd(&m_tmStart, &next, &m_tmStart);
						need= calcResult(next);
						getRunningThread()->nextActivateTime(getFolderName(), m_tmStart);
						m_bMeasure= true;
						if(debug)
						{
							char stime[18];

							strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&m_tmStart.tv_sec));
							cout << "folder should start again in " << need << " seconds by (" << stime << " ";
							cout << MeasureThread::getUsecString(m_tmStart.tv_usec) << ")" << endl;
						}
					}

				}else if( timercmp(&tv, &m_tmStart, >=) )
				{ // reaching end of count down
					if(debug)
					{
						char stime[18];
						timeval was;

						was.tv_sec= m_tmSec;
						was.tv_usec= m_tmMicroseconds;
						need= calcResult(was);
						cout << "folder was refreshed because time of " << need;
						cout << " seconds was reached" << endl;
						strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&m_tmStart.tv_sec));
						cout << "refresh time (" << stime << " ";
						cout << MeasureThread::getUsecString(m_tmStart.tv_usec) << ")" << endl;
						strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&tv.tv_sec));
						cout << " actual time (" << stime << " ";
						cout << MeasureThread::getUsecString(tv.tv_usec) << ")" << endl;
					}
					if(bswitch && !m_oEnd.isEmpty())
					{
						m_oEnd.setSubVar(getSubroutineName(), 0);
						m_oEnd.calculate(m_dSwitch);
						if(	m_dSwitch < 0 ||
							m_dSwitch > 0	)
						{// if end parameter with own subroutine as 0 is true,
						 // do not begin count down again
							bswitch= false;
							set= switchClass::END;
						}
					}
					if(bswitch)
					{ // begin count down again when begin or while is true
						bool bneed= true;
						timeval next;

						if(!m_omtime.isEmpty())
						{ // calculate seconds (m_tmSec) and microseconds (m_tmMicroseconds) from other subroutine
							double res;

							if(m_omtime.calculate(res))
							{
								need= res;
								if(need < 0 || need > 0)
									bneed= true;
								if(!bneed)
								{
									if(debug)
										cout << "no next measure time be set, make no count down" << endl;
									need= -1;
								}else
								{
									m_tmSec= static_cast<time_t>(res);
									res-= static_cast<double>(m_tmSec);
									res*= (1000 * 1000);
									m_tmMicroseconds= static_cast<suseconds_t>(res);
									m_bSeconds= m_tmMicroseconds == 0 ? true : false;
								}
							}else
							{
								string msg("cannot read time in subroutine ");

								msg+= getFolderName() + ":" + getSubroutineName();
								msg+= " from given mtime parameter " + m_omtime.getStatement();
								TIMELOG(LOG_WARNING, "mtimemeasure", msg);
								if(debug)
									cerr << msg << endl;
								m_tmSec= 0;
								m_tmMicroseconds= 0;
								need= -1;
								bneed= false;
							}
						}
						if(bneed)
						{
							next.tv_sec= m_tmSec;
							next.tv_usec= m_tmMicroseconds;
							timeradd(&m_tmStart, &next, &m_tmStart);
							if(debug)
							{
								char stime[18];

								strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&m_tmStart.tv_sec));
								cout << "refresh again for polling count down in " << need;
								cout << " seconds by (" << stime << " ";
								cout << MeasureThread::getUsecString(m_tmStart.tv_usec) << ")" << endl;
							}
							getRunningThread()->nextActivateTime(getFolderName(), m_tmStart);
							need= 0;
						}else
							m_bMeasure= false;
					}else
					{
						m_bMeasure= false;
						bEndCount= true;
						need= 0;
					}

				}else
				{ // count down is running
					timeval newtime;

					if(set != switchClass::END)
					{
						timersub(&m_tmStart, &tv, &newtime);
						need= calcResult(newtime);
					}else
					{
						need= 0;
						m_bMeasure= false;
					}
					if(debug)
					{
						char stime[18];

						if(set != switchClass::END)
						{
							cout << "folder should start again in " << need << " seconds" << endl;
							strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&m_tmStart.tv_sec));
							cout << "    by time (" << stime << " ";
							cout << MeasureThread::getUsecString(m_tmStart.tv_usec) << ")" << endl;
						}else
						{
							timersub(&m_tmStart, &tv, &newtime);
							need= calcResult(newtime);
							cout << "subroutine of timer stops " << need << " seconds before" << endl;
							need= 0;
						}
						strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&tv.tv_sec));
						cout << "actual time (" << stime << " ";
						cout << MeasureThread::getUsecString(tv.tv_usec) << ")" << endl;
					}
				}
			}else
			{ // measure time up
				if(m_bMeasure == false)
				{ // begin measuring
					m_tmStart= tv;
					need= 0;
					m_bMeasure= true;
					if(debug)
					{
						cout << "subroutine begin to measure time in ";
						if(m_bSeconds)
							cout << "seconds";
						else
							cout << "microseconds";
						cout << endl;
					}
				}else
				{ // while or end measure

					tv.tv_sec-= m_tmStart.tv_sec;
					tv.tv_usec-= m_tmStart.tv_usec;
					need= calcResult(tv);
					if(bswitch)
					{ // while measure
						if(debug)
							cout << "actually measured time is ";
					}else
					{ // end measure

						m_bMeasure= false;
						if(debug)
							cout << "ending time measure by ";
					}
					if(debug)
					{
						cout  << dec << need << " ";
						if(!m_bSeconds)
							cout << "micro";
						cout << "seconds" << endl;
					}
				}

			}
		}
		// bswitch was before also true
		// or some measuring was done
		bswitch= true;
	}else
		need= actValue;

	if(	m_bTime &&
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
	if(	!m_bTime &&
		!m_bMeasure &&
		!bEndCount &&
		!(m_dTimeBefore > 0 || m_dTimeBefore < 0) &&
		!(need > 0 || need < 0)						)
	{
		need= -1;
	}

	if(getLinkedValue("TIMER", need))
	{
		if(debug)
			cout << "result of time from linked subroutine is " << dec << need << " seconds" << endl;

	}else if(debug)
			cout << "result of time is " << dec << need << " seconds" << endl;

	m_dTimeBefore= need;
	return need;
}

#if 0
void timer::setValue(double value, const string& from)
{
	if(	!m_bTime &&
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
	m_oEnd.doOutput(bDebug);
	switchClass::setDebug(bDebug);
}

double timer::calcResult(timeval tv)
{
	double dRv;

	if(!m_bSeconds)
	{
		if(tv.tv_sec > (60 * 60)) // minutes
		{
			tv.tv_sec= 60 * 60;
			tv.tv_usec= 0;
		}
		dRv= static_cast<double>(tv.tv_usec) / (1000 * 1000);
		dRv+= static_cast<double>(tv.tv_sec);
	}else
		dRv= static_cast<double>(tv.tv_sec);
	return dRv;
}

timer::~timer()
{

}
