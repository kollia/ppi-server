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

#include "ExternPorts.h"

#include <errno.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../logger/lib/LogInterface.h"

#include "../ports/portbaseclass.h"


using namespace std;
using namespace boost;

namespace ports
{
	/**
	 * all ports as key which are opened
	 * and the current state as value
	 */
	map<unsigned long, int> global_mOpenedPorts;

	ExternPorts::ExternPorts(const vector<string>& openPorts, const PortTypes type)
	{
		m_bDebug= false;
		m_DEBUGINFO= Thread::getMutex("PORTSDEBUGINFO");
		m_vOpenPorts= openPorts;
		m_eType= type;
		m_bSet= false;
		m_bFreeze= false;
		m_maxMeasuredTime= 1500000;
	}

	bool ExternPorts::init(const IPropertyPattern* properties)
	{
		bool bOk= true;
		string property, result;
		unsigned long np;
		vector<string>::size_type nCom;

		property= "COM";
		for(unsigned short c= 1; c <= 2; ++c)
		{// running in two counts for COM and LPT interfaces

			nCom= properties->getPropertyCount(property);
			if(property == "#ERROR")
			{
				string msg;

				msg= "### ERROR: no correct ";
				if(c == 1)
					msg+= "parallel ";
				else
					msg+= "serial ";
				msg+= "extern ports be set, so do not read any subroutines from PORTS with ";
				if(c == 1)
					msg+= "COM";
				else
					msg+= "LPT";
				msg+= "-interface";
				cerr << msg << endl;
				LOG(LOG_ERROR, msg);
			}else
			{
				for(unsigned short n= 1; n <= nCom; ++n)
				{
					ostringstream sport;
					vector<string> values;

					sport << property;
					sport << n;
					result= properties->getValue(property, n - 1, /*warning*/true);
					if(result != "")
					{
						split(values, result, is_any_of(":"));
						if(values.size() != 2)
						{
							string msg;

							msg= "### ERROR: parameter for " + sport.str() + " have wrong definition. ";
							msg+= "Can not read or write on "  + sport.str();
							cerr << msg << endl;
							LOG(LOG_ERROR, msg);
						}else
						{
							istringstream portnr(values[0]);

							portnr >> hex >> np;
							if(np == 0)
							{
								string msg;

								msg= "### ERROR: on parameter from " + sport.str() + " is no right address given. ";
								msg+= "Can not read or write on "  + sport.str();
								cerr << msg << endl;
								LOG(LOG_ERROR, msg);
							}else
							{
								if(c == 1)
									m_mCOMPorts[sport.str()]= pair<unsigned long, string>(np, values[1]);
								else
									m_mLPTPorts[sport.str()]= pair<unsigned long, string>(np, values[1]);
							}
						}
					}
				}
			}
			property= "LPT";
		}
		if(!connect())
			return true;// no error try to connect all seconds
		return true;
	}

	string ExternPorts::getServerName()
	{
		string sRv("extern_computer");

		switch(m_eType)
		{
		case PORT:
			sRv+= "PORT";
			break;
		case MPORT:
			sRv+= "MPORT";
			break;
		case RWPORT:
			sRv+= "RWPORT";
			break;
		}
		return sRv;
	}

	unsigned long ExternPorts::getPortAddress(const string& sPortName) const
	{
		unsigned long address;
		map<string, pair<unsigned long, string> >::const_iterator itFace;

		if(sPortName.substr(0, 3) == "COM")
		{
			itFace= m_mCOMPorts.find(sPortName);
			if(itFace != m_mCOMPorts.end())
				address= itFace->second.first;
			else
				address= 0;

		}else if(sPortName.substr(0, 3) == "LPT")
		{
			itFace= m_mLPTPorts.find(sPortName);
			if(itFace != m_mLPTPorts.end())
				address= itFace->second.first;
			else
				address= 0;
		}else
			address= 0;

		return address;
	}

	ExternPorts::Pin ExternPorts::getPortType(const unsigned long port) const
	{
		for(map<string, pair<unsigned long, string> >::const_iterator it= m_mCOMPorts.begin(); it != m_mCOMPorts.end(); ++it)
		{
			if(it->second.first == port)
				return COM;
		}
		for(map<string, pair<unsigned long, string> >::const_iterator it= m_mLPTPorts.begin(); it != m_mLPTPorts.end(); ++it)
		{
			if(it->second.first == port)
				return LPT;
		}
		return NONE;
	}

