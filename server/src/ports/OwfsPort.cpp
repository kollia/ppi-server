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
#include <errno.h>
#include <vector>

#include "OwfsPort.h"

#include "../database/lib/DbInterface.h"

#include "../logger/lib/LogInterface.h"

using namespace ppi_database;

namespace ports
{
	bool OwfsPort::init(measurefolder_t *pStartFolder, ConfigPropertyCasher &properties)
	{
		bool bRv= true;

		m_pSettings= &properties;
		m_bRead= false;

		m_sServer= properties.needValue("type");
		if(m_sServer == "")
			bRv= false;
		m_sChipID= properties.needValue("ID");
		if(m_sChipID == "")
			bRv= false;
		if(!allocateServer())
			bRv= false;

		//cout << "init subroutine with pin " << m_sChipID << endl;
		if(bRv && !switchClass::init(properties, pStartFolder))
			bRv= false;

		m_sErrorHead= properties.getMsgHead(/*error message*/true);
		m_sWarningHead= properties.getMsgHead(/*error message*/false);
		m_sMsgHead= properties.getMsgHead();// without error identif

		return bRv;
	}

	bool OwfsPort::allocateServer()
	{
		static bool first= true;
		short chipAccess;

		m_pOWServer= OWServer::getServer(m_sServer, m_sChipID);
		if(!m_pOWServer)
		{
			string msg;
			string log("noserver_");

			log+= m_sServer;
			log+= m_sChipID;
			msg= m_pSettings->getMsgHead(/*error message*/false);
			msg+= "cannot find OWServer for ID:" + m_sChipID;
			msg+= "\n             try again later";
			if(first)
				cerr << msg << endl;
			first= false;
			TIMELOG(LOG_WARNING, log, msg);
			setDeviceAccess(false);
			return true;// no error try again later
		}
		m_pOWServer->usePropActions(m_pSettings);
		chipAccess= m_pOWServer->useChip(m_pSettings, m_sChipID, getFolderName(), getSubroutineName());
		if(	chipAccess < 1
			||
			chipAccess > 2	)
		{
			setDeviceAccess(false);
			return false;
		}
		setDeviceAccess(true);
		m_bRead= chipAccess == 1 ? true : false;
		return true;
	}

	void OwfsPort::registerSubroutine()
	{
		static bool registered= false;
		DbInterface *reader;

		if(registered)
			return;
		if(!m_pOWServer)
			return;
		reader= DbInterface::instance();
		reader->registerSubroutine(getSubroutineName(), getFolderName(), m_pOWServer->getServerName(), m_sChipID);
		registered= true;
	}

	bool OwfsPort::range(bool& bfloat, double* min, double* max)
	{
		double dmin= *min;
		double dmax= *max;

		if(!m_pOWServer)
			return false;
		m_pOWServer->range(m_sChipID, dmin, dmax, bfloat);
		*min= dmin;
		*max= dmax;
		return true;
	}

	bool OwfsPort::measure()
	{
		bool access;
		int nvalue;
		double value;

		if(!m_pOWServer)
		{
			if(!allocateServer())
			{// the settings in
				string msg("ERROR: ");

				msg+= "ID ";
				msg+= m_sChipID + " for owserver ";
				msg+= m_sServer + " cannot read correctly";
				LOG(LOG_ERROR, msg);
			}
			if(!m_pOWServer)
				return false;
			registerSubroutine();
		}
		if(m_bRead)
		{
			if(isDebug())
			{
				cout << "read from chip " << m_sChipID << " with type " << m_sChipType << endl;
			}
			//if(!value)
			//	return true;
			access= m_pOWServer->read(m_sChipID, &value);
			setDeviceAccess(access);
			if(isDebug())
			{
				if(access)
				{
					if(m_sChipFamily == "10")
					{
						cout << value << "° Grad Celsius" << endl;
					}else
					{
						cout << "on unique id '" << m_sChipID << "' (" << value << ")";

						if(value)
							cout << " valid input" << endl;
						else
							cout << " no input" << endl;
					}
				}else
					cout << "unique id '" << m_sChipID << "' do not reache correctly device for reading" << endl;
			}
			if(onlySwitch())
			{
				//cout << "read on pin " << m_sChipID << " read "<< portBase::getBinString((long)value, 2) << " set pin from " << portBase::getBinString((long)value, 2) << flush;
				if(	value < 0
					||
					value > 0	)//((int)value) & 0x01)
				{
					//cout << " to 11" << flush;
					value= 3;
				}else
				{
					nvalue= (int)portBase::getValue("i:"+getFolderName());
					nvalue&= 0x02;
					value= (double)nvalue;
					//cout << " to " << portBase::getBinString((long)value, 2) << flush;
				}
				//cout << endl;
			}
			portBase::setValue(value);
		}else
		{

			if(isDebug())
			{
				cout << "write on chip " << m_sChipID << " with type " << m_sChipType << endl;
			}
			if(onlySwitch())
			{
				if(!switchClass::measure())
					return false;
			}
			value= getValue("i:" + getFolderName());
			access= m_pOWServer->write(m_sChipID, value);
			setDeviceAccess(access);
			if(isDebug())
			{
				if(access)
				{
					cout << "on unique id '" << m_sChipID;

					if(value)
						cout << " for an output" << endl;
					else
						cout << " for no output" << endl;
				}else
					cout << "unique id '" << m_sChipID << "' do not reache correctly device for writing" << endl;
			}
		}

		return true;
	}

	double OwfsPort::getValue(const string who)
	{
		int nvalue;
		double value;

		//cout << who << " read pin " << flush;
		if(!onlySwitch())
		{
			//cout << endl;
			return portBase::getValue(who);
		}
		if(who.substr(0, 2) == "i:")
		{
			//cout << endl;
			if(portBase::getValue(who))
				return 1;
			return 0;
		}
		value= (int)portBase::getValue(who);
		//cout << "set from " << portBase::getBinString((long)value, 2);
		nvalue= (int)value;
		nvalue&= 0x01;
		portBase::setValue((double)nvalue);
		//cout << " to " << portBase::getBinString((long)nvalue, 2) << endl;
		return value;
	}

	OwfsPort::~OwfsPort()
	{
	}
}
