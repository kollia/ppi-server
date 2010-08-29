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

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "switch.h"

#include "../logger/lib/LogInterface.h"

#include "../util/CalculatorContainer.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;
using namespace util;
using namespace boost;

switchClass::switchClass(string folderName, string subroutineName)
: portBase("SWITCH", folderName, subroutineName)
{
	m_bLastValue= false;
	m_bUseInner= false;
	m_bInner= false;
}

switchClass::switchClass(string type, string folderName, string subroutineName)
: portBase(type, folderName, subroutineName)
{
	m_bLastValue= false;
	m_bUseInner= false;
	m_bInner= false;
}

bool switchClass::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const bool* const defaultValue)
{
	SHAREDPTR::shared_ptr<measurefolder_t> pAct;
	string on, sWhile, off, prop("default"), type;
	string sFolder= getFolderName();
	//DbInterface *db= DbInterface::instance();

	m_pStartFolder= pStartFolder;
	m_sOn= properties.getValue("begin", /*warning*/false);
	m_sWhile= properties.getValue("while", /*warning*/false);
	m_sOff= properties.getValue("end", /*warning*/false);
	//defaultValue= properties.getDouble(prop, /*warning*/false);
	portBase::init(properties);
	if(defaultValue != NULL)
	{
		m_bInner= *defaultValue;
		m_bUseInner= true;
	}

	pAct= m_pStartFolder;
	while(pAct != NULL)
	{
		if(pAct->name == sFolder)
		{
			m_pOwnFolder= pAct;
			break;
		}
		pAct= pAct->next;
	}

	// set default value
	/*value= db->getActEntry(exist, getFolderName(), getSubroutineName(), "value");
	if(!exist)
		value= defaultValue;
	setValue(value);*/
	return true;
}

void switchClass::setObserver(IMeasurePattern* observer)
{
	string folder(getFolderName());
	string subroutine(getSubroutineName());

	if(m_sOn != "")
		activateObserver(m_pStartFolder, observer, folder, subroutine, m_sOn);
	if(m_sWhile != "")
		activateObserver(m_pStartFolder, observer, folder, subroutine, m_sWhile);
	if(m_sOff != "")
		activateObserver(m_pStartFolder, observer, folder, subroutine, m_sOff);
}

void switchClass::removeObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
									const string& folder, const string& subroutine, const string& cCurrent, const string& addinfo)
{
	string full(cCurrent);
	portBase* found;

	while(full != "")
	{
		found= filterSubroutines(pStartFolder, folder, subroutine, full, addinfo);
		if(found != NULL)
		{
			//cout << "remove from " << found->getFolderName() << ":" << found->getSubroutineName();
			//cout << " to inform folder " << folder << endl;
			found->removeObserver(observer, folder);
		}
	}
}

void switchClass::activateObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
									const string& folder, const string& subroutine, const string& cCurrent, const string& addinfo)
{
	string full(cCurrent);
	portBase* found;

	while(full != "")
	{
		found= filterSubroutines(pStartFolder, folder, subroutine, full, addinfo);
		if(found != NULL)
		{
			//cout << "inform folder " << folder << " when value of " << found->getFolderName() << ":" << found->getSubroutineName();
			//cout << " be changed" << endl;
			found->informObserver(observer, folder);
		}
	}
}

