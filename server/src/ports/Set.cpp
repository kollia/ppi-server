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

#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Set.h"

#include "../logger/lib/LogInterface.h"

using namespace boost;
using namespace boost::algorithm;

namespace ports
{
	bool Set::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		double dDefault;
		string prop;
		vector<string> vec;

		m_pStartFolder= pStartFolder;
		m_sFrom= properties.needValue("from");
		m_sSet= properties.needValue("set");
		prop= "min";
		m_nMin= properties.getDouble(prop, /*warning*/false);
		if(prop == "#ERROR")
			m_nMin= 0;
		prop= "max";
		m_nMax= properties.getDouble(prop, /*warning*/false);
		if(prop == "#ERROR")
			m_nMax= -1;
		m_bFloat= properties.haveAction("float");
		prop= "default";
		dDefault= properties.getDouble(prop, /*warning*/false);
		if(	!switchClass::init(properties, pStartFolder) ||
			m_sFrom == "" ||
			m_sSet == ""										)
		{
			return false;
		}
		setValue(dDefault);
		return true;
	}

	bool Set::range(bool& bfloat, double* min, double* max)
	{
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	double Set::measure()
	{
		bool bOk, isdebug= isDebug();
		double value;
		string folder(getFolderName());
		string subroutine(getSubroutineName());

		if(switchClass::measure())
		{
			bOk= switchClass::calculateResult(m_pStartFolder, folder, m_sFrom, isdebug, value);
			if(bOk)
			{
				portBase* port;

				port= getPort(m_pStartFolder, folder, subroutine, m_sSet,
								/*need own folder*/true, "cannot set value in given subroutine '"+m_sSet+"'");
				if(port)
					port->setValue(value);
				if(isdebug)
				{
					if(port)
						cout << "set value " << value << "from '" << m_sFrom << " into subroutine " << m_sSet << endl;
					else
						cout << "cannot set value " << value << " into given subroutine '" << m_sSet << "'" << endl;
				}
			}else
			{
				string msg("cannot create calculation from 'from' parameter '");

				msg+= m_sFrom;
				msg+= "' in folder " + folder + " and subroutine " + subroutine;
				TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg);
				if(isdebug)
					cout << "### ERROR: " << msg << endl;
				return getValue("i:"+folder);
			}

		}else
			value= getValue("i:"+folder);
		return value;
	}

}
