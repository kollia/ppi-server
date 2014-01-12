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

#include <algorithm>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../pattern/util/LogHolderPattern.h"

#include "ListCalculator.h"

using namespace boost;
using namespace design_pattern_world::util_pattern;

ListCalculator::ListCalculator(const string& folder, const string& subroutine, const string& param, bool need, bool boolean)
: CalculatorContainer(need, boolean),
  m_sFolder(folder),
  m_sSubroutine(subroutine),
  m_sParameter(param)
{
	allowComparison(true);
	allowIfSentence(true);
}

bool ListCalculator::init(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, string calcString)
{
	bool bOk;
	string::size_type len, newlen;

	m_pStartFolder= pStartFolder;
	if(m_pStartFolder == NULL)
		return false;
	replace_all(calcString, "\n", " ");
	replace_all(calcString, "\r", " ");
	replace_all(calcString, "\t", " ");
	newlen= calcString.size();
	do{
		len= newlen;
		replace_all(calcString, "  ", " ");
		newlen= calcString.size();
	}while(len != newlen);
	statement(calcString);
	bOk= render();
	if(calcString == "")
		return true;
	return bOk;
}

CalculatorContainer* ListCalculator::newObject()
{
	ListCalculator* c;

	c= new ListCalculator(m_sFolder, m_sSubroutine, m_sParameter, /*need*/true, false);
	c->m_pStartFolder= m_pStartFolder;
	for(map<string, double>::iterator it= m_msSubVars.begin(); it != m_msSubVars.end(); ++it)
		c->setSubVar(it->first, it->second);
	return c;
}

bool ListCalculator::render()
{
	bool output, bRv;

	output= doOutput();
	doOutput(true);
	bRv= CalculatorContainer::render();
	doOutput(output);
	return bRv;
}

bool ListCalculator::calculate(double& dResult)
{
	m_nLastChange= ppi_time();
	return CalculatorContainer::calculate(dResult);
}

void ListCalculator::output(bool bError, const string& file, const int line, const string& msg)
{
	if(!isRendered())
	{
		string err("rendering ERROR by folder '");

		err+= m_sFolder + "' and subroutine '" + m_sSubroutine + "' in parameter " + m_sParameter;
		tout << "### " << err << endl;
		tout << "    " << msg << endl;
		err+= "\n" + msg;
		LOG(LOG_ERROR, err);

	}else
	{
		string out(msg);

		if(out.substr(0, 11) == "calculate('")
			out= "calculate " + m_sParameter + " parameter " + out.substr(9);
		CalculatorContainer::output(bError, file, line, out);
	}
}

IListObjectPattern* ListCalculator::getSubroutine(const string& var, bool own)
{
	sub* oRv;

	oRv= getSubroutinePointer(var, own);
	if(oRv != NULL)
		return oRv->portClass.get();
	return NULL;
}

sub* ListCalculator::getSubroutinePointer(const string& var, bool own)
{
	string sFolder, sSubroutine, msg;
	vector<string> spl;
	SHAREDPTR::shared_ptr<measurefolder_t> pFolder= m_pStartFolder;

	split(spl, var, is_any_of(":"));
	if(spl.size() < 2)
	{
		if(!own)
			return NULL;
		sFolder= m_sFolder;
		sSubroutine= spl[0];

	}else
	{
		sFolder= spl[0];
		sSubroutine= spl[1];
	}
	trim(sFolder);
	if(	!own &&
		sFolder == getFolderName()	)
	{
		return NULL;
	}
	trim(sSubroutine);

	while(pFolder && pFolder->name != sFolder)
		pFolder= pFolder->next;
	if(pFolder)
	{
		for(vector<sub>::iterator it= pFolder->subroutines.begin(); it != pFolder->subroutines.end(); ++it)
		{
			if(	(	it->bCorrect ||
					!isRendered()	) &&
				it->name == sSubroutine	)
			{
				return &(*it);
			}
		}
	}
	if(isRendered())
	{
		msg=  "by folder '" + m_sFolder + "' and subroutine '" + m_sSubroutine + "' in parameter " + m_sParameter + ":\n";
		msg+= "cannot find ";
		if(pFolder)
		{
			msg+= "subroutine '" + sSubroutine + "' ";
			msg+= "for folder '" + sFolder + "' ";
		}else
		{
			msg+= "folder '" + sFolder + "' ";
			msg+= "with subroutine '" + sSubroutine + "' ";
		}
		msg+= "\ndefine value only as 0";
		TIMELOG(LOG_WARNING, m_sFolder+" "+m_sSubroutine+" "+m_sParameter+" "+sFolder+" "+sSubroutine, msg);
	}
	return NULL;
}

