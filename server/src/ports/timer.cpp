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

#include "timer.h"

#include "../util/configpropertycasher.h"

using namespace util;

bool timer::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	long sec, min, hour;
	string prop;

	properties.notAllowedParameter("end");
	if(!switchClass::init(properties, pStartFolder))
		return false;
	prop= "sec";
	sec= properties.getInt(prop, /*warning*/false);
	prop= "min";
	min= properties.getInt(prop, /*warning*/false);
	prop= "hour";
	hour= properties.getInt(prop, /*warning*/false);
	if(!sec && !min && !hour)
	{
		prop= "sec";
		sec= properties.needUInt(prop);
		return false;
	}
	sec+= (min * 60);
	sec+= (hour * 60 * 60);
	m_tmSec= (time_t)sec;
	m_tmEnd= 0;
	return true;
}

bool timer::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= (double)LONG_MAX;
	return true;
}

bool timer::measure()
{
	time_t tmnow;
	bool bSet= false;

	if(switchClass::getValue("i:" + getFolderName()))
		bSet= true;

	if(	m_tmEnd == 0
		||
		m_sWhile != ""	)
	{
		bool bSwitch= bSet;

		bSet= false;
		switchClass::measure();
		if(bSet)
		{
			time(&tmnow);
			m_tmEnd= tmnow + m_tmSec;
		}
		bSet= bSwitch;
	}
	if(m_tmEnd != 0)
	{
		time(&tmnow);
		if(tmnow >= m_tmEnd)
		{
			if(isDebug())
				cout << "subrutine was longer active than " << m_tmSec << " seconds" << endl;
			m_tmEnd= 0;
			switchClass::setValue(0);
		}else
		{
			switchClass::setValue(1);
			if(isDebug())
				cout << "subroutines activation stops after " << (m_tmEnd - tmnow) << " seconds" << endl;
		}
	}
	return true;
}

timer::~timer()
{

}
