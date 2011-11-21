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

#include <string.h>

#include <iostream>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/split.hpp>


#include "../pattern/util/LogHolderPattern.h"
#include "../pattern/server/ichipaccesspattern.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/URL.h"

#include "../util/properties/interlacedproperties.h"

#include "../database/lib/DbInterface.h"

#include "../server/libs/client/SocketClientConnection.h"
#include "../server/libs/server/TcpServerConnection.h"

#include "libs/chipaccess/ports/ExternPorts.h"
#include "libs/chipaccess/lirc/LircClient.h"
#include "libs/chipaccess/maxim/maximchipaccess.h"
#include "libs/chipaccess/vellemann/VellemannK8055.h"


#include "owserver.h"
#include "OwServerQuestions.h"
#include "ShellWriter.h"

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
#if 0
	// display only process with parameters for debugging
	cout << "start ";
	for(int n= 0; n < argc; ++n)
		cout << argv[n] << " ";
	cout << endl;
#endif
	bool bConf= false;
	bool bfreeze= false;
	unsigned short nServerID;
	unsigned short commport;
	int nLogAllSec, err;
	ostringstream usestring;
	string commhost, property, servertype, questionservername("OwServerQuestion-");
	string workdir, sConfPath, fileName, defaultuser, shelluser;
	vector<int> vLength;
	//vector<string> vParams;
	vector<string> vDescript;
	vector<string> directorys;
	vector<string>::size_type dirlen;
	auto_ptr<OWServer> owserver;
	vector<string> vParams;
	InterlacedProperties oServerProperties;
	DbInterface *db;
	IChipAccessPattern* accessPort;
	auto_ptr<OwServerQuestions> pQuestions;
	map<string, uid_t> users;

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
	oServerProperties.setDelimiter("owreader", "[", "]");
	oServerProperties.modifier("owreader");
	oServerProperties.readLine("workdir= " + workdir);
	if(!oServerProperties.readFile(fileName))
	{
		cout << "### ERROR: owreader cannot read '" << fileName << "'" << endl;
		return EXIT_FAILURE;
	}

	defaultuser= oServerProperties.getValue("defaultuser", /*warning*/false);
	if(defaultuser == "")
	{
		cerr << "### WARNING: defaultuser is not defined" << endl;
		cerr << "             so processing run under 'nobody'" << endl;
		defaultuser= "nobody";
	}
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
	// start database interface

	DbInterface::initial(	"ppi-owreader",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														5			),
							nLogAllSec									);
	db= DbInterface::instance();
	db->setThreadName(glob::getProcessName());
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
			if(strncmp(argv[2], "SHELL", vLength[3]) == 0)
			{
				if(argc != 4)
				{
					string msg;

					msg=  "### ERROR: cannot start owreader for ShellWriter\n";
					msg+= "           need 3 parameter, <ID> <type> and <user>\n";
					msg+= "           do not start server!";
					LOG(LOG_ALERT, msg);
					cerr << msg << endl;
					exit(EXIT_FAILURE);
				}
				servertype= "SHELL";
				accessPort= new ShellWriter(nServerID, argv[3]);
				shelluser= argv[3];
				bConf= true;

			}else if(	strncmp(argv[2], "PORT", vLength[2]) == 0
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
				if(argc >= 4)
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
			for(string::size_type n= 0; n < (maxLen - it->length()); ++n)
				error << " ";
			error << " -  " << vDescript[pos] << endl;
			++pos;
		}
		LOG(LOG_ALERT, error.str());
		cerr << error.str() << endl;
		return EXIT_FAILURE;
	}

	string value;
	ostringstream output;
	Properties newProp;
	const IPropertyPattern* pProp;

	output << "    one wire reader";
	owserver= auto_ptr<OWServer>(new OWServer(nServerID, servertype, accessPort));
	output << " with name '" << owserver->getServerName();
	output << "' and ID '" << dec << nServerID << "'" << endl;
	output << "    " << usestring.str();
	if(strncmp(argv[2], "maxim", vLength[2]) != 0)
		output << endl;
	cout << output.str();

	if(owserver->start(&newProp) != 0)
		return EXIT_FAILURE;

	LOG(LOG_INFO, "starting owreader object for extern interfaces\n\n" + output.str());

	questionservername+= argv[1];
	pQuestions= auto_ptr<OwServerQuestions>(new OwServerQuestions(	"ppi-owreader", questionservername,
																	new SocketClientConnection(	SOCK_STREAM,
																								commhost,
																								commport,
																								10			),
																	owserver.get()								));
	if(servertype != "SHELL")
	{
		pProp= oServerProperties.getSection("owreader", servertype);
		if(pProp == NULL)
		{
			string msg("spezification of external port reading " + servertype + " has no entry inside server.conf");

			LOG(LOG_ERROR, msg);
			cerr << "### ERROR: " << msg << endl;
		}
		// copy all properties with value in new Properties object
		// because start method of Thread object allow no const object
		while((property= pProp->nextProp()) != "")
		{
			value= pProp->getValue(property);
			newProp.setDefault(property, value);
		}
	}
	newProp.setDefault("confpath", sConfPath, /*overwrite*/false);

	users[defaultuser]= 0;
	if(servertype == "SHELL")
		users[shelluser]= 0;
	if(	!glob::readPasswd(oServerProperties.getValue("passwd"), users) ||
		servertype != "SHELL"												)
	{
		if(servertype == "SHELL")
		{
			string msg;

			msg=  "### WARNING: do not found user id for user " + shelluser + " inside passwd\n";
			msg+= "             so set process to default user " + defaultuser;
			LOG(LOG_WARNING, msg);
			cout << msg << endl;
		}
		if(setuid(users[defaultuser]) != 0)
		{
			string err;

			err=  "### ERROR: cannot set process to default user " + defaultuser + "\n";
			err+= "    ERRNO: " + *strerror(errno);
			LOG(LOG_ALERT, err);
			cerr << err << endl;
			exit(EXIT_FAILURE);
		}
	}else
	{
		if(setuid(users[shelluser]) != 0)
		{
			string err;

			err=  "### ERROR: cannot set process to user " + shelluser + " so set to default user " + defaultuser + "\n";
			err+= "    ERRNO: " + *strerror(errno);
			LOG(LOG_ERROR, err);
			cerr << err << endl;
			if(setuid(users[defaultuser]) != 0)
			{
				err=  "### ERROR: cannot set process to default user " + defaultuser + "\n";
				err+= "    ERRNO: " + *strerror(errno);
				LOG(LOG_ALERT, err);
				cerr << err << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	err= pQuestions->run();
	owserver->stop();
	DbInterface::deleteAll();
	if(err > 0)
	{
		string msg;

		msg=  "### ERROR: cannot start question thread for one wire server " + owserver->getServerName() + "\n";
		msg+= "           " + pQuestions->strerror(err);
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		pQuestions= auto_ptr<OwServerQuestions>();// delete OwServerQuestions before OWServer
		return EXIT_FAILURE;
	}

	pQuestions= auto_ptr<OwServerQuestions>();// delete OwServerQuestions before OWServer
	glob::stopMessage("### ending correctly ppi-owreader process for library '" + servertype + "'", /*all process names*/true);
	return EXIT_SUCCESS;
}
