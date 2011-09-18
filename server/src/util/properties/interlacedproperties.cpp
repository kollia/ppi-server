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

#include "../../pattern/util/LogHolderPattern.h"

namespace util {

	InterlacedProperties& InterlacedProperties::copy(const InterlacedProperties& x, bool constructor)
	{
		InterlacedProperties* newObj;

		m_nLevel= x.m_nLevel;
		m_bRegOrder= x.m_bRegOrder;
		m_sModifier= x.m_sModifier;
		m_sValue= x.m_sValue;
		m_mvModifier= x.m_mvModifier;
		for(vector<IInterlacedPropertyPattern*>::const_iterator it= m_vSections.begin(); it != m_vSections.end(); ++it)
			delete *it;
		m_vSections.clear();
		for(vector<IInterlacedPropertyPattern*>::const_iterator it= x.m_vSections.begin(); it != x.m_vSections.end(); ++it)
		{
			newObj= dynamic_cast<InterlacedProperties*>(newObject("", "", 0));
			*newObj= *dynamic_cast<InterlacedProperties*>(*it);
			m_vSections.push_back(newObj);
		}
		if(!constructor)
			Properties::copy(x);
		return *this;
	}


	void InterlacedProperties::init(const string& modifier, const string& value, const unsigned short level)
	{
		m_bRegOrder= true;
		m_sModifier= modifier;
		m_sValue= value;
		m_nLevel= level;
		if(modifier != "")
			setDefault(modifier, value);
	}

	void InterlacedProperties::allowLaterModifier(const bool reg)
	{
		m_bRegOrder= !reg;
	}

	bool InterlacedProperties::readLine(const string& line)
	{
		Properties::param_t param;

		param.bcontinue= false;
		param.correct= false;
		param.parameter= "";
		param.read= false;
		param.uncommented= "";
		param.value= "";
		Properties::read(line, &param);
		if(!param.correct)
			return false;
		return readLine(param);
	}

	IInterlacedPropertyPattern* InterlacedProperties::newObject(const string modifier, const unsigned short level)
	{
		return newObject(modifier, "", level);
	}

	IInterlacedPropertyPattern* InterlacedProperties::newObject(const string modifier, const string value, const unsigned short level)
	{
		IInterlacedPropertyPattern *prop;

		prop= new InterlacedProperties(modifier, value, level, m_bByCheck); // maintained outside of method
		InterlacedProperties::addDefinitions(prop);
		return prop;
	}

	void InterlacedProperties::addDefinitions(IInterlacedPropertyPattern* obj) const
	{
		string value;

		for(map<string, vector<pos_t> >::const_iterator o= m_mvModifier.begin(); o != m_mvModifier.end(); ++o)
		{
			for(vector<pos_t>::const_iterator it= o->second.begin(); it != o->second.end(); ++it)
			{
				/*if(o->first == m_sModifier)
					obj->modifier(m_sModifier, m_sValue, o->second.pos);
				else*/
					obj->modifier(o->first, it->currentval, it->pos);
			}
		}
		for(map<string, string>::const_iterator o= m_mErrorParams.begin(); o != m_mErrorParams.end(); ++o)
		{
			obj->setMsgParameter(o->first, o->second);
			value= getValue(o->first, false);
			if(value != "")
				obj->setDefault(o->first, value);
		}
		obj->allowLaterModifier(!m_bRegOrder);
		Properties::addDefinitions(obj);
	}

	bool InterlacedProperties::readLine(const Properties::param_t& prop)// throw(runtime_error)
	{
		vector<pos_t>::iterator it;
		map<string, vector<pos_t> >::iterator mit;

		if(!m_vSections.empty())
		{
			if((*(m_vSections.end()-1))->readLine(prop))
				return true;
		}
		mit= m_mvModifier.find(prop.parameter);
		if(mit == m_mvModifier.end())
		{
			saveLine(prop);
			return true;
		}
		it= find(mit->second.begin(), mit->second.end(), prop.value);
		if(it == mit->second.end())
		{
			it= find(mit->second.begin(), mit->second.end(), "");
			if(it == mit->second.end())
			{
				saveLine(prop);
				return true;
			}
		}
		if(m_nLevel < it->pos)
		{
			if(	m_bRegOrder
				&&
				it->pos > (m_nLevel+1)	)
			{
				throw runtime_error("wrong order of modifier");
			}
			//cout << "create new modifier '" << prop.parameter << "' with value '" << prop.value << "' on position " << it->pos << endl;
			m_vSections.push_back(newObject(prop.parameter, prop.value, it->pos));
			return true;

		}
		return false;
	}

	vector<string>::size_type InterlacedProperties::getPropertyCount(const string property) const
	{
		map<string, vector<pos_t> >::const_iterator mit;

		mit= m_mvModifier.find(property);
		if(mit != m_mvModifier.end())
		{
			vector<string>::size_type count(0);
			vector<IInterlacedPropertyPattern*>::const_iterator sIt;

			for(sIt= m_vSections.begin(); sIt != m_vSections.end(); ++sIt)
			{
				if((*sIt)->getSectionModifier() == property)
					++count;
			}
			return count;
		}
		return Properties::getPropertyCount(property);
	}

