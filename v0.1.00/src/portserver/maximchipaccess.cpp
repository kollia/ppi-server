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

#include "maximchipaccess.h"
#ifdef _OWFSLIBRARY

#include <errno.h>
#include <owcapi.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>


#include "../util/URL.h"
#include "../util/configpropertycasher.h"

#include "../database/DefaultChipConfigReader.h"

#include "../logger/LogThread.h"

using namespace util;
using namespace std;

namespace ports
{

	short MaximChipAccess::m_nInterface= -1;
	vector<string> MaximChipAccess::m_vsIncorrect;
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

	bool MaximChipAccess::init(const IPropertyPattern* properties)
	{
		string adapter, msg;
		vector<string> ids;
		vector<string>::size_type nAdb= 0;
		map<string, chip_type_t*>::iterator condIt;

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
			if(!connect())// when connection was failt
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
		chip_type_t* chip;
		chip_pin_t* pin;
		map<string, chip_type_t*>::iterator condIt;

		condIt= m_mConductors.find(chipID);
		if(condIt !=m_mConductors.end())
			return false;// id exist

		chip= new chip_type_t;
		chip->folder= -1;
		chip->type= getChipType(chipID);
		chip->family= getChipFamily(chipID);
		chip->used= false;
		if(chip->family == "29")
		{
			string path("/");

			path= URL::addPath(path, chipID);
			path= URL::addPath(path, "strobe");
			if(write(path, 1) == -1)
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
				char cPin[10];
				string sPin;

				// create 8 pins for reading
				snprintf(cPin, 3, "%d", nPin);
				sPin= cPin;
				pin= new chip_pin_t;
				pin->id= "";
				pin->chipid= chipID;
				pin->pin= -1; // number only be set if pin should reading or writing in an cache
				pin->value= -9999;
				pin->steps= 1;
				pin->used= false;
				//pin->chip= &chip;
				pin->cache= false;
				chip->pins[sPin]= pin;

				// create 8 pins for writing
				snprintf(cPin, 9, "PIO.%d", nPin);
				sPin= cPin;
				pin= new chip_pin_t;
				pin->id= "";
				pin->chipid= chipID;
				pin->pin= -1; // number only be set if pin should reading or writing in an cache
				pin->value= -9999;
				pin->steps= 1;
				pin->used= false;
				//pin->chip= chip;
				pin->cache= false;
				chip->pins[sPin]= pin;
			}
		}
		m_mConductors[chipID]= chip;
		return true;
	}

	bool MaximChipAccess::reachAllChips()
	{
		return true;
	}

	//useChip(short folderID, string id, string pin, bool read, unsigned int priority, bool uncached, bool cache, double readCache)
	short MaximChipAccess::useChip(const IActionPropertyMsgPattern* prop, string& unique)
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

		const DefaultChipConfigReader::chips_t* defaultChip;
		DefaultChipConfigReader *defaultChipReader= DefaultChipConfigReader::instance();
		chip_type_t* ptchip;
		chip_pin_t* ptpin;
		vector<string> vPSplit;
		map<string, chip_type_t*>::iterator chipIt;
		map<string, chip_pin_t*>::iterator pinIt;

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
		if(defaultChip)
		{
			if(	pin == ""
				&&
				defaultChip->pin != "###all"	)
			{
				pin= defaultChip->pin;
			}
			if(	write
				&&
				!defaultChip->bWritable	)
			{
				string msg(prop->getMsgHead(false));

				msg+= "write is set but chip in ";
				msg+= getDefaultFileName() + " is only set for readable";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
				read= true;
				write= false;
			}
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
			ptpin= new chip_pin_t;
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
		map<string, chip_type_t*>::const_iterator iter;
		map<string, chip_pin_t*>::const_iterator used;

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
		m_vsIncorrect.push_back(chipID);
		UNLOCK(m_INCORRECTCHIPIDS);
	}

	void MaximChipAccess::foundChip(const string chipID)
	{
		vector<string>::iterator it;

		LOCK(m_INCORRECTCHIPIDS);
		it= find(m_vsIncorrect.begin(), m_vsIncorrect.end(), chipID);
		if(it != m_vsIncorrect.end())
		{
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
		typedef map<string, chip_type_t*>::const_iterator iter;

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

		//if(m_bConnected)
		//	disconnect();
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
		map<string, chip_type_t*>::const_iterator iter;
		map<string, chip_pin_t*>::const_iterator used;

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
		map<string, chip_pin_t*>::iterator pinIt;

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
			string msg("### ERROR: by reading file ");

			sprintf(cerrno, "%d", errno);
			msg+= path + " from one wire file system\n";
			msg+= "    ERRNO(";
			msg+= cerrno;
			msg+= "): ";
			msg+= strerror(errno);
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
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
		size= dirs.size();
		for(vector<string>::size_type c= 0; c < size; ++c)
		{
			string dir(dirs[c]);
			unsigned int len= dir.length();

			if(	len > 3
				&&
				dir.substr(2, 1) == "."	)
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
		// set second parameter for TIMELOG to founded directory length
		// because if reading from directorys changed it should write into log
		TIMELOG(LOG_INFO, dirStr, msg);
#ifdef DEBUG
		cout << msg << endl;
#endif
		return true;
	}

	short MaximChipAccess::write(const string id, const double value)
	{
		short nRv= 0;// writing was correctly and the pin is finished (go to the next)
		ssize_t size;
		string inval;
		string path("/");
		chip_type_t* chip= NULL;
		chip_pin_t* pin= NULL;
		map<string, chip_pin_t*>::iterator pinIt;
		map<string, chip_type_t*>::iterator chipIt;

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
				strftime(stlbuf, 15, "%H:%M:%S", localtime(&stltv.tv_sec));
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
				strftime(stlbuf, 15, "%H:%M:%S", localtime(&stltv.tv_sec));
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
		chip_type_t* chip= NULL;
		chip_pin_t* pin= NULL;
		map<string, chip_pin_t*>::iterator pinIt;
		map<string, chip_type_t*>::iterator chipIt;

		pinIt= m_mUsedChips.find(id);
		if(pinIt != m_mUsedChips.end())
		{
			pin= pinIt->second;
			if(pin->steps == 0)
			{// nothing to do, value param set, was reading befor (chip wasn't read) -> go to the next pin)
				value= pin->value;
				//cout << "end of reading with state 3" << endl;
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

		res= OW_get(path.c_str(), &buf, &size);
		if(	res > -1
			&&
			buf != NULL	)
		{
			sResult= buf;
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
			if(size > 0)
				free(buf);
			//cout << "end of reading with state -1" << endl;
			return -1;
		}
		if(chip)
		{
			if(chip->family == "10")
			{
				value= atof(buf);
				if(value > 85)
				{// unknown ERROR
				 // temperature is higher then 100 °C
				 // found problem by flat count (21.00 or 22,00 ...)
					string msg;
					char buff[51];

					msg= pin->errmsg;
					msg+= "by converting ";
					msg+= buf;
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
				chip_pin_t* chippin;

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
				value= atof(buf);
		}else
			value= atof(buf);
		free(buf);
		//cout << "end of reading with state " << nRv << endl;
		return nRv;
	}

	void MaximChipAccess::range(const string pin, double& min, double& max, bool &bfloat)
	{
		string server(getServerName());
		DefaultChipConfigReader *reader= DefaultChipConfigReader::instance();
		const DefaultChipConfigReader::chips_t *chip;

		chip= reader->getRegisteredDefaultChip(server, pin);
		if(chip)
		{
			min= chip->dmin;
			max= chip->dmax;
			bfloat= chip->bFloat;
		}else
		{
			string msg("undefined Maxim chip ");

			msg+= pin;
			msg+= "\nby define range in MaximChipAccess class";
			TIMELOG(LOG_ERROR, "maximrangedef" + pin, msg);
			cerr << "### ERROR: " << msg << endl;
			min= 0;
			max= 1;
			bfloat= false;
		}
	}

	void MaximChipAccess::endOfCacheReading(const double cachetime)
	{
		map<double, vector<chip_pin_t*> >::iterator chit;

		chit= m_mvReadingCache.find(cachetime);
		if(chit != m_mvReadingCache.end())
			for(vector<chip_pin_t*>::iterator c= m_mvReadingCache[cachetime].begin(); c != m_mvReadingCache[cachetime].end(); ++c)
			{
				if((*c)->cache)
					(*c)->steps= 3;
				else
					(*c)->steps= 1;
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
