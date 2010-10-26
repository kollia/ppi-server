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
		string prop, sFrom;
		vector<string> spl;

		sFrom= properties.needValue("from");
		m_oFrom.init(pStartFolder, sFrom);
		m_sSet= properties.needValue("set");
		trim(m_sSet);
		if(m_sSet != "")
		{
			split(spl, m_sSet, is_any_of(":"));
			if(spl.size() > 0)
				trim(spl[0]);
			if(spl.size() == 2)
				trim(spl[1]);
			if(	(	spl.size() == 1 &&
					spl[0].find(" ") != string::npos	) ||
				(	spl.size() == 2 &&
					(	spl[0].find(" ") != string::npos ||
						spl[1].find(" ") != string::npos	)	)	)
			{
				ostringstream msg;

				msg << properties.getMsgHead(/*error*/true);
				msg << " set parameter '"  << m_sSet << "' can only be an single [folder:]<sburoutine>";
				LOG(LOG_ERROR, msg.str());
				cout << msg << endl;
				m_sSet= "";
			}
		}
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
			sFrom == "" ||
			m_sSet == ""										)
		{
			return false;
		}
		setValue(dDefault, "i:"+getFolderName()+":"+getSubroutineName());
		return true;
	}

	bool Set::range(bool& bfloat, double* min, double* max)
	{
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	double Set::measure(const double actValue)
	{
		bool bOk, isdebug= isDebug();
		double value;
		string folder(getFolderName());
		string subroutine(getSubroutineName());

		m_dSwitch= switchClass::measure(m_dSwitch);
		if(m_dSwitch > 0)
		{
			bOk= m_oFrom.calculate(value);
			if(bOk)
			{
				portBase* port;

				port= m_oFrom.getSubroutine(m_sSet, /*own folder*/true);
				if(port)
				{
					if(isdebug)
					{
						cout << "set value " << value;
						cout << " into subroutine '" << m_sSet << "'" << endl;
					}
					port->setValue(value, "i:"+folder+":"+subroutine);
				}else
				{
					ostringstream msg;

					msg << "cannot set value " << value << " into given subroutine '" << m_sSet << "'";
					if(isdebug)
						cout << "### ERROR: " << msg.str() << endl;
					msg << endl << "do not found this subroutine from 'set' attribute" << endl;
					msg << "from set parameter in folder " << folder << " and subroutine " << subroutine;
					TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg.str());
				}
			}else
			{
				string msg("cannot create calculation from 'from' parameter '");

				msg+= m_oFrom.getStatement();
				msg+= "' in folder " + folder + " and subroutine " + subroutine;
				TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg);
				if(isdebug)
					cout << "### ERROR: " << msg << endl;
				return actValue;
			}

		}else
			value= actValue;
		return value;
	}

	void Set::setDebug(bool bDebug)
	{
		m_oFrom.doOutput(bDebug);
		switchClass::setDebug(bDebug);
	}

}
