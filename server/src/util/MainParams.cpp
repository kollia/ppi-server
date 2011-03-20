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

	void MainParams::spaceline(const string& text/*= ""*/)
	{
		ostringstream space;
		t_options option;
		SHAREDPTR::shared_ptr<ParamCommand> spacecommand;
		static unsigned short opnr(0);

		space << "---freespace";
		if(m_vtAllowdCommands.size() == 0)
		{
			option.bcontent= false;
			option.name= space.str();
			option.description= text;
			space << opnr;
			m_vsOrder.push_back(space.str());
			m_mstAllowedOptions[space.str()]= option;
			++opnr;

		}else
		{
			spacecommand= SHAREDPTR::shared_ptr<ParamCommand>(new ParamCommand);
			spacecommand->m_bLongShort= false;
			spacecommand->m_bContent= false;
			spacecommand->name= space.str();
			spacecommand->description= text;
			m_vtAllowdCommands.push_back(spacecommand);
		}
	}

	void MainParams::option(const string& name, const string& sh, const bool content, const string& desc)
	{
		bool exist(false);
		t_options option;
		vector<string>::iterator shIt;
		map<string, t_options>::iterator opIt, op2It;


		// get option iterator from command
		opIt= m_mstAllowedOptions.find(name);
		if(opIt == m_mstAllowedOptions.end())
		{
			option.name= name;
			option.bcontent= false;
			m_vsOrder.push_back(name);
			m_mstAllowedOptions[name]= option;
			opIt= m_mstAllowedOptions.find(name);
		}
		if(sh.length() > 1)
			m_bLongShort= true;
		for(op2It= m_mstAllowedOptions.begin(); op2It != m_mstAllowedOptions.end(); ++op2It)
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

	ParamCommand* MainParams::command(string name, bool content, string desc)
	{
		SHAREDPTR::shared_ptr<ParamCommand> newcommand;

		for(vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator it= m_vtAllowdCommands.begin(); it != m_vtAllowdCommands.end(); ++it)
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
		m_vtAllowdCommands.push_back(newcommand);
		return m_vtAllowdCommands.back().get();
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
			for(vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator it= m_vtAllowdCommands.begin(); it != m_vtAllowdCommands.end(); ++it)
			{
				if(!(*it)->options.empty())
				{
					cout << " [specific options for command]";
					break;
				}
			}
			bUseCont= false;
			bAllCont= true;
			for(vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator it= m_vtAllowdCommands.begin(); it != m_vtAllowdCommands.end(); ++it)
			{
				if((*it)->m_bContent)
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

	bool MainParams::commandUsage(vector<SHAREDPTR::shared_ptr<ParamCommand> > vCommands, const string& command, string::size_type count)
	{
		bool out;
		string nullstr("\n");
		string space(count*2, ' ');
		string::size_type len, nCoMax(0);
		vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator it;
		const string spaceline("---freespace");

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
			if((*it)->name != spaceline)
			{
				len= count + 4 + (*it)->name.length();
				if(nCoMax < len)
					nCoMax= len;
			}
		}
		nCoMax+= 4;
		nullstr.append(count*2 + nCoMax + 2, ' ');
		for(it= vCommands.begin(); it != vCommands.end(); ++it)
		{
			if((*it)->name != spaceline)
			{
				cout << space << "    ";
				cout << (*it)->name;
				len= nCoMax - (count + 4 + (*it)->name.length());
				cout << string(len, ' ') << "- ";
				replace_all((*it)->description, "\n", nullstr);
				cout << (*it)->description << endl;
				out= optionUsage((*it)->order, (*it)->options, (*it)->name, count+1);
				if(	commandUsage((*it)->next_commands, (*it)->name, count+1) ||
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
			}else
			{
				len= nCoMax - (count + 4);
				replace_all((*it)->description, "\n", "\n" + space + "   ");
				cout << space << "   ";
				cout << (*it)->description << endl;
			}
		}
		return true;
	}

	bool MainParams::optionUsage(vector<string> order, map<string, t_options> mOptions, const string& command, string::size_type count)
	{
		const string spaceline("---freespace");
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

			if(orderIt->substr(0, 12) != spaceline)
			{
				it= mOptions.find(*orderIt);
				for(vector<string>::iterator op= it->second.shdefs.begin(); op != it->second.shdefs.end(); ++op)
					shop+= "-" + *op + " ";
				len= shop.length();
				if(len > nOpMax)
					nOpMax= len;
				options.push_back(shop);
			}
		}
		opIt= options.begin();
		nOpMax+= 1;
		for(orderIt= order.begin(); orderIt != order.end(); ++orderIt)
		{
			string opstr;
			string::size_type len;

			if(orderIt->substr(0, 12) != spaceline)
			{
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
		}
		opIt= options2.begin();
		nOpMax2+= 4;
		nullstr.append(count*2 + nOpMax2 + 6, ' ');
		for(orderIt= order.begin(); orderIt != order.end(); ++orderIt)
		{
			string opstr;
			string::size_type add;

			it= mOptions.find(*orderIt);
			if(orderIt->substr(0, 12) != spaceline)
			{
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
			}else
			{
				opstr= it->second.description;
				replace_all(opstr, "\n", "\n" + space + "   ");
				cout << space << "   ";
				cout << opstr << endl;
			}
		}
		return true;
	}

	void MainParams::version(const string& soption, const string& def, unsigned int major, unsigned int minor, unsigned int sub,
			unsigned int patch, unsigned int build, unsigned int revision, const string& distribution/*= ""*/,
			unsigned int nsub/*= 2*/, unsigned int nbuild/*= 5*/, unsigned int nrevision/*= 5*/)
	{
		int count;
		string subnulls, buildnulls, revisionnulls;
		ostringstream version, substream, buildstream, revisionstream;

		version << m_sAppName << " ";
		version << major << ".";
		version << minor << ".";
		if( sub ||
			nsub	)
		{
			substream << build;
			count= nsub - substream.str().length();
			if(count>0)
				subnulls.append(count, '0');
			version << subnulls << sub << ".";
		}
		version << patch;
		if(build != 0)
		{
			buildstream << build;
			count= nbuild - buildstream.str().length();
			if(count>0)
				buildnulls.append(count, '0');
			version << "." << buildnulls << build;
		}
		if(revision != 0)
		{
			revisionstream << revision;
			count= revisionstream.str().length();
			count= nrevision -revisionstream.str().length();
			if(count>0)
				revisionnulls.append(count, '0');
			version << "." << revisionnulls << revision;
		}
		if(distribution != "")
			version << "_" << distribution;
		version << endl;
		version << "  version ";
		version << major << ".";
		version << minor;
		if(sub || nsub)
			version << "." << subnulls << sub;
		version << " ";
		if(patch > 0)
			version << "patch " << patch << " ";
		if(build > 0)
			version << "build " << buildnulls << build << " ";
		if(revision > 0)
			version << " revision " << revisionnulls << revision << " ";
		if(distribution != "")
			version << " for distribution " << distribution;
		version << endl << endl;
		m_sVersionString= version.str();
		m_sVersionOption= soption;
		option(soption, def, "show information of actual version");
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
		vector<SHAREDPTR::shared_ptr<ParamCommand> > *actCommands;
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
				vector<SHAREDPTR::shared_ptr<ParamCommand> >::iterator comIt;
				SHAREDPTR::shared_ptr<ParamCommand> newCommand;

				//comIt= find(actCommands->begin(), actCommands->end(), newCommand);
				for(comIt= actCommands->begin(); comIt != actCommands->end(); ++comIt)
				{
					if((*comIt)->name == *it)
						break;
				}
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
					m_oCommand.name= (*comIt)->name;
					m_oCommand.m_bContent= (*comIt)->m_bContent;
					psetCommand= &m_oCommand;
				}else
				{
					newCommand= SHAREDPTR::shared_ptr<ParamCommand>(new ParamCommand);
					newCommand->name= (*comIt)->name;
					newCommand->m_bContent= (*comIt)->m_bContent;
					psetCommand->next_commands.push_back(newCommand);
					psetCommand= psetCommand->next_commands[0].get();
				}
				actOptions= &(*comIt)->options;
				actCommands= &(*comIt)->next_commands;
			}// else branch of if(it->substr(0, 1) == "-")
		}// for(m_vsParams)

		if(stop)
		{
			if(error())
				exit(EXIT_FAILURE);
			if(usage())
				exit(EXIT_SUCCESS);
			if(	m_sVersionOption != "" &&
				hasOption(m_sVersionOption)	)
			{
				cout << m_sVersionString;
				exit(EXIT_SUCCESS);
			}
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

	int MainParams::getOptionIntContent(string& option) const
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

	float MainParams::getOptionFloatContent(string& option) const
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
