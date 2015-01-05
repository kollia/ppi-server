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
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <termios.h>

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../../util/XMLStartEndTagReader.h"
#include "../../util/GlobalStaticMethods.h"
#include "../../util/structures.h"

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
	cout << "send: " << sSendbuf;
#endif // SERVERDEBUG
		descriptor << sSendbuf;
		descriptor.flush();

		descriptor >> result;
		result= ConfigPropertyCasher::trim(result);
#ifdef SERVERDEBUG
	cout << "get: " << result << endl;
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
			}else
				bRightServer= false;
		}
		if(	!bRightServer
			&&
			result == ""	)
		{
			cout << "ERROR: get no result from server\ntry again later" << endl;
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
				cout << glob::addPrefix("WARNING: ", errHandle->getDescription()) << endl;
			m_bErrWritten= true;
			return errHandle;
		}

		if(bStatus)
		{
			char stime[20];
			time_t t;

			t= time(NULL);
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", localtime(&t));
			cout << endl << "actual time " << stime << endl;
			bLogin= false;
		}
		if(	m_bWait
			||
			bLogin		)
		{
			int c;
			struct termios term, backup;
			bool readtc= true;

			if((tcgetattr(STDIN_FILENO, &term)) < 0)
			{
				readtc= false;
				if(errno != 25)
				{
					errHandle->setErrnoError("ClientTransaction", "tcgetattr", errno);
					cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
					m_bErrWritten= true;
					return errHandle;
				}
			}
			backup= term;
			if(user == "")
			{
				if(readtc)
					cout << "user: ";
				getline(std::cin, user);
				user= ConfigPropertyCasher::trim(user);
			}
			if(pwd == "")
			{
				if(readtc)
				{
					term.c_lflag= term.c_lflag & ~ECHO;
					if((tcsetattr(STDIN_FILENO, TCSAFLUSH, &term)) < 0)
					{
						errHandle->setErrnoError("ClientTransaction", "tcsetattr", errno);
						cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
						m_bErrWritten= true;
						return errHandle;
					}
					cout << "password:" << flush;
				}

				do{
					c= getc(stdin);
					pwd+= c;
				}while(c != '\n');
				pwd= ConfigPropertyCasher::trim(pwd);
				if(readtc)
					cout << endl;
				if(readtc && (tcsetattr(STDIN_FILENO, TCSAFLUSH, &backup)) < 0)
				{
					errHandle->setErrnoError("ClientTransaction", "tcsetattr_back", errno);
					cerr << glob::addPrefix("ERROR: ", errHandle->getDescription()) << endl;
					m_bErrWritten= true;
					return errHandle;
				}
			}
			sSendbuf= "U:";
			sSendbuf+= user;
			sSendbuf+= ":";
			sSendbuf+= pwd;
#ifdef SERVERDEBUG
			cout << "send: '" << sSendbuf << "'" << endl;
#endif // SERVERDEBUG
			sSendbuf+= "\n";
			descriptor << sSendbuf;
			descriptor.flush();
			descriptor >> result;
			result= ConfigPropertyCasher::trim(result);
			errHandle->setErrorStr(result);
			if(errHandle->fail())
			{
				string prefix;

				errHandle->addMessage("ClientTransaction", "get_result");
				m_bErrWritten= true;
				if(errHandle->hasError())
					prefix= "ERROR: ";
				else
					prefix= "WARNING: ";
				cerr << glob::addPrefix(prefix, errHandle->getDescription()) << endl;
				if(errHandle->hasError())
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
		return errHandle;
	}

	string ClientTransaction::getFolderID(const string& folder)
	{
		string sRv;
		map<string, string>::iterator found;

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
						 */
						input >> folder;
						input >> subroutine;
						input >> value;
						input >> time;
						input >> content;
						LOCK(m_DEBUGSESSIONCHANGES);
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
								cout << endl;
							}
							cout << glob::addPrefix(id, content);
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
					cout << "XX:XX:xx (cannot calculate time)" << endl;
				}else
				{
					if(localtime_r(&time.tv_sec, &ttime) != NULL)
					{
						strftime(stime, 21, "%H:%M:%S", &ttime);
						cout << stime << endl;
					}else
						cout << "XX:XX:xx (cannot calculate time)" << endl;
				}
				cout << "Nr.| CALL | last call        |    value     |  every   | need time | length time "
					"| act. | chip ID / pin" << endl;
				cout << "---------------------------------------------------------------------------------"
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

						//cout << endl << result;
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

						cout.fill('0');
						cout.width(3);
						cout << dec << tdebug.id << " ";
						cout.fill(' ');
						cout.width(5);
						cout << tdebug.count << "   ";
						if(tdebug.btime)
						{
							long msec, usec;

							strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&tdebug.act_tm.tv_sec));
							cout << buf << " ";
							msec= tdebug.act_tm.tv_usec / 1000;
							usec= tdebug.act_tm.tv_usec - (msec * 1000);
							cout.fill('0');
							cout.width(3);
							cout << dec << msec << " ";
							cout.width(3);
							cout << dec << usec << "   ";

						}else
							cout << "-------- --- ---   ";
						if(tdebug.read)
							sDo= "read ";
						else
							sDo= "write";
						cout.setf(ios_base::fixed);
						cout.fill(' ');
						if(tdebug.ok)
						{
							cout.precision(6);
							cout.width(12);
							cout << dec << tdebug.value << "   ";
						}else
							cout << "cannot " << sDo << "   ";
						if(tdebug.read)
						{
							cout.precision(4);
							cout.width(8);
							cout << dec << tdebug.cache << "   ";
						}else
						{
							cout.width(4);
							cout << dec << tdebug.priority << "       ";
						}
						if(tdebug.btime)
						{
							cout.fill('0');
							cout.width(2);
							cout << dec << nsec << " ";
							cout.width(3);
							cout << dec << nmsec << " ";
							cout.width(3);
							cout << dec << nusec << "   ";

						}else
							cout << "-- --- ---   ";
						// measure max time
						nsec= m_mOwMaxTime[tdebug.id] / 1000000;
						nmsec= m_mOwMaxTime[tdebug.id] / 1000 - (nsec * 1000);
						nusec= m_mOwMaxTime[tdebug.id] - (nmsec * 1000) - (nsec * 1000000);
						cout.fill('0');
						cout.width(2);
						cout << dec << nsec << " ";
						cout.width(3);
						cout << dec << nmsec << " ";
						cout.width(3);
						cout << dec << nusec << "   ";

						cout << sDo << " '" << tdebug.device << "'" << endl;
					}else
					{
						cout << endl;
						bHeader= true;
						prompt();
					}
				}else
					cout << result << endl;
			}
