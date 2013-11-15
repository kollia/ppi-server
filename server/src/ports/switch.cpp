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

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "switch.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../util/thread/Terminal.h"
#include "../util/CalculatorContainer.h"

using namespace util;
using namespace boost;

switchClass::switchClass(string folderName, string subroutineName)
: portBase("SWITCH", folderName, subroutineName),
  m_oBegin(folderName, subroutineName, "begin", false, true),
  m_oWhile(folderName, subroutineName, "while", false, true),
  m_oEnd(folderName, subroutineName, "end", false, true),
  m_bSwitch(true),
  m_bLastValue(false),
  m_bCurrent(false),
  m_bAlwaysBegin(false)
{
	m_VALUELOCK= Thread::getMutex("VALUELOCK");
}

switchClass::switchClass(string type, string folderName, string subroutineName)
: portBase(type, folderName, subroutineName),
  m_oBegin(folderName, subroutineName, "begin", false, true),
  m_oWhile(folderName, subroutineName, "while", false, true),
  m_oEnd(folderName, subroutineName, "end", false, true),
  m_bLastValue(false),
  m_bCurrent(false),
  m_bAlwaysBegin(false)
{
	if(type != "SWITCH")
		m_bSwitch= false;
	m_VALUELOCK= Thread::getMutex("VALUELOCK");
}

bool switchClass::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const bool bAlwaysBegin)
{
	bool bOk= true;
	string on, sWhile, off;

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "switch_test_begin_end" &&
		getSubroutineName() == "begin_param"					)
	{
		cout << getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	m_bAlwaysBegin= bAlwaysBegin;
	on= properties->getValue("begin", /*warning*/false);
	sWhile= properties->getValue("while", /*warning*/false);
	off= properties->getValue("end", /*warning*/false);
	if(!m_oBegin.init(pStartFolder, on))
		bOk= false;
	if(!m_oWhile.init(pStartFolder, sWhile))
		bOk= false;
	if(!m_oEnd.init(pStartFolder, off))
		bOk= false;
	if(m_bSwitch)
		m_bCurrent= properties->haveAction("current");
	if(!initLinks("SWITCH", properties, pStartFolder))
		bOk= false;
	if(!portBase::init(properties, pStartFolder))
		bOk= false;
	return bOk;
}

void switchClass::setObserver(IMeasurePattern* observer)
{
	m_oBegin.activateObserver(observer);
	m_oWhile.activateObserver(observer);
	m_oEnd.activateObserver(observer);
	portBase::setObserver(observer);
}

double switchClass::measure(const double actValue)
{
	setting set;

	return measure(actValue, set);
}

