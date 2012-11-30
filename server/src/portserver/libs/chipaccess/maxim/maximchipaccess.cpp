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

#ifdef _OWFSLIBRARY

#include <errno.h>
#include <stdio.h>
#include <owcapi.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>


#include "../../../../util/URL.h"

#include "../../../../util/properties/configpropertycasher.h"

#include "../../../../pattern/util/LogHolderPattern.h"

#include "../../../../database/lib/DbInterface.h"

#include "maximchipaccess.h"

using namespace util;
using namespace std;
using namespace ppi_database;

namespace ports
{
	IChipAccessPattern* OWFSFactory(const string& need, const unsigned short actID,
									size_t& count, const IPropertyPattern* properties)
	{
		if(need == "needprocesses")
		{
			count= properties->getPropertyCount("maximinit");
			return NULL;
		}
		if(need == "chipobject")
		{
			string init(properties->getValue("maximinit", count-1));
			vector<string> adapters;
			MaximChipAccess* object;

			object= new MaximChipAccess(init, &adapters);
			return object;
		}
		return NULL;
	}

	short MaximChipAccess::m_nInterface= -1;
	set<string> MaximChipAccess::m_vsIncorrect;
	pthread_mutex_t* MaximChipAccess::m_INCORRECTCHIPIDS= NULL; //Thread::getMutex("INCORRECTMAXIMCHIPIDS");

	MaximChipAccess::MaximChipAccess(const string init/*= ""*/, vector<string>* idsBefore/*= NULL*/)
	{
		m_bDebug= false;
		m_bConnected= false;
		m_sInit= init;
		m_DEBUGINFO= Thread::getMutex("MAXIMDEBUGINFO");
		if(m_INCORRECTCHIPIDS == NULL)
			m_INCORRECTCHIPIDS= Thread::getMutex("INCORRECTMAXIMCHIPIDS");
		if(idsBefore)
			m_vsBefore= *idsBefore;
		++m_nInterface;
	}

	void MaximChipAccess::usePropActions(const IActionPropertyPattern* prop) const
	{
		prop->getValue("ID");
		prop->getValue("pin");
		prop->getValue("priority", /*warning*/false);
		prop->getValue("cache", /*warning*/false);
		prop->haveAction("read");
		prop->haveAction("write");
		prop->haveAction("writecache");
		prop->haveAction("cache");
		prop->haveAction("current");
		prop->haveAction("db");
		prop->haveAction("perm");
	}

	bool MaximChipAccess::init(const IPropertyPattern* properties)
	{
		string adapter, msg;
		vector<string> ids;
		vector<string>::size_type nAdb= 0;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::iterator condIt;

		if(!isConnected())
		{
			// fetch all adapter from properties,
			// to get no warning after objectcreation from check
			nAdb= properties->getPropertyCount("maximadapter");
			for(vector<string>::size_type c= 0; c < nAdb; ++c)
				adapter= properties->getValue("maximadapter", c, false);
			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			m_pProp= properties;
			if(m_sInit == "")
			{
				m_sInit= properties->needValue("maximinit");
				if(m_sInit == "")
					return false;
			}
			if(!connect())// when connection was fault
				return true; // return true, because server should try again later
		}
		ids= getChipIDs();
		if(!ids.size())
		{
			return false;
		}

		for(vector<string>::iterator o= ids.begin(); o != ids.end(); ++o)
		{
			fillChip(*o);
		}
		for(vector<string>::size_type c= 0; c < nAdb; ++c)
		{
			adapter= properties->getValue("maximadapter", c, false);
			condIt= m_mConductors.find(adapter);
			if(condIt != m_mConductors.end())
				m_mConductors[adapter]->used= true;
		}
		return true;
	}

