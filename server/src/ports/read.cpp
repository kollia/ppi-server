/**
 *   This file 'read.cpp' is part of ppi-server.
 *   Created on: 16.04.2014
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

#include "../util/GlobalStaticMethods.h"
#include "../util/properties/PPIConfigFileStructure.h"

#include "read.h"

namespace ports
{
	using namespace util;
	using namespace boost;

	bool Read::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bOk(true);
		bool bDebugShowContent;
		bool bHoldConnection;
		string res, address;
		ReadWorker::holdTime_e eTime;
		URL oAddress;
		PPIConfigFiles configFiles;

		configFiles= PPIConfigFileStructure::instance();
		bHoldConnection= properties->haveAction("hold");
		bDebugShowContent= properties->haveAction("debug");
		res= properties->getValue("time", /*warning*/false);
		if(res == "start")
			eTime= ReadWorker::start;
		else if(res == "connect")
			eTime= ReadWorker::connect;
		else if(res == "send")
			eTime= ReadWorker::send;
		else if(res == "end")
			eTime= ReadWorker::end;
		else
		{
			eTime= ReadWorker::end;
			if(res != "")
			{
				res= properties->getMsgHead(/*ERROR*/false);
				res+= "time definition of '" + res + "' isn't defined correctly\n";
				res+= "so define default holding time of subroutine as 'end'";
				LOG(LOG_WARNING, res);
				tout << res << endl;
			}
		}
		address= properties->getValue("src", /*warning*/true);
		oAddress= configFiles->createCommand(getFolderName(), getSubroutineName(), "address", address);
		if(!m_oReader.initial(oAddress, bHoldConnection, eTime, bDebugShowContent))
			bOk= false;

		if(	properties->getValue("begin", /*warning*/false) != "" ||
			properties->getValue("while", /*warning*/false) != "" ||
			properties->getValue("end", /*warning*/false) != ""		)
		{
			m_bWhileSet= true;
		}
		if(!m_bTimerStart)
		{
			if(!switchClass::init(properties, pStartFolder))
				bOk= false;

		}else if(m_bWhileSet)
			bOk= false;// cannot set begin/while/end and also starting from timer subroutine

		return bOk;
	}

	auto_ptr<IValueHolderPattern> Read::measure(const ppi_value& actValue)
	{
		bool debug(isDebug());
		bool bHoldFirstPass;
		ppi_time nullTime;
		auto_ptr<IValueHolderPattern> oValue;

		if(m_bTimerStart)
		{
			LOCK(m_HOLDFRISTPASS);
			bHoldFirstPass= m_bHoldFirstPass;
			m_bHoldFirstPass= false;
			UNLOCK(m_HOLDFRISTPASS);

			oValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
			if(bHoldFirstPass)
				oValue->setValue(actValue);
			if(debug)
			{
				ostringstream res;

				res << "result of subroutine ";
				if(bHoldFirstPass)
					res << "by first passing is " << actValue;
				else
					res << "is 0";
				out() << res.str() << endl;
			}
			return oValue;
		}
		oValue= switchClass::measure(m_nDo);
		if(oValue->getValue() == 0)
		{
			m_nDo= 0;
			oValue->setValue(0);
			nullTime.clear();
			oValue->setTime(nullTime);
			if(debug)
				out() << "result of subroutine is 0" << endl;
			return oValue;
		}
		m_nDo= 1;
		oValue= m_oReader.doHttpConnection(actValue, debug);
		m_oReader.writeDebug();
		if(debug)
			out() << "result of subroutine is " << oValue->getValue() << endl;
		return oValue;
	}

	void Read::setDebug(bool bDebug)
	{
		m_oReader.setDebug(bDebug);
		switchClass::setDebug(bDebug);
	}

	void Read::setValue(const IValueHolderPattern& value, const InformObject& from)
	{
		/*
		 * make lock outside from if-sentence
		 * because otherwise maybe an later
		 * setting with no READWORKER direction
		 * can be set first
		 * while value with READWORKER
		 * waiting for lock
		 */
		LOCK(m_HOLDFRISTPASS);
		if(from.getDirection() == InformObject::READWORKER)
			m_bHoldFirstPass= true;
		switchClass::setValue(value, from);
		UNLOCK(m_HOLDFRISTPASS);
	}

	bool Read::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= -1;
		return true;
	}

	string Read::checkStartPossibility()
	{
		string sRv;

		if(m_bTimerStart)
			return "";
		if(m_bWhileSet)
		{
			sRv= "cannot make starting possibility when parameter begin/while or end be set\n";
			sRv= "inside " + getFolderName() + ":" + getSubroutineName();

		}else
		{
			int policy, priority;
			EHObj err;

			getRunningThread()->getSchedulingParameter(policy, priority);
			err= m_oReader.setSchedulingParameter(policy, priority);
			if(err->fail())
			{
				ostringstream uid;

				err->addMessage("Read", "set_policy", getFolderName() + "@" + getSubroutineName());
				sRv= err->getDescription();
				LOG(LOG_WARNING, sRv);
				cout << glob::addPrefix("### WARNING: ", sRv);
				sRv= "";

			}
			err= m_oReader.initialStarting();
			if(err->fail())
			{
				err->addMessage("Read", "wrong_start", getFolderName() + "@" + getSubroutineName());
				sRv= err->getDescription();
				if(err->hasWarning())
				{
					LOG(LOG_WARNING, sRv);
					cout << glob::addPrefix("### WARNING: ", sRv);
					sRv= "";
					m_bTimerStart= true;
				}

			}else
				m_bTimerStart= true;
		}
		return sRv;
	}


} /* namespace ports */
