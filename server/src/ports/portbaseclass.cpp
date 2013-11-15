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
//Standard http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1124.pdf for boolean
#include <stdbool.h>
#include <string.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "portbaseclass.h"

#include "../util/exception.h"
#include "../util/thread/Thread.h"
#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

using namespace ports;
using namespace ppi_database;
using namespace boost;

portBase::portBase(const string& type, const string& folderName, const string& subroutineName)
: m_poMeasurePattern(NULL),
  m_oLinkWhile(folderName, subroutineName, "lwhile", false, false),
  m_dLastSetValue(0),
  m_nLinkObserver(0)
{
#ifdef DEBUG
	m_count= 0;
#endif //DEBUG
	m_nCount= 0;
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

bool portBase::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool exist;
	double ddv, dDef;
	string prop("default");
	DbInterface *db= DbInterface::instance();

	m_dValue= 0;
	m_pFolders= pStartFolder;
	m_bInfo= !properties->haveAction("noinfo");
	m_sPermission= properties->getValue("perm", /*warning*/false);
	m_bWriteDb= properties->haveAction("db");
	m_bSwitch= properties->haveAction("binary");
	dDef= properties->getDouble(prop, /*warning*/false);
	if(prop != "#ERROR")
		m_dValue= dDef;
	if(m_bWriteDb)
	{
		db->writeIntoDb(m_sFolder, m_sSubroutine);
		ddv= db->getActEntry(exist, m_sFolder, m_sSubroutine, "value");
	}else
		exist= false;
	if(!exist)
	{// write always the first value into db
	 // if it was not in database
	 // because the user which hearing for this subroutine
	 // gets otherwise an error.
	 // also when no permission be set and db not need for user
	 // because otherwise root can not set subroutine with no db and no permission
	 // set to DEBUG
		db->fillValue(m_sFolder, m_sSubroutine, "value", m_dValue);

	}else
		m_dValue= ddv;
	m_sErrorHead= properties->getMsgHead(/*error message*/true);
	m_sWarningHead= properties->getMsgHead(/*error message*/false);
	m_sMsgHead= properties->getMsgHead();// without error identification

	defineRange();
	registerSubroutine();
	return true;
}

string portBase::getSubroutineMsgHead(bool *error/*= NULL*/)
{
	if(error == NULL)
		return m_sMsgHead;
	if(*error)
		return m_sErrorHead;
	return m_sWarningHead;
}

