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
#include <algorithm>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <list>

#include "util/debug.h"
#include "util/URL.h"
#include "util/configpropertycasher.h"
#include "util/usermanagement.h"
#include "util/ProcessStarter.h"

#include "logger/lib/LogInterface.h"

#include "database/DefaultChipConfigReader.h"
#include "database/Database.h"

#include "portserver/owserver.h"
#include "portserver/maximchipaccess.h"
#include "portserver/VellemannK8055.h"

#include "ports/measureThread.h"
#include "ports/portbaseclass.h"
#include "ports/timemeasure.h"
#include "ports/switch.h"
#include "ports/switchcontact.h"
#include "ports/contactpin.h"
#include "ports/resistancemeasure.h"
#include "ports/tempmeasure.h"
#include "ports/counter.h"
#include "ports/measuredness.h"
#include "ports/timer.h"
#include "ports/shell.h"
#include "ports/valueholder.h"
#include "ports/SaveSubValue.h"
#include "ports/OwfsPort.h"

#include "server/libs/server/ServerProcess.h"
#include "server/libs/server/Communication.h"
#include "server/libs/server/communicationthreadstarter.h"
#include "server/libs/server/TcpServerConnection.h"

#include "server/ServerTransaction.h"
#include "server/ServerMethodTransaction.h"
#include "server/ClientTransaction.h"

#include "starter.h"

using namespace ppi_database;
using namespace util;
using namespace server;
using namespace user;
using namespace std;
using namespace logger;


bool Starter::openPort(unsigned long nPort, int nBaud, char cParitaetsbit, unsigned short nDatabits, unsigned short nStopbit)
// : m_nPort(nPort)
{
	int res;
	char msg[50];

	sprintf(msg, "### open interface to port %s", portBase::getPortName(nPort));
#ifndef DEBUG
	cout << msg << endl;
#endif // DEBUG
	LOG(LOG_INFO, msg);
	res= ioperm(nPort, 8, 1);
	if(res)
		return false;
	return true;
}

bool Starter::execute(vector<string> options)
{
	bool bLog, bDb, bPorts, bCommunicate, bInternet;
	unsigned int nOptions= options.size();
	vector<unsigned long> ports; // whitch ports are needet
	string fileName;
	string logpath, dbpath, sLogLevel, property;
	Database *db;
	string prop;
	ProcessStarter* process;

	Starter::isNoPathDefinedStop();
	if(signal(SIGINT, signalconverting) == SIG_ERR)
		printSigError("SIGINT");
	if(signal(SIGHUP, signalconverting) == SIG_ERR)
		printSigError("SIGHUP");
	if(signal(SIGSEGV, signalconverting) == SIG_ERR)
		printSigError("SIGSEGV");

#ifdef SINGLETHREADING
	bAsServer= false;
#endif // SINGLETHREADING

	bool bOp= true;
	for(unsigned int o= 0; o<nOptions; ++o)
	{
		bOp= false;
	}
	if(!bOp)
	{
		cerr << "### WARNING: not all options are for all commands," << endl;
		cerr << "             see -? for help" << endl;
	}

	m_vOWServerTypes.push_back("OWFS");
	m_vOWServerTypes.push_back("Vk8055");

	m_sConfPath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	dbpath= URL::addPath(m_sWorkdir, PPIDATABASEPATH, /*always*/false);

	fileName= URL::addPath(m_sConfPath, "server.conf");
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(EXIT_FAILURE);
	}
	m_oServerFileCasher.readLine("workdir= " + m_sWorkdir);
	readFile(ports, URL::addPath(m_sConfPath, "measure.conf"));

	// check wether shoud be start which server
	property= m_oServerFileCasher.getValue("logserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter logserver not be set, so start server" << endl;
		bLog= true;
	}else if(property == "true")
		bLog= true;
	else
		bLog= false;
	property= m_oServerFileCasher.getValue("databaseserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter databaseserver not be set, so start server" << endl;
		bDb= true;
	}else if(property == "true")
		bDb= true;
	else
		bDb= false;
	property= m_oServerFileCasher.getValue("portserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter portserver not be set, so start server" << endl;
		bPorts= true;
	}else if(property == "true")
		bPorts= true;
	else
		bPorts= false;
	property= m_oServerFileCasher.getValue("communicateserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter communicateserver not be set, so start server" << endl;
		bCommunicate= true;
	}else if(property == "true")
		bCommunicate= true;
	else
		bCommunicate= false;
	property= m_oServerFileCasher.getValue("internetserver", /*warning*/true);
	if(	property == ""
		||
		(	property != "true"
			&&
			property != "false"	)	)
	{
		cerr << "###          parameter internetserver not be set, so start server" << endl;
		bInternet= true;
	}else if(property == "true")
		bInternet= true;
	else
		bInternet= false;

	if(!UserManagement::initial(URL::addPath(m_sConfPath, "access.conf", /*always*/true)))
	{
		cerr << "### ERROR: cannot read correctly 'access.conf'" << endl;
		return false;
	}

	readPasswd();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// initial interface to log server
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
	LogInterface::init(	"ppi-server",
						new SocketClientConnection(	SOCK_STREAM,
													commhost,
													commport,
													0			),
						/*identif log*/nLogAllSec,
						/*wait*/true								);
	LogInterface::instance()->setThreadName("main--run-server");
	// ------------------------------------------------------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start server for communication between processes

	cout << "### start ppi communication server" << endl;
	ServerProcess communicate(	m_tDefaultUser,
								new CommunicationThreadStarter(0, 4),
								new TcpServerConnection(	commhost,
															commport,
															10,
															new ServerMethodTransaction()	),
								new SocketClientConnection(	SOCK_STREAM,
															commhost,
															commport,
															10			)						);

	if(communicate.start(&m_oServerFileCasher) > 0)
	{
		cerr << "### ALERT: cannot start communication server" << endl;
		cerr << "           so the hole application is not useable" << endl;
		cerr << "           stop server" << endl;
		exit(EXIT_FAILURE);
	}
	// ------------------------------------------------------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start logging process
	int err;

