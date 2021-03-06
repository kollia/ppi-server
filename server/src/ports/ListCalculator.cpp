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
#include "SubroutineSubVarHolder.h"

using namespace boost;
using namespace design_pattern_world::util_pattern;

ListCalculator::ListCalculator(const string& folder, const string& subroutine, const string& param,
				bool need, bool boolean, IListObjectPattern* obj)
: CalculatorContainer(need, boolean),
  m_sFolder(folder),
  m_sSubroutine(subroutine),
  m_bIfSentence(false),
  m_bUseIfSentenceTime(true),
  m_bCurrentTimeByNormalNumberCalculation(true),
  m_bCurrentTimeByOnlyVariableCalculation(true),
  m_sFolderSub(folder+":"+subroutine),
  m_sParameter(param),
  m_oOutput(obj)
{
	m_CALCUALTEMUTEX= Thread::getMutex("CALCUALTEMUTEX");
	allowComparison(true);
	allowIfSentence(true);
}

bool ListCalculator::init(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, string calcString)
{
	bool bOk;
	string inp;
	vector<string> spl;
	string::size_type len, newlen;
	vector<string>::size_type nAliasCount;
	SHAREDPTR::shared_ptr<measurefolder_t> pfolder;

	if(m_sFolder == "Raff1_Zeit_timer")
	{
		cout << flush;
		if(m_sSubroutine == "informe_thread")
			cout << flush;
	}
	m_pStartFolder= pStartFolder;
	if(m_pStartFolder == NULL)
		return false;
	pfolder= pStartFolder;
	while(pfolder != NULL)
	{
		if(pfolder->name == m_sFolder)
			break;
		pfolder= pfolder->next;
	}
	if(pfolder == NULL)
		return false;
	m_nArrayFolderID= pfolder->nFolderArrayID;
	nAliasCount= pfolder->folderProperties->getPropertyCount("foldervar");
	for(vector<string>::size_type n= 0; n < nAliasCount; ++n)
	{
		inp= pfolder->folderProperties->getValue("foldervar", n);
		split(spl, inp, is_any_of("="));
		if(spl.size() == 2)
		{// when not set correct alias, write error inside folder configuration by starting
			trim(spl[0]);
			trim(spl[1]);
			m_mFolderAlias.insert(pair<string, string>(spl[0], spl[1]));
		}
	}
	nAliasCount= pfolder->folderProperties->getPropertyCount("subvar");
	for(vector<string>::size_type n= 0; n < nAliasCount; ++n)
	{
		inp= pfolder->folderProperties->getValue("subvar", n);
		split(spl, inp, is_any_of("="));
		if(spl.size() == 2)
		{// when not set correct alias, write error inside folder configuration by starting
			trim(spl[0]);
			trim(spl[1]);
			m_mSubroutineAlias.insert(pair<string, string>(spl[0], spl[1]));
		}
	}
	inp= "calculate " + m_sParameter + " parameter ('";
	len= inp.size();
	split(spl, calcString, is_any_of("\n"));
	for(vector<string>::iterator it= spl.begin(); it != spl.end(); ++it)
	{
		if(inp == "")
			inp.append(len, ' ');
		inp+= *it;
		if((it + 1) == spl.end())
			inp+= "')";
		inp+= "\n";
		m_vsStatement.push_back(inp);
		inp= "";
	}
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

CalculatorContainer* ListCalculator::newObject(bool bIf/*= false*/)
{
	ListCalculator* c;

	c= new ListCalculator(m_sFolder, m_sSubroutine, m_sParameter, /*need*/true, false, m_oOutput);
	if(bIf)
		c->m_bIfSentence= true;
	else
		c->m_bIfSentence= m_bIfSentence;
	c->m_nArrayFolderID= m_nArrayFolderID;
	c->m_pStartFolder= m_pStartFolder;
	c->m_mFolderAlias= m_mFolderAlias;
	c->m_mSubroutineAlias= m_mSubroutineAlias;
	for(map<string, double>::iterator it= m_msSubVars.begin(); it != m_msSubVars.end(); ++it)
		c->setSubVar(it->first, it->second);
	return c;
}

bool ListCalculator::render()
{
	bool output, bRv;

	output= CalculatorContainer::doOutput();
	doOutput(true);
	bRv= CalculatorContainer::render();
	doOutput(output);
	return bRv;
}

void ListCalculator::clearTime()
{
	ListCalculator* container;
	vector<ICalculatorPattern*> childs;

	m_nLastChange.clear();
	m_sLastChangingSub= "";
	childs= getChilds();
	for(vector<ICalculatorPattern*>::iterator it= childs.begin(); it != childs.end(); ++it)
	{
		container= dynamic_cast<ListCalculator*>(*it);
		container->clearTime();
	}
}

bool ListCalculator::calculate(double& dResult)
{
	bool bRv;

	LOCK(m_CALCUALTEMUTEX);
	clearTime();
	m_bCalcMade= false;
	m_bCalcWithNumber= false;
	m_bCalcWithVariable= false;
	bRv= CalculatorContainer::calculate(dResult);
	UNLOCK(m_CALCUALTEMUTEX);
	return bRv;
}

void ListCalculator::calcMade(bool num, bool var)
{
	m_bCalcMade= true;
	m_bCalcWithNumber= num;
	m_bCalcWithVariable= var;
}

void ListCalculator::output(bool bError, const string& file, const int line, const string& msg)
{
	if(!isRendered())
	{
		string err("rendering ERROR by folder '");

		err+= m_sFolder + "' ";
		if(m_sSubroutine != "#informe_thread")
			err+= "and subroutine '" + m_sSubroutine + "' ";
		err+= "in parameter " + m_sParameter;
		m_oOutput->out() << "### " << err << endl;
		m_oOutput->out() << "    " << msg << endl;
		err+= "\n" + msg;
		LogHolderPattern::instance()->log(file, line, LOG_ERROR, err);

	}else
	{
		string out(msg);

		if(out.substr(0, 11) == "calculate('")
		{
			for(vector<string>::iterator it= m_vsStatement.begin(); it != m_vsStatement.end(); ++it)
				CalculatorContainer::output(bError, file, line, *it);
		}else
			CalculatorContainer::output(bError, file, line, out);
	}
}

string ListCalculator::getOriginalFolderName(const string& folder)
{
	string sRv;
	map<string, string>::iterator found;

	// change folder to right name when name is an foldervar alias
	found= m_mFolderAlias.find(folder);
	if(found != m_mFolderAlias.end())
		sRv= found->second;
	else
		sRv= folder;
	return sRv;
}
SHAREDPTR::shared_ptr<measurefolder_t> ListCalculator::getFolder(const string& sFolder, unsigned short nArrayFolder)
{
	SHAREDPTR::shared_ptr<measurefolder_t> pFolder= m_pStartFolder;

	while(	pFolder &&
			pFolder->sFolderArray != sFolder &&
			pFolder->name != sFolder		)
	{// search right folder
		pFolder= pFolder->next;
	}
	if(	pFolder &&
		nArrayFolder > 0 &&
		pFolder->sFolderArray == sFolder	)
	{// folder is object name, search right count
		while(	pFolder &&
				pFolder->sFolderArray == sFolder &&
				pFolder->nFolderArrayID != nArrayFolder	)
		{// search right count
			pFolder= pFolder->next;
		}
		if(pFolder != NULL)
		{
			if(	pFolder->sFolderArray != sFolder ||
				pFolder->nFolderArrayID != nArrayFolder	)
			{
				pFolder= SHAREDPTR::shared_ptr<measurefolder_t>();
			}
		}
	}
	return pFolder;
}

SHAREDPTR::shared_ptr<IListObjectPattern> ListCalculator::getSubroutine(string* var, unsigned short nObjFolder, bool own)
{
	bool bHasFolder(false);
	bool bfoundSubroutine(false);
	bool bFolderHasSubvar(false);
	bool bSubroutineHasSubvar(false);
	string sFolder, sSubroutine, sSubVar, msg;
	vector<string> spl;
	map<string, string>::iterator found;
	SHAREDPTR::shared_ptr<measurefolder_t> pFolder, pAmbiguousFoulder;
	map<string, SHAREDPTR::shared_ptr<IListObjectPattern> >::iterator exist;
	SHAREDPTR::shared_ptr<IListObjectPattern> holder;

	split(spl, *var, is_any_of(":"));
	if(spl.size() < 2)
	{
		if(!own)
			return SHAREDPTR::shared_ptr<IListObjectPattern>();
		sFolder= m_sFolder;
		sSubroutine= spl[0];

	}else
	{
		bHasFolder= true;
		sFolder= spl[0];
		sSubroutine= spl[1];
	}
	trim(sFolder);
	trim(sSubroutine);
	split(spl, sSubroutine, is_any_of("."));
	if(spl.size() > 1)
	{
		sSubroutine= spl[0];
		sSubVar= spl[1];
		trim(sSubroutine);
		trim(sSubVar);
	}else
		sSubVar= "";// take value of subroutine

	sFolder= getOriginalFolderName(sFolder);
	if(	!own &&
		sFolder == m_sFolder	)
	{
		return SHAREDPTR::shared_ptr<IListObjectPattern>();
	}
	// change subroutine to right name when name is an subvar alias
	found= m_mSubroutineAlias.find(sSubroutine);
	if(found != m_mSubroutineAlias.end())
	{
		sSubroutine= found->second;
	}
	// create correct variable
	if(bHasFolder)
		*var= sFolder + ":" + sSubroutine;
	else
		*var= sSubroutine;
	if(sSubVar != "")
		*var+= "." + sSubVar;
	exist= m_msoVars.find(*var);
	if(exist != m_msoVars.end())
	{
		/*
		 * when folder searched before
		 * do not select subroutine again
		 * from working list
		 * or create in second time
		 * an SubroutineSubVarHolder object
		 */
		return exist->second;
	}

	if( !bHasFolder &&
		sSubVar != ""	)
	{
		/**
		 * when inside calculation string
		 * only an variable without an colon defined,
		 * check also whether this an folder with sub-variable
		 * and give by rendering an WARNING
		 */
		pAmbiguousFoulder= getFolder(sSubroutine, nObjFolder);
		if(	pAmbiguousFoulder &&
			pAmbiguousFoulder->runThread->hasSubVar(sSubVar)	)
		{
			bFolderHasSubvar= true;
		}
	}
	pFolder= getFolder(sFolder, nObjFolder);
	if(pFolder)
	{
		for(vector<sub>::iterator it= pFolder->subroutines.begin(); it != pFolder->subroutines.end(); ++it)
		{
			if(	(	it->bCorrect ||
					!isRendered()	) &&
				it->name == sSubroutine	)
			{
				bfoundSubroutine= true;
				if(sSubVar == "")
					return it->portClass;
				if(it->portClass->hasSubVar(sSubVar))
				{
					bSubroutineHasSubvar= true;
					if(	!bFolderHasSubvar ||   // when the list object has also the sub-variable
						bHasFolder			)  // and no folder in the calculation string be given
					{                          // refer to the list object and write by rendering an WARNING
						holder= SHAREDPTR::shared_ptr<IListObjectPattern>(
										new ports::SubroutineSubVarHolder(it->portClass, sSubVar));
						if(sSubVar == "changed")
						{
							/*
							 * by object has to know changing
							 * write object also into running subroutine
							 * because after every running
							 * subroutine has to actualize changed values
							 */
							m_oOutput->setChangedSubVar(holder);
						}
						m_vNewSubObjs.push_back(holder);
						return holder;
					}
				}
				break;
			}
		}
		if(bFolderHasSubvar)
		{
			holder= SHAREDPTR::shared_ptr<IListObjectPattern>(
							new ports::SubroutineSubVarHolder(pAmbiguousFoulder->runThread, sSubVar));
			m_vNewSubObjs.push_back(holder);
			if(!bSubroutineHasSubvar)
				return holder;
		}
	}
	if(isRendered())
	{
		msg=  "by folder '" + m_sFolder + "' and subroutine '" + m_sSubroutine + "' in parameter " + m_sParameter + ":\n";
		if(pFolder)
		{
			if(bfoundSubroutine)
			{
				if(	bSubroutineHasSubvar &&
					bFolderHasSubvar		)
				{
					msg+= "found ambiguous sub-variable '" + sSubVar + "',\n";
					msg+= "because subroutine '" + sSubroutine + "' of this folder";
					msg+= " is also an own folder\n";
					msg+= "and both has the same sub-variable.\n";
					msg+= "Take always the sub-variable of the folder.";
				}else
				{
					msg+= "cannot find ";
					msg+= "sub-variable '" + sSubVar + "' ";
					msg+= "for folder:subroutine '" + sFolder + ":" + sSubroutine + "' ";
					msg+= "\ndefine value only as 0";
				}
			}else
			{
				msg+= "cannot find ";
				msg+= "subroutine '" + sSubroutine + "' ";
				msg+= "for folder '" + sFolder + "' ";
				msg+= "\ndefine value only as 0";
			}
		}else
		{
			msg+= "cannot find ";
			msg+= "folder '" + sFolder + "' ";
			msg+= "where subroutine '" + sSubroutine + "' should running";
			msg+= "\ndefine value only as 0";
		}
		TIMELOG(LOG_WARNING, m_sFolder+" "+m_sSubroutine+" "+m_sParameter+" "+sFolder+" "+sSubroutine, msg);
	}
	return holder;
}

ppi_time ListCalculator::getLastChanging()
{
	ppi_time nRv;

	LOCK(m_CALCUALTEMUTEX);
	nRv= getLastChangingI();
	if(	CalculatorContainer::doOutput() &&
		m_oOutput->needChangingTime()		)
	{
		if(nRv.isSet())
		{
			if(m_sLastChangingSub == "##CalcWithNumber")
			{
				m_oOutput->out() << "create current time " << nRv.toString(/*as date*/true);
				m_oOutput->out() << " for last changing because an calculation with numbers was made" << endl;

			}else if(m_sLastChangingSub == "##Calculation")
			{
				m_oOutput->out() << "create current time " << nRv.toString(/*as date*/true);
				m_oOutput->out() << " for last changing because an calculation was made" << endl;
			}
			m_oOutput->out() << "take last changing time " << nRv.toString(/*as date*/true);
			m_oOutput->out() << " from " << m_sLastChangingSub << endl;
		}else
			m_oOutput->out() << "no last changing time be set" << endl;
	}
	UNLOCK(m_CALCUALTEMUTEX);
	return nRv;
}

ppi_time ListCalculator::getLastChangingI()
{
	ppi_time lastChange;
	ListCalculator* container;
	vector<ICalculatorPattern*> childs;

	if(	m_bIfSentence &&
		!m_bUseIfSentenceTime	)
	{
		return lastChange;
	}
	if(	m_bCalcMade &&
		!m_bIfSentence	)
	{
		bool bUse(false);

		/*
		 * when inside object an calculation was made
		 * and it is allowed from user
		 * create current time as last changing
		 */
		if(	m_bCurrentTimeByNormalNumberCalculation &&
			m_bCalcWithNumber				)
		{
			bUse= true;
			if(CalculatorContainer::doOutput())
			{
				if(	m_bCurrentTimeByOnlyVariableCalculation &&
					m_bCalcWithVariable							)
				{
					m_sLastChangingSub= "##Calculation";
				}else
					m_sLastChangingSub= "##CalcWithNumber";
			}

		}else if(	m_bCurrentTimeByOnlyVariableCalculation &&
					m_bCalcWithVariable &&
					!m_bCalcWithNumber			)
		{
			bUse= true;
			if(CalculatorContainer::doOutput())
				m_sLastChangingSub= "##Calculation";
		}
		if(bUse)
		{
			if(!m_nLastChange.setActTime())
			{
				m_oOutput->out() << "cannot make current time for " << m_sFolder << ":" << m_sSubroutine;
				m_oOutput->out() << " by calculation" << endl;
				m_oOutput->out() << "(" << m_nLastChange.errorStr() << ")" << endl;
			}
			return m_nLastChange;
		}
	}
	childs= getChilds();
	for(vector<ICalculatorPattern*>::iterator it= childs.begin(); it != childs.end(); ++it)
	{
		container= dynamic_cast<ListCalculator*>(*it);
		lastChange= container->getLastChangingI();
		if(	lastChange.isSet() &&
			(	lastChange > m_nLastChange ||
				!m_nLastChange.isSet()			)	)
		{
			m_nLastChange= lastChange;
			if(CalculatorContainer::doOutput())
				m_sLastChangingSub= container->m_sLastChangingSub;
		}
	}
	return m_nLastChange;
}

bool ListCalculator::variable(string* var, double& dResult)
{
	SHAREDPTR::shared_ptr<IListObjectPattern> oSub;
	string v;
	vector<string> spl;
	map<string, double>::iterator foundSub;
	map<string, SHAREDPTR::shared_ptr<IListObjectPattern> >::iterator found;
	ValueHolder result;

	if(m_msSubVars.size())
	{
		v= *var;
		split(spl, *var, is_any_of(":"));
		if(spl.size() == 1)
			v= m_sFolder+":"+*var;
		foundSub= m_msSubVars.find(v);
		if(foundSub != m_msSubVars.end())
		{
			dResult= foundSub->second;
			return true;
		}
	}
	found= m_msoVars.find(*var);
	if(found == m_msoVars.end())
	{
		oSub= getSubroutine(var, m_nArrayFolderID, /*own folder*/true);
		if(oSub != NULL)
		{
			m_msoVars[*var]= oSub;
			result= oSub->getValue(
							InformObject(InformObject::INTERNAL, m_sFolderSub));
			if(	result.lastChanging.isSet() &&
				(	result.lastChanging > m_nLastChange ||
					!m_nLastChange.isSet()					)	)
			{
				m_nLastChange= result.lastChanging;
				if(CalculatorContainer::doOutput())
					m_sLastChangingSub= oSub->getFolderName() + ":" + oSub->getSubroutineName();
			}
			dResult= result.value;
			return true;
		}
	}else
	{
		if(found->second)
		{
			result= found->second->getValue(
							InformObject(InformObject::INTERNAL, m_sFolderSub));
			if(	result.lastChanging.isSet() &&
				(	result.lastChanging > m_nLastChange ||
					!m_nLastChange.isSet()					)	)
			{
				m_nLastChange= result.lastChanging;
				if(CalculatorContainer::doOutput())
					m_sLastChangingSub= found->second->getFolderName()
					            + ":" + found->second->getSubroutineName();
			}
			dResult= result.value;
			return true;
		}
	}
	dResult= 0;
	return false;
}

void ListCalculator::activateObserver(IMeasurePattern* observer)
{
	vector<string> inform;
	SHAREDPTR::shared_ptr<IListObjectPattern> found, own;

	if(isEmpty())
		return;
	LOCK(m_CALCUALTEMUTEX);
	inform= getVariables();
	for(vector<string>::const_iterator it= inform.begin(); it != inform.end(); ++it)
	{
		string var(*it);

		found= getSubroutine(&var, m_nArrayFolderID, /*own folder*/false);
		if(!found)
		{	// when own subroutine (has lower count)
			// defined before the other (higher count) in same folder
			// changing from other should also inform own subroutine
			// because otherwise the case can be that own subroutine
			// do not know from any changes from the other, or to late
			if(var == "run_automatic_actions.run")
				cout << flush;
			found= getSubroutine(&var, m_nArrayFolderID, /*own folder*/true);
			if(found)
			{
				var= m_sFolder+":"+m_sSubroutine;
				own= getSubroutine(&var, m_nArrayFolderID, /*own folder*/true);
				if(own->getActCount() > found->getActCount())
					found= SHAREDPTR::shared_ptr<IListObjectPattern>();
			}
		}
		if(found != NULL)
			found->informObserver(observer, m_sFolder, m_sSubroutine, m_sParameter);
	}
	UNLOCK(m_CALCUALTEMUTEX);
}

void ListCalculator::setSubVar(string var, const double* val)
{
	vector<string> spl;
	ListCalculator* container;
	vector<ICalculatorPattern*> childs;

	LOCK(m_CALCUALTEMUTEX);
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
	UNLOCK(m_CALCUALTEMUTEX);
}

void ListCalculator::removeObserver(IMeasurePattern* observer)
{
	string var;
	vector<string> inform;
	SHAREDPTR::shared_ptr<IListObjectPattern> found;

	if(isEmpty())
		return;
	LOCK(m_CALCUALTEMUTEX);
	inform= getVariables();
	for(vector<string>::const_iterator it= inform.begin(); it != inform.end(); ++it)
	{
		var= *it;
		found= getSubroutine(&var, m_nArrayFolderID, /*own folder*/false);
		if(found != NULL)
			found->removeObserver(observer, m_sFolder, m_sSubroutine, m_sParameter);
	}
	UNLOCK(m_CALCUALTEMUTEX);
}
