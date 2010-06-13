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

#include "counter.h"
#include "switch.h"

namespace ports
{

	bool Counter::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		string prop("start");

		portBase::init(properties);
		m_pStartFolder= pStartFolder;
		// toDo: insert an starting point
		//start= (double)properties.getInt(prop, /*warning*/false);
		m_sSetNull= properties.getValue("setnull", /*warning*/false);
		return true;
	}

	void Counter::setObserver(IMeasurePattern* observer)
	{
		if(m_sSetNull != "")
		{
			string folder(getFolderName());
			string subroutine(getSubroutineName());

			switchClass::activateObserver(m_pStartFolder, observer, folder, subroutine, m_sSetNull);
		}
	}

	double Counter::measure()
	{
		bool bSetNull;
		double value= getValue("i:" + getFolderName());

		if(m_sSetNull != "")
		{
			if(!switchClass::getResult(m_sSetNull, m_pStartFolder, getFolderName(), isDebug(), bSetNull))
				bSetNull= false;
		}
		if(	value == DOUBLE_MAX
			||
			bSetNull				)
		{
			value= 0;

		}else
			++value;
		return value;
	}

	bool Counter::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= (double)LONG_MAX;
		return true;
	}
}
