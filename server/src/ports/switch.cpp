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
#include "../util/configpropertycasher.h"

#include "../database/lib/DbInterface.h"

using namespace ppi_database;
using namespace util;
using namespace boost;

switchClass::switchClass(string folderName, string subroutineName)
: portBase("SWITCH", folderName, subroutineName)
{
	m_bLastValue= false;
}

switchClass::switchClass(string type, string folderName, string subroutineName)
: portBase(type, folderName, subroutineName)
{
	m_bLastValue= false;
}

//bool switchClass::init(folder *pStartFolder, string on, string sWhile, string off, double defaultValue)
bool switchClass::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	SHAREDPTR::shared_ptr<measurefolder_t> pAct;
	string on, sWhile, off, prop("default");
	string sFolder= getFolderName();
	DbInterface *db= DbInterface::instance();
	bool exist;
	double defaultValue;
	double value;

	m_pStartFolder= pStartFolder;
	m_sOn= properties.getValue("begin", /*warning*/false);
	m_sWhile= properties.getValue("while", /*warning*/false);
	m_sOff= properties.getValue("end", /*warning*/false);
	defaultValue= properties.getDouble(prop, /*warning*/false);
	portBase::init(properties);


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
	value= db->getActEntry(exist, getFolderName(), getSubroutineName(), "value");
	if(!exist)
		value= defaultValue;
	setValue(value);
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

void switchClass::activateObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
									const string& folder, const string& subroutine, const string& cCurrent)
{
	string::size_type nLen= cCurrent.length();
	string word, full;
	int nPos= 0;

	full= cCurrent;
	while(nPos < nLen)
	{
		if(	full[nPos] == '+'
			||
			full[nPos] == '-'
			||
			full[nPos] == '/'
			||
			full[nPos] == '*'
			||
			full[nPos] == '<'
			||
			full[nPos] == '>'
			||
			full[nPos] == '='
			||
			(	full[nPos] == '!'
				&&
				full[nPos+1] == '='	)	)
		{
			word= full.substr(0, nPos);
			if(full[nPos] == '!')
				++nPos;
			full= full.substr(nPos + 1);
			giveObserver(pStartFolder, observer, folder, subroutine, word);
		}
		++nPos;
	}
	giveObserver(pStartFolder, observer, folder, subroutine, full);
}

void switchClass::giveObserver(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, IMeasurePattern* observer,
								const string& folder, const string& subroutine, const string& cCurrent)
{
	bool bfound= false;
	string full, upper;
	string sFolder;
	string sSubroutine;
	vector<string> spl;
	SHAREDPTR::shared_ptr<measurefolder_t>  pfolder;

	full= cCurrent;
	trim(full);
	upper= full;
	transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int)) toupper);
	if(	isdigit(full.c_str()[0])
		||
		full.c_str()[0] == '.' // maybe a float beginning with no 0
		||
		upper == "TRUE"
		||
		upper == "FALSE"				)
	{
		// nothing to do
		// string is only an number
		// or boolean
		return;
	}

	// search in all subroutines of folders
	// where the name is equal to the current string
	// and ask from the class of the subroutine the value
	split(spl, full, is_any_of(":"));
	if(spl.size() == 1 || spl[0] == folder)
	{
		// information is for own folder
		// folder is activated and do not need again
		return;
	}else
	{
		sFolder= spl[0];
		sSubroutine= spl[1];
	}
	pfolder= getFolder(sFolder, pStartFolder);
	if(pfolder)
	{
		for(vector<sub>::iterator it= pfolder->subroutines.begin(); it != pfolder->subroutines.end(); ++it)
		{
			if(it->name == sSubroutine)
			{
				it->portClass->informObserver(observer, folder);
				bfound= true;
				break;
			}
		}
	}
	if(!bfound)
	{
		string msg("cannot found folder '");

		msg+= sFolder + "' with subroutine '" + sSubroutine;
		msg+= "' defined in folder " + folder + " and subroutine ";
		msg+= subroutine;
		LOG(LOG_ERROR, msg);
		cerr << "###ERROR: " << msg << endl;
	}
}

bool switchClass::measure()
{
	bool bDoOnOff= false;
	bool bResultTrue= false;
	bool bSwitched= false;
	bool bRemote= false;

	if(portBase::getValue("i:" + getFolderName()))
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
	}

	if(	!bSwitched
		&&
		m_sOn != ""	)
	{// if m_bSwitched is false
	 // and an begin result is set
	 // look for beginning
		if(!getResult(m_sOn, bResultTrue))
		{
			string msg("           could not resolve parameter 'begin= ");

			msg+= m_sOn + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"begin", msg);
			if(isDebug())
				cerr << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
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
		if(!getResult(m_sOff, bResultTrue))
		{
			string msg("           could not resolve parameter 'end= ");

			msg+= m_sOff + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"end", msg);
			if(isDebug())
				cerr << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
		bResultTrue= !bResultTrue;
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
		if(!getResult(m_sWhile, bResultTrue))
		{
			string msg("           could not resolve parameter 'while= ");

			msg+= m_sWhile + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"while", msg);
			if(isDebug())
				cerr << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
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
		// see second if-sentense -> if(bSwitched && !m_bLastValue) <- inside this mehtod
		bSwitched= bResultTrue;
	}
	portBase::setValue(bResultTrue);
	m_bLastValue= bSwitched;

	if(isDebug())
	{
		cout << "result for SWITCH is ";
		if(bResultTrue)
			cout << "true" << endl;
		else
			cout << "false" << endl;
	}
	return true;
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
	return getResult(from, m_pStartFolder, getFolderName(), isDebug(), result);
}