	ExternPorts::Pin ExternPorts::getPortType(const string& sPortName) const
	{
		Pin eRv;
		unsigned long address;

		address= getPortAddress(sPortName);
		if(address == 0)
			eRv= NONE;
		else if(sPortName.substr(0, 3) == "COM")
			eRv= COM;
		else if(sPortName.substr(0, 3) == "LPT")
			eRv= LPT;
		else
			eRv= NONE;

		return eRv;
	}

	ExternPorts::Pin ExternPorts::getCOMPinEnum(const string& sPinName) const
	{
		Pin eRv;

		if(sPinName=="DTR")
			eRv= DTR;
		else if(sPinName=="RTS")
			eRv= RTS;
		else if(sPinName=="TXD")
			eRv= TXD;
		else if(sPinName=="CTS")
			eRv= CTS;
		else if(sPinName=="DSR")
			eRv= DSR;
		else if(sPinName=="RI")
			eRv= RI;
		else if(sPinName=="DCD")
			eRv= DCD;
		else
			eRv= NONE;
		return eRv;
	}

	ExternPorts::Pin ExternPorts::getLPTPinEnum(const string& sPinName) const
	{
		Pin eRv;

		if(sPinName == "DATA1")
			eRv= DATA1;
		else if(sPinName == "DATA2")
			eRv= DATA2;
		else if(sPinName == "DATA3")
			eRv= DATA3;
		else if(sPinName == "DATA4")
			eRv= DATA4;
		else if(sPinName == "DATA5")
			eRv= DATA5;
		else if(sPinName == "DATA6")
			eRv= DATA6;
		else if(sPinName == "DATA7")
			eRv= DATA7;
		else if(sPinName == "DATA8")
			eRv= DATA8;
		else if(sPinName == "ERROR")
			eRv= ERROR;
		else if(sPinName == "SELECT")
			eRv= SELECT;
		else if(sPinName == "PAPEREMPTY")
			eRv= PAPEREMPTY;
		else if(sPinName == "ACKNOWLEDGE")
			eRv= ACKNOWLEDGE;
		else if(sPinName == "BUSY")
			eRv= BUSY;
		else if(sPinName == "STROBE")
			eRv= STROBE;
		else if(sPinName == "AUTOFEED")
			eRv= AUTOFEED;
		else if(sPinName == "INIT")
			eRv= INIT;
		else if(sPinName == "SELECTINPUT")
			eRv= SELECTINPUT;
		else
			eRv= NONE;

		return eRv;
	}

	ExternPorts::Pins ExternPorts::getPinsStruct(const string& sPort, const string& sPin) const
	{
		Pin interface;
		Pins tRv;

		tRv.ePin= NONE;
		tRv.nPort= getPortAddress(sPort);
		if(tRv.nPort != 0)
		{
			interface= getPortType(sPort);
			if(interface == COM)
				tRv.ePin= getCOMPinEnum(sPin);
			else if(interface == LPT)
				tRv.ePin= getLPTPinEnum(sPin);
			else
				tRv.ePin= NONE;
			if(tRv.nPort == 0)
				tRv.ePin= NONE;
			else if(tRv.ePin == NONE)
				tRv.nPort= 0;
		}
		return tRv;
	}

	ExternPorts::portpin_address_t ExternPorts::getPortPinAddress(const string& sPort, const string& sPin, const bool bSetAfter) const
	{
		Pins tPin;

		tPin= getPinsStruct(sPort, sPin);
		return getPortPinAddress(tPin, bSetAfter);
	}

