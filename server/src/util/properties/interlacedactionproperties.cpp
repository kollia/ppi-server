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

#include "../../pattern/util/LogHolderPattern.h"

namespace util {

	InterlacedActionProperties& InterlacedActionProperties::copy(const InterlacedActionProperties& x, bool constructor)
	{
		if(!constructor)
		{
			InterlacedProperties::copy(x, /*constructor*/false);
			ActionProperties::copy(x, /*constructor*/false);
		}
		return *this;
	}

	bool InterlacedActionProperties::readLine(const string& line)
	{
		Properties::param_t param;

		param.bcontinue= false;
		param.correct= false;
		param.parameter= "";
		param.read= false;
		param.uncommented= "";
		param.value= "";
		read(line, &param);
		if(!param.correct)
			return false;
		return readLine(param);
	}

	bool InterlacedActionProperties::readLine(const Properties::param_t& param)
	{
		InterlacedActionProperties *obj;

		if(!m_vSections.empty())
		{
			obj= dynamic_cast<InterlacedActionProperties*>(*(m_vSections.end()-1));
			if(obj->readLine(param))
				return true;
		}
		if(isAction(param.parameter))
			return ActionProperties::readLine(param);
		return InterlacedProperties::readLine(param);
	}

	bool InterlacedActionProperties::checkProperties(string* output, const bool head) const
	{
		bool bError;
		string ioutput;

		ActionProperties::checkProperties(output, head);
		bError= InterlacedProperties::checkInterlaced(output, head);
		if(head)
			ioutput= getMsgHead(bError) + ioutput;
		if(output == NULL)
		{
			if(bError)
			{
				cerr << ioutput << endl;
				LOG(LOG_ERROR, ioutput);
			}else
			{
				cout << ioutput << endl;
				LOG(LOG_WARNING, ioutput);
			}
		}else
			*output+= ioutput;
		return bError;
	}

	IInterlacedPropertyPattern* InterlacedActionProperties::newObject(const string modifier, const string value, const unsigned short level)
	{
		IInterlacedActionPropertyPattern *prop;
		map<string, vector<string> >::iterator it;

		prop= new InterlacedActionProperties(modifier, value, level, m_bByCheck); // maintained outside of method
		for(map<string, vector<string> >::iterator o= m_mvActions.begin(); o != m_mvActions.end(); ++o)
			prop->action(o->first);
		addDefinitions(prop);
		return prop;
	}

	InterlacedActionProperties::~InterlacedActionProperties() {
		// TODO Auto-generated destructor stub
	}

}