	bool MaximChipAccess::fillChip(const string chipID)
	{
		vector<string> path;
		SHAREDPTR::shared_ptr<chip_type_t> chip;
		SHAREDPTR::shared_ptr<chip_pin_t> pin;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::iterator condIt;

		condIt= m_mConductors.find(chipID);
		if(condIt !=m_mConductors.end())
			return false;// id exist

		chip= SHAREDPTR::shared_ptr<chip_type_t>(new chip_type_t);
		chip->folder= -1;
		chip->type= getChipType(chipID);
		chip->family= getChipFamily(chipID);
		chip->used= false;
		if(chip->family == "29")
		{
			string path("/");

			path= URL::addPath(path, chipID);
			path= URL::addPath(path, "strobe");
			if(write(path, 1, "") == -1)
			{
				char cerrno[20];
				string msg;

				msg= "WARNING: cannot write on strobe for chip ";
				msg+= chipID + "\n";
				sprintf(cerrno, "%d", errno);
				msg+= "         ERRNO(";
				msg+= cerrno;
				msg+= "): ";
				msg+= strerror(errno);
				cerr << msg << endl;
				LOG(LOG_ERROR, msg);
			}
			for(short nPin= 0; nPin < 8; ++nPin)
			{
				ostringstream cPin;
				ostringstream pioPin;

				// create 8 pins for reading
				cPin << nPin;
				pin= SHAREDPTR::shared_ptr<chip_pin_t>(new chip_pin_t);
				pin->id= "";
				pin->chipid= chipID;
				pin->pin= -1; // number only be set if pin should reading or writing in an cache
				pin->value= -9999;
				pin->steps= 1;
				pin->used= false;
				//pin->chip= &chip;
				pin->cache= false;
				chip->pins[cPin.str()]= pin;

				// create 8 pins for writing
				pioPin << "PIO." << nPin;
				pin= SHAREDPTR::shared_ptr<chip_pin_t>(new chip_pin_t);
				pin->id= "";
				pin->chipid= chipID;
				pin->pin= -1; // number only be set if pin should reading or writing in an cache
				pin->value= -9999;
				pin->steps= 1;
				pin->used= false;
				//pin->chip= chip;
				pin->cache= false;
				chip->pins[pioPin.str()]= pin;
			}
		}
		m_mConductors[chipID]= chip;
		return true;
	}

	bool MaximChipAccess::reachAllChips()
	{
		return true;
	}