	ExternPorts::portpin_address_t ExternPorts::getPortPinAddress(const Pins& tAdr, const bool bSetAfter) const
	{
		portpin_address_t tRv;

		tRv.nPort= tAdr.nPort;
		tRv.ePin= tAdr.ePin;
		tRv.bCacheWriting= false;
		tRv.eOut.nPort= 0x00;
		tRv.eOut.ePin= NONE;
		tRv.eNeg.nPort= 0x00;
		tRv.eNeg.ePin= NONE;
		switch(tAdr.ePin)
		{
			case DTR:
				if(bSetAfter)
					tRv.nPin= 0x01; // 00000001
				else
					tRv.nPin= 0xFE; // 11111110
				tRv.nAdd= 4;
				tRv.eDescript= SETPIN;
				tRv.ePort= COM;
				break;
			case RTS:
				if(bSetAfter)
					tRv.nPin= 0x02; // 00000010
				else
					tRv.nPin= 0xFD; // 11111101
				tRv.nAdd= 4;
				tRv.eDescript= SETPIN;
				tRv.ePort= COM;
				break;
			case TXD:
				if(bSetAfter)
					tRv.nPin= 0x40; // 01000000
				else
					tRv.nPin= 0xBF; // 10111111
				tRv.nAdd= 4;
				tRv.eDescript= SETPIN;
				tRv.ePort= COM;
				break;
			case CTS:
				tRv.nPin= 0x10; // 00010000
				if(bSetAfter)
					tRv.nPin|= 0x01; // 00000001
				tRv.nAdd= 4;
				tRv.eDescript= GETPIN;
				tRv.ePort= COM;
				break;
			case DSR:
				tRv.nPin= 0x20; // 00100000
				if(bSetAfter)
					tRv.nPin|= 0x02; // 00000010
				tRv.nAdd= 6;
				tRv.eDescript= GETPIN;
				tRv.ePort= COM;
				break;
			case RI:
				tRv.nPin= 0x40; // 01000000
				if(bSetAfter)
					tRv.nPin|= 0x04; // 00000100
				tRv.nAdd= 6;
				tRv.eDescript= GETPIN;
				tRv.ePort= COM;
				break;
			case DCD:
				tRv.nPin= 0x80; // 10000000
				if(bSetAfter)
					tRv.nPin|= 0x08; // 00001000
				tRv.nAdd= 6;
				tRv.eDescript= GETPIN;
				tRv.ePort= COM;
				break;
			case DATA1:
			case DATA2:
			case DATA3:
			case DATA4:
			case DATA5:
			case DATA6:
			case DATA7:
			case DATA8:
			case ERROR:
			case SELECT:
			case PAPEREMPTY:
			case ACKNOWLEDGE:
			case BUSY:
			case STROBE:
			case AUTOFEED:
			case INIT:
			case SELECTINPUT:
			default:
				tRv.nPin= 0x00;
				tRv.nPort= 0x0000;
				tRv.nAdd= 0;
				tRv.eDescript= NONE;
				tRv.ePort= NONE;
				break;
		}
		return tRv;
	}

	void ExternPorts::usePropActions(const IActionPropertyPattern* prop) const
	{
		string ID, pin;
		portpin_address_t address;

		ID= prop->getValue("ID", /*warning*/false);
		if(m_eType != RWPORT)
		{// for type PORT or MPORT
			pin= prop->getValue("pin", /*warning*/false);
			address= getPortPinAddress(ID, pin, false);
			if(m_eType == MPORT)
			{// for type MPORT
				prop->getValue("out", /*warning*/false);
				prop->getValue("neg", /*warning*/false);
				prop->haveAction("freeze");
			}else
			{// for type PORT
				if(address.eDescript == GETPIN)
				{
					prop->haveAction("current");
					prop->haveAction("cache");

				}else
					prop->getValue("priority", /*warning*/false);
			}
			if(address.eDescript == GETPIN)
				prop->getValue("cache", /*warning*/false);

			prop->haveAction("db");
			prop->haveAction("perm");
		}
	}

	short ExternPorts::useChip(const IActionPropertyMsgPattern* prop, string& id)
	{
		bool bCurrent;
		bool bCacheWriting;
		string msg, pin;
		portpin_address_t portpin;
		map<string, portpin_address_t>::iterator it;

		Pins pins;

		if(	m_eType == MPORT
			&&
			m_bSet			)
		{// this one wire server can only set for one location
			return 0;
		}
		bCurrent= prop->haveAction("current");
		bCacheWriting= prop->haveAction("cache");
		id= prop->needValue("ID");
		pin= prop->needValue("pin");
		pins= getPinsStruct(id, pin);
		if(	id == ""
			||
			pin == ""
			||
			pins.ePin == NONE	)
		{
			id= "";
			return 0;
		}

		portpin= getPortPinAddress(pins, true);
		if(portpin.eDescript == GETPIN && bCurrent)
			portpin= getPortPinAddress(pins, false);
		if(portpin.eDescript == GETPIN)
			portpin.bCacheWriting= bCacheWriting;
		if(m_eType == MPORT)
		{
			string sPort, sPin;
			vector<string> spl;

			sPin= prop->getValue("out");
			split(spl, sPin, is_any_of(":"));
			if(spl.size() == 2)
			{
				sPort= spl[0];
				sPin= spl[1];
			}else
				sPort= id;
			portpin.eOut= getPinsStruct(sPort, sPin);

			sPin= prop->getValue("neg");
			split(spl, sPin, is_any_of(":"));
			if(spl.size() == 2)
			{
				sPort= spl[0];
				sPin= spl[1];
			}else
				sPort= id;
			portpin.eNeg= getPinsStruct(sPort, sPin);

		}
		id+= ":" + pin;
		if(m_eType == PORT)
		{
			if(portpin.eDescript == GETPIN && bCurrent)
				id+= "_current";

		}else if(m_eType == MPORT)
		{
			// this one wire server for MPORT can only set for one location
			m_bSet= true;
			m_bFreeze= prop->haveAction("freeze");
			id+= "_measure";
		}
		it= m_mUsedPins.find(id);
		if(it == m_mUsedPins.end())
			m_mUsedPins[id]= portpin;

		if(portpin.eDescript == GETPIN)
			return 1;
		return 2;
	}

