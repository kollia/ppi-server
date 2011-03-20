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

#ifndef MAINPARAMS_H_
#define MAINPARAMS_H_

#include <string>
#include <vector>
#include <map>

#include "../pattern/util/IOptionStructPattern.h"
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
	 * structure of commands
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

	/**
	 * read parameters from starting main function <code>main(int argc, char* argv[])</code>
	 */
	class MainParams : virtual public IOptionStructPattern
	{
	public:
		/**
		 * constructor to read parameters and calculating working path
		 *
		 * @param argc count of exist parameters inside argv
		 * @param argv character array of all parameters
		 * @param nParent count of directorys before for return value of <code>getPath()</code> (default:0)
		 */
		explicit MainParams(int argc, char* argv[], vector<string>::size_type nParent= 0);
		/**
		 * set version option to output on command line when needed
		 *
		 * @param option name of full option to get version output
		 * @param def short option definition
		 * @param major major release version
		 * @param minor release version
		 * @param sub additional version when needed (don't display if sub parameter is 0)
		 * @param patch patch number for release
		 * @param build build number of release  (don't display if build parameter is 0)
		 * @param revision revision number of release (don't display if revision parameter is 0)
		 * @param distribution added string for distribution who compiling release
		 * @param nsub prefix for sub revision number to filled with null's ('0') before.<br /> (default:2)<br /> if nsub and sub parameter is 0, no sub version will be displayed
		 * @param nbuild prefix for sub revision number to filled with null's ('0') before (default:5)
		 * @param nrevision prefix for sub revision number to filled with null's ('0') before (default:5)
		 */
		void version(const string& option, const string& def, unsigned int major, unsigned int minor, unsigned int sub,
				unsigned int patch, unsigned int build, unsigned int revision, const string& distribution= "",
				unsigned int nsub= 2, unsigned int nbuild= 5, unsigned int nrevision= 5);
		/**
		 * set version option to output on command line when needed.<br />
		 * Version option is for standard '--version' with short definition '-v'
		 *
		 * @param major major release version
		 * @param minor release version
		 * @param sub additional version when needed (don't display if sub parameter is 0)
		 * @param patch patch number for release
		 * @param build build number of release  (don't display if build parameter is 0)
		 * @param revision revision number of release (don't display if revision parameter is 0)
		 * @param distribution added string for distribution who compiling release
		 * @param nsub prefix for sub revision number to filled with null's ('0') before.<br /> (default:2)<br /> if nsub and sub parameter is 0, no sub version will be displayed
		 * @param nbuild prefix for sub revision number to filled with null's ('0') before (default:5)
		 * @param nrevision prefix for sub revision number to filled with null's ('0') before (default:5)
		 */
		void version(unsigned int major, unsigned int minor, unsigned int sub,
				unsigned int patch, unsigned int build, unsigned int revision, const string& distribution= "",
				unsigned int nsub= 2, unsigned int nbuild= 5, unsigned int nrevision= 5)
		{ version("version", "v", major, minor, sub, patch, build, revision, distribution, nsub, nbuild, nrevision); };
		/**
		 * set version option to output on command line when needed.<br />
		 * Version option is for standard '--version' with short definition '-v'
		 *
		 * @param major major release version
		 * @param minor release version
		 * @param patch patch number for release (default:0)
		 */
		void version(unsigned int major, unsigned int minor, unsigned int patch= 0)
		{ version(major, minor, /*sub version*/0, patch, /*no build*/0, /*no revision*/0, /*no distribution*/"", /*no sub version*/0); };
		/**
		 * return created string for Version when version be set with method <code>version(...)</code>
		 *
		 * @return version string
		 */
		string getVersion()
		{ return m_sVersionString; };
		/**
		 * returning path from actual directory to binary.
		 *
		 * @return path string
		 */
		string getPath() const
		{ return m_sPath; };
		/**
		 * returning name of application
		 *
		 * @return name of application
		 */
		string getAppName() const
		{ return m_sAppName; };
		/**
		 * set description for application.<br />
		 * Descriptions for more rows can inherit with carriage return ('\n')
		 *
		 * @param desc description string
		 */
		void setDescription(const string desc)
		{ m_sDescription= desc; };
		/**
		 * allocate help option.<br />
		 * If this method not ve called, default is <code>help("help", 'h?')</code>
		 *
		 * @param name name of help option
		 * @param sh one or more character options for help
		 * @param desc description for help option (default= 'show this help')
		 */
		void help(const string name, const string sh, const string desc= "show this help")
		{ m_sHelpOption= name; option(name, sh, false, desc); };
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
		 * return how many main options be set
		 *
		 * @return count of options
		 */
		size_t optioncount() const
		{ return m_vsOptions.size(); };
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
		 * print usage of application on command line when help option be set
		 *
		 * @return whether usage was printing
		 */
		bool usage();
		/**
		 * return all errors in an map founded by typed options and or commands
		 *
		 * @return map of errors
		 */
		map<string, string> getParmeterErrors() const
		{ return m_mErrors; };
		/**
		 * execute all parameter and
		 * print error message on command line when wrong commands or options be set
		 *
		 * @return whether any error occurs
		 */
		bool error();
		/**
		 * execute all parameters, write usage when help option be set
		 * or print error message when any wrong parameter be set
		 * and stopping in this cases inside of method
		 *
		 * @param stop whether method should stop application and writing out errors or usage
		 * @return whether an error occures
		 */
		bool execute(bool stop= true);
		/**
		 * question for main option (as first) be called
		 *
		 * @param option name of option (no short defined option)
		 * @return whether option be called
		 */
		virtual bool hasOption(const string& option) const;
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
		 * return pattern of first called command which inherit
		 * all other commands and also options
		 *
		 * @return first command
		 */
		const ICommandStructPattern* getCommands() const
		{ return &m_oCommand; };

	private:
		/**
		 * option name of help.<br />
		 * If this name not be set, default options are --help, -h and -?
		 */
		string m_sHelpOption;
		/**
		 * path to binary
		 */
		string m_sPath;
		/**
		 * name of application
		 */
		string m_sAppName;
		/**
		 * description of application
		 */
		string m_sDescription;
		/**
		 * all parameter from constructor
		 */
		vector<string> m_vsParams;
		/**
		 * whether one short definition of options is longer than one character,
		 * so cannot make more options after one minus (exp. cannot make -abc is options -a -b -c)
		 */
		bool m_bLongShort;
		/**
		 * string for version to bring for output when set
		 */
		string m_sVersionString;
		/**
		 * option name
		 */
		string m_sVersionOption;
		/**
		 * vector of all allowed main options
		 */
		map<string, t_options> m_mstAllowedOptions;
		/**
		 * Order of all options
		 */
		vector<string> m_vsOrder;
		/**
		 * vector of all allowed commands
		 */
		vector<SHAREDPTR::shared_ptr<ParamCommand> > m_vtAllowdCommands;
		/**
		 * vector of all readed main options
		 */
		vector<string> m_vsOptions;
		/**
		 * read command from parameter
		 */
		ParamCommand m_oCommand;
		/**
		 * whether any getOptionContent has an error
		 */
		bool m_bError;
		/**
		 * map of all incorrect commands and options
		 */
		map<string, string> m_mErrors;

		/**
		 * write all options from given map container on command line
		 *
		 * @param inserted order of all options
		 * @param mOptions map of options
		 * @param command name of specific command when it is not the first (default:'' for first)
		 * @param count number of command cycle (default:0)
		 * @return whether options be written
		 */
		bool optionUsage(vector<string> order, map<string, t_options> mOptions, const string& command= "", string::size_type count= 0);
		/**
		 * write all commands and sub commands with options on command line
		 *
		 * @param vCommands vector of commands
		 * @param command name of specific command when it is not the first (default:'' for first)
		 * @param count number of command cycle (default:0)
		 * @return whether commands be written
		 */
		bool commandUsage(vector<SHAREDPTR::shared_ptr<ParamCommand> > vCommands, const string& command= "", string::size_type count= 0);
	};

}

#endif /* MAINPARAMS_H_ */
