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
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <lirc/lirc_client.h>

#include <algorithm>
#include <list>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>


#include "pattern/util/LogHolderPattern.h"

#include "util/debug.h"
#include "util/GlobalStaticMethods.h"
#include "util/smart_ptr.h"
#include "util/URL.h"
#include "util/thread/Terminal.h"
#include "util/usermanagement.h"
#include "util/process/ProcessStarter.h"
#include "util/properties/interlacedactionproperties.h"

#include "database/lib/DbInterface.h"

#include "portserver/owserver.h"
#include "portserver/ExternPorts.h"
#include "portserver/maximchipaccess.h"
#include "portserver/VellemannK8055.h"

#include "ports/portbaseclass.h"
#include "ports/measureThread.h"
#include "ports/portbaseclass.h"
#include "ports/timemeasure.h"
#include "ports/switch.h"
#include "ports/output.h"
#include "ports/resistancemeasure.h"
#include "ports/tempmeasure.h"
#include "ports/counter.h"
#include "ports/measuredness.h"
#include "ports/timer.h"
#include "ports/shell.h"
#include "ports/valueholder.h"
#include "ports/Set.h"
#include "ports/SaveSubValue.h"
#include "ports/ExternPort.h"
#include "ports/LircPort.h"

#include "server/libs/server/Communication.h"
#include "server/libs/server/communicationthreadstarter.h"
#include "server/libs/server/TcpServerConnection.h"
//#include "server/libs/server/ServerMethodTransaction.h"

#include "server/ServerTransaction.h"
//#include "server/ClientTransaction.h"

#include "starter.h"
#include "ProcessChecker.h"

using namespace boost;
using namespace boost::algorithm;
using namespace ppi_database;
using namespace util;
using namespace server;
using namespace user;
using namespace std;
using namespace logger;


bool Starter::execute(const IOptionStructPattern* commands)
{
	bool bDb, bPorts, bInternet;
	int err;
	unsigned short nDbConnectors;
	vector<pair<string, PortTypes> > ports; // whitch ports as string are needet. Second pair object bool is whether the port is defined for pin reading with ioperm()
	string fileName;
	string logpath, sLogLevel, property;
	DbInterface *db;
	string prop;
	auto_ptr<ProcessStarter> process, logprocess;
	vector<string>::size_type readercount;
	set<string> shellstarter;

	// starting time
	struct tm ttime;
	timeval startingtime, acttime;
	string stimemsg;
	ostringstream timemsg;

	gettimeofday(&startingtime, NULL);
	Starter::isNoPathDefinedStop();
	glob::processName("ppi-server");
	glob::setSignals("ppi-server");


	if(commands->hasOption("configure"))
		cout << "   read configuration files ..." << endl;
	m_sConfPath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(m_sConfPath, "server.conf");
	m_oServerFileCasher.setDelimiter("owreader", "[", "]");
	m_oServerFileCasher.modifier("owreader");
	m_oServerFileCasher.readLine("workdir= " + m_sWorkdir);
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	// look for defined owreader in server.conf configuration file
	readercount= m_oServerFileCasher.getPropertyCount("owreader");
	for(vector<string>::size_type owreaders= 0; owreaders < readercount; ++owreaders)
	{
		string owreader(m_oServerFileCasher.getValue("owreader", owreaders));

		m_vOWReaderTypes.insert(owreader);
		if(owreader == "PORT")
			m_vOWReaderTypes.insert("MPORT");
	}
	readFile(ports, URL::addPath(m_sConfPath, "measure.conf"));

	// check whether should be start which server
	property= m_oServerFileCasher.getValue("databaseserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter databaseserver not be set correctly, so start server" << endl;
		bDb= true;
	}else if(property == "true")
		bDb= true;
	else
		bDb= false;
	property= m_oServerFileCasher.getValue("owreaders", /*warning*/true);
	if(	property == "" ||
		(	property != "true" &&
			property != "false"		)	)
	{
		cerr << "###          parameter owreaders not be set correctly, so start server" << endl;
		bPorts= true;
	}else if(property == "true")
		bPorts= true;
	else
		bPorts= false;
	property= m_oServerFileCasher.getValue("internetserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter internetserver not be set correctly, so start server" << endl;
		bInternet= true;
	}else if(property == "true")
		bInternet= true;
	else
		bInternet= false;


	readPasswd();

	string commhost;
	unsigned short commport;
	int nLogAllSec;


	commhost= m_oServerFileCasher.getValue("communicationhost", /*warning*/false);
	if(commhost == "")
		commhost= "127.0.0.1";
	property= "communicationport";
	commport= m_oServerFileCasher.needUShort(property);
	if(	commport == 0
		&&
		property == "#ERROR"	)
	{
		exit(EXIT_FAILURE);
	}

	property= "timelogSec";
	nLogAllSec= m_oServerFileCasher.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}

	// ------------------------------------------------------------------------------------------------------------


	if(commands->hasOption("configure"))
		cout << "   create folder lists ..." << endl;
	createFolderLists(shellstarter);

	//*********************************************************************************
	//* calculate how much communication threads should be running
	bool bPort= false;
	bool bMPort= false;
	bool blirc= false;
	bool bowfs= false;
	bool bvk8055= false;
	unsigned short nOWReaderCount;
	unsigned short nOWReader(0);
	const IPropertyPattern* pProp;

	// count of all OWReaders should starting in an own user account of system
	// which are get some commands from subroutine with type SHELL and defined runuser
	nOWReader= static_cast<unsigned short>(shellstarter.size());

	// count in nOWReader how much one wire reader (OWServer) should running
	if(m_vOWReaderNeed.find("OWFS") != m_vOWReaderNeed.end())
	{
		pProp= m_oServerFileCasher.getSection("owreader", "OWFS");
		if(pProp)
		{
			property= "maximinit";
			nOWReaderCount=  static_cast<unsigned short>(pProp->getPropertyCount(property));
			if(nOWReaderCount > 0)
				bowfs= true;
			nOWReader+= nOWReaderCount;
		}
	}
	if(m_vOWReaderNeed.find("Vk8055") != m_vOWReaderNeed.end())
	{
		pProp= m_oServerFileCasher.getSection("owreader", "Vk8055");
		if(pProp)
		{
			property= "port";
			nOWReaderCount= static_cast<unsigned short>(pProp->getPropertyCount(property));
			if(nOWReaderCount > 0)
				bvk8055= true;
			nOWReader+= nOWReaderCount;
		}
	}
	if(m_vOWReaderNeed.find("LIRC") != m_vOWReaderNeed.end())
	{
		pProp= m_oServerFileCasher.getSection("owreader", "LIRC");
		if(pProp)
		{
			char type[8];
			//*********************************************************************************
			//* check whether any lirc is configured
			strncpy(type, "irexec", 6);
			if(lirc_init(type, 1) != -1)
			{
				//struct lirc_config *ptLircConfig;

				blirc= true;
				/*if(lirc_readconfig(NULL, &ptLircConfig, NULL) == 0)
				{
					blirc= true;
					lirc_freeconfig(ptLircConfig);
				}*/
				lirc_deinit();
			}
			if(blirc)
				++nOWReader;
		}
	}
	for(vector<pair<string, PortTypes> >::iterator it= ports.begin(); it != ports.end(); ++it)
	{
		if(	!bPort && it->second == PORT	)
		{
			++nOWReader;
			bPort= true;

		}else if(	!bMPort && it->second == MPORT && it->first != "freeze"	)
		{
			++nOWReader;
			bPort= true;

		}else if(it->second == RWPORT)
			++nOWReader;
	}

	//*	for all processes without db-server and internet-server to give answers
	//		ppi-server(ProcessChecker), ppi-owreader
	nDbConnectors= 1 + nOWReader;

	//*		for all process an db client but the internet-server needs an second for callback routines (second connection by client)
	//			ppi-server, ppi-internet-server, ppi-owreader
	nDbConnectors+= 1 + 2 + nOWReader;

	//*		for all one wire server (ppi-owreader) are needs the polling action list in ppi-server an OWInterface
	nDbConnectors+= nOWReader;

#ifdef ALLOCATEONMETHODSERVER
	short spaces= 14;
	ostringstream conns;

	conns << nDbConnectors;
	cout << " ******************************************************************" << endl;
	cout << " ***                                                            ***" << endl;
	cout << " ***   for database server are "
						   << conns.str() << " clients configured";
	for(short c= 0; c < (spaces - (short)conns.str().size()); ++c)
		cout << " ";
	cout <<                                                                 "***" << endl;
	cout << " ***   now also 2 extra for testing                             ***" << endl;
	cout << " ***   whether really this count be needed                      ***" << endl;
	cout << " ***                                                            ***" << endl;
	cout << " ******************************************************************" << endl;
	// only for debugging to know whether the allocated connections to db are ok
	nDbConnectors+= 2;