/*	LogProcess logger(	m_tLogUser,
						new SocketClientConnection(	SOCK_STREAM,
													commhost,
													commport,
													10			),
						new SocketClientConnection(	SOCK_STREAM,
													commhost,
													commport,
													10			)	);

	err= logger.start(&m_oServerFileCasher);*/

	cout << "### start ppi log client" << endl;
	process= new ProcessStarter(	"LogServer",
									new SocketClientConnection(	SOCK_STREAM,
																commhost,
																commport,
																10			)	);

	if(bLog)
		err= process->start(URL::addPath(m_sWorkdir, "bin/ppi-log-client").c_str(), NULL);
	else
		err= process->check();
	if(err > 0)
	{
		cerr << "### WARNING: cannot start log-server" << endl;
		cerr << "             so no log can be written into any files" << endl;
		cerr << "             " << process->strerror(err) << endl;
	}
	delete process;

	// ------------------------------------------------------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(checkServer()==true)
	{
		LOG(LOG_ERROR, "### server is running\n    -> do nothing");
#ifndef DEBUG
		printf("### server is running\n");
		printf("    ->do nothing\n");
#endif
		return false;
	}

	for(unsigned int n= 0; n < ports.size(); n++)
	{
		if(!openPort(ports[n], 1200, 'N', 8, 1))
		{
			string errorMsg("");

			errorMsg+= "ERROR: by connecting to port ";
			errorMsg+= portBase::getPortName(ports[n]);
			errorMsg+= "\n       cannot open spezified port";
			LOG(LOG_ALERT, errorMsg);
			printf("ERROR: by connecting to port %s\n", portBase::getPortName(ports[n]));
			printf("       cannot open spezified port\n");
			printf("       maybe application not started as root\n\n");
			return false;
		}
	}

	setuid(m_tDefaultUser);
	LOG(LOG_INFO, "### -> starting server application.\n       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	LOG(LOG_INFO, "Read configuration files from " + m_sConfPath);

	Database::initial(dbpath, m_sConfPath, &m_oServerFileCasher);

	/***********************************************************************************\
	 *
	 * define one wire server
	 *
	 */
#ifdef _EXTERNVENDORLIBRARYS
	int nServerID= 1;
	OWServer* owserver;
	string maximinit("");
	vector<string>::size_type nVk8055Count= 0;
#endif // _EXTERNVENDORLIBRARYS

#ifdef _K8055LIBRARY
	bool bError;
	int nVk8055Address;
	string sVk8055Address;
	vector<int> vVk8055;
	vector<int>::iterator vit;

	// start Vellemann k8055 ports
	nVk8055Count= m_oServerFileCasher.getPropertyCount("Vk8055");
	for(vector<string>::size_type n= 0; n < nVk8055Count; ++n)
	{
		bError= false;
		prop= "Vk8055";
		nVk8055Address= m_oServerFileCasher.getInt(prop, n, /*warning*/false);

		if(	prop == "#ERROR"
			||
			nVk8055Address < 0
			||
			nVk8055Address > 3	)
		{
			bError= true;
		}else
		{
			vit= find(vVk8055.begin(), vVk8055.end(), nVk8055Address);
			if(vit == vVk8055.end())
			{
				cout << "### starting OWServer" << flush;
				vVk8055.push_back(nVk8055Address);
				owserver= new OWServer(nServerID, new VellemannK8055(static_cast<long>(nVk8055Address)));
				cout << " with name '" << owserver->getServerName();
				cout << "' and ID '" << dec << nServerID << "'" << endl;
				cout << "    k8055 USB port from Vellemann on itnerface " << dec << nVk8055Address << endl;
				if(owserver->start(&m_oServerFileCasher))
					OWServer::delServers(owserver);
				++nServerID;
			}else
				bError= true;
		}
		if(bError)
		{
			sVk8055Address= m_oServerFileCasher.getValue("Vk8055", n, false);
			cerr << "    ERROR by define " << (n+1) << ". port with address " << sVk8055Address << endl;
		}
	}
#endif //_K8055LIBRARY

#ifdef _OWFSLIBRARY
	// start maxim ports with owfs driver
	maximinit= m_oServerFileCasher.getValue("maximinit", /*warning*/false);
	if(maximinit != "")
	{
		cout << "### starting OWServer" << flush;
		owserver= new OWServer(nServerID, new MaximChipAccess());
		cout << " with name '" << owserver->getServerName();
		cout << "' and ID '" << dec << nServerID << "'" << endl;
		cout << "    OWFS device - initial with '" << maximinit << "'" << endl;
		if(owserver->start(&m_oServerFileCasher))
			OWServer::delServers(owserver);
		++nServerID;
		//owserver= new OWServer(new MaximChipAccess());
		//if(owserver->start(&m_oServerFileCasher))
		//	OWServer::delServers(owserver);

/*		vector<string> inits;
		vector<string>::iterator c;

		inits= ConfigPropertyCasher::split(maximinit, " ");
		c= inits.begin();
		while(c != inits.end())
		{
			if(	*c == "-all"
				||
				*c == "--usb=all"	)
			{
				int usb= 1;
				string init;
				//MaximChipAccess* maxPort;
				OWServer* owserver;
				vector<string> allIds, ids;
				vector<string>::size_type idCount;


				owserver= new OWServer(new MaximChipAccess("-u"));
				owserver->start(&m_oServerFileCasher);
				allIds= owserver->getChipIDs();
				if(!allIds.size())
				{
					string msg("### WARNING: does not found any chip on device");

					msg+= "for initialisation";
					msg+= *c;
					cerr << msg << endl;
					LOG(LOG_WARNING, msg);
				}else
				{
					do{
						char buf[20];

						++usb;
						snprintf(buf, 20, "-u%d", usb);
						init= buf;
						owserver= new OWServer(new MaximChipAccess(init, &allIds));
						owserver->start(&m_oServerFileCasher);
						//bres= (bool) *res;
						ids= owserver->getChipIDs();
						allIds.insert(allIds.end(), ids.begin(), ids.end());
						idCount= ids.size();
						if(idCount == 0)
							OWServer::delServers(owserver);

					}while(idCount);
				}
			}else
			{
				OWServer* owserver;

				owserver= new OWServer(new MaximChipAccess(maximinit));
				if(owserver->start(&m_oServerFileCasher))
					OWServer::delServers();
			}
			++c;
		}*/
	}
#endif //_OWFSLIBRARY

	createPortObjects();

#ifdef _EXTERNVENDORLIBRARYS
	if(	maximinit != ""
		||
		nVk8055Count > 0	)
	{
		OWServer::checkUnused();
		OWServer::endOfInitialisation();
	}
#endif // _EXTERNVENDORLIBRARYS

	checkAfterContact();

	bool createThread= false;
	MeasureArgArray args;
	meash_t *pFirstMeasureThreads= NULL;
	meash_t *pCurrentMeasure= NULL;

	meash_t::clientPath= URL::addPath(m_sWorkdir, PPICLIENTPATH, /*always*/false);
	LOG(LOG_INFO, "Read layout content for clients from " + meash_t::clientPath);
	measurefolder_t *aktFolder= m_tFolderStart;
	args.ports= ports;
	cout << "### start measure thread(s)" << endl;
	while(aktFolder != NULL)
	{
		if(!aktFolder->bCorrect)
		{
			string msg("folder '");

			msg+= aktFolder->name;
			msg+= "' has no correct subroutine\n";
			msg+= "so make no measure-instance for it";
			LOG(LOG_WARNING, msg);
#ifndef DEBUG
			cout << msg << endl;
#endif // DEBUG
		}else
		{
			cout << "                     " << aktFolder->name << endl;
			createThread= true;
			if(pFirstMeasureThreads == NULL)
			{
				pFirstMeasureThreads= new meash_t;
				pCurrentMeasure= pFirstMeasureThreads;
				meash_t::firstInstance= pCurrentMeasure;
			}else
			{
				pCurrentMeasure->next= new meash_t;
				pCurrentMeasure= pCurrentMeasure->next;
			}
			pCurrentMeasure->next= NULL;
			pCurrentMeasure->pMeasure = new MeasureThread(aktFolder->name);
			args.subroutines= &aktFolder->subroutines;
			args.tAfterContactPins= aktFolder->afterContactPins;
			pCurrentMeasure->pMeasure->start(&args);
		}
		aktFolder= aktFolder->next;
	}
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
	unsigned short minThreads, maxThreads;

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
	DefaultChipConfigReader::instance()->chipsDefined(true);

	property= "minconnectionthreads";
	minThreads= m_oServerFileCasher.getUShort(property, /*warning*/true);
	property= "maxconnectionthreads";
	maxThreads= m_oServerFileCasher.getUShort(property, /*warning*/true);
	if(	property == "#ERROR"
		||
		maxThreads < 4		)
	{
		maxThreads= 4;
	}

	ServerThread server(new CommunicationThreadStarter(minThreads, maxThreads),
						new TcpServerConnection(	host,
													port,
													10,
													new ServerTransaction()	)	);

	gInternetServer= &server;
	server.start(NULL, true);

	meash_t* delMeash;
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
		if(delMeash->pMeasure)
			delete delMeash->pMeasure;
		delete delMeash;
	}

	cout << "OWServer's" << endl;
	OWServer::delServers();
	cout << "Database" << endl;
	db= Database::instance();
	db->stop(/*wait*/true);
	cout << "communication server" << endl;
	communicate.stop(/*wait*/true);
	return true;
}

