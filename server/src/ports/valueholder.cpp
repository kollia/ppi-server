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

#include <climits>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "valueholder.h"
#include "switch.h"
#include "measureThread.h"

#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;
using namespace boost;
using namespace boost::algorithm;

namespace ports
{
	bool ValueHolderSubroutine::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bOk= true, exist= false, bWarning= false;
		ValueHolder oValue;
		vector<string>::size_type nValue;
		string sMin("min"), sMax("max");
		DbInterface* db= DbInterface::instance();
		string sValue, sWhile, sLinkWhile;
		string folder(getFolderName());
		string subroutine(getSubroutineName());

		//Debug info to stop by right subroutine
		/*string stopfolder("TRANSMIT_SONY");
		string stopsub("actual_step");
		if(	getFolderName() == stopfolder &&
			getSubroutineName() == stopsub	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << stopfolder << ":" << stopsub << endl;
		}*/
		m_bFloat= !properties->haveAction("int");
		m_nMin= properties->getDouble(sMin, /*warning*/false);
		m_nMax= properties->getDouble(sMax, /*warning*/false);
		nValue= properties->getPropertyCount("value");
		if(nValue > 0)
		{
			for(vector<string>::size_type i= 0; i<nValue; ++i)
			{
				ostringstream vl;
				ListCalculator* calc;

				vl << "value[" << i << "]";
				sValue= properties->getValue("value", i, /*warning*/true);
				m_vpoValues.push_back(new ListCalculator(folder, subroutine, vl.str(), true, false, this));
				calc= m_vpoValues.back();
				if(!calc->init(pStartFolder, sValue))
					bOk= false;
			}
			if(nValue > 1)
				bWarning= true;
		}
		sWhile= properties->getValue("while", bWarning);
		properties->notAllowedAction("binary");
		if(!m_oWhile.init(pStartFolder, sWhile))
			bOk= false;
		if(!initLinks("VALUE", properties, pStartFolder))
			bOk= false;
		sValue= "default";
		m_ddefaultValue= properties->getDouble(sValue, /*warning*/false);

		if(sMin == "#ERROR")
		{
			m_nMin= LONG_MIN;
			//cout << "set minimal to " << m_nMin << endl;
		}
		if(sMax == "#ERROR")
		{
			m_nMax= LONG_MAX;
			//cout << "set maximal to " << m_nMax << endl;
		}
		if(	sMin == "#ERROR" &&
			sMax == "#ERROR"	)
		{// value can has hole range
			m_nMin= 0;
			m_nMax= -1;
		}
		if(!portBase::init(properties, pStartFolder))
			bOk= false;