portBase* switchClass::filterSubroutines(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder,
											const string& folder, const string& subroutine, string& folders, const string& addinfo)
{
	string found;
	string::size_type nPos= 0;
	string::size_type breaks= 0;
	string::size_type breakon= 0;
	string::size_type nLen= folders.length();
	portBase* pRv;

	// search first whether the folders string has an if sentence
	while(nPos < nLen)
	{
		if(folders[nPos] == '(')
			++breaks;
		else if(folders[nPos] == ')')
			--breaks;
		else if(	folders[nPos] == '?' &&
					breaks == 0				)
		{
			if(nPos == 0)
			{// all subroutines before question mark (the if sentence) be searched
				++nPos;
				while(nPos < nLen)
				{
					if(folders[nPos] == '(')
						++breaks;
					else if(folders[nPos] == ')')
						--breaks;
					else if(	folders[nPos] == ':' &&
								breaks == 0				)
					{
						if(nPos == 1)
						{// all subroutines for true value (between question mark and colon) be searched
							folders= folders.substr(2);
							return filterSubroutines(pStartFolder, folder, subroutine, folders, addinfo);
						}
						found= folders.substr(1, nPos-1);
						pRv= filterSubroutines(pStartFolder, folder, subroutine, found, addinfo);
						folders= "?" + found + folders.substr(nPos);
						return pRv;
					}
					++nPos;
				}

			}
			found= folders.substr(0, nPos);
			pRv= filterSubroutines(pStartFolder, folder, subroutine, found, addinfo);
			folders= found + folders.substr(nPos);
			return pRv;
		}
		++nPos;
	}
	nPos= 0;
	breaks= 0;
	while(nPos < nLen)
	{
		if(folders[nPos] == '(')
		{
			breakon= nPos;
			++breaks;
			++nPos;
			while(nPos < nLen)
			{
				if(folders[nPos] == '(')
					++breaks;
				else if(folders[nPos] == ')')
				{
					--breaks;
					if(breaks == 0)
					{
						found= folders.substr(breakon+1, nPos-breakon-1);
						pRv= filterSubroutines(pStartFolder, folder, subroutine, found, addinfo);
						trim(found);
						if(found != "")
							found= "(" + found + ")";
						folders= folders.substr(0, breakon) + found + folders.substr(nPos + 1);
						trim(folders);
						return pRv;
					}
				}
				++nPos;
			}
			nPos= breakon;
		}
		//cout << "position: '" << full[nPos] << "'" << endl;
		if(	folders[nPos] == '+' ||
			folders[nPos] == '-' ||
			folders[nPos] == '/' ||
			folders[nPos] == '*' ||
			folders[nPos] == '<' ||
			folders[nPos] == '>' ||
			folders[nPos] == '=' ||
			folders[nPos] == '&' ||
			folders[nPos] == '|' ||
			folders[nPos] == '(' ||
			folders[nPos] == ')' ||
			folders[nPos] == '?' ||
			folders[nPos] == '!'	)
		{
			found= folders.substr(0, nPos);
			if(folders[nPos+1] == '=')
				++nPos;
			folders= folders.substr(nPos + 1);
			return getPort(pStartFolder, folder, subroutine, found, addinfo);
		}
		++nPos;
	}
	found= folders;
	trim(found);
	folders= "";
	if(found == "")
		return NULL;
	return getPort(pStartFolder, folder, subroutine, found, addinfo);
}

portBase* switchClass::getPort(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder,
								const string& folder, const string& subroutine, const string& cCurrent, const string& addinfo)
{
	string found, upper, msg;
	string sFolder;
	string sSubroutine;
	vector<string> spl;
	SHAREDPTR::shared_ptr<measurefolder_t>  pfolder;

	found= cCurrent;
	trim(found);
	upper= found;
	transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int)) toupper);
	if(	isdigit(found.c_str()[0])
		||
		found.c_str()[0] == '.' // maybe a float beginning with no 0
		||
		upper == "TRUE"
		||
		upper == "FALSE"				)
	{
		// nothing to do
		// string is only an number
		// or boolean
		return NULL;
	}

	// search in all subroutines of folders
	// where the name is equal to the current string
	// and ask from the class of the subroutine the value
	split(spl, found, is_any_of(":"));
	if(spl.size() != 2 || spl[0] == folder)
	{
		// information is for own folder
		// folder is activated and do not need again
		return NULL;
	}else
	{
		sFolder= spl[0];
		sSubroutine= spl[1];
	}
	trim(sFolder);
	trim(sSubroutine);
	pfolder= getFolder(sFolder, pStartFolder);
	if(pfolder)
	{
		for(vector<sub>::iterator it= pfolder->subroutines.begin(); it != pfolder->subroutines.end(); ++it)
		{
			if(	it->bCorrect &&
				it->name == sSubroutine	)
			{
				return it->portClass.get();
			}
		}
	}
	msg= "cannot found folder '";
	msg+= sFolder + "' with subroutine '" + sSubroutine;
	msg+= "' defined in folder " + folder + " and subroutine ";
	msg+= subroutine;
	if(addinfo != "")
	{
		if(addinfo[0] != ' ')
			msg+= " ";
		msg+= addinfo;
	}
	LOG(LOG_ERROR, msg);
	cerr << "###ERROR: " << msg << endl;
	return NULL;
}

