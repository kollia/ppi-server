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

#include "../logger/lib/LogInterface.h"

#include "../util/thread/Thread.h"

#include "output.h"

Output::Output(string folderName, string subroutineName)
: switchClass("DEBUG", folderName, subroutineName)
{
}

Output::Output(string type, string folderName, string subroutineName)
: switchClass(type, folderName, subroutineName)
{
}

bool Output::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bSet(false), bSwitch;
	string svalue;
	string folder(getFolderName());
	string subroutine(getSubroutineName());
	vector<string>::size_type ncount;
	ListCalculator* calc;

	ncount= properties->getPropertyCount("string");
	if(ncount > 0)
		bSet= true;
	for(vector<string>::size_type n= 0; n<ncount; ++n)
	{
		svalue= properties->getValue("string", n, /*warning*/false);
		m_vsStrings.push_back(svalue);
	}
	ncount= properties->getPropertyCount("value");
	if(ncount > 0)
		bSet= true;
	for(vector<string>::size_type n= 0; n<ncount; ++n)
	{
		ostringstream val;

		val << "value[" << (n+1) << "]";
		svalue= properties->getValue("value", n, /*warning*/false);
		m_voVal.push_back(new ListCalculator(folder, subroutine, val.str(), false, false));
		calc= m_voVal.back();
		calc->init(pStartFolder, svalue);
	}
	bSwitch= switchClass::init(properties, pStartFolder);
	if(!bSet)
	{
		string msg(properties->getMsgHead(/*error*/false));

		msg+= "no string or value be set for output, set subroutine to incorrect";
		LOG(LOG_WARNING, msg);
		cout << msg << endl;
		return false;
	}
	if(!bSwitch)
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
	bool bDebug(isDebug());
	double output(0);

	if(!bDebug)
		output= switchClass::measure(actValue);
	if(	bDebug ||
		output > 0 ||
		output < 0		)
	{
		double val;
		ostringstream out;
		vector<ListCalculator*>::iterator itVal;

		itVal= m_voVal.begin();
		out << "### " << getFolderName() << ":" << getSubroutineName() << " >> ";
		for(vector<string>::iterator itStr= m_vsStrings.begin(); itStr != m_vsStrings.end(); ++itStr)
		{
			out << *itStr << " ";
			if(itVal != m_voVal.end())
			{
				if( !(*itVal)->isEmpty() &&
					(*itVal)->calculate(val)	)
				{
					out << val << " ";
				}else
					out << "(## wrong value ##) ";
				++itVal;
			}
		}
		for(itVal= itVal; itVal != m_voVal.end(); ++itVal)
		{
			if( !(*itVal)->isEmpty() &&
				(*itVal)->calculate(val)	)
			{
				out << val << " ";
			}else
				out << "(## wrong value ##) ";
		}
		cout << out.str() << endl;
		return 1;
	}
	return 0;
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
