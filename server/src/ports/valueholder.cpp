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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "valueholder.h"
#include "switch.h"
#include "measureThread.h"

#include "../logger/lib/LogInterface.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;
using namespace boost;
using namespace boost::algorithm;

namespace ports
{
	bool ValueHolder::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bOk= true, exist= false, bWarning= false;
		double value;
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
				m_vpoValues.push_back(new ListCalculator(folder, subroutine, vl.str(), true, false));
				calc= m_vpoValues.back();
				if(!calc->init(pStartFolder, sValue))
					bOk= false;
			}
			if(nValue > 1)
				bWarning= true;
		}
		sWhile= properties->getValue("while", bWarning);
		if(!m_oWhile.init(pStartFolder, sWhile))
			bOk= false;
		if(!initLinks("VALUE", properties, pStartFolder))
			bOk= false;
		sValue= "default";
		m_ddefaultValue= properties->getDouble(sValue, /*warning*/false);

		if(sMin == "#ERROR")
			m_nMin= 0;
		if(sMax == "#ERROR")
		{// value can have hole range
			m_nMax= m_nMin - 1;
		}
		if(	(	sMin == "#ERROR" &&
				sMax == "max"		)
			||
			(	sMin == "min" &&
				sMax == "#ERROR"	)	)
		{
			string msg(properties->getMsgHead(/*error*/false));

			msg+= "min and max must be set both! so value can have hole range of";
			if(m_bFloat)
				msg+= " double";
			else
				msg+= " integer";
			LOG(LOG_WARNING, msg);
			cout << msg << endl;
		}
		if(!portBase::init(properties))
			bOk= false;

		if(!bOk)
			return false;
		value= db->getActEntry(exist, folder, subroutine, "value");
		if(!exist)
		{
			if(sValue != "#ERROR")
				value= m_ddefaultValue;
			else
				value= 0;
		}
		setValue(value, "i:"+folder+":"+subroutine);
		return true;
	}

	void ValueHolder::setObserver(IMeasurePattern* observer)
	{
		m_poObserver= observer;
		m_oWhile.activateObserver(observer);
		portBase::setObserver(observer);
	}

	void ValueHolder::setDebug(bool bDebug)
	{
		m_oWhile.doOutput(bDebug);
		for(vector<ListCalculator*>::iterator it= m_vpoValues.begin(); it != m_vpoValues.end(); ++it)
			(*it)->doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	bool ValueHolder::range(bool& bfloat, double* min, double* max)
	{
		if(portBase::range(bfloat, min, max))
			return true;
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	double ValueHolder::measure(const double actValue)
	{
		bool isdebug(isDebug()), bChanged(false), bOutside(false);
		double value;

		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "TRANSMIT_SONY_choice" &&
			getSubroutineName() == "after"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
		value= actValue;
		if(	isdebug &&
			m_dLastValue != value	)
		{// boolean only needed for debug session
			bOutside= true;
		}
		getWhileStringResult(getFolderName(), getSubroutineName(),
										m_oWhile, m_vpoValues, m_ddefaultValue, value, isdebug);
		if(	isdebug &&
			(	value > actValue ||
				value < actValue	)	)
		{// boolean only needed for debug session
			bChanged= true;
		}
		if(getLinkedValue("VALUE", value))
		{
			if(isdebug)
				cout << "VALUE be set from foreign subroutine to " << dec << value << endl;

		}else
		{
			if(isdebug)
			{
				if(bChanged)
					cout << "new value " << dec << value << " be changed from own subroutine" << endl;
				else if(bOutside)
					cout << "own value was changed from outside to value " << dec << value << endl;
				else
					cout << "current value is " << dec << value << endl;
			}
		}
		m_dLastValue= value;
		return value;
	}

	double ValueHolder::getValue(const string& who)
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

	void ValueHolder::setValue(const double value, const string& who)
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
				port->setValue(value, who);
				return;
			}
		}*/
		portBase::setValue(value, who);
	}

	bool ValueHolder::getWhileStringResult(const string& folder, const string& subroutine,
											ListCalculator& oWhile, vector<ListCalculator*>& content,
											const double defaultVal, double& value, const bool debug)
	{
		bool bOk;

		if(oWhile.isEmpty())
			return false;
		bOk= oWhile.calculate(value);

		if(bOk)
		{
			vector<double>::size_type s;

			s= content.size();
			if(s != 0)
			{
				vector<string>::size_type count;

				if(value < 0)
					value= 0;
				else if(value > s-1)
					value= s - 1;
				count= static_cast<vector<string>::size_type >(value);
				if(m_bSetValueObserver)
					content[m_nValueObserver]->removeObserver(m_poObserver);
				if(	!m_bSetValueObserver ||
					count != m_nValueObserver	)
				{
					content[count]->activateObserver(m_poObserver);
					m_bSetValueObserver= true;
					m_nValueObserver= count;
				}
				if(debug)
				{
					cout << "select " << dec << count+1 << ". string from value parameters" << endl;
					for(vector<ListCalculator*>::const_iterator it= content.begin(); it != content.end(); ++it)
						cout << "               " << (*it)->getStatement() << endl;
				}
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
					value= defaultVal;
					if(debug)
						cout << "### WARNING: " << msg.str() << endl;
				}
				if(debug)
					cout << "value be set from value paramter to " << value << endl;
			}

		}else
		{
			string msg("cannot create calculation from value parameter '");

			msg+= oWhile.getStatement();
			msg+= "' in folder " + folder + " and subroutine " + subroutine;
			TIMELOG(LOG_ERROR, "ValueHoldeCalc"+folder+":"+subroutine, msg);
			if(debug)
				cout << "### ERROR: " << msg << endl;
			value= defaultVal;
			return false;
		}
		return true;
	}

	ValueHolder::~ValueHolder()
	{
		vector<ListCalculator*>::iterator it;

		for(it= m_vpoValues.begin(); it != m_vpoValues.end(); ++it)
			delete *it;
	}
}
