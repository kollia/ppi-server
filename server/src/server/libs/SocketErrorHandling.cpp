/**
 *   This file 'SocketErrorHandling.cpp' is part of ppi-server.
 *   Created on: 09.10.2014
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "SocketErrorHandling.h"

void SocketErrorHandling::read()
{
	vector<string> classes;

	classes.push_back("SocketConnection");
	classes.push_back("SocketClientConnection");
	classes.push_back("SocketServerConnection");
	addGroupErrorClasses("IClientConnectArtPattern", classes);

	setDescription("en", "SocketConnection", "getaddrinfo",
					"by reading address info from host '@1' and port @2 follow error occurred:");
	setDescription("en", "SocketConnection", "setsockopt",
					"cannot set socket option for host '@1' and port @2 to check always to be alive");
	setDescription("en", "SocketConnection", "socket",
					"cannot create socket to host '@1' with port @2 by follow error:");

	setDescription("en", "SocketClientConnection", "transfer",
					"transaction to '@1' ending -> close connection");
	setDescription("en", "SocketClientConnection", "connect",
					"by trying to connect while @1 seconds on server host '@2' port @3 follow error occurred:");
	setDescription("en", "SocketClientConnection", "fdopen",
					"cannot open descriptor to write/read on host '@1' with port @2 by follow error:");

	setDescription("en", "SocketConnection", "reuse",
					"cannot set socket option to reuse on host '@1' and port @2 to check always to be alive");
	setDescription("en", "SocketServerConnection", "bind",
					"cannot bind socket on host '@1' and port @2");
	setDescription("en", "SocketServerConnection", "listen",
					"cannot listen on host '@1' and port @2");
	setDescription("en", "SocketServerConnection", "accept",
					"cannot accept socket:@3 binding connection on host '@1' with port @2 because follow error occurred:");
	setDescription("en", "SocketServerConnection", "fdopen",
					"cannot open descriptor to write/read on host '@1' with port @2 by follow error:");

	setDescription("en", "OutsideClientTransaction", "init",
					"object of OutsideClientTransaction is not defined to handle transactions");
	setDescription("en", "OutsideClientTransaction", "descriptor",
					"descriptor of connection reaching end of transactions");

	setDescription("en", "ExternClientInputTemplate", "noTransactionClass",
					"no transaction object existing to sending anything");
	setDescription("en", "ExternClientInputTemplate", "noSendClass",
					"no connection object defined to create connection for sending");
	setDescription("en", "ExternClientInputTemplate", "noGetClass",
					"no connection object defined to create connection for getting questions");
	setDescription("en", "ExternClientInputTemplate", "noTransSendMethod",
					"no transaction object exist to send any method");
	setDescription("en", "ExternClientInputTemplate", "noConSendMethod",
					"no connection object defined to sending any method");
	setDescription("en", "ExternClientInputTemplate", "closeConnection",
					"transaction exception by close for Interface @1::@2");
	setDescription("en", "ExternClientInputTemplate", "closeSendConnection",
					"exception by closing send connection on Interface @1::@2");

	setDescription("en", "ExternClientInputTemplate", "NoAnswerSender_start",
					"can not send questions which need no answer over external thread"
					"from '@1' to '@2'");

	setDescription("en", "process", "fork",
					"@1 cannot make fork for new process '@2'");
	setDescription("en", "process", "check",
					"by checking from @1 initialization of @2");
	setDescription("en", "process", "closeSendConnection",
					"process @1 want to closing send connection to @2");
	setDescription("en", "process", "waiting",
					"@1 waiting for foreign process @2, get back wrong answer '@3'");
	setDescription("en", "process", "init",
					"by checking process @1 whether any other running on same port");
	setDescription("en", "process", "wrong_answer",
					"by checking process @1 initialization of client @2, get back wrong answer '@3'");
	setDescription("en", "process", "running",
					"by check from @1 whether process @2 running, get back wrong answer '@3'");
	setDescription("en", "process", "stopping",
					"by check from @1 whether process @2 should stopping, get back wrong answer '@3'");

	setDescription("en", "ProcessStarter", "execv",
					"by trying to process executable '@1'");

	setDescription("en", "ServerMethodTransaction", "stream_end",
					"inside @1 connection ID:@2 from @3 to @4 reaching end of stream '@5'");
	setDescription("en", "ServerMethodTransaction", "descriptor_get",
					"inside @1 connection ID:@2 from @3 to @4 is broken");

	setDescription("en", "ServerThread", "accept",
					"Server @1 try to accept communication to @2:@3");

	setDescription("en", "ServerProcess", "accept",
					"Server @1 try to accept communication to @2:@3");
	setDescription("en", "ServerProcess", "init",
					"'@1' should run on foreign host (@2), so do not start any server process");
	setDescription("en", "ServerProcess", "openSendConnection",
					"cannot open any connection for @1, maybe other server running on port");
	setDescription("en", "ServerProcess", "communicationclient_start",
					"ServerProcess @1 try to starting pool of communication threads");

	setDescription("en", "OWInterface", "openSendConnection_warn",
					"by open sending connection to external port server '@1' with ID @2");
	setDescription("en", "OWInterface", "openSendConnection_err",
					"cannot open sending connection to external port server '@1' with ID @2");

	setDescription("en", "ServerDbTransaction", "getStatusInfo_ppi_serverM",
					"by trying to get status info from working list server");
	setDescription("en", "ServerDbTransaction", "getStatusInfo_owreaderM",
					"by trying to get status info from external port server @1");
	setDescription("en", "ServerDbTransaction", "getStatusInfo_ppi_server",
					"by trying to get status info from working list server\n"
					"get unknown answer '@1'");
	setDescription("en", "ServerDbTransaction", "getStatusInfo_owreader",
					"by trying to get status info from port server @1\n"
					"get unknown answer '@2'");
}

bool SocketErrorHandling::setAddrError(const string& classname, const string& error_string,
				int error_nr, int errno_nr, const string& decl/*= ""*/)
{
	ostringstream oerror;

	oerror << error_nr;
	if(!setErrnoError(classname, error_string, errno_nr, decl))
		return false;
	m_tError.type= specific_error;
	m_tError.adderror= "getaddrinfo" + oerror.str();
	return true;
}