	vector<string> ExternPorts::getUnusedIDs() const
	{
		vector<string> unused;

		return unused;
	}

	bool ExternPorts::connect()
	{
		static bool failt= false;
		bool err= failt;
		int res;
		unsigned long actPort, actPins;
		map<unsigned long, int>::iterator opened;
		Pin ePort;

		for(vector<string>::iterator it= m_vOpenPorts.begin(); it != m_vOpenPorts.end(); ++it)
		{
			ePort= getPortType(*it);
			if(ePort == NONE)
			{
				if(!failt)
				{
					ostringstream os;

					os << "defined port " << *it << " in measure.conf is no regular port defined in server.conf";
					cerr << os.str() << endl;
					LOG(LOG_ERROR, os.str());
				}
			}else
			{
				errno= 0;
				actPort= getPortAddress(*it);
				opened= global_mOpenedPorts.find(actPort);
				if(actPort != 0)// && opened == global_mOpenedPorts.end())
				{
					ostringstream os;

					os << "### open interface to port " << *it << " " << endl;
					if(!failt)
						cout << os.str() << endl;
					TIMELOG(LOG_INFO, *it+"ioperm", os.str());
					actPort= getPortAddress(*it);
					if(ePort == COM)
						res= ioperm(actPort, 8, 1);
					else
						res= ioperm(actPort, 16, 1); // toDo: this number 16 is maybe not correct for an LPT-interface
					if(res)
					{
						os.clear();
						os << "### ERROR: Could not open extern port " << *it << endl;
						if(errno != 0)
							   os << "    ERRNO(" << dec << errno << "): " << strerror(errno);
						TIMELOG(LOG_ERROR, *it+"ExternPorts", os.str());
						if(!failt)
						{
							cerr << os.str() << endl;
							err= true;
						}

					}else
					{
						actPins= inb(actPort);
						global_mOpenedPorts[actPort]= actPins;
					}
				}
			}
		}
		if(err)
			failt= true;
		else
			m_bConnected= true;
		return true;
	}

	void ExternPorts::disconnect()
	{
		int res;
		Pin ePort;

		for(map<unsigned long, int>::iterator it= global_mOpenedPorts.begin(); it != global_mOpenedPorts.end(); ++it)
		{
			ePort= getPortType(it->first);
			if(ePort != NONE)
			{
				if(ePort == COM)
					res= ioperm(it->first, 8, 0);
				else
					res= ioperm(it->first, 16, 0); // toDo: this number 16 is maybe not correct for an LPT-interface
			}
		}
	}

	string ExternPorts::getChipType(string ID)
	{
		string sRv;

		sRv= "Computer extern ";
		if(ID.substr(0, 3) == "COM")
			sRv+= "COM ";
		else
			sRv+= "LPT ";
		sRv+= "ports";
		return sRv;
	}

