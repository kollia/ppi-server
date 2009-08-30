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

#include "../logger/lib/LogInterface.h"

#include "../database/Database.h"

using namespace ppi_database;

namespace ports
{
	bool ResistanceMeasure::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
	{
		string warning, warning2;
		string sOut, sIn, sNeg, sMeasuredness;
		string sOValue, sMValue;

		m_pStartFolder= pStartFolder;

		// take properties from casher for own measure
		sOut= properties.getValue("out", /*warning*/false);
		sIn= properties.getValue("in", /*warning*/false);
		sNeg= properties.getValue("neg", /*warning*/false);
		sMeasuredness= properties.getValue("measuredness", /*warning*/false);

		// take properties from casher for only calculate resistance
		sOValue= properties.getValue("ovalue", /*warning*/false);
		sMValue= properties.getValue("mvalue", /*warning*/false);

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
				LOG(LOG_INFO, warningOut);
				cout << warningOut << endl;
			}
			return TimeMeasure::init(properties);
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
			LOG(LOG_INFO, warningOut);
			cout << warningOut << endl;
		}
		sMValue= properties.needValue("mvalue");
		if(sMValue == "")
			return false;
		m_bOwnMeasure= false;
		m_sMeasuredSubroutine= sMValue;
		m_sOhmValue= sOValue;
		return true;
	}

	bool ResistanceMeasure::measure()
	{
		char buf[150];
		string msg;
		double dResistance= getResistance();

		TimeMeasure::setValue(dResistance);
		sprintf(buf, "%.2lf", dResistance);
		msg= "measured resistance:";
		msg+= buf;
		msg+=" Ohm";
		TIMELOG(LOG_INFO, getFolderName(), msg);
		if(isDebug())
			cout << msg << endl;
		return true;
	}

	double ResistanceMeasure::getResistance()
	{
		double resistance;
		double time;
		vector<convert_t> vNearest;
		Database* db= Database::instance();

		if(m_bOwnMeasure)
			time= (double)getMeasuredTime();
		else
		{
			if(!switchClass::calculateResult(m_pStartFolder, getFolderName(), &m_sMeasuredSubroutine[0], time))
				return 0;
		}

		vNearest= db->getNearest(getFolderName()+":"+getSubroutineName(), "ohm", time);
		if(!vNearest.size())
		{
#ifdef DEBUG
			if(isDebug())
			{
				cout << "measured time is " << time << endl;
				cout << "found no nearest given mikrosecounds," << endl;
			}
#endif // DEBUG
			return time;
		}
		if(vNearest.size() == 1)
		{
#ifdef DEBUG
			if(isDebug())
			{
				cout << "measured time is " << time << endl;
				cout << "found nearest given mikrosecounds:" << endl;
				cout << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
				cout << vNearest[0].be << " / " << vNearest[0].nMikrosec << " * " << time << endl;
			}
#endif // DEBUG
			return vNearest[0].be / vNearest[0].nMikrosec * time;
		}

	#ifdef DEBUG
		if(isDebug())
		{
			cout << "measured time is " << time << endl;
			cout << "found nearest given mikrosecounds:" << endl;
			cout << vNearest[0].be << " ohm is " << vNearest[0].nMikrosec << " mikroseconds" << endl;
			cout << vNearest[1].be << " ohm is " << vNearest[1].nMikrosec << " mikroseconds" << endl;
			cout << vNearest[0].be << " + ((" << vNearest[1].be << " - " << vNearest[0].be;
			cout << ")) * (" << time << " - " << vNearest[0].nMikrosec << ") / (";
			cout << vNearest[1].nMikrosec << " - " << vNearest[0].nMikrosec << ")" << endl;
		}
	#endif // DEBUG
		resistance= vNearest[0].be + (vNearest[1].be - vNearest[0].be) * (time - vNearest[0].nMikrosec) / (vNearest[1].nMikrosec - vNearest[0].nMikrosec);

		return resistance;
	}
}
