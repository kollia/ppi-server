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

#include "actionproperties.h"

#include "../logger/lib/LogInterface.h"

namespace util {

	bool ActionProperties::readLine(const string& line)
	{
		string type;
		string::size_type pos;
		string::size_type len;
		Properties::param_t param;
		map<string, vector<string> >::iterator mit;

		param= read(line);
		if(	!param.correct
			||
			!param.read		)
		{
			return false;
		}
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

	bool ActionProperties::notAllowedAction(const string& action, const string& defined, const bool set/*= false*/)
	{
		map<string, map<string, bool> >::const_iterator mit;
		vector<string>::const_iterator vit;

		if(!isAction(action))
			return false;
		m_mmNotAllowed[action][defined]= set;
		return true;
	}

	void ActionProperties::checkProperties(string* output/*= NULL*/, const bool head/*= true*/) const
	{
		typedef  map<string, vector<string> >::const_iterator mviter;
		typedef vector<string>::const_iterator viter;

		string msg, msg1, msg2;

		Properties::checkProperties(&msg1, false);
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
				msg= Properties::getMsgHead(false);
			if(msg1 != "")
				msg+= msg1 + "\n";
			if(msg2 != "")
				msg+= msg2;
			if(output)
				*output+= msg;
			else
				cout << msg << endl;
			LOG(LOG_WARNING, msg);
		}
	}

	ActionProperties::~ActionProperties()
	{
	}

}