	//string ExternPorts::getConstChipTypeID(const string ID) const
	string ExternPorts::getChipTypeID(const string pin)
	{
		string sRv;

		if(pin=="DTR")
			sRv= "COM_O";
		else if(pin=="RTS")
			sRv= "COM_O";
		else if(pin=="TXD")
			sRv= "COM_O";
		else if(pin=="CTS")
			sRv= "COM_I";
		else if(pin=="DSR")
			sRv= "COM_I";
		else if(pin=="RI")
			sRv= "COM_I";
		else if(pin=="DCD")
			sRv= "COM_I";
		else if(pin == "DATA1")
			sRv= "LPT_O";
		else if(pin == "DATA2")
			sRv= "LPT_O";
		else if(pin == "DATA3")
			sRv= "LPT_O";
		else if(pin == "DATA4")
			sRv= "LPT_O";
		else if(pin == "DATA5")
			sRv= "LPT_O";
		else if(pin == "DATA6")
			sRv= "LPT_O";
		else if(pin == "DATA7")
			sRv= "LPT_O";
		else if(pin == "DATA8")
			sRv= "LPT_O";
		else if(pin == "ERROR")
			sRv= "LPT_I";
		else if(pin == "SELECT")
			sRv= "LPT_I";
		else if(pin == "PAPEREMPTY")
			sRv= "LPT_I";
		else if(pin == "ACKNOWLEDGE")
			sRv= "LPT_I";
		else if(pin == "BUSY")
			sRv= "LPT_I";
		else if(pin == "STROBE")
			sRv= "LPT_O";
		else if(pin == "AUTOFEED")
			sRv= "LPT_O";
		else if(pin == "INIT")
			sRv= "LPT_O";
		else if(pin == "SELECTINPUT")
			sRv= "LPT_O";
		else
			sRv= "UNKNOWN";
		return sRv;
	}

	vector<string> ExternPorts::getChipIDs() const
	{
		vector<string> ids;

		for(map<string, portpin_address_t>::const_iterator it= m_mUsedPins.begin(); it != m_mUsedPins.end(); ++it)
			ids.push_back(it->first);
		return ids;
	}

	bool ExternPorts::existID(const string servertype, string ID) const
	{
		string owntype;
		unsigned long actPort;
		map<unsigned long, int>::const_iterator it;

		if(m_eType == PORT)
			owntype= "PORT";
		else if(m_eType == MPORT)
			owntype= "MPORT";
		else
			owntype= "RWPORT";
		if(servertype != owntype)
			return false;
		actPort= getPortAddress(ID);
		if(actPort == 0)
			return false;
		it= global_mOpenedPorts.find(actPort);
		if(it != global_mOpenedPorts.end())
			return true;
		return false;
	}

	bool ExternPorts::isDebug()
	{
		bool debug;

		Thread::LOCK(m_DEBUGINFO);
		debug= m_bDebug;
		Thread::UNLOCK(m_DEBUGINFO);
		return debug;
	}

	void ExternPorts::setDebug(const bool debug)
	{
		Thread::LOCK(m_DEBUGINFO);
		m_bDebug= debug;
		Thread::UNLOCK(m_DEBUGINFO);
	}

	bool ExternPorts::setPin(const portpin_address_t& tPin, const bool bSet)
	{
		bool ubSet= bSet;
		int actPin;
		map<unsigned long, int>::iterator foundPin;
		Pins ePin= { tPin.nPort, tPin.ePin };
		portpin_address_t tDo;

		tDo= getPortPinAddress(ePin, bSet);
		foundPin= global_mOpenedPorts.find(ePin.nPort);
		if(foundPin == global_mOpenedPorts.end())
			return false;
		/*if(!bSet)
			foundPin->second&= tDo.nPin;
		else
			foundPin->second|= tDo.nPin;*/
		actPin= inb(tDo.nPort + tDo.nAdd);
		if(!bSet)
			actPin&= tDo.nPin;
		else
			actPin|= tDo.nPin;
		if(!tPin.bCacheWriting)
		{
			int pin;
			unsigned long port;

			pin= foundPin->second;
			port= tDo.nPort + tDo.nAdd;
			outb(actPin, port);
		}
		return true;
	}

	short ExternPorts::write(const string id, const double value)
	{
		bool bSet= false;
		map<string, portpin_address_t>::iterator it;

		it= m_mUsedPins.find(id);
		if(it == m_mUsedPins.end())
			return -1;
		if(value > 0)
			bSet= true;
		if(!setPin(it->second, bSet))
			return -1;
		return 0;
	}

	inline short ExternPorts::getPin(const portpin_address_t& tSet)
	{
		int nPin;

		nPin= inb(tSet.nPort + tSet.nAdd);
	#ifdef DEBUG
		if(isDebug())
		{
			cout << "getPin " << getPinName(ePin.ePin) << endl;
			cout << "need ";
			printBin(&tSet.nPin, tSet.nPort);
			cout << "have ";
			printBin(&nPin, tSet.nPort);
		}
	#endif // DEBUG
		if(nPin < 0)
			return -1;
		if(nPin & tSet.nPin)
			return 1;
		return 0;

	}

