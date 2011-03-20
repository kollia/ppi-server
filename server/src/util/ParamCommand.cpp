/**
 *   This file 'ParamCommands.cpp' is part of ppi-server.
 *   Created on: 20.03.2011
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

#include <iostream>
#include <sstream>

#include "ParamCommand.h"

namespace util
{

	ParamCommand* ParamCommand::command(string name, bool content, string desc)
	{
		SHAREDPTR::shared_ptr<ParamCommand> newcommand;

		for(vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator it= next_commands.begin(); it != next_commands.end(); ++it)
		{
			if((*it)->name == name)
			{
				cerr << endl;
				cerr << "### ERROR: cannot implement two commands with same name '" << name << "'" << endl;
				return NULL;
			}
		}
		newcommand= SHAREDPTR::shared_ptr<ParamCommand>(new ParamCommand);
		newcommand->name= name;
		newcommand->m_bContent= content;
		newcommand->description= desc;
		next_commands.push_back(newcommand);
		return next_commands.back().get();
	}

	void ParamCommand::spaceline(const string& text/*= ""*/)
	{
		ostringstream space;
		t_options option;
		SHAREDPTR::shared_ptr<ParamCommand> spacecommand;
		static unsigned short opnr(0);

		space << "---freespace";
		if(next_commands.size() == 0)
		{
			option.bcontent= false;
			option.name= space.str();
			option.description= text;
			space << opnr;
			order.push_back(space.str());
			options[space.str()]= option;
			++opnr;

		}else
		{
			spacecommand= SHAREDPTR::shared_ptr<ParamCommand>(new ParamCommand);
			spacecommand->m_bLongShort= false;
			spacecommand->m_bContent= false;
			spacecommand->name= space.str();
			spacecommand->description= text;
			next_commands.push_back(spacecommand);
		}
	}

	void ParamCommand::option(const string& name, const string& sh, const bool content, const string& desc)
	{
		bool exist(false);
		t_options option;
		vector<string>::iterator shIt;
		map<string, t_options>::iterator opIt, op2It;


		// get option iterator from command
		opIt= options.find(name);
		if(opIt == options.end())
		{
			option.name= name;
			option.bcontent= false;
			order.push_back(name);
			options[name]= option;
			opIt= options.find(name);
		}
		if(sh.length() > 1)
			m_bLongShort= true;
		for(op2It= options.begin(); op2It != options.end(); ++op2It)
		{
			shIt= find(op2It->second.shdefs.begin(), op2It->second.shdefs.end(), sh);
			if(shIt != op2It->second.shdefs.end())
			{
				cerr << endl;
				cerr << "### ERROR: short option definition -" << sh << " already exists for option --" << op2It->first << endl;
				cerr << "           do not add this short option to option --" << name << endl;
				exist= true;
				break;
			}
		}
		if(!exist)
			opIt->second.shdefs.push_back(sh);
		if(content)
			opIt->second.bcontent= true;
		if(desc != "")
			opIt->second.description= desc;
	}

	int ParamCommand::getCommandIntContent(string& fault) const
	{
		int nRv;
		istringstream costr(m_sContent);

		fault= "";
		if(costr.eof())
		{
			fault= "##NULL";
			return 0;
		}
		costr >> nRv;
		if(	costr.fail() ||
			!costr.eof()	)
		{
			fault= "##ERROR";
			return 0;
		}
		return nRv;
	}

	float ParamCommand::getCommandFloatContent(string& fault) const
	{
		float fRv;
		istringstream costr(m_sContent);

		fault= "";
		if(costr.eof())
		{
			fault= "##NULL";
			return 0;
		}
		costr >> fRv;
		if(	costr.fail() ||
			!costr.eof()	)
		{
			fault= "##ERROR";
			return 0;
		}
		return fRv;
	}

	bool ParamCommand::hasOption(const string& option) const
	{
		map<string, t_options>::const_iterator op;

		op= options.find(option);
		if(op != options.end())
			return true;
		return false;
	}

	string ParamCommand::getOptionContent(const string& option) const
	{
		map<string, t_options>::const_iterator op;

		op= options.find(option);
		if(op == options.end())
			return "";
		return op->second.content;
	}

	int ParamCommand::getOptionIntContent(string& option) const
	{
		int nRv;
		istringstream opstr(getOptionContent(option));

		if(opstr.eof())
		{
			option= "##NULL";
			return 0;
		}
		opstr >> nRv;
		if(	opstr.fail() ||
			!opstr.eof()	)
		{
			option= "##ERROR";
			return 0;
		}
		return nRv;
	}

	float ParamCommand::getOptionFloatContent(string& option) const
	{
		float fRv;
		istringstream opstr(getOptionContent(option));

		if(opstr.eof())
		{
			option= "##NULL";
			return 0;
		}
		opstr >> fRv;
		if(	opstr.fail() ||
			!opstr.eof()	)
		{
			option= "##ERROR";
			return 0;
		}
		return fRv;
	}

	const ICommandStructPattern* ParamCommand::getNextCommand() const
	{
		if(next_commands.empty())
			return NULL;
		return next_commands[0].get();
	}

}
