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
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "configpropertycasher.h"

#include "../../pattern/util/LogHolderPattern.h"

namespace util
{

	using namespace std;

	ConfigPropertyCasher::ConfigPropertyCasher()
	: 	Properties(true),
		ActionProperties(true)
	{
		string folder("folder");
		string subroutine("name");

		action("action");
		//modifier(folder);
		setMsgParameter(folder);
		m_vsModifier.push_back(folder);

		//modifier(subroutine);
		setMsgParameter(subroutine, "subroutine");
		m_vsModifier.push_back(subroutine);
	}

	ConfigPropertyCasher& ConfigPropertyCasher::copy(const ConfigPropertyCasher& x, bool constructor)
	{
		m_sCurrent= x.m_sCurrent;
		m_vsModifier= x.m_vsModifier;
		if(!constructor)
			ActionProperties::copy(x, /*constructor*/false);
		return *this;
	}

	bool ConfigPropertyCasher::readLine(const string& line)
	{
		param_t param;

		param.bcontinue= false;
		param.correct= false;
		param.parameter= "";
		param.read= false;
		param.uncommented= "";
		param.value= "";
		read(line, &param);
		if(param.correct)
		{
			m_sCurrent= param.parameter;
			return ActionProperties::readLine(param);
		}
		return false;
	}

	bool ConfigPropertyCasher::notAllowedAction(const string& defined, bool set)
	{
		return ActionProperties::notAllowedAction("action", defined, set);
	}

	Properties::param_t ConfigPropertyCasher::newSubroutine() const
	{
		param_t param;

		param.correct= false;
		for(vector<string>::const_iterator it= m_vsModifier.begin(); it != m_vsModifier.end(); ++it)
		{
			if(*it == m_sCurrent)
			{
				param.correct= true;
				param.parameter= m_sCurrent;
				param.value= getValue(m_sCurrent, false);
				break;
			}
		}
		return param;
	}


	bool ConfigPropertyCasher::notAllowedParameter(const string& property, const string& sDefault)
	{
		string value;

		if(ActionProperties::notAllowedParameter(property, sDefault))
		{
			value= getMsgHead(/*error*/false);
			value+= "parameter " + property;
			value+= " is not allowed";
			cerr << value << endl;
			LOG(LOG_WARNING, value);
			return true;
		}
		return false;
	}

	string ConfigPropertyCasher::trim(string str, string chars/*= ""*/)
	{
		int len= static_cast<int>(str.length());
		unsigned int charslen= chars.length();
		int pos= -1;

		if(str == "")
			return "";
		for(int n= 0; n < len; ++n)
		{
			bool bFound= false;

			if(chars != "")
			{
				for(unsigned int i= 0; i < charslen; ++i)
				{
					if(str[n] == chars[i])
					{
						bFound= true;
						break;
					}
				}
			}
			if(	(	chars != ""
					&&
					!bFound		)
				||
				(	chars == ""
					&&
					isgraph(str[n])	)	)
			{
				pos= n;
				break;
			}
		}
		if(pos == -1)
			return "";
		str= str.substr(pos);
		len= static_cast<int>(str.length());
		for(int n= len-1; n >= 0; --n)
		{
			bool bFound= false;

			if(chars != "")
			{
				for(unsigned int i= 0; i < charslen; ++i)
				{
					if(str[n] == chars[i])
					{
						bFound= true;
						break;
					}
				}
			}
			if(	(	chars != ""
					&&
					!bFound		)
				||
				(	chars == ""
					&&
					isgraph(str[n])	)	)
			{
				pos= n;
				break;
			}
		}
		str= str.substr(0, pos+1);
		return str;
	}

	vector<string> ConfigPropertyCasher::split(const string value, const string delimiter)
	{
		typedef vector<unsigned int>::iterator iter;

		vector<string> oRv;
		vector<unsigned int> on;
		string::size_type pos;
		string::size_type deliLen= delimiter.length();
		//string::size_type empty;
		string::size_type valueLen;
		string::size_type lowest;
		string splited(value);

		while(splited!="")
		{
			on.clear();
			for(string::size_type o= 0; o<deliLen; ++o)
			{
				pos= splited.find_first_of(&delimiter[o]);
				on.push_back(pos);
			}
			valueLen= splited.length();
			lowest= valueLen+1;
			//empty= 0;
			for(iter i= on.begin(); i!=on.end(); ++i)
			{
				if(*i < lowest)
					lowest= *i;
				//else
				//	++empty;
			}
			//if(empty == deliLen)
			//	break;
			if(lowest < valueLen)
			{
				oRv.push_back(splited.substr(0, lowest));
				splited= splited.substr(lowest+1, splited.length()-lowest);
			}else
			{
				oRv.push_back(splited);
				splited= "";
			}
			//count++;
			//cout << "value " << count << ": '" << oRv[count-1] << "'\n";
		}
		return oRv;
	}

}
