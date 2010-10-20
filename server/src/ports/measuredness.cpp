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
#include <limits.h>

#include <iostream>

#include "measuredness.h"
#include "switch.h"

#include "../logger/lib/LogInterface.h"

namespace ports
{

	bool Measuredness::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
	{
		string mvalue, begin;

		mvalue= properties.needValue("mvalue");
		begin= properties.needValue("begin");
		if(!portBase::init(properties))
			return false;
		if(	begin == ""
			||
			mvalue == ""	)
		{
			return false;
		}
		m_pStartFolder= pStartFolder;
		m_oBegin.init(pStartFolder, begin);
		m_oMeasuredValue.init(pStartFolder, mvalue);
		return true;
	}

	void Measuredness::setDebug(bool bDebug)
	{
		m_oMeasuredValue.doOutput(bDebug);
		m_oBegin.doOutput(bDebug);
		portBase::setDebug(bDebug);
	}

	double Measuredness::measure(const double actValue)
	{
		static double origValue= 0;
		//double dBegin;
		double mvalue;
		double diff;
		double value= actValue;
		string sfolder= getFolderName();
		double result;

		if(!m_oMeasuredValue.calculate(mvalue))
		{
			string msg("### ERROR: does not found mvalue '");

			msg+= m_oMeasuredValue.getStatement() + "'\n           in folder ";
			msg+= sfolder + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "measurednessresolve"+sfolder+getSubroutineName()+"mvalue", msg);
			mvalue= 0;
			return 0;
		}

		if(m_oBegin.calculate(result))
		{
			if(result < 0 || result > 0)
			{
				// calculate differenze
				if(mvalue < origValue)
					origValue= mvalue;
				diff= mvalue - origValue;
				if(value > diff)
					diff= value;
			}else
			{
				origValue= mvalue;
				diff= 0;
			}
		}else
		{
			string msg("           could not resolve parameter 'begin= ");

			msg+= m_oBegin.getStatement() + "'\n           in folder ";
			msg+= sfolder + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "measurednessresolve"+sfolder+getSubroutineName()+"begin", msg);
			value= 0;
		}
		return diff;
	}

	bool Measuredness::range(bool& bfloat, double* min, double* max)
	{
		bfloat= false;
		*min= 0;
		*max= (double)LONG_MAX;
		return true;
	}
}
