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

#include "read.h"

namespace ports
{
	using namespace boost;

	bool Read::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bOk(true);
		bool bDebugShowContent;
		bool bHoldConnection;
		string res;
		ReadWorker::holdTime_e eTime;
		URL address;

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
		if(!m_oReader.initial(address, bHoldConnection, eTime, bDebugShowContent))
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
		ppi_time nullTime;
		auto_ptr<IValueHolderPattern> oValue;

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
		if(debug)
			out() << "result of subroutine is " << oValue->getValue() << endl;
		return oValue;
	}

	void Read::setDebug(bool bDebug)
	{
		m_oReader.setDebug(bDebug);
		switchClass::setDebug(bDebug);
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

		if(m_bWhileSet)
		{
			sRv= "cannot make starting possibility when parameter begin/while or end be set\n";
			sRv= "inside " + getFolderName() + ":" + getSubroutineName();

		}else
		{
			int policy, priority;

			getRunningThread()->getSchedulingParameter(policy, priority);
			if(!m_oReader.setSchedulingParameter(policy, priority))
			{
				sRv= "for subroutine " + getFolderName() + ":" + getSubroutineName() + "\n";
				sRv+= "cannot set policy and priority";

			}else if(m_oReader.initialStarting() != 0)
			{
				sRv= "cannot start worker thread for READ subroutine\n";
				sRv= "inside " + getFolderName() + ":" + getSubroutineName();

			}else
				m_bTimerStart= true;
		}
		return sRv;
	}


} /* namespace ports */
