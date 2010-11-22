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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "portbaseclass.h"

#include "../util/thread/Thread.h"

#include "../logger/lib/LogInterface.h"

#include "../database/lib/DbInterface.h"

using namespace ports;
using namespace ppi_database;
using namespace boost;

portBase::portBase(const string& type, const string& folderName, const string& subroutineName)
: m_poMeasurePattern(NULL),
  m_oLinkWhile(folderName, subroutineName, "lwhile", false, false),
  m_dLastLinkValue(0),
  m_nLinkObserver(0)
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
	m_OBSERVERLOCK= Thread::getMutex("OBSERVERLOCK");
}

bool portBase::init(IActionPropertyPattern* properties)
{
	bool exist;
	double ddv, dDef;
	string prop("default");
	DbInterface *db= DbInterface::instance();

	dDef= properties->getDouble(prop, /*wrning*/false);
	m_sPermission= properties->getValue("perm", /*warning*/false);
	m_bWriteDb= properties->haveAction("db");
	if(m_bWriteDb)
		db->writeIntoDb(m_sFolder, m_sSubroutine);

	ddv= db->getActEntry(exist, m_sFolder, m_sSubroutine, "value");
	if(!exist)
	{// write always the first value into db
	 // if it was not in database
	 // because the user which hearing for this subroutine
	 // gets otherwise an error
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
	m_oLinkWhile.activateObserver(observer);
}

void portBase::informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
{
	string inform(folder+":"+subroutine+" "+parameter);
	vector<string> vec;
	vector<string>::iterator found;

	if(folder == getFolderName())
		return;
	LOCK(m_OBSERVERLOCK);
	vec= m_mvObservers[observer];
	found= find(vec.begin(), vec.end(), inform);
	if(found == vec.end())
		m_mvObservers[observer].push_back(inform);
	UNLOCK(m_OBSERVERLOCK);
}

void portBase::removeObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
{
	string remove(folder+":"+subroutine+" "+parameter);
	map<IMeasurePattern*, vector<string> >::iterator foundT;
	vector<string>::iterator foundS;

	LOCK(m_OBSERVERLOCK);
	foundT= m_mvObservers.find(observer);
	if(foundT != m_mvObservers.end())
	{
		foundS= find(foundT->second.begin(), foundT->second.end(), remove);
		if(foundS != foundT->second.end())
		{
			foundT->second.erase(foundS);
			if(foundT->second.size() == 0)
				m_mvObservers.erase(foundT);
		}
	}
	UNLOCK(m_OBSERVERLOCK);
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

void portBase::setValue(double value, const string& from)
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
		bool debug(isDebug());
		string sOwn(m_sFolder+":"+m_sSubroutine);
		ostringstream output;

		LOCK(m_VALUELOCK);
		m_dValue= value;
		UNLOCK(m_VALUELOCK);

		LOCK(m_OBSERVERLOCK);
		if(debug && m_mvObservers.size() > 0)
		{
			output << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\" << endl;
			output << "  " << sOwn << " was changed to " << m_dValue << endl;
			if(	from.substr(0, 1) != "i" ||
				from.substr(2) != sOwn		)
			{
				output << "  was informed from";
				if(from.substr(0, 1) == "e")
					output << " internet account ";
				else
					output << ": ";
				output << from.substr(2) << endl;
			}
		}
		for(map<IMeasurePattern*, vector<string> >::iterator it= m_mvObservers.begin(); it != m_mvObservers.end(); ++it)
		{
			for(vector<string>::iterator fit= it->second.begin(); fit != it->second.end(); ++fit)
			{
				string::size_type pos;

				pos= fit->find(" ");
				if(	from.substr(0, 1) != "i" ||
					(	(	pos != string::npos &&
							from.substr(2) != fit->substr(0, pos)	) ||
						(	pos == string::npos &&
							from.substr(2) != *fit	) 					)	)
				{
					if(debug)
						output << "    inform " << *fit << endl;
					it->first->changedValue(*fit, sOwn);

				}else if(debug)
					output << "    do not inform " << *fit << " back" << endl;
				break;
			}
		}
		if(debug &&m_mvObservers.size() > 0)
		{
			output << "////////////////////////////////////////" << endl;
			cout << output.str();
		}
		UNLOCK(m_OBSERVERLOCK);
		if(dbvalue != oldMember)
		{
			db= DbInterface::instance();
			db->fillValue(m_sFolder, m_sSubroutine, "value", dbvalue);
		}
	}
}

double portBase::getValue(const string& who)
{
	double dValue;

	LOCK(m_VALUELOCK);
	dValue= m_dValue;
	UNLOCK(m_VALUELOCK);
	return dValue;
}

