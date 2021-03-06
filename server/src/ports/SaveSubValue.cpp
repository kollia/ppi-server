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

#include "SaveSubValue.h"

#include "../pattern/util/iactionpropertypattern.h"

using namespace design_pattern_world;

namespace ports
{

	bool SaveSubValue::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		string when;
		vector<string>::size_type count;

		properties->notAllowedParameter("default");
		properties->notAllowedAction("action", "db", false);
		m_sIdentif= properties->needValue("identif");
		if(m_sIdentif == "")
			return false;
		if(m_sIdentif.substr(0, 6) != "clear:")
		{
			count= properties->getPropertyCount("sub");
			if(count == 0)
			{ // create error massage on pull with needValue
				properties->needValue("sub");
				return false;
			}
			for(vector<string>::size_type c= 0; c < count; ++c)
			{
				ostringstream param;
				ListCalculator* calc;

				param << "sub[" << c << "]";
				//m_vSave.push_back(properties->getValue("sub", c));
				m_vpSave.push_back(new ListCalculator(getFolderName(), getSubroutineName(), param.str(), true, false, this));
				calc= m_vpSave.back();
				calc->init(pStartFolder, properties->getValue("sub", c));
			}
		}

		when= properties->getValue("when", /*warning*/false);
		if(when != "")
		{
			properties->readLine("begin= " + when);
		}
		properties->notAllowedParameter("while");
		properties->readLine("end= true");
		properties->notAllowedAction("binary");
		if(!switchClass::init(properties, pStartFolder))
			return false;
		return true;
	}

	auto_ptr<IValueHolderPattern> SaveSubValue::measure(const ppi_value& actValue)
	{
		double value= 0;
		vector<double> vValues;
		auto_ptr<IValueHolderPattern> oMeasureValue;

		oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
		m_dSwitch= switchClass::measure(m_dSwitch)->getValue();
		if(m_dSwitch > 0)
		{
			string folder(getFolderName());
			string subroutine(getSubroutineName());

			if(m_sIdentif.substr(0, 6) != "clear:")
			{
				for(vector<ListCalculator*>::iterator iter= m_vpSave.begin(); iter != m_vpSave.end(); ++iter)
				{
					if((*iter)->calculate(value))
						vValues.push_back(value);
				}
			}
			if(	vValues.size() > 0
				||
				m_sIdentif.substr(0, 6) == "clear:"	)
			{
				getRunningThread()->fillValue(folder, subroutine, m_sIdentif, vValues, /*only new values*/false);
			}
			oMeasureValue->setValue(1);
			return oMeasureValue;
		}
		oMeasureValue->setValue(0);
		return oMeasureValue;
	}

	bool SaveSubValue::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= (double)LONG_MAX;
		return true;
	}

	SaveSubValue::~SaveSubValue()
	{
		for(vector<ListCalculator*>::iterator iter= m_vpSave.begin(); iter != m_vpSave.end(); ++iter)
			delete *iter;
	}

}
