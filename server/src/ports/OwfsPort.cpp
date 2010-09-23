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
#include "valueholder.h"

#include "../database/lib/DbInterface.h"

#include "../logger/lib/LogInterface.h"

using namespace ppi_database;

namespace ports
{
	bool OwfsPort::init(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, ConfigPropertyCasher &properties)
	{
		bool bRv= true;
		DbInterface *db;
		string prop;

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

		if(m_bRead)
		{ // if server cannot be allocated m_bRead should be set to false
		  // and this block don't be reached
			properties.notAllowedParameter("while");
			properties.notAllowedParameter("value");
		}
		if(m_pOWServer)
		{ // set min and max value and action float from range by ask server if not set
			bool bfloat;
			double min, max, val;
			ostringstream sval;
			string prop;

			if(range(bfloat, &min, &max))
			{
				if(bfloat && !properties.haveAction("float"))
					properties.readLine("action= float");
				prop= "min";
				val= properties.getDouble(prop, /*warning*/false);
				if(prop == "#ERROR")
				{
					sval << min;
					properties.readLine("min= "+sval.str());
				}
				prop= "max";
				val= properties.getDouble(prop, /*warning*/false);
				if(prop == "#ERROR")
				{
					sval << max;
					properties.readLine("max= "+sval.str());
				}
			}
		}
		if(bRv && !ValueHolder::init(properties, pStartFolder))
			bRv= false;

		db= DbInterface::instance();
		db->useChip(getFolderName(), getSubroutineName(), m_sServer, m_sChipID);

		m_sErrorHead= properties.getMsgHead(/*error message*/true);
		m_sWarningHead= properties.getMsgHead(/*error message*/false);
		m_sMsgHead= properties.getMsgHead();// without error identif

		return bRv;
	}

	bool OwfsPort::allocateServer()
	{
		short chipAccess;

		m_pOWServer= OWInterface::getServer(m_sServer, m_sChipID);
		if(!m_pOWServer)
		{
			string msg;
			string log("noserver_");

			log+= m_sServer;
			log+= m_sChipID;
			msg= m_pSettings->getMsgHead(/*error message*/false);
			msg+= "cannot find OWServer for ID:" + m_sChipID;
			msg+= "\n             try again later";
			if(!m_bDisplayNotFound)
				cerr << msg << endl;
			m_bDisplayNotFound= true;
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

	double OwfsPort::measure(const double actValue)
	{
		bool access, debug, bsetNewValue= false;
		double value;

		debug= isDebug();
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
			// nothing to do!
			// value before set from the owreader (OWServer)
			value= actValue;
			if(debug)
			{
				cout << "read from chip " << m_sChipID;
				if(m_sChipType != "")
					cout << " with type " << m_sChipType;
				cout << endl;
				if(hasDeviceAccess())
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
		}else
		{
			if(debug)
			{
				cout << "write on chip " << m_sChipID;
				if(m_sChipType != "")
					cout << " with type " << m_sChipType;
				cout << endl;
			}
			value= ValueHolder::measure(actValue);

			// 2010/08/03 ppi@magnificat.at
			//				write always to server,
			//				because maybe server do output when get value 1
			//				and should only do output by next getting value 1 (like LIRC with send_once)
			//				By behavior before server get only the first 1 and do ignore the second
			//if(value != m_dLastWValue)
			{
				access= m_pOWServer->write(m_sChipID, value);
				setDeviceAccess(access);
				m_dLastWValue= value;
				bsetNewValue= true;
			}
			if(debug)
			{
				if(!bsetNewValue)
					access= hasDeviceAccess();
				if(access)
				{
					cout << "on unique id '" << m_sChipID;

					if(value)
						cout << " set to output " << value;
					else
						cout << " set to output " << value;
					if(bsetNewValue)
						cout << " in passing before";
					cout << endl;
				}else
					cout << "unique id '" << m_sChipID << "' do not reach correctly device for writing" << endl;
			}
		}

		return value;
	}

	double OwfsPort::getValue(const string who)
	{
		int nvalue;
		double value;

		if(!onlySwitch())
			return portBase::getValue(who);
		if(who.substr(0, 2) == "i:")
		{
			if(portBase::getValue(who))
				return 1;
			return 0;
		}
		value= (int)portBase::getValue(who);
		nvalue= (int)value;
		nvalue&= 0x01;
		portBase::setValue((double)nvalue);
		return value;
	}

	OwfsPort::~OwfsPort()
	{
	}
}