double switchClass::measure()
{
	bool bDoOnOff= false;
	bool bResultTrue= false;
	bool bSwitched= false;
	bool bRemote= false;
	double dResult;

	/*if(getFolderName() == "TRANSMIT_SONY"
		&& getSubroutineName() == "wait_after_measure")
				cout << "do " << flush;*/
	if(	(	m_bUseInner &&
			m_bInner		) ||
		(	!m_bUseInner &&
			portBase::getValue("i:" + getFolderName())	)	)
	{
		bResultTrue= true;
		bSwitched= true;
	}
	if(	bSwitched
		&&
		!m_bLastValue	)
	{// if m_bSwitched is true
	 // but on the last session it was false
	 // the variable be set over the server from outside
		bRemote= true;
		bDoOnOff= true;
		if(isDebug())
			cout << "SWITCH value was enabled from remote access" << endl;

	}else if(	isDebug() &&
				!m_bUseInner &&
				!bSwitched &&
				m_bLastValue	)
	{// if m_bSwitched is false
	 // but on the last session it was true
	 // the variable be set over the server from outside
			//bRemote= true;
			//bDoOnOff= true;
			//if(isDebug())
				cout << "SWITCH value was disabled from remote access" << endl;
	}

	if(	!bSwitched
		&&
		m_sOn != ""	)
	{// if m_bSwitched is false
	 // and an begin result is set
	 // look for beginning
		if(!calculateResult(m_sOn, dResult))
		{
			string msg("           could not resolve parameter 'begin= ");

			msg+= m_sOn + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"begin", msg);
			if(isDebug())
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
		if(dResult)
			bResultTrue= true;
		else
			bResultTrue= false;
		bDoOnOff= true;
	}else if(	bSwitched
				&&
				m_sOff != ""
				&&
				!bRemote		)
	{// else if m_bSwitched is true
	 // and an end result be set
	 // look for ending
	 // only in the session when m_bSwitched
	 // not set from outside
		if(!calculateResult(m_sOff, dResult))
		{
			string msg("           could not resolve parameter 'end= ");

			msg+= m_sOff + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"end", msg);
			if(isDebug())
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}

		if(dResult)
			bResultTrue= false;
		else
			bResultTrue= true;
		if(!bResultTrue)
			bDoOnOff= true;
	}
	if(	m_sWhile != ""
		&&
		(	bSwitched
			||
			m_sOn == ""	)
		&&
		!bDoOnOff			)
	{
		if(!calculateResult(m_sWhile, dResult))
		{
			string msg("           could not resolve parameter 'while= ");

			msg+= m_sWhile + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"while", msg);
			if(isDebug())
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
		if(dResult)
			bResultTrue= true;
		else
			bResultTrue= false;
	}
	if(	m_sOn != ""
		||
		m_sWhile != ""
		||
		m_sOff != ""	)
	{
		// if nothing set for begin, while or end
		// bResultTrue is always false
		// so do not set bSwitched
		// because it can be set from outside true the server
		// see second if-sentence -> if(bSwitched && !m_bLastValue) <- inside this method
		bSwitched= bResultTrue;
	}
	m_bLastValue= bSwitched;

	if(isDebug())
		cout << "result for SWITCH is " << boolalpha << bResultTrue << endl;
	if(bResultTrue)
		return 1;
	return 0;
}

bool switchClass::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= 1;
	return true;
}

bool switchClass::getResult(const string &from, bool& result)
{
	bool bRv, debug= isDebug();
	string str(from);

	if(debug)
		cout << "make from result: " << str << endl << "read ";
	bRv= getResult(str, m_pStartFolder, getFolderName(), debug, result);
	if(debug)
		cout << endl;
	return bRv;
}