void Starter::createPortObjects()
{
	bool bNewMeasure= false;
	measurefolder_t* aktualFolder= m_tFolderStart;
	//Database* db= Database::instance();
	string property("defaultSleep");
	string sMeasureFile;
	unsigned short nDefaultSleep= m_oServerFileCasher.getUShort(property, /*warning*/false);

	if(property == "#ERROR")
		nDefaultSleep= 2;
	sMeasureFile= URL::addPath(m_sConfPath, "measure.conf");
	while(aktualFolder != NULL)
	{
		bool correctFolder= false;
		int nMuch= aktualFolder->subroutines.size();

		for(int n= 0; n<nMuch; n++)
		{
			vector<ohm> *pvOhm= &aktualFolder->subroutines[n].resistor;
			vector<correction_t> *pvCorrection= &aktualFolder->subroutines[n].correction;
			bool correctSubroutine= false;
			short measuredness= aktualFolder->subroutines[n].measuredness;

			if(measuredness == 0)
				measuredness= m_nMeasuredness;
			if(pvOhm->size() == 0)
				pvOhm= &m_vOhm;
			if(pvCorrection->size() == 0)
				pvCorrection= &m_vCorrection;

			//cout << "subroutine: " << aktualFolder->subroutines[n].type << endl;
			if(aktualFolder->subroutines[n].type == "SWITCH")
			{
				switchClass *obj= new switchClass(	aktualFolder->name,
													aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	m_tFolderStart,
							aktualFolder->subroutines[n].sBegin,
							aktualFolder->subroutines[n].sWhile,
							aktualFolder->subroutines[n].sEnd,
							aktualFolder->subroutines[n].defaultValue	);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/
			}else if(aktualFolder->subroutines[n].type == "TIMER")
			{
				timer *obj= new timer(	aktualFolder->name,
										aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	m_tFolderStart,
							aktualFolder->subroutines[n].sBegin,
							aktualFolder->subroutines[n].sWhile,
							aktualFolder->subroutines[n].tmlong	);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/
			}else if(aktualFolder->subroutines[n].type == "SHELL")
			{
				Shell *obj= new Shell(	aktualFolder->name,
										aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	m_tFolderStart,
							aktualFolder->subroutines[n].sWhile,
							aktualFolder->subroutines[n].sBeginComm,
							aktualFolder->subroutines[n].sWhileComm,
							aktualFolder->subroutines[n].sEndComm	);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/
			}else if(aktualFolder->subroutines[n].type == "SWITCHCONTACT")
			{
				switchContact *obj= new switchContact(	aktualFolder->name,
														aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	m_tFolderStart,
							aktualFolder->subroutines[n].out,
							aktualFolder->subroutines[n].sBegin,
							aktualFolder->subroutines[n].sWhile,
							aktualFolder->subroutines[n].sEnd	);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/
			}else if(aktualFolder->subroutines[n].type == "GETCONTACT")
			{
				contactPin *obj= new contactPin(	aktualFolder->name,
													aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	aktualFolder->subroutines[n].in,
							aktualFolder->subroutines[n].out	);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/

			}else if(aktualFolder->subroutines[n].type == "TIMEMEASURE")
			{
				TimeMeasure *obj= new TimeMeasure(	aktualFolder->name,
													aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
			}else if(aktualFolder->subroutines[n].type == "RESISTANCE")
			{
				ResistanceMeasure *obj= new ResistanceMeasure(	aktualFolder->name,
																aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;

			}else if(aktualFolder->subroutines[n].type == "TEMP")
			{
				TempMeasure *obj= new TempMeasure(	aktualFolder->name,
													aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*obj->init(	aktualFolder->subroutines[n].out,
							aktualFolder->subroutines[n].in,
							aktualFolder->subroutines[n].negative,
							measuredness,
							pvCorrection							);
				correctFolder= true;
				correctSubroutine= true;
				aktualFolder->subroutines[n].portClass= obj;*/

			}else if(aktualFolder->subroutines[n].type == "VALUE")
			{
				//double* pValue;
				//double value;
				ValueHolder *obj= new ValueHolder(	aktualFolder->name,
													aktualFolder->subroutines[n].name	);

				//pValue= db->getActEntry(	aktualFolder->name,
				//							aktualFolder->subroutines[n].name,
				//							"value"								);

				if(obj->init(*aktualFolder->subroutines[n].property))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
				/*if(pValue)
				{
					value= *pValue;
					delete pValue;
				}else
					value= aktualFolder->subroutines[n].defaultValue;
				obj->setValue(value);
				aktualFolder->subroutines[n].portClass= obj;
				correctFolder= true;
				correctSubroutine= true;*/

			}else if(aktualFolder->subroutines[n].type == "SAVE")
			{
				SaveSubValue* obj= new SaveSubValue(	aktualFolder->name,
														aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
			}else if(aktualFolder->subroutines[n].type == "COUNTER")
			{
				Counter* obj= new Counter(	aktualFolder->name,
											aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
			}else if(aktualFolder->subroutines[n].type == "MEASUREDNESS")
			{
				Measuredness* obj= new Measuredness(	aktualFolder->name,
														aktualFolder->subroutines[n].name	);

				if(obj->init(*aktualFolder->subroutines[n].property, m_tFolderStart))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
					delete obj;
			}else if(find(m_vOWServerTypes.begin(), m_vOWServerTypes.end(), aktualFolder->subroutines[n].type) != m_vOWServerTypes.end())
			{// type is reached over an OWServer instance
				OwfsPort* obj= NULL;

				//cout << aktualFolder->subroutines[n].name << endl;
				obj= new OwfsPort(	aktualFolder->subroutines[n].type,
									aktualFolder->name,
									aktualFolder->subroutines[n].name	);
				if(obj->init(m_tFolderStart, *aktualFolder->subroutines[n].property))
				{
					correctFolder= true;
					correctSubroutine= true;
					aktualFolder->subroutines[n].portClass= obj;
				}else
				{
					aktualFolder->subroutines[n].portClass= NULL;
					delete obj;
				}
			}

			aktualFolder->subroutines[n].bCorrect= correctSubroutine;
			if(!correctSubroutine)
			{
				aktualFolder->subroutines[n].bCorrect= false;
			}else
			{
				if(dynamic_cast<TimeMeasure*> (aktualFolder->subroutines[n].portClass))
				{
					bool bFillMikro= false;
					bool bFillCorr= false;
					unsigned int nContent= aktualFolder->subroutines[n].resistor.size();
					unsigned int nCorrection= aktualFolder->subroutines[n].correction.size();
					unsigned long time;

					if(aktualFolder->subroutines[n].measuredness == -1)
					{
						TimeMeasure *object= (TimeMeasure*)aktualFolder->subroutines[n].portClass;

						bNewMeasure= true;
						measuredness= object->setNewMeasuredness(m_nMeasurednessCount, nDefaultSleep);
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
						TimeMeasure *port= (TimeMeasure*)aktualFolder->subroutines[n].portClass;
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
						TimeMeasure *port= (TimeMeasure*)aktualFolder->subroutines[n].portClass;

						bNewMeasure= true;
						time= port->getNewMikroseconds(&aktualFolder->subroutines[n].resistor);
						//readFile(sMeasureFile, aktualFolder->subroutines[n].name, "OHM", &time);
						cout << endl;
						cout << "### ending on write mikroseconds from resistance in server.conf" << endl << endl;
					}
					/*if(	aktualFolder->subroutines[n].ohmVector.size() != 2
						&&
						!bFillMikro											)
					{
						string logString("### ERROR: no vector be set in subroutine '");

						logString+= aktualFolder->subroutines[n].name;
						logString+= "' from folder '";
						logString+= aktualFolder->name;
						logString+= "'\n           or vector don't be set correctly";
						LOG(LOG_ERROR, logString);
	#ifndef DEBUG
						cout << logString << endl;
	#endif // DEBUG
					}else
					{
						correctFolder= true;
						correctSubroutine= true;
					}*/
					//if(aktualFolder->subroutines[n].property)
					//	aktualFolder->subroutines[n].property->checkProperties();
				}
			}
			sub* subroutine;

			subroutine= &aktualFolder->subroutines[n];
			if(subroutine->property)
				subroutine->property->checkProperties();
		}
		aktualFolder->bCorrect= correctFolder;
		aktualFolder= aktualFolder->next;
	}
}

Starter::~Starter()
{
	int nMuch, n;
	measurefolder_t *aktualFolder= m_tFolderStart;
	measurefolder_t *deleteFolder;

	while(aktualFolder != NULL)
	{
		deleteFolder= aktualFolder;
		aktualFolder= aktualFolder->next;

		nMuch= deleteFolder->subroutines.size();
		for(n= 0; n<nMuch; n++)
			delete deleteFolder->subroutines[n].portClass;
		delete deleteFolder;
	}
	m_tFolderStart= NULL;
}

/*string Starter::createFullPath(const string relativePath, const string fileName, const string forFile)
{
	if(relativePath.substr(0, 1) == "/")
		return relativePath;
	if(m_sWorkdir == "")
		cout << "WARNING: in file " << fileName << " workdir must be set before define " << forFile << endl;
	return m_sWorkdir + relativePath;
}*/

void Starter::readPasswd()
{
	string defaultUser(m_oServerFileCasher.needValue("defaultuser"));
	string loguser(m_oServerFileCasher.getValue("loguser", /*warning*/false));
	size_t defaultLen= defaultUser.length();
	size_t logLen= loguser.length();
	ifstream file("/etc/passwd");
	string line;
	string buffer;

	if(defaultLen == 0)
	{
		cout << "### ERROR: no default user (defaultuser) in server.conf be set" << endl;
		exit(1);
	}
	if(!file.is_open())
	{
		cout << "### ERROR: cannot read '/etc/passwd'" << endl;
		cout << "           application must be started as root" << endl;
		exit(1);
	}
	m_tDefaultUser= 0;
	m_tLogUser= 0;
	while(!file.eof())
	{
		getline(file, line);

		if(line.substr(0, defaultLen) == defaultUser)
		{
			vector<string> vec(ConfigPropertyCasher::split(line, ":"));

			m_tDefaultUser= atoi(vec[2].c_str());
		}
		if(	logLen > 0
			&&
			line.substr(0, logLen) == loguser	)
		{
			vector<string> vec(ConfigPropertyCasher::split(line, ":"));

			m_tLogUser= atoi(vec[2].c_str());
		}
	}
	if(!m_tDefaultUser)
	{
		cout << "### ERROR: cannot find default user " << defaultUser << endl;
		exit(1);
	}
	if(!m_tLogUser)
	{
		m_tLogUser= m_tDefaultUser;
	}
}

//vector<unsigned long> Starter::readFile(string fileName, string subName/*= ""*/, string wtype/*= ""*/, void *changeValue/*= NULL*/)
void Starter::readFile(vector<unsigned long> &vlRv, string fileName)
{
	static short nFolderID= 0;
	//Properties::param_t pparam;
	string line;
	//string sAktSubName;
	ifstream file(fileName.c_str());
	measurefolder_t *aktualFolder= m_tFolderStart;
	sub *subdir= NULL;
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

			if(	subdir
				&&
				subdir->type != ""	)
/*				(	subdir->type == "SAVE"
					||
					subdir->type == "RESISTANCE"
					||
					subdir->type == "TIMEMEASURE"
					||
					subdir->type == "COUNTER"
					||
					subdir->type == "MEASUREDNESS"
					||
					subdir->type == "SWITCH"
					|| // or is defined for an OWServer instnace
					find(m_vOWServerTypes.begin(), m_vOWServerTypes.end(), subdir->type) != m_vOWServerTypes.end()	)	)*/
			{
				double dSleep;
				string prop;

				if(!subdir->property)
				{
					char cID[20];
					string sFID("_folderID=");

					snprintf(cID, 20, "%d", nFolderID);
					sFID+= cID;
					subdir->property= new ConfigPropertyCasher();
					subdir->property->setDefault("folder", aktualFolder->name);
					subdir->property->setDefault("name", subdir->name);
					subdir->property->readLine(sFID);
					subdir->property->readLine("type="+subdir->type);
					// to get no error if the _folderID not fetch,
					// fetch it now
					subdir->property->getValue("_folderID");
					subdir->property->getValue("type");
				}
				subdir->property->readLine(line);
				if(!subdir->property->newSubroutine().correct)
					continue;
				if(	subdir->type == "GETCONTACT"
					||
					subdir->type == "SWITCHCONTACT"
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
					vector<unsigned long>::iterator portIt;

					if(	subdir->type == "GETCONTACT"
						||
						subdir->type == "TEMP"			)
					{
						need.push_back("in");
						need.push_back("out");
					}else if(subdir->type == "SWITCHCONTACT")
					{
						need.push_back("out");
					}else if(	subdir->type == "TIMEMEASURE"
								||
								subdir->type == "RESISTANCE"	)
					{
						need.push_back("in");
						need.push_back("out");
						need.push_back("neg");
					}

					for(vector<string>::iterator it= need.begin(); it != need.end(); ++it)
					{
						value= subdir->property->getValue(*it, /*warning*/false);
						pin= portBase::getPinsStruct(value);
						if(pin.nPort != 0)
						{
							portIt= find(vlRv.begin(), vlRv.end(), pin.nPort);
							if(portIt == vlRv.end())
								vlRv.push_back(pin.nPort);
							if(	*it == "in"
								&&
								pin.ePin != portBase::NONE	)
							{
								aktualFolder->needInPorts.insert(pin);
							}
						}
					}
				}
				prop= "sleep";
				dSleep= subdir->property->getDouble(prop, /*wrning*/false);
				if(prop != "#ERROR")
				{
					subdir->sleep= (unsigned short)dSleep;
					dSleep-= subdir->sleep;
					dSleep*= 1000000;
					subdir->usleep= (unsigned long)dSleep;
				}
			}
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
			if(type=="file")
			{
				readFile(vlRv, URL::addPath(m_sConfPath, value));

			}else if(type=="measuredness")
			{
				if(subdir)
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

			}else if(type=="folder")
			{
				++nFolderID;
				if(aktualFolder==NULL)
				{
					aktualFolder= new measurefolder_t;
					aktualFolder->next= NULL;
					m_tFolderStart= aktualFolder;
					aktualFolder->name= value;
					aktualFolder->bCorrect= false;
				}else
				{
					if(subdir!=NULL)
					{
						aktualFolder->subroutines.push_back(*subdir);
						delete subdir;
						subdir= NULL;
					}
					aktualFolder= m_tFolderStart;
					while(aktualFolder->next != NULL)
					{
						if(aktualFolder->name == value)
							break;
						aktualFolder= aktualFolder->next;
					}
					if(aktualFolder->name != value)
					{
						aktualFolder->next= new measurefolder_t;
						aktualFolder= aktualFolder->next;
						aktualFolder->next= NULL;
						aktualFolder->name= value;
						aktualFolder->bCorrect= false;
					}
				}
			}else if(type == "name")
			{
				for(unsigned int n= 0; n<names.size(); n++)
				{
					if(names[n] == value)
					{
						cout << "### found ambigous name \"" << value << "\" in " << fileName << endl;
						cout << "### STOP server" << endl;
						exit(0);
					}
				}
				if(subdir!=NULL)
				{
					aktualFolder->subroutines.push_back(*subdir);
					delete subdir;
					subdir= NULL;
				}
				subdir= new sub;
				subdir->name= value;
				subdir->bCorrect= false;
				subdir->type= "";
				subdir->producerBValue= -1;
				subdir->out.nPort= 0x00;
				subdir->out.ePin= portBase::NONE;
				subdir->in.nPort= 0x00;
				subdir->in.ePin= portBase::NONE;
				subdir->defaultValue= 0;
				subdir->negative.nPort= 0x00;
				subdir->negative.ePin= portBase::NONE;
				subdir->sleep= 0;
				subdir->usleep= 0;
				subdir->tmlong= 0;
				subdir->bAfterContact= false;
				subdir->measuredness= 0;
				subdir->property= NULL;
				string result;

			}else if(type == "type")
			{
				subdir->type= value;

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
					if(subdir)
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
					if(subdir)
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
				portBase::portpin_address_t ePortPin;
				vector<string> pin= ConfigPropertyCasher::split(value, ":");
				bool bInsert= true;
				unsigned long port;

				port= portBase::getPortAddress(pin[0]);
				for(unsigned int v= 0; v<vlRv.size(); v++)
				{
					if(vlRv[v]==port)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(port);
				subdir->out.nPort= port;
				subdir->out.ePin= portBase::getPinEnum(pin[1]);
				ePortPin= portBase::getPortPinAddress(subdir->out, false);
				if(	subdir->out.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(pin[0])
					||
					ePortPin.eDescript == portBase::GETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= pin[1] + "' is no correct pin on port '";
					msg+= pin[0] + "'\n    ERROR on line: ";
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
				portBase::portpin_address_t ePortPin;
				vector<string> pin= ConfigPropertyCasher::split(value, ":");
				bool bInsert= true;
				unsigned long port;

				port= portBase::getPortAddress(pin[0]);
				for(unsigned int v= 0; v<vlRv.size(); v++)
				{
					if(vlRv[v]==port)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(port);
				subdir->in.nPort= port;
				subdir->in.ePin= portBase::getPinEnum(pin[1]);
				aktualFolder->needInPorts.insert(subdir->in);
				ePortPin= portBase::getPortPinAddress(subdir->in, false);
				if(	subdir->in.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(pin[0])
					||
					ePortPin.eDescript == portBase::SETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= pin[1] + "' is no correct pin on port '";
					msg+= pin[0] + "'\n    ERROR on line: ";
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
				portBase::portpin_address_t ePortPin;
				vector<string> pin= ConfigPropertyCasher::split(value, ":");
				bool bInsert= true;
				unsigned long port;

				port= portBase::getPortAddress(pin[0]);
				for(unsigned int v= 0; v<vlRv.size(); v++)
				{
					if(vlRv[v]==port)
					{
						bInsert= false;
						break;
					}
				}
				if(bInsert)
					vlRv.push_back(port);
				subdir->negative.nPort= port;
				subdir->negative.ePin= portBase::getPinEnum(pin[1]);
				ePortPin= portBase::getPortPinAddress(subdir->negative, false);
				if(	subdir->negative.ePin == portBase::NONE
					||
					ePortPin.ePort != portBase::getPortType(pin[0])
					||
					ePortPin.eDescript == portBase::GETPIN			)
				{
					string msg("### on subroutine ");

					msg+= subdir->name + ", pin '";
					msg+= pin[1] + "' is no correct pin on port '";
					msg+= pin[0] + "'\n    ERROR on line: ";
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

			}else if(type == "sleep")
			{
				double dSleep= atof(value.c_str());

				subdir->sleep= (unsigned short)dSleep;
				dSleep-= subdir->sleep;
				dSleep*= 1000000;
				subdir->usleep= (unsigned long)dSleep;

			}else if(type == "after")
			{
				if(	value=="true"
					||
					value=="TRUE"	)
				{
					subdir->bAfterContact= true;
				}

			}else if(type == "port")
			{
				//toDo: implement for type PROTOCOL
			}else if(type == "file")
			{
				//toDo: implement for type PROTOCOL

			}else if(type == "default")
			{
				subdir->defaultValue= atof(&value[0]);

			}else if(	line != ""
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
				if(subdir != NULL)
				{
					msg+= "\n    with type-name '" + subdir->name + "'";
				}
				cout << msg << endl;
			}

			lines.push_back(line);
		}// end of while(!file.eof())
		if(	subdir != NULL
			&&
			subdir->sleep == 0
			&&
			subdir->usleep == 0	)
		{
			string prop("sleep");
			double dSleep= subdir->property->getDouble(prop, /*wrning*/false);

			if(prop != "#ERROR")
			{
				subdir->sleep= (unsigned short)dSleep;
				dSleep-= subdir->sleep;
				dSleep*= 1000000;
				subdir->usleep= (unsigned long)dSleep;
			}
		}
	}else
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

	if(subdir != NULL)
	{
		aktualFolder->subroutines.push_back(*subdir);
		delete subdir;
	}
/*	if(subName != "")
	{
		ofstream ofile(getChars(fileName));
		int nCount= lines.size();

		for(int n= 0; n<nCount; n++)
		{
			ofile << lines[n] << endl;
		}
	}
	return vlRv;*/
}

void Starter::checkAfterContact()
{
	unsigned int nSize;
	measurefolder_t *aktfolder;
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
			measurefolder_t *search= m_tFolderStart;
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

bool Starter::command(vector<string> options, string command)
{
	bool clres;
	unsigned short nPort;
	string fileName, prop;
	string confpath, logpath, sLogLevel, property;
	SocketClientConnection* clientCon;
	vector<string>::iterator opIt;

	Starter::isNoPathDefinedStop();
	opIt= find(options.begin(), options.end(), "-f");
	if(opIt != options.end())
	{
		fileName= opIt->substr(3);
		options.erase(opIt);
	}
	confpath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(confpath, "server.conf");
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

#if 0
	logpath= URL::addPath(m_sWorkdir, PPILOGPATH, /*always*/false);
	logpath= URL::addPath(logpath, "ppi-server_");
	log= LogThread::instance(true);
	log->setThreadName("client-log");
	property= "log";
	sLogLevel= m_oServerFileCasher.getValue(property);
	if(sLogLevel == "DEBUG")
		nLogLevel= LOG_DEBUG;
	else if(sLogLevel == "INFO")
		nLogLevel= LOG_INFO;
	else if(sLogLevel == "SERVER")
		nLogLevel= LOG_SERVER;
	else if(sLogLevel == "WARNING")
		nLogLevel= LOG_WARNING;
	else if(sLogLevel == "ERROR")
		nLogLevel= LOG_ERROR;
	else if(sLogLevel == "ALERT")
		nLogLevel= LOG_ALERT;
	else
	{
		cerr << "### WARNING: undefined log-level in config file server.conf" << endl;
		cerr << "             set log-level to DEBUG" << endl;
		nLogLevel= LOG_DEBUG;
	}
	property= "timelogSec";
	nLogAllSec= m_oServerFileCasher.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	property= "newfileAfterDays";
	nLogDays= m_oServerFileCasher.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogDays= 30;
	}
	log->setProperties(logpath, nLogLevel, nLogAllSec, nLogDays);
	log->start();
#endif

	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		return false;

	command= ConfigPropertyCasher::trim(command);
	clientCon= new SocketClientConnection(SOCK_STREAM, "127.0.0.1", nPort, 10, new ClientTransaction(options, command));
	clres= clientCon->init();
	delete clientCon;
	return clres;
}

bool Starter::stop(vector<string> options)
{
	char	buf[64];
	string  sendbuf("GET\n");
	char	stopServer[]= "stop-server\n";
	FILE 	*fp;
	int clientsocket;
	unsigned int nOptions= options.size();
	unsigned short nPort;
	string result;
	string fileName;
	string confpath, logpath, sLogLevel, property, username;
	UserManagement* user= UserManagement::instance();


	bool bOp= true;
	Starter::isNoPathDefinedStop();
	for(unsigned int o= 0; o<nOptions; ++o)
	{
		if(options[o].substr(0, 2) == "-f")
			fileName= options[o].substr(3);
		else
			bOp= false;
	}
	if(!bOp)
	{
		cerr << "### WARNING: not all options are for all commands," << endl;
		cerr << "             see -? for help" << endl;
	}
	confpath= URL::addPath(m_sWorkdir, PPICONFIGPATH, /*always*/false);
	fileName= URL::addPath(confpath, "server.conf");
	if(!m_oServerFileCasher.readFile(fileName))
	{
		cout << "### ERROR: cannot read '" << fileName << "'" << endl;
		exit(1);
	}

#if 0
	logpath= URL::addPath(m_sWorkdir, PPILOGPATH, /*always*/false);
	logpath= URL::addPath(logpath, "ppi-server_");
	log= LogThread::instance();
	log->setThreadName("main--stop-server");
	property= "log";
	sLogLevel= m_oServerFileCasher.getValue(property);
	if(sLogLevel == "DEBUG")
		nLogLevel= LOG_DEBUG;
	else if(sLogLevel == "INFO")
		nLogLevel= LOG_INFO;
	else if(sLogLevel == "SERVER")
		nLogLevel= LOG_SERVER;
	else if(sLogLevel == "WARNING")
		nLogLevel= LOG_WARNING;
	else if(sLogLevel == "ERROR")
		nLogLevel= LOG_ERROR;
	else if(sLogLevel == "ALERT")
		nLogLevel= LOG_ALERT;
	else
	{
		cerr << "### WARNING: undefined log-level in config file server.conf" << endl;
		cerr << "             set log-level to DEBUG" << endl;
		nLogLevel= LOG_DEBUG;
	}
	property= "timelogSec";
	nLogAllSec= m_oServerFileCasher.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogAllSec= 1800;
	}
	property= "newfileAfterDays";
	nLogDays= m_oServerFileCasher.getInt(property);
	if(	nLogAllSec == 0
		&&
		property == "#ERROR"	)
	{
		nLogDays= 30;
	}
	log->setProperties(logpath, nLogLevel, nLogAllSec, nLogDays);
#endif
	if(user == NULL)
	{
		if(!UserManagement::initial(URL::addPath(confpath, "access.conf", /*always*/true)))
			return false;
		user= UserManagement::instance();
	}

	property= "port";
	nPort= m_oServerFileCasher.needUShort(property);
	if(property == "#ERROR")
		exit(EXIT_FAILURE);
	clientsocket= ServerThread::connectAsClient("127.0.0.1", nPort);
	LOG(LOG_INFO, "any user is stopping server");
	if(clientsocket==0)
	{
		LOG(LOG_INFO, "no server is running");
#ifndef DEBUG
		printf("no server is running\n");
#endif // DEBUG
		return false;
	}
	fp = fdopen (clientsocket, "w+");

	fputs (sendbuf.c_str(), fp);
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

	if(!user->rootLogin())
	{
		username= user->getNoRootUser();
		sendbuf= "U:";
		sendbuf+= username + ":";
		sendbuf+= user->getPassword(username);
		sendbuf+= "\n";
		fputs(sendbuf.c_str(), fp);
		fflush(fp);
		fgets(buf, sizeof(buf), fp);

		sendbuf= "CHANGE ";
	}else
		sendbuf= "U:";
	username= user->getRootUser();
	sendbuf+= username + ":";
	sendbuf+= user->getPassword(username);
	sendbuf+= "\n";
	fputs(sendbuf.c_str(), fp);
	fflush(fp);
	fgets(buf, sizeof(buf), fp);

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
			LOG(LOG_SERVER, "ERROR: lost connection to server\n       maybe server always running");
			break;
		}
		cout << "." << flush;
	}
	fclose(fp);
	close(clientsocket);
	if(!strcmp(buf, "OK\n"))
	{
#ifdef DEBUG
		printf("\nserver was stopped\n");
#endif // DEBUG
		LOG(LOG_SERVER, "server was stopped");
	}
	return true;
}

void Starter::signalconverting(int nSignal)
{
	string msg;
	LogInterface *log= LogInterface::instance();

	if(	log
		&&
		!log->running()	)
	{
		log= NULL;
	}
	switch(nSignal)
	{
		case SIGINT:
			cout << Thread::getStatusInfo("clients") << endl << endl;
			if(log)
			{
				cout << "server terminated by user" << endl;
				LOG(LOG_SERVER, "server terminated by user");
			}else
				printf("\nserver terminated by user\n\n");
			exit(0);
			break;

		case SIGHUP:
			msg= Thread::getStatusInfo("");
			if(log)
				LOG(LOG_INFO, msg);
			cout << endl << msg << endl;
			break;

		case SIGSEGV:
			cout << Thread::getStatusInfo("clients") << endl << endl;
			if(log)
			{
				cout << "application close from system" << endl;
				LOG(LOG_SERVER, "application close from system");
				cout << "logged" << endl;
			}else
				printf("\napplication close from system - no logging\n\n");
			exit(0);
			break;
	}
}

void Starter::printSigError(const string cpSigValue)
{
	string msg;

	msg= "cannot initial signal \"";
	msg+= cpSigValue;
	msg+= "\"\nSystem-ERROR: ";
	msg+= strerror(errno);
	LOG(LOG_ERROR, msg);
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