#endif // ALLOCATEONMETHODSERVER
	//*********************************************************************************


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start database server

	cout << "### start ppi db server" << endl;
	process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", "ppi-db-server",
															new SocketClientConnection(	SOCK_STREAM,
																						commhost,
																						commport,
																						10			)	));

	if(bDb)
	{
		ostringstream oDbConnectors;

		oDbConnectors << nDbConnectors;
		err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-db-server").c_str(), oDbConnectors.str().c_str(), NULL);
	}else
		err= process->check();
	if(err > 0)
	{
		string msg;

		msg=  "### WARNING: cannot start database-server\n";
		msg+= "             " + process->strerror(err) + "\n";
		msg+= "             so the hole application is not useable stop server";
		cerr << msg << endl;
		exit(EXIT_FAILURE);
	}
	// ------------------------------------------------------------------------------------------------------------

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start database interface

	DbInterface::initial(	"ppi-server",
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														5			),
							/*identif log*/nLogAllSec					);

	DbInterface::instance()->setThreadName(glob::getProcessName());
	// ------------------------------------------------------------------------------------------------------------

	LOG(LOG_INFO, "Read configuration files from " + m_sConfPath);

	/***********************************************************************************\
	 *
	 * define one wire server
	 *
	 */
	int nServerID= 1;
	string owreader("OwServerQuestion-");

	/***************************************************
	 * starting owreader for shell commands
	 * in other user account of system
	 */
	for(set<string>::iterator it= shellstarter.begin(); it != shellstarter.end(); ++it)
	{
		ostringstream oServerID;

		cout << "### starting OWServer " << endl;
		oServerID << nServerID;
		process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																new SocketClientConnection(	SOCK_STREAM,
																							commhost,
																							commport,
																							10			)	));

		if(bPorts)
		{
			ostringstream oDbConnectors;

			oDbConnectors << nDbConnectors;
			err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
												"SHELL", it->c_str(), NULL);
		}else
			err= process->check();
		if(err > 0)
		{
			string msg;

			msg=  "### WARNING: cannot start one wire reader\n";
			msg+= "             " + process->strerror(err) + "\n";
			msg+= "             so ppi-server cannot start any shell commands in system user account of '" + *it + "'";
			cerr << endl << msg << endl;
			LOG(LOG_ALERT, msg);
		}else
		{// create reading interface to one wire reader
			OWInterface::getServer(	"ppi-server",
									new SocketClientConnection(	SOCK_STREAM,
																commhost,
																commport,
																5			),
									nServerID									);
			++nServerID;
		}
	}

	/***************************************************
	 * starting owreaders for external COM/LPT ports
	 */
	if(ports.size() > 0)
	{
		string sPorts;

		// starting OWServer for write or read pins on any ports
		for(vector<pair<string, PortTypes> >::iterator it= ports.begin(); it != ports.end(); ++it)
		{
			if(it->second == PORT)
				sPorts+= it->first + " ";
		}
		if(sPorts != "")
		{
			ostringstream oServerID;

			cout << "### starting OWServer " << endl;
			oServerID << nServerID;
			process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																	new SocketClientConnection(	SOCK_STREAM,
																								commhost,
																								commport,
																								10			)	));

			if(bPorts)
			{
				ostringstream oDbConnectors;

				oDbConnectors << nDbConnectors;
				err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
													"PORT", sPorts.c_str(), NULL);
			}else
				err= process->check();
			if(err > 0)
			{
				string msg;

				msg=  "### WARNING: cannot start one wire reader\n";
				msg+= "             " + process->strerror(err) + "\n";
				msg+= "             so ppi-server cannot read ore write on any extern COM/LPT port";
				cerr << endl << msg << endl;
				LOG(LOG_ALERT, msg);
			}else
			{// create reading interface to one wire reader
				OWInterface::getServer(	"ppi-server",
										new SocketClientConnection(	SOCK_STREAM,
																	commhost,
																	commport,
																	5			),
										nServerID									);
				++nServerID;
			}
		}

		// starting OWServer to measure on ports
		sPorts= "";
		for(vector<pair<string, PortTypes> >::iterator it= ports.begin(); it != ports.end(); ++it)
		{
			if(it->second == MPORT)
				sPorts+= it->first + "";
		}
		if(sPorts != "")
		{
			ostringstream oServerID;

			cout << "### starting OWServer " << endl;
			oServerID << nServerID;
			process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																	new SocketClientConnection(	SOCK_STREAM,
																								commhost,
																								commport,
																								10			)	));

			if(bPorts)
			{
				ostringstream oDbConnectors;

				oDbConnectors << nDbConnectors;
				err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
													"MPORT", sPorts.c_str(), NULL);
			}else
				err= process->check();
			if(err > 0)
			{
				string msg;

				msg=  "### WARNING: cannot start one wire reader\n";
				msg+= "             " + process->strerror(err) + "\n";
				msg+= "             so ppi-server cannot measure time on any extern COM/LPT port";
				cerr << endl << msg << endl;
				LOG(LOG_ALERT, msg);
			}else
			{// create reading interface to one wire reader
				OWInterface::getServer(	"ppi-server",
										new SocketClientConnection(	SOCK_STREAM,
																	commhost,
																	commport,
																	5			),
										nServerID									);
				++nServerID;
			}
		}

	}

	/***************************************************
	 * starting owreader for lirc receiver or transmitter
	 */
	if(blirc)
	{
		ostringstream oServerID;

		cout << "### starting OWServer " << endl;
		oServerID << nServerID;
		process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																new SocketClientConnection(	SOCK_STREAM,
																							commhost,
																							commport,
																							10			)	));

		if(bPorts)
		{
			ostringstream oDbConnectors;

			oDbConnectors << nDbConnectors;
			err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
												"LIRC", NULL);
		}else
			err= process->check();
		if(err > 0)
		{
			string msg;

			msg=  "### WARNING: cannot start one wire reader\n";
			msg+= "             " + process->strerror(err) + "\n";
			msg+= "             so ppi-server cannot read or write on LIRC";
			cerr << endl << msg << endl;
			LOG(LOG_ALERT, msg);
		}else
		{// create reading interface to one wire reader
			OWInterface::getServer(	"ppi-server",
									new SocketClientConnection(	SOCK_STREAM,
																commhost,
																commport,
																5			),
									nServerID									);
			++nServerID;
		}
	}

#ifdef _EXTERNVENDORLIBRARYS
	string maximinit("");
	vector<string>::size_type nVk8055Count= 0;
#endif // _EXTERNVENDORLIBRARYS

#ifdef _K8055LIBRARY
	/***************************************************
	 * starting owreader for vellemann K8055 board
	 */
	if(bvk8055)
	{
		bool bError;
		int nVk8055Address;
		vector<int> vVk8055;
		vector<int>::iterator vit;

		// start Vellemann k8055 ports
		pProp= m_oServerFileCasher.getSection("owreader", "Vk8055");
		nVk8055Count= pProp->getPropertyCount("port");
		for(vector<string>::size_type n= 0; n < nVk8055Count; ++n)
		{
			bError= false;
			prop= "port";
			nVk8055Address= pProp->getInt(prop, n, /*warning*/false);

			if(	prop == "#ERROR"
				||
				nVk8055Address < 0
				||
				nVk8055Address > 3	)
			{
				bError= true;
			}else
			{
				vit= ::find(vVk8055.begin(), vVk8055.end(), nVk8055Address);
				if(vit == vVk8055.end())
				{
					ostringstream oServerID;
					ostringstream oVK8055Address;

					cout << "### starting OWServer" << endl;
					vVk8055.push_back(nVk8055Address);
					oVK8055Address << nVk8055Address;
					oServerID << nServerID;
					process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																			new SocketClientConnection(	SOCK_STREAM,
																										commhost,
																										commport,
																										10			)	));

					if(bPorts)
					{
						ostringstream oDbConnectors;

						oDbConnectors << nDbConnectors;
						err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
															"vellemann", "k8055", oVK8055Address.str().c_str(), NULL);
					}else
						err= process->check();
					if(err > 0)
					{
						string msg;

						msg=  "### WARNING: cannot start one wire reader\n";
						msg+= "             " + process->strerror(err) + "\n";
						msg+= "             so ppi-server cannot read or write on Vellemann k8055 board";
						cerr << endl << msg << endl;
						LOG(LOG_ALERT, msg);
					}else
					{// create reading interface to one wire reader
						OWInterface::getServer(	"ppi-server",
												new SocketClientConnection(	SOCK_STREAM,
																			commhost,
																			commport,
																			5			),
												nServerID									);
						++nServerID;
					}
				}
			}
			if(bError)
			{
				string sVk8055Address;
				ostringstream msg;

				sVk8055Address= pProp->getValue("port", n, false);
				msg << "    ERROR by define " << (n+1) << ". port with address " << sVk8055Address << endl;
				msg << "          so ppi-server cannot read or write on Vellemann k8055 board";
				cerr << msg.str() << endl;
				LOG(LOG_ALERT, msg.str());
			}
			cout << endl;
		}
	}
