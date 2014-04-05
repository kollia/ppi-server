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
 * actionproperties.cpp
 *
 *  Created on: 21.03.2009
 *      Author: Alexander Kolli
 */

#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "actionproperties.h"

#include "../../pattern/util/LogHolderPattern.h"

#include "../stream/IParameterStringStream.h"
#include "../stream/OParameterStringStream.h"

using namespace boost;
using namespace boost::algorithm;

namespace util {

	ActionProperties& ActionProperties::copy(const ActionProperties& x, bool constructor)
	{
		m_sDefault= x.m_sDefault;
		m_mvAllowed= x.m_mvAllowed;
		m_mvActions= x.m_mvActions;
		m_mmNotAllowed= x.m_mmNotAllowed;
		if(!constructor)
			Properties::copy(x);
		return *this;
	}

	void ActionProperties::add(const ActionProperties& props)
	{
		typedef map<string, vector<string> >::const_iterator iterator;

		for(iterator x= props.m_mvActions.begin(); x != props.m_mvActions.end(); ++x)
			m_mvActions[x->first].insert(m_mvActions[x->first].end(), x->second.begin(), x->second.end());
		Properties::add(props);
	}

	bool ActionProperties::readLine(const string& line)
	{
		Properties::param_t param;

		param.bcontinue= false;
		param.correct= false;
		param.parameter= "";
		param.read= false;
		param.uncommented= "";
		param.value= "";
		read(line, &param);
		if(	!param.correct
			||
			!param.read		)
		{
			return false;
		}
		return readLine(param);
	}

	bool ActionProperties::readLine(const Properties::param_t& parameter)
	{
		string type;
		string::size_type pos;
		string::size_type len;
		Properties::param_t param(parameter);
		map<string, vector<string> >::iterator mit;

		mit= m_mvActions.find(param.parameter);
		if(mit != m_mvActions.end())
		{
			bool bFound= false;

			do
			{
				len= param.value.length();
				pos= param.value.find("|");
				if(pos < len)
				{
					type= param.value.substr(0, pos);
					boost::trim(type);
					if(type != "")
					{
						mit->second.push_back(type);
						bFound= true;
					}
					if(param.value.length() > pos+3)
						param.value= param.value.substr(pos+2);
					else
						pos= -1;
				}else
				{
					boost::trim(param.value);
					if(param.value != "")
					{
						mit->second.push_back(param.value);
						bFound= true;
					}
				}
			}while(pos < len);
			if(bFound)
				return true;
		}
		return saveLine(param);
	}

	void ActionProperties::action(const string& spez)
	{
		m_mvActions[spez]= vector<string>();
	}

	bool ActionProperties::isAction(const string& spez) const
	{
		map<string, vector<string> >::const_iterator it;

		it= m_mvActions.find(spez);
		if(it != m_mvActions.end())
			return true;
		return false;
	}

	void ActionProperties::setDefaultActionName(const string& name)
	{
		if(name != "")
			m_sDefault= name;
	}

	bool ActionProperties::haveAction(const string& action, const string& defined) const
	{
		map<string, vector<string> >::const_iterator mit;
		vector<string>::const_iterator vit;

		mit= m_mvActions.find(action);
		if(mit == m_mvActions.end())
			return false;
		if(!allowedAction(action, defined))
			return false;
		vit= find(mit->second.begin(), mit->second.end(), defined);
		if(vit != mit->second.end())
			return true;
		return false;
	}

	bool ActionProperties::setAction(const string& action, const string& name)
	{
		map<string, vector<string> >::iterator mit;
		vector<string>::iterator vit;

		if(!allowedAction(action, name))
			return false;
		mit= m_mvActions.find(action);
		if(mit != m_mvActions.end())
		{
			vit= find(mit->second.begin(), mit->second.end(), name);
			if(vit != mit->second.end())
				return true;
		}
		m_mvActions[action].push_back(name);
		return true;
	}

	void ActionProperties::delAction(const string& action, const string& name)
	{
		map<string, vector<string> >::iterator mit;
		vector<string>::iterator vit;

		mit= m_mvActions.find(action);
		if(mit != m_mvActions.end())
		{
			vit= find(mit->second.begin(), mit->second.end(), name);
			if(vit != mit->second.end())
				mit->second.erase(vit);
		}
	}

	bool ActionProperties::allowedAction(const string& action, const string& defined) const
	{
		map<string, map<string, bool> >::const_iterator nallowAction;
		map<string, bool>::const_iterator nallowValue;

		if(!isAction(action))
			return false;
		nallowAction= m_mmNotAllowed.find(action);
		if(nallowAction != m_mmNotAllowed.end())
		{
			nallowValue= nallowAction->second.find(defined);
			if(nallowValue != nallowAction->second.end())
				return false;
		}
		m_mvAllowed[action].push_back(defined);
		return true;
	}