bool portBase::initLinks(const string& type, IPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bOk= true, bWarning;
	string sValue, sLinkWhile;
	vector<string>::size_type nValue;

	if(m_sType != type)
		return true;
	nValue= properties->getPropertyCount("link");
	for(vector<string>::size_type i= 0; i<nValue; ++i)
	{
		ostringstream lk;
		ListCalculator* calc;
		vector<string> spl;

		lk << "link[" << i+1 << "]";
		sValue= properties->getValue("link", i);
		trim(sValue);
		split(spl, sValue, is_any_of(":"));
		if(spl.size() > 0)
			trim(spl[0]);
		if(spl.size() == 2)
			trim(spl[1]);
		if(	(	spl.size() == 1 &&
				spl[0].find(" ") == string::npos	) ||
			(	spl.size() == 2 &&
				spl[0].find(" ") == string::npos &&
				spl[1].find(" ") == string::npos	)	)
		{
			m_vpoLinks.push_back(new ListCalculator(m_sFolder, m_sSubroutine, lk.str(), true, false));
			calc= m_vpoLinks.back();
			if(!calc->init(pStartFolder, sValue))
				bOk= false;

		}else
		{
			ostringstream msg;

			msg << properties->getMsgHead(/*error*/true);
			msg << i << ". link parameter '"  << sValue << "' can only be an single [folder:]<sburoutine>, so do not set this link";
			LOG(LOG_ERROR, msg.str());
			cout << msg.str() << endl;
			bOk= false;
		}
	}
	bWarning= false;
	if(nValue > 1)
		bWarning= true;
	sLinkWhile= properties->getValue("lwhile", bWarning);
	if(!m_oLinkWhile.init(pStartFolder, sLinkWhile))
		bOk= false;
	return bOk;
}