bool switchClass::getResult(string &str, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result)
{
	bool parenthies= false;
	char op= '\0';
	char cbreak= '\0';
	bool iresult;
	string word;
	string::size_type pos= 0;
	string::size_type end;

	end= str.length();
	do{
		while(	pos != end &&
				str[pos] != '|' &&
				str[pos] != '&' &&
				str[pos] != '(' &&
				str[pos] != ')'		)
		{
			++pos;
		}
		if(pos != end)
			op= str[pos];
		else
			op= '\0';
		if(op == '(')
		{
			short p= 0;
			if(debug)
				cout << "(" << flush;
			str= str.substr(pos + 1);
			if(!getResult(str, pStartFolder, sFolder, debug, result))
				return false;
			end= str.length();
			pos= 0;
			while(pos != end)
			{
				if(str[pos] == '(')
					++p;
				else if(str[pos] == ')')
				{
					if(p == 0)
					{
						++pos;
						break;
					}else
						--p;
				}
				++pos;
			}
			if(debug)
				cout << ") " << flush;
			if(result)
				str= "true" + str.substr(pos );
			else
				str= "false" + str.substr(pos);
			end= str.length();
			pos= 0;
			continue;
		}else
		{
			word= str.substr(0, pos);
			if(pos != end)
				str= str.substr(pos + 1);
			else
				str= "";
			if(!getSubResult(word, pStartFolder, sFolder, debug, result))
				return false;
		}

		if(	result == false
			&&
			op == '&'		)
		{
			if(debug)
				cout << " break by";
			break;
		}
		if(	result == true
			&&
			op == '|'		)
		{
			if(debug)
				cout << " break by";
			break;
		}
		if(debug && op != '\0' && op != ')')
			cout << " " << op << " " << flush;
		pos= 0;
		end= str.length();

	}while(op != '\0' && op != ')');
	if(debug && op != ')')
	{
		if(result)
			cout << "  TRUE";
		else
			cout << "  FALSE";
	}
	if(op == ')')
		str= ")" + str;
	return true;
}

bool switchClass::getSubResult(const string &from, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& bCheck)
{
	string::size_type nResultLen= from.length();
	string::size_type pos= 0;
	string result= from;
	string word, msg;
	char cOperator[3];
	double value1, value2;
	double *pValue= &value1;

	bCheck= false;
	cOperator[0]= '\0';
	cOperator[1]= '\0';
	cOperator[2]= '\0';
	while(pos < nResultLen)
	{
		if(	result[pos] == '!'
			||
			result[pos] == '='
			||
			result[pos] == '>'
			||
			result[pos] == '<'	)
		{
			word= result.substr(0, pos);
			cOperator[0]= result[pos];
			if(result[pos+1] == '=')
			{
				++pos;
				cOperator[1]= result[pos];
				cOperator[2]= '\0';
			}else
				cOperator[1]= '\0';
			if(!calculateResult(pStartFolder, sFolder, word, debug, value1))
			{
				bCheck= false;
				if(debug)
					cout << "'cannot read'";
				return false;
			}
			if(debug)
			{
				trim(word);
				cout << "(" << word << ")"<< "= " << dec << *pValue << flush;
			}
			word= result.substr(pos+1);
			pValue= &value2;
			break;
		} // end of if result[pos] is operator
		++pos;
	}
	if(	cOperator[0] == '\0'
		&&
		word == ""			)
	{
		word= result;
	}
	if(!calculateResult(pStartFolder, sFolder, word, debug, *pValue))
	{
		bCheck= false;
		if(debug)
			cout << "'cannot read'";
		return false;
	}
	if(debug)
	{
		trim(word);
		cout << " " << cOperator << " ";
		cout << "(" << word << ")"<< "= " << dec << *pValue << flush;
	}
	if(cOperator[0] == '\0')
	{
		if(value1)
			bCheck= true;
		return true;
	}

	if(!strcmp(cOperator, "="))
	{
		if(value1 == value2)
			bCheck= true;
	}else if(!strcmp(cOperator, "!="))
	{
		if(value1 != value2)
			bCheck= true;
	}else if(!strcmp(cOperator, ">="))
	{
		if(value1 >= value2)
			bCheck= true;
	}else if(!strcmp(cOperator, "<="))
	{
		if(value1 <= value2)
			bCheck= true;
	}else if(!strcmp(cOperator, ">"))
	{
		if(value1 > value2)
			bCheck= true;
	}else if(!strcmp(cOperator, "<"))
	{
		if(value1 < value2)
			bCheck= true;
	}else
	{
		string msg("undefined operator '");

		msg+= cOperator;
		msg+= "' in result:\n";
		msg+= from;
		TIMELOG(LOG_ERROR, sFolder, msg);
		if(debug)
			cout << msg << endl;
		bCheck= false;
		return false;
	}
#ifdef DEBUG
	if(debug)
	{
		cout << "result is ";
		if(bCheck)
			cout << "true";
		else
			cout << "false";
		cout << endl;
	}
#endif // DEBUG
	return true;
}

