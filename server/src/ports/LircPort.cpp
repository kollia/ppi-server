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
#include <iostream>
#include <errno.h>
#include <vector>

#include "LircPort.h"
#include "timer.h"

#include "../util/Terminal.h"

namespace ports
{
	bool LircPort::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bOk(true);
		string scount;

		m_bONCE= properties->haveAction("send_once");
		if(m_bONCE)
		{
			scount= properties->getValue("count", /*warning*/false);
			if(scount != "")
				bOk= m_oCount.init(pStartFolder, scount);
		}
		bOk= bOk & ExternPort::init(properties, pStartFolder);
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		timerclear(&m_tTime);
		after.init(pStartFolder, "after");
		digits.init(pStartFolder, "digits");
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		return bOk;
	}

	void LircPort::setObserver(IMeasurePattern* observer)
	{
		m_oCount.activateObserver(observer);
		ExternPort::setObserver(observer);
	}

	void LircPort::setDebug(bool bDebug)
	{
		m_oCount.doOutput(bDebug);
		ExternPort::setDebug(bDebug);
	}

	double LircPort::measure(const double actValue)
	{
		double res;

		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "SONY_CMT_MINUS_CP100_KEY_CHANNELUP" &&
			getSubroutineName() == "send_once"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		m_bWritten= false;
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		res= ExternPort::measure(actValue);
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		double dbef;

		if(	m_bWritten &&
			!m_bONCE &&
			m_bPressed		)
		{
			digits.calculate(dbef);
			if(dbef == 0)
			{
				double safter;

				if(	!show &&
					m_mtResult.size()	)
				{
					show= true;
					tout << "-----------------------------------------------------------------------------------------" << endl;
					for(map<double, pair<double, double> >::iterator i= m_mtResult.begin(); i != m_mtResult.end(); ++i)
					{
						safter= i->second.second;
						tout << getFolderName() << ":  " << i->second.first << "  " << safter << "->" << (safter+safter/2);
						tout << " = " << i->first << endl;
					}
					tout << "-----------------------------------------------------------------------------------------" << endl;
				}
			}else
				if(dbef != digbefore)
				{
					map<double, pair<double, double> >::iterator found;

					digbefore= dbef;
					tout << getFolderName() << ":       <<<<<<<<<<<<<< wrong set before" << endl;
					tout << getFolderName() << ":  " << digbefore << "  " << setafter << "->" << (setafter+setafter/2) << " =  ";
					tout << newtime << endl;
					m_mtResult[newtime]= pair<double, double>(digbefore, setafter);
					show= false;
				}
		}
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		return res;
	}

	bool LircPort::write(const string& chipID, const double value, string& addinfo)
	{
		bool access;
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
			static bool show(false);
			static double setafter;
			static double digbefore;
			static double newtime;
#endif // DEBUG_ACTIVATEDLIRCOUTPUT

		if(	m_bONCE &&
			!m_oCount.isEmpty()	)
		{
			double count;
			ostringstream scount;

			m_oCount.calculate(count);
			scount << static_cast<unsigned int>(count);
			addinfo= scount.str();
		}else
			addinfo= "1";
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		m_bWritten= true;
		if(	value == 1 &&
			!m_bONCE	 )
		{
			m_bPressed= true;
			if(gettimeofday(&m_tTime, NULL))
			{
				string msg("ERROR: cannot get time of day,\n");

				msg+= "       for measure begin of activated pin inside folder  ";
				msg+= getFolderName() + " and subroutine " + getSubroutineName() + ".";
				TIMELOG(LOG_WARNING, "gettimeofday", msg);
				if(debug)
					cerr << msg << endl;
				timerclear(&m_tTime);
			}
		}
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		access= ExternPort::write(chipID, value, addinfo);
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
				if(	value == 0 &&
					!m_bONCE		)
				{
					timeval needtime;

					m_bPressed= false;
					if(gettimeofday(&needtime, NULL))
					{
						string msg("ERROR: cannot get time of day,\n");

						msg+= "       for measure end of activated pin inside folder  ";
						msg+= getFolderName() + " and subroutine " + getSubroutineName() + ".";
						TIMELOG(LOG_WARNING, "gettimeofday", msg);
						if(debug)
							cerr << msg << endl;
					}
					after.calculate(setafter);
					digits.calculate(digbefore);
					timersub(&needtime, &m_tTime, &needtime);
					newtime= timer::calcResult(needtime, false);
					tout << getFolderName() << ":  " << digbefore << "  " << setafter << "->" << (setafter+setafter/2) << " =  ";
					tout << newtime << endl;
					m_mtResult[newtime]= pair<double, double>(digbefore, setafter);
					show= false;
					timerclear(&m_tTime);
				}
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		return access;
	}

}
