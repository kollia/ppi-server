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
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <sys/io.h>
#include <sys/time.h>
//Standard http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1124.pdf für bool
#include <stdbool.h>
#include <string.h>

#include "portbaseclass.h"

#include "../logger/lib/LogInterface.h"

#include "../database/lib/DbInterface.h"

using namespace ports;
using namespace ppi_database;

portBase::portBase(string type, string folderName, string subroutineName)
{
#ifdef DEBUG
	m_count= 0;
#endif //DEBUG
	m_bDefined= false;
	m_bFloat= true;
	m_dMin= 1;
	m_dMax= 0;
	m_bSwitch= false;
	m_bDebug= false;
	m_bCanAfterContact= true;
	m_sType= type;
	m_sFolder= folderName;
	m_sSubroutine= subroutineName;
	m_bWriteDb= false;
	m_dValue= 0;
	// do not know access from database
	m_pbCorrectDevice= auto_ptr<bool>();
	m_VALUELOCK= Thread::getMutex("VALUELOCK");
	m_DEBUG= Thread::getMutex("portBaseDEBUG");
	m_CORRECTDEVICEACCESS= Thread::getMutex("CORRECTDEVICEACCESS");
}

bool portBase::init(ConfigPropertyCasher &properties)
{
	bool exist;
	double ddv, dDef;
	string prop("default");
	DbInterface *db= DbInterface::instance();

	dDef= properties.getDouble(prop, /*wrning*/false);
	m_sPermission= properties.getValue("perm", /*warning*/false);
	m_bWriteDb= properties.haveAction("db");
	if(m_bWriteDb)
		db->writeIntoDb(m_sFolder, m_sSubroutine);

	ddv= db->getActEntry(exist, m_sFolder, m_sSubroutine, "value");
	if(!exist)
	{// write alway the first value into db
	 // if it was not in database
	 // because the user which hearing for this subroutine
	 // gets elsewise an error
		if(prop != "#ERROR")
			m_dValue= dDef;
		db->fillValue(m_sFolder, m_sSubroutine, "value", m_dValue);
	}else
		m_dValue= ddv;

	defineRange();
	registerSubroutine();
	return true;
}

void portBase::defineRange()
{
	// min max be defined for full range
	bool bFloat= true;
	double min= 1;
	double max= 0;
	double* pmin= &min;
	double* pmax= &max;

	m_bDefined= range(bFloat, pmin, pmax);
	if(m_bDefined)
	{
		m_bFloat= bFloat;
		if(	!bFloat
			&&
			pmin
			&&
			*pmin == 0
			&&
			pmax
			&&
			*pmax == 1	)
		{
			m_bSwitch= true;
			m_dMin= 0x00;
			m_dMax= 0x03;
		}else
		{
			if(pmin && pmax)
			{
				m_dMin= *pmin;
				m_dMax= *pmax;
			}
		}
	}
}

void portBase::registerSubroutine()
{
	string server("measureRoutine");
	string chip("###DefChip ");
	DbInterface *reader= DbInterface::instance();

	chip+= m_sFolder + " ";
	chip+= m_sSubroutine;
	reader->registerChip(server, chip, "0", m_sType, "measure", &m_dMin, &m_dMax, &m_bFloat);
	reader->registerSubroutine(m_sSubroutine, m_sFolder, server, chip);
}

string portBase::getPermissionGroups()
{
	return m_sPermission;
}

void portBase::setDeviceAccess(bool access)
{
	bool same= false;
	DbInterface* db;

	LOCK(m_CORRECTDEVICEACCESS);
	if(m_pbCorrectDevice.get())
	{
		if(*m_pbCorrectDevice == access)
			same= true;
		else
			*m_pbCorrectDevice= access;
	}else
	{// for the first definition,
	 // database not be running
	 // so write access value only in database message loop
	 // and define actualy Device access
		m_pbCorrectDevice= auto_ptr<bool>(new bool);
		*m_pbCorrectDevice= access;
	}
	UNLOCK(m_CORRECTDEVICEACCESS);
	if(same)
		return;
	db= DbInterface::instance();
	db->fillValue(m_sFolder, m_sSubroutine, "access", (double)access);
}

void portBase::setObserver(IMeasurePattern* observer)
{
	// nothing to do
	//+++++++++++++++++++++++++++++++++++++++++
	// method only for overload
	// to start own folder thread
	// to get next time by changing foreign value
	// this value from the other folder
}

void portBase::informObserver(IMeasurePattern* observer, const string& folder)
{
	m_mvObservers[observer].push_back(folder);
}

