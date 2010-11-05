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

	bool Counter::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		string setnull;
		string prop("start");

		portBase::init(properties);
		m_pStartFolder= pStartFolder;
		// toDo: insert an starting point
		//start= (double)properties.getInt(prop, /*warning*/false);
		setnull= properties->getValue("setnull", /*warning*/false);
		m_oSetNull.init(pStartFolder, setnull);
		return true;
	}

	void Counter::setObserver(IMeasurePattern* observer)
	{
		m_oSetNull.activateObserver(observer);
	}

	void Counter::setDebug(bool bDebug)
	{
		m_oSetNull.doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	double Counter::measure(const double actValue)
	{
		bool bSetNull;
		double dResult;
		double value= actValue;

		if(!m_oSetNull.isEmpty())
		{
			if(m_oSetNull.calculate(dResult))
			{
				if(dResult < 0 || dResult > 0)
					bSetNull= true;
				else
					bSetNull= false;
			}else
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
