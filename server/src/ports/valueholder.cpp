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
		bool exist= false, bWarning= false;
		double value;
		vector<string>::size_type nValue;
		string sMin("min"), sMax("max");
		DbInterface* db= DbInterface::instance();
		string sValue, sWhile, sLinkWhile;
		string folder(getFolderName());
		string subroutine(getSubroutineName());

		//Debug info to stop by right subroutine
		string stopfolder("TRANSMIT_SONY");
		string stopsub("actual_step");
		if(	getFolderName() == stopfolder &&
			getSubroutineName() == stopsub	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << stopfolder << ":" << stopsub << endl;
		}
		m_bFloat= properties.haveAction("float");
		m_nMin= properties.getDouble(sMin, /*warning*/false);
		m_nMax= properties.getDouble(sMax, /*warning*/false);
		nValue= properties.getPropertyCount("value");
		if(nValue > 0)
		{
			for(vector<string>::size_type i= 0; i<nValue; ++i)
			{
				ostringstream vl;
				ListCalculator* calc;

				vl << "value[" << i << "]";
				sValue= properties.getValue("value", i, /*warning*/true);
				m_vpoValues.push_back(new ListCalculator(folder, subroutine, vl.str(), false));
				calc= m_vpoValues.back();
				calc->init(pStartFolder, sValue);
			}
			if(nValue > 1)
				bWarning= true;
		}
		sWhile= properties.getValue("while", bWarning);
		m_oWhile.init(pStartFolder, sWhile);
		nValue= properties.getPropertyCount("link");
		for(vector<string>::size_type i= 0; i<nValue; ++i)
		{
			ostringstream lk;
			ListCalculator* calc;
			vector<string> spl;

			lk << "link[" << i << "]";
			sValue= properties.getValue("link", i);
			trim(sValue);
			split(spl, sValue, is_any_of(":"));
			if(spl.size() > 0)
				trim(spl[0]);
			if(spl.size() == 2)
				trim(spl[1]);
			if(	(	spl.size() == 1 &&
					spl[0].find(" ") == string::npos	) ||
				(	spl.size() == 2 &&
					spl[0].find(" ") == string::npos &&
					spl[1].find(" ") == string::npos	)	)
			{
				m_vpoLinks.push_back(new ListCalculator(folder, subroutine, lk.str(), false));
				calc= m_vpoLinks.back();
				calc->init(pStartFolder, sValue);

			}else
			{
				ostringstream msg;

				msg << properties.getMsgHead(/*error*/true);
				msg << (i+1) << ". link parameter '"  << sValue << "' can only be an single [folder:]<sburoutine>, so do not set this link";
				LOG(LOG_ERROR, msg.str());
				cout << msg.str() << endl;
			}
		}
		bWarning= false;
		if(nValue > 1)
			bWarning= true;
		sLinkWhile= properties.getValue("lwhile", bWarning);
		m_oLinkWhile.init(pStartFolder, sLinkWhile);
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
		m_oLinkWhile.activateObserver(observer);
	}

	void ValueHolder::setDebug(bool bDebug)
	{
		vector<ListCalculator*>::iterator it;

		m_oWhile.doOutput(bDebug);
		m_oLinkWhile.doOutput(bDebug);
		for(it= m_vpoLinks.begin(); it != m_vpoLinks.end(); ++it)
			(*it)->doOutput(bDebug);
		for(it= m_vpoValues.begin(); it != m_vpoValues.end(); ++it)
			(*it)->doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	bool ValueHolder::range(bool& bfloat, double* min, double* max)
	{
		if(m_bSetLinkObserver)
		{
			portBase* port;
			string folder(getFolderName()), subroutine(getSubroutineName());

			port= m_oLinkWhile.getSubroutine(m_vpoLinks[m_nLinkObserver]->getStatement(), /*own folder*/true);
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
		vector<string>::size_type links= m_vpoLinks.size();
		portBase* port= NULL;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas;

		//Debug info to stop by right subroutine
		/*string stopfolder("TRANSMIT_SONY");
		string stopsub("actual_step");
		if(	getFolderName() == stopfolder &&
			getSubroutineName() == stopsub	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << stopfolder << ":" << stopsub << endl;
		}*/
		value= actValue;
		if(links > 0)
		{
			bool bOk;
			vector<string>::size_type pos;
			ListCalculator* link;
			string slink;

			if(!m_oLinkWhile.isEmpty())
			{
				bOk= m_oLinkWhile.calculate(lvalue);
				if(bOk)
				{
					if(lvalue < 0)
						pos= 0;
					else if(lvalue > links-1)
						pos= links - 1;
					else
						pos= static_cast<vector<string>::size_type >(lvalue);
					link= m_vpoLinks[pos];
					slink= link->getStatement();
				}else
				{
					string msg("cannot create calculation from lwhile parameter '");

					msg+= m_oLinkWhile.getStatement();
					msg+= "' in folder " + folder + " and subroutine " + subroutine;
					TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg);
					if(isdebug)
						cout << "### ERROR: " << msg << endl;
					value= m_ddefaultValue;
					return false;
				}
				if(	slink != subroutine &&
					slink != foldersub		)
				{
					bOk= link->calculate(value);
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

						port= link->getSubroutine(slink, /*own folder*/true);
						if(isdebug)
						{
							cout << "take value from ";
							if(	m_dLastValue != value ||
								m_dLastValue == actValue	)
							{
								cout << "other subroutine " << slink;
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
							split(spl, slink, is_any_of(":"));
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

								if(	m_nLinkObserver < links	)
								{
									m_vpoLinks[m_nLinkObserver]->removeObserver(m_poObserver);
								}else
								{
									cout << "### ERROR: in " << folder << ":" << subroutine << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
								}
							}
							link->activateObserver(m_poObserver);
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
					if(isdebug)
						cout << pos+1 << ". link value '" << slink << "' link to own folder" << endl;
					if(m_bSetLinkObserver)
					{
						if(	m_nLinkObserver < links	)
						{
							m_vpoLinks[m_nLinkObserver]->removeObserver(m_poObserver);
							m_nLinkObserver= 0;
							m_bSetLinkObserver= false;
							defineRange();
						}else
						{
							cout << "### ERROR: in " << folder << ":" << subroutine;
							cout << " nLinkObserver was set to " << dec << m_nLinkObserver << endl;
						}
					}
				}
			}else
				slink= m_vpoLinks[0]->getStatement();
		}

		if( !bChanged &&
			!m_oWhile.isEmpty())
		{
			if(getWhileStringResult(folder, subroutine,
									m_oWhile, m_vpoValues, m_ddefaultValue, value, isdebug))
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

	bool ValueHolder::getWhileStringResult(const string& folder, const string& subroutine,
											ListCalculator& oWhile, vector<ListCalculator*>& content,
											const double defaultVal, double& value, const bool debug)
	{
		bool bOk;

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
		for(it= m_vpoLinks.begin(); it != m_vpoLinks.end(); ++it)
			delete *it;
	}
}
