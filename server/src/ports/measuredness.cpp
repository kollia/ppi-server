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
#include <limits.h>

#include <iostream>

#include "measuredness.h"
#include "switch.h"

#include "../logger/LogInterface.h"

namespace ports
{

	bool Measuredness::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
	{
		portBase::init(properties);
		m_pStartFolder= pStartFolder;
		m_sMeasuredValue= properties.needValue("mvalue");
		m_sBegin= properties.needValue("begin");
		if(	m_sBegin ==""
			||
			m_sMeasuredValue == ""	)
		{
			return false;
		}
		return true;
	}

	bool Measuredness::measure()
	{
		static double origValue= 0;
		//double dBegin;
		double mvalue;
		double diff;
		double value= getValue("i:" + getFolderName());
		string sfolder= getFolderName();

		if(!switchClass::calculateResult(m_pStartFolder, sfolder, &m_sMeasuredValue[0], mvalue))
		{
			string msg("### ERROR: does not found mvalue '");

			msg+= m_sMeasuredValue + "'\n           in folder ";
			msg+= sfolder + " with subroutine ";
			msg+= getSubroutineName();
			cout << msg << endl;
			TIMELOG(LOG_ERROR, "mvalue", msg);
			mvalue= 0;
		}

		/*{
			string msg("### ERROR: does not found begin '");

			msg+= m_sBegin + "'\n           in folder ";
			msg+= sfolder + " with subroutine ";
			msg+= getSubroutineName();
			cout << msg << endl;
			TIMELOG(LOG_ERROR, "begin", msg);
			dBegin= 0;
		}*/

		if(!switchClass::getResult(&m_sBegin[0], m_pStartFolder, sfolder, isDebug()))
		{
			// calculate differenze
			if(mvalue < origValue)
				origValue= mvalue;
			diff= mvalue - origValue;
			if(value > diff)
				diff= value;
		}else
		{
			origValue= mvalue;
			diff= 0;
		}

		setValue(diff);
		return true;
	}

	bool Measuredness::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= (double)LONG_MAX;
		return true;
	}
}