bool portBase::getLinkedValue(const string& type, double& val)
{
	bool bOk, isdebug;
	double lvalue, linkvalue;
	string slink, foldersub;
	vector<string>::size_type pos;
	vector<ListCalculator*>::size_type links(m_vpoLinks.size());
	ListCalculator* link;
	IListObjectPattern* port;

	if(m_sType != type)
		return false;
	if(links > 0)
	{
		isdebug= isDebug();
		if(isdebug)
		{
			cout << "  __________________" << endl;
			cout << " << check link's >>>>" << endl;
		}
		foldersub= m_sFolder+":"+m_sSubroutine;
		// create first lwhile parameter
		if(!m_oLinkWhile.isEmpty())
		{
			bOk= m_oLinkWhile.calculate(lvalue);
			if(bOk)
			{
				if(	lvalue < 1 ||
					lvalue > links	)
				{
					if(lvalue != 0)
					{
						string msg("calculation of lwhile parameter is out of range but also not 0\n");

						msg += "             so do not create any link to foreign subroutine";
						if(isdebug)
							cout << "### WARNING: " << msg << endl;
						msg= "in folder '"+m_sFolder+"' and subroutine '"+m_sSubroutine+"'\n"+msg;
						msg+= "\n";
						TIMELOG(LOG_WARNING, m_sFolder+m_sSubroutine+"linkwhile", msg);
						bOk= false; // no linked value be used

					}else if(isdebug)
						cout << "calculation of lvhile parameter is 0, so take own value" << endl;
					slink= foldersub;
					pos= 0;
					bOk= false;

				}else
				{
					pos= static_cast<vector<string>::size_type >(lvalue);
					link= m_vpoLinks[pos-1];
					slink= link->getStatement();
				}
			}else
			{
				string msg("cannot create calculation from lwhile parameter '");

				msg+= m_oLinkWhile.getStatement();
				msg+= "' in folder " + m_sFolder + " and subroutine " + m_sSubroutine;
				TIMELOG(LOG_ERROR, "calcResult"+m_sFolder+":"+m_sSubroutine, msg);
				if(isdebug)
					cout << "### ERROR: " << msg << endl;
				bOk= false;
			}
		}else
			slink= m_vpoLinks[0]->getStatement();

		if(	bOk &&
			slink != m_sSubroutine &&
			slink != foldersub		)
		{
			bOk= link->calculate(linkvalue);
			if(!bOk)
			{
				ostringstream msg;

				msg << "cannot find subroutine '";
				msg << link << "' from " << dec << pos << ". link parameter";
				msg << " in folder " << m_sFolder << " and subroutine " << m_sSubroutine;
				TIMELOG(LOG_ERROR, "searchresult"+m_sFolder+":"+m_sSubroutine, msg.str());
				if(isdebug)
					cout << "### ERROR: " << msg << endl;
				bOk= false;
			}else
			{
				// define which value will be use
				// the linked value or own
				if(m_nLinkObserver != pos)
				{// subroutine link to new other subroutine -> take now this value
				 // set observer to linked subroutine
					if(isdebug)
						cout << "link was changed from " << m_nLinkObserver << ". to " << pos << endl;
					if(m_nLinkObserver)
						m_vpoLinks[m_nLinkObserver-1]->removeObserver( m_poMeasurePattern);
					if(pos > 0)
						link->activateObserver( m_poMeasurePattern);
					m_nLinkObserver= pos;
					//defineRange();
					val= linkvalue;
					bOk= true;

				}else if(m_dLastLinkValue != val)
				{ // value changed from outside of server, owreader, or with subroutine SET
				  // write new value inside foreign subroutine
					port= link->getSubroutine(slink, /*own folder*/true);
					if(port)
					{
						if(isdebug)
						{
							cout << "value was changed in own subroutine or outside," << endl;
							cout << "set foreign subroutine " << slink << " to " << dec << val << endl;
						}
						port->setValue(val, "i:"+foldersub);
						port->getRunningThread()->changedValue(slink, foldersub);
					}else
					{
						ostringstream err;

						err << dec << pos << ". link '" << slink << "' is no correct subroutine";
						if(isdebug)
							cerr << "## ERROR: " << err.str() << endl;
						TIMELOG(LOG_ERROR, "incorrecttimerlink"+foldersub, "In TIMER routine of foler '"+m_sFolder+
																		"' and subroutine '"+m_sSubroutine+"'\n"+err.str());
					}
					bOk= false;

				}else if(m_dLastLinkValue != linkvalue)
				{// linked value from other subroutine was changed
					if(isdebug)
						cout << "take changed value " << linkvalue << " from foreign subroutine " << slink << endl;
					val= linkvalue;
					bOk= true;

				}else
				{ // nothing was changed
					if(isdebug)
						cout << "no changes be necessary" << endl;
					bOk= false;
				}
			}
		}else // else of if(slink own subroutine)
		{
			if(isdebug)
				cout << "link is showen to owen subroutine, make no changes" << endl;
			if(m_nLinkObserver != 0)
			{
				if(isdebug && bOk)
					cout << pos << ". link value '" << slink << "' link to own subroutine" << endl;
				if(	m_nLinkObserver-1 < links	)
				{
					m_vpoLinks[m_nLinkObserver]->removeObserver( m_poMeasurePattern);
					m_nLinkObserver= 0;
					//defineRange();
				}else
				{
					cout << "### ERROR: in " << m_sFolder << ":" << m_sSubroutine;
					cout << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
				}
			}
			bOk= false;
		} // end else if(slink own subroutine)

		if(isdebug)
		{
			cout << " << end of check >>>>" << endl;
			cout << "   --------------" << endl;
		}
	}else // if(links > 0)
		bOk= false;

	m_dLastLinkValue= val;
	if(	!bOk ||
		slink == m_sSubroutine ||
		slink == foldersub			)
	{
		return false;
	}
	return true;
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
	m_oLinkWhile.doOutput(bDebug);
	for(vector<ListCalculator*>::iterator it= m_vpoLinks.begin(); it != m_vpoLinks.end(); ++it)
		(*it)->doOutput(bDebug);
	LOCK(m_DEBUG);
	m_bDebug= bDebug;
	UNLOCK(m_DEBUG);
}

bool portBase::range(bool& bfloat, double* min, double* max)
{
	if(m_nLinkObserver)
	{
		IListObjectPattern* port;

		port= m_oLinkWhile.getSubroutine(m_vpoLinks[m_nLinkObserver-1]->getStatement(), /*own folder*/true);
		if(	port &&
			(	port->getFolderName() != m_sFolder ||
				port->getSubroutineName() != m_sSubroutine	)	)
		{
			return port->range(bfloat, min, max);
		}
	}
	return false;
}

bool portBase::isDebug()
{
	bool debug;

	LOCK(m_DEBUG);
	debug= m_bDebug;
	UNLOCK(m_DEBUG);

	return debug;
}

#if 0
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

	for(long n= bits-1; n>=0; n--)
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
#endif

void portBase::lockApplication(bool bSet)
{
	int res;
	int police;
	sched_param spSet;

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
	{
		police= SCHED_OTHER;
		spSet.sched_priority= 0;
	}
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

portBase::~portBase()
{
	vector<ListCalculator*>::iterator it;

	DESTROYMUTEX(m_VALUELOCK);
	DESTROYMUTEX(m_DEBUG);
	DESTROYMUTEX(m_CORRECTDEVICEACCESS);
	for(it= m_vpoLinks.begin(); it != m_vpoLinks.end(); ++it)
		delete *it;
}
