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
#include <iostream>

#include "contactpin.h"

#include "../logger/LogThread.h"

namespace ports
{
	//void contactPin::init(Pins tIn, Pins tOut)
	bool contactPin::init(ConfigPropertyCasher &properties)
	{
		string value;
		vector<string> split;
		portBase::portpin_address_t ePortPin;

		m_bContact= false;
		properties.getValue("out", /*warning*/false);
		value= properties.needValue("in");
		if(value == "")
			return false;
		split= ConfigPropertyCasher::split(value, ":");
		if(split.size() == 2)
		{
			m_tIn.nPort= getPortAddress(split[0]);
			m_tIn.ePin= getPinEnum(split[1]);
			ePortPin= getPortPinAddress(m_tIn, /*after*/false);
			if(	m_tIn.ePin == portBase::NONE
				||
				ePortPin.ePort != portBase::getPortType(split[0])
				||
				ePortPin.eDescript == portBase::SETPIN				)
			{
				string msg;

				msg= properties.getMsgHead(/*error*/true);
				msg+= "pin '";
				msg+= split[1] + "' for parameter in is no correct pin on port '";
				msg+= split[0];
				LOG(LOG_ERROR, msg);
				cerr << msg << endl;
				return false;
			}
		}else
		{
			string msg(properties.getMsgHead(/*error*/true));

			msg+= "undefined <port>:<pin> ";
			msg+= value + " for parameter in";
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return false;
		}
		value= properties.getValue("out", /*warning*/false);
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
					ePortPin.eDescript == portBase::SETPIN				)
				{
					string msg;

					msg= properties.getMsgHead(/*error*/false);
					msg+= "pin '";
					msg+= split[1] + "' for parameter out is no correct pin on port '";
					msg+= split[0];
					LOG(LOG_WARNING, msg);
					cerr << msg << endl;
					m_tOut.ePin= NONE;
				}else
					setPin(m_tOut, true);
			}else
			{
				string msg(properties.getMsgHead(/*error*/false));

				msg+= "undefined <port>:<pin> ";
				msg+= value + " for parameter out";
				LOG(LOG_WARNING, msg);
				cerr << msg << endl;
				m_tOut.ePin= NONE;
			}
		}else
			m_tOut.ePin= NONE;
		return true;
	}

	bool contactPin::measure()
	{
		m_bContact= getPin(m_tIn);
		if(isDebug())
		{
			cout << "subroutine has ";
			if(!m_bContact)
				cout << "no ";
			cout << "contact" << endl;
		}
		return true;
	}

	bool contactPin::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= 1;
		return true;
	}

	void contactPin::setValue(const double value)
	{
		if(value)
			m_bContact= true;
		else
			m_bContact= false;
	}

	double contactPin::getValue()
	{
		if(m_bContact)
			return 1;
		return 0;
	}

	contactPin::~contactPin()
	{
		if(m_tOut.ePin != NONE)
			setPin(m_tOut, false);
	}
}
