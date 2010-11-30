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
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>

#include "output.h"

Output::Output(string folderName, string subroutineName)
: switchClass("DEBUG", folderName, subroutineName),
  m_bDebug(false)
{
}

Output::Output(string type, string folderName, string subroutineName)
: switchClass(type, folderName, subroutineName),
  m_bDebug(false)
{
}

bool Output::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	string svalue;
	string folder(getFolderName());
	string subroutine(getSubroutineName());
	vector<string>::size_type ncount;
	ListCalculator* calc;

	ncount= properties->getPropertyCount("before");
	for(vector<string>::size_type n= 0; n<ncount; ++n)
	{
		svalue= properties->getValue("before", n, /*warning*/false);
		m_vsBefore.push_back(svalue);
	}
	ncount= properties->getPropertyCount("value");
	for(vector<string>::size_type n= 0; n<ncount; ++n)
	{
		ostringstream val;

		val << "value[" << (n+1) << "]";
		svalue= properties->getValue("value", n, /*warning*/false);
		m_voVal.push_back(new ListCalculator(folder, subroutine, val.str(), false, false));
		calc= m_voVal.back();
		calc->init(pStartFolder, svalue);
	}
	m_sBehind= properties->getValue("behind", /*warning*/false);
	if(!switchClass::init(properties, pStartFolder))
		return false;
	return true;
}

void Output::setObserver(IMeasurePattern* observer)
{
	for(vector<ListCalculator*>::iterator it= m_voVal.begin(); it!=m_voVal.end(); ++it)
		(*it)->activateObserver(observer);
	switchClass::setObserver(observer);
}

double Output::measure(const double actValue)
{
	double output;

	output= switchClass::measure(actValue);
	if(	m_bDebug ||
		output > 0 ||
		output < 0		)
	{
		bool bOk(true);
		double val;
		ostringstream out;
		vector<string>::size_type ncount, ncountS, ncountV;

		out << "### " << getFolderName() << ":" << getSubroutineName() << " >> ";
		ncount= ncountS= m_vsBefore.size();
		ncountV= m_voVal.size();
		if(ncountV > ncountS)
			ncount= ncountV;
		for(vector<string>::size_type n= 0; n<ncount; ++n)
		{
			if(n < ncountS)
				out << m_vsBefore[n] << " ";
			if(	n < ncountV &&
				!m_voVal[n]->isEmpty()	)
			{
				if(!m_voVal[n]->calculate(val))
				{
					out << "(## wrong value ##)";
					bOk= false;

				}else
					out << val << " ";
			}
		}
		out << m_sBehind << endl;
		cout << out.str();
		if(!bOk)
		{
			for(vector<ListCalculator*>::iterator it= m_voVal.begin(); it!=m_voVal.end(); ++it)
			{
				if(	!(*it)->isEmpty() &&
					!(*it)->calculate(val)	)
				{
					(*it)->doOutput(true);
					(*it)->calculate(val);
					(*it)->doOutput(false);
				}
			}
			return 0;
		}
		return 1;
	}
	return 0;
}

void Output::setDebug(bool bDebug)
{
	m_bDebug= bDebug;
}

bool Output::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= 1;
	return true;
}

Output::~Output()
{
	for(vector<ListCalculator*>::iterator it= m_voVal.begin(); it!=m_voVal.end(); ++it)
		delete (*it);
}
