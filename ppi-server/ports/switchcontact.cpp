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
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <iostream>

#include "../util/structures.h"

#include "../logger/LogThread.h"

#include "switch.h"
#include "switchcontact.h"

//void switchContact::init(folder *pStartFolder, Pins tOut, string on, string sWhile, string off)
bool switchContact::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
{
	string value;
	vector<string> split;
	portBase::portpin_address_t ePortPin;

	if(!switchClass::init(properties, pStartFolder))
		return false;
	value= properties.needValue("out");
	if(value != "")
	{
		split= ConfigPropertyCasher::split(value, ":");
		if(split.size() == 2)
		{
			m_tOut.nPort= getPortAddress(split[0]);
			m_tOut.ePin= getPinEnum(split[1]);
			ePortPin= getPortPinAddress(m_tOut, /*after*/false);
			if(	m_tOut.ePin == portBase::NONE
				||
				ePortPin.ePort != portBase::getPortType(split[0])
				||
				ePortPin.eDescript != portBase::SETPIN				)
			{
				string msg;

				msg= properties.getMsgHead(/*error*/true);
				msg+= "pin '";
				msg+= split[1] + "' for parameter out is no correct pin on port '";
				msg+= split[0];
				LOG(AKERROR, msg);
				cerr << msg << endl;
				return false;
			}
		}else
		{
			string msg(properties.getMsgHead(/*error*/false));

			msg+= "undefined <port>:<pin> ";
			msg+= value + " for parameter out";
			LOG(AKERROR, msg);
			cerr << msg << endl;
			return false;
		}
	}else
		return false;
	return true;
}

bool switchContact::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= 1;
	return true;
}

bool switchContact::measure()
{
	bool bSwitched= false;
	bool bResultTrue= false;
	string msg("set pin ");
	string who("i:" + getFolderName());

	if(getValue(who))
		bSwitched= true;
	switchClass::measure();
	if(getValue(who))
	{
		bResultTrue= true;
		setPin(m_tOut, true);
	}else
		setPin(m_tOut, false);

	if(bResultTrue)
	{
		if(!bSwitched)
		{
			msg+= getPinName(m_tOut.ePin);
			msg+= " on port ";
			msg+= getPortName(m_tOut.nPort);
			msg+= " to +10 Volt";
			LOG(AKDEBUG, msg);
			//setPin(m_tOut, true);
			if(isDebug())
				cout << msg << endl;
		}
	}else
	{
		if(bSwitched)
		{
			msg+= getPinName(m_tOut.ePin);
			msg+= " on port ";
			msg+= getPortName(m_tOut.nPort);
			msg+= " to -10 Volt";
			LOG(AKDEBUG, msg);
			//setPin(m_tOut, false);
			if(isDebug())
				cout << msg << endl;
		}
	}
	return true;
}

switchContact::~switchContact()
{
	if(m_tOut.ePin != NONE)
		setPin(m_tOut, false);
}
