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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <sstream>

#include "../logger/lib/LogInterface.h"

#include "../util/XMLStartEndTagReader.h"
#include "../util/configpropertycasher.h"

#include "ClientTransaction.h"

using namespace std;
using namespace util;

namespace server
{
	ClientTransaction::ClientTransaction(vector<string> options, string command)
	: m_mOwDevices(), m_mOwMaxTime(), m_mOwMaxCount()
	{
		m_bWait= false;
		m_bShowENum= false;
		m_vOptions= options;
		m_sCommand= command;
		m_o2Client= NULL;
		m_bHearing= false;
		m_bOwDebug= false;
	}

	bool ClientTransaction::init(IFileDescriptorPattern& descriptor)
	{
		short x= 0;
		/**
		 * unique ID get from server
		 */
		string sCommID;
		string result, logMsg, user, pwd;
		string sSendbuf("GET");
		bool bOp= true;
		bool bSecConn= false;
		bool bRightServer= true;
		bool bStatus= false;
		unsigned int nOptions= m_vOptions.size();
		unsigned short owserver;

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
				else
					bOp= false;

			}else
			{
				if(m_vOptions[o] == "-w")
					m_bWait= true;
				else if(m_vOptions[o] == "-d")
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
		if(m_bWait)
			sSendbuf+= " wait";
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

		if(	result.length() < 13
			||
			result.substr(0, 12) != "port-server:"	)
		{
			bRightServer= false;
		}else
			sCommID= result.substr(12);
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
		if(!bRightServer)
		{
			string msg("ERROR: undefined server running on port\n");


			msg+= "       getting '";
			msg+= result + "'";
			cerr << msg << endl;
			LOG(LOG_ALERT, msg);
			return false;
		}

		if(bStatus)
		{
			char stime[20];
			time_t t;

			t= time(NULL);
			strftime(stime, sizeof(stime), "%H:%M:%S %d.%m.%Y", localtime(&t));
			cout << endl << "actual time " << stime << endl;
		}
		if(	m_bWait
			||
			bSecConn
			||
			user != ""
			||
			pwd != ""
			||
			!bStatus	)
		{
			int c;
			struct termios term, backup;
			bool readtc= true;

			if((tcgetattr(STDIN_FILENO, &term)) < 0)
			{
				readtc= false;
				if(errno != 25)
				{
					cerr << "### ERROR: cannot read tc address for password" << endl;
					cerr << "           " << strerror(errno) << endl;
					return false;
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
						cerr << "### ERROR: cannot set tc address for password" << endl;
						cerr << "           " << strerror(errno) << endl;
						return false;
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
					cerr << "### ERROR: cannot set back tc address for any inserts" << endl;
					cerr << "           " << strerror(errno) << endl;
					return false;
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
			if(result != "OK")
			{
				printError(result);
				return false;
			}
		}
		if(bSecConn)
		{
			m_o2Client= new HearingThread(descriptor.getHostAddressName(), descriptor.getPort(),
											sCommID, user, pwd, m_bOwDebug);
			m_o2Client->start();
		}


		return true;
	}

	bool ClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		bool bWaitEnd= false;
		bool bHeader= true;
		long nsec, nmsec, nusec;
		char buf[10];
		string logMsg, result, sDo;
		string sSendbuf, last;
		XMLStartEndTagReader* xmlReader= NULL;
		OWServer::device_debug_t tdebug;

		if(m_bHearing)
		{
			do{
				descriptor >> result;
				if(bHeader)
				{
					cout << "Nr.| CALL | last call        |    value     |  every   | need time | length time "
						"| act. | chip ID / pin" << endl;
					cout << "---------------------------------------------------------------------------------"
						"------------------------------" << endl;
					bHeader= false;
				}
				if(result != "stopclient\n")
				{
					if(m_bOwDebug && result.substr(0, 1) != "0")
					{
						if(result != "done\n")
						{
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
							devString >> tdebug.device;

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

							cout << sDo << " " << tdebug.device << endl;
						}else
						{
							cout << endl;
							bHeader= true;
						}
					}else
						cout << result;
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
		do{
			if(m_sCommand == "")
			{
				cout << "$>" << flush;
				getline(std::cin, m_sCommand);
				if(	m_sCommand == "quit"
					||
					m_sCommand == "exit"	)
				{
					m_sCommand= "ending\n";
					descriptor << m_sCommand;
					return false;
				}
			}

			if(	m_sCommand.substr(0, 3) == "DIR"
				||
				m_sCommand.substr(0, 6) == "status"	)
			{
				bWaitEnd= true;
			}else if(m_sCommand.substr(0, 7) == "CONTENT")
				xmlReader= new XMLStartEndTagReader();

#ifdef SERVERDEBUG
			cout << "send: '" << m_sCommand << "'" << endl;
#endif // SERVERDEBUG
			if(m_sCommand.substr(0, 15) == "GETERRORSTRING ")
			{
				string com;
				int number;
				istringstream icommand(m_sCommand);
				ostringstream ocommand;

				icommand >> com;
				icommand >> number;
				if(number > 0)
					number+= m_nOutsideErr;
				else
					number-= m_nOutsideWarn;
				ocommand << com << " " << number;
				m_sCommand= ocommand.str();
			}
			m_sCommand+= "\n";
			descriptor << m_sCommand;
			descriptor.flush();
			m_sCommand= "";

			do{
				descriptor >> result;
				result= ConfigPropertyCasher::trim(result, "\n");
#ifdef SERVERDEBUG
				cout << "get: " << result << endl;
#endif // SERVERDEBUG

				if(descriptor.eof())
				{
					cerr << endl;
					cerr << "ERROR: lost connection to server" << endl;
					return false;
				}
				if(result.substr(0, 6) == "ERROR ")
				{
					printError(result);
				}else if(result == "done")
				{
					bWaitEnd= false;
				}else
				{
					if(xmlReader)
					{
						result= xmlReader->readLine(result);
						if(xmlReader->end())
						{
							string error;

							error= xmlReader->error();
							if(error != "")
								printError(error);
							delete xmlReader;
							xmlReader= NULL;
							break;
						}
					}else if(m_sCommand == "GETMINMAXERRORNUMS")
					{
						int warn, err;
						istringstream iresult(result);
						ostringstream oresult;

						iresult >> warn;
						iresult >> err;
						oresult << (warn - m_nOutsideWarn);
						oresult << " ";
						oresult << (err + m_nOutsideErr);
						result= oresult.str();
					}
					cout << result << endl;
				}
			}while(	bWaitEnd
					||
					xmlReader	);

		}while(m_bWait);

		return false;
	}

	string ClientTransaction::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 0:
			str= "no error occurred";
			break;
		default:
			if(error > 0)
				str= "Undefined transaction error occurred";
			else
				str= "Undefined transaction warning occurred";
			break;
		}
		return str;
	}

	void ClientTransaction::printError(string error)
	{
		short param= 0;
		int nErrorNum;
		string buffer;
		stringstream ss(error);

		if(m_bShowENum)
		{
			ostringstream newerr;

			ss >> buffer;
			ss >> nErrorNum;
			if(buffer == "ERROR")
				nErrorNum+= m_nOutsideErr;
			else
				nErrorNum+= m_nOutsideWarn;
			newerr << buffer << " ";
			newerr.width(3);
			newerr.fill('0');
			newerr << nErrorNum;
			cerr << newerr.str() << endl;
		}
		while(ss >> buffer)
		{
			if(buffer != "ERROR")
			{
				//char *cErr= getChars(buffer);
				int num= atoi(buffer.c_str());

				if(param == 1)
				{
					switch(num)
					{
					case 1:
						cerr << "client beginning fault transaction" << endl;
						return;

					case 2:
						cerr << "no correct command given" << endl;
						return;

					case 3:
						nErrorNum= 3;
						break;

					case 4:
						cerr << "cannot found given folder for operation" << endl;
						return;

					case 5:
						cerr << "cannot found given subroutine in folder for operation" << endl;
						break;

					case 6:
						cerr << "unknow value to set in subroutine" << endl;
						break;
					case 7:
						cerr << "no filter be set for read directory" << endl;
						break;
					case 8:
						cerr << "cannot read any directory" << endl;
						break;
					case 9:
						cerr << "cannot found given file for read content" << endl;
						break;
					case 10:
						cerr << "given ID from client do not exist" << endl;
						break;
					case 11:
						cerr << "given user do not exist" << endl;
						break;
					case 12:
						cerr << "wrong password for given user" << endl;
						break;
					case 13:
						cerr << "user has no permission" << endl;
						break;
					case 14:
						cerr << "subrutine isn't correct defined by the settings of config file" << endl;
						break;
					case 15:
						cerr << "root cannot login as first user" << endl;
						break;
					case 16:
						cerr << "subroutine has no correct access to device" << endl;
						break;
					default:
						cerr << error << endl;
					}
				}else
				{
					switch(nErrorNum)
					{
					case 3:
						cerr << "command parameter " << num << "is incorrect" << endl;
						return;
					}
				}
			}
			++param;
		}
	}

	ClientTransaction::~ClientTransaction()
	{
		if(m_o2Client)
		{
			m_o2Client->stop();
			delete m_o2Client;
		}
	}

}