#ifdef SERVERDEBUG
			else
				cout << "### server has stop hearing thread" << endl;
#endif // SERVERDEBUG
		}while(	!descriptor.eof()
				&&
				result != "stopclient"	);
		if(descriptor.eof())
			cout << "### by heareing on server for OWServer debug messages -> connection is broken" << endl;
		return false;
	}

	string ClientTransaction::ask(bool yesno, string promptStr)
	{
		string result;
	//	char character;
		char str[200];
		//int charNr;

	/*	do{
			//cin.get(character);
			//charNr= getchar_unlocked();
			//ungetc(charNr, stdin);
			fgets(charNr, 199, stdin);
			if(*charNr != '\n')
			{
				result+= *charNr;
				cout << "type " << charNr << endl;
				cout << "$> " << result << flush;
			}
		}while(*charNr != '\n');*/
		if(yesno)
			promptStr+= " (Y/N) [n] ";
		do{
			if(m_o2Client.get())
				m_o2Client->transObj()->prompt(promptStr);
			else
				prompt(promptStr);
			*str= '\0';
			std::cin.clear();
			while(!(std::cin.getline(str, 199, '\n')))
			{
				std::cin.clear();
				//std::cin.ignore(std::numeric_limits<streamsize>::max(),'\n');
				usleep(500000);
				cout << "(" << str << "[ " << (int)*str <<"])." << flush;
				continue;
			}
			result= str;
			trim(result);
			if(yesno)
			{
				if(result == "")
					result= "N";
			}else
				break;
			if(	result != "Y" &&
				result != "y" &&
				result != "N" &&
				result != "n"	)
			{
				cout << " please type only Y or N" << endl;
			}
		}while(	result != "Y" &&
				result != "y" &&
				result != "N" &&
				result != "n"	);
		if(yesno)
		{
			if(result == "n")
				result= "N";
			else if(result == "y")
				result= "Y";
		}
		return result;
	}

	bool ClientTransaction::userTransfer(IFileDescriptorPattern& descriptor)
	{
		bool bWaitEnd= false;
		bool bHoldDebug(false);
		string logMsg, result, command;
		string sSendbuf, word;
		string folder, subroutine;
		auto_ptr<XMLStartEndTagReader> xmlReader;
		vector<pair<string, pair<ppi_time, vector<string> > > > vNextSubs;
		vector<pair<string, vector<string> > >::size_type nCurLayer(0);
		vector<string> vHistory;
		ppi_time currentTime;

		if(	m_bWait ||
			m_bHearing	)
		{
			cout << endl;
			cout << "stop connection to server by typing 'exit' or 'quit'" << endl;
			cout << "for help type '?', '?value' or '?debug'" << endl;
			cout << "--------------------------------------------------------" << endl;
		}
		command= m_sCommand;
		do{
			istringstream iresult;

			bHoldDebug= false;
			if(command == "")
			{
				if(m_o2Client.get())
					m_o2Client->transObj()->runUserTransaction(false);
				command= ask(/*YesNo*/false, "$> ");
				trim(command);
				if(command == "")
					continue;
			}
			if(m_o2Client.get())
				m_o2Client->transObj()->runUserTransaction(true);
			if(command.substr(0, 1) == "!")
			{
				unsigned long nr;
				istringstream res(command.substr(1));

				res >> nr;
				if(	res.fail() ||
					nr < 1000 ||
					nr > (vHistory.size() + 1000)	)
				{
					cout << " no correct number given for history command" << endl;
					command= "";
					continue;
				}
				nr-= 1000;
				command= vHistory[nr];

			}else if(	command == "quit" ||
						command == "exit"	)
			{
				command= "ending\n";
				descriptor << command;
				descriptor.endl();
				descriptor.flush();
				if(m_o2Client.get())
					m_o2Client->stop(/*wait*/true);
				return false;

			}else if(command.substr(0, 1) == "?")
			{
				writeHelpUsage(command);
				vHistory.push_back(command);
				command= "";
				continue;

			}else if(	command == "run" ||
						command == "RUNDEBUG"	)
			{
				map<string, unsigned long> folderCount;

				LOCK(m_DEBUGSESSIONCHANGES);
				folderCount= m_o2Client->transObj()->getRunningFolderList();
				UNLOCK(m_DEBUGSESSIONCHANGES);

				if(!folderCount.empty())
				{
					for(map<string, unsigned long>::iterator it= folderCount.begin();
									it != folderCount.end(); ++it			)
					{
						cout << "   " << it->second << "  " << it->first << endl;
					}
				}else
					cout << " no folders running since starting or last CLEARDEBUG" << endl;

				vHistory.push_back(command);
				command= "";
				continue;
			}

			if(	command.substr(0, 3) == "DIR" ||
				command.substr(0, 6) == "status"	)
			{
				bWaitEnd= true;
			}else if(command.substr(0, 7) == "CONTENT")
				xmlReader= auto_ptr<XMLStartEndTagReader>(new XMLStartEndTagReader());

			if(command.substr(0, 15) == "GETERRORSTRING ")
			{
				string com;
				int number;
				istringstream icommand(command);
				ostringstream ocommand;

				icommand >> com;
				icommand >> number;
				if(number > 0)
					number-= m_nOutsideErr;
				else
					number+= m_nOutsideWarn;
				ocommand << com << " " << number;
				command= ocommand.str();
			}
			iresult.str(command);
			iresult >> word;
			if(	word == "hold" ||
				word == "HOLDDEBUG"	)
			{
				if(m_o2Client.get() == NULL)
				{
					cout << "ppi-client not defined with second connection "
									"to server (start client with option --hear)" << endl;
					command= "";
					continue;
				}
				bHoldDebug= true;
				if(word == "HOLDDEBUG")
					command= command.substr(4);
				else
					command= "DEBUG" + command.substr(4);

			}else if(word == "stop")
			{
				command= "STOPDEBUG" + command.substr(4);

			}else if(	word == "add" ||
						word == "ADDDEBUG"	)
			{
				string secWord;

				iresult >> secWord;
				trim(secWord);
				if(	secWord == "" ||
					vNextSubs.empty() ||
					vNextSubs[nCurLayer].first == ""	)
				{
					cout << " no folder:subroutine defined" << endl;
					cout << " define first with $> CURDEBUG <folder>[:subroutine]" << endl;
					command= "";
					continue;
				}
				if(!existOnServer(descriptor, vNextSubs[nCurLayer].first, secWord))
				{
					cout << " subroutine '" << vNextSubs[nCurLayer].first << ":" << secWord;
					cout << "' do not exist inside ppi-server working list" << endl;
					command= "";
					continue;
				}
				vector<string>::iterator found;

				found= find(vNextSubs[nCurLayer].second.second.begin(),
								vNextSubs[nCurLayer].second.second.end(), secWord);
				if(found == vNextSubs[nCurLayer].second.second.end())
					vNextSubs[nCurLayer].second.second.push_back(secWord);
				else
				{
					cout << " subroutine '" << secWord << "' was added before" << endl;
					continue;
				}
				word= "CURDEBUG";

			}else if(	word == "rm" ||
						word == "remove" ||
						word == "REMOVEDEBUG"	)
			{
				string secWord;
				vector<string>::iterator it;

				iresult >> secWord;
				trim(secWord);
				it= find(vNextSubs[nCurLayer].second.second.begin(),
								vNextSubs[nCurLayer].second.second.end(), secWord);
				if(it == vNextSubs[nCurLayer].second.second.end())
				{
					cout << " current subroutines in folder ";
					cout << vNextSubs[nCurLayer].first << " are:" << endl;
					for(it= vNextSubs[nCurLayer].second.second.begin();
									it != vNextSubs[nCurLayer].second.second.end(); ++it)
					{
						cout << "                                ";
						cout << *it << endl;
					}
					cout << " cannot find given subroutine '" << secWord;
					cout << "' inside queue" << endl;
					command= "";
					continue;
				}
				vHistory.push_back(command);
				if(vNextSubs[nCurLayer].second.second.size() == 1)
				{
					string msg;

					msg=  " only this one subroutine is inside queue\n";
					msg+= " do you want remove all current debugging?\n";
					m_o2Client->transObj()->runUserTransaction(false);
					msg= ask(/*YesNo*/true, msg);
					m_o2Client->transObj()->runUserTransaction(true);
					if(msg == "N")
					{
						command= "";
						continue;
					}
				}
				vNextSubs[nCurLayer].second.second.erase(it);
				command= "";
				continue;
			}
			if(	word == "show" ||
				word == "SHOWDEBUG" ||
				word == "cur" ||
				word == "current" ||
				word == "CURDEBUG" ||
				word == "next" ||
				word == "NEXTDEBUG" ||
				word == "prev" ||
				word == "previous" ||
				word == "PREVDEBUG" ||
				word == "first" ||
				word == "FIRSTDEBUG" ||
				word == "last" ||
				word == "LASTDEBUG"	||
				word == "back" ||
				word == "BACKDEBUG"		)
			{
				short nExist(-2);
				string secWord;
				direction_e direction;
				unsigned long folderNr(0);

				if(	word == "show" ||
					word == "SHOWDEBUG"	)
				{
					direction= all;

				}else if(	word == "first" ||
							word == "FIRSTDEBUG"	)
				{
					direction= first;

				}else if(	word == "cur" ||
							word == "current" ||
							word == "CURDEBUG" ||
							word == "back" ||
							word == "BACKDEBUG" 	)
				{
					if(	word == "back" ||
						word == "BACKDEBUG"	)
					{
						if(nCurLayer == 0)
						{
							cout << " no lower layer from an CURDEBUG before exist" << endl;
							continue;
						}
						--nCurLayer;
					}
					direction= current;

				}else if(	word == "next" ||
							word == "NEXTDEBUG"	)
				{
					direction= next;

				}else if(	word == "prev" ||
							word == "previous" ||
							word == "PREVDEBUG"		)
				{
					direction= previous;

				}else
					direction= last;
				folder= "";
				subroutine= "";
				if(!iresult.eof())
				{
					if(	direction == next ||
						direction == previous	)
					{
						iresult >> folderNr;
						if(iresult.fail())
						{
							iresult.clear();
							secWord= "";
							iresult >> secWord;
							if(	secWord == "ch" ||
								secWord == "changed"	)
							{
								if(direction == next)
									direction= next_changed;
								else // direction should be previous
									direction= previous_changed;

							}else if(	secWord == "uch" ||
										secWord == "unchanged"	)
							{
								if(direction == next)
									direction= next_unchanged;
								else // direction should be previous
									direction= previous_unchanged;

							}else
							{
								cout << " no correct command or folder number ";
								cout << " after '" << word << "' defined" << endl;
								command= "";
								continue;
							}
							if(secWord != "")
							{
								bool bFoundSubroutine(false);
								vector<string>::iterator subIt;
								/*
								 * secWord is defined for direction
								 * with changed or unchanged
								 * search now for subroutine
								 * which should be changed/unchanged
								 */
								secWord= "";
								iresult >> secWord;
								if(vNextSubs[nCurLayer].second.second.empty())
								{
									if(secWord != "")
									{
										/*
										 * hole folder should be shown,
										 * define secWord with folder name
										 * for later existing check
										 * and in subroutines queue define
										 * an '#' before
										 * to know later inside method writeDebugSession()
										 * that all subroutines should be shown
										 * but check for changed/unchanged
										 * this subroutine with '#'
										 */
										vNextSubs[nCurLayer].second.second.push_back("#" + secWord);
										secWord= vNextSubs[nCurLayer].first
														+ ":" +secWord;
										bFoundSubroutine= true;
									}
								}else if(vNextSubs[nCurLayer].second.second.size() == 1)
								{
									if(	secWord == "" ||
										secWord == vNextSubs[nCurLayer].second.second[0]	)
									{
										bFoundSubroutine= true;
									}
								}else
								{
									subIt= find(vNextSubs[nCurLayer].second.second.begin(),
													vNextSubs[nCurLayer].second.second.end(), secWord);
									if(subIt != vNextSubs[nCurLayer].second.second.end())
									{
										bFoundSubroutine= true;
										if(secWord != vNextSubs[nCurLayer].second.second[0])
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
											vNextSubs[nCurLayer].second.second.erase(subIt);
											newVec.push_back(secWord);
											newVec.insert(newVec.end(),
															vNextSubs[nCurLayer].second.second.begin(),
															vNextSubs[nCurLayer].second.second.end()	);
										}
									}
								}
								if(!bFoundSubroutine)
								{
									if(secWord == "")
									{
										cout << " need after command ";
										if(	direction == next_changed ||
											direction == previous_changed	)
										{
											cout << "'changed' ";
										}else
											cout << "'unchanged' ";
										cout << "an subroutine for check" << endl;
									}else
									{
										cout << " predefined subroutines:" << endl;
										for(subIt= vNextSubs[nCurLayer].second.second.begin();
														subIt != vNextSubs[nCurLayer].second.second.end(); ++subIt	)
										{
											cout << "             " << *subIt << endl;
										}
										cout << " subroutine '" << secWord << "' do not exist " << endl;
										secWord= "";
									}
									command= "";
									continue;
								}
							}

						}else if(folderNr < 1)
						{
							cout << " folder number after '" << word;
							cout << " has to be greater than 0" << endl;
							command= "";
							continue;
						}
					}else
						iresult >> secWord;
				}
				if(secWord != "")
				{
					vector<string> spl;

					if(	direction != all &&
						direction != current &&
						direction != next_changed &&
						direction != next_unchanged &&
						direction != previous_changed &&
						direction != previous_unchanged	)
					{
						cout << " for command '" << command << "'";
						cout << " is no definition of folder[:subroutine] allowed" << endl;
						command= "";
						continue;
					}
					split(spl, secWord, is_any_of(":"));
					folder= spl[0];
					if(spl.size() > 1)
						subroutine= spl[1];
					nExist= m_o2Client->transObj()->exist(folder, subroutine);
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
							cout << " '" << folder << ":" << subroutine << "'";
							cout << " do not exist in folder working list of ppi-server" << endl;
							if(	direction != next_changed &&
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
								vNextSubs[nCurLayer].second.second.clear();
							}
							command= "";
							continue;
						}
					}
					if(	direction == current ||
						vNextSubs.empty() || // direction is all
						vNextSubs[nCurLayer].first == ""	)
					{
						/*
						 * implement new folder[:subroutine]
						 * into current output queue when folder was same
						 * otherwise make an new layer
						 */
						ppi_time nullTime;
						vector<string> sub;
						pair<ppi_time, vector<string> > newSub;

						if(subroutine != "")
							sub.push_back(subroutine);
						if(	vNextSubs.size() >= (nCurLayer + 1) &&
							folder != vNextSubs[nCurLayer].first	)
						{
							/*
							 * define new layer
							 */
							++nCurLayer;
						}
						if(vNextSubs.size() >= (nCurLayer + 1))
						{
							vNextSubs[nCurLayer].first= folder;
							vNextSubs[nCurLayer].second.second= sub;
						}else
						{
							newSub= pair<ppi_time, vector<string> >(nullTime, sub);
							vNextSubs.push_back(pair<string, pair<ppi_time, vector<string> > >(folder, newSub));
						}
					} // i(direction == current && folder == "")

				}else // if(secWord != "")
				{
					if(	direction != all &&
						(	vNextSubs.empty() ||
							vNextSubs[nCurLayer].first == ""	)	)
					{
						cout << " no folder:subroutine defined" << endl;
						cout << " define first with $> CURDEBUG <folder>[:subroutine]" << endl;
						command= "";
						continue;
					}
				} // end of else from secWord != ""
				if(nExist == -1)
				{
					cout << " '" << folder << ":" << subroutine << "'";
					cout << " exist inside ppi-server, but is not defined for holding" << endl;
					cout << " type first $> HOLDDEBUG " << folder << ":" << subroutine << endl;
					vHistory.push_back(command);
					command= "";
					continue;
				}
				if(m_o2Client.get())
				{
					ppi_time newTime;
					vector<string> vShowSubroutines;

					if(direction != all)
					{
						folder= vNextSubs[nCurLayer].first;
						if(folderNr == 0)
							newTime= vNextSubs[nCurLayer].second.first;
						vShowSubroutines= vNextSubs[nCurLayer].second.second;

					}else
						vShowSubroutines.push_back(subroutine);
					newTime= *m_o2Client->transObj()->writeDebugSession(
									folder, vShowSubroutines, direction, &newTime, folderNr	);
					if(	direction == all ||
						newTime.isSet()		)
					{
						if(direction != all)
							vNextSubs[nCurLayer].second.first= newTime;
						vHistory.push_back(command);
					}
				}else
					cout << "ppi-client not defined with second connection "
									"to server (start client with option --hear)" << endl;
				command= "";
				continue;

			}else if(	command == "clear" ||
						command == "CLEARDEBUG"	)
			{
				nCurLayer= 0;
				vNextSubs.clear();
				if(m_o2Client.get())
					m_o2Client->transObj()->clearDebugSessionContent();
				else
					cout << "ppi-client not defined with second connection "
									"to server (start client with option --hear)" << endl;
				vHistory.push_back(command);
				command= "";
				continue;
			}else if(command == "history")
			{
				vector<string>::size_type n(1000);

				if(!vHistory.empty())
				{
					for(vector<string>::iterator it= vHistory.begin(); it != vHistory.end(); ++it)
					{
						cout << n << "  " << *it << endl;
						++n;
					}
				}else
					cout << " no history exist" << endl;
				command= "";
				continue;
			}