ppi_time ListCalculator::getLastChanging()
{
	return m_nLastChange;
}

bool ListCalculator::variable(const string& var, double& dResult)
{
	sub* oSub;
	string v;
	vector<string> spl;
	map<string, double>::iterator foundSub;
	map<string, sub*>::iterator found;
	ValueHolder result;

	if(m_msSubVars.size())
	{
		v= var;
		split(spl, var, is_any_of(":"));
		if(spl.size() == 1)
			v= m_sFolder+":"+var;
		foundSub= m_msSubVars.find(v);
		if(foundSub != m_msSubVars.end())
		{
			dResult= foundSub->second;
			return true;
		}
	}
	found= m_msoVars.find(var);
	if(found == m_msoVars.end())
	{
		oSub= getSubroutinePointer(var, /*own folder*/true);
		if(oSub)
		{
			m_msoVars[var]= oSub;
			if(oSub->bCorrect)
			{
				result= oSub->portClass->getValue("i:"+m_sFolder);
				if(result.lastChanging > m_nLastChange)
					m_nLastChange= result.lastChanging;
				dResult= result.value;
				return true;
			}
		}
	}else if(found->second->bCorrect)
	{
		result= found->second->portClass->getValue("i:"+m_sFolder);
		if(result.lastChanging > m_nLastChange)
			m_nLastChange= result.lastChanging;
		dResult= result.value;
		return true;
	}
	dResult= 0;
	return false;
}

void ListCalculator::activateObserver(IMeasurePattern* observer)
{
	vector<string> inform;
	IListObjectPattern *found, *own;

	if(isEmpty())
		return;
	inform= getVariables();
	for(vector<string>::const_iterator it= inform.begin(); it != inform.end(); ++it)
	{
		found= getSubroutine(*it, /*own folder*/false);
		if(!found)
		{	// when own subroutine (has lower count)
			// defined before the other (higher count) in same folder
			// changing from other should also inform own subroutine
			// because otherwise the case can be that own subroutine
			// do not know from any changes from the other, or to late
			found= getSubroutine(*it, /*own folder*/true);
			if(found)
			{
				own= getSubroutine(m_sFolder+":"+m_sSubroutine, /*own folder*/true);
				if(own->getActCount() > found->getActCount())
					found= NULL;
			}
		}
		if(found)
			found->informObserver(observer, m_sFolder, m_sSubroutine, m_sParameter);
	}
}

void ListCalculator::setSubVar(string var, const double* val)
{
	vector<string> spl;
	ListCalculator* container;
	vector<ICalculatorPattern*> childs;

	split(spl, var, is_any_of(":"));
	if(spl.size() == 1)
		var= m_sFolder+":"+var;
	if(val)
		m_msSubVars[var]= *val;
	else
		m_msSubVars.erase(var);
	childs= getChilds();
	for(vector<ICalculatorPattern*>::iterator it= childs.begin(); it != childs.end(); ++it)
	{
		container= dynamic_cast<ListCalculator*>(*it);
		container->setSubVar(var, val);
	}
}

void ListCalculator::removeObserver(IMeasurePattern* observer)
{
	vector<string> inform;
	IListObjectPattern* found;

	if(isEmpty())
		return;
	inform= getVariables();
	for(vector<string>::const_iterator it= inform.begin(); it != inform.end(); ++it)
	{
		found= getSubroutine(*it, /*own folder*/false);
		if(found)
			found->removeObserver(observer, m_sFolder, m_sSubroutine, m_sParameter);
	}
}
