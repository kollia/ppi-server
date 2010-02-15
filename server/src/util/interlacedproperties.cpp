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
 * interlacedproperties.cpp
 *
 *  Created on: 20.03.2009
 *      Author: Alexander Kolli
 */

#include <algorithm>
#include <iostream>

#include "interlacedproperties.h"

namespace util {

	void InterlacedProperties::init(const string& modifier, const string& value, const unsigned short level)
	{
		m_bRegOrder= true;
		m_sModifier= modifier;
		m_sValue= value;
		m_nLevel= level;
	}

	void InterlacedProperties::allowLaterModifier(const bool reg)
	{
		m_bRegOrder= !reg;
	}

	bool InterlacedProperties::readLine(const string& line)
	{
		Properties::param_t prop;

		prop= Properties::read(line);
		if(!prop.correct)
			return false;
		return readLine(prop);
	}

	IInterlacedPropertyPattern* InterlacedProperties::newObject(const string modifier, const unsigned short level)
	{
		return newObject(modifier, "", level);
	}

	IInterlacedPropertyPattern* InterlacedProperties::newObject(const string modifier, const string value, const unsigned short level)
	{
		IInterlacedPropertyPattern *prop;

		prop= new InterlacedProperties(modifier, value, level, m_bByCheck); // maintained outside of method
		addModifier(prop);
		return prop;
	}

	void InterlacedProperties::addModifier(IInterlacedPropertyPattern* obj) const
	{
		string value;

		for(map<string, pos_t>::const_iterator o= m_mModifier.begin(); o != m_mModifier.end(); ++o)
		{
			if(o->first == m_sModifier)
				obj->modifier(m_sModifier, m_sValue, o->second.pos);
			else
				obj->modifier(o->first, o->second.currentval, o->second.pos);
		}
		for(map<string, string>::const_iterator o= m_mErrorParams.begin(); o != m_mErrorParams.end(); ++o)
		{
			obj->setMsgParameter(o->first, o->second);
			value= getValue(o->first, false);
			if(value != "")
				obj->setDefault(o->first, value);
		}
		obj->allowLaterModifier(!m_bRegOrder);
	}

	bool InterlacedProperties::readLine(const Properties::param_t& prop) throw(runtime_error)
	{
		vector<string>::iterator it;
		map<string, pos_t>::iterator mit;

		if(!m_vSections.empty())
		{
			if((*(m_vSections.end()-1))->readLine(prop))
				return true;
		}
		mit= m_mModifier.find(prop.parameter);
		if(mit == m_mModifier.end())
		{
			saveLine(prop);
			return true;
		}
		if(m_nLevel < mit->second.pos)
		{
			if(	m_bRegOrder
				&&
				mit->second.pos > (m_nLevel+1)	)
			{
				throw runtime_error("wrong order of modifier");
			}
			m_vSections.push_back(newObject(prop.parameter, prop.value, mit->second.pos));
			return true;

		}
		return false;
	}

	string InterlacedProperties::getValue(const string property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		if(property == m_sModifier)
		{
			return m_sValue;
		}else
		{
			map<string, pos_t>::const_iterator mit;

			mit= m_mModifier.find(property);
			if(mit != m_mModifier.end())
				return mit->second.currentval;
		}
		return Properties::getValue(property, index, warning);
	}

	void InterlacedProperties::modifier(const string& spez, const string& value/*= ""*/, const unsigned short pos/*= 0*/)
	{
		pos_t val;

		if(pos == 0)
			val.pos= m_mModifier.size() + 1;
		else
			val.pos= pos;
		val.currentval= value;
		m_mModifier[spez]= val;
	}

	bool InterlacedProperties::isModifier(const string& parameter) const
	{
		map<string, pos_t>::const_iterator mit;

		mit= m_mModifier.find(parameter);
		if(mit != m_mModifier.end())
			return true;
		return false;
	}

	bool InterlacedProperties::foundModifier(const string& spez) const
	{
		map<string, pos_t>::const_iterator last;

		if(m_mModifier.empty())
		{
			if(m_sModifier != spez)
				return false;
			return true;
		}
		last= m_mModifier.end();
		--last;
		return(spez != "" && spez == last->first ? true : false);
	}

	void InterlacedProperties::checkProperties(string* output, const bool head) const
	{
		typedef vector<IInterlacedPropertyPattern*>::const_iterator it;

		Properties::checkProperties(output, head);
		checkInterlaced(output, head);
		if(!m_vSections.empty())
		{
			for(it o= m_vSections.begin(); o != m_vSections.end(); ++o)
				(*o)->checkProperties(output, head);
		}
	}

	void InterlacedProperties::checkInterlaced(string* output, const bool head) const
	{
		for(vector<IInterlacedPropertyPattern*>::const_iterator o= m_vSections.begin(); o != m_vSections.end(); ++o)
			(*o)->checkProperties(output, head);
	}

	InterlacedProperties::~InterlacedProperties()
	{
		for(vector<IInterlacedPropertyPattern*>::iterator o= m_vSections.begin(); o != m_vSections.end(); ++o)
			delete *o;
	}

}