string portBase::getSubroutineMsgHead(bool error)
{
	bool *perror;
	string sRv;

	perror= new bool;
	*perror= error;
	sRv= getSubroutineMsgHead(perror);
	delete perror;
	return sRv;
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
		if(	m_bSwitch &&
			!bFloat &&
			pmin &&
			*pmin == 0 &&
			pmax &&
			*pmax == 1		)
		{
			m_dMin= 0b00;
			m_dMax= 0b10;
		}else
		{
			if(pmin)
				m_dMin= *pmin;
			else
				m_dMin= 0;
			if(pmax)
				m_dMax= *pmax;
			else
				m_dMax= m_dMin >= 0 ? -1 : m_dMin -1;
			if(	m_bSwitch &&
				(	m_bFloat ||
					m_dMin != 0 ||
					m_dMax != 1		)	)
			{
				string msg;

				msg=  getSubroutineMsgHead(/*error message*/false);
				msg+= "cannot define as binary when subroutine";
				if(bFloat)
					msg+= " defined as float";
				if(m_dMin != 0)
					msg+= ", minimal value not 0";
				if(m_dMax != 1)
					msg+= " and maximal not 1";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
				m_bSwitch= false;
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
	getRunningThread()->fillValue(m_sFolder, m_sSubroutine, "access", (double)access);
}

void portBase::setObserver(IMeasurePattern* observer)
{
	if(m_nCount == 0) // set actually count number of subroutine inside folder
		m_nCount= observer->getActCount();
	m_oLinkWhile.activateObserver(observer);
}

void portBase::informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
{
	typedef map<IMeasurePattern*, vector<string> > measureMap;

	string inform(folder+":"+subroutine+" "+parameter);
	vector<string> vec;
	vector<string>::iterator found;
	measureMap::iterator fObs;

	try{
		if(	folder == getFolderName() &&
			(	subroutine == getSubroutineName() ||
				observer->getActCount(subroutine) < m_nCount	)	)
		{// do not inform subroutine from own folder list, which running after own
			return;
		}
	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("subroutine " + getFolderName() + ":" + getSubroutineName() +
						" should inform "+inform+", maybe observer is undefined");
		err= ex.getTraceString();
		cerr << err << endl;
		LOG(LOG_ERROR, err);
		return;
	}
	LOCK(m_OBSERVERLOCK);
	fObs= m_mvObservers.find(observer);
	if(fObs != m_mvObservers.end())
	{
		vec= m_mvObservers[observer];
		found= find(vec.begin(), vec.end(), inform);
		if(found == vec.end())
			m_mvObservers[observer].push_back(inform);
	}else
	{
		vec.push_back(inform);
		m_mvObservers.insert(measureMap::value_type(observer, vec));
	}
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

string portBase::switchBinStr(double value)
{
	string sRv("");

	if(((int)value & 0b10))
		sRv+= "1";
	else
		sRv+= "0";
	if(((int)value & 0b01))
		sRv+= "1";
	else
		sRv+= "0";
	return sRv;
}

void portBase::setValue(double value, const string& from)
{
//#define __moreOutput
	try{
		double invalue(value);
		double dbvalue(value);
		double oldMember(m_dValue);

		LOCK(m_VALUELOCK);
		if(!m_bDefined)// if device not found by starting in init method
			defineRange(); // try again, maybe device was found meantime
		if(m_bDefined)
		{
#ifdef __moreOutput
			cout << "-------------------------------------------------------------------" << endl;
			cout << "subroutine '" << m_sFolder << ":" << m_sSubroutine << "'";
			if(m_bSwitch)
				cout << " defined for binary switch";
			cout << endl;
			cout << "        set new value from '" << from << "'" << endl;
			cout << "            Incoming value:" << dec << value;
			if(m_bSwitch)
				cout << " binary " << switchBinStr(value);
			cout << endl;
			cout << "value before in subroutine:" << dec << m_dValue;
			if(m_bSwitch)
				cout << " binary " << switchBinStr(m_dValue);
			cout << endl;
			if(!m_bSwitch)
			{
				cout << "                 min range:" << dec << m_dMin << endl;
				cout << "                 max range:" << dec << m_dMax << endl;
			}
#endif // __moreOutput
			if(m_bSwitch)
			{
				short svalue(static_cast<short>(value));

		//		if(from.substr(0, 1) == "e")
		//			value= value != 0 ? 0b11 : 0b00;
				if(svalue & 0b01)
					svalue= 0b11;
				else if(svalue == 0b10) // only when svalue is 2 and not 6, 14, ...
					svalue= 0b10;
				else
					svalue= static_cast<short>(m_dValue) & 0b10;
				value= static_cast<double>(svalue);
				dbvalue= svalue & 0b01 ? 1 : 0;
				oldMember= ((int)m_dValue & 0b01) ? 1 : 0;

			}else
			{
				if(m_dMin < m_dMax)
				{
					if(value < m_dMin)
						value= m_dMin;
					if(value > m_dMax)
						value= m_dMax;
				}
				if(!m_bFloat)
					value= (double)((long)value);
				dbvalue= value;
				oldMember= m_dValue;
			}
#ifdef __moreOutput
			cout << "          old member value:" << dec << oldMember;
			if(m_bSwitch)
				cout << " binary " << switchBinStr(oldMember);
			cout << endl;
			cout << "         new defined value:" << dec << value;
			if(m_bSwitch)
				cout << " binary " << switchBinStr(value);
			cout << endl;
			cout << "       new defined dbvalue:" << dec << dbvalue << endl;
#endif // __moreOutput
		}
		if(value != m_dValue)
		{
			bool debug(isDebug());
			string sOwn(m_sFolder+":"+m_sSubroutine);
			ostringstream output;
			vector<string> spl;

			m_dValue= value;
			if(m_bSwitch)
			{
				for(map<string, short>::iterator it= m_mdValue.begin(); it != m_mdValue.end(); ++it)
					it->second= static_cast<short>(value);
			}

			split(spl, from, is_any_of(":"));
			LOCK(m_OBSERVERLOCK);

			if(	m_poMeasurePattern &&
				spl[0] == "i" &&
				(	spl[1] != m_sFolder ||
					(	spl[1] == m_sFolder &&
						m_nCount < m_poMeasurePattern->getActCount(spl[2])	)	)	)
			{// inform own folder to restart
			 // when setting was from other folder
			 // or in same folder, subroutine was from an later one
				m_poMeasurePattern->changedValue(m_sFolder, from.substr(2));
			}
			if(debug)
			{
				if(m_mvObservers.size() > 0)
				{
					output << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\" << endl;
					output << "  " << sOwn << " was changed to " << m_dValue << endl;
					if(	spl[0] != "i" ||
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
			if(debug && m_mvObservers.size() > 0)
			{
				bool registeredThread;

				output << "////////////////////////////////////////" << endl;
				registeredThread= Terminal::instance()->isRegistered();
				tout << output.str();
				if(!registeredThread)
					TERMINALEND;
			}
			UNLOCK(m_OBSERVERLOCK);
			if(	dbvalue != oldMember &&
				(	m_bWriteDb ||
					m_sPermission != ""	)	)
			{
#ifdef __moreOutput
			cout << "            fill new value:" << dec << dbvalue << " into database" << endl;
#endif // __moreOutput

				getRunningThread()->fillValue(m_sFolder, m_sSubroutine, "value", dbvalue);
			}

		}else if(	invalue != value &&
					(	m_bWriteDb ||
						m_sPermission != ""	)	)
		{
			// make correction of value in database
			// because client which set wrong value
			// should now that value was wrong
#ifdef __moreOutput
			cout << "             correct value:" << dec << dbvalue << " inside database to inform all clients" << endl;
#endif // __moreOutput

			getRunningThread()->fillValue(m_sFolder, m_sSubroutine, "value", dbvalue, /*update to old value*/true);
		}
#ifdef __moreOutput
		cout << "       last state of value:" << dec << m_dValue;
		if(m_bSwitch)
			cout << " binary " << switchBinStr(m_dValue);
		cout << endl;
		cout << "-------------------------------------------------------------------" << endl;
#endif // __moreOutput
		UNLOCK(m_VALUELOCK);
	}catch(SignalException& ex)
	{
		string err;
		ostringstream msg;

		msg << "set value " << value << " from " << from;
		ex.addMessage(msg.str());
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
			cerr         << "       on file " << __FILE__ << " line " << __LINE__ << endl;
		}

	}
}

bool portBase::setValue(const string& folder, const string& subroutine, double value, const string& account)
{
	string foldersub;
	IListObjectPattern* port;

	foldersub= folder + ":" + subroutine;
	port= m_oLinkWhile.getSubroutine(foldersub, /*own folder*/true);
	if(port == NULL)
	{
		ostringstream msg;

		msg << "cannot set value " << value << " into given ";
		msg << "folder:subroutine " << foldersub << " ";
		msg << "set from '" << account << "'";
		TIMELOGEX(LOG_ERROR, "setValue"+folder+":"+subroutine, msg.str(),
						getRunningThread()->getExternSendDevice());
		return false;
	}
	port->setValue(value, "i:"+account);
	return true;
}

double portBase::getValue(const string& who)
{
	short nValue;
	double dValue;
	map<string, short>::iterator found;

	LOCK(m_VALUELOCK);
#ifdef __moreOutput
		cout << "-------------------------------------------------------------------" << endl;
		cout << "subroutine '" << m_sFolder << ":" << m_sSubroutine << "'";
		if(m_bSwitch)
			cout << " defined for binary switch";
		cout << endl;
		cout << "        will be ask from '" << who << "'" << endl;
#endif // __moreOutput
	if(	m_bSwitch &&
		who.substr(0, 2) == "e:"	)
	{
		nValue= static_cast<short>(m_dValue);
		found= m_mdValue.find(who);
		if(found == m_mdValue.end())
		{
			m_mdValue[who]= nValue;
			dValue= m_dValue;

		}else
		{
			if(nValue & 0b01)
				dValue= m_dValue;
			else
			{
				dValue= static_cast<double>(found->second);
				found->second&= 0b01;
			}
		}
	}else
		dValue= m_dValue;
#ifdef __moreOutput
	cout << " return value " << dValue;
	if(m_bSwitch)
		cout << " binary " << switchBinStr(dValue);
	cout << endl;
	cout << "-------------------------------------------------------------------" << endl;
#endif // __moreOutput
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
			tout << msg.str() << endl;
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

bool portBase::getLinkedValue(const string& type, double& val, const double& maxCountDownValue/*=0*/)
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
			tout << "  __________________" << endl;
			tout << " << check link's >>>>" << endl;
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
							tout << "### WARNING: " << msg << endl;
						msg= "in folder '"+m_sFolder+"' and subroutine '"+m_sSubroutine+"'\n"+msg;
						msg+= "\n";
						TIMELOG(LOG_WARNING, m_sFolder+m_sSubroutine+"linkwhile", msg);
						bOk= false; // no linked value be used

					}else if(isdebug)
						tout << "calculation of lvhile parameter is 0, so take own value" << endl;
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
					tout << "### ERROR: " << msg << endl;
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
					tout << "### ERROR: " << msg << endl;
				bOk= false;
			}else
			{
				// define which value will be use
				// the linked value or own
				if(m_nLinkObserver != pos)
				{// subroutine link to new other subroutine -> take now this value
				 // set observer to linked subroutine
					if(isdebug)
					{
						ostringstream msg;

						msg << "link was changed from ";
						if(m_nLinkObserver)
						{
							msg << m_nLinkObserver << ". link (";
							msg << m_vpoLinks[m_nLinkObserver-1]->getStatement() << ") to ";
						}else
							msg << "own link ";
						msg << "to " << pos << ". link (" << slink << ")" << endl;
						tout << msg.str();
					}
					if(m_nLinkObserver)
						m_vpoLinks[m_nLinkObserver-1]->removeObserver( m_poMeasurePattern);
					if(pos > 0)
						link->activateObserver( m_poMeasurePattern);
					m_nLinkObserver= pos;
					//defineRange();
					val= linkvalue;
					bOk= true;

				}else if(	maxCountDownValue &&
							linkvalue != m_dLastSetValue &&
							(	linkvalue <= 0 ||
								linkvalue == maxCountDownValue	)	)
				{
					if(isdebug)
					{
						tout << "value be set from linked subroutine to";
						if(linkvalue == 0)
							tout << " end (0)";
						else
							tout << " new begin (" << linkvalue << ")";
						tout << " time" << endl;
					}
					val= linkvalue;
					bOk= true;

				}else if(	m_dLastSetValue != val &&
							(	maxCountDownValue == 0 || // <- no timer count down set
								val < linkvalue || // by count down wins the lower or max value
								val == maxCountDownValue	)	)
				{ // value is changed inside own subroutine
				  // write new value inside foreign subroutine
					port= link->getSubroutine(slink, /*own folder*/true);
					if(port)
					{
						if(isdebug)
						{
							tout << "value was changed in own subroutine," << endl;
							tout << "set foreign subroutine " << slink << " to " << dec << val << endl;
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

				}else if(	linkvalue != m_dLastSetValue &&
							(	maxCountDownValue == 0 ||
								linkvalue < val				)		)
				{// linked value from other subroutine was changed
					if(isdebug)
						tout << "take changed value " << linkvalue << " from foreign subroutine " << slink << endl;
					val= linkvalue;
					bOk= true;

				}else
				{ // nothing was changed
					if(isdebug)
						tout << "no changes be necessary" << endl;
					bOk= false;
				}
			}
		}else // else of if(slink own subroutine)
		{
			if(isdebug)
				tout << "link is showen to owen subroutine, make no changes" << endl;
			if(m_nLinkObserver != 0)
			{
				if(isdebug && bOk)
					tout << pos << ". link value '" << slink << "' link to own subroutine" << endl;
				if(	m_nLinkObserver-1 < links	)
				{
					m_vpoLinks[m_nLinkObserver-1]->removeObserver( m_poMeasurePattern);
					m_nLinkObserver= 0;
					//defineRange();
				}else
				{
					cerr << "### ERROR: in " << m_sFolder << ":" << m_sSubroutine;
					cerr << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
				}
			}
			bOk= false;
		} // end else if(slink own subroutine)

		if(isdebug)
		{
			tout << " << end of check >>>>" << endl;
			tout << "   --------------" << endl;
		}
	}else // if(links > 0)
		bOk= false;

	m_dLastSetValue= val;
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

string portBase::getFolderRunningID()
{
	folderSpecNeed_t res;
	vector<string> specs;
	string sRv;
	SHAREDPTR::shared_ptr<measurefolder_t> pCurrent;

	specs= getRunningThread()->getAllSpecs();
	if(specs.size() == 0)
		return string("");
	if(m_vpFolderSpecs.size() == 0)
	{
		pCurrent= m_pFolders;
		while(pCurrent)
		{
			if(	pCurrent->runThread &&
				pCurrent->bCorrect		)
			{
				if(getFolderName() == pCurrent->runThread->getFolderName())
				{// own folder is always running by question
					res.needRun= true;
					res.isRun= true;
				}else
					res= pCurrent->runThread->isFolderRunning(specs);
				if(res.needRun)
				{
					m_vpFolderSpecs.push_back(pCurrent->runThread);
					if(res.isRun)
						sRv+= "1";
					else
						sRv+= "0";
				}
			}
			pCurrent= pCurrent->next;
		}
	}else
	{
		typedef vector<SHAREDPTR::shared_ptr<IMeasurePattern> >::iterator thIt;

		for(thIt it= m_vpFolderSpecs.begin(); it != m_vpFolderSpecs.end(); ++it)
		{
			if(getFolderName() == (*it)->getFolderName())
			{// own folder is always running by question
				res.needRun= true;
				res.isRun= true;
			}else
				res= (*it)->isFolderRunning(specs);
			if(res.isRun)
				sRv+= "1";
			else
				sRv+= "0";
		}
	}
	return sRv;
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
