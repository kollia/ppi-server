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

/**
 * show by writing debug session
 * also time of written
 * and also times of #start by loading
 * form hard disk or directly from server
 */
#define __SHOWORDEREDTIMES 0
/**
 * show loading of starting folders
 * only by specific folder of __SPECIFICLOAD
 * when not defined, show all folders
 */
//#define __SPECIFICLOAD "Raff1_Zeit"

using namespace std;
using namespace util;
using namespace boost;

namespace server
{
	ClientTransaction::ClientTransaction(vector<string> options, string command)
	: m_pError(EHObj(new ErrorHandling)),
	  m_mOwMaxTime(),
	  m_mOwMaxCount()
	{
		m_bConnected= false;
		m_bErrWritten= false;
		m_bCorrectTC= false;
		m_bWait= false;
		m_bShowENum= false;
		m_vOptions= options;
		m_sCommand= command;
		m_bHearing= false;
		m_bOwDebug= false;
		m_fProtocol= 0;
		m_bServerLoad= false;
		m_sServerLoadStep= 1;
		m_PASSWORDCHECK= Thread::getMutex("PASSWORDCHECK");
		m_DEBUGSESSIONCHANGES= Thread::getMutex("DEBUGSESSIONCHANGES");
		m_PROMPTMUTEX= Thread::getMutex("PROMPTMUTEX");
		m_LOADMUTEX= Thread::getMutex("LOADMUTEX");
	}

	EHObj ClientTransaction::init(IFileDescriptorPattern& descriptor)
	{
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
		m_pError->setError("ClientTransaction", "get_result");
		cerr << glob::addPrefix("ERROR: ", m_pError->getDescription()) << endl;
		return m_pError;
	}
		if(!bRightServer)
		{
			string::size_type nPos;
			ostringstream decl;

			decl << descriptor.getHostAddressName() << "@" << descriptor.getPort();
			m_pError->setErrorStr(result);
			if(!m_pError->fail())
			{
				if(result.length() > 20)
					result= result.substr(0, 20) + " ...";
				nPos= result.find('\n');
				if(nPos != string::npos)
					result= result.substr(0, nPos-1) + " ...";
				decl << "@" << result;
				m_pError->setError("ClientTransaction", "undefined_server", decl.str());
			}else
			{
				m_pError->addMessage("ClientTransaction", "client_send", decl.str());
			}
			if(m_pError->hasError())
				cerr << glob::addPrefix("ERROR: ", m_pError->getDescription()) << endl;
			else
				std::cout << glob::addPrefix("WARNING: ", m_pError->getDescription()) << endl;
			m_bErrWritten= true;
			return m_pError;
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
			if(	//m_bWait &&
				!m_bHearing	)
			{
				readTcBackup();
			}
			if(!compareUserPassword(descriptor, user, pwd))
			{
				m_pError->setError("ClientTransaction", "user_password_error");
				m_pError->addMessage("ClientTransaction", "get_result");
				return m_pError;
			}
		}
		if(bSecConn)
		{
			HearingThread* pThread;

			pThread= new HearingThread(descriptor.getHostAddressName(), descriptor.getPort(),
							sCommID, user, pwd, this, m_bOwDebug);
			m_oHearObj= SHAREDPTR::shared_ptr<IHearingThreadPattern>(pThread);
			m_o2Client= m_oHearObj;
			(*m_pError)= pThread->start();
		}
		if(	!m_bHearing &&
			!m_pError->hasError()	)
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
			command("first inform");
			command("first external");
			command("first folder-");
			command("first folder");
			command("last");
			command("last inform");
			command("last external");
			command("last folder-");
			command("last folder");
			command("up");
			command("back");
			command("clear");
			command("add #subroutine");
			command("remove #subroutine");
			command("save #file");
			command("load #file");

			command("current");
			command("current #folder");
			command("current #folderSub");

			command("next");
			command("next inform");
			command("next external #subroutine");
			command("next folder- #subroutine");// <- folder without external
			command("next folder #subroutine");// <- folder with external
			command("next changed #subroutine");
			command("next unchanged #subroutine");

			command("previous");
			command("previous inform");
			command("previous external #subroutine");
			command("previous folder- #subroutine");// <- folder without external
			command("previous folder #subroutine");// <- folder with external
			command("previous changed #subroutine");
			command("previous unchanged #subroutine");


			// commands sending to server
			command("CHANGE #string");
			command("PERMISSION #folderSub");
			command("GET #folderSub");
			command("SET #folderSub #string");
			command("HEAR #folderSub");
			command("NEWENTRYS");
			command("FIXHEARING #folderSub");
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
		return m_pError;
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

