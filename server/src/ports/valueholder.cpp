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
	bool ValueHolder::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool exist, bWarning= false;
		double value;
		vector<string>::size_type nValue;
		string sMin("min"), sMax("max");
		DbInterface* db= DbInterface::instance();
		string sValue;
		string::size_type nWhileLen;

		m_pStartFolder= pStartFolder;
		m_bFloat= properties.haveAction("float");
		m_nMin= properties.getDouble(sMin, /*warning*/false);
		m_nMax= properties.getDouble(sMax, /*warning*/false);
		nValue= properties.getPropertyCount("value");
		if(nValue > 0)
		{
			for(vector<string>::size_type i= 0; i<nValue; ++i)
			{
				sValue= properties.getValue("value", i, /*warning*/true);
				m_vdValues.push_back(sValue);
			}
			if(nValue > 1)
				bWarning= true;
		}
		m_sWhile= properties.getValue("while", bWarning);
		m_bBooleanWhile= false;
		nWhileLen= m_sWhile.size();
		if(nValue <= 1)
		{
			if(m_sWhile.find("|") >= nWhileLen)
			{
				if(m_sWhile.find("&") < nWhileLen)
					m_bBooleanWhile= true;
			}else
				m_bBooleanWhile= true;
		}
		nValue= properties.getPropertyCount("link");
		for(vector<string>::size_type i= 0; i<nValue; ++i)
		{
			sValue= properties.getValue("link", i);
			m_vsLinks.push_back(sValue);
		}
		bWarning= false;
		if(nValue > 1)
			bWarning= true;
		m_sLinkWhile= properties.getValue("lwhile", bWarning);
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
		sValue= "default";
		m_ddefaultValue= properties.getDouble(sValue, /*warning*/false);

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
			string msg(properties.getMsgHead(/*error*/false));

			msg+= "min and max must be set both! so value can have hole range of";
			if(m_bFloat)
				msg+= " double";
			else
				msg+= " integer";
			LOG(LOG_WARNING, msg);
			cout << msg << endl;
		}
		if(!portBase::init(properties))
			return false;

		value= db->getActEntry(exist, getFolderName(), getSubroutineName(), "value");
		if(!exist)
		{
			if(sValue != "#ERROR")
				value= m_ddefaultValue;
			else
				value= 0;
		}
		setValue(value, "i:"+getFolderName()+":"+getSubroutineName());
		return true;
	}

	void ValueHolder::setObserver(IMeasurePattern* observer)
	{
		string folder(getFolderName());
		string subroutine(getSubroutineName());

		m_poObserver= observer;
		if(m_sWhile != "")
			switchClass::activateObserver(m_pStartFolder, observer, folder, subroutine, m_sWhile, " for parameter while");
		if(m_sLinkWhile != "")
			switchClass::activateObserver(m_pStartFolder, observer, folder, subroutine, m_sLinkWhile, " for parameter lwhile");
	}

	bool ValueHolder::range(bool& bfloat, double* min, double* max)
	{
		if(m_bSetLinkObserver)
		{
			portBase* port;
			string folder(getFolderName()), subroutine(getSubroutineName());

			port= switchClass::getPort(m_pStartFolder, folder, subroutine, m_vsLinks[m_nLinkObserver], /*need own folder*/true, "by search range");
			if(	port &&
				(	port->getFolderName() != folder ||
					port->getSubroutineName() != subroutine	)	)
			{
				return port->range(bfloat, min, max);
			}
		}
		bfloat= m_bFloat;
		*min= m_nMin;
		*max= m_nMax;
		return true;
	}

	double ValueHolder::measure(const double actValue)
	{
		bool isdebug= isDebug();
		bool bChanged= false;
		double value, lvalue;
		string linkfolder;
		string subroutine(getSubroutineName());
		string folder(getFolderName());
		string foldersub(folder + ":" + subroutine);
		vector<string>::size_type links= m_vsLinks.size();
		SHAREDPTR::shared_ptr<portBase> port;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas;

		/*if(	getFolderName() == "TRANSMIT_SONY" &&
			getSubroutineName() == "calculate"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << "TRANSMIT_SONY:calculating" << endl;
		}*/
		value= actValue;
		if(links > 0)
		{
			bool bOk;
			vector<string>::size_type pos;
			string link;

			if(m_sLinkWhile != "")
			{
				bOk= switchClass::calculateResult(m_pStartFolder, folder, m_sLinkWhile, isdebug, lvalue);
				if(bOk)
				{
					if(lvalue < 0)
						pos= 0;
					else if(lvalue > links-1)
						pos= links - 1;
					else
						pos= static_cast<vector<string>::size_type >(lvalue);
					link= m_vsLinks[pos];
				}else
				{
					string msg("cannot create calculation from lwhile parameter '");

					msg+= m_sLinkWhile;
					msg+= "' in folder " + folder + " and subroutine " + subroutine;
					TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg);
					if(isdebug)
						cout << "### ERROR: " << msg << endl;
					value= m_ddefaultValue;
					return false;
				}
				if(	link != subroutine &&
					link != foldersub		)
				{
					bOk= switchClass::searchResult(m_pStartFolder, folder, link, value, &port);
					if(bOk)
					{
						// this time if an link be set the own value have the same value then the link
						// maybe it's better if only the linked value be changed
						// now it's impossible because when any layout file from any client show to this subroutine
						// this subroutine do not actualizes if the linked value will be changed
						// if you want changing to this state
						// you have to enable the source from setValue() and getValue in this class
						// and also in this function after method getResult() remove setValue
						// and by the end return variable actValue
						// then compile and try to found more bugs
						setValue(value, "i:"+foldersub);

						if(isdebug)
						{
							cout << "take value from ";
							if(	m_dLastValue != value ||
								m_dLastValue == actValue	)
							{
								cout << "other subroutine " << link;
								cout << " where actual value is " << value << endl;
							}else
								cout << "own subroutine where actual value is " << actValue << endl;
						}
						// define which value will be use
						// the linked value or own
						if(m_dLastValue != value)
						{ // value be changed in linked subroutine
							bChanged= true;

						}else if(m_dLastValue != actValue)
						{ // value changed from outside of server or with subroutine SET
							vector<string> spl;

							value= actValue;
							bChanged= true;
							split(spl, link, is_any_of(":"));
							linkfolder= spl[0];
							// set pCurMeas to inform other folder routine
							pCurMeas= meash_t::firstInstance;
							while(pCurMeas)
							{
								if(pCurMeas->pMeasure->getThreadName() == linkfolder)
									break;
								pCurMeas= pCurMeas->next;
							}
						}
						if(	!m_bSetLinkObserver ||
							m_nLinkObserver != pos	)
						{// set observer to linked subroutine if not set before
							ostringstream errorinfo;

							if(m_bSetLinkObserver)
							{
								ostringstream einfo;

								if(	m_nLinkObserver < links	)
								{
									einfo << " for " << dec << m_nLinkObserver+1 << ". link parameter";
									switchClass::removeObserver(m_pStartFolder, m_poObserver, folder, subroutine, m_vsLinks[m_nLinkObserver], einfo.str());
								}else
								{
									cout << "### ERROR: in " << folder << ":" << subroutine << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
								}
							}
							errorinfo << " for " << dec << pos+1 << ". link parameter";
							switchClass::activateObserver(m_pStartFolder, m_poObserver, folder, subroutine, link, errorinfo.str());
							m_bSetLinkObserver= true;
							m_nLinkObserver= pos;
							defineRange();
						}
					}else
					{
						ostringstream msg;

						msg << "cannot find subroutine '";
						msg << link << "' from " << dec << (pos+1) << ". link parameter";
						msg << " in folder " << folder << " and subroutine " << subroutine;
						TIMELOG(LOG_ERROR, "searchresult"+folder+":"+subroutine, msg.str());
						if(isdebug)
							cout << "### ERROR: " << msg << endl;
						value= m_ddefaultValue;
						return false;
					}
				}else
				{
					if(m_bSetLinkObserver)
					{
						ostringstream einfo;

						if(	m_nLinkObserver < links	)
						{
							einfo << " for " << dec << m_nLinkObserver+1 << ". link parameter";
							switchClass::removeObserver(m_pStartFolder, m_poObserver, folder, subroutine, m_vsLinks[m_nLinkObserver], einfo.str());
							m_nLinkObserver= 0;
							m_bSetLinkObserver= false;
							defineRange();
						}else
						{
							cout << "### ERROR: in " << folder << ":" << subroutine << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
						}
					}
				}
			}else
				link= m_vsLinks[0];
		}

		if( !bChanged &&
			m_sWhile != "")
		{
			if(getWhileStringResult(m_pStartFolder, folder, subroutine,
									m_sWhile, m_vdValues, m_ddefaultValue, value, m_bBooleanWhile, isdebug))
			{
				//if(port == NULL)
				//	actValue= value;
			}

		}else
		{
			if(isdebug)
				cout << "VALUE be set from outside with " << dec << actValue << endl;
		}
		// this time if an link be set the own value have the same value then the link
		// maybe it's better if only the linked value be changed
		// now it's impossible because when any layout file from any client show to this subroutine
		// this subroutine do not actualizes if the linked value will be changed
		// if you want changing to this state
		// you have to enable the source from setValue() and getValue in this class
		// and also in this function after method getResult() remove setValue
		// and by the end return variable actValue
		// then compile and try to found more bugs
		if(port != NULL)
		{
			port->setValue(value, "i:"+foldersub);
			if(pCurMeas)
				pCurMeas->pMeasure->changedValue(linkfolder, folder);
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

	bool ValueHolder::getWhileStringResult(const SHAREDPTR::shared_ptr<measurefolder_t> pStartFolder, const string& folder, const string& subroutine, const string& whileStr, const vector<string>& content, const double defaultVal, double& value, const bool readBool, const bool debug)
	{
		bool bOk;

		bOk= switchClass::calculateResult(pStartFolder, folder, whileStr, debug, value);

		if(bOk)
		{
			vector<double>::size_type s;

			if(debug)
				cout << "calculated string from while parameter '" << whileStr << "' is " << value << endl;
			s= content.size();
			if(s != 0)
			{
				vector<string>::size_type count;
				ostringstream errorinfo;

				if(value < 0)
					value= 0;
				else if(value > s-1)
					value= s - 1;
				count= static_cast<vector<string>::size_type >(value);
				if(m_bSetValueObserver)
				{
					ostringstream einfo;

					einfo << " for " << dec << m_nValueObserver+1 << ". value parameter";
					switchClass::removeObserver(m_pStartFolder, m_poObserver, folder, subroutine, content[m_nValueObserver], einfo.str());
				}
				if(	!m_bSetValueObserver ||
					count != m_nValueObserver	)
				{
					errorinfo << " for " << dec << count+1 << ". value parameter";
					switchClass::activateObserver(m_pStartFolder, m_poObserver, folder, subroutine, content[count], errorinfo.str());
					m_bSetValueObserver= true;
					m_nValueObserver= count;
				}
				if(debug)
				{
					cout << "select " << dec << count+1 << ". string from value parameters" << endl;
					for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
						cout << "               " << *it << endl;
				}
				if(!switchClass::calculateResult(pStartFolder, folder, content[count], debug, value))
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