#endif //_K8055LIBRARY

#ifdef _OWFSLIBRARY
	/***************************************************
	 * starting owreader for maxim/dallas semiconductors
	 */
	if(bowfs)
	{
		vector<string> adapters;
		vector<string> maximconf, conf;
		vector<string>::iterator first;
		vector<string>::size_type nMaximCount;

		// read first all maxim adapters
		pProp= m_oServerFileCasher.getSection("owreader", "OWFS");
		nMaximCount= pProp->getPropertyCount("maximadapter");
		for(vector<string>::size_type n= 0; n < nMaximCount; ++n)
		{
			maximinit= pProp->getValue("maximadapter", n, /*warning*/false);
			adapters.push_back(maximinit);
		}
		// start maxim ports with owfs driver
		nMaximCount= pProp->getPropertyCount("maximinit");
		for(vector<string>::size_type n= 0; n < nMaximCount; ++n)
		{
			ostringstream oServerID;

			maximinit= pProp->getValue("maximinit", n, /*warning*/false);
			for(vector<string>::iterator ad= adapters.begin(); ad != adapters.end(); ++ad)
			{
				maximinit+= ":";
				maximinit+= *ad;
			}
			cout << "### starting OWServer" << endl;
			oServerID << nServerID;
			process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", owreader + oServerID.str(),
																	new SocketClientConnection(	SOCK_STREAM,
																								commhost,
																								commport,
																								10			)	));

			if(bPorts)
			{
				ostringstream oDbConnectors;

				oDbConnectors << nDbConnectors;
				err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-owreader").c_str(), oServerID.str().c_str(),
													"maxim", maximinit.c_str(), NULL);
			}else
				err= process->check();
			if(err > 0)
			{
				string msg;

				msg=  "### WARNING: cannot start one wire reader\n";
				msg+= "             " + process->strerror(err) + "\n";
				msg+= "             so ppi-server cannot read or write on Maxim/Dallas semiconductors";
				cerr << endl << msg << endl;
				LOG(LOG_ALERT, msg);
			}else
			{// create reading interface to one wire reader
				OWInterface::getServer(	"ppi-server",
										new SocketClientConnection(	SOCK_STREAM,
																	commhost,
																	commport,
																	5			),
										nServerID									);
				++nServerID;
			}
			cout << endl;
		}
	}
#endif //_OWFSLIBRARY

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// check whether database loading is finished

	db= DbInterface::instance();
	if(!db->isDbLoaded())
	{
		cout << "### database is busy" << endl;
		cout << "    wait for initialing " << flush;
		while(!db->isDbLoaded())
		{
			sleep(1);
			cout << "." << flush;
		}
		cout << " OK" << endl << endl;
	}
	// ------------------------------------------------------------------------------------------------------------

	LOG(LOG_DEBUG, "after knowing database was loaded, starting to configure all control list's to measure");
	configurePortObjects(commands->hasOption("configure"));
	TERMINALEND;

	--nServerID;
	OWInterface::checkUnused(nServerID);
	OWInterface::endOfInitialisation(nServerID, commands->hasOption("firstvalue"));

	//checkAfterContact();

	bool createThread= false;
	bool bStarting(commands->hasOption("folderstart"));
	MeasureArgArray args;
	SHAREDPTR::shared_ptr<meash_t> pFirstMeasureThreads;
	SHAREDPTR::shared_ptr<meash_t> pCurrentMeasure;

	meash_t::clientPath= URL::addPath(m_sWorkdir, PPICLIENTPATH, /*always*/false);
	LOG(LOG_INFO, "Read layout content for clients from " + meash_t::clientPath);
	SHAREDPTR::shared_ptr<measurefolder_t> aktFolder= m_tFolderStart;
	args.ports= ports;
	cout << endl;
	if(bStarting)
		cout << "### start folder thread(s) from measure.conf" << endl;
	while(aktFolder != NULL)
	{
		if(!aktFolder->bCorrect)
		{
			string msg("### WARNING: folder '");

			msg+= aktFolder->name;
			msg+= "' has no correct subroutine\n";
			msg+= "             so make no measure-instance for it";
			LOG(LOG_WARNING, msg);
			cout << msg << endl;
		}else
		{
			if(bStarting)
				cout << "                     " << aktFolder->name << endl;
			createThread= true;
			if(pFirstMeasureThreads == NULL)
			{
				pFirstMeasureThreads= SHAREDPTR::shared_ptr<meash_t>(new meash_t);
				pCurrentMeasure= pFirstMeasureThreads;
				meash_t::firstInstance= pCurrentMeasure;
			}else
			{
				pCurrentMeasure->next= SHAREDPTR::shared_ptr<meash_t>(new meash_t);
				pCurrentMeasure= pCurrentMeasure->next;
			}
			pCurrentMeasure->pMeasure = SHAREDPTR::shared_ptr<MeasureThread>(new MeasureThread(aktFolder->name));
			args.subroutines= &aktFolder->subroutines;
			pCurrentMeasure->pMeasure->start(&args);
		}
		aktFolder= aktFolder->next;
	}
	if(bStarting)
		cout << endl;
	if(!createThread)
	{

		LOG(LOG_ALERT, "### start no measure-thread for any folder\n    no correct subroutines is founding\n    does not start server");
		// toDo: sending stop to communication server
		cout << "### start no measure-thread for any folder" << endl;
		cout << "    no correct subroutines is founding" << endl;
		cout << "    does not start server" << endl;
		exit(EXIT_FAILURE);
	}

	string host;
	unsigned short port;

	host= m_oServerFileCasher.getValue("listen", /*warning*/false);
	property= "port";
	port= m_oServerFileCasher.needUShort(property);
	if(	port == 0
		&&
		property == "#ERROR"	)
	{
		exit(EXIT_FAILURE);
	}

	// after creating all threads and objects
	// all chips should be defined in DefaultChipConfigReader
	db->chipsDefined(true);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start internet server

	cout << "### start ppi internet server" << endl;
	process= auto_ptr<ProcessStarter>(new ProcessStarter(	"ppi-starter", "ppi-internet-server",
															new SocketClientConnection(	SOCK_STREAM,
																						host,
																						port,
																						10			)	));
	if(commands->hasOption("configure"))
		cout << "  ... read configuration files" << endl;
	process->openendSendConnection("GET wait", "ending");

	if(bInternet)
		err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-internet-server").c_str(), NULL);
	else
		err= process->check();
	if(err > 0)
	{
		string msg;

		msg=  "### WARNING: cannot start internet server for communication\n";
		msg+= "             " + process->strerror(err) + "\n";
		msg+= "             so no communication from outside (any client) is available";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
	}

	logprocess= auto_ptr<ProcessStarter>();
	process= auto_ptr<ProcessStarter>();
	// ------------------------------------------------------------------------------------------------------------

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set process id to default user

	setuid(m_tDefaultUser);
	// ------------------------------------------------------------------------------------------------------------

	// start ProcessChecker
	ProcessChecker checker(	new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														5				),
							new SocketClientConnection(	SOCK_STREAM,
														commhost,
														commport,
														5				),
							nServerID										);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// calculate starting time
	timemsg << "    server was starting in ";
	gettimeofday(&acttime, NULL);
	acttime.tv_sec-= startingtime.tv_sec;
	if(acttime.tv_usec < startingtime.tv_usec)
	{
		acttime.tv_sec-= 1;
		acttime.tv_usec+= 1000000;
	}
	ttime= *localtime(&acttime.tv_sec);
	acttime.tv_usec-= startingtime.tv_usec;
	if(ttime.tm_min)
	{
		timemsg << ttime.tm_min << " minute";
		if(ttime.tm_min > 1)
			timemsg << "s";
		timemsg << " and ";
	}
	timemsg << ttime.tm_sec << "." << acttime.tv_usec << " seconds";
	stimemsg= timemsg.str();
	cout << stimemsg << " ..." << endl;
	LOG(LOG_INFO, stimemsg);
	// ------------------------------------------------------------------------------------------------------------
	checker.start(pFirstMeasureThreads.get(), true);

	OWInterface::deleteAll();
	Thread::applicationStops();
	// ending process ppi-server
	SHAREDPTR::shared_ptr<meash_t> delMeash;
	//system("ps -eLf | grep ppi-server");
	pCurrentMeasure= pFirstMeasureThreads;
	if(!pCurrentMeasure)
		cout << "first measure thread is null" << endl;
	while(pCurrentMeasure)
	{
		delMeash= pCurrentMeasure;
		pCurrentMeasure= pCurrentMeasure->next;
		if(delMeash->pMeasure->running())
			delMeash->pMeasure->stop(true);

	}
	pFirstMeasureThreads= SHAREDPTR::shared_ptr<meash_t>();
	checker.stop(true);
	DbInterface::deleteAll();
	return true;
}