bool portBase::hasDeviceAccess() const
{
	bool bDevice;

	LOCK(m_CORRECTDEVICEACCESS);
	if(m_pbCorrectDevice.get())
		bDevice= *m_pbCorrectDevice;
	else// if no CorrectDevice be defined
		// the subroutine is an SWITCH, VALUE, COUNTER, TIMER or SHELL
		// and this devices (ports) are always true
		bDevice= true;
	UNLOCK(m_CORRECTDEVICEACCESS);
	return bDevice;
}

void portBase::setValue(double value)
{
	double dbvalue= value;
	double oldMember= m_dValue;
	DbInterface* db;

	if(!m_bDefined)// if device not found by starting in init method
		defineRange(); // try again, maybe device was found meantime
	if(m_bDefined)
	{
		//cout << "value:" << dec << value << endl;
		//cout << "min:" << dec << m_dMin << endl;
		//cout << "max:" << dec << m_dMax << endl;
		if(m_bSwitch)
		{
			dbvalue= ((int)value & 0x01) ? 1 : 0;
			oldMember= ((int)m_dValue & 0x01) ? 1 : 0;
		}
		if(m_dMin < m_dMax)
		{
			if(value < m_dMin)
				value= m_dMin;
			if(value > m_dMax)
				value= m_dMax;
		}
		if(!m_bFloat)
			value= (double)((long)value);
		if(!m_bSwitch)
			dbvalue= value;
	}
	if(value != m_dValue)
	{
		LOCK(m_VALUELOCK);
		m_dValue= value;
		UNLOCK(m_VALUELOCK);

		for(map<IMeasurePattern*, vector<string> >::iterator it= m_mvObservers.begin(); it != m_mvObservers.end(); ++it)
		{
			for(vector<string>::iterator fit= it->second.begin(); fit != it->second.end(); ++fit)
			{
				it->first->changedValue(*fit);
				// toDo: delete break when folder threads running in one thread
				break;
			}
		}
		if(dbvalue != oldMember)
		{
			db= DbInterface::instance();
			db->fillValue(m_sFolder, m_sSubroutine, "value", dbvalue);
		}
	}
}

double portBase::getValue(const string who)
{
	double dValue;

	LOCK(m_VALUELOCK);
	dValue= m_dValue;
	UNLOCK(m_VALUELOCK);
	return dValue;
}

string portBase::getFolderName()
{
	return m_sFolder;
}

string portBase::getSubroutineName()
{
	return m_sSubroutine;
}

void portBase::setDebug(bool bDebug)
{
	LOCK(m_DEBUG);
	m_bDebug= bDebug;
	UNLOCK(m_DEBUG);
}

bool portBase::isDebug()
{
	bool debug;

	LOCK(m_DEBUG);
	debug= m_bDebug;
	UNLOCK(m_DEBUG);

	return debug;
}

void portBase::noAfterContactPublication()
{
	m_bCanAfterContact= false;
}

void portBase::setAfterContact(const map<unsigned long, unsigned> &ports, const set<portBase::Pins> &pins)
{
	m_vAfterContactPorts= ports;
	m_vAfterContactPins= pins;
}

bool portBase::doForAfterContact()
{
	return m_bCanAfterContact;
}

string portBase::getBinString(const long value, const size_t bits)
{
	ostringstream bRv;
	long bit= 0x01;

	for(size_t n= bits-1; n>=0; n--)
	{
		//cout << "value:" << n << " bit:" << bit << endl;
		if(value & bit)
			bRv << "1";
		else
			bRv << "0";
		if(n == 0)
			break;
		bit<<= 1;
	}
	return bRv.str();
}

void portBase::printBin(int* value, unsigned long nPort)
{
	string byte;

	byte= getBinString((long)*value, 8);
	printf("%s(bin) 0x%03X(hex) %3d(dez)  on port:0x%3X\n", byte.c_str(), *(unsigned*)value, *value, (unsigned int)nPort);
	//cout << byte << endl;
	//cout << *value << endl;
}

string portBase::getPinName(Pin ePin)
{
	ostringstream sPinName;

	if(ePin==DTR)
		sPinName << "DTR";
	else if(ePin==RTS)
		sPinName << "RTS";
	else if(ePin==TXD)
		sPinName << "TXD";
	else if(ePin==CTS)
		sPinName << "CTS";
	else if(ePin==DSR)
		sPinName << "DSR";
	else if(ePin==RI)
		sPinName << "RI";
	else if(ePin==DCD)
		sPinName << "DCD";
	else
		sPinName << "unknown";
	return sPinName.str();
}

portBase::Pin portBase::getPinEnum(string sPinName)
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

