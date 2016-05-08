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
#include "ExternPort.h"

#include "../util/debugsubroutines.h"
#include "../util/GlobalStaticMethods.h"
#include "../util/exception.h"
#include "../util/thread/Thread.h"
#include "../util/thread/Terminal.h"
#include "../util/stream/ErrorHandling.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

using namespace util;
using namespace ports;
using namespace ppi_database;
using namespace boost;

portBase::portBase(const string& type, const string& folderName,
					const string& subroutineName, unsigned short objectID	) :
#ifdef __followSETbehaviorToFolder
  m_oToFolderExp(__followSETbehaviorToFolder),
  m_oToSubExp(__followSETbehaviorToSubroutine),
#endif
  m_poMeasurePattern(NULL),
  m_oLinkWhile(folderName, subroutineName, "lwhile", false, false, this),
  m_dLastSetValue(0),
  m_nLinkObserver(0)
{
	m_nCount= 0;
	m_bDefined= false;
	m_bFloat= true;
	m_dMin= 1;
	m_dMax= 0;
	m_bBinary= false;
	m_bConfigure= true;
	m_bDebug= false;
	m_sType= type;
	m_sFolder= folderName;
	m_sSubroutine= subroutineName;
	m_nObjFolderID= objectID;
	m_bWriteDb= false;
	m_dValue.value= 0;
	m_dOldVar= 0;
	m_bChanged= false;
	m_bOutsideChanged= false;
	// do not know access from database
	m_pbCorrectDevice= auto_ptr<bool>();
	m_THREADLOCKMUTEX= Thread::getMutex("THREADLOCKMUTEX");
	m_VALUEOBJECTLOCK= Thread::getMutex("VALUEOBJECTLOCK");
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
#ifdef __followSETbehaviorToFolder
	if(	(	string(__followSETbehaviorToFolder) == "" ||
			boost::regex_match(m_sFolder, m_oToFolderExp)	) &&
		(	string(__followSETbehaviorToSubroutine) == "" ||
			boost::regex_match(m_sSubroutine, m_oToSubExp)		)	)
	{
		m_bFollow= true;
	}else
		m_bFollow= false;
#endif // __followSETbehaviorToFolder

	//Debug info to stop by right subroutine
/*	if(	getFolderName() == "Raff1_Zeit" &&
		getSubroutineName() == "schliessen"					)
	{
		cout << getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	m_dValue.value= 0;
	m_pFolders= pStartFolder;
	m_bInfo= !properties->haveAction("noinfo");
	m_bTime= !properties->haveAction("notime");
	m_sPermission= properties->getValue("perm", /*warning*/false);
	m_bWriteDb= properties->haveAction("db");
	m_bBinary= properties->haveAction("binary");
	m_nCount= getRunningThread()->getActCount();
	dDef= properties->getDouble(prop, /*warning*/false);
	if(prop != "#ERROR")
	{
		m_dValue.value= dDef;
		m_dOldVar= dDef;
	}
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
		db->fillValue(m_sFolder, m_sSubroutine, "value", m_dValue.value);

	}else
		m_dValue.value= ddv;
	m_sErrorHead= properties->getMsgHead(/*error message*/true);
	m_sWarningHead= properties->getMsgHead(/*error message*/false);
	m_sMsgHead= properties->getMsgHead();// without error identification

	defineRange();
	registerSubroutine();
	return true;
}

string portBase::checkStartPossibility()
{
	ostringstream sRv;

	sRv << "subroutine " << getFolderName() << ":" << getSubroutineName() << " from type " << getType() << endl;
	sRv << "is not designed to start any action per time";
	return sRv.str();
}

