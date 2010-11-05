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

//#include <boost/algorithm/string/classification.hpp>
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

bool ListCalculator::init(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& calcString)
{
	bool bOk;

	m_pStartFolder= pStartFolder;
	if(m_pStartFolder == NULL)
		return false;
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

void ListCalculator::output(bool bError, const string& file, const int line, const string& msg)
{
	if(!isRendered())
	{
		string err("rendering ERROR by folder '");

		err+= m_sFolder + "' and subroutine '" + m_sSubroutine + "' in parameter " + m_sParameter;
		cerr << "### " << err << endl;
		cerr << "    " << msg << endl;
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
			if(	it->bCorrect &&
				it->name == sSubroutine	)
			{
				return it->portClass.get();
			}
		}
	}
	if(isRendered())
	{
		msg= "cannot found folder '";
		msg+= sFolder + "' with subroutine '" + sSubroutine;
		msg+= "' defined in folder " + m_sFolder + " and subroutine ";
		msg+= m_sSubroutine + " by parameter " + m_sParameter;
		LOG(LOG_ERROR, msg);
	}
	return NULL;
}

bool ListCalculator::variable(const string& var, double& dResult)
{
	IListObjectPattern* oSub;
	string v;
	vector<string> spl;
	map<string, double>::iterator foundSub;
	map<string, IListObjectPattern*>::iterator found;

	if(!isRendered())
		return true;
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
		oSub= getSubroutine(var, /*own folder*/true);
		if(oSub)
		{
			m_msoVars[var]= oSub;
			dResult= oSub->getValue("i:"+m_sFolder);
			return true;
		}
	}else
	{
		dResult= found->second->getValue("i:"+m_sFolder);
		return true;
	}
	return false;
}

void ListCalculator::activateObserver(IMeasurePattern* observer)
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
			found->informObserver(observer, m_sFolder, m_sSubroutine, m_sParameter);
	}
}

void ListCalculator::setSubVar(string var, const double val)
{
	vector<string> spl;

	split(spl, var, is_any_of(":"));
	if(spl.size() == 1)
		var= m_sFolder+":"+var;
	m_msSubVars[var]= val;
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
