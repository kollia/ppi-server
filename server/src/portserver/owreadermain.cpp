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

#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>

#include <boost/algorithm/string/split.hpp>

#include "../pattern/server/ichipaccesspattern.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/URL.h"
#include "../util/properties.h"

#include "../logger/lib/LogInterface.h"

#include "../database/lib/DbInterface.h"

#include "../server/libs/client/SocketClientConnection.h"
#include "../server/libs/server/TcpServerConnection.h"

#include "owserver.h"
#include "OwServerQuestions.h"
#include "ExternPorts.h"
#include "LircClient.h"
#include "maximchipaccess.h"
#include "VellemannK8055.h"

using namespace std;
using namespace boost;
using namespace util;
using namespace logger;
using namespace design_pattern_world;
using namespace ppi_database;
using namespace ports;
using namespace server;

int main(int argc, char* argv[])
{
	bool bConf= false;
	bool bfreeze= false;
	unsigned short nServerID;
	unsigned short commport;
	int nLogAllSec, err;
	uid_t defaultuserID;
	ostringstream usestring;
	string commhost, property, servertype, questionservername("OwServerQuestion-");
	string workdir, sConfPath, fileName, defaultuser;
	vector<int> vLength;
	//vector<string> vParams;
	vector<string> vDescript;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	auto_ptr<OWServer> owserver;
	vector<string> vParams;
	Properties oServerProperties;
	DbInterface *db;
	IChipAccessPattern* accessPort;
	auto_ptr<OwServerQuestions> pQuestions;

	glob::processName("ppi-owreader");
	glob::setSignals("ppi-owreader");
	// create working directory
	directorys= split(directorys, argv[0], is_any_of("/"));
	dirlen= directorys.size();
	for(vector<string>::size_type c= 0; c < dirlen; ++c)
	{
		if(c == dirlen-2)
		{// directory is bin, Debug or Release
			if(directorys[c] == ".")
				workdir+= "../";
			break;
		}
		workdir+= directorys[c] + "/";
	}
	sConfPath= URL::addPath(workdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(sConfPath, "server.conf");
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		return EXIT_FAILURE;
	}
	oServerProperties.readLine("workdir= " + workdir);

	defaultuser= oServerProperties.getValue("defaultuser", /*warning*/false);
	if(defaultuser == "")
	{
		cerr << "### WARNING: defaultuser is not defined" << endl;
		cerr << "             so process run under 'nobody'" << endl;
		defaultuser= "nobody";
	}
	defaultuserID= URL::getUserID(defaultuser);
	commhost= oServerProperties.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= oServerProperties.needUShort(property);

	// initial interface to log client
	property= "timelogSec";
	nLogAllSec= oServerProperties.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start logging interface
	LogInterface::initial(	"ppi-owreader",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														0			),
							/*identif log*/nLogAllSec,
							/*wait*/true								);

	// ------------------------------------------------------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start database interface

	DbInterface::initial(	"ppi-owreader",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														5			)	);
	db= DbInterface::instance();
	// ------------------------------------------------------------------------------------------------------------


	for(int n= 0; n < argc; ++n)
		vLength.push_back(strlen(argv[n]));
	vParams.push_back("<ID>");
	vDescript.push_back("Specific ID of server");
	vParams.push_back("<type>");
	vDescript.push_back("type of server");
	if(argc >= 3)
	{
		istringstream oServerID(argv[1]);

		oServerID >> nServerID;
		if(nServerID > 0)
		{
			if(	strncmp(argv[2], "PORT", vLength[2]) == 0
				||
				strncmp(argv[2], "MPORT", vLength[2]) == 0
				||
				strncmp(argv[2], "RWPORT", vLength[2]) == 0 )
			{
				int needArgc= 4;
				PortTypes type;
				vector<string> sPorts, vSplit;
				vector<string>::iterator found;

				servertype= argv[2];
				for(int c= 3; c < argc; ++c)
				{
					if(strncmp(argv[c], "freeze", strlen(argv[c])))
					{
						split(vSplit, argv[c], is_any_of(" "));
						for(vector<string>::iterator it= vSplit.begin(); it != vSplit.end(); ++it)
						{
							if(*it != "")
								sPorts.push_back(*it);
						}
					}else
						bfreeze= true;
				}
				if(strncmp(argv[2], "PORT", vLength[2]) == 0)
				{
					type= PORT;
					usestring << "for all used external ports";
				}else if(strncmp(argv[2], "MPORT", vLength[2]) == 0)
				{
					type= MPORT;
					usestring << "to measure time on given external ports";
					if(bfreeze)
						needArgc= 5;
					vParams.push_back("[freeze]");
					vDescript.push_back("whether process should run under root to set sheduler to highest priority");

				}else if(strncmp(argv[2], "RWPORT", vLength[2]) == 0 )
				{
					type= RWPORT;
					usestring << "to read or write on given external port";
				}
				if(argc >= needArgc && sPorts.size() > 0)
				{
					accessPort= new ExternPorts(sPorts, type); // object will be given as pointer into OWServer, need no auto_ptr
					bConf= true;
				}else
				{
					vParams.push_back("<port> ...");
					vDescript.push_back("all external ports for measuring");
				}

			}else if(strncmp(argv[2], "LIRC", vLength[2]) == 0)
			{
				servertype= "LIRC";
				accessPort= new LircClient();
				bConf= true;

			}
#ifdef _OWFSLIBRARY
			else if(strncmp(argv[2], "maxim", vLength[2]) == 0)
			{
				string maximconf;
				vector<string> adapters;
				vector<string>::iterator first;

				servertype= "OWFS";
				split(adapters, argv[3], is_any_of(":"));
				if(adapters.size() > 0)
				{
					first= adapters.begin();
					maximconf= *first;
					adapters.erase(first);
					accessPort= new MaximChipAccess(maximconf, &adapters); // object will be given as pointer into OWServer, need no auto_ptr
					bConf= true;
				}else
				{
					vParams.push_back("<conf:adapters...>");
					vDescript.push_back("configuration string with maybe some adapters delimited with an colon");
				}

			}
#endif //_OWFSLIBRARY
#ifdef _K8055LIBRARY
			else if(strncmp(argv[2], "vellemann", vLength[2]) == 0)
			{
				vParams.push_back("<board>");
				vDescript.push_back("which board from Vellemann be used");
				usestring << "k8055 USB port from Vellemann on interface ";
				if(strncmp(argv[3], "k8055", vLength[3]) == 0)
				{
					long nJumper;
					istringstream oJumper(argv[4]);

					servertype= "Vk8055";
					vParams.push_back("<jumper>");
					vDescript.push_back("which jumper be set on board");
					oJumper >> nJumper;
					usestring << nJumper;
					if(	nJumper >= 0 && nJumper <= 3)
					{
						accessPort= new VellemannK8055(nJumper); // object will be given as pointer into OWServer, need no auto_ptr
						bConf= true;
					}
				}
			}
#endif // _K8055LIBRARY
		}
	}
	if(!bConf)
	{
		int pos= 0;
		string::size_type maxLen= 0;
		ostringstream error;

		error << "### ERROR: no correct parameters for " << argv[0] << " be set" << endl;
		error << "    usage: ppi-owreader ";
		for(vector<string>::iterator it= vParams.begin(); it != vParams.end(); ++it)
		{
			error << *it << " ";
			if(it->length() > maxLen)
				maxLen= it->length();
		}
		error << endl;
		for(vector<string>::iterator it= vParams.begin(); it != vParams.end(); ++it)
		{
			error << "               " << *it;
			for(int n= 0; n < (maxLen - it->length()); ++n)
				error << " ";
			error << " -  " << vDescript[pos] << endl;
			++pos;
		}
		LOG(LOG_ALERT, error.str());
		cerr << error.str() << endl;
		return EXIT_FAILURE;
	}


	cout << "    one wire reader" << flush;
	owserver= auto_ptr<OWServer>(new OWServer(nServerID, servertype, accessPort));
	cout << " with name '" << owserver->getServerName();
	cout << "' and ID '" << dec << nServerID << "'" << endl;
	cout << "    " << usestring.str();
	if(strncmp(argv[2], "maxim", vLength[2]) != 0)
		cout << endl;

	questionservername+= argv[1];
	pQuestions= auto_ptr<OwServerQuestions>(new OwServerQuestions(	"ppi-owreader", questionservername,
																	new SocketClientConnection(	SOCK_STREAM,
																								commhost,
																								commport,
																								10			),
																	owserver.get()								));

	if(owserver->start(&oServerProperties) != 0)
		return EXIT_FAILURE;

	err= pQuestions->run();
	if(err > 0)
	{
		string msg;

		msg=  "### ERROR: cannot start question thread for one wire server " + owserver->getServerName() + "\n";
		msg+= "           " + pQuestions->strerror(err);
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		pQuestions= auto_ptr<OwServerQuestions>();// delete OwServerQuestions before OWServer
		LogInterface::deleteObj();
		DbInterface::deleteAll();
		return EXIT_FAILURE;
	}

	pQuestions= auto_ptr<OwServerQuestions>();// delete OwServerQuestions before OWServer
	LogInterface::deleteObj();
	DbInterface::deleteAll();
	return EXIT_SUCCESS;
}
