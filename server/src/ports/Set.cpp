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

#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Set.h"

#include "../util/thread/Terminal.h"

#include "../pattern/util/LogHolderPattern.h"

using namespace boost;
using namespace boost::algorithm;

namespace ports
{
	bool Set::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		bool bAllFault(true);
		ValueHolder oDefault;
		string prop, sFrom, sSet;
		string folderName(getFolderName());
		string subroutineName(getSubroutineName());
		vector<string> spl;
		vector<string>::size_type nFrom, nSet;
		ListCalculator* calc;
		ostringstream seterr;

		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "weather_timecheck" &&
			getSubroutineName() == "set_back"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
		nFrom= properties->getPropertyCount("from");
		nSet= properties->getPropertyCount("set");
		sFrom= properties->needValue("from");
		properties->needValue("set"); // only to create error message
		if(	nFrom > 0 &&
			nSet > 0	)
		{
			m_vpoFrom.push_back(
					new ListCalculator(folderName, subroutineName, "from[1]", true, false)	);
			calc= m_vpoFrom.back();
			calc->init(pStartFolder, sFrom);
			for(size_t n= 0; n<nSet; ++n)
			{
				sSet= properties->getValue("set", n, /*warning*/true);
				split(spl, sSet, is_any_of(":"));
				if(spl.size() > 0)
					trim(spl[0]);
				if(spl.size() == 2)
					trim(spl[1]);
				if(	(	spl.size() == 1 &&
						spl[0].find(" ") != string::npos	) ||
					(	spl.size() == 2 &&
						(	spl[0].find(" ") != string::npos ||
							spl[1].find(" ") != string::npos	)	)	)
				{
					if(seterr.str() != "")
						seterr << "           ";
					seterr << (n+1) << ". set parameter '"  << sSet;
					seterr << "' can only be an single [folder:]<sburoutine>." << endl;
					seterr << "           Do not set any value in this subroutine." << endl;
					LOG(LOG_ERROR, seterr.str());

				}else
				{
					IListObjectPattern* port;

					port= m_vpoFrom[0]->getSubroutine(sSet, /*own folder*/true);
					if(!port)
					{
						if(seterr.str() != "")
							seterr << "           ";
						seterr << (n+1) << ". set parameter '"  << sSet;
						seterr << " do not exist." << endl;
						LOG(LOG_ERROR, seterr.str());
					}else
						bAllFault= false;
					m_vsSet.push_back(sSet);
				}
			}
			if(	nFrom != 1 &&
				nFrom != m_vsSet.size()	)
			{
				ostringstream msg;

				msg << "by setting more 'from' parameter (" << nFrom << ") than one, same count of 'set' ";
				msg << "parameter (" << m_vsSet.size() << ") have to exist. Set subroutine to incorrect!";
				LOG(LOG_ERROR, properties->getMsgHead(/*error*/true) + msg.str());
				if(seterr.str() != "")
					seterr << "           ";
				seterr << msg.str() << endl;
				nFrom= 0;
				nSet= 0;
				bAllFault= true;

			}else
			{
				for(vector<string>::size_type n=1; n<nFrom; ++n)
				{
					ostringstream from;

					from << "from[" << (n+1) << "]";
					sFrom= properties->getValue("from", n, /*warning*/true);
					m_vpoFrom.push_back(
							new ListCalculator(folderName, subroutineName, from.str(), true, false) );
					calc= m_vpoFrom.back();
					calc->init(pStartFolder, sFrom);
				}
			}
		}
		if(seterr.str() != "")
			tout << properties->getMsgHead(/*error*/true) << seterr.str() << endl;
		prop= "default";
		oDefault.value= properties->getDouble(prop, /*warning*/false);
		properties->notAllowedAction("binary");
		if(	!switchClass::init(properties, pStartFolder) ||
			nFrom == 0 ||
			nSet == 0										)
		{
			return false;
		}
		if(bAllFault)
			return false;
		setValue(oDefault, "i:"+getFolderName()+":"+getSubroutineName());
		return true;
	}

	bool Set::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= 1;
		return true;
	}

	IValueHolderPattern& Set::measure(const ppi_value& actValue)
	{
		bool bOk, isdebug= isDebug();
		double swRv;
		ValueHolder oValue;
		string folder(getFolderName());
		string subroutine(getSubroutineName());
		vector<string>::size_type startSet(0), endSet(m_vsSet.size()-1);
		vector<ListCalculator*>::size_type nFrom(m_vpoFrom.size());
		IListObjectPattern* port;

		//Debug info to stop by right subroutine
		/*if(	getFolderName() == "set_probe" &&
			getSubroutineName() == "stop"	)
		{
			cout << __FILE__ << __LINE__ << endl;
			cout << getFolderName() << ":" << getSubroutineName() << endl;
		}*/
		swRv= switchClass::measure(actValue).getValue();
		if(	swRv > 0 ||
			swRv < 0		)
		{
			for(vector<ListCalculator*>::size_type n= 0; n<nFrom; ++n)
			{
				bOk= m_vpoFrom[n]->calculate(oValue.value);
				if(bOk)
				{
					oValue.lastChanging= m_vpoFrom[n]->getLastChanging();
					if(	oValue.lastChanging.isSet() &&
						(	!m_oMeasureValue.lastChanging.isSet() ||
							m_oMeasureValue.lastChanging < oValue.lastChanging	)	)
					{
						m_oMeasureValue.lastChanging= oValue.lastChanging;
					}
					if(nFrom > 1)
						startSet= endSet= n;
					for(vector<string>::size_type s= startSet; s<=endSet; ++s)
					{
						port= m_vpoFrom[0]->getSubroutine(m_vsSet[s], /*own folder*/true);
						if(port)
						{
							if(isdebug)
							{
								tout << "set value " << oValue.value;
								tout << " into " << (s+1) << ". subroutine '";
								if(m_vsSet[s].find(":") == string::npos)
									tout << folder << ":";
								tout << m_vsSet[s] << "'" << endl;
							}
							port->setValue(oValue, "i:"+folder+":"+subroutine);
						}else
						{
							ostringstream msg;

							msg << "cannot set value " << oValue.value << " into given ";
							msg << (s+1) << ". 'set' parameter '";
							if(m_vsSet[s].find(":") == string::npos)
								msg << folder << ":";
							msg << m_vsSet[s] << "'";
							if(isdebug)
								tout << "### ERROR: " << msg.str() << endl;
							msg << endl << "do not found this subroutine from 'set' attribute" << endl;
							msg << "from set parameter in folder " << folder << " and subroutine " << subroutine;
							TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg.str());
						}
					}
				}else
				{
					string msg("cannot create calculation from 'from' parameter '");

					msg+= m_vpoFrom[n]->getStatement();
					msg+= "' in folder " + folder + " and subroutine " + subroutine;
					TIMELOG(LOG_ERROR, "calcResult"+folder+":"+subroutine, msg);
					if(isdebug)
						tout << "### ERROR: " << msg << endl;
				}
			}

		}
		if(isdebug)
		{
			tout << "result of subroutine is ";
			if(	swRv > 0 ||
				swRv < 0		)
			{
				tout << "TRUE";

			}else
				tout << "FALSE";
			tout << endl;
		}
		m_oMeasureValue.value= swRv;
		return m_oMeasureValue;
	}

	void Set::setDebug(bool bDebug)
	{
		for(vector<ListCalculator*>::iterator it= m_vpoFrom.begin(); it!= m_vpoFrom.end(); ++it)
			(*it)->doOutput(bDebug);
		switchClass::setDebug(bDebug);
	}

	Set::~Set()
	{
		for(vector<ListCalculator*>::iterator it= m_vpoFrom.begin(); it!= m_vpoFrom.end(); ++it)
			delete *it;
	}

}
