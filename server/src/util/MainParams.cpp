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

#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <queue>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "MainParams.h"

namespace util
{
	using namespace boost;

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
		return &next_commands[0];
	}

	MainParams::MainParams(int argc, char* argv[], vector<string>::size_type nParent /*= 0*/)
	:	m_bLongShort(false),
	 	m_bError(false)
	{
		vector<string> directorys;
		vector<string>::size_type dirlen;

		// create working directory
		directorys= split(directorys, argv[0], is_any_of("/"));
		m_sAppName= directorys.back();
		dirlen= directorys.size();
		if(dirlen < nParent+1)
		{
			if(directorys.front() == ".")
				m_sPath+= "../";
			for(vector<string>::size_type n= 0; n<(nParent-dirlen+1); ++n)
				m_sPath+= "../";

		}else if(dirlen == nParent+1)
		{
			if(directorys.front() == ".")
				m_sPath+= "../";
		}else
		{
			for(vector<string>::size_type c= 0; c < dirlen; ++c)
			{
				if(c == dirlen-1-nParent)
					break;
				m_sPath+= directorys[c] + "/";
			}
		}
		// read all other parameters
		for(int n= 1; n < argc; ++n)
			m_vsParams.push_back(argv[n]);
	}

	void MainParams::option(string command_id, const string& name, const string& sh, const bool content, const string& desc)
	{
		t_options option;
		ParamCommand *command;
		map<string, t_options>::iterator opIt;

		trim(command_id);
		option.bcontent= false;
		if(command_id == "")
		{
			// get main option iterator from
			opIt= m_mstAllowedOptions.find(name);
			if(opIt == m_mstAllowedOptions.end())
			{
				m_vsOrder.push_back(name);
				m_mstAllowedOptions[name]= option;
				opIt= m_mstAllowedOptions.find(name);
			}
			if(sh.length() > 1)
				m_bLongShort= true;
		}else
		{
			vector<ParamCommand>::size_type pos;
			istringstream cid(command_id);

			// search first right command
			cid >> pos;
			if(pos+1 > m_vtAllowdCommands.size())
			{
				cerr << "### ERROR: fault command_id be set" << endl;
				return;
			}
			command= &m_vtAllowdCommands[pos];
			while(!cid.eof())
			{
				cid >> pos;
				if(pos+1 > command->next_commands.size())
				{
					cerr << "### ERROR: fault command_id be set" << endl;
					return;
				}
				command= &command->next_commands[pos];
			}
			// get option iterator from command
			opIt= command->options.find(name);
			if(opIt == command->options.end())
			{
				command->order.push_back(name);
				command->options[name]= option;
				opIt= command->options.find(name);
			}
			if(sh.length() > 1)
				command->m_bLongShort= true;
		}
		opIt->second.name= name;
		opIt->second.shdefs.push_back(sh);
		if(content)
			opIt->second.bcontent= true;
		if(desc != "")
			opIt->second.description= desc;
	}

	const string MainParams::command(const string command_id, string name, bool content, string desc)
	{
		string sRv;
		ostringstream id;
		istringstream cid(command_id);
		ParamCommand *command, newcommand;

		if(command_id != "")
		{
			unsigned short pos;

			cid >> pos;
			command= &m_vtAllowdCommands[pos];
			cid >> pos;
			while(pos != 0)
			{
				command= &command->next_commands[pos];
				cid >> pos;
			}
			id << command->m_id << " ";
			id << command->next_commands.size() << " ";

		}else
			id << m_vtAllowdCommands.size() << " ";
		newcommand.m_id= id.str();
		newcommand.name= name;
		newcommand.m_bContent= content;
		newcommand.description= desc;
		if(command_id == "")
			m_vtAllowdCommands.push_back(newcommand);
		else
			command->next_commands.push_back(newcommand);
		return newcommand.m_id;
	}

	bool MainParams::usage()
	{
		bool bUseCont;
		bool bAllCont;

		if(	(	m_oCommand.name == "" &&
				m_vsOptions.empty()		) ||
			!hasOption(m_sHelpOption)		)
		{
			return false;
		}

		cout << endl;                        // --help option is always set
		cout << "syntax: " << m_sAppName << " [options]";
		if(!m_vtAllowdCommands.empty())
		{
			cout << " <commands>";
			for(vector<ParamCommand>::iterator it= m_vtAllowdCommands.begin(); it != m_vtAllowdCommands.end(); ++it)
			{
				if(!it->options.empty())
				{
					cout << " [specific options for command]";
					break;
				}
			}
			bUseCont= false;
			bAllCont= true;
			for(vector<ParamCommand>::iterator it= m_vtAllowdCommands.begin(); it != m_vtAllowdCommands.end(); ++it)
			{
				if(it->m_bContent)
					bUseCont= true;
				else
					bAllCont= false;
			}
			if(bUseCont)
			{
				if(bAllCont)
					cout << " <command content>";
				else
					cout << " [command content]";
			}
		}
		cout << endl;
		if(m_sDescription != "")
		{
			replace_all(m_sDescription, "\n", "\n    ");
			cout << endl << "    " << m_sDescription << endl;
		}
		optionUsage(m_vsOrder, m_mstAllowedOptions);
		commandUsage(m_vtAllowdCommands);
		cout << endl;
		return true;
	}

	bool MainParams::commandUsage(vector<ParamCommand> vCommands, const string& command, string::size_type count)
	{
		bool out;
		string nullstr("\n");
		string space(count*2, ' ');
		string::size_type len, nCoMax(0);
		vector<ParamCommand>::iterator it;

		if(vCommands.empty())
			return false;
		cout << endl;
		cout << space;
		if(command != "")
		{
			cout << "     specific commands for " << command << ":";
			space.append(2, ' ');
			++count;
		}else
			cout << "  commands:";
		cout << endl;
		for(it= vCommands.begin(); it != vCommands.end(); ++it)
		{
			len= count + 4 + it->name.length();
			if(nCoMax < len)
				nCoMax= len;
		}
		nCoMax+= 4;
		nullstr.append(count*2 + nCoMax + 2, ' ');
		for(it= vCommands.begin(); it != vCommands.end(); ++it)
		{
			cout << space << "    " << it->name;
			len= nCoMax - (count + 4 + it->name.length());
			cout << string(len, ' ') << "- ";
			replace_all(it->description, "\n", nullstr);
			cout << it->description << endl;
			out= optionUsage(it->order, it->options, it->name, count+1);
			if(	commandUsage(it->next_commands, it->name, count+1) ||
				out														)
			{
				cout << endl << endl;
#if 0
				if(command != "")
					cout << "     specific commands for " << command << ":";
				else
					cout << "  commands:";
#endif
			}
		}
		return true;
	}

	bool MainParams::optionUsage(vector<string> order, map<string, t_options> mOptions, const string& command, string::size_type count)
	{
		string space(count*2, ' ');
		string nullstr("\n");
		string::size_type nOpMax(0), nOpMax2(0);
		vector<string> options, options2;
		vector<string>::iterator opIt;
		vector<string>::iterator orderIt;
		map<string, t_options>::iterator it;

		if(mOptions.empty())
			return false;
		cout << endl;
		cout << space;
		if(command != "")
		{
			cout << "    specific options for  " << command << ":";
			space.append(2, ' ');
			++count;
		}else
			cout << "  options:";
		cout << endl;
		for(orderIt= order.begin(); orderIt != order.end(); ++orderIt)
		{
			string shop;
			string::size_type len;

			it= mOptions.find(*orderIt);
			for(vector<string>::iterator op= it->second.shdefs.begin(); op != it->second.shdefs.end(); ++op)
				shop+= "-" + *op + " ";
			len= shop.length();
			if(len > nOpMax)
				nOpMax= len;
			options.push_back(shop);
		}
		opIt= options.begin();
		nOpMax+= 1;
		for(orderIt= order.begin(); orderIt != order.end(); ++orderIt)
		{
			string opstr;
			string::size_type len;

			it= mOptions.find(*orderIt);
			opstr= *opIt;
			opstr.append((nOpMax - opIt->length()), ' ');
			opstr+= "--" + it->first;
			options2.push_back(opstr);
			len= opstr.length();
			if(len > nOpMax2)
				nOpMax2= len;
			++opIt;
		}
		opIt= options2.begin();
		nOpMax2+= 4;
		nullstr.append(count*2 + nOpMax2 + 6, ' ');
		for(orderIt= order.begin(); orderIt != order.end(); ++orderIt)
		{
			string opstr;
			string::size_type add;

			it= mOptions.find(*orderIt);
			opstr= *opIt;
			add= nOpMax2;
			add-= opstr.length();
			opstr.append(add, ' ');
			opstr+= "- ";
			cout << space << "    " << opstr << flush;
			opstr= it->second.description;
			replace_all(opstr, "\n", nullstr);
			cout << opstr << endl;
			++opIt;
		}
		return true;
	}

	bool MainParams::error()
	{
		string help;
		t_options ohelp;

		if(m_mErrors.empty())
			return false;

		for(map<string, string>::iterator it= m_mErrors.begin(); it != m_mErrors.end(); ++it)
			cerr << it->second << endl;
		ohelp= m_mstAllowedOptions[m_sHelpOption];
		if(ohelp.shdefs.empty())
			help= "--" + m_sHelpOption;
		else
			help= "-" + ohelp.shdefs.front();
		cerr << "  start ./" << m_sAppName << " with option " << help << " for more description" << endl;
		return true;
	}

	bool MainParams::execute(bool stop/*= true*/)
	{
		string err;
		t_options emptyOp;
		vector<ParamCommand> *actCommands;
		map<string, t_options> *actOptions;
		ParamCommand *psetCommand= NULL;
		map<string, t_options>::iterator setOption;
		map<string, t_options>::iterator opIt;
		queue<t_options*> optionContent;

		m_oCommand.name= "";
		m_oCommand.options.clear();
		m_oCommand.next_commands.clear();
		m_vsOptions.clear();
		if(m_sHelpOption == "")
		{
			help("help", "?");
			help("help", "h");
		}
		actCommands= &m_vtAllowdCommands;
		actOptions= &m_mstAllowedOptions;
		for(vector<string>::iterator it= m_vsParams.begin(); it != m_vsParams.end(); ++it)
		{
			if(it->substr(0, 2) == "--")
			{// search for options with full length
				opIt= actOptions->find(it->substr(2));
				if(opIt == actOptions->end())
				{
					err= "wrong option " + *it;
					if(psetCommand != NULL)
						err+= " after command " + psetCommand->name;
					err+= " be set";
					m_mErrors[*it]= err;

				}else
				{
					if(psetCommand == NULL)
					{
						m_vsOptions.push_back(it->substr(2));
						if(opIt->second.bcontent)
							optionContent.push(&opIt->second);
					}else
					{
						psetCommand->options[it->substr(2)]= emptyOp;
						if(opIt->second.bcontent)
							optionContent.push(&psetCommand->options[it->substr(2)]);
					}
				}

			}else if(it->substr(0, 1) == "-")
			{// search for options with short definition
				bool found(false);

				if(	(	psetCommand == NULL &&
						m_bLongShort			) ||
					(	psetCommand != NULL &&
						psetCommand->m_bLongShort	)	)
				{ // short definition of options where have one more than one characters
					for(opIt= actOptions->begin(); opIt != actOptions->end(); ++opIt)
					{
						for(vector<string>::iterator op= opIt->second.shdefs.begin(); op != opIt->second.shdefs.end(); ++op)
						{
							if(it->substr(1) == *op)
							{
								found= true;
								break;
							}
						}
						if(found)
							break;
					}
					if(found)
					{
						if(psetCommand == NULL)
						{
							m_vsOptions.push_back(opIt->first);
							if(opIt->second.bcontent)
								optionContent.push(&opIt->second);
						}else
						{
							psetCommand->options[opIt->first]= emptyOp;
							if(opIt->second.bcontent)
								optionContent.push(&psetCommand->options[opIt->first]);
						}
					}else
					{
						err= "wrong option " + *it;
						if(psetCommand != NULL)
							err+= " after command " + psetCommand->name;
						err+= " be set";
						m_mErrors[*it]= err;
					} // for(actOptions)
				}else // if(m_bLongShort)
				{ // short definition of options where all have only one character
					for(string::size_type count= 1; count < it->length(); ++count)
					{
						string comp(*it);

						comp= comp[count];
						found= false;
						for(opIt= actOptions->begin(); opIt != actOptions->end(); ++opIt)
						{
							for(vector<string>::iterator op= opIt->second.shdefs.begin(); op != opIt->second.shdefs.end(); ++op)
							{
								if(*op == comp)
								{
									found= true;
									break;
								}
							}
							if(found)
								break;
						}
						if(found)
						{
							if(psetCommand == NULL)
							{
								m_vsOptions.push_back(opIt->first);
								if(opIt->second.bcontent)
									optionContent.push(&opIt->second);
							}else
							{
								psetCommand->options[opIt->first]= emptyOp;
								if(opIt->second.bcontent)
									optionContent.push(&psetCommand->options[opIt->first]);
							}
						}else
						{
							err= "wrong option -" + comp;
							if(psetCommand != NULL)
								err+= " after command " + psetCommand->name;
							err+= " be set";
							m_mErrors["-"+comp]= err;
						} // for(actOptions)
						if(!found)
							break;
					} // for(string of m_vParams)
				}// else branch of if(m_bLongShort)
			}/*if("-")*/else if(!optionContent.empty())
			{
				t_options* op;

				op= optionContent.front();
				op->content= *it;
				optionContent.pop();

			}/*if(optionContent)*/else if(	psetCommand &&
											psetCommand->m_bContent	&&
											psetCommand->m_sContent == ""	)
			{
				psetCommand->m_sContent= *it;

			}/*if(command content)*/ else
			{
				// search for commands
				vector<ParamCommand>::iterator comIt;
				ParamCommand newCommand;

				newCommand.name= *it;
				comIt= find(actCommands->begin(), actCommands->end(), newCommand);
				if(comIt == actCommands->end())
				{
					err= "no correct command '" + *it + "'";
					if(psetCommand != NULL)
						err+= " after command " + psetCommand->name;
					err+= " be set";
					m_mErrors[*it]= err;
					break;
				}
				if(m_oCommand.name == "")
				{
					m_oCommand.name= comIt->name;
					m_oCommand.m_bContent= comIt->m_bContent;
					psetCommand= &m_oCommand;
				}else
				{
					newCommand.name= comIt->name;
					newCommand.m_bContent= comIt->m_bContent;
					psetCommand->next_commands.push_back(newCommand);
					psetCommand= &psetCommand->next_commands[0];
				}
				actOptions= &comIt->options;
				actCommands= &comIt->next_commands;
			}// else branch of if(it->substr(0, 1) == "-")
		}// for(m_vsParams)

		if(stop)
		{
			if(error())
				exit(EXIT_FAILURE);
			if(usage())
				exit(EXIT_SUCCESS);
			return false;
		}
		if(m_mErrors.empty())
			return false;
		return true;
	}

	bool MainParams::hasOption(const string& option) const
	{
		vector<string>::const_iterator op;

		op= find(m_vsOptions.begin(), m_vsOptions.end(), option);
		if(op != m_vsOptions.end())
			return true;
		return false;
	}

	string MainParams::getOptionContent(const string& option) const
	{
		map<string, t_options>::const_iterator found;

		found= m_mstAllowedOptions.find(option);
		if(found != m_mstAllowedOptions.end())
			return found->second.content;
		return "";
	}

	int MainParams::getOptionIntContent(string option) const
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

	float MainParams::getOptionFloatContent(string option) const
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
}