double switchClass::measure(const double actValue, setting& set, const double* newValue/*= NULL*/)
{
	bool debug(isDebug());
	bool bbinary(binary());
	bool bSwitched(false);
	bool bOutside(false);
	double dResult(actValue);
	const double* nullValue= NULL;
	string subroutine(getSubroutineName());

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "switch_test_begin_end" &&
		getSubroutineName() == "test_begin_end"					)
	{
		cout << getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	set= NONE;
	if(bbinary)
	{
		if(static_cast<short>(actValue) & 0b01)
			bSwitched= true;
		else
			bSwitched= false;

	}else
	{
		if(	(actValue > 0 || actValue < 0)	)
			bSwitched= true;
		else
			bSwitched= false;
	}

	if(	bSwitched &&
		!m_bLastValue	)
	{// if m_bSwitched is true
	 // but on the last session it was false
	 // the variable be set over the server from outside
		bOutside= true;
		if(debug)
			tout << "SWITCH value was enabled from remote access" << endl;

	}else if(	!bSwitched &&
				m_bLastValue	)
	{// if m_bSwitched is false
	 // but on the last session it was true
	 // the variable be set over the server from outside
		if(debug)
			tout << "SWITCH value was disabled from remote access" << endl;
		bOutside= true;
	}


	if(!bOutside)
	{
		if(	!m_oBegin.isEmpty() &&
			(	!m_bLastValue ||
				m_bAlwaysBegin	)	)
		{// if m_bSwitched is false
		 // and an begin result is set
		 // look for beginning
			if(newValue)
				m_oBegin.setSubVar(subroutine, newValue);
			if(!m_oBegin.calculate(dResult))
			{
				string msg("           could not resolve parameter 'begin= ");

				msg+= m_oBegin.getStatement() + "'\n           in folder ";
				msg+= getFolderName() + " with subroutine ";
				msg+= getSubroutineName();
				TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"begin", msg);
				if(debug)
					cerr << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
			{
				bSwitched= true;
				set= BEGIN;
			}
			if(newValue)
				m_oBegin.setSubVar(subroutine, nullValue);
		}
		if(	set == NONE &&
			bSwitched &&
			!m_oEnd.isEmpty()	)
		{// else if m_bSwitched is true
		 // and an end result be set
		 // look for ending
		 // only in the session when m_bSwitched
		 // not set from outside
			if(newValue)
				m_oEnd.setSubVar(subroutine, newValue);
			if(!m_oEnd.calculate(dResult))
			{
				string msg("           could not resolve parameter 'end= ");

				msg+= m_oEnd.getStatement() + "'\n           in folder ";
				msg+= getFolderName() + " with subroutine ";
				msg+= getSubroutineName();
				TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"end", msg);
				if(debug)
					cerr << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
			{
				bSwitched= false;
				set= END;
			}
			if(newValue)
				m_oEnd.setSubVar(subroutine, nullValue);
		}
		if(	set == NONE &&
			!m_oWhile.isEmpty() &&
			(	bSwitched ||
				m_oBegin.isEmpty()	)	)
		{
			if(newValue)
				m_oWhile.setSubVar(subroutine, newValue);
			if(!m_oWhile.calculate(dResult))
			{
				string msg("           could not resolve parameter 'while= ");

				msg+= m_oWhile.getStatement() + "'\n           in folder ";
				msg+= getFolderName() + " with subroutine ";
				msg+= getSubroutineName();
				TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"while", msg);
				if(debug)
					cerr << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
			{
				bSwitched= true;
			}else
				bSwitched= false;
			set= WHILE;
			if(newValue)
				m_oWhile.setSubVar(subroutine, nullValue);
		}
	}

	if(bbinary)
	{
		if(bSwitched)
			dResult= 0b11;
		else
			dResult= static_cast<double>(static_cast<short>(actValue) & 0b10);
	}else
	{
		if(bSwitched)
			dResult= 1;
		else
			dResult= 0;
	}
	if(getLinkedValue("SWITCH", dResult))
	{
		if(bbinary)
		{
			if(static_cast<short>(dResult) & 0b01)
				bSwitched= true;
			else
				bSwitched= false;
		}else
		{
			if(	dResult > 0 ||
				dResult < 0		)
			{
				bSwitched= true;
			}else
				bSwitched= false;
		}
		bOutside= true;
		set= NONE;
	}
	if(debug)
	{
		tout << "result for SWITCH is ";
		if(bOutside)
			tout << "set from outside to ";
		tout << boolalpha << bSwitched << " (";
		switch(set)
		{
		case BEGIN:
			tout << "BEGIN";
			break;
		case WHILE:
			tout << "WHILE";
			break;
		case END:
			tout << "END";
			break;
		case NONE:
		default:
			tout << "NONE";
		}
		tout << ")" << endl;
	}
	m_bLastValue= bSwitched;
	if(bbinary)
	{
		if(bSwitched)
			return 0b11;
		return static_cast<double>(static_cast<short>(actValue) & 0b10);
	}
	if(bSwitched)
		return 1;
	return 0;
}

void switchClass::setDebug(bool bDebug)
{
	m_oBegin.doOutput(bDebug);
	m_oWhile.doOutput(bDebug);
	m_oEnd.doOutput(bDebug);
	portBase::setDebug(bDebug);
}

bool switchClass::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= 1;
	return true;
}


const SHAREDPTR::shared_ptr<measurefolder_t>  switchClass::getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	SHAREDPTR::shared_ptr<measurefolder_t> current= pStartFolder;

	while(current && current->name != name)
		current= current->next;
	return current;
}

#if 0
double switchClass::getValue(const string& who)
{
	short nRv(0x00);
	double dRv;
	map<string, bool>::iterator found;

	dRv= portBase::getValue(who);
	if(	m_bSwitch &&
		!m_bCurrent)
	{
		if(	dRv > 0 ||
			dRv < 0		)
		{
			nRv= 0x11;
		}
		LOCK(m_VALUELOCK);
		found= m_msbSValue.find(who);
		if(found == m_msbSValue.end())
		{
			m_msbSValue[who]= false;

		}else if(found->second)
		{
			nRv= nRv | 0x10;
			found->second= false;
		}
		UNLOCK(m_VALUELOCK);
		dRv= static_cast<double>(nRv);
	}
	return dRv;
}

void switchClass::setValue(double value, const string& from)
{
	if(	m_bSwitch &&
		!m_bCurrent &&
		(	value > 0 ||
			value < 0	)	)
	{
		LOCK(m_VALUELOCK);
		for(map<string, bool>::iterator it= m_msbSValue.begin(); it != m_msbSValue.end(); ++it)
			it->second= true;
		UNLOCK(m_VALUELOCK);
	}
	portBase::setValue(value, from);
}
#endif

switchClass::~switchClass()
{
	DESTROYMUTEX(m_VALUELOCK);
}
