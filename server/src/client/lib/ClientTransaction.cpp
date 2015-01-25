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
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../../util/debug.h"
#include "../../util/XMLStartEndTagReader.h"
#include "../../util/GlobalStaticMethods.h"
#include "../../util/structures.h"
#include "../../util/URL.h"
#include "../../util/exception.h"

#include "../../util/stream/ErrorHandling.h"
#include "../../util/stream/IParameterStringStream.h"

#include "ClientTransaction.h"
#include "hearingthread.h"

using namespace std;
using namespace util;
using namespace boost;

namespace server
{
	ClientTransaction::ClientTransaction(vector<string> options, string command)
	: /*m_mOwDevices(),*/ m_mOwMaxTime(), m_mOwMaxCount()
	{
		m_bConnected= false;
		m_bErrWritten= false;
		m_bWait= false;
		m_bShowENum= false;
		m_vOptions= options;
		m_sCommand= command;
		m_bHearing= false;
		m_bOwDebug= false;
		m_fProtocol= 0;
		m_bHoldAll= false;
		m_DEBUGSESSIONCHANGES= Thread::getMutex("DEBUGSESSIONCHANGES");
		m_PROMPTMUTEX= Thread::getMutex("PROMPTMUTEX");
	}

	EHObj ClientTransaction::init(IFileDescriptorPattern& descriptor)
	{
		EHObj errHandle(EHObj(new ErrorHandling));
		short x= 0;
		/**
		 * unique ID get from server
		 */
		string sCommID;
		string result, logMsg, user, pwd;
		string sSendbuf;
		ostringstream sget;
		bool bOp= true;
		bool bSecConn= false;
		bool bRightServer= true;
		bool bLogin= true;
		bool bStatus= false;
		unsigned int nOptions= m_vOptions.size();
		unsigned short owserver;

		sget << "GET v";
		sget.setf(ios_base::fixed, ios_base::floatfield);
		sget.precision(2);
		sget << PPI_SERVER_PROTOCOL;
		sSendbuf= sget.str();
		if(m_sCommand == "status")
			bStatus= true;
		for(unsigned int o= 0; o<nOptions; ++o)
		{
			if(bStatus)
			{
				if(m_vOptions[o] == "-t")
					m_sCommand+= " threads";
				else if(m_vOptions[o] == "-c")
					m_sCommand+= " clients";
				else if(m_vOptions[o] == "-p")
					m_sCommand+= " pid";
				else
					bOp= false;

			}else
			{
				if(m_vOptions[o] == "-w")
					m_bWait= true;
				else if(m_vOptions[o] == "-h")
				{
					m_bWait= true;
					bSecConn= true;

				}else if(m_vOptions[o] == "-e")
					m_bShowENum= true;
				else if(m_vOptions[o].substr(0, 2) == "-u")
					user= m_vOptions[o].substr(3);
				else if(m_vOptions[o].substr(0, 2) == "-p")
					pwd= m_vOptions[o].substr(3);
				else if(m_vOptions[o].substr(0, 3) == "-id")
				{
					sCommID= m_vOptions[o].substr(4);
					m_bHearing= true;
				}else if(m_vOptions[o] == "-ow")
					m_bOwDebug= true;
				else
					bOp= false;
			}
		}
		if(	!m_bWait
			&&
			(	m_sCommand == "GETMINMAXERRORNUMS"
				||
				m_sCommand.substr(0, 15) == "GETERRORSTRING "	)	)
		{
			bLogin= false;
		}
		if(!bOp)
		{
			cerr << "### WARNING: not all options are for all commands," << endl;
			cerr << "             see -? for help" << endl;
		}
		if(m_sCommand.substr(0, 5) == "DEBUG")
		{
			string ip;
			istringstream op(m_sCommand);

			op >> ip; // DEBUG
			op >> ip;
			if(ip == "-ow")
			{
				op >> owserver;
				if(owserver != 0)
				{
					m_bWait= true;
					bSecConn= true;
					m_bOwDebug= true;
				}else
				{
					m_bWait= false;
					bSecConn= false;
				}
			}
		}
		if(m_bHearing)
		{
			sSendbuf+= " ID:";
			sSendbuf+= sCommID;
		}
		sSendbuf+= "\n";
	while(x < 3)
	{
#ifdef SERVERDEBUG
	std::cout << "send: " << sSendbuf;
#endif // SERVERDEBUG
		descriptor << sSendbuf;
		descriptor.flush();

		descriptor >> result;
		result= ConfigPropertyCasher::trim(result);
#ifdef SERVERDEBUG
	std::cout << "get: " << result << endl;
#endif // SERVERDEBUG

		if(	result.length() < 18
			||
			result.substr(0, 11) != "ppi-server:"	)
		{
			bRightServer= false;
		}else
		{
			istringstream res(result.substr(11));

			res >> sCommID;
			result= res.str();

			if(!res.fail())
			{
				istringstream res2(result);

				res2 >> m_fProtocol;
				if(res2.fail())
					bRightServer= false;
#if 0
				else
				{
					if(m_bHearing)
					{
						runHearingTransaction(true);
						std::cout << "\n make second ";
					}else
						std::cout << "make ";
					std::cout << "connect to server with ID " << sCommID << endl;
					if(m_bHearing)
					{
						runHearingTransaction(false);
						prompt();
					}
				}
#endif
			}else
				bRightServer= false;
		}
		if(	!bRightServer
			&&
			result == ""	)
		{
			std::cout << "ERROR: get no result from server\ntry again later" << endl;
			++x;
			sleep(1);
		}else
			break;
	}
	if(	!bRightServer
		&&
		result == ""	)
	{
		m_bErrWritten= true;
		errHandle->setError("ClientTransaction", "get_result");
		cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
		return errHandle;
	}
		if(!bRightServer)
		{
			string::size_type nPos;
			ostringstream decl;

			decl << descriptor.getHostAddressName() << "@" << descriptor.getPort();
			errHandle->setErrorStr(result);
			if(!errHandle->fail())
			{
				if(result.length() > 20)
					result= result.substr(0, 20) + " ...";
				nPos= result.find('\n');
				if(nPos != string::npos)
					result= result.substr(0, nPos-1) + " ...";
				decl << "@" << result;
				errHandle->setError("ClientTransaction", "undefined_server", decl.str());
			}else
			{
				errHandle->addMessage("ClientTransaction", "client_send", decl.str());
			}
			if(errHandle->hasError())
				cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
			else
				std::cout << glob::addPrefix("WARNING: ", errHandle->getDescription()) << endl;
			m_bErrWritten= true;
			return errHandle;
		}

		if(bStatus)
		{
			char stime[20];
			time_t t;

			t= time(NULL);
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", localtime(&t));
			std::cout << endl << "actual time " << stime << endl;
			bLogin= false;
		}
		if(	m_bWait ||
			bLogin		)
		{
			readTcBackup();
			if(!compareUserPassword(descriptor, user, pwd))
			{
				errHandle->setError("ClientTransaction", "user_password_error");
				errHandle->addMessage("ClientTransaction", "get_result");
				return errHandle;
			}
		}
		if(bSecConn)
		{
			HearingThread* pThread;

			pThread= new HearingThread(descriptor.getHostAddressName(), descriptor.getPort(),
							sCommID, user, pwd, m_bOwDebug);
			m_o2Client= auto_ptr<IHearingThreadPattern>(pThread);
			(*errHandle)= pThread->start();
		}
		if(	!m_bHearing &&
			!errHandle->hasError()	)
		{
			// helping commands
			command("?");
			command("?value");
			command("?debug");

			// editor commands
			command("quit");
			command("exit");
			command("history");

			// editor commands for debug session
			command("run");
			command("first");
			command("last");
			command("up");
			command("back");
			command("clear");
			command("add #subroutine");
			command("remove #subroutine");
			command("save #file");
			command("load #file");

			command("hold #folderSub");
			command("hold -i #folderSub");

			command("show");
			command("show #folder");
			command("show #folderSub");

			command("current");
			command("current #folder");
			command("current #folderSub");

			command("next changed #subroutine");
			command("next unchanged #subroutine");

			command("previous");
			command("previous changed #subroutine");
			command("previous unchanged #subroutine");


			// commands sending to server
			command("CHANGE #string");
			command("PERMISSION #folderSub");
			command("GET #folderSub");
			command("SET #folderSub #string");
			command("HEAR #folderSub");
			command("NEWENTRY");
			command("DIR #string");
			command("CONTENT #string");
			command("GETERRORSTRING #string");

			command("DEBUG #folderSub");
			command("DEBUG -i #folderSub");
			command("DEBUG -ow #string");

			command("SHOW");
			command("SHOW #string");
			command("SHOW -c #string");

			command("STOPDEBUG #folderSub");
			command("STOPDEBUG -ow #string");
		}
		return errHandle;
	}

	void ClientTransaction::command(string command)
	{
		vector<string> spl;
		get_params_vec_t param, nextParam;

		trim(command);
		split(spl, command, is_any_of(" \t"), boost::token_compress_on);
		if(spl.empty())
			return;
		for(vector<string>::iterator it= spl.begin();
						it != spl.end(); ++it		)
		{
			if(it == spl.begin())
			{
				param.content= NULL;
				addCommand(*it, &param);

			}else
			{
				nextParam.content= NULL;
				addParam(&param, *it, &nextParam);
				param.content= nextParam.content;
			}
		}
	}

	void ClientTransaction::addCommand(const string& command, get_params_vec_t* params/*= NULL*/)
	{
		if(m_mUserInteraction.find(command) == m_mUserInteraction.end())
			m_mUserInteraction[command]= parameter_types();
		if(params != NULL)
		{
			if(params->content != NULL)
			{
				for(parameter_types::iterator it= params->content->begin();
								it != params->content->end(); ++it			)
				{
					m_mUserInteraction[command].push_back(*it);
				}
			}else
				params->content= &m_mUserInteraction[command];
		}
	}

	void ClientTransaction::addParam(get_params_vec_t* into, const string& parameter, get_params_vec_t* params/*= NULL*/)
	{
		bool bFound(false);
		parameter_type param;

		for(vector<parameter_type>::iterator it= into->content->begin();
						it != into->content->end(); ++it				)
		{
			if((*it)->param == parameter)
			{
				param= *it;
				bFound= true;
				break;
			}
		}
		if(!bFound)
		{
			param= parameter_type(new params_t);
			param->param= parameter;
			into->content->push_back(param);
		}
		if(params != NULL)
		{
			if(params->content != NULL)
			{
				for(parameter_types::iterator it= params->content->begin();
								it != params->content->end(); ++it			)
				{
					param->follow.push_back(*it);
				}
			}else
				params->content= &param->follow;
		}
	}

