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
#include <stdio.h>
#include <iostream>
#include <list>

#include "resistancemeasure.h"
#include "switch.h"

#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;

namespace ports
{
	bool ResistanceMeasure::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		string warning, warning2;
		string sOut, sIn, sNeg, sMeasuredness;
		string sOValue, sMValue;

		m_pStartFolder= pStartFolder;

		// take properties from casher for own measure
		sOut= properties->getValue("out", /*warning*/false);
		sIn= properties->getValue("in", /*warning*/false);
		sNeg= properties->getValue("neg", /*warning*/false);
		sMeasuredness= properties->getValue("measuredness", /*warning*/false);

		// take properties from casher for only calculate resistance
		sOValue= properties->getValue("ovalue", /*warning*/false);
		sMValue= properties->getValue("mvalue", /*warning*/false);

		if(	sOut != ""
			&&
			sIn != ""	)
		{
			m_bOwnMeasure= true;
			if(sOValue != "")
				warning+= "'ovalue' ";
			if(sMValue != "")
			{
				if(warning != "")
					warning+= "and ";
				warning+= "'mvalue' ";
			}
			if(warning != "")
			{
				string warningOut("### WARNING: in folder ");

				warningOut+= getFolderName();
				warningOut+= " and subroutine ";
				warningOut+= getSubroutineName();
				warningOut+= "\n             on set parameter 'out' and 'in', ";
				warningOut+= warning + "will be ignored.";
				LOGEX(LOG_INFO, warningOut, getRunningThread()->getExternSendDevice());
				tout << warningOut << endl;
			}
			properties->notAllowedAction("binary");
			return TimeMeasure::init(properties, pStartFolder);
		}
		if(sOut != "")
			warning= "'out'";
		if(sIn != "")
			warning+= "'in'";
		if(sNeg != "")
		{
			if(warning == "")
				warning= "'neg'";
			else
				warning2= "'neg'";
		}
		if(sMeasuredness != "")
		{
			if(warning == "")
				warning= "'measuredness'";
			else if(warning2 == "")
				warning2= "'measuredness'";
			else
			{
				warning+= ", ";
				warning+= warning2;
				warning2= "'measuredness'";
			}
		}
		if(warning != "")
		{
			string warningOut("### WARNING: in folder ");

			warningOut+= getFolderName();
			warningOut+= " and subroutine ";
			warningOut+= getSubroutineName();
			if(warning2 != "")
			{
				warning+= " and ";
				warning+= warning2;
			}
			warningOut+= "\n             if not set both parameter 'out' and 'in', RESISTANCE do not measure own time";
			warningOut+= "\n             so parameters ";
			warningOut+= warning + " will be ignored";
			LOGEX(LOG_INFO, warningOut, getRunningThread()->getExternSendDevice());
			tout << warningOut << endl;
		}
		sMValue= properties->needValue("mvalue");
		properties->notAllowedAction("binary");
		if(portBase::init(properties, pStartFolder))
			return false;
		if(sMValue == "")
			return false;
		m_bOwnMeasure= false;
		m_oMeasuredSubroutine.init(pStartFolder, sMValue);
		m_sOhmValue= sOValue;
		return true;
	}

	valueHolder_t ResistanceMeasure::measure(const double actValue)
	{
		char buf[150];
		string msg;
		valueHolder_t oRv;

		oRv.value= getResistance();
		sprintf(buf, "%.2lf", oRv.value);
		msg= "measured resistance:";
		msg+= buf;
		msg+=" Ohm";
		TIMELOGEX(LOG_INFO, getFolderName(), msg, getRunningThread()->getExternSendDevice());
		if(isDebug())
			tout << msg << endl;
		return oRv;
	}

	void ResistanceMeasure::setDebug(bool bDebug)
	{
		m_oMeasuredSubroutine.doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	double ResistanceMeasure::getResistance()
	{
		double resistance;
		double time;
		vector<convert_t> vNearest;
		DbInterface* db= DbInterface::instance();

		if(m_bOwnMeasure)
			time= (double)getMeasuredTime();
		else
		{
			if(!m_oMeasuredSubroutine.calculate(time))
				return 0;
		}

		vNearest= db->getNearest(getFolderName()+":"+getSubroutineName(), "ohm", time);
		if(!vNearest.size())
		{
#ifdef DEBUG
			if(isDebug())
			{
				tout << "measured time is " << time << endl;
				tout << "found no nearest given mikrosecounds," << endl;
			}
#endif // DEBUG
			return time;
		}
		if(vNearest.size() == 1)
		{
#ifdef DEBUG
			if(isDebug())
			{
				tout << "measured time is " << time << endl;
				tout << "found nearest given mikrosecounds:" << endl;
				tout << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
				tout << vNearest[0].be << " / " << vNearest[0].nMikrosec << " * " << time << endl;
			}
#endif // DEBUG
			return vNearest[0].be / vNearest[0].nMikrosec * time;
		}

	#ifdef DEBUG
		if(isDebug())
		{
			tout << "measured time is " << time << endl;
			tout << "found nearest given mikrosecounds:" << endl;
			tout << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
			tout << vNearest[1].be << " ohm is " << vNearest[1].nMikrosec << " mikroseconds" << endl;
			tout << vNearest[0].be << " + ((" << vNearest[1].be << " - " << vNearest[0].be;
			tout << ")) * (" << time << " - " << vNearest[0].nMikrosec << ") / (";
			tout << vNearest[1].nMikrosec << " - " << vNearest[0].nMikrosec << ")" << endl;
		}
	#endif // DEBUG
		resistance= vNearest[0].be + (vNearest[1].be - vNearest[0].be) * (time - vNearest[0].nMikrosec) / (vNearest[1].nMikrosec - vNearest[0].nMikrosec);

		return resistance;
	}
}
