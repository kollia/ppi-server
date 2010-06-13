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

#include "../logger/lib/LogInterface.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;
using namespace boost;
using namespace boost::algorithm;

namespace ports
{
	bool ValueHolder::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool exist, bWarning= false;
		double value, firstValueParam;
		unsigned int nValue;
		string sMin("min"), sMax("max");
		vector<string> vsValues;
		DbInterface* db= DbInterface::instance();
		string sValue("default");
		string sDefValues;
		string::size_type nWhileLen;

		m_pStartFolder= pStartFolder;
		m_bFloat= properties.haveAction("float");
		m_nMin= properties.getDouble(sMin, /*warning*/false);
		m_nMax= properties.getDouble(sMax, /*warning*/false);
		sDefValues= properties.getValue("value", /*warning*/false);
		if(sDefValues != "")
		{
			vector<string> vvalues;

			bWarning= true;
			split(m_vdValues, sDefValues, is_any_of("|"));
		}
		m_sWhile= properties.getValue("while", bWarning);
		m_bBooleanWhile= false;
		nWhileLen= m_sWhile.size();
		if(m_sWhile.find("|") >= nWhileLen)
		{
			if(m_sWhile.find("&") < nWhileLen)
				m_bBooleanWhile= true;
		}else
			m_bBooleanWhile= true;
		if(m_bBooleanWhile)
		{
			if(m_vdValues.size() > 2)
			{
				string msg(properties.getMsgHead(/*error*/false));

				msg+= "while parameter can be only set for first or second value";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
			}
		}
		m_ddefaultValue= properties.getDouble(sValue, /*warning*/false);

		if(	sMin == "#ERROR"
			||
			sMax == "#ERROR"	)
		{// value can have hole range
			m_nMin= 0;
			m_nMax= -1;
			if(	sMin == "#ERROR"
				||
				sMax == "#ERROR"	)
			{
				string msg(properties.getMsgHead(/*error*/false));

				msg+= "min and max must be set both! so value can have hole range of";
				if(m_bFloat)
					msg+= " double";
				else
					msg+= " integer";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
			}
		}
		if(!portBase::init(properties))
			return false;

		// check whether all value parameter are correct
		nValue= 1;
		for(vector<string>::iterator it= m_vdValues.begin(); it != m_vdValues.end(); ++it)
		{
			if(!switchClass::calculateResult(pStartFolder, getFolderName(), *it, value))
			{
				ostringstream msg, sdefault;

				msg << "### ERROR: In folder " + getFolderName() + " and subroutine " + getSubroutineName() << "." << endl;
				msg << "           Calculation of " << nValue << ". value parameter '" << *it << "' ";
				msg << "cannot be done correctly, so set this " << nValue << ". value to ";
				// if defaultValue not be set,
				// the variable should be also 0
				value= m_ddefaultValue;
				sdefault << m_ddefaultValue;
				*it= sdefault.str();
				if(sValue != "#ERROR")
					msg << "defined default ";
				msg << m_ddefaultValue;
				cerr << msg.str() << endl;
				LOG(LOG_ERROR, msg.str());
			}
			if(nValue == 0)
				firstValueParam= value;
			++nValue;
		}
		value= db->getActEntry(exist, getFolderName(), getSubroutineName(), "value");
		if(!exist)
		{
			if(sValue != "#ERROR")
				value= m_ddefaultValue;
			else if(!m_vdValues.empty())
				value= firstValueParam;
			else
				value= 0;
		}
		setValue(value);
		return true;
	}

	void ValueHolder::setObserver(IMeasurePattern* observer)
	{
		if(m_sWhile != "")
		{
			string folder(getFolderName());
			string subroutine(getSubroutineName());

			switchClass::activateObserver(m_pStartFolder, observer, folder, subroutine, m_sWhile);
			if(!m_vdValues.empty())
			{
				for(vector<string>::iterator it= m_vdValues.begin(); it != m_vdValues.end(); ++it)
					switchClass::activateObserver(m_pStartFolder, observer, folder, subroutine, *it);
			}
		}
	}

	bool ValueHolder::range(bool& bfloat, double* min, double* max)
	{
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	double ValueHolder::measure()
	{
		bool isdebug= isDebug();
		double aktValue;

		aktValue= getValue("i:"+getFolderName());
		if(m_sWhile != "")
		{
			double value;

			if(getWhileStringResult(m_pStartFolder, getFolderName(), getSubroutineName(),
									m_sWhile, m_vdValues, m_ddefaultValue, value, m_bBooleanWhile, isdebug))
			{
				aktValue= value;
			}

		}else if(isdebug)
			cout << "VALUE be set from outside with " << dec << aktValue << endl;
		return aktValue;
	}

	bool ValueHolder::getWhileStringResult(const SHAREDPTR::shared_ptr<measurefolder_t> pStartFolder, const string& folder, const string& subroutine, const string& whileStr, const vector<string>& content, const double defaultVal, double& value, const bool readBool, const bool debug)
	{
		bool bOk;
		bool bValue;

		if(readBool)
		{
			string from(whileStr);

			if(debug)
				cout << "make from result: " << from << endl << "read ";
			bOk= switchClass::getResult(from, pStartFolder, folder, debug, bValue);
			if(debug)
				cout << endl;
			if(bOk)
				value= bValue ? 1 : 0;
			else
				value= 0;
		}else
			bOk= switchClass::calculateResult(pStartFolder, folder, whileStr, value);

		if(bOk)
		{
			vector<double>::size_type s;

			if(debug)
				cout << "calculated string from while parameter '" << whileStr << "' is " << value << endl;
			s= content.size();
			if(s != 0)
			{
				int count;

				count= static_cast<int>(value);
				if(value < 0 && value >= s)
				{
					string to;
					string msg("calculated value from '");

					if(value < 0)
					{
						value= 0;
						to= "first position ('0')";
					}else
					{
						value= s;
						to= "last position";
					}
					msg+= whileStr +"' is outside of range from defined value parameter";
					msg+= " so set to " + to + " of value parameter";
					msg+= " in folder " + folder + " and subroutine " + subroutine;
					TIMELOG(LOG_WARNING, "ValueHolderRange"+folder+":"+subroutine, msg);
					if(debug)
						cout << "### WARNING: " << msg << endl;
				}
				if(!switchClass::calculateResult(pStartFolder, folder, content[count], value))
				{
					ostringstream msg;
					ostringstream def;

					++count;
					msg << "calculation of " << count;
					msg << ". value parameter '" << content[count] << "' ";
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

			msg+= whileStr;
			msg+= "' in folder " + folder + " and subroutine " + subroutine;
			TIMELOG(LOG_ERROR, "ValueHoldeCalc"+folder+":"+subroutine, msg);
			if(debug)
				cout << "### ERROR: " << msg << endl;
			value= defaultVal;
			return false;
		}
		return true;
	}
}