unsigned long portBase::getPortAddress(string sPortName)
{
	unsigned long lRv;

	if(sPortName=="COM1")
		lRv= COM1;
	else if(sPortName=="COM2")
		lRv= COM2;
	else if(sPortName=="COM3")
		lRv= COM3;
	else if(sPortName=="COM4")
		lRv= COM4;
	else if(sPortName=="LPT1")
		lRv= LPT1;
	else if(sPortName=="LPT2")
		lRv= LPT2;
	else if(sPortName=="LPT3")
		lRv= LPT3;
	else
		lRv= 0x00;

	return lRv;
}

portBase::Pin portBase::getPortType(string sPortName)
{
	unsigned long port= getPortAddress(sPortName);

	return getPortType(port);
}

portBase::Pin portBase::getPortType(unsigned long port)
{
	Pin eRv;

	if(port==COM1)
		eRv= COM;
	else if(port==COM2)
		eRv= COM;
	else if(port==COM3)
		eRv= COM;
	else if(port==COM4)
		eRv= COM;
	else if(port==LPT1)
		eRv= LPT;
	else if(port==LPT2)
		eRv= LPT;
	else if(port==LPT3)
		eRv= LPT;
	else
		eRv= NONE;
	return eRv;
}

string portBase::getPortName(unsigned long nPort)
{
	string sPortName;

	if(nPort==COM1)
		sPortName= "COM1";
	else if(nPort==COM2)
		sPortName= "COM2";
	else if(nPort==COM3)
		sPortName= "COM3";
	else if(nPort==COM4)
		sPortName= "COM4";
	else if(nPort==LPT1)
		sPortName= "LPT1";
	else if(nPort==LPT2)
		sPortName= "LPT2";
	else if(nPort==LPT3)
		sPortName= "LPT3";
	else
		sPortName= "unknown";
	return sPortName;
}

portBase::Pins portBase::getPinsStruct(string sPin)
{
	Pins tRv;
	vector<string> pin= ConfigPropertyCasher::split(sPin, ":");

	if(pin.size() < 2)
	{
		tRv.nPort= 0;
		tRv.ePin= NONE;
	}else
	{
		tRv.sPort= pin[0];
		tRv.nPort= getPortAddress(pin[0]);
		tRv.sPin= pin[1];
		tRv.ePin= getPinEnum(pin[1]);
	}
	return tRv;
}

void portBase::setDTR(unsigned long port, bool set)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= DTR;
	portBase::setPin(tAdr, set);
}

void portBase::setRTS(unsigned long port, bool set)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= RTS;
	portBase::setPin(tAdr, set);
}

void portBase::setTXD(unsigned long port, bool set)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= TXD;
	portBase::setPin(tAdr, set);
}

void portBase::setPin(Pins ePin, bool bSet)
{
	portpin_address_t tSet;
	portpin_address_t tDo= getPortPinAddress(ePin, bSet);
	unsigned state= inb(tDo.nPort);

	if(!bSet)
	{
		tSet= getPortPinAddress(ePin, true);
		if(state & tSet.nPin)
			state&= tDo.nPin;
	}else
		state|= tDo.nPin;
	outb(state, tDo.nPort);
	return;
}

bool portBase::getCTS(unsigned long port)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= CTS;
	return portBase::getPin(tAdr);

/*	int res;
	int nPin= 0x10;

	if(bAfter)
		nPin|= 0x01;
	res= inb(port+6);// 00010001  begin signal
	if(res & 0x0010) // 00010000 = 16 dez
		return true;	 // 00000001  had signal
	return false;*/
}

bool portBase::getDSR(unsigned long port)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= DSR;
	return portBase::getPin(tAdr);

/*	int res;
	int nPin= 0x20;

	if(bAfter)
		nPin|= 0x02;
	res= inb(port+6);// 00100010  begin signal
	printf("%d\n",res);
	if(res & 0x0020) // 00100000 = 32 dez
		return true;	 // 00000010  had signal
	return false;*/
}

bool portBase::getRI(unsigned long port)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= RI;
	return portBase::getPin(tAdr);

/*	int res;
	int nPin= 0x40;

	if(bAfter)
		nPin|= 0x04;
	res= inb(port+6);
	if(res & 0x0040) // 01000000 = 64 dez
		return true;	 // 00000100  had connection
	return false;*/
}