#ifdef SERVERDEBUG
			cout << "send: '" << m_sCommand << "'" << endl;
#endif // SERVERDEBUG
			command+= "\n";
			descriptor << command;
			descriptor.flush();

			do{
				descriptor >> result;
				trim(result);
#ifdef SERVERDEBUG
				cout << "get: " << result << endl;
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

				}else if(result == "done")
				{
					bWaitEnd= false;
					if(bHoldDebug)
						command= "HOLD" + command;
					trim(command);
					vHistory.push_back(command);
				}
				if(xmlReader.get())
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
				}else if(	m_o2Client.get() &&
							(	word == "HOLDDEBUG" ||
								word == "hold"			)	)
				{
					vector<string> spl;

					word= "";
					iresult >> word;
					if(word != "")
					{
						/*
						 * second word after HOLDDEBUG
						 * should be always correct <folder>:<subroutine>
						 * because server do not return error
						 * when write DEBUG command
						 */
						split(spl, word, is_any_of(":"));
						m_o2Client->transObj()->setHoldingFolder(spl[0], spl[1]);

					}else
					{
						m_bHoldAll= true;
						m_o2Client->transObj()->allFolderHolding();
					}
				}
				if(result != "done")
					cout << result << endl;
			}while(	bWaitEnd
					||
					xmlReader.get()	);
			command= "";

		}while(m_bWait);

		descriptor << "ending\n";
		descriptor.flush();
		return true;
	}

	short ClientTransaction::exist(const string& folder, const string& subroutine)
	{
		short nExist(0);
		debugSessionTimeMap::iterator timeIt;
		vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator folderIt;
		map<string, map<string, unsigned long> >::iterator foundFolder;
		map<string, unsigned long>::iterator foundSubroutine;

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
		LOCK(m_DEBUGSESSIONCHANGES);
		m_bHoldAll= true;
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	void ClientTransaction::writeHelpUsage(const string& sfor)
	{
		if(sfor == "?")
		{
			cout << endl;
			cout << "    ?         - show this help" << endl;
			cout << "    ?value    - show all commands round of values "
							"inside ppi-server" << endl;
			cout << "    ?debug    - show all commands round "
							"debug session of working list" << endl;
			cout << endl;
			cout << "    quit      - ending connection to server" << endl;
			cout << "                and abort ppi-client" << endl;
			cout << "    history   - show all last typed commands with number" << endl;
			cout << "                which can allocate with '!<number>'" << endl;
			cout << "    CHANGE <user>" << endl;
			cout << "    change <user>   - changing user by current session with server" << endl;
			cout << "                    - after pressing return, will be asked for password" << endl;

		}else if(sfor == "?value")
		{
			cout << endl;
			cout << "    GET <folder>:<subroutine>" << endl;
			cout << "          -     get the current value from the subroutines in the folder" << endl;
			cout << "                folder and subroutine are separated with an colon" << endl;
			cout << "    SET <folder>:<subroutine> <value>" << endl;
			cout << "          -     set the given value from given subroutine in given folder" << endl;
			cout << "    HEAR <folder>:<subroutine>" << endl;
			cout << "          -     if the client has set an second connection with option --hear," << endl;
			cout << "                client can order with this command to hear on the given folder:subroutine's" << endl;
			cout << "                 for changes" << endl;
			cout << "    NEWENTRYS" << endl;
			cout << "          -     clearing all entry's which are set with the command HEAR" << endl;
			cout << "                this command is only when ppi-server is started with option --hear" << endl;

		}else if(sfor == "?debug")
		{
			cout << endl;
			cout << "    DEBUG [-i|-ow] <folder[:subroutine]/owreaderID>" << endl;
			cout << "                -   show by running server debugging messages for given folder and subroutine" << endl;
			cout << "                    when no subroutine given, the hole folder is set for debugging" << endl;
			cout << "                    by add option -i, when for folder defined an inform parameter" << endl;
			cout << "                    show this calculation also by debug output." << endl;
			cout << "                    if option -ow be set client get DEBUG info for benchmark of external ports" << endl;
			cout << "                    and folder have to be the OWServer ID (owreaderID)." << endl;
			cout << "                    (the OWServer ID you can see by starting ppi-server on command line after '### starting OWServer)" << endl;
			cout << "                    both option -i and -ow cannot be used in same time" << endl;
			cout << "    HOLDDEBUG [-i] <folder[:subroutine]>" << endl;
			cout << "                -   same as command DEBUG (without possibility of option -ow)" << endl;
			cout << "                    but debug session output will be saved in the background" << endl;
			cout << "                    and will be shown only with follow commands" << endl;
			cout << "                    (command only be allowed when ppi-client started with hearing thread option --hear)" << endl;
			cout << "    run" << endl;
			cout << "    RUNDEBUG    -   show all getting debug session folders with count" << endl;
			cout << "    show [folder[:subroutine]]" << endl;
			cout << "    SHOWDEBUG [folder[:subroutine]]" << endl;
			cout << "                -   show debug session output of working list" << endl;
			cout << "                    which was set before with HOLDDEBUG into holding state" << endl;
			cout << "                    to save in background" << endl;
			cout << "                    when before no 'CURDEBUG' defined, or removed," << endl;
			cout << "                    and an folder and or subroutine given by command" << endl;
			cout << "                    this will be defined inside current layer" << endl;
			cout << "                    (command only be allowed when ppi-client started with hearing thread option --hear)" << endl;
			cout << "    cur [folder[:subroutine]]" << endl;
			cout << "    current [folder[:subroutine]]" << endl;
			cout << "    CURDEBUG [folder[:subroutine]]" << endl;
			cout << "               -    show always only folder or folder:subroutine of debug session" << endl;
			cout << "                    running by one pass" << endl;
			cout << "                    by first calling it will take the first pass of folder given name which found" << endl;
			cout << "                    CURDEBUG show always the folder by same pass" << endl;
			cout << "                    with below commands you can navigate thru the others" << endl;
			cout << "                    by typing again you can leaf the info of folder:subroutine" << endl;
			cout << "                    because the system remember the last entry" << endl;
			cout << "                    when typing command again with info of folder and or subroutine," << endl;
			cout << "                    when the folder is the same, all subroutines defined with ADDDEBUG (see below)" << endl;
			cout << "                    will be removed and the new one will be shown." << endl;
			cout << "                    Otherwise, by an new folder, it will be created an new layer" << endl;
			cout << "                    with the new folder and or subroutine," << endl;
			cout << "                    in which you can go back with 'BACKDEBUG' in a later time." << endl;
			cout << "                    In this case, when an other folder was taken, the client" << endl;
			cout << "                    try to show the new folder from same time," << endl;
			cout << "                    or one step before." << endl;
			cout << "    add <subroutine>" << endl;
			cout << "    ADDDEBUG <subroutine>" << endl;
			cout << "                -   add subroutine to current folder defined with 'CURDEBUG'" << endl;
			cout << "                    which is defined for listing" << endl;
			cout << "    rm <subroutine>" << endl;
			cout << "    remove <subroutine>" << endl;
			cout << "    REMOVEDEBUG <subroutine>" << endl;
			cout << "                -   remove the subroutine from current folder defined inside 'CURDEBUG'" << endl;
			cout << "                    which is defined for listing" << endl;
			cout << "    next" << endl;
			cout << "    NEXTDEBUG  -    show also only one subroutine like 'CURDEBUG'" << endl;
			cout << "                    but always the next one" << endl;
			cout << "                    after command can also be typed the folder pass" << endl;
			cout << "                    which want to be shown" << endl;
			cout << "    prev" << endl;
			cout << "    previous" << endl;
			cout << "    PREVDEBUG   -   same as command 'NEXTDEBUG'" << endl;
			cout << "                    but show the subroutine before" << endl;
			cout << "                    after command can also be typed the folder pass" << endl;
			cout << "                    which want to be shown" << endl;
			cout << "    back" << endl;
			cout << "    BACKDEBUG   -   go from an new defined layer with 'CURDEBUG'" << endl;
			cout << "                    back to the last layer with the last time" << endl;
			cout << "    stop [-ow] <folder[:subroutine]/owreaderID>" << endl;
			cout << "    STOPDEBUG [-ow] <folder[:subroutine]/owreaderID>" << endl;
			cout << "                -   stop debugging session for folder:subroutine" << endl;
			cout << "                    and or saving into background" << endl;
			cout << "    clear" << endl;
			cout << "    CLEARDEBUG  -   clear debug content from background" << endl;
			cout << "                    but not currently debugging session" << endl;
/*			cout << "    save <file>" << endl;
			cout << "    SAVEDEBUG <file>" << endl;
			cout << "                -   save all debug session content, since time called," << endl;
			cout << "                    into given file in same directory where ppi-client was called" << endl;
			cout << "                    ending by call this command again, call 'STOPDEBUG' or stop editor" << endl;
			cout << "    load <file>" << endl;
			cout << "    LOADDEBUG <file>" << endl;
			cout << "                -   load debug session content which was saved before with 'SAVEDEBUG'" << endl;
			cout << "                    This is the only one command which can called on beginning" << endl;
			cout << "                    ('ppi-client LOADDEBUG <file>') where the ppi-server" << endl;
			cout << "                    not need to run" << endl;*/
		}else
		{
			cout << endl;
			cout << " no helping defined for '" << sfor << "'" << endl;
		}
	}

	bool ClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		if(m_bHearing)
			return hearingTransfer(descriptor);
		return userTransfer(descriptor);
	}

	void ClientTransaction::setHoldingFolder(const string& folder, const string& subroutine)
	{
		pair<string, string> folderSub(folder, subroutine);

		LOCK(m_DEBUGSESSIONCHANGES);
		m_vsHoldFolders[folder][subroutine]= 0;
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	void ClientTransaction::clearHoldingFolder(const string& folder, const string& subroutine)
	{
		map<string, map<string, unsigned long> >::iterator foundFolder;
		map<string, unsigned long>::iterator foundSubroutine;

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
		}else
			m_vsHoldFolders.clear();
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	void ClientTransaction::clearDebugSessionContent()
	{
		LOCK(m_DEBUGSESSIONCHANGES);
		m_mmDebugSession.clear();
		UNLOCK(m_DEBUGSESSIONCHANGES);
	}

	map<string, unsigned long> ClientTransaction::getRunningFolderList()
	{
		unsigned long ulRun;
		map<string, unsigned long> mRv;
		debugSessionTimeMap::iterator timeIt;
		vector<IDbFillerPattern::dbgSubroutineContent_t>::iterator folderIt;

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
		folderCount= getRunningFolderList();
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

						msg=  " show folder '";
						msg+= folderIt->folder;
						msg+= "' again from begin?\n";
						runUserTransaction(false);
						msg= ask(/*YesNo*/true, msg);
						runUserTransaction(true);
						if(msg == "N")
						{
							m_dbgSessTime= *curTime;
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
						runUserTransaction(false);
						msg= ask(/*YesNo*/true, msg);
						runUserTransaction(true);
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
							return &m_dbgSessTime;
						}
					}
					break;

				default:// show == previous
					if(want <= 1)
					{
						string msg;

						msg=  " show folder '";
						msg+= folderIt->folder;
						msg+= "' again from begin?\n";
						runUserTransaction(false);
						msg= ask(/*YesNo*/true, msg);
						runUserTransaction(true);
						if(msg == "N")
						{
							m_dbgSessTime= *curTime;
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
						cout << " only " << folderCount[folder];
						cout << " folder count of '";
						cout << folder << "' do exist" << endl;
						m_dbgSessTime= *curTime;
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
							cout << glob::addPrefix(id, mFolderStart[folderIt->folder]);
							mFolderStart[folderIt->folder]= "";
						}
						cout << glob::addPrefix(id, content.str());
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
			cout << "no debug session content ";
			if(folder == "")
				cout << "from working list exist" << endl;
			else
			{
				cout << " exist by ";
				if(subroutines.size() != 1)
					cout << "folder ";
				cout << folder;
				if(subroutines.size() > 1)
				{
					cout << " with follow subroutines:" << endl;
					for(vector<string>::const_iterator it= subroutines.begin();
									it != subroutines.end(); ++it				)
					{
						cout << "                     " << *it << endl;
					}
				}else if(subroutines.size() == 1)
					cout << ":" << subroutines[0] << endl;
				else
					cout << endl;
			}
		}else if(show != all)
		{
			bool moreThanOne(false);
			string noWrite;
			string notWritten;

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
					cout << " follow subroutines ";
				else
					cout << " subroutine '" << noWrite << "' ";
				cout << "wasn't inside current folder pass" << endl;
				cout << " of debug session getting from server";
				if(moreThanOne)
					cout << ":";
				cout << endl;
				if(moreThanOne)
					cout << notWritten;
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
				cout << result << endl;
		}
		return false;
	}

	void ClientTransaction::prompt(const string& str/*= ""*/)
	{
		LOCK(m_PROMPTMUTEX);
		if(str != "")
			m_sPrompt= str;
		if(	!m_bRunHearTran &&
			!m_bRunUserTrans	)
		{
			cout << m_sPrompt << flush;
		}
		UNLOCK(m_PROMPTMUTEX);
	}

	void ClientTransaction::runUserTransaction(bool run)
	{
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
		int nErrorNum;
		string buffer, str;
		stringstream ss(error);
		ostringstream newerr;

		ss >> buffer;
		ss >> nErrorNum;
		if(m_bShowENum)
		{
			if(buffer == "ERROR")
				nErrorNum+= m_nOutsideErr;
			else
				nErrorNum+= m_nOutsideWarn;
			newerr << buffer << " ";
			newerr.width(3);
			newerr.fill('0');
			newerr << nErrorNum;
			cerr << newerr.str() << endl;
			return;
		}

		newerr << "GETERRORSTRING ";
		if(buffer == "WARNING")
			nErrorNum*= -1;
		newerr << nErrorNum << endl;
		descriptor << newerr.str();
		descriptor.flush();

		descriptor >> buffer;
		if(!descriptor.error())
		{
			trim(buffer);
			cerr << buffer << endl;
			return;
		}
		cerr << "lost connection to server" << endl;
		cerr << "default error string from 'src/client/lib/ClientTransaction':" << endl;

		// default error strings
		// when any go wrong on connection
		switch(nErrorNum)
		{
		case 1:
			str= "client beginning fault transaction";
			return;

		case 2:
			str= "no correct command given";
			return;

		case 3:
			nErrorNum= 3;
			break;

		case 4:
			str= "cannot found given folder for operation";
			return;

		case 5:
			str= "cannot found given subroutine in folder for operation";
			break;

		case 6:
			str= "Unknown value to set in subroutine";
			break;
		case 7:
			str= "no filter be set for read directory";
			break;
		case 8:
			str= "cannot read any directory";
			break;
		case 9:
			str= "cannot found given file for read content";
			break;
		case 10:
			str= "given ID from client do not exist";
			break;
		case 11:
			str= "wrong user or password";
			break;
		case 12:
			str= "do not use error number 12 now";
			break;
		case 13:
			str= "user has no permission";
			break;
		case 14:
			str= "subrutine isn't correct defined by the settings of config file";
			break;
		case 15:
			str= "user cannot login as first";
			break;
		case 16:
			str= "subroutine has no correct access to device";
			break;
		case 17:
			str= "cannot find OWServer for debugging";
			break;
		case 18:
			str= "no communication thread is free for answer "
					"(this case can behavior when the mincommunicationthreads parameter be 0)";
			break;
		case 19:
			str= "server will be stopping from administrator";
			break;
		case 20:
			str= "cannot load UserManagement correctly";
			break;
		case 21:
			str= "unknown options after command SHOW";
			break;
		default:
			ostringstream ostr;

			ostr << "### error code '" << error << "' is not defined for default" << endl;
			ostr << "    on file " << __FILE__ << endl;
			ostr << "   and line " << __LINE__;
			str= ostr.str();
			break;
		}
		cerr << str << endl;
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