	short ExternPorts::read(const string id, double &value)
	{
		short res;
		short nRv;
		map<string, portpin_address_t>::const_iterator it;

		if(m_eType == PORT)
		{
			it= m_mUsedPins.find(id);
			if(it == m_mUsedPins.end())
				return -1;
			res= getPin(it->second);
			switch(res)
			{
			case 0:
				value= 0;
				nRv= 0;
			case 1:
				value= 1;
				nRv= 0;
				break;
			default:
				value= 0;
				nRv= -1;
			}
		}else if(m_eType == MPORT)
		{
			value= static_cast<double>(getMeasuredTime(id));
			if(value == 0)
				nRv= -1;
			else
				nRv= 0;
		}
		return nRv;
	}

	unsigned long ExternPorts::getMeasuredTime(const string& id)
	{
		static bool bSet= false;
		static portpin_address_t* ptRead= NULL;
		Pins tRead;
		portpin_address_t tSet;
		portpin_address_t tGet;
		portpin_address_t tNeg;
		unsigned long nSetPort;
		unsigned long nGetPort;
		int nSetPin;
		unsigned nGetPin;
		unsigned long mikroSleepTime= 0;
		struct itimerval time;
		double correction;
		vector<correction_t> vCorrection;

		if(ptRead == NULL)
		{
			if(	bSet == true
				||
				m_mUsedPins.size() == 0	)
			{
				string msg("### ERROR: no COM or LPT port be set for measure in one wire server");

				if(!bSet)
					cerr << msg << endl;
				TIMELOG(LOG_ERROR, "portMeasure", msg);
				bSet= true;
				return 0;
			}
			ptRead= &m_mUsedPins.begin()->second;
			// toDo: why is ioperm not open by MPROT
			//       opening by method connect() is not reached
			ioperm(ptRead->nPort, 8, 1);
			if(ptRead->eOut.nPort != 0 && ptRead->eOut.nPort != ptRead->nPort)
				ioperm(ptRead->eOut.nPort, 8, 1);
			if(ptRead->eNeg.nPort != 0 && ptRead->eNeg.nPort != ptRead->nPort)
				ioperm(ptRead->eNeg.nPort, 8, 1);
			bSet= true;
		}
		tRead.nPort= ptRead->nPort;
		tRead.ePin= ptRead->ePin;
		tGet= getPortPinAddress(tRead, false);
		tSet= getPortPinAddress(ptRead->eOut, true);
		tNeg= getPortPinAddress(ptRead->eNeg, false);
		nGetPort= tGet.nPort + tGet.nAdd;
		nSetPort= tSet.nPort + tSet.nAdd;
		nGetPin= tGet.nPin;
		nSetPin= tSet.nPin;
		setPin(tNeg, false);
		setPin(tSet, false);

		unsigned res;
		int nPrintPin;

		time.it_interval.tv_sec= ITIMERSTARTSEC;
		time.it_interval.tv_usec= 0;
		time.it_value.tv_sec= ITIMERSTARTSEC;
		time.it_value.tv_usec= 0;
		if(isDebug())
		{
			cout << "maximal calculating for " << m_maxMeasuredTime << " mikroseconds" << endl;
			cout << "beginning status: ";
			res= inb(nGetPort);
			nPrintPin= (int)res;
			printBin(&nPrintPin, nGetPort);
			cout << "wait for          ";
			nPrintPin= (int)nGetPin;
			printBin(&nPrintPin, nGetPort);
		}
		if(m_bFreeze)
			lockProcess(true);
		if(setitimer(ITIMERTYPE, &time, NULL)==-1)
		{
			char cError[100];
			string msg("timer ERROR ");

			sprintf(cError, "%d: ", errno);
			msg+= cError;
			msg+= "cannot read correctly time";
			LOG(LOG_ERROR, msg);
			return 0;
		}else
		{
			outb(inb(nSetPort) | nSetPin, nSetPort);
			//cout << flush; // flush after outb() -> maybe an bug
			while(	!(res= inb(nGetPort) & nGetPin)
					&&
					(unsigned long)getMikrotime() < m_maxMeasuredTime	);
			{
				usleep(1);
			}
			mikroSleepTime= getMikrotime();
			setPin(tSet, false);
		}
		if(m_bFreeze)
			lockProcess(false);
		if(isDebug())
		{
			cout << "result:           ";
			nPrintPin= (int)res;
			printBin(&nPrintPin, nGetPort);
		}
		if(mikroSleepTime >= m_maxMeasuredTime)
		{
			ostringstream msg;

			msg << "overflow of measured given time ";
			msg << m_maxMeasuredTime;
			TIMELOG(LOG_ERROR, "MPORT"+id, msg.str());
			return 0;
		}
		return mikroSleepTime;
	}

