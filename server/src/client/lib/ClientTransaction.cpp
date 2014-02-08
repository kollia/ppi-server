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

#include <boost/algorithm/string/trim.hpp>

#include "../../util/XMLStartEndTagReader.h"
#include "../../util/structures.h"

#include "ClientTransaction.h"

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
	}

	bool ClientTransaction::init(IFileDescriptorPattern& descriptor)
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
		cerr << "ERROR: get no result from server" << endl;
		m_bErrWritten= true;
		return false;
	}
		if(!bRightServer)
		{
			string::size_type nPos;
			ostringstream msg;

			if(result.length() > 20)
				result= result.substr(0, 20) + " ...";
			nPos= result.find('\n');
			if(nPos != string::npos)
				result= result.substr(0, nPos-1) + " ...";
			msg << "ERROR: undefined server running on ";
			msg << descriptor.getHostAddressName() << ":" << descriptor.getPort() << endl;
			msg << "       getting '" << result << "'";
			cerr << msg.str() << endl;
			m_bErrWritten= true;
			return false;
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
					cerr << "### ERROR: cannot read tc address for password" << endl;
					cerr << "           " << strerror(errno) << endl;
					m_bErrWritten= true;
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
						m_bErrWritten= true;
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
					m_bErrWritten= true;
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
				printError(descriptor, result);
				m_bErrWritten= true;
				return false;
			}
		}
		if(bSecConn)
		{
			m_o2Client= auto_ptr<HearingThread>(new HearingThread(descriptor.getHostAddressName(), descriptor.getPort(),
																							sCommID, user, pwd, m_bOwDebug));
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
		auto_ptr<XMLStartEndTagReader> xmlReader;
		device_debug_t tdebug;
		timeval time;
		struct tm ttime;
		char stime[22];

		if(m_bHearing)
		{
			do{
				descriptor >> result;
				if(bHeader)
				{
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
				if(result != "stopclient\n")
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
				char str[200];

				cout << "$>" << flush;
				while(!(std::cin.getline(str, 199, '\n')))
				{
					std::cin.clear();
				    //std::cin.ignore(std::numeric_limits<streamsize>::max(),'\n');
					usleep(500000);
					cout << "." << flush;
				}
				m_sCommand= str;
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
				xmlReader= auto_ptr<XMLStartEndTagReader>(new XMLStartEndTagReader());

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
					number-= m_nOutsideErr;
				else
					number+= m_nOutsideWarn;
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
				if(	result.substr(0, 6) == "ERROR "
					||
					result.substr(0, 8) == "WARNING "	)
				{
					printError(descriptor, result);

				}else if(result == "done")
				{
					bWaitEnd= false;
				}else
				{
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
					xmlReader.get()	);

		}while(m_bWait);

		descriptor << "ending\n";
		descriptor.flush();
		return true;
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
	}

}