	void ClientTransaction::implementFolderSorting(
					SHAREDPTR::shared_ptr<
						IDbFillerPattern::dbgSubroutineContent_t> content)
	{
		sortedFolderSessionIterator foundFolder;
		folderSessionIterator foundFolderObj;
		static map<string, bool> mFolderRun;
		map<string, bool>::iterator itFolderRun;
		static map<string, ppi_time> mLastImplement;
		map<string, ppi_time>::iterator itLastImplement;

		if(m_mSortedSessions.empty())
		{
			mLastImplement.clear();
			mFolderRun.clear();
		}
#if 0
		/*
		 * debug
		 * definition
		 */
		if(content->folder == "Raff1_port")
		{
			std::cout << "implement " << content->folder << ":" << content->subroutine;
			std::cout << "  " << content->currentTime->toString(true) << std::endl;
			if(content->subroutine == "#start")
				std::cout << flush;
		}
#endif
		itFolderRun= mFolderRun.find(content->folder);
		if(itFolderRun == mFolderRun.end())
		{
			mFolderRun[content->folder]= false;
			itFolderRun= mFolderRun.find(content->folder);
		}
		if(content->subroutine == "#start")
			itFolderRun->second= true;
		else if(content->subroutine == "#end")
			itFolderRun->second= false;
		if(content->subroutine.substr(0, 1) != "#")
			m_mFolderSubs[content->folder].insert(content->subroutine);
		m_mmDebugSession[*content->currentTime].push_back(content);
		foundFolder= m_mSortedSessions.find(content->folder);
		if(foundFolder == m_mSortedSessions.end())
		{
			m_mSortedSessions[content->folder]= map<ppi_time, sorted_debugSessions_t>();
			foundFolder= m_mSortedSessions.find(content->folder);
		}
		itLastImplement= mLastImplement.find(content->folder);
		if(itLastImplement == mLastImplement.end())
		{
			ppi_time tmImpl(*content->currentTime), addTime;

			/**
			 * when an object folder with current time exist
			 * add 1 microsecond for new implementation
			 * because otherwise before defined object folder
			 * will be overwritten
			 */
			addTime.tv_sec= 0;
			addTime.tv_usec= 1;
			while(foundFolder->second.find(tmImpl) != foundFolder->second.end())
				tmImpl+= addTime;
			foundFolder->second[tmImpl]= sorted_debugSessions_t();
			foundFolderObj= foundFolder->second.find(tmImpl);
			mLastImplement[content->folder]= tmImpl;
			itLastImplement= mLastImplement.find(content->folder);
			if(content->subroutine == "#end")
				mLastImplement.erase(itLastImplement);
		}else
		{
			foundFolderObj= foundFolder->second.find(itLastImplement->second);
			if(content->subroutine == "#end")
			{
				/*
				 * shift folder object from current time inserted
				 * to new time of start
				 */
				foundFolder->second[*content->currentTime]= foundFolderObj->second;
				foundFolder->second.erase(foundFolderObj);
#if(__SHOWORDEREDTIMES)
#ifdef __SPECIFICLOAD
				if(content->folder == __SPECIFICLOAD)
#endif // #ifdef __SPECIFICLOAD
				{
					std::cout << "    move created content from "
						<< itLastImplement->second.toString(/*as date*/true)
						<< " to "
						<< content->currentTime->toString(/*as date*/true) << endl;
					std::cout << "    " << content->folder << " has now "
						<< m_mSortedSessions[content->folder].size()
						<< " objects from start to end" << endl;
				}
#endif // #if(__SHOWORDEREDTIMES)
				foundFolderObj= foundFolder->second.find(*content->currentTime);
				itLastImplement->second= *content->currentTime;
				mLastImplement.erase(itLastImplement);
			}

		}
		if(	itFolderRun->second == true ||
			content->subroutine == "#end" ||
			content->subroutine == "#started"	)
		{
			/*
			 * implement also #inform subroutine
			 * inside folder content
			 * when #start be defined
			 */
			foundFolderObj->second.folder[*content->currentTime]= content;

		}else if(content->subroutine == "#inform")
		{
			foundFolderObj->second.inform[*content->currentTime]= content;

		}else
			foundFolderObj->second.external[*content->currentTime]= content;
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
		bool bErrorWritten(false);
		long nsec, nmsec, nusec;
		//timeval time;
		char buf[10];
		char stime[22];
		struct tm ttime;
		string sDo;
		device_debug_t tdebug;
		map<string, sorted_debugSessions_t> preDefSessions;
		map<string, sorted_debugSessions_t>::iterator foundFolder;

		do{
			bErrorWritten= false;
			load(/*get from server*/false);
			descriptor >> result;
			trim(result);
			if(result == "done")
				continue;
			if(result == "#password#")
			{
				descriptor << getCurrentPassword();
				descriptor.endl();
				continue;
			}
			load(/*get from server*/true);
			if(result == "ppi-server debugsession")
			{
				bool bFirstOutput(true);

				do{
					descriptor >> result;
					trim(result);
					if(result == "#password#")
					{
						descriptor << getCurrentPassword();
						descriptor.endl();

					}else if(	result != "done" &&
								result != "#stopclient"	)
					{
						IParameterStringStream input(result);
						//map<string, map<string, unsigned long> >::iterator foundFolder;
						//map<string, unsigned long>::iterator foundSubroutine;

						load(/*get from server*/true);
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
						if(subroutine != "#setDebug")
						{// folder is set for holding
							SHAREDPTR::shared_ptr<
								IDbFillerPattern::dbgSubroutineContent_t> subCont;

							subCont= SHAREDPTR::shared_ptr<
											IDbFillerPattern::dbgSubroutineContent_t>
													(new IDbFillerPattern::dbgSubroutineContent_t);
							subCont->folder= folder;
							subCont->subroutine= subroutine;
							subCont->value= value;
							subCont->content= content;
							subCont->currentTime=
								SHAREDPTR::shared_ptr<IPPITimePattern>
													(new ppi_time(time));
							implementFolderSorting(subCont);
						}else
						{// folder isn't set for holding
							id= getFolderID(folder);
							if(bFirstOutput)
							{
								runHearingTransaction(true);
								bFirstOutput= false;
								std::cout << endl;
							}
							std::cout << glob::addPrefix(id, content) << endl;
						}
						UNLOCK(m_DEBUGSESSIONCHANGES);
					}
				}while(	result != "done" &&
						result != "#stopclient"	);
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
			if(	result == "#stopclient" ||
				(	result.length() > 5 &&
					result.substr(0, 5) == "ERROR"	) ||
				(	result.length() > 7 &&
					result.substr(0, 7) == "WARNING"	)	)
			{
				bool bError(false);

				if(	result.length() > 5 &&
					result.substr(0, 5) == "ERROR"	)
				{
					bError= true;
				}
				runHearingTransaction(true);
				std::cout << endl;
				if(result != "#stopclient")
				{
					if(result != "ERROR 019")
						std::cout << "Uncaught ";
					if(bError)
						std::cout << "ERROR ";
					else
						std::cout << "WARNING ";
					std::cout << "message by hearing thread getting from server" << endl;
					printError(descriptor, result);
				}else
					std::cout << " server stopping regular hearing thread" << endl;
				if(	result != "#stopclient"	&&
					result != "ERROR 019"		)
				{
					runHearingTransaction(false);
					prompt();
				}
				bErrorWritten= true;
			}
			if(!bErrorWritten)
			{
				if(m_bOwDebug && bHeader)
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
		}while(	!descriptor.eof() &&
				result != "#stopclient" &&
				result != "ERROR 019"		);
		if(descriptor.eof())
		{
			std::cout << endl;
			std::cout << "### by heareing on server on second connection "
							"-> connection is broken" << endl;
		}
		runHearingTransaction(false);
		prompt();
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
		struct termios termios_flag;

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

					m_pError->setErrnoError("ClientTransaction", "tcgetattr", ENOTTY);/* Not a typewriter */
					errmsg << "getline() " << __FILE__ << "  line " << __LINE__ << endl;
					errmsg << m_pError->getDescription();
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
		try{
			if(!m_bScriptState)
			{
				termios_flag= m_tTermiosBackup;
				termios_flag.c_lflag &= ~ICANON;
				termios_flag.c_lflag &= ~ECHO;
				//termios_flag.c_cc[VMIN] = 1;
				//termios_flag.c_cc[VTIME] = 0;
				tcsetattr(TCSANOW, &termios_flag);
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
		}catch(SignalException& ex)
		{
			resetTc();
			throw ex;

		}catch(std::exception& ex)
		{
			resetTc();
			throw ex;

		}catch(...)
		{
			resetTc();
			throw "unexpected exception";
		}
		resetTc();
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
		if(!m_bCorrectTC)
			return false;
		if((::tcsetattr(STDIN_FILENO, action, termiosp)) < 0)
		{
			m_pError->setErrnoError("ClientTransaction", "tcsetattr", errno);
			cerr << glob::addPrefix("ERROR: ", m_pError->getDescription()) << endl;
			m_bErrWritten= true;
			correctTC(false);
			return false;
		}
		return true;
	}

	void ClientTransaction::correctTC(bool read)
	{
		m_bCorrectTC= true;
		if(m_o2Client.get())
			m_o2Client->transObj()->correctTC(read);
	}

	void ClientTransaction::setTcBackup(const struct termios& backup)
	{
		if(m_o2Client.get())
			m_o2Client->transObj()->setTcBackup(backup);
		else
		{
			m_bCorrectTC= true;
			m_tTermiosBackup= backup;
		}
	}

	void ClientTransaction::readTcBackup()
	{
		int nErrno(0);

		m_bScriptState= false;
		if((::tcgetattr(STDIN_FILENO, &m_tTermiosBackup)) < 0)
		{
			nErrno= errno;
			correctTC(false);
			if(nErrno != ENOTTY)/* Not a typewriter */
			{
				m_pError->setErrnoError("ClientTransaction", "tcgetattr", nErrno);
				cerr << glob::addPrefix("ERROR: ", m_pError->getDescription()) << endl;
				m_bErrWritten= true;
			}else
				m_bScriptState= true;
		}else
		{
			correctTC(true);
			if(m_o2Client.get())
				m_o2Client->transObj()->setTcBackup(m_tTermiosBackup);
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

		bool bFirst(true);
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

		correctTC(m_bCorrectTC);
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
				if(bFirst)
				{
					correctTC(m_bCorrectTC);
					bFirst= false;
				}
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
					writeHelpUsage(command[0], (m_o2Client.get() != NULL));

			}else if(command[0] == "CHANGE")
			{
				paramDefs.push_back("[user]");
				if(checkWaitCommandCount(command, bErrorWritten, &paramDefs))
				{
					struct termios term;
					string user, pwd;

					readTcBackup();
					bSendCommand= false;
					term= m_tTermiosBackup;
					if(command.size() == 2)
						user= command[1];
					runUserTransaction(false);
					compareUserPassword(descriptor, user, pwd);
					runUserTransaction(true);
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

			}else if(command[0] == "remove")
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
				(	command[0] == "current" ||
					command[0] == "next" ||
					command[0] == "previous" ||
					command[0] == "first" ||
					command[0] == "last" ||
					command[0] == "up" ||
					command[0] == "back"		)	)
			{
				short nExist(-2);
				direction_t direction;
				short folderNr(0);

				direction.direction= all;
				direction.action= none;
				bSendCommand= false;
				if(command[0] == "first")
				{
					direction.direction= first;

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
					direction.direction= current;

				}else if(command[0] == "next")
				{
					direction.direction= next;

				}else if(	command[0] == "previous")
				{
					direction.direction= previous;

				}else
					direction.direction= last;
				if(!bErrorWritten)
				{
					if(	direction.direction == all ||
						direction.direction == current	)
					{
						paramDefs.push_back("[ <folder>:<subroutine> ]");

					}else if(	direction.direction == next ||
								direction.direction == previous	)
					{
						paramDefs.push_back("[ folder number or 'inform/external/folder-/folder/changed/unchanged' ]");
						paramDefs.push_back("[ <subroutine> when not "
										"only one defined to show by changed/unchanged ]");

					}else if(	direction.direction == first ||
								direction.direction == last		)
					{
						paramDefs.push_back("[ inform, external, folder or folder- ]");
					}
					if(checkHearCommandCount(command, bErrorWritten, &paramDefs))
					{
						folder= "";
						subroutine= "";
						if(command.size() > 1)
						{
							if(	direction.direction == next ||
								direction.direction == previous ||
								direction.direction == first ||
								direction.direction == last			)
							{
								bool bChangedUnchanged(false);

								if(command[1] == "inform")
								{
									direction.action= inform;

								}else if(command[1] == "external")
								{
									direction.action= external;

								}else if(command[1] == "folder-")
								{
									direction.action= ClientTransaction::folder;

								}else if(command[1] == "folder")
								{
									direction.action= folder_external;

								}else if(	command[1] == "changed" &&
											direction.direction != first &&
											direction.direction != last		)
								{
									bChangedUnchanged= true;
									direction.action= changed;

								}else if(	command[1] == "unchanged" &&
											direction.direction != first &&
											direction.direction != last		)
								{
									bChangedUnchanged= true;
									direction.action= unchanged;

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
											if(direction.action == changed)
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
								}// if(bChangedUnchanged
								if(	!bErrorWritten &&
									!bChangedUnchanged &&
									(	direction.action == none ||
										command.size() == 3			)	)
								{
									int pos(1);

									if(command.size() == 3)
										pos= 2;
									iresult.str(command[pos]);
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
							command.size() > 1 &&
							(	direction.action == none ||
								command.size() > 2			)	)
						{
							vector<string> spl;

							if(	direction.direction != all &&
								direction.direction != current &&
								folderNr == 0 &&
								(	direction.action == none ||
									command.size() > 2				)	)
							{
								cout(" for command '" + command[0] + "'"
										" is no definition of folder[:subroutine] allowed\n");
								bErrorWritten= true;

							}else
							{
								int pos(1);

								if(direction.action != none)
									pos= 2;
								split(spl, command[pos], is_any_of(":"));
								folder= spl[0];
								if(spl.size() > 1)
									subroutine= spl[1];
								nExist= exist(folder, subroutine);
								if(nExist != 1)
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
											direction.action != changed &&
											direction.action != unchanged )
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
									(	direction.direction == current ||
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
							if(	direction.direction != all &&
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

							if(direction.direction != all)
							{
								folder= vLayers[nCurLayer].first;
								if(folderNr == 0)
									newTime= vLayers[nCurLayer].second.first;
								vShowSubroutines= vLayers[nCurLayer].second.second;

							}else
								vShowSubroutines.push_back(subroutine);
							newTime= *writeDebugSession(
											folder, vShowSubroutines, direction, &newTime, folderNr	);
							if(	direction.direction == all ||
								newTime.isSet()		)
							{
								if(direction.direction != all)
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
									"            please define with DEBUG\n");
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
					}else if(command[0] == "STOPDEBUG")
					{
						bReadDbgSessionContent= false;
						closeFile();
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
				{
					if(	m_bWait &&
						!m_bScriptState	)
					{
						result= "   " + result;
					}
					result+= "\n";
					cout(result);
				}
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
					SHAREDPTR::shared_ptr<
						IDbFillerPattern::dbgSubroutineContent_t> sub;

					sub= SHAREDPTR::shared_ptr<
									IDbFillerPattern::dbgSubroutineContent_t>
											(new IDbFillerPattern::dbgSubroutineContent_t);
					result >> sub->folder;
					if(!result.fail())
						result >> sub->subroutine;
					if(!result.fail())
						result >> sub->value;
					if(!result.fail())
						result >> currentTime;
					if(!result.fail())
					{
						sub->currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time(currentTime));
						result >> sub->content;
					}
					if(!result.fail())
					{
#if(__SHOWORDEREDTIMES)
						if(	sub->subroutine == "#start"
#ifdef __SPECIFICLOAD
							&& sub->folder == __SPECIFICLOAD
#endif // #ifdef __SPECIFICLOAD
																)
						{
							std::cout << "reading starting folder from folder "
											<< sub->folder << " at "
											<< sub->currentTime->toString(/*as date*/true)
											<< endl;
						}
#endif // if(__SHOWORDEREDTIMES)
						implementFolderSorting(sub);
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
					for(sharedDebugSessionVec::iterator it= tit->second.begin();
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
						input << (*it)->folder;
						input << (*it)->subroutine;
						input << (*it)->value;
						if((*it)->currentTime.get() != NULL)
							currentTime= *(*it)->currentTime;
						input << currentTime;
						input << (*it)->content;
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
		sharedDebugSessionVec::iterator folderIt;

		if(m_o2Client.get())
			return m_o2Client->transObj()->exist(folder, subroutine);
		if(folder == "")
			return -1;
		LOCK(m_DEBUGSESSIONCHANGES);
		for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
		{
			for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
			{
				if(	folder == (*folderIt)->folder	&&
					(	subroutine == (*folderIt)->subroutine ||
						subroutine == ""						)	)
				{
					nExist= 1;
					break;
				}
			}
			if(nExist == 1)
				break;
		}
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return nExist;
	}

	void ClientTransaction::writeHelpUsage(const string& sfor, bool editor)
	{
		if(sfor == "?")
		{
			std::cout << endl;
			std::cout << "    ?         - show this help" << endl;
			std::cout << "    ?value    - show all commands round of values "
							"inside ppi-server" << endl;
			if(editor)
			{
				std::cout << "    ?debug    - show all commands round "
							"debug session of working list" << endl;
			}
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
			if(editor)
			{
				std::cout << endl;
				std::cout << "    DEBUG [-i|-ow] <folder[:subroutine]/owreaderID>" << endl;
				std::cout << "                -   load by running server debugging messages for given folder and subroutine" << endl;
				std::cout << "                    into an queue to scroll with some commands (current, next ...) throu it." << endl;
				std::cout << "                    when no subroutine given, the hole folder is set for debugging" << endl;
				std::cout << "                    by add option -i, when for folder defined an inform parameter" << endl;
				std::cout << "                    show this calculation also by debug output." << endl;
				std::cout << "                    (command only be allowed when ppi-client started with hearing thread option --hear)" << endl;
				std::cout << "                    if option -ow be set client get DEBUG info for benchmark of external ports" << endl;
				std::cout << "                    and show the output on an live stream on screen" << endl;
				std::cout << "                    folder parameter have to be the OWServer ID (owreaderID)." << endl;
				std::cout << "                    (the OWServer ID you can see by starting ppi-server on command line after '### starting OWServer)" << endl;
				std::cout << "                    both option -i and -ow cannot be used in same time" << endl;
				std::cout << "    run         -   show all getting debug session folders with count" << endl;
				std::cout << "    show [folder[:subroutine]]" << endl;
				std::cout << "                -   show debug session output of working list" << endl;
				std::cout << "                    which was set before with DEBUG into holding state" << endl;
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
				std::cout << "    remove <subroutine>" << endl;
				std::cout << "                -   remove the subroutine from current folder defined inside 'current'" << endl;
				std::cout << "                    which is defined for listing" << endl;
				std::cout << "    first [type]" << endl;
				std::cout << "                -   show first subroutine of folder when no type be set" << endl;
				std::cout << "                    (additional optional second [type] parameter see by command 'next')" << endl;
				std::cout << "    next [type] -   show also only one subroutine like 'current'" << endl;
				std::cout << "                    but always the next one" << endl;
				std::cout << "                    after command can also be typed the folder pass" << endl;
				std::cout << "                    which want to be shown" << endl;
				std::cout << "                    additional type can be:" << endl;
				std::cout << "                      inform    - for folder information of change from" << endl;
				std::cout << "                                  other folder or external clients" <<endl;
				std::cout << "                      external  - for external started changing of subroutine content" << endl;
				std::cout << "                                  can be changed from other folder" << endl;
				std::cout << "                                  or started from an TIMER subroutine" << endl;
				std::cout << "                      folder-   - show only folder between folder start and stop" << endl;
				std::cout << "                                  (because an minus ('-') is followed after type name 'folder')" << endl;
				std::cout << "                      folder    - show only folder or external content (no inform)" << endl;
				std::cout << "                      <number>  - show the next folder (like folder-) on given position" << endl;
				std::cout << "                   > next types only usable inside folder <" << endl;
				std::cout << "                      changed   - show the next or previous subroutine which is changed" << endl;
				std::cout << "                      unchanged - show the next or previous unchanged subroutine" << endl;
				std::cout << "    previous [type]   -   same as command 'NEXTDEBUG'" << endl;
				std::cout << "                    but show the subroutine before" << endl;
				std::cout << "                    after command can also be typed the folder pass" << endl;
				std::cout << "                    which want to be shown" << endl;
				std::cout << "                    (additional optional second [type] parameter see by command 'next' before)" << endl;
				std::cout << "    last [type] -   show last subroutine of folder when no type be set" << endl;
				std::cout << "                    (additional optional second [type] parameter see by command 'next' before)" << endl;
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

		if(m_bHearing)
		{
			try{
				bRv= hearingTransfer(descriptor);

			}catch(SignalException& ex)
			{
				ex.addMessage("reading second connection from server");
				ex.printTrace();
				bRv= true;

			}catch(std::exception& ex)
			{
				std::cout << "std exception by reading second connection from server" << endl;
				std::cout << string(ex.what()) << endl;
				bRv= true;
			}
		}else
		{
			try{
				bRv= userTransfer(descriptor);

			}catch(SignalException& ex)
			{
				ex.addMessage("working inside editor");
				ex.printTrace();
				resetTc();
				exit(EXIT_FAILURE);

			}catch(std::exception& ex)
			{
				std::cout << "std exception inside editor" << endl;
				std::cout << string(ex.what()) << endl;
				resetTc();
				exit(EXIT_FAILURE);
			}
		}
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
		m_mSortedSessions.clear();
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	map<string, unsigned long> ClientTransaction::getRunningFolderList(bool locked, bool outside/*= false*/)
	{
		unsigned long ulRun;
		map<string, bool> mFoundStart;
		map<string, unsigned long> mRv;
		debugSessionTimeMap::iterator timeIt;
		sharedDebugSessionVec::iterator folderIt;

		if(m_o2Client.get())
			return m_o2Client->transObj()->getRunningFolderList(locked, outside);
		if(!locked)
			LOCK(m_DEBUGSESSIONCHANGES);
		for(sortedFolderSessionIterator fIt= m_mSortedSessions.begin();
						fIt != m_mSortedSessions.end(); ++fIt			)
		{
			unsigned long count(0);

#if(__SHOWORDEREDTIMES)
#ifdef __SPECIFICLOAD
			if(fIt->first == __SPECIFICLOAD)
#endif // #ifdef __SPECIFICLOAD
			{
				std::cout << " " << fIt->first << endl;
			}
#endif // if(__SHOWORDEREDTIMES)
			for(folderSessionIterator oIt= fIt->second.begin();
							oIt != fIt->second.end(); ++oIt		)
			{
				if(!oIt->second.folder.empty())
				{
					++count;
#if(__SHOWORDEREDTIMES)
#ifdef __SPECIFICLOAD
					if(fIt->first == __SPECIFICLOAD)
#endif // #ifdef __SPECIFICLOAD
					{
						std::cout << "    reading " <<
							oIt->second.folder.begin()->first.toString(/*as date*/true) <<
							" on END time " << oIt->first.toString(/*as date*/true) << endl;
					}
#endif // if(__SHOWORDEREDTIMES)
				}
			}
			mRv[fIt->first]= count;
		}
		if(!outside)
		{
			if(!locked)
				UNLOCK(m_DEBUGSESSIONCHANGES);
			return mRv;
		}
		mRv.clear();
		for(timeIt= m_mmDebugSession.begin(); timeIt != m_mmDebugSession.end(); ++timeIt)
		{
			for(folderIt= timeIt->second.begin(); folderIt != timeIt->second.end(); ++folderIt)
			{
				map<string, bool>::iterator found;

				found= mFoundStart.find((*folderIt)->folder);
				if(found == mFoundStart.end())
				{
					mFoundStart[(*folderIt)->folder]= false;
					found= mFoundStart.find((*folderIt)->folder);
				}
				if(	(*folderIt)->subroutine == "#start" ||
					(	outside == true &&
						found->second == false &&
						(*folderIt)->subroutine != "#started"	)	)
				{
					ulRun= mRv[(*folderIt)->folder];
					++ulRun;
					mRv[(*folderIt)->folder]= ulRun;
					if((*folderIt)->subroutine == "#start")
						found->second= true;
				}
				if((*folderIt)->subroutine == "#end")
					found->second= false;
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

	bool ClientTransaction::subroutineSet(const dbSubroutineInfoType& folder,
					const vector<string>& subroutines)
	{
		for(dbSubroutineInfoType::const_iterator it= folder.begin();
						it != folder.end(); ++it				)
		{
			if(subroutineSet(it->second->subroutine, subroutines))
				return true;
		}
		return false;
	}

	ClientTransaction::found_subroutine_t ClientTransaction::currentSubroutineTyp(
															IPPITimePattern* curTime,
															folderSessionIterator folderObj,
															vector<string>& subroutines		)
	{
		found_subroutine_t tRv, nearest;
		dbSubroutineInfoType::iterator it, previous;

		nearest.vector= none;
		/*
		 * search inside
		 * informer subroutines
		 */
		tRv.vector= none;
		tRv.first= folderObj->second.inform.begin();
		tRv.previous= tRv.first;
		tRv.last= folderObj->second.inform.end();
		tRv.count= 0;
		previous= tRv.previous;
		for(it= tRv.first; it != tRv.last; ++it	)
		{
			if(it->first <= *curTime)
			{
				tRv.vector= inform;
				tRv.previous= previous;
				tRv.found= it;
				if(it->first == *curTime)
					return tRv;
			}else
				break;
			previous= it;
			++tRv.count;
		}
		if(tRv.vector != none)
			nearest= tRv;
		/*
		 * search inside
		 * external subroutines
		 */
		tRv.vector= none;
		tRv.first= folderObj->second.external.end();
		tRv.previous= tRv.first;
		tRv.last= folderObj->second.external.end();
		tRv.count= 0;
		previous= folderObj->second.external.begin();
		dbSubroutineInfoType::iterator nextPrevious= previous;
		for(it= folderObj->second.external.begin();
			it != folderObj->second.external.end(); ++it	)
		{
			if(subroutineSet(it->second->subroutine, subroutines))
			{
				if(tRv.first == folderObj->second.external.end())
				{
					tRv.first= it;
					tRv.previous= it;
					previous= it;
					nextPrevious= it;
				}
				if(it->first <= *curTime)
				{
					tRv.vector= external;
					tRv.previous= previous;
					tRv.found= it;
					previous= nextPrevious;
					nextPrevious= it;
				}
				++tRv.count;
				previous= it;
				tRv.last= it;
			}
		}
		if(tRv.vector != none)
		{
			if(tRv.last != folderObj->second.external.end())
				++tRv.last;
			if(	nearest.vector == none ||
				ppi_time(*curTime - tRv.found->first) <=
							ppi_time(*curTime - nearest.found->first)	)
			{
				nearest= tRv;
			}
		}
		if(	tRv.vector != none &&
			tRv.found->first == *curTime	)
		{
			return tRv;
		}
		/*
		 * search inside
		 * running subroutines
		 * between folder starting and ending
		 */
		tRv.vector= none;
		tRv.first= folderObj->second.folder.begin();
		tRv.previous= tRv.last;
		tRv.last= folderObj->second.folder.end();
		tRv.count= 0;
		for(it= tRv.first; it != tRv.last; ++it	)
		{
			if(	(	subroutines.empty() &&
					it->second->subroutine == "#start"	) ||
				(	!subroutines.empty() &&
					it->second->subroutine == subroutines[0]	)	)
			{
				/*
				 * search always only
				 * for first subroutine
				 */
				tRv.vector= folder;
				tRv.previous= previous;
				tRv.found= it;
				if(it->first == *curTime)
					return tRv;
				break;
			}
		}
		if(	tRv.vector != none &&
			(	nearest.vector == none ||
				ppi_time(*curTime - tRv.found->first) <=
						ppi_time(*curTime - nearest.found->first)	)	)
		{
			nearest= tRv;
		}
		return nearest;
	}



	IPPITimePattern* ClientTransaction::writeDebugSession(const string& folder,
					vector<string>& subroutines,
					const direction_t show, IPPITimePattern* curTime,
					const unsigned long nr/*= 0*/)
	{
		bool bDisplayedContent(false);
		bool bFoundCurrentFolder(false);
		size_t nOutput, nFolderOutput, existFolderObjs;
		size_t nLastChangedFolder, nLastUnchangedFolder;
		size_t nLastInformFolder, nLastExternalFolder;
		direction_t make;
		IPPITimePattern* pRv;
		ppi_time incommingTime;
		ostringstream content;
		sortedFolderSessionIterator currFolderIt;
		dbSubroutineInfoType::iterator showSubroutine, endSubroutine;
		folderSessionIterator showContent, previousContent;
		folderSessionIterator lastChangedContent, lastUnchangedContent;
		folderSessionIterator lastInformContent, lastExternalContent;
		map<ppi_time,
			SHAREDPTR::shared_ptr<
				IDbFillerPattern::dbgSubroutineContent_t> >::
					iterator innerIt;
		static ppi_time nullTime;

		if(m_o2Client.get())
			return m_o2Client->transObj()->writeDebugSession(folder, subroutines, show, curTime, nr);

		std::cout << endl;
		incommingTime= *curTime;
		make.direction= show.direction;
		make.action= show.action;
		pRv= &nullTime;
		LOCK(m_DEBUGSESSIONCHANGES);
		currFolderIt= m_mSortedSessions.find(folder);
		if(currFolderIt == m_mSortedSessions.end())
		{
			std::cout << "  folder " << folder << " isn't inside "
							"loaded debug content from server" << endl;
			return pRv;
		}
		do{// while(!bDisplayedContent);
			if(make.direction == last)
			{
				folderSessionReverseIterator oIt;

				/*
				 * search for
				 * last entry
				 */
				nFolderOutput= currFolderIt->second.size();
				oIt= currFolderIt->second.rbegin();
				while(oIt != currFolderIt->second.rend())
				{
					if(make.action == none)
					{
						if(!oIt->second.folder.empty())
						{
							make.action= IClientTransactionPattern::folder;
							++oIt;
							showContent= oIt.base();
							break;

						}else if(subroutineSet(oIt->second.external, subroutines))
						{
							make.action= external;
							++oIt;
							showContent= oIt.base();
							break;

						}else if(!oIt->second.inform.empty())
						{
							make.action= inform;
							++oIt;
							showContent= oIt.base();
							break;
						}
					}else // if(make.action == none)
					{
						if(	(	make.action == ClientTransaction::folder ||
								make.action == folder_external				) &&
							!oIt->second.folder.empty()							)
						{
							make.action= IClientTransactionPattern::folder;
							++oIt;
							showContent= oIt.base();
							break;
						}
						if(	(	make.action == external ||
								make.action == folder_external	) &&
							subroutineSet(oIt->second.external, subroutines)	)
						{
							make.action= external;
							++oIt;
							showContent= oIt.base();
							break;
						}
						if(	make.action == inform &&
							!oIt->second.inform.empty()	)
						{
							make.action= inform;
							++oIt;
							showContent= oIt.base();
							break;
						}
					} // end else from if(make.action == none)
					--nFolderOutput;
					++oIt;
				}// reverse iteration of curFolderIt
				if(oIt == currFolderIt->second.rend())
				{
					showContent= currFolderIt->second.begin();
					make.action= inform;
				}
			}else // if(make.direction == last)
			{
				/*
				 * as first searching
				 * correct folder object
				 * where defined inform, external and
				 * all subroutines from one folder
				 * from start to end
				 */
				nFolderOutput= 0;
				nLastChangedFolder= 0;
				nLastUnchangedFolder= 0;
				endSubroutine= currFolderIt->second.begin()->second.inform.end();
				showSubroutine= endSubroutine;
				showContent= currFolderIt->second.end();
				lastChangedContent= showContent;
				lastUnchangedContent= showContent;
				lastInformContent= showContent;
				lastExternalContent= showContent;
				previousContent= currFolderIt->second.end();
				do{
					folderSessionIterator oIt= currFolderIt->second.begin();

					while(oIt != currFolderIt->second.end())
					{
						++nFolderOutput;
						if(	!curTime->isSet() ||
							make.direction == first	)
						{
							if(	make.action == changed ||
								make.action == unchanged	)
							{
								make.action= ClientTransaction::folder;

							}else if(make.action == none)
								make.action= inform;
							showContent= oIt;
							nFolderOutput= 1;
							break;

						}else if(	!bFoundCurrentFolder &&
									(	*curTime < oIt->first ||	// when no subroutine exist inside folder
										oIt->second.folder.empty()	)	) // last entry be reached
						{
							found_subroutine_t dir;

							showContent= oIt;
							dir= currentSubroutineTyp(curTime, oIt, subroutines);
							if(make.direction != current)
							{
								if(	(	dir.vector == inform ||
										dir.vector == external	) &&
									(	show.action == changed ||
										show.action == unchanged	)	)
								{
									std::cout << "  displayed ";
									if(dir.vector == inform)
										std::cout << "information output ";
									else
										std::cout << "  external folder ";
									std::cout << "has no value for stepping by ";
									if(show.action == unchanged)
										std::cout << "un";
									std::cout << "changed" << endl;
									std::cout << "  please go first to any folder "
													"running inside start/end routine" << endl;
									UNLOCK(m_DEBUGSESSIONCHANGES);
									pRv= &incommingTime;
									return pRv;
								}
								if(	make.direction == next &&
									show.action != changed &&
									show.action != unchanged	)
								{
									make.action= show.action;
									make.direction= show.direction;
									showContent= oIt;
									showSubroutine= getNextSubroutine(	make,
																		nFolderOutput,
																		showContent,
																		currFolderIt->second.end(),
																		dir,
																		endSubroutine,
																		subroutines					);
								}else // if(make.direction == next)
								{
									if(	show.action == ClientTransaction::folder ||
										(	show.action == folder_external &&
											(	dir.vector != ClientTransaction::folder ||
												!subroutineSet(oIt->second.external, subroutines)	) &&
											(	dir.vector != external ||
												dir.found == dir.first		)							)	)
									{
										--nFolderOutput;
										showContent= previousContent;
										make.action= IClientTransactionPattern::folder;

									}else
									{
										if(	show.action != none &&
											show.action != dir.vector &&
											(	show.action != folder_external ||
												dir.vector == inform				)	)
										{
											if(show.action < dir.vector)
											{
												make.action= show.action;
												if(show.action == inform)
												{
													dbSubroutineInfoType::reverse_iterator it;

													if(oIt->second.inform.empty())
													{
														nFolderOutput= nLastInformFolder;
														showContent= lastInformContent;
													}else
														showContent= oIt;
													if(showContent != currFolderIt->second.end())
													{
														it= showContent->second.inform.rbegin();
														++it;
														showSubroutine= it.base();
													}
													make.action= inform;

												}else if(show.action == external)
												{
													showContent= oIt;
													if(subroutineSet(oIt->second.external, subroutines))
													{
														vector<string>::iterator found;
														dbSubroutineInfoType::reverse_iterator it;

														make.action= external;
														for(it= oIt->second.external.rbegin();
																		it != oIt->second.external.rend(); ++it						)
														{
															found= find(subroutines.begin(), subroutines.end(),
																			it->second->subroutine);
															if(found != subroutines.end())
															{
																++it;
																showSubroutine= it.base();
																break;
															}
														}
													}else
													{
														dbSubroutineInfoType::reverse_iterator it;

														nFolderOutput= nLastExternalFolder;
														showContent= lastExternalContent;
														if(showContent != currFolderIt->second.end())
														{
															for(it= showContent->second.external.rbegin();
																			it != showContent->second.external.rend();
																			++it										)
															{
																if(subroutineSet(it->second->subroutine, subroutines))
																	break;
															}
															++it;
															showSubroutine= it.base();
														}
														make.action= external;
													}
												}
											}else
											{
												if(show.action == external)
												{
													dbSubroutineInfoType::reverse_iterator it;

													nFolderOutput= nLastExternalFolder;
													showContent= lastExternalContent;
													if(showContent != currFolderIt->second.end())
													{
														for(it= showContent->second.external.rbegin();
																		it != showContent->second.external.rend();
																		++it										)
														{
															if(subroutineSet(it->second->subroutine, subroutines))
																break;
														}
														++it;
														showSubroutine= it.base();
													}
													make.action= external;
												}
											}
										}else
										{
											if(dir.vector == ClientTransaction::folder)
											{
												showContent= oIt;
												if(subroutineSet(oIt->second.external, subroutines))
												{
													vector<string>::iterator found;
													dbSubroutineInfoType::reverse_iterator it;

													make.action= external;
													for(it= oIt->second.external.rbegin();
																	it != oIt->second.external.rend(); ++it						)
													{
														found= find(subroutines.begin(), subroutines.end(),
																		it->second->subroutine);
														if(found != subroutines.end())
														{
															++it;
															showSubroutine= it.base();
															break;
														}
													}
												}else if(!oIt->second.inform.empty())
												{
													dbSubroutineInfoType::reverse_iterator it;

													it= oIt->second.inform.rbegin();
													++it;
													showSubroutine= it.base();
													make.action= inform;
												}else
												{
													--nFolderOutput;
													showContent= previousContent;
													make.action= IClientTransactionPattern::folder;
												}
											}else
											{
												make.action= dir.vector;
												if(dir.first == dir.found)
												{
													if(dir.vector == external)
													{
														dbSubroutineInfoType::reverse_iterator it;

														if(show.action == external)
														{
															nFolderOutput= nLastExternalFolder;
															showContent= lastExternalContent;
															if(showContent != currFolderIt->second.end())
															{
																for(it= showContent->second.external.rbegin();
																				it != showContent->second.external.rend();
																				++it										)
																{
																	if(subroutineSet(it->second->subroutine, subroutines))
																		break;
																}
																++it;
																showSubroutine= it.base();
															}
															make.action= external;
														}else
														{
															dbSubroutineInfoType::reverse_iterator it;

															if(oIt->second.inform.empty())
															{
																nFolderOutput= nLastInformFolder;
																showContent= lastInformContent;
															}else
																showContent= oIt;
															if(showContent != currFolderIt->second.end())
															{
																it= showContent->second.inform.rbegin();
																++it;
																showSubroutine= it.base();
															}
															make.action= inform;
														}
													}else
													{
														if(	show.action == none)
														{
															--nFolderOutput;
															showContent= previousContent;
															make.action= IClientTransactionPattern::folder;

														}else if( show.action == inform)
														{
															dbSubroutineInfoType::reverse_iterator it;

															nFolderOutput= nLastInformFolder;
															showContent= lastInformContent;
															if(showContent != currFolderIt->second.end())
															{
																it= showContent->second.inform.rbegin();
																++it;
																showSubroutine= it.base();
															}
															make.action= inform;
															//make.direction= last;
														}
													}
												}else
													showSubroutine= dir.previous;
											}
										}
									}
								}
								if(dir.vector == IClientTransactionPattern::folder)
								{
									if(make.direction == previous)
									{
										if(make.action == changed)
										{
											make.action= dir.vector;
											nFolderOutput= nLastChangedFolder;
											showContent= lastChangedContent;
											break;

										}else if(make.action == unchanged)
										{
											make.action= dir.vector;
											nFolderOutput= nLastUnchangedFolder;
											showContent= lastUnchangedContent;
											break;

								//		}else if(make.action == ClientTransaction::folder)
										}/*else if(	show.action == ClientTransaction::folder ||
													(	show.action == folder_external &&
														(	dir.vector != ClientTransaction::folder ||
															!subroutineSet(oIt->second.external, subroutines)	) &&
														(	dir.vector != external ||
															dir.found == dir.first		)							)	)
										{
											//--nFolderOutput;
											showContent= previousContent;
										}*/
									}
								}

							}else // if(make.direction != current)
							{
								if(	dir.vector == inform ||
									dir.vector == external	)
								{
									if(dir.found == dir.last)
									{
										make.direction= first;
										make.action= external;
										showContent= oIt;
										dir.vector= IClientTransactionPattern::folder;
									}else
									{
										make.action= dir.vector;
										showSubroutine= dir.found;
									}
								}
								if(dir.vector == IClientTransactionPattern::folder)
									showContent= oIt;
							}// end else if(make.direction != current)
							if(	show.action == changed ||
								show.action == unchanged	)
							{
								make.action= IClientTransactionPattern::folder;
								if(show.direction == previous)
									break;
								bFoundCurrentFolder= true;
								showContent= oIt;
								for(dbSubroutineInfoType::iterator it= oIt->second.folder.begin();
												it != oIt->second.folder.end(); ++it				)
								{
									if(it->second->subroutine == subroutines[0])
									{
										showSubroutine= it;
										break;
									}
								}
							}else
								break;
						}
						previousContent= oIt;
						if(!oIt->second.inform.empty())
						{
							nLastInformFolder= nFolderOutput;
							lastInformContent= oIt;
						}
						if(	!oIt->second.external.empty() &&
							subroutineSet(oIt->second.external, subroutines)	)
						{
							nLastExternalFolder= nFolderOutput;
							lastExternalContent= oIt;
						}
						++oIt;
						if(	( 	make.direction == previous &&
								(	make.action == changed ||
									make.action == unchanged	)	) ||
							(	bFoundCurrentFolder &&
								oIt != currFolderIt->second.end()	)	)
						{
							long curVal, oldVal;
							dbSubroutineInfoType::iterator it;

							for(it= oIt->second.folder.begin();
									it != oIt->second.folder.end(); ++it	)
							{
								/*
								 * when begin of content form subroutine
								 * is "\\\\\\\\\\\\\\\\\\\\" subroutine
								 * is an external part of only changing
								 * value, this is not the correct subroutine content
								 * so this one is not the correct one
								 */
								if(	it->second->subroutine == subroutines[0] &&
									it->second->content.substr(0, 10) != "\\\\\\\\\\\\\\\\\\\\"	)
								{
									vector<string> spl;

									/*
									 * when third row of content begin with "was started external
									 * subroutine is an external running part of type READ
									 * so this one is not the correct one
									 * toDo: in next time should make better
									 *       when debug session from server
									 *       sending also type of subroutine
									 *       and more information whether subroutine
									 *       was running by external
									 */
									split(spl, it->second->content, is_any_of("\n"));
									if(	spl.size() < 3 ||
										spl[2].substr(0, 25) != "---  was started external"	)
									{
										break;
									}
								}
							}
							/*
							 * compare only with the first 4 precision
							 * after decimal point
							 * toDo: maybe it's not enough
							 *       by my test there was 6 precision OK
							 *       but I take 4, because that will be shown
							 *       inside timer subroutine as result
							 */
							curVal= static_cast<long>(it->second->value * 10000);
							if(showSubroutine != endSubroutine)
								oldVal= static_cast<long>(showSubroutine->second->value * 10000);
							else
								oldVal= curVal;
							if(oldVal != curVal)
							{
								showSubroutine= it;
								nLastChangedFolder= nFolderOutput;
								lastChangedContent= previousContent;
								if(	show.action == changed &&
									show.direction == next		)
								{
									++nFolderOutput;
									showContent= oIt;
									break;
								}
							}else
							{
								showSubroutine= it;
								nLastUnchangedFolder= nFolderOutput + 1;
								lastUnchangedContent= oIt;
								if(	show.direction == next &&
									show.action == unchanged	)
								{
									++nFolderOutput;
									showContent= oIt;
									break;
								}
							}
						}
					}// while(oIt != currFolderIt->second.end())
					if(	(	showContent == currFolderIt->second.end() &&
							showSubroutine == endSubroutine				) ||
						(	show.direction == next &&
							oIt == currFolderIt->second.end() &&
							(	show.action == changed ||
								show.action == unchanged	)		) ||
						(	show.direction == previous &&
							showContent == currFolderIt->second.begin() &&
							(	show.action == changed ||
								show.action == unchanged	)				)	)
					{
						string msg;

						if(	make.direction == next ||
							make.direction == first		)
						{
							msg=  " reaching end of folders\n";
							msg+= " show folder '";
							msg+= folder;
							msg+= "' again from begin?\n";
							msg= ask(/*YesNo*/true, msg);
							if(msg == "Y")
							{
								nFolderOutput= 0;
								make.direction= first;
								make.action= show.action;
								showContent= currFolderIt->second.end();
								showSubroutine= endSubroutine;

							}else if(	show.action != changed &&
										show.action != unchanged	)
							{
								UNLOCK(m_DEBUGSESSIONCHANGES);
								pRv= &incommingTime;
								return pRv;
							}
						}else if(make.direction == previous)
						{
							msg=  " reaching begin of folders\n";
							msg+= " show folder '";
							msg+= folder;
							msg+= "' again from end?\n";
							msg= ask(/*YesNo*/true, msg);
							if(msg == "Y")
							{
								make.direction= last;
								make.action= show.action;
								break;

							}else if(	show.action != changed &&
										show.action != unchanged	)
							{
								UNLOCK(m_DEBUGSESSIONCHANGES);
								pRv= &incommingTime;
								return pRv;
							}
						}else if(make.direction == current)
						{
							/*
							 * last time of subroutine
							 * was overrun
							 * so show now the last one
							 */
							make.direction= last;
							break;
						}else
						{
							/*
							 * unknown direction
							 * was set
							 */
							std::cout << "  reach begin/end of folders (unknown direction)" << endl;
							UNLOCK(m_DEBUGSESSIONCHANGES);
							pRv= &incommingTime;
							return pRv;
						}
					}
				}while(	showContent == currFolderIt->second.end() &&
						showSubroutine == endSubroutine					);
				if(make.direction == last)
					continue;
			} // end else of if(make.direction == last)

			existFolderObjs= currFolderIt->second.size();
			if(currFolderIt->second.rbegin()->second.folder.empty())
				--existFolderObjs;
			/*
			 * try to display first
			 * from inform vector
			 */
			if(	make.action <= inform &&
				(	show.action == none ||
					show.action == inform	) &&
				showContent != currFolderIt->second.end() &&
				!showContent->second.inform.empty()			)
			{
				nOutput= 0;
				for(innerIt= showContent->second.inform.begin();
								innerIt != showContent->second.inform.end(); ++innerIt	)
				{
					++nOutput;
					if(	!curTime->isSet() ||
						make.direction == first ||
						make.direction == last ||
						innerIt == showSubroutine	)
					{
						content.str("");
						content << nOutput << ". try from ";
						content << showContent->second.inform.size();
						content << " to start folder ";
						if(nFolderOutput > existFolderObjs)
							content << "after last running" << endl;
						else
							content << "of " << nFolderOutput << ". run" << endl;
						content << innerIt->second->content << endl;
#if(__SHOWORDEREDTIMES)
						content << "^^^ ";
						content << innerIt->second->subroutine << " at "
										<< innerIt->second->currentTime->toString(/*as date*/true)
										<< " ^^^" << endl;
#endif // #if(__SHOWORDEREDTIMES)
						pRv= &(*innerIt->second->currentTime);
						bDisplayedContent= true;
						if(make.direction != last)
						{
							std::cout << glob::addPrefix(getFolderID(folder), content.str());
							break;
						}
					}
				}
				if(	make.direction == last &&
					bDisplayedContent			)
				{
					std::cout << glob::addPrefix(getFolderID(folder), content.str());
				}
			}
			/*
			 * than try to display
			 * from external vector
			 */
			if(	!bDisplayedContent &&
				(	make.action <= external ||
					make.action == folder_external	) &&
				(	show.action == none ||
					show.action == external ||
					show.action == folder_external	) &&
				showContent != currFolderIt->second.end() &&
				!showContent->second.external.empty()		)
			{
				size_t count(0);
				string sSubroutine;

				nOutput= 0;
				/*
				 * there is no break inside while routine
				 * because at all count for existing
				 * correct external subroutines
				 */
				innerIt= showContent->second.external.begin();
				while(innerIt != showContent->second.external.end())
				{
					if(subroutineSet(innerIt->second->subroutine, subroutines))
					{
						++count;
						if(	make.direction == last ||
							(	(	!curTime->isSet() ||
									make.direction == first	||
									innerIt == showSubroutine	) &&
								sSubroutine.empty()						)	)
						{
							nOutput= count;
							sSubroutine= innerIt->second->content;
							trim(sSubroutine);
							sSubroutine+= "\n";
#if(__SHOWORDEREDTIMES)
							sSubroutine+= "^^^ ";
							if(innerIt->second->subroutine.substr(0, 1) != "#")
								sSubroutine+= "subroutine ";
							sSubroutine+= innerIt->second->subroutine + " at "
											+ innerIt->second->currentTime->toString(/*as date*/true)
											+ " ^^^\n";
#endif // #if(__SHOWORDEREDTIMES)
							pRv= &(*innerIt->second->currentTime);
							bDisplayedContent= true;
						}
					}
					++innerIt;
				}
				if(!sSubroutine.empty())
				{
					content << nOutput << ". of ";
					content << count;
					content << " external subroutine running ";
					if(nFolderOutput <=  existFolderObjs)
					{
						content << "before start folder of ";
						content << nFolderOutput << ". run" << endl;
					}else
						content << "after last folder running" << endl;
					content << sSubroutine << endl;
					std::cout << glob::addPrefix(getFolderID(folder), content.str());
				}
			}
			/*
			 * and at last
			 * try to display
			 * subroutines between start and end
			 */
			if(	!bDisplayedContent &&
				(	make.action <= IClientTransactionPattern::folder ||
					make.action == folder_external						) &&
					(	show.action == none ||
						show.action >= ClientTransaction::folder ) &&
				showContent != currFolderIt->second.end() &&
				!showContent->second.folder.empty()							)
			{
				content << "start " << nFolderOutput << ". folder of ";
				content << existFolderObjs << endl;
				for(innerIt= showContent->second.folder.begin();
								innerIt != showContent->second.folder.end(); ++innerIt	)
				{
					if(	innerIt->second->subroutine == "#inform" ||
						innerIt->second->subroutine == "#start" ||
						innerIt->second->subroutine	== "#started" ||
						innerIt->second->subroutine == "#end" ||
						subroutineSet(innerIt->second->subroutine, subroutines)	)
					{
						string sContent(innerIt->second->content);

						bDisplayedContent= true;
						trim(sContent);
						content << sContent << endl;
#if(__SHOWORDEREDTIMES)
						content << "^^^ ";
						if(innerIt->second->subroutine.substr(0, 1) != "#")
							content << "subroutine ";
						content << innerIt->second->subroutine << " at "
										<< innerIt->second->currentTime->toString(/*as date*/true)
										<< " with value " << innerIt->second->value
										<< " ^^^" << endl;
#endif // #if(__SHOWORDEREDTIMES)
						std::cout << glob::addPrefix(getFolderID(folder), content.str());
						if(	(	subroutines.empty() &&
								innerIt->second->subroutine == "#start"	) ||
							(	!subroutines.empty() &&
								innerIt->second->subroutine == subroutines[0]	)	)
						{
							pRv= &(*innerIt->second->currentTime);
						}
						content.str("");
					}
				}
			}
			if(!bDisplayedContent)
			{
				string msg;

				if(	make.direction == next ||
					make.direction == first	)
				{
					/*
					 * when direction is first
					 * is meaning make from next
					 * action external/folder or inform
					 * the first entry
					 * so direction meaning is also next direction
					 */
					msg=  " reaching end of folders\n";
					msg+= " show folder '";
					msg+= folder;
					msg+= "' again from begin?\n";
					msg= ask(/*YesNo*/true, msg);
					if(msg == "Y")
					{
						make.direction= first;
						make.action= show.action;
					}else
						break;
				}else if(	make.direction == previous ||
							make.direction == last		)
				{
					/*
					 * when direction is last
					 * is meaning make from previous
					 * action external/inform or folder
					 * the last entry
					 * so direction meaning is also previous direction
					 */
					msg=  " reaching begin of folders\n";
					msg+= " show folder '";
					msg+= folder;
					msg+= "' again from end?\n";
					msg= ask(/*YesNo*/true, msg);
					if(msg == "Y")
					{
						make.direction= last;
						make.action= show.action;
					}else
						break;
				}else if( make.direction == current)
				{
					/*
					 * last subroutine
					 * had no content
					 * so show now the last one
					 * with content
					 */
					make.direction= last;
				}else
				{
					/*
					 * unknown direction
					 * was set
					 */
					std::cout << "  reach begin/end of folders (unknown direction)" << endl;
					break;
				}
			}
		}while(!bDisplayedContent);
		UNLOCK(m_DEBUGSESSIONCHANGES);
		return pRv;
	}

	ClientTransaction::dbSubroutineInfoType::iterator
		ClientTransaction::getNextSubroutine(	direction_t& show,
												size_t& nCurFolder,
												folderSessionIterator& curContent,
												const folderSessionIterator& endContent,
												const found_subroutine_t& foundSubroutine,
												const dbSubroutineInfoType::iterator endSubroutine,
												const vector<string>& subroutines					)
	{
		bool bFound;
		dbSubroutineInfoType::iterator iRv= endSubroutine;

		if(show.action == none)
		{
			if(foundSubroutine.vector == ClientTransaction::folder)
			{
				++nCurFolder;
				++curContent;
				show.action= inform;
				show.direction= first;
				return iRv;
			}
			show.action= foundSubroutine.vector;
			iRv= foundSubroutine.found;
			++iRv;
			if(iRv != foundSubroutine.last)
				return iRv;
			/*
			 * make next action
			 */
			if(	show.action == inform &&
				subroutineSet(curContent->second.external, subroutines)	)
			{
				show.action= external;
				iRv= curContent->second.external.begin();
				return iRv;
			}
			/*
			 * show.action should be external
			 * where next is ClientTransaction::folder
			 * or ahow.action is inform
			 * and external vector has no subroutines
			 * to display
			 */
			show.action= ClientTransaction::folder;
			iRv= endSubroutine;
			return iRv;
		}// if(show.action == none)
		bFound= false;
		if(	(	foundSubroutine.vector == show.action ||
				(	show.action == folder_external &&
					(	foundSubroutine.vector == external ||
						foundSubroutine.vector == ClientTransaction::folder	)	)	) &&
			foundSubroutine.found != foundSubroutine.last								)
		{
			if(foundSubroutine.vector != ClientTransaction::folder)
			{
				iRv= foundSubroutine.found;
				++iRv;
				if(iRv != foundSubroutine.last)
					bFound= true;
			}
			if(	bFound == false &&
				(	foundSubroutine.vector == ClientTransaction::folder ||
					(	foundSubroutine.vector == external &&
						show.action == folder_external			)			)	)
			{
				if(foundSubroutine.vector != external)
				{
					++nCurFolder;
					++curContent;
				}
				if(show.action == folder_external)
				{
					show.action= external;
					if(foundSubroutine.vector == ClientTransaction::folder)
						show.direction= first;
				}else
					show.action= ClientTransaction::folder;
				bFound= true;
			}
		}
		if(!bFound)
		{
			bool bFirst(true);

			do{
				bFound= false;
				if(curContent != endContent)
				{
					switch(show.action)
					{
					case inform:
						if(!curContent->second.inform.empty())
							bFound= true;
						break;
					case external:
						if(subroutineSet(curContent->second.external, subroutines))
							bFound= true;
						break;
					case ClientTransaction::folder:
						bFound= true;
						break;
					case folder_external:
						show.action= external;
						if(subroutineSet(curContent->second.external, subroutines))
							bFound= true;
						else if(!curContent->second.folder.empty())
							bFound= true;
						break;
					default:
						bFound= false;
						break;
					}
					if(bFirst)
					{
						if(	foundSubroutine.vector >= show.action ||
							(	show.action == folder_external &&
								foundSubroutine.vector == ClientTransaction::folder	)	)
						{
							bFound= false;
						}
						bFirst= false;
					}
					if(!bFound)
					{
						++nCurFolder;
						++curContent;
					}
				}

			}while(	curContent != endContent &&
					bFound == false				);
			if(curContent != endContent)
			{
				switch(show.action)
				{
				case inform:
					iRv= curContent->second.inform.begin();
					break;
				case external:
					for(dbSubroutineInfoType::iterator it= curContent->second.external.begin();
									it != curContent->second.external.end(); ++it				)
					{
						if(subroutineSet(it->second->subroutine, subroutines))
						{
							iRv= it;
							break;
						}
					}
					break;
				case ClientTransaction::folder:
					iRv= endSubroutine;
					break;
				default:
					iRv= endSubroutine;
					break;
				}
			}
		}
		return iRv;
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

	void ClientTransaction::setPromptString(const string& str)
	{
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
	}

	void ClientTransaction::prompt(const string& str/*= ""*/)
	{
		if(m_bScriptState)
			return;
		if(m_o2Client.get())
		{
			/**
			 * set prompt string
			 * also inside user thread,
			 * because when hearing thread
			 * stopping, user thread
			 * should also know from current prompt
			 */
			setPromptString(str);
			m_o2Client->transObj()->prompt(str);
			return;
		}
		LOCK(m_PROMPTMUTEX);
		setPromptString(str);
		if(	!m_bRunHearTran &&
			!m_bRunUserTrans	)
		{
			if(m_sPrompt != "")
				std::cout << m_sPrompt;
			writeLastPromptLine(/*lock*/false);
		}
		UNLOCK(m_PROMPTMUTEX);
	}

	bool ClientTransaction::compareUserPassword(IFileDescriptorPattern& descriptor, string& user, string& pwd/*= ""*/, bool* pbHear/*= NULL*/)
	{
		bool bHear;
		int c;
		struct termios term;
		string sSendbuf, result;

		if(pbHear == NULL)
		{
			bHear= m_bHearing;
			pbHear= &bHear;
		}else
			bHear= *pbHear;
		if(m_o2Client.get())
			return m_o2Client->transObj()->compareUserPassword(descriptor, user, pwd, pbHear);
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
		if(!m_bScriptState &&
			!bHear			)
		{
			std::cout << endl;
			resetTc();
		}

		LOCK(m_PASSWORDCHECK);
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
		m_pError->setErrorStr(result);
		if(	m_pError->fail() ||
			result.substr(0, 6) == "ERROR " ||
			result.substr(0, 8) == "WARNING "	)
		{
			string prefix;
			string errmsg;

			m_pError->addMessage("ClientTransaction", "get_result");
			m_bErrWritten= true;
			if(	m_pError->hasError() ||
				result.substr(0, 6) == "ERROR "	)
			{
				prefix= "ERROR: ";
			}else
				prefix= "WARNING: ";
			if(m_pError->fail())
				errmsg= m_pError->getDescription();
			else
				errmsg= getError(descriptor, result);
			cerr << glob::addPrefix(prefix, errmsg) << endl;
			if(	m_pError->hasError() ||
				result.substr(0, 6) == "ERROR "	)
			{
				if(!m_pError->hasError())
					m_pError->setError("ClientTransaction", "user_password_error");
				UNLOCK(m_PASSWORDCHECK);
				return false;
			}
		}
		m_bConnected= true;
		m_sPassword= pwd;
		UNLOCK(m_PASSWORDCHECK);
		return true;
	}

	string ClientTransaction::getCurrentPassword()
	{
		string pwd;

		LOCK(m_PASSWORDCHECK);
		pwd= m_sPassword;
		UNLOCK(m_PASSWORDCHECK);
		return pwd;
	}

	void ClientTransaction::load(bool get)
	{
		ppi_time current, next;

		next.tv_sec= 0;
		next.tv_usec= 500000;
		LOCK(m_LOADMUTEX);
		if(!get)
		{
			unsigned short before(m_sServerLoadStep);

			m_bServerLoad= false;
			m_tLastLoad.clear();
			UNLOCK(m_LOADMUTEX);
			if(before > 0)
				writeLastPromptLine(/*need lock*/true);
			return;
		}
		if(current.setActTime())
		{
			if(	m_tLastLoad.isSet() &&
				next > (current - m_tLastLoad)	)
			{
				UNLOCK(m_LOADMUTEX);
				return;
			}
			m_tLastLoad= current;
		}
		m_bServerLoad= true;
		++m_sServerLoadStep;
		if(m_sServerLoadStep > 4)
			m_sServerLoadStep= 1;
		UNLOCK(m_LOADMUTEX);
		writeLastPromptLine(/*need lock*/true);
	}

	void ClientTransaction::writeLastPromptLine(bool lock,
					string::size_type cursor/*= string::npos*/, const string& str/*= ""*/, bool end/*= false*/)
	{
		string sNullStr;
		string sload;

		if(m_o2Client.get())
		{
			m_o2Client->transObj()->writeLastPromptLine(lock, cursor, str, end);
			return;
		}
		if(lock)
			LOCK(m_PROMPTMUTEX);

		LOCK(m_LOADMUTEX);
		if(m_bServerLoad)
		{
			switch(m_sServerLoadStep)
			{
			case 1:
				sload= "- ";
				break;
			case 2:
				sload= "\\ ";
				break;
			case 3:
				sload= "| ";
				break;
			case 4:
				sload= "/ ";
				break;
			default:
				sload= "  ";
				break;
			}
		}else
			sload= "  ";
		UNLOCK(m_LOADMUTEX);
		if(	cursor != string::npos &&
			!m_bRunHearTran	)
		{
			sNullStr.append(m_nOldResultLength, ' ');
			std::cout << "\r" << sload << m_sLastPromptLine << sNullStr << std::flush;
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
			ostringstream out;

			out << "\r";
			out << sload << m_sLastPromptLine << m_sPromptResult;
			std::cout << out.str() << flush;
			if(end)
				std::cout << endl;
			else if(m_sPromptResult.length() > m_nResultPos)
			{
				out.str("");
				out << "\r" << sload << m_sLastPromptLine;
				out << m_sPromptResult.substr(0, m_nResultPos);
				std::cout << out.str() << flush;
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
		if(!m_bWait)
		{
			std::cout << str;
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
		if(	!descriptor.fail() &&
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
		DESTROYMUTEX(m_PASSWORDCHECK);
		DESTROYMUTEX(m_DEBUGSESSIONCHANGES);
		DESTROYMUTEX(m_PROMPTMUTEX);
		DESTROYMUTEX(m_LOADMUTEX);
	}

}
