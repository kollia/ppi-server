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
#include "valueholder.h"

#include "../logger/lib/LogInterface.h"

#include "../database/Database.h"

using namespace ppi_database;


namespace ports
{
	bool ValueHolder::init(ConfigPropertyCasher &properties)
	{
		double defaultValue, *pValue;
		string sMin("min"), sMax("max");
		Database* db= Database::instance();
		string sValue("default");

		m_bFloat= properties.haveAction("float");
		m_nMin= properties.getDouble(sMin, /*warning*/false);
		m_nMax= properties.getDouble(sMax, /*warning*/false);
		defaultValue= properties.getDouble(sValue, /*warning*/false);
		if(	sMin == "#ERROR"
			||
			sMax == "#ERROR"	)
		{// value can have hole range
			m_nMin= 0;
			m_nMax= -1;
			if(	sMin == "#ERROR"
				||
				sMax == "#ERROR"	)
			{
				string msg(properties.getMsgHead(/*error*/false));

				msg+= "min and max must be set both! so value can have hole range of";
				if(m_bFloat)
					msg+= " double";
				else
					msg+= " integer";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
			}
		}
		if(!portBase::init(properties))
			return false;

		if(sValue != "#ERROR")
		{
			// set default value
			pValue= db->getActEntry(getFolderName(), getSubroutineName(), "value");
			if(pValue == NULL)
			{
				setValue(defaultValue);
			}else
				delete pValue;
		}
		return true;
	}

	bool ValueHolder::range(bool& bfloat, double* min, double* max)
	{
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	bool ValueHolder::measure()
	{
		// this method have nothing to do
		return true;
	}
}