bool portBase::startingBy(const ppi_time& tm, const InformObject& from)
{
	return false;
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
		if(	m_bBinary &&
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
			if(	m_bBinary &&
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
				m_bBinary= false;
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
	m_oLinkWhile.activateObserver(observer);
}

void portBase::informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
{
	string inform(folder+":"+subroutine+" "+parameter);
	vector<string>::iterator found;
	IInformerCachePattern::memObserverVector::iterator fObs;
	SHAREDPTR::shared_ptr<IInformerCachePattern> pInformCache;

	try{
		if(	folder == getFolderName() &&
			(	subroutine == getSubroutineName() ||
				m_nCount < observer->getActCount(subroutine)	)	)
		{// do not inform other subroutine from same folder list, which running after own (higher count)
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
	pInformCache= observer->getInformerCache(m_sFolder);
	fObs= m_mvObservers.end();
	for(IInformerCachePattern::memObserverVector::iterator it= m_mvObservers.begin();
					it != m_mvObservers.end(); ++it		)
	{
		if(it->first.get() == pInformCache.get())
		{
			fObs= it;
			break;
		}
	}
	if(fObs != m_mvObservers.end())
	{
		found= find(fObs->second.begin(), fObs->second.end(), inform);
		if(found == fObs->second.end())
			fObs->second.push_back(inform);
	}else
	{
		vector<string> vec;

		vec.push_back(inform);
		if(folder == getFolderName())
			m_mvObservers.insert(m_mvObservers.begin(), IInformerCachePattern::memObserverPair(pInformCache, vec));
		else
			m_mvObservers.push_back(IInformerCachePattern::memObserverPair(pInformCache, vec));
	}
	UNLOCK(m_OBSERVERLOCK);
}

string portBase::getObserversString() const
{
	ostringstream oRv;

	LOCK(m_OBSERVERLOCK);
	if(m_mvObservers.empty())
	{
		UNLOCK(m_OBSERVERLOCK);
		return "";
	}
	oRv << "for folder " << m_sFolder << " in subroutine " << m_sSubroutine << endl;
	for(IInformerCachePattern::memObserverVector::const_iterator it= m_mvObservers.begin();
								it != m_mvObservers.end(); ++it		)
	{
		oRv << "     define observer " << it->first->getFolderName() << endl;
		for(vector<string>::const_iterator vit= it->second.begin();
								vit != it->second.end(); ++vit		)
		{
			oRv << "                               " << *vit << endl;
		}
	}
	UNLOCK(m_OBSERVERLOCK);
	oRv << endl;
	return oRv.str();
}

void portBase::removeObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter)
{
	bool bEraseVector(false);
	string remove(folder+":"+subroutine+" "+parameter);
	IInformerCachePattern::memObserverVector::iterator foundT;
	vector<string>::iterator foundS;
	SHAREDPTR::shared_ptr<IInformerCachePattern> pInformCache;

	if(observer == NULL)
		return;
	pInformCache= observer->getUsedInformerCache(m_sFolder);
	if(pInformCache == NULL)
		return;
	LOCK(m_OBSERVERLOCK);
	foundT= m_mvObservers.end();
	for(IInformerCachePattern::memObserverVector::iterator it= m_mvObservers.begin();
					it != m_mvObservers.end(); ++it		)
	{
		if(it->first.get() == pInformCache.get())
		{
			foundT= it;
			break;
		}
	}
	//foundT= m_mvObservers.find(pInformCache);
	if(foundT != m_mvObservers.end())
	{
		foundS= find(foundT->second.begin(), foundT->second.end(), remove);
		if(foundS != foundT->second.end())
		{
			foundT->second.erase(foundS);
			if(foundT->second.size() == 0)
			{
				m_mvObservers.erase(foundT);
				bEraseVector= true;
			}
			observer->removeObserverCache(m_sFolder);
		}
		if(	bEraseVector == false &&
			foundT->second.empty()	)
		{
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

bool portBase::hasSubVar(const string& subvar) const
{
	if(	subvar == "changed" ||
		subvar == "lastvalue" ||
		subvar == "value"		)
	{
		return true;
	}
	return false;
}

void portBase::setChangedSubVar(SHAREDPTR::shared_ptr<IListObjectPattern> subVarObj)
{
	vector<SHAREDPTR::shared_ptr<IListObjectPattern> >::iterator it;
	string byFolder(subVarObj->getFolderName());
	string bySubroutine(subVarObj->getSubroutineName());

	for(it= m_voChangedSubVars.begin();
					it != m_voChangedSubVars.end(); ++it						)
	{
		if(	(*it)->getFolderName() == byFolder &&
			(*it)->getSubroutineName() == bySubroutine	)
		{
			/*
			 * SubroutineSubVarHolder object
			 * was before defined
			 * maybe by different parameters
			 * from subroutine
			 */
			return;
		}
	}
	m_voChangedSubVars.push_back(subVarObj);
}

void portBase::actualizeChangedSubVars()
{
	vector<SHAREDPTR::shared_ptr<IListObjectPattern> >::iterator it;
	auto_ptr<InformObject> informer;

	for(it= m_voChangedSubVars.begin();
					it != m_voChangedSubVars.end(); ++it						)
	{
		if(informer.get() == NULL)
		{
			/*
			 * create object of informer
			 * only when need
			 * for better performance
			 */
			informer= auto_ptr<InformObject>(new InformObject(InformObject::INTERNAL,
							m_sFolder + ":" + m_sSubroutine));
		}
		(*it)->getValue(*informer.get());
	}
}

bool portBase::lockObject(const string& file, int line, const InformObject& who)
{
	/**
	 * whether object will be
	 * currently locked
	 */
	bool bRv(false);
	/**
	 * own ID of thread
	 */
	pid_t threadID(Thread::gettid());

	LOCK(m_THREADLOCKMUTEX);
	if(m_nLockThread != threadID)
	{
		UNLOCK(m_THREADLOCKMUTEX);
		Thread::mutex_lock(file, line, m_VALUEOBJECTLOCK);
		bRv= true;
		LOCK(m_THREADLOCKMUTEX);
		m_nLockThread= threadID;
	}
	UNLOCK(m_THREADLOCKMUTEX);
	return bRv;
}

inline void portBase::unlockObject(bool locked, const string& file, int line)
{
	if(!locked)
		return;
	LOCK(m_THREADLOCKMUTEX);
	m_nLockThread= 0;
	Thread::mutex_unlock(file, line, m_VALUEOBJECTLOCK);
	UNLOCK(m_THREADLOCKMUTEX);

}

ppi_value portBase::getSubVar(const InformObject& who, const string& subvar) const
{
	short used(0);
	ppi_value dRv(0);
	ppi_time changed;
	map<InformObject, ppi_time>::iterator foundChange;
	map<InformObject, ppi_value>::iterator foundOldVar;

	if(subvar == "changed")
		used= 1;
	else if(subvar == "lastvalue")
		used= 2;
	else if(subvar == "value")
		used= 3;
	if(used)
	{
		LOCK(m_VALUELOCK);
		switch(used)
		{
		case 1:
			changed= m_dValue.getTime();
			foundChange= m_motChangeingQuestion.find(who);
			if(foundChange != m_motChangeingQuestion.end())
			{
				/*
				 * ask for different time (!=)
				 * and not whether subroutine time
				 * will be greater (>)
				 * because last change time of subroutine
				 * can be lower than a changing before
				 * when subroutine was changed
				 * from an subroutine with type SET
				 * which had the action 'fromtime'
				 * and where this 'from' properties
				 * had an older changing times
				 */
				if(changed != foundChange->second)
				{
					dRv= 1;
					foundChange->second= changed;
				}else
					dRv= 0;
			}else
			{
				m_motChangeingQuestion[who]= changed;
				if(changed.isSet())
					dRv= 1;
				else
					dRv= 0;
			}
			break;
		case 2:
			dRv= m_dOldVar;
			break;
		case 3:
			dRv= m_dValue.value;
			break;
		default:
			dRv= 0;
			break;
		}
		UNLOCK(m_VALUELOCK);
	}
	return dRv;
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

void portBase::noChange()
{
	bool locked;

	locked= LOCKOBJECT(InformObject(InformObject::INTERNAL, m_sFolder+":"+m_sSubroutine));
	if(m_bOutsideChanged)
	{// value was changed from outside
	 // and now by this pass nothing changed
	 // but for other asking subroutines
	 // value should be defined as changed
		m_bOutsideChanged= false;
		m_bChanged= true;
	}else
		m_bChanged= false;
	UNLOCKOBJECT(locked);
}

void portBase::setValue(const IValueHolderPattern& value, const InformObject& from)
{
	bool locked;
	bool bValLocked(false);
//#define __moreOutput
#ifdef __moreOutput
	bool debug(isDebug());
#endif //__moreOutput

	locked= LOCKOBJECT(from);
	try{
		vector<string> from_spl;
		string fromWhere(from.getWhoDescription());
		InformObject::posPlace_e place(from.getDirection());
		ppi_value dValue(value.getValue());
		ppi_value invalue(value.getValue());
		ppi_value dbvalue(value.getValue());
		ppi_value oldMember;
		ppi_time changedTime(value.getTime());

		// debug stopping
	/*	if(	getFolderName() == "kalibrierung1" &&
			getSubroutineName() == "is_calctime_grad"		)
		{
			cout << "stopping inside method setValue from subroutine"
							<< getFolderName() << ":" << getSubroutineName() << endl;
			cout << __FILE__ << __LINE__ << endl;
		}*/

		LOCK(m_VALUELOCK);
		bValLocked= true;
		oldMember= m_dValue.value;
		split(from_spl, fromWhere, is_any_of(":"));
		if(	m_bOutsideChanged &&
			from_spl.size() == 2 &&
			place == InformObject::INTERNAL &&
			from_spl[0] == m_sFolder &&
			from_spl[1] == m_sSubroutine					)
		{
			m_bOutsideChanged= false;
		}
#ifdef __followSETbehaviorToFolder
		if(	m_bFollow &&
			__followSETbehaviorFrom <= 2 &&
			__followSETbehaviorTo >= 2		)
		{
			cout << "[2] folder " << m_sFolder << ":" << m_sSubroutine << " will be set to " << invalue
							<< " where old was " << m_dValue.value << endl;
		}
#endif // __followSETbehaviorToFolder

		if(!m_bDefined)// if device not found by starting in init method
			defineRange(); // try again, maybe device was found meantime
		if(m_bDefined)
		{
#ifdef __moreOutput
			if(debug)
			{
				out() << "-------------------------------------------------------------------" << endl;
				out() << "subroutine '" << m_sFolder << ":" << m_sSubroutine << "'";
				if(m_bBinary)
					out() << " defined for binary switch";
				out() << endl;
				out() << "        set new value from " << from.toString() << endl;
				out() << "            Incoming value:" << dec << value.getValue();
				if(m_bBinary)
					out() << " binary " << switchBinStr(value.getValue());
				out() << endl;
				out() << "value before in subroutine:" << dec << m_dValue.value;
				if(m_bBinary)
					out() << " binary " << switchBinStr(m_dValue.value);
				out() << endl;
				if(!m_bBinary)
				{
					out() << "                 min range:" << dec << m_dMin << endl;
					out() << "                 max range:" << dec << m_dMax << endl;
				}
			}
#endif // __moreOutput
			if(m_bBinary)
			{
				short svalue(static_cast<short>(dValue));

		//		if(from.substr(0, 1) == "e")
		//			dValue= dValue != 0 ? 0b11 : 0b00;
				if(svalue & 0b01)
					svalue= 0b11;
				else if(svalue == 0b10) // only when svalue is 2 and not 6, 14, ...
				{
					if(static_cast<short>(m_dValue.value) & 0b01)
						svalue= 0b00;
					else
						svalue= 0b10;
				}else
					svalue= 0b00;
				dValue= static_cast<double>(svalue);
				dbvalue= svalue & 0b01 ? 1 : 0;
				oldMember= (static_cast<short>(m_dValue.value) & 0b01) ? 1 : 0;

			}else
			{
				if(m_dMin < m_dMax)
				{
					if(dValue < m_dMin)
						dValue= m_dMin;
					if(dValue > m_dMax)
						dValue= m_dMax;
				}
				if(!m_bFloat)
					dValue= (double)((long)dValue);
				dbvalue= dValue;
				oldMember= m_dValue.value;
			}
#ifdef __moreOutput
			if(debug)
			{
				out() << "          old member value:" << dec << oldMember;
				if(m_bBinary)
					out() << " binary " << switchBinStr(oldMember);
				out() << endl;
				out() << "         new defined value:" << dec << dValue;
				if(m_bBinary)
					out() << " binary " << switchBinStr(dValue);
				out() << endl;
				out() << "       new defined dbvalue:" << dec << dbvalue << endl;
			}
#endif // __moreOutput
		}
		if(dValue != m_dValue.value)
		{
			bool debug(isDebug());
			ostringstream output;

			if(	from_spl.size() != 2 ||
				place != InformObject::INTERNAL ||
				from_spl[0] != m_sFolder ||
				from_spl[1] != m_sSubroutine					)
			{// value was changed not from own subroutine,
			 // so remember for next pass of subroutine
			 // because then nothing will be changed, but flag m_bChanged should be set
				m_bOutsideChanged= true;
			}
			if(	m_bTime &&
				!changedTime.isSet())
			{
				if(gettimeofday(&changedTime, NULL))
				{
					string msg("ERROR: cannot get time of day,\n");

					msg+= "       so cannot measure time for TIMER function in folder ";
					msg+= getFolderName() + " for changing value in subroutine " + getSubroutineName();
					TIMELOG(LOG_ALERT, "changedValueMeasureThread", msg);
					changedTime.clear();
				}
			}
			m_dOldVar= m_dValue.value;
			m_dValue.value= dValue;
			m_bChanged= true;
			if(m_bTime)
				m_dValue.lastChanging= changedTime;

			// unlock VALUELOCK before informFolder()
			// because when folder has an inform parameter (inside measure.conf)
			// maybe inform content want to get again own value
			// and by method getValue() will be lock this mutex again
			UNLOCK(m_VALUELOCK);
			bValLocked= false;

			LOCK(m_OBSERVERLOCK);
			if(	m_mvObservers.size() ||
				place != InformObject::INTERNAL ||
				from_spl[0] != m_sFolder ||
				m_nCount < m_poMeasurePattern->getActCount(from_spl[1])	)
			{
				UNLOCK(m_OBSERVERLOCK);
#ifdef __followSETbehaviorToFolder
				if(	m_bFollow &&
					__followSETbehaviorFrom <= 3 &&
					__followSETbehaviorTo >= 3		)
				{
					cout << "[3] " << m_sFolder << ":" << m_sSubroutine
									<< " was changed from " << from.toString() << endl;
				}
#endif // __followSETbehaviorToFolder
				getRunningThread()->informFolders(m_mvObservers, from,
								m_sSubroutine, debug, m_OBSERVERLOCK);
			}else
				UNLOCK(m_OBSERVERLOCK);
			if(	dbvalue != oldMember &&
				(	m_bWriteDb ||
					m_sPermission != ""	)	)
			{
#ifdef __moreOutput
				if(debug)
					out() << "            fill new value:" << dec << dbvalue << " into database" << endl;
#endif // __moreOutput

				getRunningThread()->fillValue(m_sFolder, m_sSubroutine, "value", dbvalue);
			}

		}else if(	invalue != dValue &&
					(	m_bWriteDb ||
						m_sPermission != ""	)	)
		{
			m_bChanged= false;
			// make correction of dValue in database
			// because client which set wrong dValue
			// should now that dValue was wrong
#ifdef __moreOutput
			if(debug)
				out() << "             correct value:" << dec << dbvalue
						<< " inside database to inform all clients" << endl;
#endif // __moreOutput

			getRunningThread()->fillValue(m_sFolder, m_sSubroutine, "value",
							dbvalue, /*update to old dValue*/true);
		}else
		{
			m_bChanged= false;
		}
#ifdef __moreOutput
		if(debug)
		{
			out() << "       last state of value:" << dec << dValue;
			if(m_bBinary)
				out() << " binary " << switchBinStr(dValue);
			out() << endl;
			out() << "-------------------------------------------------------------------" << endl;
		}
#endif // __moreOutput
	}catch(SignalException& ex)
	{
		string err;
		ostringstream msg;

		msg << "set value " << value.getValue() << " from " << from.toString();
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
	if(bValLocked)
		UNLOCK(m_VALUELOCK);
	// debug stopping
/*	if(	getFolderName() == "power_switch" &&
		getSubroutineName() == "port2set"		)
	{
		cout << "stopping inside method setValue from subroutine"
						<< getFolderName() << ":" << getSubroutineName() << endl;
		cout << "value be set to " << m_dValue.value << " on "
					<< m_dValue.lastChanging.toString(true) << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	UNLOCKOBJECT(locked);
}

bool portBase::setValue(const string& folder, const string& subroutine,
				const IValueHolderPattern& value, const InformObject& account)
{
	ostringstream msg;
	SHAREDPTR::shared_ptr<measurefolder_t> pFolder= m_pFolders;

	while(pFolder && pFolder->name != folder)
		pFolder= pFolder->next;
	if(pFolder)
	{
		for(vector<sub>::iterator it= pFolder->subroutines.begin(); it != pFolder->subroutines.end(); ++it)
		{
			if(	it->bCorrect &&
				it->name == subroutine	)
			{
#ifdef __followSETbehaviorToFolder
				if(	__followSETbehaviorFrom <= 1 &&
					__followSETbehaviorTo >= 1 &&
					(	string(__followSETbehaviorToFolder) == "" ||
						boost::regex_match(folder, m_oToFolderExp)		) &&
					(	string(__followSETbehaviorToSubroutine) == "" ||
						boost::regex_match(subroutine, m_oToSubExp)		)	)
				{
					cout << "[1] write " << folder << ":" << subroutine << " into sub object" << endl;
				}
#endif // __followSETbehaviorToFolder

				it->portClass->setValue(value, account);
				return true;
			}
		}
	}
	msg << "cannot set value " << value.getValue() << " into given '";
	msg << folder << ":" << subroutine << "'" << endl;
	msg << "because ";
	if(pFolder == NULL)
		msg << "folder";
	else
		msg << "subroutine";
	msg << " can not be found" << endl;
	msg << "set from " << account.toString();
	TIMELOGEX(LOG_ERROR, "setValue"+folder+":"+subroutine, msg.str(),
					getRunningThread()->getExternSendDevice());
	cerr << msg << endl;
	return false;
}

auto_ptr<IValueHolderPattern> portBase::getValue(const InformObject& who)
{
//#undef __moreOutput
	short nValue, sValue;
	map<InformObject, pair<short, ppi_time> >::iterator found;
	auto_ptr<IValueHolderPattern> oRv;

	oRv= auto_ptr<IValueHolderPattern>(new ValueHolder());
	LOCK(m_VALUELOCK);
	// debug stopping
/*	if(	getFolderName() == "kalibrierung1" &&
		getSubroutineName() == "is_calctime_grad"		)
	{
		cout << "stopping inside method setValue from subroutine"
						<< getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
#ifdef __moreOutput
	bool debug(isDebug());

	if(debug)
	{
		out() << "-------------------------------------------------------------------" << endl;
		out() << "subroutine '" << m_sFolder << ":" << m_sSubroutine << "'";
		if(m_bBinary)
			out() << " defined for binary switch";
		out() << endl;
		out() << "        will be ask from '" << who << "'" << endl;
	}
#endif // __moreOutput

	oRv->setTimeValue(m_dValue);
	if(	m_bBinary &&
		who.getDirection() != InformObject::INTERNAL	)
	{
		ppi_time curT;

		curT.setActTime();
		nValue= static_cast<short>(m_dValue.value);
		found= m_mdtLastValue.find(who);
		if(found == m_mdtLastValue.end())
		{
			if(nValue & 0b01)
			{
				m_mdtLastValue[who]= pair<short, ppi_time>(0b01, curT);
				oRv->setValue(0b01);
			}else
				m_mdtLastValue[who]= pair<short, ppi_time>(0b00, curT);

		}else
		{
			if(nValue & 0b01)
			{
				if(found->second.first & 0b01)
					sValue= 0b11;
				else
					sValue= 0b01;
			}else
			{
				if(	(found->second.first | 0b00) == 0b00 &&
					found->second.second < m_dValue.lastChanging	)
				{
					sValue= 0b10;
				}else
					sValue= 0b00;
			}
			oRv->setValue(static_cast<ppi_value>(sValue));
			found->second= pair<short, ppi_time>(sValue, curT);
		}
	}
#ifdef __moreOutput
	if(debug)
	{
		out() << " return value " << m_oGetValue.value;
		if(m_bBinary)
			cout << " binary " << switchBinStr(m_oGetValue.value);
		out() << endl;
		out() << "-------------------------------------------------------------------" << endl;
	}
#endif // __moreOutput
	UNLOCK(m_VALUELOCK);
	return oRv;
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
			m_vpoLinks.push_back(new ListCalculator(m_sFolder, m_sSubroutine, lk.str(), true, false, this));
			calc= m_vpoLinks.back();
			if(!calc->init(pStartFolder, sValue))
				bOk= false;

		}else
		{
			ostringstream msg;

			msg << properties->getMsgHead(/*error*/true);
			msg << i << ". link parameter '"  << sValue << "' can only be an single [folder:]<sburoutine>, so do not set this link";
			LOG(LOG_ERROR, msg.str());
			out() << msg.str() << endl;
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

bool portBase::getLinkedValue(const string& type, auto_ptr<IValueHolderPattern>& val, const double& maxCountDownValue/*=0*/)
{
	bool bOk, isdebug;
	double lvalue, linkvalue;
	string slink, foldersub;
	vector<string>::size_type pos;
	vector<ListCalculator*>::size_type links(m_vpoLinks.size());
	ListCalculator* link;
	SHAREDPTR::shared_ptr<IListObjectPattern> port;

	if(m_sType != type)
		return false;
	if(links > 0)
	{
		isdebug= isDebug();
		if(isdebug)
		{
			out() << "  __________________" << endl;
			out() << " << check link's >>>>" << endl;
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
							out() << "### WARNING: " << msg << endl;
						msg= "in folder '"+m_sFolder+"' and subroutine '"+m_sSubroutine+"'\n"+msg;
						msg+= "\n";
						TIMELOG(LOG_WARNING, m_sFolder+m_sSubroutine+"linkwhile", msg);
						bOk= false; // no linked value be used

					}else if(isdebug)
						out() << "calculation of lvhile parameter is 0, so take own value" << endl;
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
					out() << "### ERROR: " << msg << endl;
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
					out() << "### ERROR: " << msg << endl;
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
						out() << msg.str();
					}
					if(m_nLinkObserver)
						m_vpoLinks[m_nLinkObserver-1]->removeObserver( m_poMeasurePattern);
					if(pos > 0)
						link->activateObserver( m_poMeasurePattern);
					m_nLinkObserver= pos;
					//defineRange();
					val->setValue(linkvalue);
					val->setTime(link->getLastChanging());
					bOk= true;

				}else if(	maxCountDownValue &&
							linkvalue != m_dLastSetValue &&
							(	linkvalue <= 0 ||
								linkvalue == maxCountDownValue	)	)
				{
					if(isdebug)
					{
						out() << "value be set from linked subroutine to";
						if(linkvalue == 0)
							out() << " end (0)";
						else
							out() << " new begin (" << linkvalue << ")";
						out() << " time" << endl;
					}
					val->setValue(linkvalue);
					val->setTime(link->getLastChanging());
					bOk= true;

				}else if(	m_dLastSetValue != val->getValue() &&
							(	maxCountDownValue == 0 || // <- no timer count down set
								val->getValue() < linkvalue || // by count down wins the lower or max value
								val->getValue() == maxCountDownValue	)	)
				{ // value is changed inside own subroutine
				  // write new value inside foreign subroutine
					port= link->getSubroutine(&slink, getObjectFolderID(), /*own folder*/true);
					if(port != NULL)
					{
						InformObject from(InformObject::INTERNAL, foldersub);
						SHAREDPTR::shared_ptr<IInformerCachePattern> cache;

						if(isdebug)
						{
							out() << "value was changed in own subroutine," << endl;
							out() << "set foreign subroutine " << slink << " to " << dec << val->getValue() << endl;
						}
						port->setValue(*val.get(), from);
						cache= port->getRunningThread()->getInformerCache(m_sFolder);
						cache->changedValue(slink, from);
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
								linkvalue < val->getValue()			)		)
				{// linked value from other subroutine was changed
					if(isdebug)
						out() << "take changed value " << linkvalue << " from foreign subroutine " << slink << endl;
					val->setValue(linkvalue);
					val->setTime(link->getLastChanging());
					bOk= true;

				}else
				{ // nothing was changed
					if(isdebug)
						out() << "no changes be necessary" << endl;
					bOk= false;
				}
			}
		}else // else of if(slink own subroutine)
		{
			if(isdebug)
				out() << "link is showen to owen subroutine, make no changes" << endl;
			if(m_nLinkObserver != 0)
			{
				if(isdebug && bOk)
					out() << pos << ". link value '" << slink << "' link to own subroutine" << endl;
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
			out() << " << end of check >>>>" << endl;
			out() << "   --------------" << endl;
		}
	}else // if(links > 0)
		bOk= false;

	m_dLastSetValue= val->getValue();
	if(	!bOk ||
		slink == m_sSubroutine ||
		slink == foldersub			)
	{
		return false;
	}
	return true;
}

string portBase::getFolderName() const
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

string portBase::getSubroutineName() const
{
	return m_sSubroutine;
}

void portBase::endOfConfigure()
{
	LOCK(m_DEBUG);
	m_bConfigure= false;
	UNLOCK(m_DEBUG);
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
	string var;

	if(m_nLinkObserver)
	{
		SHAREDPTR::shared_ptr<IListObjectPattern> port;

		var= m_vpoLinks[m_nLinkObserver-1]->getStatement();
		port= m_oLinkWhile.getSubroutine(&var, getObjectFolderID(), /*own folder*/true);
		if(	port != NULL &&
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
	return m_bBinary;
}

ostringstream& portBase::out()
{
#ifdef __WRITEDEBUGALLLINES
#if ( __DEBUGSESSIONOutput == debugsession_SERVER || __DEBUGSESSIONOutput == debugsession_BOTH)
	string output(m_sStreamObj.str());

	if(output != "")
	{
		tout << output;
		tout << flush;
#if (__DEBUGSESSIONOutput == debugsession_SERVER)
		m_sStreamObj.str("");
#endif
	}
#endif
#endif // __WRITEDEBUGALLLINES
	return m_sStreamObj;
}

void portBase::writeDebugStream()
{
	bool bWriteTerminal(false);
	bool bWriteOverClient(false);

	if(m_sStreamObj.eof())
		return;
	try{
#ifndef __WRITEDEBUGALLLINES
#if ( __DEBUGSESSIONOutput == debugsession_SERVER || __DEBUGSESSIONOutput == debugsession_BOTH)
		bWriteTerminal= true;
		string output(m_sStreamObj.str());

		if(output.length() > 0)
		{
			tout << output;
			tout << flush;
		}
#if (__DEBUGSESSIONOutput == debugsession_SERVER)
		m_sStreamObj.str("");
#endif
#endif
#endif // __WRITEDEBUGALLLINES

#if (__DEBUGSESSIONOutput == debugsession_CLIENT || __DEBUGSESSIONOutput == debugsession_BOTH)
		bWriteOverClient= true;
#endif

		LOCK(m_DEBUG);
		if(	m_bConfigure ||
			getRunningThread() == NULL	)
		{
			bWriteTerminal= true;
			bWriteOverClient= false;
		}
		UNLOCK(m_DEBUG);

		if(bWriteTerminal == true)
		{
#ifndef __WRITEDEBUGALLLINES
			string output(m_sStreamObj.str());

			if(output.length() > 0)
			{
				tout << output;
				tout << flush;
			}
			m_sStreamObj.str("");
#endif
		}
		if(bWriteOverClient == true)
		{
			ppi_value value;

			LOCK(m_VALUELOCK);
			value= m_dValue.value;
			UNLOCK(m_VALUELOCK);
			fillDebugSession(m_sFolder, m_sSubroutine, value, m_sStreamObj.str());
			m_sStreamObj.str("");
		}
	}catch(SignalException& ex)
	{
		string err;

		ex.addMessage("try to write debug session info for '" + m_sFolder + ":" + m_sSubroutine + "'");
		err= ex.getTraceString();
		cerr << endl << err << endl;
		try{
			LOG(LOG_ALERT, err);
		}catch(...)
		{
			cerr << endl << "ERROR: catch exception by trying to log error message" << endl;
		}

	}
}

void portBase::fillDebugSession(const string& folder, const string& subroutine,
				const ppi_value& value, const string& contentStr)
{
	IDbgSessionPattern::dbgSubroutineContent_t content;

	/**
	 * place of new definition of content are:
	 * DbInterface::fillDebugSession
	 * ProcessChecker::execute by method == "debugSubroutine"
	 * Informer::informing
	 * MeasureThread::setDebug on method end
	 * MeasureThread::execute by 2 times
	 * MeasureThread::doDebugStartingOutput
	 * MeasureThread::checkToStart
	 * portBase::writeDebugStream
	 * ServerDbTransaction::transfer by method == "fillDebugSession"
	 */
	content.folder= folder;
	content.subroutine= subroutine;
	content.value= value;
	try{
		ppi_time cur;

		cur.setActTime();
		if(cur.error())
		{
			string err("cannot create current time for debugging session\n");

			err+= cur.errorStr();
			cout << glob::addPrefix("### WARNING: ", err) << endl;
			TIMELOG(LOG_WARNING, "debugsessiontime", err);
		}
		content.currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time(cur));

	}catch(SignalException& ex)
	{
		ex.addMessage("create shared pointer for ppi_time");
		throw ex;
	}
	content.content= contentStr;
	getRunningThread()->fillDebugSession(content);
}

portBase::~portBase()
{
	vector<ListCalculator*>::iterator it;

	DESTROYMUTEX(m_THREADLOCKMUTEX);
	DESTROYMUTEX(m_VALUEOBJECTLOCK);
	DESTROYMUTEX(m_VALUELOCK);
	DESTROYMUTEX(m_DEBUG);
	DESTROYMUTEX(m_CORRECTDEVICEACCESS);
	for(it= m_vpoLinks.begin(); it != m_vpoLinks.end(); ++it)
		delete *it;
}