string SocketErrorHandling::getErrorDescriptionString(errorVals_t error) const
{
	string::size_type nMethodLen;
	string sRv, err;

	if(error.type == specific_error)
	{
		sRv= "getaddrinfo";
		nMethodLen= sRv.length();
		if(	error.adderror.length() >= nMethodLen &&
			error.adderror.substr(0, nMethodLen) == sRv)
		{
			int error_nr;
			istringstream ierror;

			ierror.str(error.adderror.substr(nMethodLen));
			ierror >> error_nr;
			sRv= BaseErrorHandling::getErrorDescriptionString(error);
			if(sRv == "") // when no correct description string found, return null string
				return "";// because then define BaseErrorHandling an message for not found
			sRv+= "\n";
			if(error_nr != EAI_SYSTEM)
				sRv+= gai_strerror(error_nr);
			else
				sRv+= getErrnoString(error.errno_nr);
		}
	}
	if(sRv == "")
		sRv= BaseErrorHandling::getErrorDescriptionString(error);
	return sRv;
}


bool SocketErrorHandling::searchResultError(vector<string>& result)
{
	bool warning;
	IEH::error_types err;
	SocketErrorHandling error;

	if(hasError())
		return true;
	warning= hasWarning();
	for(vector<string>::iterator it= result.begin();
					it != result.end(); ++it	)
	{
		error.setErrorStr(*it);
		err= error.getErrorType();
		if(	err != IEH::NO &&
			err != IEH::UNKNOWN	)
		{
			if(error.hasError())
			{
				set(&error);
				return true;
			}else
			{
				/**
				 * warning occurred,
				 * implement when no warning
				 * before exist
				 */
				if(!warning)
				{
					set(&error);
					warning= true;
				}
			}
		}// if(	err != IEH::NO && != IEH::UNKNOWN	)
	}// foreach(result)

	/*
	 * ask only for warning
	 * because by any error
	 * returned before directly
	 */
	if(hasWarning())
		return true;
	return false;
}



