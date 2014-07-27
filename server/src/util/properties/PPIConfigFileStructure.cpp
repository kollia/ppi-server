/**
 *   This file 'PPIConfigFileStructure.cpp' is part of ppi-server.
 *   Created on: 30.05.2014
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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/regex.hpp>

#include "PPIConfigFileStructure.h"

#include "../GlobalStaticMethods.h"

#include "../../database/logger/lib/logstructures.h"
#include "../../pattern/util/LogHolderPattern.h"

namespace util
{
	using namespace boost;

	SHAREDPTR::shared_ptr<PPIConfigFileStructure>
		PPIConfigFileStructure::__instance= SHAREDPTR::shared_ptr<PPIConfigFileStructure>();

	void PPIConfigFileStructure::init(const string& workingDir, const bool bFirst)
	{
		if(__instance == NULL)
		{
			__instance= PPIConfigFiles(new PPIConfigFileStructure(workingDir, bFirst));
			if(__instance == NULL)
			{
				cerr << "ERROR: cannot create instance of PPIConfigFileStructure" << endl;
				cerr << "       so do not start process of '" << glob::getProcessName() << "'" << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	void PPIConfigFileStructure::readServerConfig()
	{
		string fileName;

		fileName= URL::addPath(m_sConfigPath, m_sServerConf, /*always*/false);
		m_oServerFileCasher.setDelimiter("owreader", "[", "]");
		m_oServerFileCasher.modifier("owreader");
		m_oServerFileCasher.readLine("workdir= " + m_sConfigPath);
		if(!m_oServerFileCasher.readFile(fileName))
		{
			cerr << "### ALERT: cannot read '" << fileName << "'" << endl;
			exit(EXIT_FAILURE);
		}
	}

	set<string> PPIConfigFileStructure::getPortIntercfaceNames()
	{
		set<string> sRv;
		vector<string>::size_type readercount;

		readercount= m_oServerFileCasher.getPropertyCount("owreader");
		for(vector<string>::size_type owreaders= 0; owreaders < readercount; ++owreaders)
		{
			string owreader(m_oServerFileCasher.getValue("owreader", owreaders));

			sRv.insert(owreader);
			if(owreader == "PORT")
				sRv.insert("MPORT");
		}
		return sRv;
	}

	bool PPIConfigFileStructure::needInformThreads()
	{
		static bool bFirst(true);
		bool bRv;
		string res;

		/*
		 * doDo: implementation of running informer threads
		 *       controllable from server.conf
		 * ########################################
		 * # informing measure folders
		 * # for better performance
		 * # over extra threads
		 * informer_threads= true
		 *
		 * threads are implemented finished to start
		 * and also reading flag from server.conf OK
		 * running a while, than functionality
		 * of server stopping
		 * folder thread should running in this case
		 * when flag true faster
		 * have now no time to search where the bug
		 */
		return false;

		res= m_oServerFileCasher.getValue("informer_threads");
		if(res == "")
		{
			bRv= false;
			if(bFirst)
			{
				LOG(LOG_WARNING, "no property informer_threads is defined\n"
								"so do not create any extra thread for folder informing");
				bFirst= false;
			}
		}else
		{
			if(res == "true")
				bRv= true;
			else if(res == "false")
				bRv= false;
			else
			{
				bRv= false;
				if(bFirst)
				{
					LOG(LOG_WARNING, "cannot recognize defined value '" + res + "' for property informer_threads\n"
								"so do not create any extra thread for folder informing");
					bFirst= false;
				}
			}
		}
		return bRv;
	}

	short PPIConfigFileStructure::getFolderDbThreads()
	{
		static bool bFirst(true);
		short nRv;
		string res;

		res= m_oServerFileCasher.getValue("folder_db_threads");
		if(res == "")
		{
			nRv= 1;
			if(bFirst)
			{
				LOG(LOG_WARNING, "no property folder_db_thread is defined\n"
								"so create ONE thread for all folders");
				bFirst= false;
			}
		}else
		{
			if(res == "DIRECT")
				nRv= -1;
			else if(res == "ONE")
				nRv= 1;
			else if(res == "EVERY")
				nRv= 0;
			else
			{
				nRv= 1;
				if(bFirst)
				{
					LOG(LOG_WARNING, "cannot read property folder_db_thread\n"
									"readed entry '" + res + "' is not defined correctly\n"
									"so create ONE thread for all folders");
					bFirst= false;
				}
			}
		}
		return nRv;
	}

	bool PPIConfigFileStructure::startDbServer()
	{
		bool bDb;
		string property;

		property= m_oServerFileCasher.getValue("databaseserver", /*warning*/true);
		if(	property == ""
			||
			(	property != "true"
				&&
				property != "false"	)	)
		{
			if(m_bFirstReading)
			{
				cerr << "###          parameter databases erver not be set correctly," << endl;
				cerr << "             so start server" << endl;
			}
			bDb= true;

		}else if(property == "true")
			bDb= true;
		else
			bDb= false;
		return bDb;
	}

	bool PPIConfigFileStructure::startPortInterfaces()
	{
		bool bPorts;
		string property;

		property= m_oServerFileCasher.getValue("owreaders", /*warning*/true);
		if(	property == "" ||
			(	property != "true" &&
				property != "false"		)	)
		{
			if(m_bFirstReading)
			{
				cerr << "###          parameter owreaders not be set correctly," << endl;
				cerr << "             so start server" << endl;
			}
			bPorts= true;
		}else if(property == "true")
			bPorts= true;
		else
			bPorts= false;
		return bPorts;
	}

	bool PPIConfigFileStructure::startInternetServer()
	{
		bool bInternet;
		string property;

		property= m_oServerFileCasher.getValue("internetserver", /*warning*/true);
		if(	property == ""
			||
			(	property != "true"
				&&
				property != "false"	)	)
		{
			if(m_bFirstReading)
			{
				cerr << "###          parameter internet server not be set correctly," << endl;
				cerr << "             so start server" << endl;
			}
			bInternet= true;

		}else if(property == "true")
			bInternet= true;
		else
			bInternet= false;
		return bInternet;
	}

	time_t PPIConfigFileStructure::getPortSearchInterval()
	{
		time_t nRv;
		int interval;
		string property("serversearch");

		interval= m_oServerFileCasher.getInt(property, /*warning*/true);
		if(	property == "#ERROR" ||
			interval <= 0	)
		{
			if(m_bFirstReading)
			{
				cerr << "###          parameter serversearch not be set correctly," << endl;
				cerr << "             set as default to 15 seconds" << endl;
			}
			nRv= 15;
		}else
			nRv= static_cast<time_t>(interval);
		return nRv;
	}

	string PPIConfigFileStructure::getCommunicationHost()
	{
		string host;

		host= m_oServerFileCasher.getValue("communicationhost", /*warning*/false);
		if(host == "")
		{
			host= "127.0.0.1";
			if(m_bFirstReading)
			{
				cerr << "###          parameter communicationhost not be set correctly," << endl;
				cerr << "             set as default localhost" << endl;
			}
		}
		return host;
	}

	string PPIConfigFileStructure::getInternetHost()
	{
		string host;

		// when no listen parameter be set
		// server should hear on all IP adresses
		// and host is an null string
		host= m_oServerFileCasher.getValue("listen", /*warning*/false);
		return host;
	}

	unsigned short PPIConfigFileStructure::getCommunicationPort()
	{
		unsigned short port;
		string property("communicationport");

		port= m_oServerFileCasher.needUShort(property);
		if(	port == 0 &&
			property == "#ERROR"	)
		{
			port= 20005;
			if(m_bFirstReading)
			{
				cerr << "###          parameter communicationport not be set correctly," << endl;
				cerr << "             set as default port 20005" << endl;
			}
		}
		return port;
	}

	unsigned short PPIConfigFileStructure::getInternetPort()
	{
		unsigned short port;
		string property("port");

		port= m_oServerFileCasher.needUShort(property);
		if(	port == 0 &&
			property == "#ERROR"	)
		{
			port= 20004;
			if(m_bFirstReading)
			{
				cerr << "###          parameter port not be set correctly," << endl;
				cerr << "             set as default port 20004" << endl;
			}
		}
		return port;
	}

	pair<short, short> PPIConfigFileStructure::getCPUtimeSplitting()
	{
		pair<short, short> pRv;
		string property;

		property= "folderlength_split";
		pRv.first= m_oServerFileCasher.getInt(property);
		if(	property == "#ERROR" ||
			pRv.first <= 0 ||
			pRv.first > 100 ||
			(100 % pRv.first) != 0	)
		{
			string err1("parameter folderlength_split not be set correctly,\n");
			string err2("or not modular to 100, so do not differ between CPU times");

			cerr << "###          " << err1;
			cerr << "             " << err2 << endl;
			LOG(LOG_ERROR, err1 + err2);
			pRv.first= 100;
		}
		property= "finishedtime_split";
		pRv.second= m_oServerFileCasher.getInt(property);
		if(	property == "#ERROR" ||
			pRv.second <= 0 ||
			pRv.second > 100 ||
			(100 % pRv.second) != 0	)
		{
			string err1("parameter finishedtime_split not be set correctly,\n");
			string err2("or not modular to 100, so do not differ between CPU times");

			cerr << "###          " << err1;
			cerr << "             " << err2 << endl;
			LOG(LOG_ERROR, err1 + err2);
			pRv.second= 100;
		}
		return pRv;
	}

	int PPIConfigFileStructure::getTimerLogSeconds()
	{
		int nRv;
		string property("timelogSec");

		nRv= m_oServerFileCasher.getInt(property);
		if(	nRv == 0 &&
			property == "#ERROR"	)
		{
			nRv= 1800;
			if(m_bFirstReading)
			{
				cerr << "###          parameter timerlogSec not be set correctly," << endl;
				cerr << "             set as default to 1800 seconds" << endl;
			}
		}
		return nRv;
	}

	const IPropertyPattern* PPIConfigFileStructure::getExternalPortProperties(const string& port)
	{
		const IPropertyPattern* pProp;
		set<string> vFirstDefined;
		set<string>::iterator found;

		pProp= m_oServerFileCasher.getSection("owreader", port);
		found= find(vFirstDefined.begin(), vFirstDefined.end(), port);
		if(	!pProp &&
			m_bFirstReading	&&
			found == vFirstDefined.end()	)
		{
			cerr << "### WARNING: no external port inside clamps, like '[" << port << "]'," << endl;
			cerr << "             be defined inside " << m_sServerConf << endl;
			cerr << "             so cannot run subroutines with this TYPE" << endl;
		}
		vFirstDefined.insert(port);
		return pProp;
	}

	string PPIConfigFileStructure::getDefaultUser()
	{
		string user;

		user= m_oServerFileCasher.needValue("defaultuser");
		if(user == "")
		{
			cout << "### ALERT: no default user (defaultuser) in server.conf be set" << endl;
			exit(EXIT_FAILURE);
		}
		return user;
	}

	string PPIConfigFileStructure::getExternalPortUser()
	{
		string user;

		user= m_oServerFileCasher.getValue("defaultextuser");
		if(user == "")
		{
			user= getDefaultUser();
			cout << "### WARNING: no default user (defaultextuser) for external ports be defined" << endl;
			cout << "             take default user '" << user << "' from server" << endl;
		}
		return user;
	}

	string PPIConfigFileStructure::getLogLevelName()
	{
		static bool bfirst(true);
		string level;

		level= m_oServerFileCasher.getValue("log", /*warning*/false);
		if(level == "")
		{
			if(	bfirst &&
				m_bFirstReading	)
			{
				cout << "### WARNING: no logging level defined inside " << m_sServerConf << endl;
				cout << "             so take DEBUG as default logging level" << endl;
				bfirst= false;
			}
			level= "DEBUG";
		}
		return level;
	}

	int PPIConfigFileStructure::getLogLevel()
	{
		int nRv;
		string level;

		level= getLogLevelName();
		if(level == "DEBUG")
			nRv= LOG_DEBUG;
		else if(level == "INFO")
			nRv= LOG_INFO;
		else if(level == "WARNING")
			nRv= LOG_WARNING;
		else if(level == "ERROR")
			nRv= LOG_ERROR;
		else if(level == "ALERT")
			nRv= LOG_ALERT;
		return nRv;
	}

	string PPIConfigFileStructure::getPasswdFile()
	{
		string passwd;
		ifstream file;

		passwd= m_oServerFileCasher.needValue("passwd");
		if(passwd == "")
		{
			cerr << "### ALERT: no default passwd file for all users on system is defined in "
							<< m_sServerConf << endl;
			cerr << "           so hole server cannot change running user" << endl;
			cerr << "           for this reason do not start server" << endl;
			exit(EXIT_FAILURE);
		}
		file.open(passwd.c_str());
		if(!file.is_open())
		{
			cerr << "### ALERT: given passwd file '" << passwd << "' inside " << m_sServerConf
							<< " cannot open for reading" << endl;
			cerr << "           so hole server cannot change running user" << endl;
			cerr << "           for this reason do not start server" << endl;
			exit(EXIT_FAILURE);
		}
		file.close();
		return passwd;
	}

	bool PPIConfigFileStructure::readMeasureConfig()
	{
		typedef vector<IInterlacedPropertyPattern*>::iterator secIt;

		bool bInObj(false);
		unsigned short nFolderID(0), nObjFolderID(0);
		string configFile;
		string firstObjFolder;
		string modifier, value;
		string curObj("NULL");
		vector<string> objFolders;
		auto_ptr<sub> subdir;
		ActionProperties *pProperty;
		InterlacedActionProperties mainprop(/*check after*/true);
		vector<IInterlacedPropertyPattern*> objSections;
		vector<IInterlacedPropertyPattern*> folderSections;
		vector<IInterlacedPropertyPattern*> *pFolderSections;
		vector<IInterlacedPropertyPattern*> subSections;
		SHAREDPTR::shared_ptr<measurefolder_t> aktualFolder= m_tFolderStart;
		SHAREDPTR::shared_ptr<measurefolder_t> pFirstObjFolder;
		SHAREDPTR::shared_ptr<IActionPropertyPattern> pHold1OBJfolderprops;
		secIt oit, fit, defObj;

		configFile= URL::addPath(m_sConfigPath, m_sMeasureConf, /*always*/false);
		mainprop.allowLaterModifier(true);
		mainprop.action("action");
		// object name shouldn't be shown by error, so do not define
		mainprop.modifier("object");// an setMsgParameter for modifier
		mainprop.modifier("folder");
		mainprop.setMsgParameter("folder");
		mainprop.modifier("name");
		mainprop.setMsgParameter("name", "subroutine");
		mainprop.valueLocalization("\"", "\"", /*remove*/true);
		if(!mainprop.readFile(configFile))
			return false;
		objSections= mainprop.getSections();
		// first can be the folder section
		// also be an object section
		pFolderSections= &objSections;
		oit= objSections.begin();
		fit= oit;
		do{
			if(fit == pFolderSections->end())
			{
				if(!bInObj)
					break;
				if(curObj == "NULL")
				{
					++oit;
					if(oit == objSections.end())
						break;
					fit= oit;

				}else
				{
					// fill all folders from object
					aktualFolder= m_tFolderStart;
					while(aktualFolder->next != NULL)
					{
						if(aktualFolder->name == firstObjFolder)
							break;
						aktualFolder= aktualFolder->next;
					}
					//cout << "copy all folders from first object folder " << aktualFolder->name << endl;
					pFirstObjFolder= aktualFolder;
					pFirstObjFolder->vsObjFolders= objFolders;
					aktualFolder= aktualFolder->next;
					while(aktualFolder  != NULL)
					{
						if(aktualFolder->bDefined == false)
						{
							//cout << "fill object folder " << aktualFolder->name << endl;
							aktualFolder->bCorrect= false;
							pProperty= new ActionProperties;
							*pProperty= *dynamic_cast<ActionProperties*>(pFirstObjFolder->folderProperties.get());
							pProperty->add(*dynamic_cast<ActionProperties*>(aktualFolder->folderProperties.get()));
							aktualFolder->folderProperties= SHAREDPTR::shared_ptr<IActionPropertyPattern>(pProperty);
							aktualFolder->subroutines= pFirstObjFolder->subroutines;
							pFirstObjFolder->vsObjFolders= objFolders;
							aktualFolder->bDefined= true;
						}
						aktualFolder= aktualFolder->next;
					}
					if(pHold1OBJfolderprops != NULL)
					{
						// fill now properties from 1 folder
						// defined by reading real 1 folder
						// into aktual 1 folder object
						pProperty= dynamic_cast<ActionProperties*>(pFirstObjFolder->folderProperties.get());
						pProperty->add(*dynamic_cast<ActionProperties*>(pHold1OBJfolderprops.get()));
						pHold1OBJfolderprops= SHAREDPTR::shared_ptr<IActionPropertyPattern>();
					}

					// goto new object
					++oit;
					fit= oit;
					bInObj= false;
					curObj= "NULL";
					modifier= (*fit)->getSectionModifier();
					while(	modifier == "object" &&
							(*fit)->getSectionValue() == "NULL"	)
					{
						folderSections= (*fit)->getSections();
						if(folderSections.size() > 0)
						{
							pFolderSections= &folderSections;
							fit= folderSections.begin();
							bInObj= true;
						}else
						{
							++oit;
							fit= oit;
						}
						modifier= (*fit)->getSectionModifier();
					}
				}
			}
			modifier= (*fit)->getSectionModifier();
			//cout << "create " << modifier << " with " << (*fit)->getSectionValue() << endl;
			if(modifier == "object")
			{
				bInObj= true;
				curObj= (*fit)->getSectionValue();
				glob::replaceName(curObj, "object name");
				objFolders.clear();
				folderSections= (*oit)->getSections();
				pFolderSections= &folderSections;
				fit= pFolderSections->begin();
				modifier= (*fit)->getSectionModifier();
				firstObjFolder= (*fit)->getSectionValue();
				nObjFolderID= 0;
			}
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
				// create new folder
				++nFolderID;
				++nObjFolderID;
				value= (*fit)->getSectionValue();
				glob::replaceName(value, "folder name");
				if(	curObj != "NULL" &&
					value == curObj		)
				{
					value= firstObjFolder;
				}else
					objFolders.push_back(value);
				if(m_tFolderStart == NULL)
				{
					aktualFolder= SHAREDPTR::shared_ptr<measurefolder_t>(new measurefolder_t);
					m_tFolderStart= aktualFolder;
					aktualFolder->name= value;
					aktualFolder->bCorrect= false;
					aktualFolder->bDefined= false;
					aktualFolder->nFolderID= nFolderID;
					if(curObj != "NULL")
					{
						aktualFolder->nObjectID= nObjFolderID;
						aktualFolder->sObject= curObj;
					}else
						aktualFolder->nObjectID= 0;
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

						if(aktualFolder->bDefined)
						{
							warn=  "### WARNING: found second folder name '" + value + "'\n";
							warn+= "             and change all subroutines to new folder!";
							cout << warn << endl;
							LOG(LOG_WARNING, warn);
						}

					}else
					{
						aktualFolder->next= SHAREDPTR::shared_ptr<measurefolder_t>(new measurefolder_t);
						aktualFolder= aktualFolder->next;
						aktualFolder->name= value;
						aktualFolder->bCorrect= false;
						aktualFolder->bDefined= false;
						aktualFolder->nFolderID= nFolderID;
						if(curObj != "NULL")
						{
							aktualFolder->nObjectID= nObjFolderID;
							aktualFolder->sObject= curObj;
						}else
							aktualFolder->nObjectID= 0;
					}
				}
				pProperty= new ActionProperties;
				*pProperty= *dynamic_cast<ActionProperties*>(*fit);
				if(	!aktualFolder->bDefined &&
					curObj != "NULL" &&
					aktualFolder->folderProperties != NULL	)
				{// folder is first folder of an object,
				 // so hold folder properties from actual folder to fill in by filling objects
					pHold1OBJfolderprops= aktualFolder->folderProperties;
				}
				aktualFolder->folderProperties= SHAREDPTR::shared_ptr<IActionPropertyPattern>(pProperty);
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
								for(vector<pair<string, PortTypes> >::iterator it= m_vPortTypes.begin();
												it != m_vPortTypes.end(); ++it)
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
									m_vPortTypes.push_back(pair<string, PortTypes>(port, act));
							}
							/************************************************************/
							if(buse)
							{
								ostringstream sFID, sOID;

								pProperty= new ActionProperties;
								*pProperty= *dynamic_cast<ActionProperties*>(*sit);
								subdir->property= SHAREDPTR::shared_ptr<IActionPropertyPattern>(pProperty);
								subdir->name= value;
								subdir->bCorrect= true;	// set first subroutine to correct,
														// because later by wrong initial will be set to false
								/************************************************************\
								 * define values for subroutine witch should be obsolete
								\************************************************************/
								subdir->producerBValue= -1;
								subdir->defaultValue= 0;
								subdir->tmlong= 0;
								subdir->bAfterContact= false;
								subdir->measuredness= 0;
								/************************************************************/

								aktualFolder->subroutines.push_back(*subdir.get());
							}
						}
					}// modifier is subroutine
				}// iterate subroutines of folder
				if(!subSections.empty())
					aktualFolder->bDefined= true;
				if(aktualFolder->nFolderID != nFolderID)
				{//first folder of object was defined, do not count object name
					--nFolderID;//so count ID one back
				}
			}// modifier is folder
			++fit;
			if(!bInObj)
				++oit;
		}while(oit != objSections.end());
		return true;


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

	vector<sub>* PPIConfigFileStructure::getFolderList(const string& folder)
	{
		SHAREDPTR::shared_ptr<measurefolder_t> currentFolder(m_tFolderStart);

		while(currentFolder != NULL)
		{
			if(currentFolder->name == folder)
				return &currentFolder->subroutines;
			currentFolder= currentFolder->next;
		}
		return NULL;
	}

	sub* PPIConfigFileStructure::getSubroutine(const string& folder, const string& subroutine)
	{
		vector<sub>* subs;

		subs= getFolderList(folder);
		if(subs == NULL)
			return NULL;
		for(vector<sub>::iterator it= subs->begin(); it != subs->end(); ++it)
		{
			if(it->name == subroutine)
				return &(*it);
		}
		return NULL;
	}

	SHAREDPTR::shared_ptr<IActionPropertyPattern> PPIConfigFileStructure::
		getSubroutineProperties(const string& folder, const string& subroutine)
	{
		sub* oSubroutine;
		SHAREDPTR::shared_ptr<IActionPropertyPattern> prop;

		oSubroutine= getSubroutine(folder, subroutine);
		if(oSubroutine == NULL)
			return prop;
		return oSubroutine->property;
	}

	string PPIConfigFileStructure::createCommand(const string& folder,
					const string& subroutine, const string& def, string command)
	{
		smatch what;
		regex exp("([^${}]*(\\$\\{([^${}]+)\\})[^${}]*)+");
		SHAREDPTR::shared_ptr<measurefolder_t> pFolder;
		map<string, string> mSubroutineAlias;

		if(command == "")
			return "";
		pFolder= m_tFolderStart;
		while(	pFolder != NULL &&
				pFolder->name != folder	)
		{
			pFolder= pFolder->next;
		}
		if(pFolder != NULL)
		{
			string inp;
			vector<string> spl;
			vector<string>::size_type nAliasCount;
			map<string, string>::iterator found;

			nAliasCount= pFolder->folderProperties->getPropertyCount("subvar");
			for(vector<string>::size_type n= 0; n < nAliasCount; ++n)
			{
				inp= pFolder->folderProperties->getValue("subvar", n);
				split(spl, inp, is_any_of("="));
				if(spl.size() == 2)
				{// when not set correct alias, write error inside folder configuration by starting
					trim(spl[0]);
					trim(spl[1]);
					mSubroutineAlias.insert(pair<string, string>(spl[0], spl[1]));
				}
			}
		}else
		{
			string err("inside '" + folder + ":" + subroutine
							+ "' for replacing subvar in "+ def + " '"
							+ command + " will be not found inside working list");

			cerr << "### ALERT: " << err << endl;
			LOG(LOG_ALERT, err);
			return command;
		}

		for(map<string, string>::const_iterator it= mSubroutineAlias.begin(); it != mSubroutineAlias.end(); ++it)
		{
			string result;
			regex pattern("\\$\\{[ \t]*" + it->first + "[ \t]*\\}");

			command= regex_replace(command, pattern, it->second);
		}
		if(regex_match(command, what, exp))
		{
			string msg1, msg2;

			msg1= "inside '" + folder + ":" + subroutine + "' for " + def + " '" + command + "'";
			msg2=  "predefined subvar variable ${" + what.str(what.size() -1);
			msg2+= "} isn't defined";
			LOG(LOG_WARNING, msg1 + "\n" + msg2);
			cerr << "### WARNING: " << msg1 << endl;
			cerr << "             " << msg2 << endl;
		}
		return command;
	}


} /* namespace util */