void Starter::createFolderLists(set<string>& shellstarter)
{
	SHAREDPTR::shared_ptr<measurefolder_t> aktualFolder= m_tFolderStart;
	//DbInterface* db= DbInterface::instance();

	while(aktualFolder != NULL)
	{
		int nMuch= aktualFolder->subroutines.size();

		//cout << "  create folder: '" << aktualFolder->name << "'" << endl;
		for(int n= 0; n<nMuch; n++)
		{
			//cout << "    subroutine: " << aktualFolder->subroutines[n].name;
			//cout << " with type " << aktualFolder->subroutines[n].type << endl;
			if(aktualFolder->subroutines[n].type == "SWITCH")
			{
				SHAREDPTR::shared_ptr<switchClass> obj;

				obj= SHAREDPTR::shared_ptr<switchClass>(new switchClass(aktualFolder->name,
																		aktualFolder->subroutines[n].name));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "DEBUG")
			{
				SHAREDPTR::shared_ptr<switchClass> obj;

				obj= SHAREDPTR::shared_ptr<Output>(new Output(	aktualFolder->name,
																aktualFolder->subroutines[n].name));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "TIMER")
			{
				auto_ptr<timer> obj= auto_ptr<timer>(new timer(	aktualFolder->name,
																aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "SHELL")
			{
				string user;
				auto_ptr<Shell> obj= auto_ptr<Shell>(new Shell(	aktualFolder->name,
																aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;
				user= aktualFolder->subroutines[n].property->getValue("runuser", /*warning*/false);
				if(user != "")
					shellstarter.insert(user);

			}else if(aktualFolder->subroutines[n].type == "TIMEMEASURE")
			{
				auto_ptr<TimeMeasure> obj= auto_ptr<TimeMeasure>(new TimeMeasure(	aktualFolder->name,
																					aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "RESISTANCE")
			{
				auto_ptr<ResistanceMeasure> obj= auto_ptr<ResistanceMeasure>(new ResistanceMeasure(	aktualFolder->name,
																									aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "TEMP")
			{
				auto_ptr<TempMeasure> obj= auto_ptr<TempMeasure>(new TempMeasure(	aktualFolder->name,
																					aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "VALUE")
			{
				auto_ptr<ValueHolder> obj= auto_ptr<ValueHolder>(new ValueHolder(	aktualFolder->name,
																					aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "SET")
			{
				auto_ptr<Set> obj= auto_ptr<Set>(new Set(	aktualFolder->name,
															aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "SAVE")
			{
				auto_ptr<SaveSubValue> obj= auto_ptr<SaveSubValue>(new SaveSubValue(aktualFolder->name,
																					aktualFolder->subroutines[n].name));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "COUNTER")
			{
				auto_ptr<Counter> obj= auto_ptr<Counter>(new Counter(	aktualFolder->name,
																		aktualFolder->subroutines[n].name	));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(aktualFolder->subroutines[n].type == "MEASUREDNESS")
			{
				auto_ptr<Measuredness> obj= auto_ptr<Measuredness>(new Measuredness(aktualFolder->name,
																					aktualFolder->subroutines[n].name));
				aktualFolder->subroutines[n].portClass= obj;

			}else if(::find(m_vOWReaderTypes.begin(), m_vOWReaderTypes.end(), aktualFolder->subroutines[n].type) != m_vOWReaderTypes.end())
			{// type is reached over an OWServer instance
				auto_ptr<ExternPort> obj;

				//cout << "subroutine " << aktualFolder->subroutines[n].name << " from type " << aktualFolder->subroutines[n].type << endl;
				if(aktualFolder->subroutines[n].type == "LIRC")
				{
					obj= auto_ptr<ExternPort>(new LircPort(	aktualFolder->subroutines[n].type,
															aktualFolder->name,
															aktualFolder->subroutines[n].name	));
				}else
				{
					obj= auto_ptr<ExternPort>(new ExternPort(	aktualFolder->subroutines[n].type,
																aktualFolder->name,
																aktualFolder->subroutines[n].name	));
				}
				aktualFolder->subroutines[n].portClass= obj;
				m_vOWReaderNeed.insert(aktualFolder->subroutines[n].type);

			}else
			{
				ostringstream msg;

				if(aktualFolder->subroutines[n].type == "")
				{
					msg << "### WARNING: in folder '" << aktualFolder->name << "' and subroutine '" << aktualFolder->subroutines[n].name << "'" << endl;
					msg << "             is no type specified, define subroutine as incorrect!";

				}else
				{
					msg << aktualFolder->subroutines[n].property->getMsgHead(/*error*/false);
					msg << "cannot define given type '" << aktualFolder->subroutines[n].type << "', define subroutine as incorrect!";
				}
				cout << msg.str() << endl;
				LOG(LOG_WARNING, msg.str());
			}

		}
		aktualFolder= aktualFolder->next;
	}
}

void Starter::configurePortObjects(bool bShowConf)
{
	bool bNewMeasure(false);
	SHAREDPTR::shared_ptr<measurefolder_t> aktualFolder= m_tFolderStart;
	//DbInterface* db= DbInterface::instance();
	string property("defaultSleep");
	string sMeasureFile;
	unsigned short nDefaultSleep= m_oServerFileCasher.getUShort(property, /*warning*/false);

	if(property == "#ERROR")
		nDefaultSleep= 2;
	sMeasureFile= URL::addPath(m_sConfPath, "measure.conf");
	while(aktualFolder != NULL)
	{
		bool correctFolder= true;
		int nMuch= aktualFolder->subroutines.size();

		if(bShowConf)
		{
			TERMINALEND;
			cout << " configure folder: '" << aktualFolder->name << "'" << endl;
		}
		for(int n= 0; n<nMuch; n++)
		{
			//cout << "    subroutine: " << aktualFolder->subroutines[n].name;
			//cout << " with type " << aktualFolder->subroutines[n].type << endl;
			SHAREDPTR::shared_ptr<IListObjectPattern> obj(aktualFolder->subroutines[n].portClass);
			portBase* pobj;
			vector<ohm> *pvOhm= &aktualFolder->subroutines[n].resistor;
			vector<correction_t> *pvCorrection= &aktualFolder->subroutines[n].correction;
			short measuredness= aktualFolder->subroutines[n].measuredness;

			if(measuredness == 0)
				measuredness= m_nMeasuredness;
			if(pvOhm->size() == 0)
				pvOhm= &m_vOhm;
			if(pvCorrection->size() == 0)
				pvCorrection= &m_vCorrection;

			pobj= dynamic_cast<portBase*>(obj.get());
			if(pobj)
			{
				//tout << pobj->getFolderName() << ":" << pobj->getSubroutineName() << endl;
				if(pobj->init(aktualFolder->subroutines[n].property.get(), m_tFolderStart))
					aktualFolder->subroutines[n].bCorrect= true;
				else
					correctFolder= false;
				//tout << "-----------" << endl;
			}else
			{
				string alert("inside folder ");

				correctFolder= false;
				alert+= aktualFolder->name + " witch subroutine ";
				alert+= aktualFolder->subroutines[n].name;
				alert+= " from type " + aktualFolder->subroutines[n].type + "\n";
				alert+= "can not cast object IListObjectPattern to object portBase\n";
				alert+= "this is an programming FAULT, maype the ";
				alert+= aktualFolder->subroutines[n].type + "-object don't depence from portBase?";
				LOG(LOG_ALERT, alert);
				cerr << "### ALLERT ERROR: " << alert << endl << endl;
			}
			if(!aktualFolder->subroutines[n].bCorrect)
				aktualFolder->subroutines[n].portClass= SHAREDPTR::shared_ptr<IListObjectPattern>();

			if(aktualFolder->subroutines[n].bCorrect)
			{
				if(dynamic_cast<TimeMeasure*>(aktualFolder->subroutines[n].portClass.get()))
				{
					bool bFillMikro= false;
					bool bFillCorr= false;
					unsigned int nContent= aktualFolder->subroutines[n].resistor.size();
					unsigned int nCorrection= aktualFolder->subroutines[n].correction.size();
					unsigned long time;
					TimeMeasure* port;

					port= dynamic_cast<TimeMeasure*>(aktualFolder->subroutines[n].portClass.get());
					if(aktualFolder->subroutines[n].measuredness == -1)
					{
						bNewMeasure= true;
						measuredness= port->setNewMeasuredness(m_nMeasurednessCount, nDefaultSleep);
						aktualFolder->subroutines[n].measuredness= measuredness;
						//cout << "set measuredness to " << measuredness << endl;

						//readFile(sMeasureFile, aktualFolder->subroutines[n].name, "measuredness", &measuredness);
					}
					unsigned int nCorr;
					for(nCorr= 0; nCorr<nCorrection; ++nCorr)
					{
						if(!aktualFolder->subroutines[n].correction[nCorr].bSetTime)
						{
							bFillCorr= true;
							break;
						}
					}
					if(bFillCorr)
					{
						correction_t correction= port->getNewCorrection(aktualFolder->subroutines[n].correction[nCorr],
																		*pvOhm, nDefaultSleep);

						bNewMeasure= true;
						//readFile(sMeasureFile, aktualFolder->subroutines[n].name, "correction", &correction);
						cout << endl;
						cout << "### ending on write mikroseconds from resistance in server.conf" << endl << endl;
					}
					for(unsigned int i= 0; i<nContent; ++i)
					{
						if(!aktualFolder->subroutines[n].resistor[i].bSetTime)
						{
							bFillMikro= true;
							break;
						}
					}
					if(bFillMikro)
					{
						bNewMeasure= true;
						time= port->getNewMikroseconds(&aktualFolder->subroutines[n].resistor);
						//readFile(sMeasureFile, aktualFolder->subroutines[n].name, "OHM", &time);
						cout << endl;
						cout << "### ending on write mikroseconds from resistance in server.conf" << endl << endl;
					}
				}
			}

			if(aktualFolder->subroutines[n].property)
				aktualFolder->subroutines[n].property->checkProperties();
			if(	(	obj->needServer() &&		// if subroutine need an external Server
					obj->hasServer()	) ||	// but has actually no access to them
				!obj->needServer()			)	// subroutine check all seconds for access
												// and subroutine should starting
			{
				if(!aktualFolder->subroutines[n].bCorrect)
					tout << "             SUBROUTINE do not running inside folder" << endl << endl;
			}
		}
		aktualFolder->bCorrect= correctFolder;
		aktualFolder= aktualFolder->next;
	}
	TERMINALEND;
	if(bShowConf)
		cout << endl << endl;
}

Starter::~Starter()
{
}

void Starter::readPasswd()
{
	map<string, uid_t> users;
	string defaultUser(m_oServerFileCasher.needValue("defaultuser"));
	string passwd(m_oServerFileCasher.needValue("passwd"));
	size_t defaultLen= defaultUser.length();

	if(defaultLen == 0)
	{
		cout << "### ERROR: no default user (defaultuser) in server.conf be set" << endl;
		exit(EXIT_FAILURE);
	}
	users[defaultUser]= 0;
	if(!glob::readPasswd(passwd, users))
		exit(EXIT_FAILURE);
	m_tDefaultUser= users[defaultUser];
}

inline vector<pair<string, PortTypes> >::iterator Starter::find(vector<pair<string, PortTypes> >& vec, string port)
{
	vector<pair<string, PortTypes> >::iterator portIt;

	for(portIt = vec.begin(); portIt != vec.end(); ++portIt)
	{
		if(portIt->first == port)
			break;
	}
	return portIt;
}

void Starter::readFile(vector<pair<string, PortTypes> > &vlRv, string fileName)
{
	typedef vector<IInterlacedPropertyPattern*>::iterator secIt;

	short nFolderID= 0;
	string modifier, value;
	auto_ptr<sub> subdir;
	ActionProperties *pProperty;
	InterlacedActionProperties mainprop(/*check after*/true);
	vector<IInterlacedPropertyPattern*> folderSections;
	vector<IInterlacedPropertyPattern*> subSections;
	SHAREDPTR::shared_ptr<measurefolder_t> aktualFolder= m_tFolderStart;

	mainprop.action("action");
	mainprop.modifier("folder");
	mainprop.setMsgParameter("folder");
	mainprop.modifier("name");
	mainprop.setMsgParameter("name", "subroutine");
	mainprop.valueLocalization("\"", "\"", /*remove*/true);
	mainprop.readFile(fileName);
	folderSections= mainprop.getSections();
	for(secIt fit= folderSections.begin(); fit != folderSections.end(); ++fit)
	{
		modifier= (*fit)->getSectionModifier();
		if(modifier != "folder")
		{
			ostringstream out;

			out << "### ERROR by reading measure.conf" << endl;
			out << "          wrong defined modifier '" << modifier << "'" << endl;
			out << "          with value '" << (*fit)->getSectionValue() << "'." << endl;
			out << "          (maybe an subroutine is defined outside from an folder)" << endl;
			out << "          Do not create this subroutine for working!";
			cerr << out << endl << endl;
			LOG(LOG_ERROR, out.str());
		}else
		{
			ostringstream sFID;

			// create new folder
			++nFolderID;
			sFID << "_folderID=" << nFolderID;
			value= (*fit)->getSectionValue();
			value= (*fit)->getValue("folder");
			glob::replaceName(value, "folder name");
			//cout << "create folder: " << value << endl;
			if(aktualFolder==NULL)
			{
				aktualFolder= SHAREDPTR::shared_ptr<measurefolder_t>(new measurefolder_t);
				m_tFolderStart= aktualFolder;
				aktualFolder->name= value;
				aktualFolder->bCorrect= false;
			}else
			{
				aktualFolder= m_tFolderStart;
				while(aktualFolder->next != NULL)
				{
					if(aktualFolder->name == value)
						break;
					aktualFolder= aktualFolder->next;
				}
				if(aktualFolder->name == value)
				{
					string warn;

					warn=  "### WARNING: found second folder name '" + value + "'\n";
					warn+= "             write all subroutines into this folder!";
					cout << warn << endl;
					LOG(LOG_WARNING, warn);

				}else
				{
					aktualFolder->next= SHAREDPTR::shared_ptr<measurefolder_t>(new measurefolder_t);
					aktualFolder= aktualFolder->next;
					aktualFolder->name= value;
					aktualFolder->bCorrect= false;
				}
			}
			subSections= (*fit)->getSections();
			for(secIt sit= subSections.begin(); sit != subSections.end(); ++sit)
			{
				modifier= (*sit)->getSectionModifier();
				if(modifier != "name")
				{
					cerr << "### ALERT: modifier '" << modifier << "' defined inside folder" << endl;
					cerr << "           STOP process of ppi-server!" << endl;
					exit(EXIT_FAILURE);
				}else
				{
					bool buse(true);

					// create new subroutine
					value= (*sit)->getSectionValue();
					glob::replaceName(value, "folder '" + aktualFolder->name + "' for subroutine name");
					//cout << "    with subroutine: " << value << endl;
					for(vector<sub>::iterator it= aktualFolder->subroutines.begin(); it != aktualFolder->subroutines.end(); ++it)
					{
						if(it->name == value)
						{
							string err;

							buse= false;
							err=  "### Error: found ambiguous name \"" + value + "\" in folder " + aktualFolder->name + "\n";
							err+= "           Do not create this subroutine for working!";
							cerr << err << endl;
							LOG(LOG_ERROR, err);
							break;
						}
					}
					if(buse)
					{
						subdir= auto_ptr<sub>(new sub);
						/************************************************************\
						 * fill into vlRv vector witch COM or LPT ports are needed
						 * when type of subroutine was PORT, MPORT or RWPORT
						 * to start
						\************************************************************/
						subdir->type= (*sit)->needValue("type");
						if(	subdir->type == "PORT" ||
							subdir->type == "MPORT" ||
							subdir->type == "RWPORT"	)
						{
							bool bInsert= true;
							PortTypes act;
							string port;

							port= (*sit)->needValue("ID");
							if(subdir->type == "PORT")
								act= PORT;
							else if(subdir->type == "MPORT")
								act= MPORT;
							else
								act= RWPORT;
							for(vector<pair<string, PortTypes> >::iterator it= vlRv.begin(); it != vlRv.end(); ++it)
							{
								if(	it->first == port
									&&
									it->second == act	)
								{
									bInsert= false;
									break;
								}
							}
							if(bInsert)
								vlRv.push_back(pair<string, PortTypes>(port, act));
						}
						/************************************************************/
						if(buse)
						{
							pProperty= new ActionProperties;
							*pProperty= *dynamic_cast<ActionProperties*>(*sit);
							subdir->property= SHAREDPTR::shared_ptr<IActionPropertyPattern>(pProperty);
							subdir->property->readLine(sFID.str());
							subdir->name= value;
							subdir->bCorrect= false;
							/************************************************************\
							 * define values for subroutine witch should be obsolete
							\************************************************************/
							subdir->producerBValue= -1;
							subdir->defaultValue= 0;
							subdir->tmlong= 0;
							subdir->bAfterContact= false;
							subdir->measuredness= 0;
							/************************************************************/

							// to get no error if the _folderID not fetched,
							// fetch it now
							subdir->property->getValue("_folderID");

							aktualFolder->subroutines.push_back(*subdir.get());
						}
					}
				}// modifier is subroutine
			}// iterate subroutines of folder
		}// modifier is folder
	}// iterate all folder in mainprop


#if 0
	bool bRead;
	//Properties::param_t pparam;
	string line;
	//string sAktSubName;
	ifstream file(fileName.c_str());

	//bool bWrite= true;
	vector<string> lines;
	vector<string> names;
	//vector<unsigned long> vlRv;
	list<unsigned long>::iterator result; // result of find in port-list of folder
	//bool bWroteMikrosec= false;
	//bool bWroteCorrection= false;

	//if(subName=="")
	//	bWrite= false;
	if(file.is_open())
	{
		while(!file.eof())
		{
			//bool bCasherRead= false;

			getline(file, line);
			//cout << "\"" << line << "\"" << endl;
			//if(line=="in= COM1:DCD")
			//	cout << "stop" << endl;
			stringstream ss(line);
			string buffer;
			string type("");
			string value("");
			string::size_type pos;


			while(ss >> buffer)
			{
				if(buffer.substr(0, 1) != "#")
				{
					//cout << "  ... " << buffer << endl;
					if(type == "")
					{
						pos= buffer.find("=");
						if(pos < buffer.size())
						{
							type= buffer.substr(0, pos);
							//printf("size:%d pos:%d", buffer.size(), pos);
							if(pos+1 < buffer.size())
							{
								value= buffer.substr(pos+1);
								break;
							}
						}else
							type= buffer;
						buffer= "";
					}
					if(	buffer!=""
						&&
						buffer!="=")
					{
						if(buffer.substr(0, 1)=="=")
							buffer= buffer.substr(1, buffer.size()-1);
						value+= buffer;
					}
				}else
					break;
			}
			/*if(type != "")
			{
				cout << " >> found TYPE:\"" << type << "\" with value \"" << value << "\"" << flush;
				cout << endl;
			}*/
			bRead= false;
			if(type=="file")
			{
				readFile(vlRv, URL::addPath(m_sConfPath, value));
				bRead= true;

			}else if(type=="folder")
			{

			}else if(type == "name")
			{

			}
			if(	subdir.get()
				&&
				subdir->type != ""	)
			{
				if(	(	type == "ID"
						&&
						(	subdir->type == "PORT"
							||
							subdir->type == "MPORT"
							||
							subdir->type == "RWPORT"	)	)
					||
					(	subdir->type == "MPORT"
						&&
						(	value.substr(0, 3) == "COM"
							||
							value.substr(0, 3) == "LPT"	)
						&&
						(	type == "out"
							||
							type == "neg"	)	)				)
				{
					bool bInsert= true;
					PortTypes act;
					string port(value);

					if(	value.substr(0, 3) == "COM"
						||
						value.substr(0, 3) == "LPT"	)
					{
						vector<string> spl;

						split(spl, value, is_any_of(":"));
						port= spl[0];
					}
					if(subdir->type == "PORT")
						act= PORT;
					else if(subdir->type == "MPORT")
						act= MPORT;
					else
						act= RWPORT;
					for(vector<pair<string, PortTypes> >::iterator it= vlRv.begin(); it != vlRv.end(); ++it)
					{
						if(	it->first == port
							&&
							it->second == act	)
						{
							bInsert= false;
							break;
						}
					}
					if(bInsert)
						vlRv.push_back(pair<string, PortTypes>(value, act));
				}
				subdir->property->readLine(line);
				{
					ConfigPropertyCasher* p;

					p= dynamic_cast<ConfigPropertyCasher*>(subdir->property.get());
					if(	p && !p->newSubroutine().correct)
						continue;
				}

// writing pins into aktual folder
// hoping to never used
/**********************************************************************************************************/
				if(	subdir->type == "MPORTS"
					||
					subdir->type == "TEMP"
					||
					subdir->type == "TIMEMEASURE"
					||
					subdir->type == "RESISTANCE"	)
				{
					portBase::Pins pin;
					string value;
					vector<string> need;
					vector<pair<string, PortTypes> >::iterator portIt;

					if(	subdir->type == "GETCONTACT"
						||
						subdir->type == "TEMP"			)
					{
						need.push_back("pin");
						need.push_back("out");
					}else if(subdir->type == "SWITCHCONTACT")
					{
						need.push_back("out");
					}else if(	subdir->type == "TIMEMEASURE"
								||
								subdir->type == "RESISTANCE"	)
					{
						need.push_back("pin");
						need.push_back("out");
						need.push_back("neg");
					}

					for(vector<string>::iterator it= need.begin(); it != need.end(); ++it)
					{
						value= subdir->property->getValue(*it, /*warning*/false);
						pin= portBase::getPinsStruct(value);
						if(pin.nPort != 0)
						{
							portIt= find(vlRv, pin.sPort);
							if(portIt == vlRv.end())
								vlRv.push_back(pair<string, PortTypes>(pin.sPort, MPORT));
							if(	*it == "in"
								&&
								pin.ePin != portBase::NONE	)
							{
								aktualFolder->needInPorts.insert(pin);
							}
						}
					}
				}
			}

			if(type == "type")
			{
				subdir->type= value;

			}
// all this next stuff is reading inside Properties
// hope this things are not usable in future
			if(type=="measuredness")
			{
				if(subdir.get())
				{
					if(value == "now")
						subdir->measuredness= -1;
					else
						subdir->measuredness= atoi(value.c_str());
				}else
					m_nMeasuredness=  atoi(value.c_str());

			}else if(type=="measurednessCount")
			{
				m_nMeasurednessCount=  atoi(value.c_str());

			}else if(type=="microsecCount")
			{
				m_nMicrosecCount=  (unsigned short)atoi(value.c_str());

			}else if(type == "correction")
			{
				correction_t tCorr;
				vector<string> correction= ConfigPropertyCasher::split(value, ":");

				if(correction[1] == "now")
				{
					/*if(subName != "")
					{
						char sMikrosec[500];

						tCorr= *(correction_t*)changeValue;
						tCorr.bSetTime= true;
						//cout << "write correction" << resistor.nMikrosec << endl;
						sprintf(sMikrosec, "%lu", tCorr.nMikrosec);
						line= "correction= ";
						line+= sMikrosec;
						line+= ":";
						sprintf(sMikrosec, "%.60lf", tCorr.correction);
						line+= sMikrosec;
						bWroteCorrection= true;
						//cout << line << endl;
					}else
					{*/
						tCorr.bSetTime= false;
						tCorr.be= atof(&correction[0][0]);
					//}
				}else
				{
					string sValue;

					sValue= correction[0];
					tCorr.nMikrosec= strtoul(&sValue[0], NULL, 0);
					sValue= correction[1];
					tCorr.correction= strtod(&sValue[0], NULL);
					tCorr.bSetTime= true;
				}
				//if(subName=="")
				//{
					if(subdir.get())
						subdir->correction.push_back(tCorr);
					else
						m_vCorrection.push_back(tCorr);
				//}

			}else if(type == "OHM")
			{
				ohm resistor;
				vector<string> correction= ConfigPropertyCasher::split(value, ":");

				resistor.be= (double)atof(correction[0].c_str());
				if(correction[1]=="now")
				{
					/*if(subName != "")
					{
						char sMikrosec[50];

						resistor.nMikrosec= *(unsigned long*)changeValue;
						resistor.bSetTime= true;
						//cout << "write correction" << resistor.nMikrosec << endl;
						sprintf(sMikrosec, "%lu", *(unsigned long*)changeValue);
						line= "OHM= " + correction[0] + ":" + sMikrosec;
						bWroteMikrosec= true;
						//cout << line << endl;
					}else*/
						resistor.bSetTime= false;
				}else
				{
					const char *cCorrection= correction[1].c_str();

					resistor.nMikrosec= strtoul(cCorrection, NULL, 0);
					resistor.bSetTime= true;
				}
				//if(subName=="")
				//{
					if(subdir.get())
						subdir->resistor.push_back(resistor);
					else
						m_vOhm.push_back(resistor);
				//}
			}else if(type == "vector")
			{
				vector<string> vOhm= ConfigPropertyCasher::split(value, ":");
				const char *cOhm1= vOhm[0].c_str();
				const char *cOhm2= vOhm[1].c_str();
				unsigned short a, b;

				a= (unsigned short)atoi(cOhm1);
				b= (unsigned short)atoi(cOhm2);
				if(a > b)
				{
					unsigned short buffer= a;

					a= b;
					b= buffer;
				}
				subdir->ohmVector.push_back(a);
				subdir->ohmVector.push_back(b);

			}else if(type == "BVALUE")
			{
				subdir->producerBValue= atoi(value.c_str());

			}else if(type == "out")
			{
				portBase::Pins ePort= portBase::getPinsStruct(value);
				portBase::portpin_address_t ePortPin;
				bool bInsert= true;

				for(vector<pair<string, PortTypes> >::iterator it= vlRv.begin(); it != vlRv.end(); ++it)
				{
					if(	it->first == ePort.sPort
						&&
						it->second == MPORT		)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(pair<string, PortTypes>(ePort.sPort, MPORT));
				subdir->out= ePort;
				ePortPin= portBase::getPortPinAddress(subdir->out, false);
				if(	subdir->out.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(ePort.sPort)
					||
					ePortPin.eDescript == portBase::GETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= ePort.sPin + "' is no correct pin on port '";
					msg+= ePort.sPort + "'\n    ERROR on line: ";
					msg+= line + "\n    stop server!";
					LOG(LOG_ALERT, msg);
#ifndef DEBUG
					cout << msg << endl;
#endif
					cout << endl;
					exit(1);
				}
			}else if(type == "in")
			{
				portBase::Pins ePort= portBase::getPinsStruct(value);
				portBase::portpin_address_t ePortPin;
				bool bInsert= true;

				for(vector<pair<string, PortTypes> >::iterator it= vlRv.begin(); it != vlRv.end(); ++it)
				{
					if(	it->first == ePort.sPort
						&&
						it->second == MPORT		)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(pair<string, PortTypes>(ePort.sPort, MPORT));
				subdir->in= ePort;
				aktualFolder->needInPorts.insert(subdir->in);
				ePortPin= portBase::getPortPinAddress(subdir->in, false);
				if(	subdir->in.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(ePort.sPort)
					||
					ePortPin.eDescript == portBase::SETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= ePort.sPin + "' is no correct pin on port '";
					msg+= ePort.sPort + "'\n    ERROR on line: ";
					msg+= line + "\n    stop server!";
					LOG(LOG_ALERT, msg);
#ifndef DEBUG
					cout << msg << endl;
#endif
					cout << endl;
					exit(1);
				}
			}else if(type == "neg")
			{
				portBase::Pins ePort= portBase::getPinsStruct(value);
				portBase::portpin_address_t ePortPin;
				bool bInsert= true;

				for(vector<pair<string, PortTypes> >::iterator it= vlRv.begin(); it != vlRv.end(); ++it)
				{
					if(	it->first == ePort.sPort
						&&
						it->second == MPORT		)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(pair<string, PortTypes>(ePort.sPort, MPORT));
				subdir->negative= ePort;
				ePortPin= portBase::getPortPinAddress(subdir->negative, false);
				if(	subdir->negative.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(ePort.sPort)
					||
					ePortPin.eDescript == portBase::GETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= ePort.sPin + "' is no correct pin on port '";
					msg+= ePort.sPort + "'\n    ERROR on line: ";
					msg+= line + "\n    stop server!";
					LOG(LOG_ALERT, msg);
#ifndef DEBUG
					cout << msg << endl;
#endif
					cout << endl;
					exit(1);
				}

			}else if(type == "max")
			{
				int max= atoi(value.c_str());

				subdir->nMax= max;

			}else if(type == "after")
			{
				if(	value=="true"
					||
					value=="TRUE"	)
				{
					subdir->bAfterContact= true;
				}

			}else if(type == "default")
			{
				subdir->defaultValue= atof(&value[0]);

			}else if(	!bRead
						&&
						line != ""
						&&
						line.substr(0, 1) != "#")
			{
				string msg;

				msg=  "### warning: cannot read line '";
				msg+= line + "'";
				if(aktualFolder != NULL)
				{
					msg+= "\n    under folder >> ";
					msg+= aktualFolder->name;
					msg+= " <<  ";
				}
				if(subdir.get() != NULL)
				{
					msg+= "\n    with type-name '" + subdir->name + "'";
				}
				cout << msg << endl;
			}

			lines.push_back(line);
		}// end of while(!file.eof())
	}else
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

	if(subdir.get() != NULL)
		aktualFolder->subroutines.push_back(*subdir);
#endif
}

#if 0
void Starter::checkAfterContact()
{
	unsigned int nSize;
	SHAREDPTR::shared_ptr<measurefolder_t> aktfolder;
	set<portBase::Pins>::iterator res;
	set<portBase::Pins> afterContactPins;


	if(m_tFolderStart==NULL)
		return;
	aktfolder= m_tFolderStart;
	// search for folder, wiche should be getting contact while sleeping
	while(aktfolder)
	{
		//cout << "afterContact:" << aktfolder->needInPorts.size() << endl;
		nSize= aktfolder->subroutines.size();
		for(unsigned int n= 0; n<nSize; n++)
		{
			if(aktfolder->subroutines[n].bAfterContact)
			{
				afterContactPins.insert(aktfolder->subroutines[n].in);
			}

		}
		if(!afterContactPins.empty())
		{
			bool bCan= true;
			SHAREDPTR::shared_ptr<measurefolder_t> search= m_tFolderStart;
			portBase::portpin_address_t ePortPin1;
			portBase::portpin_address_t ePortPin2;

			// search also for other folder which have access to same ports.
			// showes error wether this case
			while(search)
			{
				if(aktfolder->name != search->name)
				{
					for(set<portBase::Pins>::iterator i= afterContactPins.begin(); i != afterContactPins.end(); i++)
					{
						ePortPin1= portBase::getPortPinAddress(*i, false);
						for(set<portBase::Pins>::iterator n= search->needInPorts.begin(); n!=search->needInPorts.end(); n++)
						{
							ePortPin2= portBase::getPortPinAddress(*n, false);
							if(ePortPin1.nPort == ePortPin2.nPort)
							{
								char offset[10];
								string msg("### ERROR: cannot set getting contact for thread ");

								msg+= aktfolder->name + " while sleeping\n";
								msg+= "           port ";
								msg+= portBase::getPortName(i->nPort);
								msg+= " with offset ";
								sprintf(offset, "%d", (int)(ePortPin1.nPort - i->nPort));
								msg+= offset;
								msg+= " can only set in this one thread!";
								LOG(LOG_ERROR, msg);
	#ifndef DEBUG
								cout << msg << endl;
	#endif // DEBUG
								bCan= false;
								break;
							} // end if(res != NULL)
						}
					} // end for(afterContactPins)
					if(!bCan)
						break;
				} // end if(aktfolder->name != search->name)
				search= search->next;
			} // end while(search)
			if(bCan)
				aktfolder->afterContactPins= afterContactPins;
			afterContactPins.clear();
		} // end if(!afterContactPins.empty())
		aktfolder= aktfolder->next;
	} // end while(aktfolder)

	aktfolder= m_tFolderStart;
	// search for measure-threads for getting contact while sleeping
	// show error wether this case
	while(aktfolder)
	{
		bool canAfterContact= true;
		string subroutine;

		if(	!aktfolder->afterContactPins.empty()
			&&
			!aktfolder->needInPorts.empty())
		{
			nSize= aktfolder->subroutines.size();
			for(unsigned int n= 0; n<nSize; n++)
			{
				if(!aktfolder->subroutines[n].portClass->doForAfterContact())
				{
					canAfterContact= false;
					subroutine+= aktfolder->subroutines[n].name;
					subroutine+= " with type ";
					subroutine+= aktfolder->subroutines[n].type;
					subroutine+= ", ";
				}
			}
			if(!canAfterContact)
			{
				string msg("### ERROR: cannot set getting contact for thread ");

				subroutine= subroutine.substr(0, subroutine.length()-2);
				msg+= aktfolder->name + " while sleeping\n";
				msg+= "           subroutines >> ";
				msg+= subroutine;
				msg+= "\n           cannot set in this folder";
				LOG(LOG_ERROR, msg);
#ifndef DEBUG
				cout << msg << endl;
#endif // DEBUG
				aktfolder->needInPorts.clear();
			} // end if(!canAfterContact)
		} // end if(!aktfolder->needInPorts.empty())
		//cout << "afterContact:" << aktfolder->needInPorts.size() << endl;
		aktfolder= aktfolder->next;
	}
}
#endif

bool Starter::status()
{
	bool bOK;
	char	buf[165];
	FILE 	*fp;
	int clientsocket, err;
	unsigned short nPort;
	string result;
	string property;
	string confpath, fileName;


	if(m_oServerFileCasher.isEmpty())
	{
		confpath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
		fileName= URL::addPath(confpath, "server.conf");
		if(!m_oServerFileCasher.readFile(fileName))
		{
			cout << "### ERROR: cannot read '" << fileName << "'" << endl;
			return false;
		}
	}
	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		exit(EXIT_FAILURE);
	clientsocket= ServerThread::connectAsClient("127.0.0.1", nPort, false);
	if(clientsocket==0)
	{
		printf("no server is running\n");
		return false;
	}
	fp = fdopen (clientsocket, "w+");

	fputs ("GET\n", fp);
	buf[0]= '\0';
	fgets(buf, sizeof(buf), fp);
	result= buf;

	if(	result.size() <= 12
		||
		result.substr(0, 12) != "port-server:")
	{
		cout << "ERROR: undefined server running on port" << endl;
		return false;
	}

	fputs("status text\n", fp);
	bOK= true;
	do{
		buf[0]= '\0';
		fgets(buf, sizeof(buf), fp);
		result= buf;
		err= ExternClientInputTemplate::error(result);
		if(err)
		{
			if(ExternClientInputTemplate::error(result) > 0)
			{
				ostringstream send;

				send << "GETERRORSTRING " << err;
				fputs(send.str().c_str(), fp);
				fgets(buf, sizeof(buf), fp);
				cerr << buf;
				bOK= false;
				break;
			}else
				cout << result;
		}
		if(result != "done\n")
		{
			cout << result;
		}

	}while(result != "done\n");

	fclose(fp);
	close(clientsocket);
	return true;
}

bool Starter::stop(bool configure)
{
	char	buf[64];
	string  sendbuf;
	string 	stopping, dostop;
	char	stopServer[]= "stop-server\n";
	FILE 	*fp;
	int clientsocket;
	unsigned short nPort;
	string result;
	string fileName;
	string confpath, logpath, sLogLevel, property, username;
	UserManagement* user= UserManagement::instance();
	ostringstream hello;

	hello << "GET v" << PPI_SERVER_PROTOCOL << endl;
	sendbuf= hello.str();
	confpath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(confpath, "server.conf");
	m_oServerFileCasher.setDelimiter("owreader", "[", "]");
	m_oServerFileCasher.modifier("owreader");
	m_oServerFileCasher.readLine("workdir= " + m_sWorkdir);
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

	if(user == NULL)
	{
		if(!UserManagement::initial(URL::addPath(confpath, "access.conf", /*always*/true),
									""/*needs no definition of subroutine permission*/)		)
			return false;
		user= UserManagement::instance();
	}

	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		exit(EXIT_FAILURE);
	clientsocket= ServerThread::connectAsClient("127.0.0.1", nPort);
	if(clientsocket==0)
	{
		printf("no server is running\n");
		return false;
	}
	fp = fdopen (clientsocket, "w+");

	fputs (sendbuf.c_str(), fp);
	buf[0]= '\0';
	fgets(buf, sizeof(buf), fp);
	result= buf;

	if(	result.size() < 18
		||
		result.substr(0, 11) != "ppi-server:")
	{
		cout << "### ERROR: undefined server running on port" << endl;
		fclose(fp);
		return false;
	}

	username= user->getRootUser();
	if(!user->canLoginFirst(username))
	{
		username= user->getNoRootUser();
		sendbuf= "U:";
		sendbuf+= username + ":";
		sendbuf+= user->getPassword(username);
		sendbuf+= "\n";
		fputs(sendbuf.c_str(), fp);
		fflush(fp);
		fgets(buf, sizeof(buf), fp);
		if(strcmp(buf, "OK\n"))
		{
			writeErrString(buf, fp);
			fputs("ending", fp);
			fflush(fp);
			fclose(fp);
			return false;
		}
		sendbuf= "CHANGE ";
		username= user->getRootUser();
	}else
		sendbuf= "U:";
	sendbuf+= username + ":";
	sendbuf+= user->getPassword(username);
	sendbuf+= "\n";
	fputs(sendbuf.c_str(), fp);
	fflush(fp);
	fgets(buf, sizeof(buf), fp);
	if(strcmp(buf, "OK\n"))
	{
		writeErrString(buf, fp);
		fputs("ending", fp);
		fflush(fp);
		fclose(fp);
		return false;
	}

	fputs (stopServer, fp);
	fflush(fp);
	buf[0]= '\0';
	while(strcmp(buf, "OK\n"))
	{
		buf[0]= '\0';
		fgets(buf, sizeof(buf), fp);
		if(feof(fp))
		{
#ifndef DEBUG
			printf("\n");
			printf("ERROR: lost connection to server\n");
			printf("       maybe server always running\n");
#endif // DEBUG
			LOG(LOG_ERROR, "ERROR: lost connection to server\n       maybe server always running");
			break;
		}
		dostop= buf;
		boost::trim(dostop);
		if(	dostop.substr(0, 5) == "ERROR" ||
			dostop.substr(0, 7) == "WARNING"	)
		{
			writeErrString(buf, fp);
			fclose(fp);
			return false;
		}
		if(stopping == dostop)
			cout << "." << flush;
		else
		{
			stopping= dostop;
			if(stopping != "OK" && stopping != "")
				cout << endl << stopping  << " ." << flush;
		}
	}
	fclose(fp);
	close(clientsocket);
	cout << endl;
	return true;
}

bool Starter::writeErrString(const string& err, FILE *fp) const
{
	bool bError(false);
	char buf[1024];
	string result, code, sendbuf("GETERRORSTRING ");
	istringstream errstr(err);

	errstr >> result >> code;
	if(result == "WARNING")
		code= "-" + code;
	else if(result != "ERROR")
	{
		cerr << "### undefined ERROR occured" << endl;
		return true;
	}else
		bError= true;
	sendbuf+= code + "\n";
	fputs(sendbuf.c_str(), fp);
	fflush(fp);
	fgets(buf, sizeof(buf), fp);
	result+= buf;
	if(bError)
		cerr << "### ERROR: " << result << endl;
	else
		cout << "### WARNING: " << result << endl;
	return bError;
}

bool Starter::checkServer()
{
	char	buf[64];
	char 	sendbuf[] = "GET\n";
	FILE 	*fp;
	int clientsocket= 0;
	unsigned short nDefaultPort;
	string result;
	string host;
	string property("port");

	nDefaultPort= m_oServerFileCasher.needUShort(property);
	//host= m_oServerFileCasher.needValue("host");
	if(	nDefaultPort == 0
		&&
		property == "#ERROR"	)
	{
		exit(EXIT_FAILURE);
	}
	clientsocket= ServerThread::connectAsClient("127.0.0.1", nDefaultPort);
	if(clientsocket == 0)
	{
		// no server is running
		return false;
	}
	fp = fdopen (clientsocket, "w+");

	fputs (sendbuf, fp);

	buf[0]= '\0';
	fgets(buf, sizeof(buf), fp);

	fclose(fp);
	close(clientsocket);

	result= buf;
	if(	result != "port-server:noClient\n"
		&&
		result != "port-server:clientRunning\n")
	{
#ifndef DEBUG
		cout << "ERROR: undefined server running on port" << endl;
#endif // DEBUG
		LOG(LOG_ALERT, "ERROR: undefined server running on port");
		exit(1);
	}
	return true;
}

void Starter::isNoPathDefinedStop()
{
#ifndef PPICONFIGPATH
	cerr << "no configuration path PPICONFIGPATH defined for preprocessor" << endl;
	exit(EXIT_FAILURE);
#endif
#ifndef PPIDATABASEPATH
	cerr << "no path for database PPIDATABASEPATH defined for preprocessor" << endl;
	exit(EXIT_FAILURE);
#endif
#ifndef PPILOGPATH
	cerr << "no logging path PPILOGPATH defined for preprocessor" << endl;
	exit(EXIT_FAILURE);
#endif
}