portBase::portpin_address_t portBase::getPortPinAddress(Pins tAdr, bool bSetAfter)
{
	portpin_address_t tRv;

	tRv.nPort= tAdr.nPort;
	switch(tAdr.ePin)
	{
		case DTR:
			if(bSetAfter)
				tRv.nPin= 0x01; // 00000001
			else
				tRv.nPin= 0xFE; // 11111110
			tRv.nPort+= 4;
			tRv.eDescript= SETPIN;
			tRv.ePort= COM;
			break;
		case RTS:
			if(bSetAfter)
				tRv.nPin= 0x02; // 00000010
			else
				tRv.nPin= 0xFD; // 11111101
			tRv.nPort+= 4;
			tRv.eDescript= SETPIN;
			tRv.ePort= COM;
			break;
		case TXD:
			if(bSetAfter)
				tRv.nPin= 0x40; // 01000000
			else
				tRv.nPin= 0xBF; // 10111111
			tRv.nPort+= 3;
			tRv.eDescript= SETPIN;
			tRv.ePort= COM;
			break;
		case CTS:
			tRv.nPin= 0x10; // 00010000
			if(bSetAfter)
				tRv.nPin|= 0x01; // 00000001
			tRv.nPort+= 6;
			tRv.eDescript= GETPIN;
			tRv.ePort= COM;
			break;
		case DSR:
			tRv.nPin= 0x20; // 00100000
			if(bSetAfter)
				tRv.nPin|= 0x02; // 00000010
			tRv.nPort+= 6;
			tRv.eDescript= GETPIN;
			tRv.ePort= COM;
			break;
		case RI:
			tRv.nPin= 0x40; // 01000000
			if(bSetAfter)
				tRv.nPin|= 0x04; // 00000100
			tRv.nPort+= 6;
			tRv.eDescript= GETPIN;
			tRv.ePort= COM;
			break;
		case DCD:
			tRv.nPin= 0x80; // 10000000
			if(bSetAfter)
				tRv.nPin|= 0x08; // 00001000
			tRv.nPort+= 6;
			tRv.eDescript= GETPIN;
			tRv.ePort= COM;
			break;
		default:
			tRv.nPin= 0x00;
			tRv.nPort= 0x0000;
			tRv.eDescript= NONE;
			tRv.ePort= NONE;
			break;
	}
	return tRv;
}

bool portBase::getDCD(unsigned long port)
{
	Pins tAdr;

	tAdr.nPort= port;
	tAdr.ePin= DCD;
	return portBase::getPin(tAdr);

/*	int res;
	int nPin= 0x80;

	if(bAfter)
		nPin|= 0x08;
	res= inb(port+6);// 10001000  begin signal
	if(res & 0x0080) // 10000000 = 128 dez
		return true;	 // 10001000  had signal
	return false;*/
}

bool portBase::getPin(Pins ePin)
{
	typedef map<unsigned long, unsigned>::iterator iter;
	bool bFoundPort= false;
	bool bFoundPin= false;
	int nPin= 0x00;
	portpin_address_t tSet= getPortPinAddress(ePin, true);// fetch PortPinAddress for after contact

	if(!m_vAfterContactPorts.empty())
	{
		for(iter i= m_vAfterContactPorts.begin(); i!=m_vAfterContactPorts.end(); ++i)
		{
			if(i->first==tSet.nPort)
			{
				bFoundPort= true;
				nPin= i->second;
				break;
			}
		}
		for(set<portBase::Pins>::iterator s= m_vAfterContactPins.begin(); s!=m_vAfterContactPins.end(); ++s)
		{
			if(s->ePin == ePin.ePin)
			{
				bFoundPin= true;
				break;
			}
		}
		if(	bFoundPort
			&&
			!bFoundPin	)
		{// found port, but pin is not for after contact
		 // fetch new PortPinAddress
			tSet= getPortPinAddress(ePin, false);
		}
	}
	if(!bFoundPort)
	{
		//cout << "does not found" << endl;
		tSet= getPortPinAddress(ePin, false);
		nPin= inb(tSet.nPort);
	}

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
	if(nPin & tSet.nPin)
		return true;
	return false;

/*	switch(ePin.ePin)
	{
		case CTS:
			return getCTS(ePin.nPort, bAfter);
			break;
		case DSR:
			return getDSR(ePin.nPort, bAfter);
			break;
		case RI:
			return getRI(ePin.nPort, bAfter);
			break;
		case DCD:
			return getDCD(ePin.nPort, bAfter);
		default:
#ifdef DEBUG
			char *sPort= getPortName(ePin.nPort);
			char *sPin= getPinName(ePin.ePin);

			printf("on port %s cannot get signal from pin %s\n", sPort, sPin);
			delete sPort;
			delete sPin;
#endif //DEBUG
			break;
	}
	return false;*/
}

void portBase::lockApplication(bool bSet)
{
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
		if(bSet)
			printf("set real-time scheduling is %d\n", res);
		else
			printf("set scheduling to normal is %d\n", res);
		printf("ERROR %d\n", errno);
		perror("sched_setscheduller");
	}
}

bool portBase::onlySwitch()
{
	return m_bSwitch;
}

bool portBase::measure()
{
	return false;
}

portBase::~portBase()
{
	DESTROYMUTEX(m_VALUELOCK);
	DESTROYMUTEX(m_DEBUG);
	DESTROYMUTEX(m_CORRECTDEVICEACCESS);
}
