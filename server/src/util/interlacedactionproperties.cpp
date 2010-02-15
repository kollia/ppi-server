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

/*
 * interlacedactionproperties.cpp
 *
 *  Created on: 22.03.2009
 *      Author: Alexander Kolli
 */

#include "interlacedactionproperties.h"

namespace util {

	bool InterlacedActionProperties::readLine(const string& line)
	{
		Properties::param_t param;
		vector<string>::iterator it;
		map<string, pos_t>::iterator mit;
		InterlacedActionProperties *obj;

		param= read(line);
		if(!param.correct)
			return false;
		if(!m_vSections.empty())
		{
			obj= dynamic_cast<InterlacedActionProperties*>(*(m_vSections.end()-1));
			if(obj->readLine(line))
				return true;
		}
		if(isAction(param.parameter))
			return ActionProperties::readLine(line);
		return InterlacedProperties::readLine(param);
	}

	void InterlacedActionProperties::checkProperties(string* output, const bool head) const
	{
		ActionProperties::checkProperties(output, head);
		InterlacedProperties::checkInterlaced(output, head);
	}

	IInterlacedPropertyPattern* InterlacedActionProperties::newObject(const string modifier, const string value, const unsigned short level)
	{
		IInterlacedActionPropertyPattern *prop;
		map<string, vector<string> >::iterator it;

		prop= new InterlacedActionProperties(modifier, value, level, m_bByCheck); // maintained outside of method
		for(map<string, vector<string> >::iterator o= m_mvActions.begin(); o != m_mvActions.end(); ++o)
			prop->action(o->first);
		addModifier(prop);
		return prop;
	}

	InterlacedActionProperties::~InterlacedActionProperties() {
		// TODO Auto-generated destructor stub
	}

}
