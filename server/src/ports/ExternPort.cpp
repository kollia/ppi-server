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

#include "../pattern/util/LogHolderPattern.h"

#include "../util/thread/Terminal.h"

#include "../database/lib/DbInterface.h"

#include "ExternPort.h"
#include "valueholder.h"
#include "measureThread.h"
#include "timer.h"

using namespace ppi_database;

namespace ports
{
	bool ExternPort::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bRv= true;
		bool bDb;
		DbInterface *db;
		string prop;

		m_pSettings= properties;
		m_bRead= false;

		m_sServer= properties->needValue("type");
		if(m_sServer == "")
			bRv= false;
		m_sChipID= properties->needValue("ID");
		if(m_sChipID == "")
			bRv= false;
		// check before writing in allocateServer() first device access
		// whether should writing into database
		db= DbInterface::instance();
		bDb= properties->haveAction("db");
		if(bDb)
			db->writeIntoDb(getFolderName(), getSubroutineName());
		if(!allocateServer())
			bRv= false;

		if(m_pOWServer)
		{ // set min and max value and action float from range by ask server if not set
			bool bfloat, bCurrent;
			double min, max, val;
			ostringstream sval;
			string prop;

			if(range(bfloat, &min, &max))
			{
				if(!bfloat)
				{
					properties->notAllowedAction("float");
					properties->setAction("int");
				}
				prop= "min";
				val= properties->getDouble(prop, /*warning*/false);
				if(prop == "#ERROR")
				{
					sval << min;
					properties->setDefault("min", sval.str());
				}else
					min= val;
				prop= "max";
				val= properties->getDouble(prop, /*warning*/false);
				if(prop == "#ERROR")
				{
					sval << max;
					properties->setDefault("max", sval.str());
				}else
					max= val;
				if(	!bfloat &&
					min == 0 &&
					max == 1	)
				{
					bCurrent= properties->haveAction("current");
					if(!bCurrent)
						properties->setAction("binary");
				}
			}
		}
		if(!m_bRead)
		{
			if(!switchClass::init(properties, pStartFolder))
				bRv= false;
			prop= properties->getValue("value", /*warning*/false);
			if(prop != "")
			{
				m_oValue.init(pStartFolder, prop);
			}

			m_bDoSwitch= true;
			prop= properties->getValue("begin", /*warning*/false);
			if(prop == "")
			{
				prop= properties->getValue("while", /*warning*/false);
				if(prop == "")
				{
					prop= properties->getValue("end", /*warning*/false);
					if(prop == "")
						m_bDoSwitch= false;
				}
			}
		}else
		{
			if(!portBase::init(properties, pStartFolder))
				bRv= false;
		}
/*		if(	bRv &&
			m_pOWServer	)
		{
			db->registerPortID(getFolderName(), getSubroutineName(), m_pOWServer->getServerName(), m_sChipID);
		}*/
		return bRv;
	}

	void ExternPort::setObserver(IMeasurePattern* observer)
	{
		m_oValue.activateObserver(observer);
		switchClass::setObserver(observer);
	}

	void ExternPort::setDebug(bool bDebug)
	{
		m_oValue.doOutput(bDebug);
		switchClass::setDebug(bDebug);
	}

	bool ExternPort::allocateServer()
	{
		short chipAccess;
		IMeasurePattern* pRunFolder;

		m_pOWServer= OWInterface::getServer(m_sServer, m_sChipID);
		if(!m_pOWServer)
		{
			string msg;
			string log("noserver_");

			log+= m_sServer;
			log+= m_sChipID;
			msg= m_pSettings->getMsgHead(/*error message*/false);
			if(m_bFirstAllocate)
			{
				m_pOWServer= OWInterface::getServer(m_sServer);
				if(m_pOWServer)
				{
					m_pOWServer->usePropActions(m_pSettings);
					m_pOWServer= SHAREDPTR::shared_ptr<OWInterface>();
				}
			}
			msg+= "cannot find OWServer for ID:" + m_sChipID;
			msg+= "\n             try again later";
			if(m_bFirstAllocate)
				cerr << msg << endl;
			m_bFirstAllocate= false;
			TIMELOG(LOG_WARNING, log, msg);
			setDeviceAccess(false);
			pRunFolder= getRunningThread();
			if(pRunFolder != NULL)// when running folder is NULL method was called from init() method and running folder do not exist
				pRunFolder->foundPortServer(false, m_sServer, m_sChipID); // so inform running folder only by first pass of folder list
			return true;// no error try again later
		}
		pRunFolder= getRunningThread();
		if(pRunFolder != NULL)// when running folder is NULL method was called from init() method and running folder do not exist
			pRunFolder->foundPortServer(true, m_sServer, m_sChipID); // so inform running folder only by first pass of folder list
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

	void ExternPort::registerSubroutine()
	{
		DbInterface *reader;

		/*if(registered)
			return;*/
		if(!m_pOWServer)
			return;
		//cout << "register " << m_sChipID << " on " << getFolderName() << ":" << getSubroutineName() << endl;
		reader= DbInterface::instance();
		reader->registerPortID(getFolderName(), getSubroutineName(), m_sServer, m_sChipID);
	}

	bool ExternPort::range(bool& bfloat, double* min, double* max)
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

	auto_ptr<IValueHolderPattern> ExternPort::measure(const ppi_value& actValue)
	{
		bool access, debug, bsetNewValue(false);
		double value(0);
		string addinfo;
		ValueHolder oMeasureValue;
		auto_ptr<IValueHolderPattern> oRv;

		oRv= auto_ptr<IValueHolderPattern>(new ValueHolder());
		debug= isDebug();
		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "readVellemann0" &&
			getSubroutineName() == "counter1"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
		if(!m_pOWServer)
		{
			if(!allocateServer())
			{// the settings in
				string msg("ERROR: ");

				msg+= "ID ";
				msg+= m_sChipID + " for owserver ";
				msg+= m_sServer + " cannot read correctly";
				LOGEX(LOG_ERROR, msg, getRunningThread()->getExternSendDevice());
				if(debug)
					out() << msg << endl;
			}
			if(!m_pOWServer)
			{
				oMeasureValue.value= 0;
				oMeasureValue.lastChanging.clear();
				(*oRv)= oMeasureValue;
				return oRv;
			}
			registerSubroutine();
		}
		if(m_bRead)
		{
			// nothing to do by running thread, only by first!
			// because value before set from owreader (OWServer)
			if(m_bFirstRead)
			{
				m_pOWServer->read(m_sChipID, &value);
				m_bFirstRead= false;
			}else
				value= actValue;
			read(&value);
			if(debug)
			{
				out() << "read from chip " << m_sChipID;
				if(m_sChipType != "")
					out() << " with type " << m_sChipType;
				out() << endl;
				if(hasDeviceAccess())
				{
					if(m_sChipFamily == "10")
					{
						out() << value << "° Grad Celsius" << endl;
					}else
					{
						out() << "on unique id '" << m_sChipID << "' (" << value << ")";

						if(value)
							out() << " valid input" << endl;
						else
							out() << " no input" << endl;
					}
				}else
					out() << "unique id '" << m_sChipID << "' do not reach correctly device for reading" << endl;
			}
		}else
		{
			if(debug)
			{
				out() << "write on chip " << m_sChipID;
				if(m_sChipType != "")
					out() << " with type " << m_sChipType;
				out() << endl;
			}

			if(!m_bDoSwitch)
				m_dLastWValue= actValue;
			m_dLastWValue= switchClass::measure(m_dLastWValue)->getValue();
			if(	!m_bDoSwitch ||
				m_dLastWValue != 0	)
			{
				if(!m_oValue.isEmpty())
					m_oValue.calculate(value);
				else
					value= m_dLastWValue ? 1 : 0;
				access= write(m_sChipID, value, addinfo);
				setDeviceAccess(access);
				bsetNewValue= true;
			}else
				value= actValue;

			if(debug)
			{
				if(!bsetNewValue)
					access= hasDeviceAccess();
				if(access)
				{
					if(	!m_bDoSwitch ||
						m_dLastWValue != 0	)
					{
						out() << "write on unique id '" << m_sChipID;
						out() << "' to output " << value;
						if(addinfo != "")
							out() << " with additional info '" << addinfo << "'";
					}else
						out() << " do not write on '" << m_sChipID << "'";
					out() << endl;
				}else
					out() << "unique id '" << m_sChipID << "' by the last pass, do not reach correctly device for writing" << endl;
			}
		}

		oMeasureValue.value= value;
		(*oRv)= oMeasureValue;
		return oRv;
	}

	auto_ptr<IValueHolderPattern> ExternPort::getValue(const InformObject& who)
	{
		int nvalue;
		double value;
		auto_ptr<IValueHolderPattern> oGetValue;

		if(!onlySwitch())
			return portBase::getValue(who);
		oGetValue= portBase::getValue(who);
		if(who.getDirection() == InformObject::INTERNAL)
		{
			if(oGetValue->getValue())
				oGetValue->setValue(1);
			else
				oGetValue->setValue(0);
			return oGetValue;
		}
		value= (int)oGetValue->getValue();
		nvalue= (int)value;
		nvalue&= 0x01;
		oGetValue->setValue(static_cast<ppi_value>(nvalue));
		portBase::setValue(*oGetValue.get(), who);
		return oGetValue;
	}

	ExternPort::~ExternPort()
	{
	}
}
