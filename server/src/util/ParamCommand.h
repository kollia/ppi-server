/**
 *   This file 'ParamCommand.h' is part of ppi-server.
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

#ifndef PARAMCOMMAND_H_
#define PARAMCOMMAND_H_

#include <string>
#include <vector>
#include <map>

#include "../pattern/util/ICommandStructPattern.h"

#include "smart_ptr.h"

using namespace std;

namespace util
{

	/**
	 * sturcture of options
	 */
	struct t_options
	{
		/**
		 * name of option beginning with two minus '--'
		 */
		string name;
		/**
		 * vector of all short definitions beginning with one minus '-'
		 */
		vector<string> shdefs;
		/**
		 * whether option has an content separated with an splash
		 */
		bool bcontent;
		/**
		 * content of option
		 */
		string content;
		/**
		 * description of option
		 */
		string description;
	};

	/**
	 * parameter commands inside of class MainParams
	 */
	class ParamCommand : virtual public ICommandStructPattern
	{
	public:
		/**
		 * constructor of ParamCommand
		 */
		ParamCommand()
		:	m_bLongShort(false),
		 	m_bContent(false)
		{};
		/**
		 * return name of actual command
		 *
		 * @return name of command
		 */
		virtual string command() const
		{ return name; };
		/**
		 * allocate allowd command after binary
		 *
		 * @param name command name
		 * @param desc description for command
		 * @return command class where can insert new options and or commands
		 */
		ParamCommand* command(const string name, const string desc)
		{ return command(name, false, desc); };
		/**
		 * allocate allowd command after spezific command
		 *
		 * @param name command name
		 * @param content whether option has an content
		 * @param desc description for command
		 * @return command class where can insert new options and or commands
		 */
		ParamCommand* command(const string name, const bool content, const string desc);
		/**
		 * allocate allowed option with no content
		 *
		 * @param name option name to set with two minus '--' before
		 * @param sh character to set option with one minus '-' before
		 * @param desc description for option
		 */
		void option(const string& name, const string& sh, const string& desc)
		{ option(name, sh, false, desc); };
		/**
		 * allocate allowed option for specific command
		 *
		 * @param name option name to set with two minus '--' before
		 * @param sh character to set option with one minus '-' before
		 * @param content whether option has an content
		 * @param desc description for option
		 */
		void option(const string& name, const string& sh, const bool content, const string& desc);
		/**
		 * make an space line with or without text between main options or main commands
		 *
		 * @param text specific text between commands or options
		 */
		void spaceline(const string& text= "");
		/**
		 * return content of actual command
		 *
		 * @return content behind command
		 */
		virtual string getCommandContent() const
		{ return m_sContent; };
		/**
		 * return content of actual command
		 * casted as integer number
		 *
		 * @param fault is '##ERROR' if content is no integer or '##NULL' when not set
		 * @return content behind command
		 */
		virtual int getCommandIntContent(string& fault) const;
		/**
		 * return content of actual command
		 * casted as floating number
		 *
		 * @param fault is '##ERROR' if content is no float or '##NULL' when not set
		 * @return content behind command
		 */
		virtual float getCommandFloatContent(string& fault) const;
		/**
		 * question for main option (as first) be called
		 *
		 * @param option name of option (no short defined option)
		 * @return whether option be called
		 */
		virtual bool hasOption(const string& option) const;
		/**
		 * return how many options be set
		 *
		 * @return count of options
		 */
		virtual size_t optioncount() const
		{ return options.size(); };
		/**
		 * return content from option in actual command
		 *
		 * @param option name of option
		 * @return content of option
		 */
		virtual string getOptionContent(const string& option) const;
		/**
		 * return content from option in actual command
		 * casted as integer number
		 *
		 * @param option name of option (is '##ERROR' if content is no integer or '##NULL' when not set)
		 * @return content of option
		 */
		virtual int getOptionIntContent(string& option) const;
		/**
		 * return content from option in actual command
		 * cated as floating number
		 *
		 * @param option name of option (is '##ERROR' if content is no float or '##NULL' when not set)
		 * @return content of option
		 */
		virtual float getOptionFloatContent(string& option) const;
		/**
		 * return pattern of next called command which inherit
		 * all other commands and also options
		 *
		 * @return next command
		 */
		virtual const ICommandStructPattern* getNextCommand() const ;
		/**
		 * operator for comparison
		 */
		int operator==(const ParamCommand &other) const
		{
			if(name != other.name)
				return 0;
			return 1;
		};
		/**
		 * operator for comparison
		 */
		int operator<(const ParamCommand &other) const
		{
			if(name < other.name)
				return 1;
			return 0;
		};
		/**
		 * operator for comparison
		 */
		int operator>(const ParamCommand &other) const
		{
			if(name > other.name)
				return 1;
			return 0;
		};
		/**
		 * destruktor of ParamCommand
		 */
		virtual ~ParamCommand() {};

	private:
		/**
		 * name of command.<br />
		 * can not begin with minus '-'
		 */
		string name;
		/**
		 * map of all options behind this command
		 */
		map<string, t_options> options;
		/**
		 * order of all options
		 */
		vector<string> order;
		/**
		 * whether one short definition of options is longer than one character,
		 * so cannot make more options after one minus (exp. cannot make -abc is options -a -b -c)
		 */
		bool m_bLongShort;
		/**
		 * whether command has an content separated with an splash.<br />
		 * Before content can be set also options when defined.
		 */
		bool m_bContent;
		/**
		 * content of command
		 */
		string m_sContent;
		/**
		 * description of option
		 */
		string description;
		/**
		 * all other commands behind this one
		 */
		vector<SHAREDPTR::shared_ptr<ParamCommand> > next_commands;
		/**
		 * class MainParams has full access to ParamCommand
		 */
		friend class MainParams;
	};

}

#endif /* PARAMCOMMAND_H_ */