	void ClientTransaction::createTabResult(string& result, string::size_type& nPos, short& count)
	{
		bool bFound(false);
		/**
		 * whether allowed to write
		 * an various parameter string
		 * or number
		 */
		bool bNStr(false);
		/**
		 * whether should be folder names
		 * inside searching result
		 */
		bool bNFolder(false);
		/**
		 * whether should be subroutine names
		 * inside searching result
		 */
		bool bNSubs(false);
		/**
		 * whether should be folder with
		 * subroutines inside searching
		 * result
		 */
		bool bNFolderSubs(false);
		/**
		 * whether should be file names
		 * from current directory
		 * inside searching result
		 */
		bool bNFiles(false);
		/**
		 * whether an normal parameter found
		 */
		bool bFoundParameter(false);
		parameter_types* pCurParamVec;
		string folder, subroutine;
		string sCurStr, resBefore, resBehind;
		string::size_type nLen, nMaxLen, nNewPos(string::npos);
		vector<string> spl, searchVec;
		vector<string>::size_type nCommandCount;
		map<string, parameter_types >::iterator foundCommand;

		if(nPos >= result.length())
			resBefore= result;
		else if(nPos == 0)
			resBehind= result;
		else
		{
			resBefore= result.substr(0, nPos);
			resBehind= result.substr(nPos);
		}
		if(nPos > 0)
		{
			string dupl(resBefore);

			trim(dupl);
			split(spl, dupl, is_any_of(" "), boost::token_compress_on);
			if(resBefore.substr(resBefore.length() - 1, 1) != " ")
			{
				sCurStr= spl.back();
				spl.pop_back();
			}
		}

		nCommandCount= spl.size();
		if(!spl.empty())
		{
			foundCommand= m_mUserInteraction.find(spl[0]);
			if(foundCommand == m_mUserInteraction.end())
			{
				/*
				 * first command not found
				 * so question of any parameter
				 * after unknown command
				 * cannot find anything
				 */
				return;
			}
			pCurParamVec= &foundCommand->second;
			for(vector<string>::size_type n= 1; n < nCommandCount; ++n)
			{
				bFound= false;
				for(parameter_types::iterator it= pCurParamVec->begin();
								it != pCurParamVec->end(); ++it			)
				{
					if(	(*it)->param == spl[n] ||
						(*it)->param == "#string"	)
					{
						bFound= true;
						pCurParamVec= &(*it)->follow;
						break;
					}
				}
				if(bFound == false)
				{
					/*
					 * one parameter before not found
					 * so question of any parameter
					 * after unknown command
					 * cannot find anything
					 */
					return;
				}
			}
		}
		nLen= sCurStr.length();
		nMaxLen= 0;
		bFound= false;
		if(nCommandCount == 0)
		{
			/*
			 * search for commands
			 */
			for(map<string, parameter_types >::iterator it= m_mUserInteraction.begin();
							it != m_mUserInteraction.end(); ++it						)
			{
				if(	it->first.length() >= nLen &&
					sCurStr == it->first.substr(0, nLen)	)
				{
					searchVec.push_back(it->first);
					if(nMaxLen < it->first.length())
						nMaxLen= it->first.length();
					bFound= true;

				}else if(bFound)
					break;
			}
		}else
		{
			vector<string> vFolderSubs;

			/*
			 * search for parameter
			 * after command
			 */
			for(parameter_types::iterator it= pCurParamVec->begin();
							it != pCurParamVec->end(); ++it			)
			{
				if((*it)->param.substr(0, 1) != "#")
				{
					if(	(*it)->param.length() >= nLen &&
						sCurStr == (*it)->param.substr(0, nLen)	)
					{
						searchVec.push_back((*it)->param);
						if(nMaxLen < (*it)->param.length())
							nMaxLen= (*it)->param.length();
						bFoundParameter= true;

					}else if(bFoundParameter)
						break;
				}else
				{
					if((*it)->param == "#string")
						bNStr= true;
					else if((*it)->param == "#folder")
						bNFolder= true;
					else if((*it)->param == "#subroutine")
						bNSubs= true;
					else if((*it)->param == "#folderSub")
						bNFolderSubs= true;
					else if((*it)->param == "#file")
						bNFiles= true;
				}
			}
			if(	bNStr &&
				searchVec.empty() &&
				!bNFolder &&
				!bNSubs &&
				!bNFolderSubs &&
				!bNFiles			)
			{
				/*
				 * search only for an variable string
				 * no string with tabulator is found able
				 */
				return;
			}
			if(bNFolderSubs)
			{
				vector<string>::size_type nSize;

				/*
				 * search for folder with subroutine
				 * check first whether should search for folders,
				 * or when an colon be given for subroutines
				 */
				split(spl, sCurStr, is_any_of(":"));
				if(spl.size() == 2)
				{
					folder= spl[0];
					subroutine= spl[1];
					vFolderSubs= getUsableFolders(folder);
					nSize= vFolderSubs.size();
					if(nSize > 1)
					{
						/*
						 * more than one folder be found
						 * search whether one folder in queue
						 * is same than given
						 */
						for(vector<string>::iterator it= vFolderSubs.begin();
										it != vFolderSubs.end(); ++it		)
						{
							if(*it == folder)
							{
								nSize= 1;
								break;
							}
						}
					}
					if(	nSize == 0 ||
						nSize > 1		)
					{
						/*
						 * from given folder with section of subroutine,
						 * folder not found
						 * or folder is not complete
						 * this can only be wrong
						 * make no result creation
						 */
						return;
					}
				}else if(spl.size() > 1)
				{
					/*
					 * folder with section of subroutine
					 * is given with more than one colon
					 * this can only be wrong
					 * make no result creation
					 */
					return;
				}
			}
			if(	bNFolder ||
				(	bNFolderSubs &&
					folder == ""	)	)
			{
				// Usable folders
				vFolderSubs= getUsableFolders(sCurStr);
				searchVec.insert(searchVec.end(), vFolderSubs.begin(), vFolderSubs.end());
			}
			if(	bNSubs ||
				(	bNFolderSubs &&
					folder != ""	)	)
			{
				// Usable subroutines
				if(folder == "")
				{
					/*
					 * subroutine is also not set
					 */
					subroutine= sCurStr;
				}
				if(	bNSubs &&
					(	folder == "" ||
						sCurStr.find(":") == string::npos	)	)
				{
					folder= getCurrentFolder();
					if(folder == "")
					{
						writeLastPromptLine(/*lock*/true, nPos, result, /*end*/true);
						prompt("\n   no current folder for searching subroutine be defined\n"
										"   please define first with current <folder>[:subroutine]\n$> "			);
						writeLastPromptLine(/*lock*/true, nPos, result);
						return;
					}
				}
				vFolderSubs= getUsableSubroutines(folder, subroutine);
				searchVec.insert(searchVec.end(), vFolderSubs.begin(), vFolderSubs.end());
			}
			if(bNFiles)
			{
				const string extension(".dbgsession");
				const string::size_type extLen(extension.length());
				string::size_type fileLen;
				map<string, string> dir;

				dir= URL::readDirectory("./", sCurStr, extension);
				for(map<string, string>::iterator it= dir.begin();
								it != dir.end(); ++it				)
				{
					fileLen= it->second.length();
					if(	fileLen > extLen &&
						it->second.substr(fileLen - extLen) == extension	)
					{
						searchVec.push_back(it->second.substr(0, fileLen - extLen));
						if((fileLen - extLen) > nMaxLen)
							nMaxLen= fileLen - extLen;
					}
				}
			}
			if(	bNFolder ||
				bNSubs ||
				bNFolderSubs	)
			{
				for(vector<string>::iterator it= searchVec.begin();
								it != searchVec.end(); ++it			)
				{
					if(it->length() > nMaxLen)
						nMaxLen= it->length();
				}
			}
		}
		if(searchVec.size() > 1)
		{
			if(count > 0)
			{
				string::size_type columns;
				string::size_type nCount(0);
				string out;
				char *ccolumns;

				ccolumns= getenv("COLUMNS");
				if(ccolumns != NULL)
					columns = (string::size_type)atoi(ccolumns);
				else
				{
				    struct winsize w;
				    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
				    {
				    	columns= (string::size_type)w.ws_col;
				    }else
				    {
				    	std::cout << endl << strerror(errno) << endl;
				    	columns= 0;
				    }
				}
				nMaxLen= 0;
				for(vector<string>::iterator it= searchVec.begin();
								it != searchVec.end(); ++it			)
				{
					if(it->length() > nMaxLen)
						nMaxLen= it->length();
				}
				nMaxLen+= 8;
				if(columns > nMaxLen)
					columns= columns / nMaxLen;
				else
					columns= 1;
				nMaxLen-= 3;
				writeLastPromptLine(/*lock*/true, nPos, result, /*end*/true);
				for(vector<string>::iterator it= searchVec.begin();
								it != searchVec.end(); ++it			)
				{
					string nullStr;

					nullStr.append(nMaxLen - it->length(), ' ');
					out+= "   " + *it + nullStr;
					++nCount;
					if(nCount >= columns)
					{
						nCount= 0;
						out+= "\n";
					}
				}
				out+= "\n$> ";
				prompt(out);
				writeLastPromptLine(/*lock*/true, nPos, result);
				return;
			}// if(count > 0)
			if(folder != "")
			{
				/*
				 * search for folder or subroutine
				 * by splitting or search subroutine
				 * entries before
				 * set subroutine as search value
				 */
				nLen= subroutine.length();
			}
			for(string::size_type n= nLen+1; n <= nMaxLen; ++n)
			{
				bFound= true;
				sCurStr= searchVec[0].substr(0, n);
				for(vector<string>::iterator it= searchVec.begin();
								it != searchVec.end(); ++it			)
				{
					if(it->substr(0, n) != sCurStr)
					{
						bFound= false;
						nMaxLen= n - 1;
						break;
					}
				}
				if(!bFound)
				{
					sCurStr= searchVec[0].substr(0, nMaxLen);
					if(nMaxLen > nLen)
						nNewPos= nPos + (nMaxLen - nLen);
					break;
				}else
					sCurStr= searchVec[0].substr(0, n);
			}
			result= resBefore.substr(0, resBefore.length() - nLen) +
										sCurStr + resBehind;

		}else if(searchVec.size() == 1)
		{
			bool bFolderSubComplete(false);

			if(	folder != "" &&
				bNSubs == false	)
			{
				/*
				 * when folder is'nt an null string
				 * the result inside searchVec is
				 * only an subroutine
				 * when now parameter not defined
				 * for searching inside subroutines
				 * (bNSubs == false)
				 * add to folder subroutine inside searchVec
				 */
				folder+= ":" + searchVec[0];
				bFolderSubComplete= true;
			}else
				folder= searchVec[0];
			result= resBefore.substr(0, resBefore.length() - nLen);
			result+= folder;
			if(	bNFolderSubs &&
				!bFolderSubComplete	&&
				!bFoundParameter		)
			{
				/*
				 * if searching for folders
				 * with subroutines
				 * add automatically an colon
				 * to show that now will be searching
				 * for subroutines
				 * but do this only when not also
				 * single folder names allowed
				 * or founded result is an
				 * predefined parameter
				 */
				if(bNFolder)
				{
					if(count > 1)
					{
						string out;

						out= "\n\nfound correct folder\n";
						out+= "when should search for subroutines\n";
						out+= "type now an colon ':'\n";
						out+= "because also only for an folder can be searched\n";
						out+= "type space ' ' for finish\n\n";
						out+= "$> ";
						prompt(out);
					}
				}else
				{
					vector<string> res;

					result+= ":";
					res= getUsableSubroutines(folder, "");
					if(res.size() == 1)
						result+= res[0] + " ";
				}

			}else
				result+= " ";
			nNewPos= result.length();
			result+= resBehind;
		}
		if(nNewPos != string::npos)
		{
			nPos= nNewPos;
			writeLastPromptLine(/*lock*/true, nPos, result);
		}
		if(count < 10)
			++count;
		return;
	}

	void ClientTransaction::setCurrentFolder(const string& folder)
	{
		m_sCurrentFolder= folder;
	}

	string ClientTransaction::getCurrentFolder() const
	{
		return m_sCurrentFolder;
	}