	const vector<IInterlacedPropertyPattern*> InterlacedProperties::getSections(const string& modifier/*= ""*/) const
	{
		vector<IInterlacedPropertyPattern*>::const_iterator it;
		vector<IInterlacedPropertyPattern*> vRv;

		if(modifier == "")
			return m_vSections;
		for(it= m_vSections.begin(); it != m_vSections.end(); ++it)
			if((*it)->getSectionModifier() == modifier)
				vRv.push_back(*it);
		return vRv;
	}

	const IInterlacedPropertyPattern* InterlacedProperties::getSection(const string& modifier, const string& value,
																			vector<IInterlacedPropertyPattern*>::size_type index) const
	{
		vector<IInterlacedPropertyPattern*>::size_type count(0);
		vector<IInterlacedPropertyPattern*>::const_iterator it;

		for(it= m_vSections.begin(); it != m_vSections.end(); ++it)
		{
			if(	(*it)->getSectionModifier() == modifier &&
				(*it)->getSectionValue() == value			)
			{
				if(count == index)
					return *it;
				++count;
			}
		}
		return NULL;
	}

	vector<IInterlacedPropertyPattern*>::size_type InterlacedProperties::getSectionCount(const string& modifier/*= ""*/, const string& value/*= ""*/) const
	{
		vector<IInterlacedPropertyPattern*>::size_type nRv(0);
		vector<IInterlacedPropertyPattern*>::const_iterator it;

		if(modifier == "")
			return m_vSections.size();
		for(it= m_vSections.begin(); it != m_vSections.end(); ++it)
		{
			if(	modifier == "" ||
				(*it)->getSectionModifier() == modifier	)
			{
				if(	value == "" ||
					(*it)->getSectionValue() == value	)
				{
					++nRv;
				}
			}
		}
		return nRv;
	}

	string InterlacedProperties::getValue(const string property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		map<string, vector<pos_t> >::const_iterator mit;

		if(property == m_sModifier)
		{
			return m_sValue;
		}
		mit= m_mvModifier.find(property);
		if(mit != m_mvModifier.end())
		{
			vector<string>::size_type count(0);
			vector<IInterlacedPropertyPattern*>::const_iterator sIt;

			for(sIt= m_vSections.begin(); sIt != m_vSections.end(); ++sIt)
			{
				if((*sIt)->getSectionModifier() == property)
				{
					if(index == count)
						return (*sIt)->getSectionValue();
					++count;
				}
			}
		}
		return Properties::getValue(property, index, warning);
	}

	void InterlacedProperties::modifier(const string& spez, const string& value/*= ""*/, const unsigned short pos/*= 0*/)
	{
		pos_t val;

		if(pos == 0)
		{
			val.pos= 0;
			for(map<string, vector<pos_t> >::iterator it= m_mvModifier.begin(); it != m_mvModifier.end(); ++it)
				val.pos+= it->second.size();
			++val.pos;
		}else
			val.pos= pos;
		val.currentval= value;
		m_mvModifier[spez].push_back(val);
	}

	bool InterlacedProperties::isModifier(const string& parameter) const
	{
		map<string, vector<pos_t> >::const_iterator mit;

		mit= m_mvModifier.find(parameter);
		if(mit != m_mvModifier.end())
			return true;
		return false;
	}

	bool InterlacedProperties::foundModifier(const string& spez) const
	{
		map<string, vector<pos_t> >::const_iterator last;

		if(m_mvModifier.empty())
		{
			if(m_sModifier != spez)
				return false;
			return true;
		}
		last= m_mvModifier.end();
		--last;
		return(spez != "" && spez == last->first ? true : false);
	}

	bool InterlacedProperties::checkProperties(string* output, const bool head) const
	{
		typedef vector<IInterlacedPropertyPattern*>::const_iterator it;

		bool bError;

		bError= Properties::checkProperties(output, head);
		checkInterlaced(output, head);
		if(!m_vSections.empty())
		{
			for(it o= m_vSections.begin(); o != m_vSections.end(); ++o)
			{
				if((*o)->checkProperties(output, head) == true)
					bError= true;
			}
		}
		return bError;
	}

	bool InterlacedProperties::checkInterlaced(string* output, const bool head) const
	{
		bool bError(false);
		string ioutput;

		for(vector<IInterlacedPropertyPattern*>::const_iterator o= m_vSections.begin(); o != m_vSections.end(); ++o)
		{
			if((*o)->checkProperties(&ioutput, false))
				bError= true;
		}
		if(	head &&
			ioutput != ""	)
		{
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
		}
		return bError;
	}

	InterlacedProperties::~InterlacedProperties()
	{
		for(vector<IInterlacedPropertyPattern*>::iterator o= m_vSections.begin(); o != m_vSections.end(); ++o)
			delete *o;
	}

}