	string ExternPorts::getBinString(const long value, const size_t bits) const
	{
		string sRv;
		char* byte= new char[bits+1];
		//long* pvalue= value;
		long bit= 0x01;

		memset(byte, '0', bits);
		byte[bits]= '\0';
		for(size_t n= bits-1; n>=0; n--)
		{
			//cout << "value:" << n << " bit:" << bit << endl;
			if(value & bit)
				byte[n]= '1';
			if(n == 0)
				break;
			bit<<= 1;
		}
		sRv= byte;
		delete [] byte;
		return sRv;
	}

	void ExternPorts::printBin(const int* value, const unsigned long nPort) const
	{
		string byte;

		byte= getBinString((long)*value, 8);
		printf("%s(bin) 0x%03X(hex) %3d(dez)  on port:0x%3X\n", byte.c_str(), *(unsigned*)value, *value, (unsigned int)nPort);
		//cout << byte << endl;
		//cout << *value << endl;
	}

	void ExternPorts::lockProcess(const bool bSet)
	{
		static bool blockf= false;
		int res;
		int police;
		struct sched_param spSet=
		{
	    	sched_priority: 0
		};

		/*
		 * toDo:	if getrlimit(RLIMIT_RTPRIO, struct rtlimit) have not privilegs
		 * 			set with setrlimit(RLIMIT_RTPRIO, struct rtlimit) new authority
		 * 			see man setrlimit
		 */
		if(bSet)
		{
			police= SCHED_FIFO;
			spSet.sched_priority= 1;
		}else
			police= SCHED_OTHER;
		res= sched_setscheduler(0, police, &spSet);
		if(res != 0)
		{
			ostringstream msg;
			ostringstream identif;

			msg << "### ERROR: cannot set ";
			if(bSet)
				msg << "real-time scheduling";
			else
				msg << "scheduling to normal";
			msg << endl << "ERRNO(" <<  errno << "): ";
			msg << strerror(res);
			identif << "sched_schedulling" << errno << bSet;
			TIMELOG(LOG_ERROR, identif.str(), msg.str());
			if(bSet && !blockf)
			{
				blockf= true;
				cerr << msg.str() << endl;
			}
		}
	}

	inline unsigned long ExternPorts::getMikrotime()
	{
		int res;
		long sec, lRv;
		struct itimerval timerv;
		struct itimerval *ptimer;

		//usleep(1);
		ptimer= &timerv;
		res= getitimer(ITIMERTYPE, ptimer);
		if(res==-1)
		{
			printf("result:%d\n", res);
			printf("ERROR: %d\n", errno);
			perror("getitimer");
			if(res==EFAULT)
				printf("      value oder ovalue sind keine gültigen Pointer.\n");
			else if(res==EINVAL)
				printf("      which ist weder ITIMER_REAL noch ITIMER_VIRTUAL noch ITIMER_PROF.\n");
			return -1;

		}

		sec= ITIMERSTARTSEC - timerv.it_value.tv_sec;
		lRv= 1000000 - timerv.it_value.tv_usec;
		if(sec > 1)
		{
			lRv+= 1000000 * sec;
		}
		return lRv;
	};

	void ExternPorts::range(const string id, double& min, double& max, bool &bfloat)
	{
		if(m_eType == PORT)
		{
			min= 0;
			max= 1;
			bfloat= false;
		}else
		{ // m_eType == MPORT
			min= 0;
			max= static_cast<double>(m_maxMeasuredTime);
			bfloat= false;
		}
	}

	void ExternPorts::endOfCacheReading(const double cachetime)
	{

	}

	bool ExternPorts::reachAllChips()
	{
		return true;
	}

	void ExternPorts::endOfLoop()
	{
		// nothing to do
	}

	ExternPorts::~ExternPorts()
	{
		DESTROYMUTEX(m_DEBUGINFO);
	}

}