	vector<string> ClientTransaction::getUsableFolders(const string& str)
	{
		bool bFound;
		string::size_type nLen;
		vector<string> vRv;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getUsableFolders(str);
		nLen= str.length();
		LOCK(m_DEBUGSESSIONCHANGES);
		for(map<string, set<string> >::iterator it= m_mFolderSubs.begin();
						it != m_mFolderSubs.end(); ++it						)
		{
			if(	it->first.length() >= nLen &&
				str == it->first.substr(0, nLen)	)
			{
				vRv.push_back(it->first);
				bFound= true;

			}else if(bFound)
				break;
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return vRv;
	}

	vector<string> ClientTransaction::getUsableSubroutines(const string& folder, const string& str)
	{
		bool bFound;
		string::size_type nLen;
		vector<string> vRv;
		map<string, set<string> >::iterator found;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getUsableSubroutines(folder, str);
		nLen= str.length();
		LOCK(m_DEBUGSESSIONCHANGES);
		found= m_mFolderSubs.find(folder);
		if(found != m_mFolderSubs.end())
		{
			for(set<string>::iterator it= found->second.begin();
							it != found->second.end(); ++it			)
			{
				if(	it->length() >= nLen &&
					str == it->substr(0, nLen)	)
				{
					vRv.push_back(*it);
					bFound= true;

				}else if(bFound)
					break;
			}
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return vRv;
	}

	string ClientTransaction::getFolderID(const string& folder)
	{
		string sRv;
		map<string, string>::iterator found;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getFolderID(folder);
		found= m_mFolderId.find(folder);
		if(found == m_mFolderId.end())
		{
			ostringstream nr;

			nr << "[" << (m_mFolderId.size() + 1) << "] ";
			sRv= nr.str();
			m_mFolderId[folder]= sRv;
		}else
			sRv= found->second;
		return sRv;
	}

	bool ClientTransaction::hearingTransfer(IFileDescriptorPattern& descriptor)
	{
		string result;
		// folder list debug session
		ppi_time time;
		ppi_value value;
		string folder, subroutine, content, id;
		// debug external port server
		bool bHeader= true;
		long nsec, nmsec, nusec;
		//timeval time;
		char buf[10];
		char stime[22];
		struct tm ttime;
		string sDo;
		device_debug_t tdebug;

		do{
			descriptor >> result;
			trim(result);
			if(result == "done")
				continue;
			if(result == "ppi-server debugsession")
			{
				bool bFirstOutput(true);

				do{
					descriptor >> result;
					trim(result);
					if(	result != "done" &&
						result != "stopclient"	)
					{
						bool bWrite(true);
						IParameterStringStream input(result);
						map<string, map<string, unsigned long> >::iterator foundFolder;
						map<string, unsigned long>::iterator foundSubroutine;

						/*
						 * data type order below
						 * is specified inside follow methods:
						 * DbInterface::fillDebugSession
						 * ServerDbTransaction::transfer by method == "fillDebugSession"
						 * ClientTransaction::hearingTransfer
						 * ClientTransaction::saveFile
						 * ClientTransaction::loadFile
						 */
						input >> folder;
						input >> subroutine;
						input >> value;
						input >> time;
						input >> content;
						LOCK(m_DEBUGSESSIONCHANGES);
						if(m_oStoreFile.is_open())
							m_oStoreFile << result << endl;
						if(	subroutine.length() > 1 &&
							subroutine.substr(0, 1) != "#"	)
						{
							m_mFolderSubs[folder].insert(subroutine);
						}
						if(!m_bHoldAll)
						{
							foundFolder= m_vsHoldFolders.find(folder);
							if(foundFolder != m_vsHoldFolders.end())
							{
								foundSubroutine= foundFolder->second.find(subroutine);
								if(	subroutine == "#start" || // <- entry is starting of folder list
									subroutine == "#inform" || // <- entry is start information
															   //    which can't shown by starting
									subroutine == "#end" || // <- entry is stopping of folder list
									foundSubroutine != foundFolder->second.end())
								{// folder is set for holding
									IDbFillerPattern::dbgSubroutineContent_t subCont;

									subCont.folder= folder;
									subCont.subroutine= subroutine;
									subCont.value= value;
									subCont.content= content;
									m_mmDebugSession[time].push_back(subCont);
									bWrite= false;
								}
							}
						}else
						{
							if(subroutine != "#setDebug")
							{
								IDbFillerPattern::dbgSubroutineContent_t subCont;

								subCont.folder= folder;
								subCont.subroutine= subroutine;
								subCont.value= value;
								subCont.content= content;
								subCont.currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time(time));
								m_mmDebugSession[time].push_back(subCont);
								bWrite= false;
							}
						}
						if(bWrite)
						{// folder isn't set for holding
							id= getFolderID(folder);
							if(bFirstOutput)
							{
								runHearingTransaction(true);
								bFirstOutput= false;
								std::cout << endl;
							}
							std::cout << glob::addPrefix(id, content);
						}
						UNLOCK(m_DEBUGSESSIONCHANGES);
					}
				}while(	result != "done" &&
						result != "stopclient"	);
				if(result == "done")
				{
					if(!bFirstOutput)
					{
						runHearingTransaction(false);
						prompt();
					}
					continue;
				}
			}
			if(result == "stopclient")
				bHeader= false;
			if(bHeader)
			{
				runHearingTransaction(true);
				if(gettimeofday(&time, NULL))
				{
					std::cout << "XX:XX:xx (cannot calculate time)" << endl;
				}else
				{
					if(localtime_r(&time.tv_sec, &ttime) != NULL)
					{
						strftime(stime, 21, "%H:%M:%S", &ttime);
						std::cout << stime << endl;
					}else
						std::cout << "XX:XX:xx (cannot calculate time)" << endl;
				}
				std::cout << "Nr.| CALL | last call        |    value     |  every   | need time | length time "
					"| act. | chip ID / pin" << endl;
				std::cout << "---------------------------------------------------------------------------------"
					"------------------------------" << endl;
				bHeader= false;
			}
			if(result != "stopclient")
			{
				if(m_bOwDebug && result.substr(0, 1) != "0")
				{
					if(result != "done\n")
					{
						string device;
						istringstream devString(result);

						//std::cout << endl << result;
						devString >> tdebug.id;
						devString >> tdebug.btime;
						devString >> tdebug.act_tm.tv_sec;
						devString >> tdebug.act_tm.tv_usec;
						devString >> tdebug.count;
						devString >> tdebug.read;
						devString >> tdebug.ok;
						devString >> tdebug.utime;
						devString >> tdebug.value;
						devString >> tdebug.cache;
						devString >> tdebug.priority;
						tdebug.device= "";
						while(	!devString.eof() &&
								!devString.fail()		)
						{
							devString >> device;
							tdebug.device+= device + " ";
						}
						trim(tdebug.device);

						if(	tdebug.btime
							&&
							m_mOwMaxTime[tdebug.id] < tdebug.utime	)
						{
							m_mOwMaxTime[tdebug.id]= tdebug.utime;
						}
						nsec= tdebug.utime / 1000000;
						nmsec= tdebug.utime / 1000 - (nsec * 1000);
						nusec= tdebug.utime - (nmsec * 1000) - (nsec * 1000000);

						std::cout.fill('0');
						std::cout.width(3);
						std::cout << dec << tdebug.id << " ";
						std::cout.fill(' ');
						std::cout.width(5);
						std::cout << tdebug.count << "   ";
						if(tdebug.btime)
						{
							long msec, usec;

							strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&tdebug.act_tm.tv_sec));
							std::cout << buf << " ";
							msec= tdebug.act_tm.tv_usec / 1000;
							usec= tdebug.act_tm.tv_usec - (msec * 1000);
							std::cout.fill('0');
							std::cout.width(3);
							std::cout << dec << msec << " ";
							std::cout.width(3);
							std::cout << dec << usec << "   ";

						}else
							std::cout << "-------- --- ---   ";
						if(tdebug.read)
							sDo= "read ";
						else
							sDo= "write";
						std::cout.setf(ios_base::fixed);
						std::cout.fill(' ');
						if(tdebug.ok)
						{
							std::cout.precision(6);
							std::cout.width(12);
							std::cout << dec << tdebug.value << "   ";
						}else
							std::cout << "cannot " << sDo << "   ";
						if(tdebug.read)
						{
							std::cout.precision(4);
							std::cout.width(8);
							std::cout << dec << tdebug.cache << "   ";
						}else
						{
							std::cout.width(4);
							std::cout << dec << tdebug.priority << "       ";
						}
						if(tdebug.btime)
						{
							std::cout.fill('0');
							std::cout.width(2);
							std::cout << dec << nsec << " ";
							std::cout.width(3);
							std::cout << dec << nmsec << " ";
							std::cout.width(3);
							std::cout << dec << nusec << "   ";

						}else
							std::cout << "-- --- ---   ";
						// measure max time
						nsec= m_mOwMaxTime[tdebug.id] / 1000000;
						nmsec= m_mOwMaxTime[tdebug.id] / 1000 - (nsec * 1000);
						nusec= m_mOwMaxTime[tdebug.id] - (nmsec * 1000) - (nsec * 1000000);
						std::cout.fill('0');
						std::cout.width(2);
						std::cout << dec << nsec << " ";
						std::cout.width(3);
						std::cout << dec << nmsec << " ";
						std::cout.width(3);
						std::cout << dec << nusec << "   ";

						std::cout << sDo << " '" << tdebug.device << "'" << endl;
					}else
					{
						std::cout << endl;
						bHeader= true;
						prompt();
					}
				}else
					std::cout << result << endl;
			}
#ifdef SERVERDEBUG
			else
				std::cout << "### server has stop hearing thread" << endl;
#endif // SERVERDEBUG
		}while(	!descriptor.eof()
				&&
				result != "stopclient"	);
		if(descriptor.eof())
			std::cout << "### by heareing on server for OWServer debug messages -> connection is broken" << endl;
		return false;
	}

	string ClientTransaction::ask(bool yesno, string promptStr)
	{
		vector<string>::size_type nHistoryPos(0);
		map<vector<string>::size_type, string> newHistory;
		string::size_type nPos(0);
		string result;
		string lastCommand;
		int charNr;
		short nTabCount(0);
		bool bSpecial(false);
		string sSpecial;

		if(yesno)
			promptStr+= " (Y/N) ";
		runUserTransaction(false);
		prompt(promptStr);
		if(!m_bCorrectTC)
		{
			unsigned short count(0);

			while(result == "")
			{
				errno= 0;
				getline(std::cin, result);
				if(count > 1000)
				{
					ostringstream errmsg;
					ErrorHandling errHandle;

					errHandle.setErrnoError("ClientTransaction", "tcgetattr", ENOTTY);/* Not a typewriter */
					errmsg << "getline() " << __FILE__ << "  line " << __LINE__ << endl;
					errmsg << errHandle.getDescription();
					if(m_bScriptState)
					{
						errmsg << endl << endl;
						errmsg << "maybe an file was shifted inside ppi-client" << endl;
						errmsg << "like 'ppi-client -w < any_file'" << endl;
						errmsg << "without any exit or quite sequence "
										"inside file at last line" << endl;
					}
					errmsg << endl;
					cerr << endl;
					cerr << glob::addPrefix("ALERT: ", errmsg.str()) << endl;
					//if(m_o2Client.get())
					//	m_o2Client->stop(/*wait*/true);
					exit(EXIT_FAILURE);
				}
				++count;
			}// while(result == "")
			runUserTransaction(true);
			return result;
		}
		result= "";
		while(	(charNr= getch()) > 0 &&
				(	(	!yesno &&
						charNr != 13 &&
						charNr != 10	) ||
					/*
					 * when yesno true
					 * get an ending of loop
					 * directly from loop
					 */
					yesno					)	)
		{
			//std::cout << "- typed (" << charNr << ") " << (char)charNr << endl;
			if(	!bSpecial &&
				charNr == 9	)
			{
				//std::cout << "pecial character TAB" << endl;
				createTabResult(result, nPos, nTabCount);
				continue;

			}else
				nTabCount= 0;
			if(charNr == 27)
			{
				/*
				 * pre-definition for
				 * all special characters
				 * only tabulator, BACK deletion
				 * will be an single character (127)
				 */
				bSpecial= true;
				sSpecial= "";

			}else if(	!bSpecial &&
						charNr == 127	)
			{
				//std::cout << "special character BACK deletion" << endl;
				if(nPos > 0)
				{
					string out;

					--nPos;
					out= result.substr(0, nPos);
					if(result.length() > nPos + 1)
						out+= result.substr(nPos + 1);
					result= out;
					bSpecial= false;
				}

			}else if(bSpecial)
			{
				sSpecial+= (char)charNr;
				if(sSpecial.length() >= 2)
				{
					if(sSpecial == "[A")
					{
						//std::cout << "special character UP" << endl;
						if(!yesno)
						{
							if(nHistoryPos == 0)
							{
								lastCommand= result;
							}else
							{
								if(result != "")
									setHistory(result, nHistoryPos);
							}
							result= getHistory(nHistoryPos, Older);
							nPos= result.length();
						}
						bSpecial= false;

					}else if(sSpecial == "[B")
					{
						//std::cout << "special character DOWN" << endl;
						if(!yesno)
						{
							if(nHistoryPos == 0)
								lastCommand= result;
							else
							{
								if(result != "")
									setHistory(result, nHistoryPos);
							}
							result= getHistory(nHistoryPos, Newer);
							if(result == "")
							{
								result= lastCommand;
								nHistoryPos= 0;
							}
							nPos= result.length();
						}
						bSpecial= false;

					}else if(sSpecial == "[C")
					{
						//std::cout << "special character RIGHT" << endl;
						if(nPos < result.length())
							++nPos;
						bSpecial= false;

					}else if(sSpecial == "[D")
					{
						//std::cout << "special character LEFT" << endl;
						if(!yesno)
						{
							if(nPos > 0)
								--nPos;
						}
						bSpecial= false;

					}else if(sSpecial == "[H")
					{
						//std::cout << "special character Pos1" << endl;
						nPos= 0;
						bSpecial= false;

					}else if(sSpecial == "[F")
					{
						//std::cout << "special character End" << endl;
						nPos= result.length();
						bSpecial= false;

					}else if(sSpecial.length() >= 3)
					{
						if(sSpecial == "[2~")
						{
							//std::cout << "special character Insert" << endl;
							bSpecial= false;

						}else if(sSpecial == "[3~")
						{
							//std::cout << "special character Del" << endl;
							if(!yesno)
							{
								if(nPos < result.length())
								{
									string out;

									out= result.substr(0, nPos);
									if(result.length() > nPos + 1)
										out+= result.substr(nPos + 1);
									result= out;
								}
							}
							bSpecial= false;

						}else if(sSpecial == "[5~")
						{
							//std::cout << "special character screen UP" << endl;
							bSpecial= false;

						}else if(sSpecial == "[6~")
						{
							//std::cout << "special character screen DOWN" << endl;
							bSpecial= false;

						}else
						{
							result+= sSpecial;
							bSpecial= false;
						}
					}// if(sSpecial.length() >= 3)
				}else if(sSpecial != "[")
				{ // sSpecial length should be always 1
					if(!yesno)
					{
						result+= (char)charNr;
						++nPos;
					}
					bSpecial= false;
				}
			}else // end of special characters
			{
				if(yesno)
				{
					result= (char)charNr;
					if(	(char)charNr == 'Y' ||
						(char)charNr == 'y' ||
						(char)charNr == 'N' ||
						(char)charNr == 'n'		)
					{
						nPos= 1;
						if(result == "n")
							result= "N";
						else if(result == "y")
							result= "Y";
						break;
					}else
					{
						writeLastPromptLine(/*lock*/true, nPos,
										result + "  (please type only Y or N)", /*end*/true);
						writeLastPromptLine(/*lock*/true, 0, "");
						nPos= 0;
						result= "";
					}
				}else
				{
					if(nPos < result.length())
						result= result.substr(0, nPos) + (char)charNr + result.substr(nPos);
					else
						result+= (char)charNr;
					++nPos;

				}
				bSpecial= false;
			}

			if(!bSpecial)
				writeLastPromptLine(/*lock*/true, nPos, result);
		}// end of getch()
		writeLastPromptLine(/*lock*/true, nPos, result, /*end*/true);
		//std::cout << "end of ask '" << result << "'" << endl;
		runUserTransaction(true);
		return result;
	}

	int ClientTransaction::getch()
	{
	    int r;
	    unsigned char c;
	    if ((r = read(STDIN_FILENO, &c, sizeof(c))) < 0)
	    {
	        return r;
	    } else {
	        return c;
	    }
	}

	void ClientTransaction::resetTc()
	{
		tcsetattr(TCSAFLUSH, &m_tTermiosBackup);
	}

	bool ClientTransaction::tcsetattr(int action, const struct termios *termiosp)
	{
		EHObj errHandle(EHObj(new ErrorHandling));

		if(!m_bCorrectTC)
			return false;
		if((::tcsetattr(STDIN_FILENO, action, termiosp)) < 0)
		{
			errHandle->setErrnoError("ClientTransaction", "tcsetattr", errno);
			cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
			m_bErrWritten= true;
			m_bCorrectTC= false;
			return false;
		}
		return true;
	}

	void ClientTransaction::readTcBackup()
	{
		EHObj errHandle(EHObj(new ErrorHandling));
		int nErrno(0);

		m_bCorrectTC= true;
		m_bScriptState= false;
		if((::tcgetattr(STDIN_FILENO, &m_tTermiosBackup)) < 0)
		{
			nErrno= errno;
			m_bCorrectTC= false;
			if(nErrno != ENOTTY)/* Not a typewriter */
			{
				errHandle->setErrnoError("ClientTransaction", "tcgetattr", nErrno);
				cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
				m_bErrWritten= true;
			}else
				m_bScriptState= true;
		}
	}

	void ClientTransaction::setHistory(const string& command, vector<string>::size_type pos/*= 0*/)
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->setHistory(command, pos);
			return;
		}
		if(	pos >= 1000 &&
			pos <= m_vHistory.size() + 1000 -1)
		{
			pos-= 1000;
			if(m_vHistory[pos] != command)
			{
				m_vHistory[pos]= command;
				m_vChangedHistory.push_back(pos + 1000);
			}
			return;
		}
		m_vHistory.push_back(command);
	}

	string ClientTransaction::getHistory(vector<string>::size_type& count, history_get_e pos)
	{
		string sfirst, scur;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getHistory(count, pos);
		if(	count == 0 ||
			count > m_vHistory.size() + 1000 -1	)
		{
			count= m_vHistory.size() + 1000;
			//std::cout << "set count to " << count << endl;

		}else if(count < 1000 )
			count= 1000;
		if(pos == Current)
		{
			if(count > m_vHistory.size() + 1000)
				return "";
			return m_vHistory[count - 1000];

		}else if(pos == Older)
		{
			count-= 1000;
			if((count + 1) <= m_vHistory.size())
				sfirst= m_vHistory[count];
			scur= sfirst;
			while(	count > 0 &&
					scur == sfirst	)
			{
				--count;
				scur= m_vHistory[count];
			}
			count+= 1000;
			return scur;
		}
		/*
		 * pos == Newer
		 */
		vector<string>::size_type nMax;

		nMax= m_vHistory.size() -1;
		count-= 1000;
		if(count >= (nMax + 1))
		{
			count+= 1000;
			return "";
		}
//		if(count <= nMax)
			sfirst= m_vHistory[count];
		scur= sfirst;
		while(	count < nMax &&
				scur == sfirst	)
		{
			++count;
			scur= m_vHistory[count];
		}
		if(	scur == sfirst)
		{
			/*
			 * count can only be
			 * the last entry
			 * (same as nMax)
			 */
			++count;
			scur= "";
		}
		count+= 1000;
		return scur;
	}

	void ClientTransaction::writeHistory()
	{
		vector<string>::size_type n(1000);

		if(m_o2Client.get())
		{
			m_o2Client->transObj()->writeHistory();
			return;
		}
		if(!m_vHistory.empty())
		{
			for(vector<string>::iterator it= m_vHistory.begin();
							it != m_vHistory.end(); ++it)
			{
				std::cout << n;
				if(find(m_vChangedHistory.begin(), m_vChangedHistory.end(), n) != m_vChangedHistory.end())
					std::cout << "*";
				else
					std::cout << " ";
				std::cout << " " << *it << endl;
				++n;
			}
		}else
			std::cout << " no history exist" << endl;
	}

	bool ClientTransaction::checkCommandCount(vector<string> commands,
					bool& writtenError, vector<string>* descriptions/*= NULL*/)
	{
		bool bRv(true);
		unsigned short n;
		ostringstream out;
		vector<string>::size_type nMaxParams(0), nMinParams(0);
		vector<string>::size_type nCommandCount(commands.size());

		if(descriptions != NULL)
		{
			nMaxParams= descriptions->size();
			for(vector<string>::iterator it= descriptions->begin();
							it != descriptions->end(); ++it	)
			{
				if(	it->length() > 1 &&
					it->substr(0,1) == "[" &&
					it->substr(it->length() - 1, 1) == "]"	)
				{
					++nMinParams;// optional
				}
				nMinParams= nMaxParams - nMinParams;
			}
		}

		if(	(nCommandCount -1) < nMinParams ||
			(nCommandCount -1) > nMaxParams		)
		{
			bRv= false;
			if(!writtenError)
			{
				out << "   ";
				if(	nMaxParams == 0 ||
					(nCommandCount -1) < nMaxParams	)
				{
					out << "after ";
				}
				out << "command '" << commands[0] << "' ";
				if(nMaxParams == 0)
				{
					out << "cannot be any parameter" << endl;

				}else
				{
					if((nCommandCount -1) > nMaxParams)
						out << "can only has ";
					else
						out << "has to exist ";
					if(nMaxParams == 1)
						out << "one ";
					else if(nMaxParams == 2)
						out << "two ";
					else if(nMaxParams == 3)
						out << "three ";
					else
						out << nMaxParams << " ";
					out << "parameter";
					if(nMaxParams > 1)
					{
						out << "s" << endl;
						n= 1;
						for(vector<string>::iterator it= descriptions->begin();
										it != descriptions->end(); ++it)
						{
							out << "     " << n << ". as '" << *it << "'";
							if(	it->length() > 2 &&
								it->substr(0,1) == "[" &&
								it->substr(it->length() - 1, 1) == "]"	)
							{
								out << "  optional";
							}
							out << endl;
							++n;
						}
					}else
					{
						if((nCommandCount -1) > nMaxParams)
							out << " can only be an ";
						else
							out << " has to be an ";
						if(	(*descriptions)[1].length() > 2 &&
							(*descriptions)[1].substr(0,1) == "[" &&
							(*descriptions)[1].substr((*descriptions)[1].length() - 1, 1) == "]"	)
						{
							out << "optional ";
						}
						out << "parameter as '" << (*descriptions)[1] << "'" << endl;
					}
				}
				writtenError= true;
			}
		}
		cout(out.str());
		if(descriptions != NULL)
			descriptions->clear();
		return bRv;
	}

	bool ClientTransaction::checkWaitCommandCount(vector<string> commands,
					bool& errorWritten, vector<string>* descriptions/*= NULL*/)
	{
		if(!m_bWait)
		{
			if(!errorWritten)
			{
				cout("   ppi-client not defined to read more than one commands\n"
						"   command only usable by starting client with option --wait\n");
				errorWritten= true;
			}
			return false;
		}else
			return checkCommandCount(commands, errorWritten, descriptions);
	}

	bool ClientTransaction::checkHearCommandCount(vector<string> commands,
					bool& errorWritten, vector<string>* descriptions/*= NULL*/)
	{
		if(m_o2Client.get() == NULL)
		{
			if(!errorWritten)
			{
				cout("   ppi-client not defined with second connection to server\n"
						"   command only usable by starting client with option --hear\n");
				errorWritten= true;
			}
			return false;
		}else
			return checkCommandCount(commands, errorWritten, descriptions);
	}

	bool ClientTransaction::userTransfer(IFileDescriptorPattern& descriptor)
	{
		typedef vector<pair<string, pair<ppi_time, vector<string> > > > layerVecDef;
		typedef pair<string, pair<ppi_time, vector<string> > > layerDef;
		typedef pair<ppi_time, vector<string> > layerContentDef;

		bool bErrorWritten;
		bool bWaitEnd= false;
		bool bSendCommand(true);
		bool bReadDbgSessionContent(false);
		bool bDebSessionContentLoaded(false);
		string logMsg, result, org_command, defcommand;
		vector<string> command;
		vector<string> paramDefs;
		string sSendbuf;
		string folder, subroutine;
		layerVecDef vLayers;
		layerVecDef::size_type nCurLayer(0);
		ppi_time currentTime;
		struct termios termios_flag;

		readTcBackup();
		if(!m_bScriptState)
		{
			termios_flag= m_tTermiosBackup;
			termios_flag.c_lflag &= ~ICANON;
			termios_flag.c_lflag &= ~ECHO;
			//termios_flag.c_cc[VMIN] = 1;
			//termios_flag.c_cc[VTIME] = 0;
			tcsetattr(TCSANOW, &termios_flag);
		}
		if(	m_bWait &&
			!m_bScriptState	)
		{
			std::cout << endl;
			std::cout << "stop connection to server by typing 'exit' or 'quit'" << endl;
			std::cout << "for help type '?'";
			if(m_o2Client.get())
				std::cout << ", ";
			else
				std::cout << " or ";
			std::cout << "'?value'";
			if(m_o2Client.get())
				std::cout << " or '?debug'";
			std::cout << endl;
			std::cout << "--------------------------------------------------------" << endl;
		}
		org_command= m_sCommand;
		do{
			istringstream iresult;

			if(org_command == "")
			{
				org_command= ask(/*YesNo*/false, "$> ");
				trim(org_command);
				if(org_command == "")
					continue;
			}
			bErrorWritten= false;
			bWaitEnd= false;
			bSendCommand= true;
			command.clear();
			iresult.str(org_command);
			while(!iresult.eof())
			{
				string word;
				iresult >> word;
				command.push_back(word);
			}
			defcommand= org_command;
			if(command[0].substr(0, 1) == "!")
			{
				vector<string>::size_type nr;
				istringstream res(command[0].substr(1));

				res >> nr;
				if(	res.fail() ||
					nr < 1001 ||
					(org_command= getHistory(nr, Current)) == ""	)
				{
					cout(" no correct number given for history command\n");
					org_command= "";
					continue;
				}
				if(checkWaitCommandCount(command, bErrorWritten))
					continue;
				command.clear();
				iresult.str(org_command);
				while(!iresult.eof())
				{
					string word;
					iresult >> word;
					command.push_back(word);
				}
				defcommand= org_command;
			}
			if(	command[0] == "quit" ||
				command[0] == "exit"	)
			{
				if(checkWaitCommandCount(command, bErrorWritten))
				{
					resetTc();
					org_command= "ending";
					descriptor << org_command;
					descriptor.endl();
					descriptor.flush();
					if(m_o2Client.get())
						m_o2Client->stop(/*wait*/true);
					return false;
				}

			}else if(command[0].substr(0, 1) == "?")
			{
				bSendCommand= false;
				if(checkWaitCommandCount(command, bErrorWritten))
					writeHelpUsage(command[0]);

			}else if(command[0] == "CHANGE")
			{
				paramDefs.push_back("[user]");
				if(checkWaitCommandCount(command, bErrorWritten, &paramDefs))
				{
					struct termios term;
					string user, pwd;

					bSendCommand= false;
					term= m_tTermiosBackup;
					resetTc();
					if(command.size() == 2)
						user= command[1];
					compareUserPassword(descriptor, user, pwd);
					if(!m_bScriptState)
					{
						termios_flag= m_tTermiosBackup;
						termios_flag.c_lflag &= ~ICANON;
						termios_flag.c_lflag &= ~ECHO;
						//termios_flag.c_cc[VMIN] = 1;
						//termios_flag.c_cc[VTIME] = 0;
						tcsetattr(TCSANOW, &termios_flag);
					}
				}
			}else if(command[0] == "run")
			{
				map<string, unsigned long> folderCount;

				bSendCommand= false;
				if(checkHearCommandCount(command, bErrorWritten))
				{
					folderCount= getRunningFolderList(/*locked*/false);
					if(!folderCount.empty())
					{
						for(map<string, unsigned long>::iterator it= folderCount.begin();
										it != folderCount.end(); ++it			)
						{
							std::cout << "   " << it->second << "  " << it->first << endl;
						}
					}else
						std::cout << " no folders running since starting or last 'clear'" << endl;
				}//checkHearCommandCount

			}else if(	command[0] == "DIR" ||
						command[0] == "status"	)
			{
				if(checkCommandCount(command, bErrorWritten))
					bWaitEnd= true;
			}

			if(command[0] == "GETERRORSTRING ")
			{
				string com;
				int number;
				istringstream icommand;
				ostringstream ocommand;

				paramDefs.push_back("string of 'ERROR' or 'WARNING'");
				paramDefs.push_back("number of error/warning");
				if(checkCommandCount(command, bErrorWritten, &paramDefs))
				{
					icommand >> number;
					if(number > 0)
						number-= m_nOutsideErr;
					else
						number+= m_nOutsideWarn;
					ocommand << com << " " << number;
					defcommand= ocommand.str();
				}
			}else if(command[0] == "hold")
			{
				if(command.size() > 1)
				{
					paramDefs.push_back("[inform option ( -i ) ]");
					paramDefs.push_back("<folder>:<subroutine>");
				}
				if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
				{
					string sdo("Y");

					if(bDebSessionContentLoaded)
					{
						sdo= ask(/*YesNo*/true, "   some debug session content is loaded from hard disk\n"
									"   do you want remove?");
						if(sdo == "Y")
						{
							bDebSessionContentLoaded= false;
							clearDebugSessionContent();
							clearHoldingFolder("", "");
						}
					}
					if(sdo == "Y")
					{
						bReadDbgSessionContent= true;
						if(command.size() == 1)
							defcommand= "DEBUG";// -i";
						else
							defcommand= "DEBUG" + defcommand.substr(4);
					}else
						bSendCommand= false;
				}

			}else if(command[0] == "add")
			{
				paramDefs.push_back("<subroutine>");
				if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
				{
					vector<string>::iterator found;

					if(	vLayers.empty() ||
						vLayers[nCurLayer].first == ""	)
					{
						cout("   no folder:subroutine defined\n"
								"   define first with $> current <folder>[:subroutine]\n");
						bErrorWritten= true;

					}else if(!existOnServer(descriptor, vLayers[nCurLayer].first, command[1]))
					{
						cout(" subroutine '" + vLayers[nCurLayer].first + ":" + command[1]
						       + "' do not exist inside ppi-server working list\n");
						bErrorWritten= true;

					}else
					{
						found= find(vLayers[nCurLayer].second.second.begin(),
										vLayers[nCurLayer].second.second.end(), command[1]);
						if(found == vLayers[nCurLayer].second.second.end())
						{
							vLayers[nCurLayer].second.second.push_back(command[1]);
							command.clear();
							command.push_back("current");
							defcommand= "current";

						}else
						{
							cout(" subroutine '" + command[1] + "' was added before\n");
							bErrorWritten= true;
						}
					}
					bSendCommand= false;
				}

			}else if(	command[0] == "rm" ||
						command[0] == "remove"	)
			{
				vector<string>::iterator it;
				ostringstream out;

				paramDefs.push_back("<subroutine>");
				if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
				{
					bool bRemove(true);

					it= find(vLayers[nCurLayer].second.second.begin(),
									vLayers[nCurLayer].second.second.end(), command[1]);
					if(it == vLayers[nCurLayer].second.second.end())
					{
						out << " current subroutines in folder ";
						out << vLayers[nCurLayer].first << " are:" << endl;
						for(it= vLayers[nCurLayer].second.second.begin();
										it != vLayers[nCurLayer].second.second.end(); ++it)
						{
							out << "                                ";
							out << *it << endl;
						}
						out << " cannot find given subroutine '" << command[1];
						out << "' inside queue" << endl;
						cout(out.str());
						bErrorWritten= true;
						bRemove= false;

					}else if(vLayers[nCurLayer].second.second.size() == 1)
					{
						string msg;

						msg=  " only this one subroutine is inside queue\n";
						msg+= " do you want remove ";
						if(nCurLayer == 0)
							msg+= "all current debugging?\n";
						else
							msg+= "the current debugging layer?\n";
						msg= ask(/*YesNo*/true, msg);
						if(msg == "N")
							bRemove= false;
					}
					if(bRemove)
					{
						if(vLayers[nCurLayer].second.second.size() > 1)
						{
							vLayers[nCurLayer].second.second.erase(it);
							command.clear();
							command.push_back("current");
							defcommand= "current";
						}else
						{
							layerVecDef::size_type nCount(0);

							for(layerVecDef::iterator it= vLayers.begin();
											it != vLayers.end(); ++it		)
							{
								if(nCurLayer == nCount)
								{
									vLayers.erase(it);
									break;
								}
							}
							if(nCurLayer > 0)
								--nCurLayer;
							if(!vLayers.empty())
							{
								setCurrentFolder(vLayers[nCurLayer].first);
								command.clear();
								command.push_back("current");
								defcommand= "current";
							}else
							{
								setCurrentFolder("");
								bSendCommand= false;
							}
						}
					}else
						bSendCommand= false;
				}// checkHearCommandCount()
			}
			if(	bErrorWritten == false &&
				(	command[0] == "show" ||
					command[0] == "current" ||
					command[0] == "next" ||
					command[0] == "previous" ||
					command[0] == "first" ||
					command[0] == "last" ||
					command[0] == "up" ||
					command[0] == "back"		)	)
			{
				short nExist(-2);
				direction_e direction;
				short folderNr(0);

				bSendCommand= false;
				if(command[0] == "show")
				{
					direction= all;

				}else if(command[0] == "first")
				{
					direction= first;

				}else if(	command[0] == "current" ||
							command[0] == "up" ||
							command[0] == "back"	 	)
				{
					if(command[0] == "back")
					{
						if(nCurLayer > 0)
						{
							--nCurLayer;
							setCurrentFolder(vLayers[nCurLayer].first);
						}else
						{
							cout(" no lower layer from command 'current' before exist\n");
							bErrorWritten= true;

						}
					}
					if(command[0] == "up")
					{
						if((nCurLayer + 1) < vLayers.size())
						{
							++nCurLayer;
							setCurrentFolder(vLayers[nCurLayer].first);
						}else
						{
							cout(" no higher layer from command 'current' before exist\n");
							bErrorWritten= true;
						}

					}
					direction= current;

				}else if(command[0] == "next")
				{
					direction= next;

				}else if(	command[0] == "previous")
				{
					direction= previous;

				}else
					direction= last;
				if(!bErrorWritten)
				{
					if(	direction == all ||
						direction == current	)
					{
						paramDefs.push_back("[ <folder>:<subroutine> ]");

					}else if(	direction == next ||
								direction == previous	)
					{
						paramDefs.push_back("[ folder number or 'changed/unchanged' ]");
						paramDefs.push_back("[ <subroutine> when not "
										"only one defined to show by changed/unchanged ]");
					}
					if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
					{
						folder= "";
						subroutine= "";
						if(command.size() > 1)
						{
							if(	direction == next ||
								direction == previous	)
							{
								bool bChangedUnchanged(false);

								if(	command[1] == "ch" ||
									command[1] == "changed"	)
								{
									bChangedUnchanged= true;
									if(direction == next)
										direction= next_changed;
									else // direction should be previous
										direction= previous_changed;

								}else if(	command[1] == "uch" ||
											command[1] == "unchanged"	)
								{
									bChangedUnchanged= true;
									if(direction == next)
										direction= next_unchanged;
									else // direction should be previous
										direction= previous_unchanged;

								}
								if(bChangedUnchanged)
								{
									bool bFoundSubroutine(false);
									vector<string>::iterator subIt;
									/*
									 * command is defined for direction
									 * with changed or unchanged
									 * search now for subroutine
									 * which should be changed/unchanged
									 */
									if(vLayers[nCurLayer].second.second.empty())
									{
										if(command.size() == 3)
										{
											/*
											 * hole folder should be shown,
											 * define folder and subroutine
											 * for later existing check
											 * inside 1. parameter (command[1])
											 * and in subroutines queue define
											 * an '#' before
											 * to know later inside method writeDebugSession()
											 * that all subroutines should be shown
											 * but check for changed/unchanged
											 * this subroutine with '#'
											 */
											command[1]= vLayers[nCurLayer].first + ":";
											command[1]+= command[2];
											vLayers[nCurLayer].second.second.push_back("#" + command[2]);
											bFoundSubroutine= true;
										}
									}else if(vLayers[nCurLayer].second.second.size() == 1)
									{
										if(	command.size() < 3 ||
											command[2] == vLayers[nCurLayer].second.second[0]	)
										{
											bFoundSubroutine= true;
											command[1]= vLayers[nCurLayer].first + ":";
											command[1]+= vLayers[nCurLayer].second.second[0];
										}
									}else
									{
										if(command.size() == 3)
										{
											subIt= find(vLayers[nCurLayer].second.second.begin(),
															vLayers[nCurLayer].second.second.end(), command[2]);
											if(subIt != vLayers[nCurLayer].second.second.end())
											{
												bFoundSubroutine= true;
												if(command[2] != vLayers[nCurLayer].second.second[0])
												{
													vector<string> newVec;
													/*
													 * when defined subroutine
													 * is not as first,
													 * do this now
													 * because method writeDebugSession()
													 * know in this case
													 * that this first subroutine
													 * has to check for changed or unchanged
													 */
													vLayers[nCurLayer].second.second.erase(subIt);
													newVec.push_back(command[2]);
													newVec.insert(newVec.end(),
																	vLayers[nCurLayer].second.second.begin(),
																	vLayers[nCurLayer].second.second.end()	);
													vLayers[nCurLayer].second.second= newVec;
												}
												command[1]= vLayers[nCurLayer].first + ":";
												command[1]+= command[2];
											}
										}// if(command.size() == 3)
									}
									if(!bFoundSubroutine)
									{
										ostringstream out;

										if(command.size() < 3)
										{
											out << " need after command ";
											if(	direction == next_changed ||
												direction == previous_changed	)
											{
												out << "'changed' ";
											}else
												out << "'unchanged' ";
											out << "an subroutine for check" << endl;
										}else
										{
											out << " predefined subroutines:" << endl;
											for(subIt= vLayers[nCurLayer].second.second.begin();
															subIt != vLayers[nCurLayer].second.second.end(); ++subIt	)
											{
												out << "             " << *subIt << endl;
											}
											out << " subroutine '" << command[2] << "' do not exist " << endl;
										}
										cout(out.str());
										bErrorWritten= true;
									}
								}else// if(bChangedUnchanged
								{
									iresult.str(command[1]);
									iresult >> folderNr;
									if(iresult.fail())
									{
										cout(" no correct command or folder number "
												" after '" + command[0] + "' defined\n");
										bErrorWritten= true;

									}else if(folderNr < 1)
									{
										cout(" folder number after '" + command[0] +
												" has to be greater than 0\n");
										bErrorWritten= true;
									}
								}
							}// if(direction == next || direction == previous)
						}
						if(	!bErrorWritten &&
							command.size() > 1	)
						{
							vector<string> spl;

							if(	direction != all &&
								direction != current &&
								direction != next_changed &&
								direction != next_unchanged &&
								direction != previous_changed &&
								direction != previous_unchanged	)
							{
								cout(" for command '" + command[0] + "'"
										" is no definition of folder[:subroutine] allowed\n");
								bErrorWritten= true;

							}else
							{
								split(spl, command[1], is_any_of(":"));
								folder= spl[0];
								if(spl.size() > 1)
									subroutine= spl[1];
								nExist= exist(folder, subroutine);
								if(	nExist == -1 ||
									(	nExist == 0 &&
										m_bHoldAll		)	)
								{
									/*
									 * check now whether
									 * folder:subroutine
									 * exist inside
									 * folder working list
									 * of ppi-server
									 */
									if(!existOnServer(descriptor, folder, subroutine))
									{
										cout(" '" + folder + ":" + subroutine + "'"
												" do not exist in folder working list of ppi-server\n");
										if(	!vLayers.empty() &&
											direction != next_changed &&
											direction != next_unchanged &&
											direction != previous_changed &&
											direction != previous_unchanged	)
										{
											/*
											 * this point reach only when
											 * should searching for changed or unchanged
											 * and no subroutines be defined for folder
											 * inside subroutine should be one subroutine
											 * with '#' before
											 * this will be not need by error
											 */
											vLayers[nCurLayer].second.second.clear();
										}
										bErrorWritten= true;
									}
								}
								if(	!bErrorWritten &&
									(	direction == current ||
										vLayers.empty() || // direction is all
										vLayers[nCurLayer].first == ""	)	)
								{
									/*
									 * implement new folder[:subroutine]
									 * into current output queue when folder was same
									 * otherwise make an new layer
									 */
									ppi_time startTime;
									vector<string> sub;
									pair<ppi_time, vector<string> > newSub;

									setCurrentFolder(folder);
									if(subroutine != "")
										sub.push_back(subroutine);
									if(vLayers.size() > (nCurLayer + 1))
									{
										/*
										 * write new folder:subroutine
										 * into next higher layer
										 */
										startTime= vLayers[nCurLayer].second.first;
										++nCurLayer;
										vLayers[nCurLayer].first= folder;
										vLayers[nCurLayer].second.first= startTime;
										vLayers[nCurLayer].second.second= sub;
									}else
									{
										/*
										 * define new layer
										 */
										if(!vLayers.empty())
										{
											startTime= vLayers[nCurLayer].second.first;
											++nCurLayer;
											/*
											 * otherwise starting time stay by null
											 * to show the first folder entry
											 */
										}
										newSub= pair<ppi_time, vector<string> >(startTime, sub);
										vLayers.push_back(pair<string, pair<ppi_time, vector<string> > >(folder, newSub));
									}
								} // i(direction == current && folder == "")
							}

						}else if(!bErrorWritten) // end of if(command.size() > 1)
						{
							if(	direction != all &&
								(	vLayers.empty() ||
									vLayers[nCurLayer].first == ""	)	)
							{
								cout(" no folder:subroutine defined,\n"
										" define first with $> current <folder>[:subroutine]\n");
								bErrorWritten= true;
							}
						} // end of else from secWord != ""
						if(	!bErrorWritten &&
							nExist == -1		)
						{
							cout(string(" '") + folder + ":" + subroutine + "'"
									" exist inside ppi-server, but is not defined for holding\n"
									" type first $> hold " + folder + ":" + subroutine + "\n"	);
							bErrorWritten= true;
						}
						if(!bErrorWritten)
						{
							ppi_time newTime;
							vector<string> vShowSubroutines;

							if(direction != all)
							{
								folder= vLayers[nCurLayer].first;
								if(folderNr == 0)
									newTime= vLayers[nCurLayer].second.first;
								vShowSubroutines= vLayers[nCurLayer].second.second;

							}else
								vShowSubroutines.push_back(subroutine);
							newTime= *writeDebugSession(
											folder, vShowSubroutines, direction, &newTime, folderNr	);
							if(	direction == all ||
								newTime.isSet()		)
							{
								if(direction != all)
									vLayers[nCurLayer].second.first= newTime;
							}
						}// if(!bErrorWritten)
					}// checkHearCommandCount()
				}// if(!bErrorWritten)

			}else if(command[0] == "clear")
			{
				bSendCommand= false;
				if(checkHearCommandCount(command, bErrorWritten))
				{
					nCurLayer= 0;
					clearDebugSessionContent();
					vLayers.clear();
				}
			}else if(command[0] == "history")
			{
				bSendCommand= false;
				if(checkWaitCommandCount(command, bErrorWritten))
				{
					setHistory(command[0]);
					writeHistory();
					/*
					 * history is the only
					 * one command
					 * which should'nt
					 * go to end of loop
					 * because it set before
					 * self an new history entry
					 */
					org_command= "";
					continue;
				}
			}else if(command[0] == "save")
			{
				bSendCommand= false;
				paramDefs.push_back("file name");
				if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
				{
					string answer("Y");
					string file(command[1] + ".dbgsession");
					map<string, string> dir;

					dir= URL::readDirectory(/*path*/"./", /*beginfilter*/"", ".dbgsession");
					for(map<string, string>::iterator it= dir.begin();
									it != dir.end(); ++it				)
					{
						if(it->second == file)
						{
							answer= ask(/*YesNo*/true, "   file exist on harddisk\n"
											"   do you want replace ");
							break;
						}
					}
					if(answer == "Y")
					{
						bErrorWritten= !saveFile(command[1]);
						if(	!bErrorWritten &&
							!bReadDbgSessionContent	)
						{
							cout("   WARNING: no debug session content is define to get from server\n"
									"            please define with hold or DEBUG\n");
						}
					}
				}
			}else if(command[0] == "load")
			{
				bSendCommand= false;
				paramDefs.push_back("file name");
				if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
				{
					string answer("Y");

					if(	bReadDbgSessionContent ||
						bDebSessionContentLoaded	)
					{
						string msg("do you want to stop all debug session content reading from ");

						if(bDebSessionContentLoaded)
							msg+= "hard disk before?\n";
						else
							msg+= "server before?\n";
						answer= ask(/*YesNo*/true, msg);
						if(answer == "Y")
						{
							bErrorWritten= send(descriptor, "STOPDEBUG", /*bWaitEnd*/false);
							if(!bErrorWritten)
							{
								clearDebugSessionContent();
								clearHoldingFolder("", "");
								vLayers.clear();
							}else
								cout("   cannot stop debug session on server\n");
						}
					}
					if(	answer == "Y" &&
						!bErrorWritten	)
					{
						bErrorWritten= !loadFile(command[1]);
						if(!bErrorWritten)
							bDebSessionContentLoaded= true;
					}
				}
			}
			if(	command[0] == "DEBUG" &&
				bDebSessionContentLoaded	)
			{
				string sdo;

				sdo= ask(/*YesNo*/true, "   some debug session content is loaded from hard disk\n"
								"   do you want remove?");
				if(sdo == "Y")
				{
					bDebSessionContentLoaded= false;
					clearDebugSessionContent();
					clearHoldingFolder("", "");
				}
			}
			if(	!bErrorWritten &&
				bSendCommand		)
			{
				/// send command to ppi-server
				bErrorWritten= send(descriptor, defcommand, bWaitEnd);

				if(	m_o2Client.get() &&
					!bErrorWritten		)
				{
					if(command[0] == "DEBUG")
					{
						bReadDbgSessionContent= true;
					}else if(command[0] == "hold")
					{
						vector<string> spl;

						bReadDbgSessionContent= true;
						if(command.size() > 1)
						{
							/*
							 * second word after hold
							 * should be always correct <folder>:<subroutine>
							 * because server do not return error
							 * when write DEBUG command was correct
							 */
							split(spl, command[1], is_any_of(":"));
							setHoldingFolder(spl[0], spl[1]);

						}else
						{
							m_bHoldAll= true;
							allFolderHolding();
						}
					}else// if(command[0] == "hold")
					if(command[0] == "STOPDEBUG")
					{
						if(command.size() > 1)
						{
							vector<string> spl;

							/*
							 * second word after STOPDEBUG
							 * should be always correct <folder>:<subroutine>
							 * because server do not return error
							 * when was done
							 */
							split(spl, command[1], is_any_of(":"));
							if(clearHoldingFolder(spl[0], spl[1]))
							{
								bReadDbgSessionContent= false;
								closeFile();
							}

						}else
						{
							bReadDbgSessionContent= false;
							clearHoldingFolder("", "");
							closeFile();
						}
					}// if(command[0] == "STOPDEBUG")
				}// if(!bErrorWritten)
			}// if(bSendCommand)
			setHistory(org_command);
			org_command= "";

		}while(m_bWait);

		descriptor << "ending\n";
		descriptor.flush();
		return true;
	}

	bool ClientTransaction::send(IFileDescriptorPattern& descriptor, const string& command, bool bWaitEnd)
	{
		auto_ptr<XMLStartEndTagReader> xmlReader;
		bool bError(false);
		string result;

		if(command.substr(0, 8) == "CONTENT ")
			xmlReader= auto_ptr<XMLStartEndTagReader>(new XMLStartEndTagReader());

#ifdef SERVERDEBUG
		std::cout << "send: '" << command << "'" << endl;
#endif // SERVERDEBUG

		descriptor << command;
		descriptor.endl();
		descriptor.flush();

		do{
			descriptor >> result;
			trim(result);
#ifdef SERVERDEBUG
			std::cout << "get: " << result << endl;
#endif // SERVERDEBUG

			if(descriptor.eof())
			{
				cerr << endl;
				cerr << "ERROR: lost connection to server" << endl;
				if(m_o2Client.get())
					m_o2Client->stop(/*wait*/true);
				return false;
			}
			if(	result.substr(0, 6) == "ERROR " ||
				result.substr(0, 8) == "WARNING "	)
			{
				printError(descriptor, result);
				if(result.substr(0, 6) == "ERROR ")
					bError= true;

			}else if(xmlReader.get())
			{
				result= xmlReader->readLine(result);
				if(xmlReader->end())
				{
					string error;

					error= xmlReader->error();
					if(error != "")
						printError(descriptor, error);
					xmlReader= auto_ptr<XMLStartEndTagReader>();
					break;
				}
			}else
			{
				bWaitEnd= false;
				if(result != "done")
					cout(string("   ") + result + "\n");
			}
		}while(	bWaitEnd ||
				xmlReader.get()	);
		return bError;
	}

	void ClientTransaction::closeFile()
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->closeFile();
			return;
		}

		LOCK(m_DEBUGSESSIONCHANGES);
		m_oStoreFile.close();
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	bool ClientTransaction::loadFile(const string& file)
	{
		bool bOk;
		ifstream handle;
		int major_release, minor_release, subversion;
		int patch_level, revision;
		float protocol;
		string read;
		vector<pair<string, string> > vProtocol;

		if(m_o2Client.get())
			return m_o2Client->transObj()->loadFile(file);

		LOCK(m_DEBUGSESSIONCHANGES);
		handle.open(string(file + ".dbgsession").c_str());
		if(!handle.is_open())
		{
			cerr << "   cannot open file" << endl;
			cerr << "   " << BaseErrorHandling::getErrnoString(errno) << endl;
			UNLOCK(m_DEBUGSESSIONCHANGES);
			return false;
		}
		bOk= false;
		handle >> read;
		if(read == "ppi-server")
		{
			istringstream iread;

			handle >> read;
			if(read.substr(0, 1) == "v")
			{
				iread.str(read.substr(1));
				iread >> major_release;
				if(!iread.eof())
				{
					iread >> read;
					if(read.substr(0, 1) == ".")
					{
						iread.clear();
						iread.str(read.substr(1));
						iread >> minor_release;
						if(!iread.eof())
						{
							iread >> read;
							if(read.substr(0, 1) == ".")
							{
								iread.clear();
								iread.str(read.substr(1));
								iread >> subversion;
								if(!iread.eof())
								{
									iread >> read;
									if(read.substr(0, 1) == ".")
									{
										iread.clear();
										iread.str(read.substr(1));
										iread >> patch_level;
										if(!iread.eof())
										{
											iread >> read;
											if(read.substr(0, 1) == ":")
											{
												iread.clear();
												iread.str(read.substr(1));
												iread >> revision;
												if(!handle.eof())
												{
													handle >> read;
													if(read.substr(0, 1) == "p")
													{
														iread.clear();
														iread.str(read.substr(1));
														iread >> protocol;
														if(!iread.fail())
															bOk= true;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if(bOk)
		{
			vector<string> spl;

			bOk= false;
			getline(handle, read);// <- first should be ending of line
			getline(handle, read);
			trim(read);
			split(spl, read, is_any_of(" ,()"), boost::token_compress_on);
			spl.pop_back();// <- last string should be an null string ( because after ')' )
			if((spl.size() % 2) == 1)
			{
				vector<string>::iterator it;

				it= spl.begin();
				while(it != spl.end())
				{
					if(it == spl.begin())
					{
						if(*it != "store:")
						{
							/*
							 * first splitting string
							 * has to be 'store:'
							 * otherwise an fault occurred
							 */
							break;
						}
						++it;
					}else
					{
						string field;

						bOk= true;
						field= *it;
						++it;
						if(it != spl.end())
							vProtocol.push_back(pair<string, string>(field, *it));
						else
						{
							bOk= false;
							break;
						}
						++it;
					}

				}
			}
		}
		if(bOk)
		{
			vector<string>::size_type fields(0);

			bOk= false;
			/*
			 * check now whether
			 * fields from protocol
			 * are OK
			 */
			if(protocol == 1.0)
			{
				fields= 5;
				if(vProtocol.size() == fields)
				{
					if(	vProtocol[0].first == "folder" &&
						vProtocol[0].second == "string" &&
						vProtocol[1].first == "subroutine" &&
						vProtocol[1].second == "string" &&
						vProtocol[2].first == "value" &&
						vProtocol[2].second == "double" &&
						vProtocol[3].first == "time" &&
						vProtocol[3].second == "timeval" &&
						vProtocol[4].first == "content" &&
						vProtocol[4].second == "string"		)
					{
						bOk= true;
					}
				}
			}
		}
		if(bOk == false)
		{
			cerr << "   file is not from ppi-server" << endl;
			cerr << "   or is corrupt" << endl;
			handle.close();
			UNLOCK(m_DEBUGSESSIONCHANGES);
			return false;
		}
		/*
		 * read now hole content
		 * from file
		 */
		if(bOk)
		{
			unsigned short nReadLine;

			bOk= true;
			nReadLine= 1;
			std::cout << "." << flush;
			while(!handle.eof())
			{
				getline(handle, read);
				trim(read);
				if(	read != "" &&
					read.substr(0, 1) != "#"	)
				{
					ppi_time currentTime;
					IParameterStringStream result(read);
					IDbFillerPattern::dbgSubroutineContent_t sub;

					result >> sub.folder;
					if(!result.fail())
						result >> sub.subroutine;
					if(!result.fail())
						result >> sub.value;
					if(!result.fail())
						result >> currentTime;
					if(!result.fail())
					{
						sub.currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time(currentTime));
						result >> sub.content;
					}
					if(!result.fail())
					{
						m_mmDebugSession[currentTime].push_back(sub);
						m_mFolderSubs[sub.folder].insert(sub.subroutine);
					}else
						bOk= false;
					if(nReadLine > 100)
					{
						std::cout << "." << flush;
						nReadLine= 0;
					}
					++nReadLine;
				}// if(	read != "" && read.substr(0, 1) != "#" )
			}// while(!handle.eof())
			if(bOk)
				std::cout << " OK";
			std::cout << endl;
		}// if(bOk)
		if(bOk == false)
		{
			if(!m_mmDebugSession.empty())
			{
				cerr << "   some lines of file are corrupt" << endl;
				cerr << "   maybe no correct result can be shown" << endl;
				bOk= true;
			}else
				cerr << "   file is corrupt" << endl;
		}
		handle.close();
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return bOk;
	}

	bool ClientTransaction::saveFile(const string& file)
	{
		unsigned short nWriteLines(0);

		if(m_o2Client.get())
			return m_o2Client->transObj()->saveFile(file);

		LOCK(m_DEBUGSESSIONCHANGES);
		m_oStoreFile.open(string(file + ".dbgsession").c_str(), ios::trunc);
		if(m_oStoreFile.fail())
		{
			cerr << "   cannot store file on hard disk" << endl;
			cerr << "   " << BaseErrorHandling::getErrnoString(errno) << endl;
			UNLOCK(m_DEBUGSESSIONCHANGES);
			return false;
		}
		m_oStoreFile << "ppi-server  ";
		m_oStoreFile << "v" << PPI_MAJOR_RELEASE << "." << PPI_MINOR_RELEASE;
		m_oStoreFile << "." << PPI_SUBVERSION << "." << PPI_PATCH_LEVEL;
		m_oStoreFile << ":" << PPI_REVISION_NUMBER;
		m_oStoreFile << "  p" << PPI_SERVER_PROTOCOL << endl;
		m_oStoreFile << "store: folder(string), subroutine(string),";
		m_oStoreFile << " value(double), time(timeval), content(string)" << endl;
		if(!m_mmDebugSession.empty())
		{
			string answer;

			answer= ask(/*YesNo*/true, "   should be stored also current debug session content\n"
							"   getting from Server? ");
			if(answer == "Y")
			{
				IDbFillerPattern::dbgSubroutineContent_t subCont;

				for(debugSessionTimeMap::iterator tit= m_mmDebugSession.begin();
								tit != m_mmDebugSession.end(); ++tit									)
				{
					for(vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator it= tit->second.begin();
									it != tit->second.end(); ++it											)
					{
						ppi_time currentTime;
						OParameterStringStream input;

						/*
						 * data type order below
						 * is specified inside follow methods:
						 * DbInterface::fillDebugSession
						 * ServerDbTransaction::transfer by method == "fillDebugSession"
						 * ClientTransaction::hearingTransfer
						 * ClientTransaction::saveFile
						 * ClientTransaction::loadFile
						 */
						input << it->folder;
						input << it->subroutine;
						input << it->value;
						if(it->currentTime.get() != NULL)
							currentTime= *it->currentTime;
						input << currentTime;
						input << it->content;
						m_oStoreFile << input.str() << endl;
						if(nWriteLines > 100)
						{
							std::cout << "." << flush;
							nWriteLines= 0;
						}
						++nWriteLines;
					}
				}
				std::cout << " OK" << endl;
			}
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return true;
	}

	short ClientTransaction::exist(const string& folder, const string& subroutine)
	{
		short nExist(0);
		debugSessionTimeMap::iterator timeIt;
		vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator folderIt;
		map<string, map<string, unsigned long> >::iterator foundFolder;
		map<string, unsigned long>::iterator foundSubroutine;

		if(m_o2Client.get())
			return m_o2Client->transObj()->exist(folder, subroutine);
		if(folder == "")
			return -1;
		LOCK(m_DEBUGSESSIONCHANGES);
		for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
		{
			for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
			{
				if(	folder == folderIt->folder	&&
					(	subroutine == folderIt->subroutine ||
						subroutine == ""						)	)
				{
					nExist= 1;
					break;
				}
			}
			if(nExist == 1)
				break;
		}
		if(	nExist == 0 &&
			!m_bHoldAll		)
		{
			nExist= -1;
			foundFolder= m_vsHoldFolders.find(folder);
			if(foundFolder != m_vsHoldFolders.end())
			{
				foundSubroutine= foundFolder->second.find(subroutine);
				if(foundSubroutine != foundFolder->second.end())
				{
					/*
					 * folder is set for holding
					 * but not inside debug session queue
					 * from server
					 */
					nExist= 0;
				}
			}
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return nExist;
	}

	void ClientTransaction::allFolderHolding()
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->allFolderHolding();
			return;
		}
		LOCK(m_DEBUGSESSIONCHANGES);
		m_bHoldAll= true;
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	void ClientTransaction::writeHelpUsage(const string& sfor)
	{
		if(sfor == "?")
		{
			std::cout << endl;
			std::cout << "    ?         - show this help" << endl;
			std::cout << "    ?value    - show all commands round of values "
							"inside ppi-server" << endl;
			std::cout << "    ?debug    - show all commands round "
							"debug session of working list" << endl;
			std::cout << endl;
			std::cout << "    quit            - ending connection to server" << endl;
			std::cout << "                      and abort ppi-client" << endl;
			std::cout << "    history         - show all last typed commands with number" << endl;
			std::cout << "                      which can allocate with '!<number>'" << endl;
			std::cout << "    CHANGE [user]   - changing user by current session with server" << endl;
			std::cout << "                      after pressing return, will be asked for user" << endl;
			std::cout << "                      when not written as parameter" << endl;
			std::cout << "                      and than for password" << endl;

		}else if(sfor == "?value")
		{
			std::cout << endl;
			std::cout << "    GET <folder>:<subroutine>" << endl;
			std::cout << "          -     get the current value from the subroutines in the folder" << endl;
			std::cout << "                folder and subroutine are separated with an colon" << endl;
			std::cout << "    SET <folder>:<subroutine> <value>" << endl;
			std::cout << "          -     set the given value from given subroutine in given folder" << endl;
			std::cout << "    HEAR <folder>:<subroutine>" << endl;
			std::cout << "          -     if the client has set an second connection with option --hear," << endl;
			std::cout << "                client can order with this command to hear on the given folder:subroutine's" << endl;
			std::cout << "                 for changes" << endl;
			std::cout << "    NEWENTRYS" << endl;
			std::cout << "          -     clearing all entry's which are set with the command HEAR" << endl;
			std::cout << "                this command is only when ppi-server is started with option --hear" << endl;

		}else if(sfor == "?debug")
		{
			if(m_o2Client.get())
			{
				std::cout << endl;
				std::cout << "    DEBUG [-i|-ow] <folder[:subroutine]/owreaderID>" << endl;
				std::cout << "                -   show by running server debugging messages for given folder and subroutine" << endl;
				std::cout << "                    when no subroutine given, the hole folder is set for debugging" << endl;
				std::cout << "                    by add option -i, when for folder defined an inform parameter" << endl;
				std::cout << "                    show this calculation also by debug output." << endl;
				std::cout << "                    if option -ow be set client get DEBUG info for benchmark of external ports" << endl;
				std::cout << "                    and folder have to be the OWServer ID (owreaderID)." << endl;
				std::cout << "                    (the OWServer ID you can see by starting ppi-server on command line after '### starting OWServer)" << endl;
				std::cout << "                    both option -i and -ow cannot be used in same time" << endl;
				std::cout << "    hold [-i] <folder[:subroutine]>" << endl;
				std::cout << "                -   same as command DEBUG (without possibility of option -ow)" << endl;
				std::cout << "                    but debug session output will be saved in the background" << endl;
				std::cout << "                    and will be shown only with follow commands" << endl;
				std::cout << "                    (command only be allowed when ppi-client started with hearing thread option --hear)" << endl;
				std::cout << "    run         -   show all getting debug session folders with count" << endl;
				std::cout << "    show [folder[:subroutine]]" << endl;
				std::cout << "                -   show debug session output of working list" << endl;
				std::cout << "                    which was set before with HOLDDEBUG into holding state" << endl;
				std::cout << "                    to save in background" << endl;
				std::cout << "                    when before no 'current' defined, or removed," << endl;
				std::cout << "                    and an folder and or subroutine given by command" << endl;
				std::cout << "                    this will be defined inside current layer" << endl;
				std::cout << "                    (command only be allowed when ppi-client started with hearing thread option --hear)" << endl;
				std::cout << "    current [folder[:subroutine]]" << endl;
				std::cout << "               -    show always only folder or folder:subroutine of debug session" << endl;
				std::cout << "                    running by one pass" << endl;
				std::cout << "                    by first calling it will take the first pass of folder given name which found" << endl;
				std::cout << "                    the command 'current' show always the folder by same pass" << endl;
				std::cout << "                    with below commands you can navigate thru the others" << endl;
				std::cout << "                    when typing command again with other info of folder and or subroutine," << endl;
				std::cout << "                    it will be created an new layer" << endl;
				std::cout << "                    with the new folder and or subroutine," << endl;
				std::cout << "                    in which you can go back with command 'back' in a later time." << endl;
				std::cout << "                    In this case, when create an new layer, the client" << endl;
				std::cout << "                    try to show the new folder:subroutine from same time," << endl;
				std::cout << "                    or one step before." << endl;
				std::cout << "    add <subroutine>" << endl;
				std::cout << "                -   add subroutine to current folder defined with 'current'" << endl;
				std::cout << "                    which is defined for listing" << endl;
				std::cout << "    rm <subroutine>" << endl;
				std::cout << "    remove <subroutine>" << endl;
				std::cout << "                -   remove the subroutine from current folder defined inside 'current'" << endl;
				std::cout << "                    which is defined for listing" << endl;
				std::cout << "    next        -   show also only one subroutine like 'current'" << endl;
				std::cout << "                    but always the next one" << endl;
				std::cout << "                    after command can also be typed the folder pass" << endl;
				std::cout << "                    which want to be shown" << endl;
				std::cout << "    previous    -   same as command 'NEXTDEBUG'" << endl;
				std::cout << "                    but show the subroutine before" << endl;
				std::cout << "                    after command can also be typed the folder pass" << endl;
				std::cout << "                    which want to be shown" << endl;
				std::cout << "    back        -   go from an new defined layer with 'current'" << endl;
				std::cout << "                    back to the last layer with the last time" << endl;
				std::cout << "    up          -   go from the current layer up to the next" << endl;
				std::cout << "                    which before leafe with 'back'" << endl;
				std::cout << "    STOPDEBUG [-ow] <folder[:subroutine]/owreaderID>" << endl;
				std::cout << "                -   stop debugging session for folder:subroutine" << endl;
				std::cout << "                    and or saving into background" << endl;
				std::cout << "    clear       -   clear debug content from background" << endl;
				std::cout << "                    but not currently debugging session" << endl;
				std::cout << "    save <file>" << endl;
				std::cout << "                -   save all debug session content, since time called," << endl;
				std::cout << "                    into given file in same directory where ppi-client was called" << endl;
				std::cout << "                    ending by call this command again, call 'STOPDEBUG' or stop editor" << endl;
				std::cout << "    load <file>" << endl;
				std::cout << "                -   load debug session content which was saved before with 'save'" << endl;
				std::cout << "                    This is the only one command which can called on beginning" << endl;
				std::cout << "                    ('ppi-client load <file>') where the ppi-server" << endl;
				std::cout << "                    not need to run" << endl;

			}else // if(m_o2Client.get())
			{
				std::cout << "   all debugging commands only available" << endl;
				std::cout << "   when client was started with option --hear" << endl;
			}
		}else
		{
			std::cout << endl;
			std::cout << " no helping defined for '" << sfor << "'" << endl;
		}
	}

	bool ClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool bRv;

		try{
			if(m_bHearing)
				bRv= hearingTransfer(descriptor);
			else
				bRv= userTransfer(descriptor);
		}catch(SignalException& ex)
		{
			ex.printTrace();
			exit(EXIT_FAILURE);

		}catch(std::exception& ex)
		{
			std::cout << string(ex.what()) << endl;
			exit(EXIT_FAILURE);
		}
		return bRv;
	}

	void ClientTransaction::setHoldingFolder(const string& folder, const string& subroutine)
	{
		pair<string, string> folderSub(folder, subroutine);

		if(m_o2Client.get())
		{
			m_o2Client->transObj()->setHoldingFolder(folder, subroutine);
			return;
		}
		LOCK(m_DEBUGSESSIONCHANGES);
		m_vsHoldFolders[folder][subroutine]= 0;
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	bool ClientTransaction::clearHoldingFolder(const string& folder, const string& subroutine)
	{
		bool bRv(false);
		map<string, map<string, unsigned long> >::iterator foundFolder;
		map<string, unsigned long>::iterator foundSubroutine;

		if(m_o2Client.get())
			return m_o2Client->transObj()->clearHoldingFolder(folder, subroutine);
		LOCK(m_DEBUGSESSIONCHANGES);
		if(folder != "")
		{
			foundFolder= m_vsHoldFolders.find(folder);
			if(foundFolder != m_vsHoldFolders.end())
			{
				if(subroutine != "")
				{
					foundSubroutine= foundFolder->second.find(subroutine);
					if(foundSubroutine != foundFolder->second.end())
						foundFolder->second.erase(foundSubroutine);
					if(foundFolder->second.empty())
						m_vsHoldFolders.erase(foundFolder);
				}else
					m_vsHoldFolders.erase(foundFolder);
			}
			if(m_vsHoldFolders.empty())
			{
				m_bHoldAll= false;
				bRv= true;
			}
		}else
		{
			m_vsHoldFolders.clear();
			m_bHoldAll= false;
			bRv= true;
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return bRv;
	}

	void ClientTransaction::clearDebugSessionContent()
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->clearDebugSessionContent();
			return;
		}
		LOCK(m_DEBUGSESSIONCHANGES);
		m_mmDebugSession.clear();
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	bool ClientTransaction::emptyDbgQueue() const
	{
		bool bRv(false);

		if(m_o2Client.get())
			return m_o2Client->transObj()->emptyDbgQueue();
		LOCK(m_DEBUGSESSIONCHANGES);
		if(m_vsHoldFolders.empty())
			bRv= true;
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return bRv;
	}

	map<string, unsigned long> ClientTransaction::getRunningFolderList(bool locked)
	{
		unsigned long ulRun;
		map<string, unsigned long> mRv;
		debugSessionTimeMap::iterator timeIt;
		vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator folderIt;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getRunningFolderList(locked);
		if(!locked)
			LOCK(m_DEBUGSESSIONCHANGES);
		for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
		{
			for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
			{
				if(folderIt->subroutine == "#start")
				{
					ulRun= mRv[folderIt->folder];
					++ulRun;
					mRv[folderIt->folder]= ulRun;
				}
			}
		}
		if(!locked)
			UNLOCK(m_DEBUGSESSIONCHANGES);
		return mRv;
	}

	bool ClientTransaction::subroutineSet(const string& subroutine,
					const vector<string>& subroutines)
	{
		vector<string>::const_iterator found;

		if(subroutines.empty())
			return true;//all subroutines are set
		found= find(subroutines.begin(), subroutines.end(), subroutine);
		if(found != subroutines.end())
			return true;
		return false;
	}

	IPPITimePattern* ClientTransaction::writeDebugSession(const string& folder,
					vector<string>& subroutines,
					const direction_e& show, const IPPITimePattern* curTime,
					const unsigned long nr/*= 0*/)
	{
		bool bDone(false);
		bool bWritten(false);
		unsigned long ulFoundPass(0);
		map<string, bool> vWritten;
		bool bFoundChanged(false), bSetValue(false);
		double dCurrentValue;
		unsigned long ulRun, ulLastChanged;
		unsigned long want;
		ostringstream content;
		map<string, string> mFolderStart;
		map<string, unsigned long> folderCount;
		map<string, unsigned long> mmRunning;
		debugSessionTimeMap::iterator timeIt;
		vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator folderIt;
		string sFirstSubroutine;

		if(m_o2Client.get())
			return m_o2Client->transObj()->writeDebugSession(folder, subroutines, show, curTime, nr);
		if(!subroutines.empty())
		{
			sFirstSubroutine= subroutines[0];
			if(sFirstSubroutine.substr(0, 1) == "#")
			{
				/*
				 * first subroutine is behind '#'
				 * and all subroutines should be shown
				 * and so clear 'subroutines'
				 */
				sFirstSubroutine= sFirstSubroutine.substr(1);
				subroutines.clear();
			}
		}
		m_dbgSessTime.clear();
		for(vector<string>::const_iterator it= subroutines.begin();
						it != subroutines.end(); ++it				)
		{
			vWritten[*it]= false;
		}
		LOCK(m_DEBUGSESSIONCHANGES);
		/*
		 * getRunningFolderList has to be called
		 * inside lock, because result
		 * should be unique with output content
		 */
		folderCount= getRunningFolderList(/*locked*/true);
		/*
		 * check first which folder pass
		 * should be written
		 */
		if(show != all)
		{
			if(	show != first &&
				show != last &&
				curTime->isSet() &&
				nr == 0				)
			{
				want= 0;
				bDone= false;
				for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
				{
					for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
					{
						/*
						 * write out only correct
						 * folder:subroutine when set
						 */
						if(	folder == "" || // <- show content of all folders
							folder == folderIt->folder	)
						{
							if(folderIt->subroutine == "#start")
							{
								++want;
								if(	ulFoundPass == 0 &&
									timeIt->first >= *curTime	)
								{
									ulFoundPass= want;
									if(	show != next_changed &&
										show != next_unchanged	)
									{
										if(timeIt->first > *curTime)
											--want;
										bDone= true;
										break;
									}
								}
							}else if(	folder != "" &&
										sFirstSubroutine == folderIt->subroutine	)
							{
								if(bSetValue)
								{
									if(	ulFoundPass != 0 &&
										want > ulFoundPass &&
										show == next_changed	)
									{
										if(	dCurrentValue != folderIt->value)
										{
											bFoundChanged= true;
											bDone= true;
											break;
										}
									}else if(	ulFoundPass != 0 &&
												want > ulFoundPass &&
												show == next_unchanged	)
									{
										if(dCurrentValue == folderIt->value)
										{
											bFoundChanged= true;
											bDone= true;
											break;
										}
									}else if(show == previous_changed)
									{
										if(dCurrentValue != folderIt->value)
										{// previous changed value
											ulLastChanged= want;
											bFoundChanged= true;
										}
									}else if(show == previous_unchanged)
									{
										if(dCurrentValue == folderIt->value)
										{// previous unchanged value
											ulLastChanged= want;
											bFoundChanged= true;
										}
									}
								}else
									bSetValue= true;
								dCurrentValue= folderIt->value;
							}
						}
					}
					if(bDone)
						break;
				}
				switch(show)
				{
				case current:
					if(want == 0)
						want= 1;
					break;

				case next:
					++want;
					if(want > folderCount[folderIt->folder])
					{
						string msg;

						msg=  " reaching end of folder\n";
						msg+= " show folder '";
						msg+= folderIt->folder;
						msg+= "' again from begin?\n";
						msg= ask(/*YesNo*/true, msg);
						if(msg == "N")
						{
							m_dbgSessTime= *curTime;
							UNLOCK(m_DEBUGSESSIONCHANGES);
							return &m_dbgSessTime;
						}
						want= 1;
					}
					break;

				case previous_changed:
				case previous_unchanged:
					want= ulLastChanged;
				case next_changed:
				case next_unchanged:
					/*
					 * by next changed/unchanged
					 * want is calculated pass
					 * of changed or unchanged
					 */
					if(!bFoundChanged)
					{
						string msg;

						msg=  " do not found ";
						if(	show == next_changed ||
							show == next_unchanged	)
						{
							msg+= "next ";
						}else
							msg+= "previous ";
						if(	show == next_changed ||
							show == previous_changed	)
						{
							msg+= "changed ";
						}else
							msg+= "unchanged ";
						msg+= "subroutine\n";
						msg+= " should be shown the ";
						if(	show == next_changed ||
							show == next_unchanged	)
						{
							msg+= "last ";
						}else
							msg+= "first ";
						msg+= "one?\n";
						msg= ask(/*YesNo*/true, msg);
						if(msg == "Y")
						{
							if(	show == next_changed ||
								show == next_unchanged	)
							{
								want= folderCount[folder];
							}else
								want= 1;

						}else
						{
							m_dbgSessTime= *curTime;
							UNLOCK(m_DEBUGSESSIONCHANGES);
							return &m_dbgSessTime;
						}
					}
					break;

				default:// show == previous
					if(want <= 1)
					{
						string msg;

						msg=  " reaching first entry of folder\n";
						msg=  " show folder '";
						msg+= folderIt->folder;
						msg+= "' again from end?\n";
						msg= ask(/*YesNo*/true, msg);
						if(msg == "N")
						{
							m_dbgSessTime= *curTime;
							UNLOCK(m_DEBUGSESSIONCHANGES);
							return &m_dbgSessTime;
						}
						want= folderCount[folderIt->folder];
					}else
						--want;
					break;
				}
			}else // from show != first && show != last && curTime->isSet()
			{
				if(	nr > 0 &&
					(	show == next ||
						show == previous	)	)
				{
					if(nr > folderCount[folder])
					{
						ostringstream out;

						out << "   only " << folderCount[folder];
						out << " folder count of '" << folder << "' do exist" << endl;
						cout(out.str());
						m_dbgSessTime= *curTime;
						UNLOCK(m_DEBUGSESSIONCHANGES);
						return &m_dbgSessTime;
					}
					want= nr;

				}else if(show == last)
					want= folderCount[folder];
				else
					want= 1;
			}
		}
		bDone= false;
		/*
		 * output now all content
		 * which need
		 */
		for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
		{
			for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
			{
				/*
				 * write out only correct
				 * folder:subroutine when set
				 */
				if(	folder == "" || // <- show content of all folders
					(	folder == folderIt->folder &&
						(	folderIt->subroutine == "#start" || // <- entry is starting of folder list
							folderIt->subroutine == "#inform" || // <- entry is start information
							                                       //    which can't shown by starting
							folderIt->subroutine == "#end" || // <- entry is stopping of folder list
							subroutineSet(folderIt->subroutine, subroutines)	)	)	)
				{
					bool bWrite(false);

					content.str("");
					/*
					 * create first
					 * current folder
					 * running count
					 * and current content
					 */
					ulRun= mmRunning[folderIt->folder];
					if(folderIt->subroutine == "#start")
					{
						++ulRun;
						mmRunning[folderIt->folder]= ulRun;
						content << "folder running by " << ulRun << ". time" << endl;
						content << folderIt->content;
						m_dbgSessTime= timeIt->first;
						/*
						 * when method called to show
						 * only next or previous subroutine
						 * write folder start content
						 * into an folder map
						 * because writing out later
						 * when know whether have to writing
						 */
						if(show != all)
							mFolderStart[folderIt->folder]= content.str();
					}else
						content << folderIt->content;
					if(show != all)
					{
						/*
						 * when method called to show
						 * only next or previous subroutine
						 * check running count, whether subroutine should
						 * be written
						 */

						if(	folderIt->subroutine != "#start" &&
							folderIt->subroutine != "#inform" &&
							folderIt->subroutine != "#end"		)
						{
							if(want == ulRun)
							{
								m_vsHoldFolders[folderIt->folder][folderIt->subroutine]= ulRun;
								bWrite= true;
							}
						}else
						{
							if(!bWritten)
							{
								if(folderIt->subroutine == "#inform")
								{
									string sContent;

									sContent= mFolderStart[folderIt->folder] + content.str();
									mFolderStart[folderIt->folder]= sContent;
								}

							}else // subroutine can only be #inform or #end
								bWrite= true;
						}
					}else
						bWrite= true;
					if(bWrite)
					{
						string id(getFolderID(folderIt->folder));

						if(	show != all &&
							mFolderStart[folderIt->folder] != "" )
						{
							std::cout << glob::addPrefix(id, mFolderStart[folderIt->folder]);
							mFolderStart[folderIt->folder]= "";
						}
						std::cout << glob::addPrefix(id, content.str());
						if(	folderIt->subroutine != "#start" &&
							folderIt->subroutine != "#inform" &&
							folderIt->subroutine != "#end"		)
						{
							bWritten= true;
							vWritten[folderIt->subroutine]= true;
						}
					}
					if(	bWritten &&
						show != all &&
						folderIt->subroutine == "#end"	)
					{
						bDone= true;
						break;
					}
				}// for each all folders
				if(bDone)
					break;
			}// for each all times
			if(bDone)
				break;
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		if(!bWritten)
		{
			ostringstream out;

			out << "no debug session content ";
			if(folder == "")
				out << "from working list exist" << endl;
			else
			{
				out << " exist by ";
				if(subroutines.size() != 1)
					out << "folder ";
				out << folder;
				if(subroutines.size() > 1)
				{
					out << " with follow subroutines:" << endl;
					for(vector<string>::const_iterator it= subroutines.begin();
									it != subroutines.end(); ++it				)
					{
						out << "                     " << *it << endl;
					}
				}else if(subroutines.size() == 1)
					out << ":" << subroutines[0] << endl;
				else
					out << endl;
			}
			cout(out.str());

		}else if(show != all)
		{
			bool moreThanOne(false);
			string noWrite;
			string notWritten;
			ostringstream out;

			for(map<string, bool>::iterator it= vWritten.begin();
							it != vWritten.end(); ++it				)
			{
				if(it->second == false)
				{
					if(noWrite != "")
						moreThanOne= true;
					noWrite= it->first;
					notWritten+= "                     " + it->first + "\n";
				}
			}
			if(noWrite != "")
			{
				if(moreThanOne)
					out << " follow subroutines ";
				else
					out << " subroutine '" << noWrite << "' ";
				out << "wasn't inside current folder pass" << endl;
				out << " of debug session getting from server";
				if(moreThanOne)
					out << ":";
				out << endl;
				if(moreThanOne)
					out << notWritten;
				cout(out.str());
			}
		}
		if(	show == all &&
			!bWritten		)
		{
			m_dbgSessTime.clear();
		}
		return &m_dbgSessTime;
	}

	bool ClientTransaction::existOnServer(IFileDescriptorPattern& descriptor,
					const string& folder, const string& subroutine)
	{
		string result;

		descriptor << "PERMISSION ";
		descriptor << folder;
		descriptor << ":";
		if(subroutine != "")
			descriptor << subroutine;
		else
			descriptor << "#none";
		descriptor.endl();
		descriptor >> result;
		trim(result);
		if(	result == "read" ||
			result == "write" ||
			result == "none" ||
			(	result == "noSubroutine" &&
				subroutine == ""			)	)
		{
			return true;
		}
		if(	result.substr(0, 5) == "ERROR" ||
			result.substr(0, 7) == "WARNING"	)
		{
			if(result != "ERROR 003")
				cout(string("   ") + result +"\n");
		}
		return false;
	}

	void ClientTransaction::prompt(const string& str/*= ""*/)
	{
		if(m_bScriptState)
			return;
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->prompt(str);
			return;
		}
		LOCK(m_PROMPTMUTEX);
		if(str != "")
		{
			vector<string>::size_type nLen;
			vector<string>::size_type nCount(0);
			vector<string>::iterator it;
			vector<string> spl;

			m_sPrompt= "";
			m_sLastPromptLine= "";
			m_sPromptResult= "";
			m_nResultPos= 0;
			m_nOldResultLength= 0;
			if(str.substr(str.length()-1, 1) != "\n")
			{
				split(spl, str, is_any_of("\n"));
				nLen= spl.size();
				for(it= spl.begin(); it != spl.end(); ++it)
				{
					++nCount;
					if(nCount < nLen)
						m_sPrompt+= *it + "\n";
					else
						m_sLastPromptLine= *it;
				}
			}else
			{
				m_sPrompt= str;
				m_sLastPromptLine= "";
			}
		}
		if( str != "" &&
			m_sAddPromptString != ""	)
		{
			m_sPrompt= m_sAddPromptString + m_sPrompt;
			m_sAddPromptString= "";
		}
		if(	!m_bRunHearTran &&
			!m_bRunUserTrans	)
		{
			if(m_sPrompt != "")
				std::cout << m_sPrompt;
			writeLastPromptLine(/*lock*/false);
		}
		UNLOCK(m_PROMPTMUTEX);
	}

	bool ClientTransaction::compareUserPassword(IFileDescriptorPattern& descriptor, string& user, string& pwd/*= ""*/)
	{
		int c;
		struct termios term;
		EHObj errHandle(EHObj(new ErrorHandling));
		string sSendbuf, result;

		term= m_tTermiosBackup;
		if(user == "")
		{
			if(!m_bScriptState)
				prompt("user: ");
			getline(std::cin, user);
			trim(user);
		}
		if(pwd == "")
		{
			if(!m_bScriptState)
			{
				string msg;

				term.c_lflag= term.c_lflag & ~ECHO;
				if(!tcsetattr(TCSAFLUSH, &term))
				{
					msg+= " WARNING: cannot blanking output\n";
					msg+= "          so typing of password is readable\n";
				}
				msg+= "password: ";
				prompt(msg);
			}

			do{
				c= getc(stdin);
				pwd+= c;
			}while(c != '\n');
		}
		trim(pwd);
		if(	!m_bScriptState &&
			!m_bHearing			)
		{
			std::cout << endl;
			resetTc();
		}

		if(!m_bConnected)
			sSendbuf= "U:";
		else
			sSendbuf= "CHANGE ";
		sSendbuf+= user;
		sSendbuf+= ":";
		sSendbuf+= pwd;
#ifdef SERVERDEBUG
		std::cout << "send: '" << sSendbuf << "'" << endl;
#endif // SERVERDEBUG
		sSendbuf+= "\n";
		descriptor << sSendbuf;
		descriptor.flush();
		descriptor >> result;
		trim(result);
		errHandle->setErrorStr(result);
		if(	errHandle->fail() ||
			result.substr(0, 6) == "ERROR " ||
			result.substr(0, 8) == "WARNING "	)
		{
			string prefix;
			string errmsg;

			errHandle->addMessage("ClientTransaction", "get_result");
			m_bErrWritten= true;
			if(	errHandle->hasError() ||
				result.substr(0, 6) == "ERROR "	)
			{
				prefix= "ERROR: ";
			}else
				prefix= "WARNING: ";
			if(errHandle->fail())
				errmsg= errHandle->getDescription();
			else
				errmsg= getError(descriptor, result);
			cerr << glob::addPrefix(prefix, errmsg) << endl;
			if(	errHandle->hasError() ||
				result.substr(0, 6) == "ERROR "	)
			{
				if(!errHandle->hasError())
					errHandle->setError("ClientTransaction", "user_password_error");
				return false;
			}
		}
		m_bConnected= true;
		return true;
	}

	void ClientTransaction::writeLastPromptLine(bool lock,
					string::size_type cursor/*= string::npos*/, const string& str/*= ""*/, bool end/*= false*/)
	{
		string sNullStr;

		if(m_o2Client.get())
		{
			m_o2Client->transObj()->writeLastPromptLine(lock, cursor, str, end);
			return;
		}
		if(lock)
			LOCK(m_PROMPTMUTEX);

		if(	cursor != string::npos &&
			!m_bRunHearTran	)
		{
			sNullStr.append(m_nOldResultLength, ' ');
			std::cout << "\r" << m_sLastPromptLine << sNullStr << std::flush;
		}
		if(cursor != string::npos)
		{
			m_sPromptResult= str;
			m_nOldResultLength= m_sPromptResult.length();
			if(cursor != string::npos)
				m_nResultPos= cursor;
			else
				m_nResultPos= str.length();
		}
		if(!m_bRunHearTran)
		{
			std::cout << "\r" << m_sLastPromptLine << m_sPromptResult << flush;
			if(end)
				std::cout << endl;
			else if(m_sPromptResult.length() > m_nResultPos)
			{
				std::cout << "\r" << m_sLastPromptLine;
				std::cout << m_sPromptResult.substr(0, m_nResultPos) << flush;
			}
		}

		if(lock)
			UNLOCK(m_PROMPTMUTEX);
	}

	void ClientTransaction::cout(const string& str)
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->cout(str);
			return;
		}
		LOCK(m_PROMPTMUTEX);
		m_sAddPromptString+= str;
		UNLOCK(m_PROMPTMUTEX);
	}

	void ClientTransaction::runUserTransaction(bool run)
	{
		if(m_o2Client.get())
		{
			m_o2Client->transObj()->runUserTransaction(run);
			return;
		}
		LOCK(m_PROMPTMUTEX);
		m_bRunUserTrans= run;
		UNLOCK(m_PROMPTMUTEX);
	}

	void ClientTransaction::runHearingTransaction(bool run)
	{
		LOCK(m_PROMPTMUTEX);
		m_bRunHearTran= run;
		UNLOCK(m_PROMPTMUTEX);
	}

	void ClientTransaction::printError(IFileDescriptorPattern& descriptor, const string& error)
	{
		cerr << getError(descriptor, error) << endl;
	}

	string ClientTransaction::getError(IFileDescriptorPattern& descriptor, const string& error)
	{
		int nErrorNum;
		string buffer, str;
		stringstream ss(error);
		ostringstream newerr;

		ss >> buffer;
		ss >> nErrorNum;
		if(	m_bShowENum ||
			m_bScriptState	)
		{
			return error;
		}

		newerr << "GETERRORSTRING ";
		if(buffer == "WARNING")
			nErrorNum*= -1;
		newerr << nErrorNum << endl;
		descriptor << newerr.str();
		descriptor.flush();

		descriptor >> buffer;
		trim(buffer);
		if(	!descriptor.error() &&
			buffer != ""			)
		{
			trim(buffer);
			return buffer;
		}
		str= "lost connection to server\n";
		str+= "default error string from 'src/client/lib/ClientTransaction':\n";

		// default error strings
		// when any go wrong on connection
		switch(nErrorNum)
		{
		case 1:
			str+= "client beginning fault transaction";
			break;

		case 2:
			str+= "no correct command given";
			break;

	/*	case 3:
			nErrorNum= 3;
			break;*/

		case 4:
			str+= "cannot found given folder for operation";
			break;

		case 5:
			str+= "cannot found given subroutine in folder for operation";
			break;

		case 6:
			str+= "Unknown value to set in subroutine";
			break;
		case 7:
			str+= "no filter be set for read directory";
			break;
		case 8:
			str+= "cannot read any directory";
			break;
		case 9:
			str+= "cannot found given file for read content";
			break;
		case 10:
			str+= "given ID from client do not exist";
			break;
		case 11:
			str= "wrong user or password";
			break;
		case 12:
			str+= "do not use error number 12 now";
			break;
		case 13:
			str+= "user has no permission";
			break;
		case 14:
			str+= "subroutine isn't correct defined by the settings of config file";
			break;
		case 15:
			str+= "user cannot login as first";
			break;
		case 16:
			str+= "subroutine has no correct access to device";
			break;
		case 17:
			str+= "cannot find OWServer for debugging";
			break;
		case 18:
			str+= "no communication thread is free for answer\n";
			str+= "(this case can behavior when the mincommunicationthreads parameter be 0)";
			break;
		case 19:
			str+= "server will be stopping from administrator";
			break;
		case 20:
			str+= "cannot load UserManagement correctly";
			break;
		case 21:
			str+= "unknown options after command SHOW";
			break;
		default:
			str+= "error code '" + error + "' is not defined for default";
			break;
		}
		return str;
	}

	ClientTransaction::~ClientTransaction()
	{
		if(m_o2Client.get())
		{
			m_o2Client->stop();
		}
		DESTROYMUTEX(m_DEBUGSESSIONCHANGES);
		DESTROYMUTEX(m_PROMPTMUTEX);
	}

}