bool switchClass::subroutineResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pfolder, const string &cCurrent, double &dResult, SHAREDPTR::shared_ptr<portBase>* port)
{
	string subroutine(cCurrent);
	unsigned nCount= pfolder->subroutines.size();

	trim(subroutine);
	for(unsigned c= 0; c<nCount; ++c)
	{
		//cout << "if('" << pfolder->subroutines[c].name << "'=='" << pcCurrent << "'" << endl;
		if(pfolder->subroutines[c].name == subroutine)
		{
			if(	pfolder->subroutines[c].bCorrect &&
				pfolder->subroutines[c].portClass.get() != NULL	)
			{
				dResult= pfolder->subroutines[c].portClass->getValue("i:" + pfolder->name);
				if(port != NULL)
					*port= pfolder->subroutines[c].portClass;
				return true;
			}
			return false;
		}
	}
	return false;
}

const SHAREDPTR::shared_ptr<measurefolder_t>  switchClass::getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	SHAREDPTR::shared_ptr<measurefolder_t> current= pStartFolder;

	while(current && current->name != name)
		current= current->next;
	return current;
}

bool switchClass::calculateResult(const string &cCurrent, double &dResult)
{
	bool bFound= calculateResult(m_pStartFolder, getFolderName(), cCurrent, isDebug(), dResult);

	if(!bFound)
	{
		string msg("cannot create string '");

		msg+= cCurrent;
		msg+= "' in any folder\nfrom result: ";
		msg+= cCurrent;
		TIMELOG(LOG_ERROR, getFolderName(), msg);
		if(isDebug())
			cout << msg << endl;
		dResult= 0;
	}
	return bFound;
}

bool switchClass::calculateResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &cCurrent, const bool debug, double &dResult)
{
	CalculatorContainer calc;
	string::size_type nLen= cCurrent.length();
	bool bCorrect= false;
	double value;
	string word, full;
	char cOp;
	int nPos= 0;
	int breaks= 0;

	full= cCurrent;
	// search first for if sentences
	while(nPos < nLen)
	{
		if(full[nPos] == '(')
			++breaks;
		else if(full[nPos] == ')')
			--breaks;
		else if(	full[nPos] == '?' &&
					breaks == 0				)
		{// calculate boolean for if sentence before question mark ('?')
			bool bResult;
			int x;
			string decision(full.substr(0, nPos));

			if(debug)
				cout << "calculating if sentence ('" << decision << "')" << endl;
			bCorrect= getResult(decision, pStartFolder, actFolder, debug, bResult);
			if(debug)
				cout << endl;
			if(!bCorrect)
				return false;
			++nPos;
			x= nPos;
			bCorrect= false;
			while(nPos < nLen)
			{
				if(full[nPos] == '(')
					++breaks;
				else if(full[nPos] == ')')
					--breaks;
				else if(	full[nPos] == ':' &&
							breaks == 0				)
				{ // calculate result for true value (before colon (':'), or for false value (after colon (':')
					if(bResult)
						full= full.substr(x, nPos - x);
					else
						full= full.substr(nPos + 1);
					return calculateResult(pStartFolder, actFolder, full, debug, dResult);
				}
				++nPos;
			}
			if(debug)
				cout << "fault results be set" << endl;
			return false;
		}
		++nPos;
	}

	// look whether an operator for boolean calculation ('|', '&', '<', '>', '=" or '!') exists
	// and ask than only for boolean result
	nPos= 0;
	while(nPos < nLen)
	{
		if(	full[nPos] == '|' ||
			full[nPos] == '&' ||
			full[nPos] == '>' ||
			full[nPos] == '<' ||
			full[nPos] == '=' ||
			full[nPos] == '!'	)
		{
			bool bResult;
			string decision(cCurrent);

			if(!getResult(decision, pStartFolder, actFolder, debug, bResult))
				return false;
			if(bResult)
				dResult= 1;
			else
				dResult= 0;
			if(debug)
				cout << endl;
			return true;
		}
		++nPos;
	}

	// create result of string
	bool bNoVal= true;

	breaks= 0;
	nPos= 0;
	while(nPos < nLen)
	{
		if(full[nPos] == '(')
		{
			int nstart;

			++nPos;
			nstart= nPos;
			++breaks;
			while(nPos < nLen)
			{
				if(full[nPos] == '(')
					++breaks;
				else if(full[nPos] == ')')
				{
					--breaks;
					if(breaks == 0)
						break;
				}
				++nPos;
			}
			bCorrect= calculateResult(pStartFolder, actFolder, full.substr(nstart, nPos-2), debug, value);
			if(!bCorrect)
				return false;
			calc.add(value);
			if(nPos+1 >= nLen)
			{
				dResult= calc.getResult();
				return true;
			}
			full= full.substr(nPos+1);
			nLen= full.length();
		}
		if(	!bNoVal &&
			(	full[nPos] == '+' ||
				full[nPos] == '-' ||
				full[nPos] == '/' ||
				full[nPos] == '*'	)	)
		{
			cOp= full[nPos];
			word= full.substr(0, nPos);
			bCorrect= searchResult(pStartFolder, actFolder, word.c_str(), value);
			if(!bCorrect)
				return false;
			calc.add(value);
			bCorrect= calc.add(cOp);
			if(!bCorrect)
				return false;
			full= full.substr(nPos + 1);
			nPos= -1;
			nLen= full.length();
			bNoVal= true;

		}else if(	bNoVal &&
					full[nPos] != ' ' &&
					full[nPos] != '+' &&
					full[nPos] != '-'	)
		{// for beginning if there an minus value
		 // calc them to the value
			bNoVal= false;
		}
		++nPos;
	}
	bCorrect= searchResult(pStartFolder, actFolder, full.c_str(), value);
	if(!bCorrect)
		return false;
	calc.add(value);
	dResult= calc.getResult();
	return true;
}

