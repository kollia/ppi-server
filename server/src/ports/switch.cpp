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

switchClass::switchClass(string folderName, string subroutineName, unsigned short objectID)
: portBase("SWITCH", folderName, subroutineName, objectID),
  m_oBegin(folderName, subroutineName, "begin", false, true, this),
  m_oWhile(folderName, subroutineName, "while", false, true, this),
  m_oEnd(folderName, subroutineName, "end", false, true, this),
  m_bSwitch(true),
  m_bLastValue(false),
  m_bCurrent(false),
  m_bAlwaysBegin(false)
{
	m_VALUELOCK= Thread::getMutex("VALUELOCK");
}

switchClass::switchClass(string type, string folderName, string subroutineName, unsigned short objectID)
: portBase(type, folderName, subroutineName, objectID),
  m_oBegin(folderName, subroutineName, "begin", false, true, this),
  m_oWhile(folderName, subroutineName, "while", false, true, this),
  m_oEnd(folderName, subroutineName, "end", false, true, this),
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

auto_ptr<IValueHolderPattern> switchClass::measure(const ppi_value& actValue)
{
	setting set;

	return measure(actValue, set);
}

auto_ptr<IValueHolderPattern> switchClass::measure(const ppi_value& actValue, bool bOutsidePossible)
{
	setting set;

	return measure(actValue, set, NULL, bOutsidePossible);
}

auto_ptr<IValueHolderPattern> switchClass::measure(const ppi_value& actValue, setting& set,
				const double* newValue/*= NULL*/, bool bOutsidePossible/*= true*/)
{
	bool debug(isDebug());
	bool bbinary(binary());
	bool bSwitched(false);
	bool bOutside(false);
	double dResult(actValue);
	const double* nullValue= NULL;
	string subroutine(getSubroutineName());
	auto_ptr<IValueHolderPattern> oMeasureValue;

	//Debug info to stop by right subroutine
	/*if(	getFolderName() == "Raff1" &&
		getSubroutineName() == "Zu"					)
	{
		cout << getFolderName() << ":" << getSubroutineName() << endl;
		cout << __FILE__ << __LINE__ << endl;
	}*/
	set= NONE;
	oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
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

	if(bOutsidePossible)
	{
		if(	bSwitched &&
			!m_bLastValue	)
		{// if m_bSwitched is true
		 // but on the last session it was false
		 // the variable be set over the server from outside
			bOutside= true;
			if(debug)
				out() << "SWITCH value was enabled from remote access" << endl;

		}else if(	!bSwitched &&
					m_bLastValue	)
		{// if m_bSwitched is false
		 // but on the last session it was true
		 // the variable be set over the server from outside
			if(debug)
				out() << "SWITCH value was disabled from remote access" << endl;
			bOutside= true;
		}
	}else
		m_bLastValue= bSwitched;


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
					out() << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
			{
				bSwitched= true;
				set= BEGIN;
			}
			oMeasureValue->setTime(m_oBegin.getLastChanging());
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
					out() << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
			{
				bSwitched= false;
				set= END;
			}
			oMeasureValue->setTime(m_oEnd.getLastChanging());
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
					out() << endl << "### ERROR: " << msg.substr(11) << endl;

			}else if(dResult > 0 || dResult < 0)
				bSwitched= true;
			else
				bSwitched= false;
			oMeasureValue->setTime(m_oWhile.getLastChanging());
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
	oMeasureValue->setValue(dResult);
	if(getLinkedValue("SWITCH", oMeasureValue))
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
		ostringstream sout;

		sout << "result for SWITCH is ";
		if(bOutside)
			sout << "set from outside to ";
		sout << boolalpha << bSwitched << " (";
		switch(set)
		{
		case BEGIN:
			sout << "BEGIN";
			break;
		case WHILE:
			sout << "WHILE";
			break;
		case END:
			sout << "END";
			break;
		case NONE:
		default:
			sout << "NONE";
			break;
		}
		out() << sout.str() << ")" << endl;
	}
	m_bLastValue= bSwitched;
	if(bbinary)
	{
		if(bSwitched)
			oMeasureValue->setValue(0b11);
		else
			oMeasureValue->setValue(static_cast<double>(static_cast<short>(actValue) & 0b10));
		return oMeasureValue;
	}
	if(bSwitched)
		oMeasureValue->setValue(1);
	else
		oMeasureValue->setValue(0);
	return oMeasureValue;
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

switchClass::~switchClass()
{
	DESTROYMUTEX(m_VALUELOCK);
}
