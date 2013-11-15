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

#include "../pattern/util/LogHolderPattern.h"

#include "../util/thread/Thread.h"
#include "../util/thread/Terminal.h"

#include "output.h"

Output::Output(string folderName, string subroutineName)
: switchClass("DEBUG", folderName, subroutineName),
  m_nLogLevel(-1),
  m_bCL(true)
{
	m_DEBUG= Thread::getMutex("outputDEBUG");
}

Output::Output(string type, string folderName, string subroutineName)
: switchClass(type, folderName, subroutineName),
  m_nLogLevel(-1),
  m_bCL(true)
{
	m_DEBUG= Thread::getMutex("outputDEBUG");
}

bool Output::init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	bool bSet(false), bSwitch;
	string svalue;
	string folder(getFolderName());
	string subroutine(getSubroutineName());
	string sLogLevel;
	string on, sWhile, off;
	vector<string>::size_type ncount;
	ListCalculator* calc;

	m_bNeedSwitch= false;
	on= properties->getValue("begin", /*warning*/false);
	sWhile= properties->getValue("while", /*warning*/false);
	off= properties->getValue("end", /*warning*/false);
	if(	on != "" ||
		sWhile != "" ||
		off != ""		)
	{
		m_bNeedSwitch= true;
	}
	properties->haveAction("print");// ask only because when no log property set but action 'print'
	sLogLevel= properties->getValue("log", /*warning*/false);  // should gives no warning output
	if(sLogLevel != "")
	{
		m_bCL= properties->haveAction("print");
		if(sLogLevel =="DEBUG")
			m_nLogLevel= LOG_DEBUG;
		else if(sLogLevel =="INFO")
			m_nLogLevel= LOG_INFO;
		else if(sLogLevel =="WARNING")
			m_nLogLevel= LOG_WARNING;
		else if(sLogLevel =="ERROR")
			m_nLogLevel= LOG_ERROR;
		else if(sLogLevel =="ALERT")
			m_nLogLevel= LOG_ALERT;
	}else
		m_bCL= true;
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
	properties->notAllowedAction("binary");
	bSwitch= switchClass::init(properties, pStartFolder);
	if(!bSet)
	{
		string msg(properties->getMsgHead(/*error*/false));

		msg+= "no string or value be set for output, set subroutine to incorrect";
		LOG(LOG_WARNING, msg);
		tout << msg << endl;
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

void Output::setDebug(bool bDebug)
{
	if(getRunningThread()->isDebug())	// when also set other subroutine(s) for debug output
		switchClass::setDebug(bDebug);	// output also debug strings from switch class (parent objects)
	else
		switchClass::setDebug(false);// maybe parent object was before true
	LOCK(m_DEBUG);
	m_bDebug= bDebug;
	UNLOCK(m_DEBUG);
}

bool Output::isDebug()
{
	bool bRv;

	if(Thread::gettid() == getRunningThread()->getRunningThreadID())
		return switchClass::isDebug();	// when own parent class ask for debug
	LOCK(m_DEBUG);						// write debug output only when hole subroutine
	bRv= m_bDebug;						// (also other subroutine(s) set for debug)
	UNLOCK(m_DEBUG);					// be set. Otherwise measureThread object set from outside
	return bRv;							// debug session and should know whether this object was set to debug
}

double Output::measure(const double actValue)
{
	bool bDebug(isDebug()), ownDebug;
	double output(0);

	LOCK(m_DEBUG);
	ownDebug= m_bDebug;
	UNLOCK(m_DEBUG);
	if(m_bNeedSwitch)
		output= switchClass::measure(actValue);
	if(	output > 0 ||
		output < 0 ||
		(	ownDebug &&
			!m_bNeedSwitch	)	)
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
		if(	m_bCL ||
			ownDebug	)
		{
			tout << out.str() << endl;
		}
		if(m_nLogLevel > -1)
			LOGEX(m_nLogLevel, out.str(), getRunningThread()->getExternSendDevice());
		output= 1;
	}else
		output= 0;
	if(bDebug)
	{
		if(!output)
			tout << "do not write any output" << endl;
		tout << "result of DEBUG output is " << output << endl;
	}
	return output;
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
	DESTROYMUTEX(m_DEBUG);
}