bool switchClass::searchResult(const char* pcCurrent, double &dResult)
{
	bool bFound= searchResult(m_pStartFolder, getFolderName(), pcCurrent, dResult);

	if(!bFound)
	{
		string msg("does not found subroutine '");

		msg+= pcCurrent;
		msg+= "' in any folder\nfrom result: ";
		msg+= pcCurrent;
		TIMELOG(LOG_ERROR, getFolderName(), msg);
		if(isDebug())
			cout << msg;
		dResult= 0;
	}
	return bFound;
}

bool switchClass::searchResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &cCurrent, double &dResult, SHAREDPTR::shared_ptr<portBase>* port/*=NULL*/)
{
	bool bFound= false;
	string full;
	measurefolder_t *pfolder;

	full= cCurrent;
	trim(full);
	if(	isdigit(full[0]) ||
		full[0] == '.' /*maybe a float beginning with no 0*/
		||
		(	(	full[0] == '-' ||
				full[0] == '+'		)
			&&
			(	isdigit(full[1]) ||
			 	full[1] == '.'		 )	)					)
	{
		istringstream cNum(full);
		ostringstream cBack;

		cNum >> dResult;
		cBack << dResult;
		if(full == cBack.str())
			bFound= true;
	}

	if(!bFound)
	{
		string upper(full);

		transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int)) toupper);
		if(upper == "TRUE")
		{
			bFound= true;
			dResult= 1;

		}else if(upper == "FALSE")
		{
			bFound= true;
			dResult= 0;
		}
	}

	if(!bFound)
	{// search in all subroutines of folders
	 // where the name is equal to the current string
	 // and ask from the class of the subroutine the value
		string sFolder;
		string sSubroutine;
		vector<string> spl;
		SHAREDPTR::shared_ptr<measurefolder_t>  folder;

		split(spl, full, is_any_of(":"));
		if(spl.size() == 2)
		{
			sFolder= spl[0];
			sSubroutine= spl[1];
		}else
		{
			sFolder= actFolder;
			sSubroutine= spl[0];
		}
		folder= getFolder(sFolder, pStartFolder);
		if(folder)
			bFound= subroutineResult(folder, sSubroutine, dResult, port);
	}

	return bFound;
}

/*void switchClass::setValue(const double value)
{
	double bValue= 0;

	if(value)
		bValue= 1;
	portBase::setValue(bValue);
}*/

switchClass::~switchClass()
{
}