	short MaximChipAccess::useChip(const IActionPropertyMsgPattern* prop, string& unique, unsigned short& kernelmode)
	{
		bool read, write, cache= false;
		bool currentRead= false;
		bool cacheWrite= false;
		bool writecacheWrite= false;
		bool bnew= false;
		short nPin;
		short steps= 1;// default step
		double cachetime;
		string folder, chipID, ID, path, pin;
		string sprop;

		kernelmode= 0;
		DbInterface::chips_t defaultChip;
		DbInterface *defaultChipReader= DbInterface::instance();
		SHAREDPTR::shared_ptr<chip_type_t> ptchip;
		SHAREDPTR::shared_ptr<chip_pin_t> ptpin;
		vector<string> vPSplit;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::iterator chipIt;
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::iterator pinIt;

		folder= prop->needValue("_folderID");
		ID= prop->needValue("ID");
		path= ID;
		pin= prop->getValue("pin", /*warning*/false);
		read= prop->haveAction("read");
		write= prop->haveAction("write");
		cacheWrite= prop->haveAction("cache");
		writecacheWrite= prop->haveAction("writecache");
		currentRead= prop->haveAction("current");
		sprop= "cache";
		cachetime= prop->getDouble(sprop, /*warning*/false);
		if(cachetime == 0)
			cachetime= 9999;
		if(	folder == ""
			||
			path == ""	)
		{
			return 0;
		}
		vPSplit= ConfigPropertyCasher::split(path, "/");
		for(vector<string>::iterator c= vPSplit.begin(); c != vPSplit.end(); ++c)
		{
			if((*c).substr(2, 1) == ".")
			{
				chipID= *c;
				break;
			}
		}
		chipIt= m_mConductors.find(chipID);
		if(chipIt == m_mConductors.end())
		{
			string msg(prop->getMsgHead(true));

			msg+= "do not found dallas chip id ";
			msg+= chipID;
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return 0;
		}
		ptchip= chipIt->second;
		defaultChip= defaultChipReader->getRegisteredDefaultChip(getServerName(), ptchip->family, ptchip->type, ID);
		//defaultChip= defaultChipReader->getDefaultChip(getServerName(), ID);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// seting typical chip behavior
		if(pin == "")
		{
			nPin= -1;
			for(vector<string>::iterator p= ++vPSplit.begin(); p != vPSplit.end(); ++p)
				pin= URL::addPath(pin, *p);
		}
		if(defaultChip.exists)
		{
			if(	pin == ""
				&&
				defaultChip.pin != "###all"	)
			{
				pin= defaultChip.pin;
			}
			if(	write
				&&
				!defaultChip.bWritable	)
			{
				string msg(prop->getMsgHead(false));

				msg+= "write is set but chip in ";
				msg+= getDefaultFileName() + " is only set for readable";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
				read= true;
				write= false;
			}
			if(!defaultChip.bWritable)
				read= true;
		}
		if(pin != "")
		{
			if(isdigit(*pin.c_str()))
			{
				nPin= atoi(pin.c_str());
				if(ptchip->family == "29")
				{
					if(write)
					{
						if(	!cacheWrite
							&&
							!writecacheWrite	)
						{
							pin= "PIO." + pin;
							path= URL::addPath(path, pin);
							nPin= -1;
						}
					}
					// else by reading do not add number to the path
					// because pin of path can be chance in more varios pins
				}
			}else
			{
				path= URL::addPath(path, pin);
				nPin= -1;
			}
		}else
		{
			string msg(prop->getMsgHead(true));

			msg+= "no pin is set for chip";
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return 0;
		}

		if(ptchip->family == "29")
		{
			if(	read
				&&
				(	cacheWrite
					||
					writecacheWrite	)	)
			{
				string msg(prop->getMsgHead(false));

				msg+= "action cache or writecache only for writing and will be ignored";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
				cacheWrite= false;

			}else if(	write == true
						&&
						currentRead	== true	)
			{
				string msg(prop->getMsgHead(false));

				msg+= "action current only for reading and will be ignored";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
			}
			if(!read && !write)
			{
				if(	pin.length() >= 3
					&&
					pin.substr(0, 3) == "PIO"	)
				{
					write= true;
				}else if(	(	pin.length() >= 5
								&&
								pin.substr(0, 5) == "latch"	)
							||
							(	pin.length() >= 6
								&&
								pin.substr(0, 6) == "sensed"	)	)
				{
					read= true;
				}
			}
			if(write)
			{
				if(isdigit(pin[0]))
				{
					pin= "PIO." + pin;
				}
				cache= cacheWrite;
			}else
			{
				cache= !currentRead;
				if(currentRead)
				{

				}else
					steps= 3;
				// else by currentReading
				// only read sensed pin, is one step (default)
			}

		}else if(	currentRead
					||
					cacheWrite	)
		{
			string msg(prop->getMsgHead(false));

			msg+= "action ";
			if(currentRead)
			{
				msg+= "current ";
				if(cacheWrite)
					msg+= "and ";
			}
			if(cacheWrite)
				msg+= "cache ";
			msg+= " be only for DS2408 (family:29) and will be ignored";
			LOG(LOG_WARNING, msg);
			cout << msg << endl;
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		pinIt= ptchip->pins.find(pin);
		if(pinIt == chipIt->second->pins.end())
		{
			bnew= true;
			ptpin= SHAREDPTR::shared_ptr<chip_pin_t>(new chip_pin_t);
		}else
			ptpin= pinIt->second;
		unique= URL::addPath(folder + "." + path, pin);
		ptpin->id= unique;
		ptpin->chipid= chipID;
		ptpin->pin= nPin;
		ptpin->path= URL::addPath("/", path);
		ptpin->value= 0;
		ptpin->steps= steps;
		ptpin->used= true;
		ptpin->cache= cache;
		ptpin->msg= prop->getMsgHead();// without error identif
		ptpin->errmsg= prop->getMsgHead(/*error message*/true);
		ptpin->warnmsg= prop->getMsgHead(/*warning message*/false);
		ptchip->used= true;

		if(read)
		{// fill in cache time for set steps return to begin
		 // whether reading of cache was ending
			m_mvReadingCache[cachetime].push_back(ptpin);
		}
		m_mUsedChips[unique]= ptpin;
		if(bnew)// create new pin in chip
			ptchip->pins[pin]= ptpin;
		foundChip(getChipID(unique));
		defaultChipReader->registerChip(getServerName(), unique, pin, ptchip->type, ptchip->family);
		if(read)
			return 1;
		return 2;
	}

	bool MaximChipAccess::existID(const string type, const string id) const
	{
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::const_iterator iter;
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::const_iterator used;

		if(type != "OWFS")
			return false;
		iter= m_mConductors.find(id);
		if(iter != m_mConductors.end())
			return true;
		used= m_mUsedChips.find(id);
		if(used != m_mUsedChips.end())
			return true;
		incorrectChip(id);
		return false;
	}

	void MaximChipAccess::incorrectChip(const string chipID)
	{
		LOCK(m_INCORRECTCHIPIDS);
		cout << " incorrect chip " << chipID << "-----------------------------------------------------------------------" << endl;
		m_vsIncorrect.insert(chipID);
		UNLOCK(m_INCORRECTCHIPIDS);
	}

	void MaximChipAccess::foundChip(const string chipID)
	{
		set<string>::iterator it;

		LOCK(m_INCORRECTCHIPIDS);
		it= find(m_vsIncorrect.begin(), m_vsIncorrect.end(), chipID);
		if(it != m_vsIncorrect.end())
		{
			cout << " finding chip " << chipID << "-----------------------------------------------------------------------" << endl;
			m_vsIncorrect.erase(it);
		}
		UNLOCK(m_INCORRECTCHIPIDS);
	}

	bool MaximChipAccess::hasIncorrectChips()
	{
		bool bRv= true;

		LOCK(m_INCORRECTCHIPIDS);
		if(m_vsIncorrect.empty())
			bRv= false;
		UNLOCK(m_INCORRECTCHIPIDS);
		return bRv;
	}

	vector<string> MaximChipAccess::getUnusedIDs() const
	{
		typedef map<string, SHAREDPTR::shared_ptr<chip_type_t> >::const_iterator iter;

		vector<string> unused;

		for(iter i= m_mConductors.begin(); i != m_mConductors.end(); ++i)
		{
			if(!i->second->used)
				unused.push_back(i->first);
		}
		return unused;
	}

	bool MaximChipAccess::connect()
	{
		static bool failt= false;
		ssize_t result;

		//cout << endl << "try to connect with string " << m_sInit << endl << endl;
		result= OW_init(m_sInit.c_str());
		if(result != 0)
		{
			ostringstream log;
			ostringstream msg;


			log << "maximchipaccess_" << m_sInit << errno;
			if(errno == 0)
			{
				// successful trying to connect,
				// but have not found any port with init
				TIMELOG(LOG_ERROR, log.str(), "ERROR: no correct OW_init, but no errno be set");
				return true;
			}
			msg << "### ERROR: can not initial owfs one wire device with string '";
			msg << m_sInit << "'\n    ERRNO(";
			msg << dec << errno << "): ";
			msg << strerror(errno);
			if(!failt)
				cerr << msg.str() << endl;
			failt= true;
			TIMELOG(LOG_ERROR, log.str(), msg.str());
			return false;
		}
		m_bConnected= true;
		m_vsIds.clear();
		if(!readChipIDs(&m_vsIds))
		{
			if(errno == 5)
			{
				string msg("### ERROR: input ouptput error -> cannot read directory from maximchips correctly");

				LOG(LOG_ERROR, msg);
				m_bConnected= false;
#ifdef DEBUG
				cerr << msg << endl;
#endif
			}
			return false;
		}
		if(failt)
		{
			string msg("first initialization of owfs one wire device with string '");

			msg+= m_sInit + "'";
			LOG(LOG_INFO, msg);
			cout << msg << endl;
		}
		return true;
	}

	void MaximChipAccess::disconnect()
	{
		OW_finish();
	}

	string MaximChipAccess::getChipID(const string ID)
	{
		vector<string> ids;
		vector<string>::const_iterator idsIter;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::const_iterator iter;
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::const_iterator used;

		iter= m_mConductors.find(ID);
		if(iter != m_mConductors.end())
			return iter->first;
		used= m_mUsedChips.find(ID);
		if(used != m_mUsedChips.end())
			return used->second->chipid;
		ids= getChipIDs();
		idsIter= find(ids.begin(), ids.end(), ID);
		if(idsIter != ids.end())
			return ID;
		return "";
	}

	string MaximChipAccess::getChipTypeID(const string ID)
	{
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::iterator pinIt;

		pinIt= m_mUsedChips.find(ID);
		if(pinIt == m_mUsedChips.end())
			return "";
		return pinIt->second->chipid;
	}

	string MaximChipAccess::getChipType(const string ID)
	{
		int res;
		char* buf;
		string path("/");
		string type;
		size_t size;

		path= URL::addPath(path, ID);
		path= URL::addPath(path, "/type");
		res= OW_get(path.c_str(), &buf, &size);
		if(res < 0)
		{
			char cerrno[20];
			string msg("### WARNING: searched ID:'");

			sprintf(cerrno, "%d", errno);
			msg+= ID + " is no chip type\n";
			msg+= "             ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			msg+= strerror(errno);
			TIMELOG(LOG_DEBUG, "chipType"+ID, msg);
			if(size > 0)
				free(buf);
			return "";
		}
		type= buf;
		free(buf);
		return type;
	}

	string MaximChipAccess::getChipFamily(const string ID)
	{
		int res;
		char* buf;
		string path("/");
		size_t size;
		//unsigned short rlen= 6;
		//char result[rlen];
		string rID;
		string sRv;
		map<string, string>::iterator cIt;

		rID= getChipID(ID);
		cIt= m_mFamilyCodes.find(rID);
		if(cIt != m_mFamilyCodes.end())
			return cIt->second;
		path= URL::addPath(path, rID);
		path= URL::addPath(path, "/family");
		errno= 0;
		res= OW_get(path.c_str(), &buf, &size);
		if(res < 1)
		{
			char cerrno[20];
			string msg("### ERROR: by reading file ");

			sprintf(cerrno, "%d", errno);
			msg+= path + " from one wire file system\n";
			msg+= "    ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			if(errno == 0)
				msg+= "undefined error (cannnot read correctly)";
			else
				msg+= strerror(errno);
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			if(size > 0)
				free(buf);
			return "";
		}
		sRv= buf;
		free(buf);
		if(	!isdigit(sRv.c_str()[0])
		   	||
		   	!isdigit(sRv.c_str()[1])	)
		{
			sRv= rID.substr(0, 2);
			cout << "change family code to beginn of ID " << rID << " is " << sRv << endl;
		}
		m_mFamilyCodes[rID]= sRv;
		return sRv;
	}

	void MaximChipAccess::setDebug(const bool debug)
	{
		LOCK(m_DEBUGINFO);
		m_bDebug= debug;
		UNLOCK(m_DEBUGINFO);
	}

	bool MaximChipAccess::isDebug()
	{
		bool debug;

		LOCK(m_DEBUGINFO);
		debug= m_bDebug;
		UNLOCK(m_DEBUGINFO);
		return debug;
	}


	vector<string> MaximChipAccess::getChipIDs() const
	{
		if(m_vsIds.size() == 0)
		{
			readChipIDs(&m_vsIds);
		}
		return m_vsIds;
	}

	bool MaximChipAccess::readChipIDs(vector<string>* vsIds) const
	{
		int res;
		char* buf;
		size_t s;
		string msg;
		string dirStr;
		vector<string> dirs;
		vector<string>::size_type size;
		vector<string>::iterator oldIt;

#ifdef __OWSERVERREADWRITE
		ostringstream scout2;
		scout2 << "---------------------------------------------------------------------------------------------------------" << endl;
		cout << scout2.str();
#endif // __OWSERVERREADWRITE
		res= OW_get("/", &buf, &s);
		if(res < 0)
		{
			char cerrno[20];

			sprintf(cerrno, "%d", errno);
			msg= "### ERROR: fault reading directorys from one wire file system\n";
			msg+= "    ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			msg+= strerror(errno);
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			if(s > 0)
				free(buf);
			return false;
		}
		dirStr= buf;
		free(buf);
		msg= "read root directory from Maxim-chips access:\n";
		msg+= dirStr;
		msg+= "\nfound follow chips:\n";
		dirs= ConfigPropertyCasher::split(dirStr, ",");
		dirStr= "logdirectory:";
		size= dirs.size();
		for(vector<string>::size_type c= 0; c < size; ++c)
		{
			string dir(dirs[c]);
			unsigned int len= dir.length();

			if(len > 3)
			{
				dirStr+= dir.substr(0, 1);
				if(dir.substr(2, 1) == ".")
				{
					string ID;

					ID= dir.substr(0, dir.length()-1);
					oldIt= find(m_vsBefore.begin(), m_vsBefore.end(), ID);
					if(oldIt == m_vsBefore.end())
					{
						vsIds->push_back(ID);
						msg+= "      ";
						msg+= ID;
						msg+= "\n";
					}
				}
			}
		}
#ifdef __OWSERVERREADWRITE
		ostringstream scout;

		scout << msg << endl;
		scout << "---------------------------------------------------------------------------------------------------------" << endl;
		cout << scout.str();
#endif // __OWSERVERREADWRITE
		// set second parameter for TIMELOG to founded directory length
		// because if reading from directorys changed it should write into log
		TIMELOG(LOG_INFO, dirStr, msg);
#ifdef DEBUG
		cout << msg << endl;
#endif
		return true;
	}

	int MaximChipAccess::command_exec(const string& command, vector<string>& result, bool& more)
	{
		int res;
		char* buf;
		size_t s;
		string msg;
		string read;
		string sresult;
		string chippath;
		string action;
		istringstream ocommand(command);

		result.clear();
		ocommand >> action;
		if(action == "--write")
		{// test writing on pin
			ocommand >> chippath;
			if(OW_put(chippath.c_str(), "0", 1) < 0)
				return -1;
			return 0;

		}else if(action == "--read")
		{
			ocommand >> chippath;
			if(ocommand.fail())
			{
				msg= "cannot read from given command '" + command + "'";
				msg+= " inside command_exec()";
				LOG(LOG_ERROR, msg);
				return -1;
			}
		}else
		{
			chippath= command;
			action= "";// reading directory
		}

		read= "/" + chippath;
		res= OW_get(read.c_str(), &buf, &s);
		if(res < 0)
		{
			char cerrno[20];

			sprintf(cerrno, "%d", errno);
			msg= "### ERROR: fault reading directorys from one wire file system";
			if(command != "")
				msg+= "for chip ID " + read;
			msg+="\n";
			msg+= "    ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			msg+= strerror(errno);
			if(action == "--read")
				LOG(LOG_INFO, msg);
			else
				LOG(LOG_ERROR, msg);
			if(s > 0)
				free(buf);
			return -1;
		}
		sresult= buf;
		free(buf);
		if(action == "")
			result= ConfigPropertyCasher::split(sresult, ",");
		else
		{// action == "--read"
			if(sresult != "")
				result.push_back(sresult);
		}
		return 0;
	}

	short MaximChipAccess::write(const string id, const double value, const string& addinfo)
	{
		short nRv= 0;// writing was correctly and the pin is finished (go to the next)
		ssize_t size;
		string inval;
		string path("/");
		SHAREDPTR::shared_ptr<chip_type_t> chip;
		SHAREDPTR::shared_ptr<chip_pin_t> pin;
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::iterator pinIt;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::iterator chipIt;

		pinIt= m_mUsedChips.find(id);
		if(pinIt != m_mUsedChips.end())
		{
			pin= pinIt->second;
			path= pin->path;
			pin->value= value;
			if(pin->pin > -1)
			{
				chipIt= m_mConductors.find(pin->chipid);
				chip= chipIt->second;
				if(chip->family == "29")
				{// DS2408
					char buf[7];
					string sbuf;

					if(pin->cache)
						return 2;// an entry was made but writing prime in an next time on an other pin
					for(short n= 0; n < 8; ++n)
					{
						snprintf(buf, 6, "PIO.%d", n);
						sbuf= buf;
						pinIt= chip->pins.find(sbuf);
						if(pinIt != chip->pins.end())
						{
							string v;

							if(pinIt->second->value == 0)
								v= "no";
							else
								v= "yes";
							inval+= v + ", ";
						}else
							inval+= "no, ";
					}
					path= URL::addPath(path, "PIO.ALL");
					inval= inval.substr(0, inval.length()-2);
					if(inval == chip->holeValue)
						return 3;// nothing to do
					chip->holeValue= inval;
					//nRv must not set, because the value is the same as set on begin from function
					//nRv= 0 - writing was correctly and the pin is finished (go to the next)
				}
			}else
			{
				if(value)
					inval= "yes";
				else
					inval= "no";
			}
		}else
		{
			path= id;
			if(value)
				inval= "yes";
			else
				inval= "no";
		}
#ifdef SERVERTIMELOG
			timeval stltv;
			string msg;
			char stlbuf[20];
			tm l;

			if(gettimeofday(&stltv, NULL))
			{
				string msg("WARNING: cannot get time of day,\n");

				msg+= "         so do not read sequence inside defined cache.\n";
				msg+= "         In this case measureing only in sequence back to back";
				TIMELOG(LOG_WARNING, "gettimeofday", msg);
			}else
			{
				msg= "write on " + path;
				msg+= " value ";
				msg+= inval + "\n";
				if(localtime_r(&stltv.tv_sec, &l) == NULL)
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				strftime(stlbuf, 15, "%H:%M:%S", &l);
				msg+= stlbuf;
				msg+= " mikrosec:";
				snprintf(stlbuf, 15, "%ld", stltv.tv_usec);
				msg+= stlbuf;
				LOG(LOG_INFO, msg);
			}
#endif // SERVERTIMELOG
		//size= OW_lwrite(path.c_str(), inval.c_str(), inval.length(), 0 );
		size= OW_put(path.c_str(), inval.c_str(), inval.length());
#ifdef SERVERTIMELOG

			if(gettimeofday(&stltv, NULL))
			{
				string msg("WARNING: cannot get time of day,\n");

				msg+= "         so do not read sequence inside defined cache.\n";
				msg+= "         In this case measureing only in sequence back to back";
				TIMELOG(LOG_WARNING, "gettimeofday", msg);
			}else
			{
				msg= "writing finished on " + path;
				msg+= "\n";
				if(localtime_r(&stltv.tv_sec, &l) == NULL)
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				strftime(stlbuf, 15, "%H:%M:%S", &l);
				msg+= stlbuf;
				msg+= " mikrosec:";
				snprintf(stlbuf, 15, "%ld", stltv.tv_usec);
				msg+= stlbuf;
				LOG(LOG_INFO, msg);
			}
#endif // SERVERTIMELOG
		if(size <= 0)
		{
			char cerrno[20];
			string msg("by writing on path ");

			msg+= path + " some error occured.\nmake new connection";
			LOG(LOG_DEBUG, msg);
			if(	!connect()
				||
				OW_put(path.c_str(), inval.c_str(), inval.length())	<= 0	)
				//OW_lwrite(path.c_str(), inval.c_str(), inval.length(), 0 ) <= 0	)
			{
				if(pinIt != m_mUsedChips.end())
					msg= pinIt->second->errmsg;
				msg= "cannot write to chip with path '";
				msg+= path + "'\n";
				sprintf(cerrno, "%d", errno);
				msg+= "    ERRNO(";
				msg+= cerrno;
				msg+= "): ";
				msg+= strerror(errno);
				cerr << msg << endl;
				LOG(LOG_ERROR, msg);
#ifdef DEBUG
				cerr << msg << endl;
#endif //DEBUG
				return -1;
			}
		}
		return nRv;
	}

	short MaximChipAccess::read(const string id, double& value)
	{
		/**
		 * reading was correctly and the pin is finished (go to the next)
		 */
		short nRv= 0;
		int res;
		size_t size;
		char* buf;
		string path("/");
		string sResult;
		SHAREDPTR::shared_ptr<chip_type_t> chip;
		SHAREDPTR::shared_ptr<chip_pin_t> pin;
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> >::iterator pinIt;
		map<string, SHAREDPTR::shared_ptr<chip_type_t> >::iterator chipIt;

		pinIt= m_mUsedChips.find(id);
		if(pinIt != m_mUsedChips.end())
		{
			pin= pinIt->second;
			if(pin->steps == 0)
			{// nothing to do, value param set, was reading befor (chip wasn't read) -> go to the next pin)
				value= pin->value;
				return 3;
			}
			path= pin->path;
			//cout << "read pin " << pin->pin << endl;
			chipIt= m_mConductors.find(pin->chipid);
			if(chipIt == m_mConductors.end())
			{
				string msg("ERROR: cannot find chip id ");

				msg+= pin->chipid + " for uniqe pin ";
				msg+= pin->id + " in m_nConductors witch is in UsedChips";
				LOG(LOG_ALERT, msg);
#ifdef debug
				cerr << msg << endl;
#endif // debug
			}else
			{
				chip= chipIt->second;
				if(pin->pin > -1)
				{
					if(chip->family == "29")
					{// DS2408
						if(pin->steps == 1)
						{
							if(pin->cache)
							{// write all latch to 0 bit
								string inval("no, no, no, no, no, no, no, no");

								value= pin->value;
								path= URL::addPath(path, "latch.ALL");
								//OW_lread(path.c_str(), result, 500, 0);
								//nRv= OW_lwrite(path.c_str(), inval.c_str(), inval.length(), 0 );
								nRv= OW_put(path.c_str(), inval.c_str(), inval.length());
								if(nRv <= 0)
								{
									ostringstream msg;

									msg << "### ERROR: cannot write into " << path << "\n";
									msg << "    ERRNO(" << errno << "); " << strerror(errno);
									TIMELOG(LOG_ERROR, path, msg.str());
								}else
								{
									char buf[4];
									string sbuf;

									for(short c= 0; c < 8; ++c)
									{
										snprintf(buf, 3, "%d", c);
										sbuf= buf;
										pin= chip->pins[sbuf];
										if(	pin->steps == 1
											&&
											pin->cache		)
										{
											pin->steps= 0;
										}
									}
								}
								// read procedure is correctly and the pin is finished (go to the next)
								return 0;
							}else
							{
								path= URL::addPath(path, "sensed.ALL");
								// nRv is set on beginning to default 0
								// nRv= 0; reading was correctly and the pin is finished (go to the next)
							}

						}else if(pin->steps == 2)
						{
							path= URL::addPath(path, "latch.ALL");
							nRv= 1;// reading was correctly but the next time should make the same pin

						}else
						{// pin->step is 3
							path= URL::addPath(path, "sensed.ALL");
							nRv= 1;// reading was correctly but the next time should make the same pin
						}

					}
				}
			}
		}else
			path= id;

		buf= NULL;
		sResult= "";
		res= OW_get(path.c_str(), &buf, &size);
		//cout << "maxim " << path << " have value " << buf << flush;
		if(buf != NULL)
		{
			if(res > 1)
				sResult= buf;
			free(buf);
		}
		if(res < 1)
		{
			//int oldErrno= errno;
			char cerrno[20];
			string msg;

			msg= "### ERROR: by reading path ";
			sprintf(cerrno, "%d", errno);
			msg+= path + " from one wire file system\n";
			msg+= "    ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			if(errno == 0)
				msg+= "undefined value (cannot read correctly)";
			else
				msg+= strerror(errno);
			//cerr << msg << endl;
			incorrectChip(getChipID(id));
			TIMELOG(LOG_ERROR, "maximread"+path, msg);
#ifdef DEBUG
				cerr << msg << endl;
#endif //DEBUG
			value= 0;
			return -1;
		}
		if(chip)
		{
			if(chip->family == "10")
			{
				value= atof(sResult.c_str());
				if(value > 85)
				{// unknown ERROR
				 // temperature is higher then 100 °C
				 // found problem by flat count (21.00 or 22,00 ...)
					string msg;
					char buff[51];

					msg= pin->errmsg;
					msg+= "by converting ";
					msg+= sResult;
					msg+= " to ";
					snprintf(buff, 50, "%lf", value);
					msg+= buff;
					msg+= ", maybe VCC is not correct 5 V";
					TIMELOG(LOG_ERROR, "convertingOver100", msg);
					value/= 10;
					//snprintf(result, 50, "%lf", v);
					return -1;
				}

			}else if(chip->family == "29")
			{// DS2408
				bool bReadLatch= false;
				short nPin= 0;
				char buf[4];
				//string sResult;
				string sbuf;
				vector<string> splited;
				SHAREDPTR::shared_ptr<chip_pin_t> chippin;

				//sResult= buf;
				sResult= ConfigPropertyCasher::trim(sResult);
				splited= ConfigPropertyCasher::split(sResult, ", ");
				for(vector<string>::iterator i= splited.begin(); i != splited.end(); ++i)
				{
					snprintf(buf, 3, "%d", nPin);
					sbuf= buf;
					chippin= chip->pins[sbuf];
					switch(chippin->steps)
					{
					case 3:
						//if(*i == "yes")

						if(*i == "1")// if read actual is set ('1'),
						{
							chippin->value= 1;
							chippin->steps= 0;// do not read or write latch
						}else
						{
							chippin->value= 0;
							--chippin->steps;
						}
						break;

					case 2:
						if(*i == "1")
						{// step comming alwas from 3
						 // and is not set ('0')
							//bReadLatch= true;
							chippin->value= 1;
							--chippin->steps;
						}else
							//chippin->value= 0;
							chippin->steps= 0;
						break;

					case 1:
						if(*i == "1")
							chippin->value= 1;
						else
							chippin->value= 0;
						--chippin->steps;
						break;
					}
					++nPin;
				}
				if(bReadLatch)
				{
					string inval("no, no, no, no, no, no, no, no");

					//value= pin->value;
					//path= URL::addPath(path, "latch.ALL");
					//OW_lread(path.c_str(), result, 500, 0);
					//nRv= OW_lwrite(path.c_str(), inval.c_str(), inval.length(), 0 );
					nRv= OW_put(path.c_str(), inval.c_str(), inval.length());
				}
				value= pin->value;
			}else
				value= atof(sResult.c_str());
		}else
			value= atof(sResult.c_str());
		return nRv;
	}

	void MaximChipAccess::range(const string pin, double& min, double& max, bool &bfloat)
	{
		string server(getServerName());
		DbInterface *reader= DbInterface::instance();
		DbInterface::chips_t chip;

		chip= reader->getRegisteredDefaultChip(server, pin);
		if(chip.exists)
		{
			min= chip.dmin;
			max= chip.dmax;
			bfloat= chip.bFloat;
		}else
		{
			string msg("### DEV INFO: undefined Maxim chip ");

			msg+= pin;
			msg+= "\n              by define range in MaximChipAccess class";
			msg+= "\n              so allow hole range for chip";
			TIMELOG(LOG_INFO, "maximrangedef" + pin, msg);
			min= 1;
			max= 0;
			bfloat= true;
		}
	}

	void MaximChipAccess::endOfCacheReading(const double cachetime)
	{
		map<double, vector<SHAREDPTR::shared_ptr<chip_pin_t> > >::iterator chit;

		chit= m_mvReadingCache.find(cachetime);
		if(chit != m_mvReadingCache.end())
		{
			for(vector<SHAREDPTR::shared_ptr<chip_pin_t> >::iterator c= m_mvReadingCache[cachetime].begin(); c != m_mvReadingCache[cachetime].end(); ++c)
			{
				if((*c)->cache)
					(*c)->steps= 3;
				else
					(*c)->steps= 1;
			}
		}
	}

	void MaximChipAccess::endOfLoop()
	{
		if(hasIncorrectChips())
		{
			vector<string>::iterator it;

			m_vsIds.clear();
			readChipIDs(&m_vsIds);
			for(it= m_vsIds.begin(); it != m_vsIds.end(); ++it)
			{
				foundChip(*it);
				fillChip(*it);
			}
		}
	}

	MaximChipAccess::~MaximChipAccess()
	{
		// toDo: erase created chips
		//for(map<string, chip_type_t*>::iterator it= m_mConductors.begin(); it != m_mConductors.end(); ++it)
		//	delete it->second;
	}

}
#endif //_OWFSLIBRARY