		if(!bOk)
			return false;
		oValue.value= db->getActEntry(exist, folder, subroutine, "value");
		if(!exist)
		{
			if(sValue != "#ERROR")
				oValue.value= m_ddefaultValue;
			else
				oValue.value= 0;
		}
		setValue(oValue, InformObject(InformObject::INTERNAL, folder+":"+subroutine));
		return true;
	}

	void ValueHolderSubroutine::setObserver(IMeasurePattern* observer)
	{
		m_poObserver= observer;
		m_oWhile.activateObserver(observer);
		LOCK(m_OBSERVERVALUEMUTEX);
		if(	m_vpoValues.size() > 0 &&
			m_nValueObserver != vector_npos &&
			!m_bSetValueObserver				)
		{
			m_vpoValues[m_nValueObserver]->activateObserver(m_poObserver);
			m_bSetValueObserver= true;
		}
		UNLOCK(m_OBSERVERVALUEMUTEX);
		portBase::setObserver(observer);
	}

	void ValueHolderSubroutine::setDebug(bool bDebug)
	{
		m_oWhile.doOutput(bDebug);
		for(vector<ListCalculator*>::iterator it= m_vpoValues.begin(); it != m_vpoValues.end(); ++it)
			(*it)->doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	bool ValueHolderSubroutine::range(bool& bfloat, double* min, double* max)
	{
		if(portBase::range(bfloat, min, max))
			return true;
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	auto_ptr<IValueHolderPattern> ValueHolderSubroutine::measure(const ppi_value& actValue)
	{
		bool isdebug(isDebug()), bChanged(false), bOutside(false);
		auto_ptr<IValueHolderPattern> oMeasureValue;

		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "kalibrierung1" &&
			getSubroutineName() == "calctime_grad"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
		oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
		if(	isdebug &&
			actValue != m_dOldValue	)
		{// boolean only needed for debug session
			bOutside= true;
		}
		oMeasureValue->setValue(actValue);
		getWhileStringResult(getFolderName(), getSubroutineName(),
										m_oWhile, m_vpoValues, m_ddefaultValue, oMeasureValue, isdebug);
		if(	isdebug &&
			(	oMeasureValue->getValue() > actValue ||
				oMeasureValue->getValue() < actValue	)	)
		{// boolean only needed for debug session
			bChanged= true;
		}
		if(getLinkedValue("VALUE", oMeasureValue))
		{
			if(isdebug)
				out() << "VALUE be set from foreign subroutine to " << dec << oMeasureValue->getValue() << endl;

		}else
		{
			if(isdebug)
			{
				if(bChanged)
					out() << "new value " << dec << oMeasureValue->getValue() << " be changed from own subroutine" << endl;
				else if(bOutside)
					out() << "own value was changed from outside to value " << dec << oMeasureValue->getValue() << endl;
				else
					out() << "current value is " << dec << oMeasureValue->getValue() << endl;
			}
		}
		m_dOldValue= oMeasureValue->getValue();
		return oMeasureValue;
	}

	auto_ptr<IValueHolderPattern> ValueHolderSubroutine::getValue(const InformObject& who)
	{
		// this time if an link be set the own value have the same value then the link
		// maybe it's better if only the linked value be changed
		// now it's impossible because when any layout file from any client show to this subroutine
		// this subroutine do not actualizes if the linked value will be changed
		// if you want changing to this state
		// you have to enable the source from setValue() and getValue in this class
		// and also in this function after method getResult() remove setValue
		// and by the end return variable aktValue
		// then compile and try to found more bugs
		/*if(m_bSetLinkObserver)
		{
			double dRv;

			if(switchClass::calculateResult(m_pStartFolder, getFolderName(), m_vsLinks[m_nLinkObserver], false, dRv))
				return dRv;
		}*/
		return portBase::getValue(who);

	}

	void ValueHolderSubroutine::setValue(const IValueHolderPattern& value, const InformObject& who)
	{
		// this time if an link be set the own value have the same value then the link
		// maybe it's better if only the linked value be changed
		// now it's impossible because when any layout file from any client show to this subroutine
		// this subroutine do not actualizes if the linked value will be changed
		// if you want changing to this state
		// you have to enable the source from setValue() and getValue in this class
		// and also in this function after method getResult() remove setValue
		// and by the end return variable aktValue
		// then compile and try to found more bugs
		/*if(m_bSetLinkObserver)
		{
			portBase* port;

			port= switchClass::getPort(m_pStartFolder, getFolderName(), getSubroutineName(), m_vsLinks[m_nLinkObserver], "");
			if(port != NULL)
			{
				port->setValue(value, who, changed);
				return;
			}
		}*/
		portBase::setValue(value, who);
	}

	bool ValueHolderSubroutine::getWhileStringResult(const string& folder, const string& subroutine,
											ListCalculator& oWhile, vector<ListCalculator*>& content,
											const double defaultVal, auto_ptr<IValueHolderPattern>& ovalue, const bool debug)
	{
		bool bOk;
		ppi_value rvalue;
		ppi_value value;

		if(oWhile.isEmpty())
			return false;
		bOk= oWhile.calculate(rvalue);

		if(bOk)
		{
			vector<double>::size_type s;

			s= content.size();
			if(s != 0)
			{
				vector<string>::size_type count;

				if(rvalue < 0)
					rvalue= 0;
				else if(rvalue > s-1)
					rvalue= s - 1;
				count= static_cast<vector<string>::size_type >(rvalue);
				LOCK(m_OBSERVERVALUEMUTEX);
				if(	m_bSetValueObserver &&
					count != m_nValueObserver	)
				{
					m_bSetValueObserver= false;
					content[m_nValueObserver]->removeObserver(m_poObserver);
				}
				if(	!m_bSetValueObserver &&
					m_poObserver != NULL	)
				{
					content[count]->activateObserver(m_poObserver);
					m_bSetValueObserver= true;
				}
				m_nValueObserver= count;
				UNLOCK(m_OBSERVERVALUEMUTEX);
				if(debug)
				{
					out() << "select " << dec << count+1 << ". string from value parameters" << endl;
					for(vector<ListCalculator*>::const_iterator it= content.begin(); it != content.end(); ++it)
						out() << "               " << (*it)->getStatement() << endl;
				}
				value= ovalue->getValue();
				if(!content[count]->calculate(value))
				{
					ostringstream msg;
					ostringstream def;

					++count;
					msg << "calculation of " << count;
					msg << ". value parameter '" << content[count]->getStatement() << "' ";
					msg << "cannot be done correctly. ";
					msg << "in folder " + folder + " and subroutine " + subroutine;
					def << "ValueHolderCalcVal" << count << folder << ":" << subroutine;
					TIMELOG(LOG_WARNING, def.str(), msg.str());
					ovalue->setValue(defaultVal);
					if(debug)
						out() << "### WARNING: " << msg.str() << endl;
				}else
				{
					ovalue->setValue(value);
					ovalue->setTime(content[count]->getLastChanging());
				}
				if(debug)
					out() << "value be set from value paramter to " << ovalue->getValue() << endl;
			}else
			{// no value parameter exists, so take while result
				ovalue->setValue(rvalue);
				ovalue->setTime(oWhile.getLastChanging());
			}

		}else
		{
			string msg("cannot create calculation from value parameter '");

			msg+= oWhile.getStatement();
			msg+= "' in folder " + folder + " and subroutine " + subroutine;
			TIMELOG(LOG_ERROR, "ValueHoldeCalc"+folder+":"+subroutine, msg);
			if(debug)
				out() << "### ERROR: " << msg << endl;
			ovalue->setValue(defaultVal);
			return false;
		}
		return true;
	}

	ValueHolderSubroutine::~ValueHolderSubroutine()
	{
		vector<ListCalculator*>::iterator it;

		for(it= m_vpoValues.begin(); it != m_vpoValues.end(); ++it)
			delete *it;
		DESTROYMUTEX(m_OBSERVERVALUEMUTEX);
	}
}