bool switchClass::getResult(const string &from, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& result)
{
	char op= '\0';
	string worth, str= from;
	string::size_type pos= 0;
	string::size_type end= from.length();

	if(debug)
		cout << "make from result: " << from << endl;
	do{
		while(	pos != end
				&&
				str[pos] != '|'
				&&
				str[pos] != '&'	)
		{
			++pos;
		}
		if(pos != end)
			op= from[pos];
		else
			op= '\0';
		worth= str.substr(0, pos);
		if(pos != end)
			str= str.substr(pos + 1);
		else
			str= "";
		if(!getSubResult(worth, pStartFolder, sFolder, debug, result))
			return false;

		/*if(op != '\0')
		{
			op= str[0];//*pos= op;
			++pos;
			//worth= pos;
		}*/
		if(	result == false
			&&
			op == '&'		)
		{
			break;
		}
		if(	result == true
			&&
			op == '|'		)
		{
			break;
		}
		pos= 0;
		end= str.length();

	}while(op != '\0');
	return true;
}

bool switchClass::getSubResult(const string &from, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string& sFolder, const bool debug, bool& bCheck)
{
	string::size_type nResultLen= from.length();
	string::size_type pos= 0;
	string result= from;
	string word;
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
			//cout << "current string is '" << pcCurrent << "'\n";
			word= result.substr(0, pos);
			cOperator[0]= result[pos];
			if(result[pos+1] == '=')
			{
				++pos;
				cOperator[1]= result[pos];
				cOperator[2]= '\0';
			}else
				cOperator[1]= '\0';
			if(!calculateResult(pStartFolder, sFolder, word, value1))
			{
				bCheck= false;
				return false;
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
	if(!calculateResult(pStartFolder, sFolder, word, *pValue))
	{
		bCheck= false;
		return false;
	}
	if(cOperator[0] == '\0')
	{
		if(value1)
			bCheck= true;
#ifdef DEBUG
		if(debug)
		{
			cout << "make from subresult: " << from << "  := " << value1 << endl;
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


#ifdef DEBUG
	if(debug)
		cout << "make from subresult: " << from << "  := " << value1 << cOperator << value2 << endl;
#endif // DEBUG
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

bool switchClass::subroutineResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pfolder, const string &cCurrent, double &dResult)
{
	string subroutine(cCurrent);
	unsigned nCount= pfolder->subroutines.size();

	trim(subroutine);
	for(unsigned c= 0; c<nCount; ++c)
	{
		//cout << "if('" << pfolder->subroutines[c].name << "'=='" << pcCurrent << "'" << endl;
		if(pfolder->subroutines[c].name == subroutine)
		{
			dResult= pfolder->subroutines[c].portClass->getValue("i:" + pfolder->name);
			return true;
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
	bool bFound= calculateResult(m_pStartFolder, getFolderName(), cCurrent, dResult);

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

bool switchClass::calculateResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &cCurrent, double &dResult)
{
	CalculatorContainer calc;
	string::size_type nLen= cCurrent.length();
	bool bCorrect= false;
	double value;
	string word, full;
	char cOp;
	int nPos= 0;

	full= cCurrent;
	while(nPos < nLen)
	{
		if(	full[nPos] == '+'
			||
			full[nPos] == '-'
			||
			full[nPos] == '/'
			||
			full[nPos] == '*'	)
		{
			cOp= full[nPos];
			word= full.substr(0, nPos);
			full= full.substr(nPos + 1);
			bCorrect= searchResult(pStartFolder, actFolder, word.c_str(), value);
			if(!bCorrect)
				return false;
			calc.add(value);
			bCorrect= calc.add(cOp);
			if(!bCorrect)
				return false;
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
			cout << msg << endl;
		dResult= 0;
	}
	return bFound;
}

bool switchClass::searchResult(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const string &actFolder, const string &cCurrent, double &dResult)
{
	bool bFound= false;
	string full;
	measurefolder_t *pfolder;

	full= cCurrent;
	trim(full);
	if(	isdigit(full.c_str()[0])
		||
		full.c_str()[0] == '.' /*maybe a float beginning with no 0*/	)
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
			bFound= subroutineResult(folder, sSubroutine, dResult);
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