	string ActionProperties::str() const
	{
		string stream, sAction;
		map<string, vector<string> >::const_iterator mit;
		vector<string>::const_iterator vit;
		map<string, map<string, bool> >::const_iterator nallowAction;
		map<string, bool>::const_iterator nallowValue;
		OParameterStringStream oAction;

		stream= Properties::str();
		stream= stream.substr(0, stream.length() - 2);
		for(mit= m_mvActions.begin(); mit != m_mvActions.end(); ++mit)
		{
			nallowAction= m_mmNotAllowed.find(mit->first);
			if(nallowAction == m_mmNotAllowed.end())
			{
				sAction= mit->first + "=";
				for(vit= mit->second.begin(); vit != mit->second.end(); ++vit)
				{
					if(allowedAction(mit->first, *vit))
						sAction+= *vit + "|";
				}
				sAction= sAction.substr(0, sAction.length() - 1);
				oAction << sAction;
			}
		}
		stream+= oAction.str() + " />";
		return stream;
	}

	string ActionProperties::pulled() const
	{
		string pull;
		OParameterStringStream actions;
		map<string, vector<string> >::const_iterator mit;
		vector<string>::const_iterator vit;

		for(mit= m_mvAllowed.begin(); mit != m_mvAllowed.end(); ++mit)
		{
			pull= mit->first + "=";
			for(vit= mit->second.begin(); vit != mit->second.end(); ++vit)
				pull+= *vit + "|";
			actions << pull.substr(0, pull.length() - 1);
		}
		pull= Properties::pulled();
		pull= pull.substr(0, pull.length() - 2);
		pull+= actions.str() + " />";
		return pull;
	}

	void ActionProperties::pulled(const string& params) const
	{
		string newParams;
		string paramstr, parameter, value;
		string::size_type nLen= params.length();
		OParameterStringStream oNew;

		if(nLen < 20 || params.substr(0, 18) != "<pulledproperties ")
			return;
		newParams= params.substr(18, nLen - 20);

		IParameterStringStream ps(newParams);

		while(!ps.empty())
		{
			ps >> paramstr;
			nLen= paramstr.find("=");
			if(nLen < paramstr.length())
			{
				value= paramstr.substr(nLen + 1, paramstr.length() - 2);
				parameter= paramstr.substr(0, nLen);
				if(value[0] != '\'')
				{
					vector<string> actions;

					split(actions, value, is_any_of("|"));
					for(vector<string>::iterator it= actions.begin(); it != actions.end(); ++it)
						m_mvAllowed[parameter].push_back(*it);
				}else
					oNew << paramstr;
			}
		}
		newParams= "<pulledproperties " + oNew.str() + " />";
		Properties::pulled(newParams);
	}

	bool ActionProperties::notAllowedAction(const string& action, const string& defined, const bool set/*= false*/)
	{
		map<string, map<string, bool> >::const_iterator mit;
		vector<string>::const_iterator vit;

		if(!isAction(action))
			return false;
		m_mmNotAllowed[action][defined]= set;
		return true;
	}

	bool ActionProperties::checkProperties(string* output/*= NULL*/, const bool head/*= true*/) const
	{
		typedef  map<string, vector<string> >::const_iterator mviter;
		typedef vector<string>::const_iterator viter;

		bool bError;
		string msg, msg1, msg2;

		bError= Properties::checkProperties(&msg1, false);
		for(mviter c= m_mvActions.begin(); c != m_mvActions.end(); ++c)
		{
			mviter action= m_mvAllowed.find(c->first);

			for(viter cc= c->second.begin(); cc != c->second.end(); ++cc)
			{
				if(action != m_mvAllowed.end())
				{
					viter allowed= find(action->second.begin(), action->second.end(), *cc);
					if(allowed == action->second.end())
					{
						msg2+= "\n                               ";
						msg2+= *cc;
					}
				}else
				{
					msg2+= "\n                               ";
					msg2+= *cc;
				}
			}
			if(msg2 != "")
			{
				msg= "             follow ";
				msg+= c->first;
				msg+= " are set, but not allowed:";
				msg+= msg2 + "\n";
				msg2= "";
			}
		}

		msg2= msg;
		msg= "";
		if( msg1 != ""
			||
			msg2 != ""	)
		{
			if(head)
				msg= Properties::getMsgHead(bError);
			if(msg1 != "")
				msg+= msg1 + "\n";
			if(msg2 != "")
				msg+= msg2;
			if(output == NULL)
			{
				if(bError)
				{
					cerr << msg << endl;
					LOG(LOG_ERROR, msg);
				}else
				{
					cout << msg << endl;
					LOG(LOG_WARNING, msg);
				}
			}else
				*output+= msg;
		}
		return bError;
	}

	ActionProperties::~ActionProperties()
	{
	}

}
