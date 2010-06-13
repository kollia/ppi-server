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

#include "../logger/lib/LogInterface.h"

#include "../util/configpropertycasher.h"

using namespace util;

bool timer::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bsec, bmicro, btime, bSwitch= false;
	int milli= 0, min= 0, hour= 0, days= 0;
	time_t sec= 0;
	suseconds_t micro= 0;
	string prop;

	//properties.notAllowedParameter("end");
	//m_bTime= properties.haveAction("time");
	m_bSeconds= true;
	bsec= properties.haveAction("sec");
	bmicro= properties.haveAction("micro");
	if(bsec && bmicro)
	{
		prop= properties.getMsgHead(/*ERROR*/true);
		prop+= "action sec and micro can not be set both";
		LOG(LOG_ERROR, prop);
		cerr << prop << endl;
		return false;
	}else if(bmicro)
		m_bSeconds= false;
	if(!bsec && !bmicro)
	{ // make count down of time
		m_bTime= false;
		m_smtime= properties.getValue("mtime", /*warning*/false);
		if(m_smtime == "")
		{
			if(m_bSeconds)
			{
				prop= "day";
				days= properties.getInt(prop, /*warning*/false);
				prop= "hour";
				hour= properties.getInt(prop, /*warning*/false);
			}
			prop= "min";
			min= properties.getInt(prop, /*warning*/false);
			prop= "sec";
			sec= static_cast<time_t>(properties.getInt(prop, /*warning*/false));
			if(!m_bSeconds)
			{
				prop= "millisec";
				milli= properties.getInt(prop, /*warning*/false);
				prop= "microsec";
				sec= static_cast<suseconds_t>(properties.getInt(prop, /*warning*/false));
			}
			sec+= (static_cast<time_t>(min) * 60);
			sec+= (static_cast<time_t>(hour) * 60);
			sec+= (static_cast<time_t>(days) * 60);
			m_tmSec= sec;
			micro+= (static_cast<suseconds_t>(milli) * 1000);
			m_tmMicroseconds= micro;
		}
	}else // measure time
		m_bTime= true;
	bSwitch= false;
	if(!switchClass::init(properties, pStartFolder, &bSwitch))
		return false;
	if(	!m_bTime &&
		m_smtime == "" &&
		m_tmSec == 0 &&
		m_tmMicroseconds == 0	)
	{
		return false;
	}
	return true;
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

double timer::measure()
{
	bool bswitch;
	bool debug= isDebug();
	double need, oldValue;

	oldValue= switchClass::getValue("i:" + getFolderName());

	bswitch= switchClass::measure();
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
			{ // measure countdown
				if(m_bMeasure == false)
				{ // start time measureelse
					bool bneed= true;

					m_tmStart= tv;
					if(m_smtime != "")
					{
						if(calculateResult(m_pStartFolder, getFolderName(), m_smtime, need))
						{
							bneed= true;
							m_tmSec= static_cast<time_t>(need);
							need-= static_cast<double>(m_tmSec);
							need*= (1000 * 1000);
							m_tmMicroseconds= static_cast<suseconds_t>(need);
							m_bSeconds= m_tmMicroseconds == 0 ? true : false;
						}else
						{
							string msg("cannot read time in subroutine ");

							msg+= getFolderName() + ":" + getSubroutineName();
							msg+= " from given mtime parameter " + m_smtime;
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

						m_tmStart.tv_sec+= m_tmSec;
						m_tmStart.tv_usec+= m_tmMicroseconds;
						if(m_tmStart.tv_usec > 999999)
						{
							suseconds_t microsec;

							microsec= m_tmStart.tv_usec / (1000 * 1000);
							m_tmStart.tv_usec-= (microsec * 1000 * 1000);
							m_tmStart.tv_sec+= static_cast<time_t>(microsec);
						}
						next.tv_sec= m_tmSec;
						next.tv_usec= m_tmMicroseconds;
						need= calcResult(next);
						getRunningThread()->nextActivateTime(getFolderName(), m_tmStart);
						m_bMeasure= true;
						if(debug)
							cout << "folder should start again in " << need << " seconds" << endl;
					}

				}else if(	m_tmStart.tv_sec < tv.tv_sec ||
							(	m_tmStart.tv_sec == tv.tv_sec &&
								m_tmStart.tv_usec <= tv.tv_sec	)	)
				{
					if(debug)
					{
						timeval was;

						was.tv_sec= m_tmSec;
						was.tv_usec= m_tmMicroseconds;
						need= calcResult(was);
						cout << "folder was refreshed because time of " << need;
						cout << " seconds was reached" << endl;
						cout << "refresh time " << m_tmStart.tv_sec << "." << m_tmStart.tv_usec;
						cout << " actual time " << tv.tv_sec << "." << tv.tv_usec << endl;
					}
					need= 0;
					m_bMeasure= false;

				}else
				{
					timeval newtime;

					newtime.tv_sec= m_tmStart.tv_sec - tv.tv_sec;
					newtime.tv_usec= m_tmStart.tv_usec - tv.tv_usec;
					need= calcResult(newtime);
					if(debug)
					{
						cout << "folder should start again in " << need << " seconds" << endl;
						cout << "refresh time " << m_tmStart.tv_sec << "." << m_tmStart.tv_usec;
						cout << " actual time " << tv.tv_sec << "." << tv.tv_usec << endl;
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
	}else
	{
		need= oldValue;
		if(!m_bTime)
			need= -1;
	}
	if(debug)
		cout << "result of time is " << dec << need << " seconds" << endl;
	return need;
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
