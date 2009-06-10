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

#include "../database/Database.h"

using namespace ppi_database;

namespace ports
{

	bool SaveSubValue::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
	{
		string when;
		vector<string>::size_type count;

		m_sIdentif= properties.needValue("identif");
		if(m_sIdentif == "")
			return false;
		if(m_sIdentif.substr(0, 6) != "clear:")
		{
			count= properties.getPropertyCount("sub");
			if(count == 0)
			{ // create error massage on pull with needValue
				properties.needValue("sub");
				return false;
			}
			for(vector<string>::size_type c= 0; c < count; ++c)
				m_vSave.push_back(properties.getValue("sub", c));
		}

		when= properties.getValue("when", /*warning*/false);
		if(when != "")
		{
			properties.readLine("begin= " + when);
		}
		properties.notAllowedParameter("while");
		properties.readLine("end= true");
		if(!switchClass::init(properties, pStartFolder))
			return false;
		return true;
	}

	bool SaveSubValue::measure()
	{
		bool bFound= true;
		double value= 0;
		vector<double> vValues;
		Database* db= Database::instance();

		//value= getResult(&);
		switchClass::measure();
		if(getValue("i:" + getFolderName()))
		{
			string folder(getFolderName());
			string subroutine(getSubroutineName());

			if(m_sIdentif.substr(0, 6) != "clear:")
			{
				for(vector<string>::iterator iter= m_vSave.begin(); iter != m_vSave.end(); ++iter)
				{
					bFound= calculateResult(&(*iter)[0], value);
					if(bFound)
						vValues.push_back(value);
				}
			}
			if(	vValues.size() > 0
				||
				m_sIdentif.substr(0, 6) == "clear:"	)
			{
				db->fillValue(folder, subroutine, m_sIdentif, vValues, /*only new values*/false);
			}
		}
		return true;
	}

	bool SaveSubValue::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= (double)LONG_MAX;
		return true;
	}

}