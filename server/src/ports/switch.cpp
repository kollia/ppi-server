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

#include "switch.h"

#include "../logger/LogInterface.h"

#include "../util/CalculatorContainer.h"
#include "../util/configpropertycasher.h"

#include "../database/Database.h"

using namespace ppi_database;
using namespace util;

switchClass::switchClass(string folderName, string subroutineName)
: portBase("SWITCH", folderName, subroutineName)
{
	m_bLastValue= false;
	m_pStartFolder= NULL;
	m_pOwnFolder= NULL;
	m_pOn= NULL;
	m_pWhile= NULL;
	m_pOff= NULL;
}

switchClass::switchClass(string type, string folderName, string subroutineName)
: portBase(type, folderName, subroutineName)
{
	m_bLastValue= false;
	m_pStartFolder= NULL;
	m_pOwnFolder= NULL;
	m_pOn= NULL;
	m_pWhile= NULL;
	m_pOff= NULL;
}

//bool switchClass::init(folder *pStartFolder, string on, string sWhile, string off, double defaultValue)
bool switchClass::init(ConfigPropertyCasher &properties, measurefolder_t *pStartFolder)
{
	size_t nLen;
	measurefolder_t *pAct;
	string on, sWhile, off, prop("default");
	string sFolder= getFolderName();
	Database *db= Database::instance();
	double defaultValue, *pValue;
	double value;

	m_pStartFolder= pStartFolder;
	on= properties.getValue("begin", /*warning*/false);
	sWhile= properties.getValue("while", /*warning*/false);
	off= properties.getValue("end", /*warning*/false);
	defaultValue= properties.getDouble(prop, /*warning*/false);
	portBase::init(properties);

	nLen= strlen(&on[0]);
	if(nLen>0)
	{
		m_pOn= new char[nLen+2];
		strncpy(m_pOn, &on[0], nLen+2);
	}
	nLen= strlen(&sWhile[0]);
	if(nLen>0)
	{
		m_pWhile= new char[nLen+2];
		strncpy(m_pWhile, &sWhile[0], nLen+2);
	}
	nLen= strlen(&off[0]);
	if(nLen>0)
	{
		m_pOff= new char[nLen+2];
		strncpy(m_pOff, &off[0], nLen+2);
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
	pValue= db->getActEntry(getFolderName(), getSubroutineName(), "value");
	if(pValue)
	{
		value= *pValue;
		delete pValue;
	}else
		value= defaultValue;
	setValue(value);
	return true;
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
		m_pOn		)
	{// if m_bSwitched is false
	 // and an begin result is set
	 // look for beginning
		bResultTrue= getResult(m_pOn);
		bDoOnOff= true;
	}else if(	bSwitched
				&&
				m_pOff
				&&
				!bRemote		)
	{// else if m_bSwitched is true
	 // and an end result be set
	 // look for ending
	 // only in the session when m_bSwitched
	 // not set from outside
		bResultTrue= !getResult(m_pOff);
		if(!bResultTrue)
			bDoOnOff= true;
	}
	if(	m_pWhile
		&&
		(	bSwitched
			||
			!m_pOn		)
		&&
		!bDoOnOff			)
	{
		bResultTrue= getResult(m_pWhile);
	}
	if(	m_pOn
		||
		m_pWhile
		||
		m_pOff	)
	{
		// if nothing set for begin, while or end
		// bResultTrue is always false
		// so do not set m_bSwitched
		// becaue it can be set from outside thrue the server
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

bool switchClass::getResult(char *str)
{
	return getResult(str, m_pStartFolder, getFolderName(), isDebug());
}

bool switchClass::getResult(char *str, measurefolder_t* pStartFolder, string sFolder, bool debug)
{
	bool result;
	char op= '\0';
	char *worth= str;
	char *pos= str;

	if(debug)
		cout << "make from result: " << str << endl;
	do{
		while(	*pos != '\0'
				&&
				*pos != '|'
				&&
				*pos != '&'	)
		{
			++pos;
		}
		op= *pos;
		*pos= '\0';
		result= getSubResult(worth, pStartFolder, sFolder, debug);

		if(op != '\0')
		{
			*pos= op;
			++pos;
			worth= pos;
		}
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

	}while(op != '\0');
	return result;
}

bool switchClass::getSubResult(char *str, measurefolder_t* pStartFolder, string sFolder, bool debug)
{
	size_t nResultLen= strlen(str);
	bool bCheck= false;
	char cOperator[3];
	char *pcCurrent= new char[nResultLen+1];
	char *pResultChar= str;
	char *pCurrentChar= pcCurrent;
	double value1, value2;
	double *pValue= &value1;

	cOperator[0]= '\0';
	while(*pResultChar != '\0')
	{
		if(	*pResultChar == '!'
			||
			*pResultChar == '='
			||
			*pResultChar == '>'
			||
			*pResultChar == '<'	)
		{
			//cout << "current string is '" << pcCurrent << "'\n";
			*pCurrentChar= '\0';
			cOperator[0]= *pResultChar;
			if(pResultChar[1] == '=')
			{
				++pResultChar;
				cOperator[1]= *pResultChar;
				cOperator[2]= '\0';
			}else
				cOperator[1]= '\0';
			if(!calculateResult(pStartFolder, sFolder, pcCurrent, value1))
			{
				delete [] pcCurrent;
				return false;
			}
			pValue= &value2;
			pCurrentChar= pcCurrent;
			++pResultChar;
		} // end of if pResultChar is operator
		*pCurrentChar= *pResultChar;
		++pCurrentChar;
		++pResultChar;
	}
	*pCurrentChar= '\0';
	if(!calculateResult(pStartFolder, sFolder, pcCurrent, *pValue))
	{
		delete [] pcCurrent;
		return false;
	}
	if(cOperator[0] == '\0')
	{
		delete pcCurrent;
		if(value1)
			bCheck= true;
#ifdef DEBUG
		if(debug)
		{
			cout << "make from subresult: " << str << "  := " << value1 << endl;
			cout << "result is ";
			if(bCheck)
				cout << "true";
			else
				cout << "false";
			cout << endl;
		}
#endif // DEBUG
		return bCheck;
	}

#ifdef DEBUG
	if(debug)
		cout << "make from subresult: " << str << "  := " << value1 << cOperator << value2 << endl;
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
		msg+= str;
		TIMELOG(LOG_ERROR, sFolder, msg);
		if(debug)
			cout << msg << endl;
		delete[] pcCurrent;
		return false;
	}
	delete[] pcCurrent;
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
	return bCheck;
}

bool switchClass::subroutineResult(const measurefolder_t* pfolder, const char* pcCurrent, double &dResult)
{
	string subroutine(ConfigPropertyCasher::trim(pcCurrent));
	unsigned nCount= pfolder->subroutines.size();

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

measurefolder_t* switchClass::getFolder(string name, measurefolder_t* pStartFolder)
{
	measurefolder_t* current= pStartFolder;

	while(current && current->name != name)
		current= current->next;
	return current;
}

bool switchClass::calculateResult(const char* pcCurrent, double &dResult)
{
	bool bFound= calculateResult(m_pStartFolder, getFolderName(), pcCurrent, dResult);

	if(!bFound)
	{
		string msg("cannot create string '");

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

bool switchClass::calculateResult(measurefolder_t *const pStartFolder, string actFolder, const char* pcCurrent, double &dResult)
{
	CalculatorContainer calc;
	bool bCorrect= false;
	double value;
	char *pcString= new char[strlen(pcCurrent) + 2];
	char *cPos;
	char cOp;
	int nPos= 0;

	strcpy(pcString, pcCurrent);
	cPos= pcString;
	while(*cPos != '\0')
	{
		if(	*cPos == '+'
			||
			*cPos == '-'
			||
			*cPos == '/'
			||
			*cPos == '*'	)
		{
			cOp= *cPos;
			*cPos= '\0';
			bCorrect= searchResult(pStartFolder, actFolder, pcString, value);
			if(!bCorrect)
			{
				delete [] pcString;
				return false;
			}
			calc.add(value);
			bCorrect= calc.add(cOp);
			if(!bCorrect)
			{
				delete [] pcString;
				return false;
			}
			++cPos;
			pcString= cPos;
		}
		++cPos;
		++nPos;
	}
	bCorrect= searchResult(pStartFolder, actFolder, pcString, value);
	if(!bCorrect)
	{
		delete [] pcString;
		return false;
	}
	calc.add(value);
	dResult= calc.getResult();
	delete [] pcString;
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

bool switchClass::searchResult(measurefolder_t *const pStartFolder, string actFolder, const char* pcCurrent, double &dResult)
{
	bool bFound= false;
	measurefolder_t *pfolder;

	if(isdigit(*pcCurrent))
	{
		size_t len= strlen(pcCurrent);
		char *pcResult= new char[len + 2];
		double d;

		dResult= atof(pcCurrent);
		snprintf(pcResult, len+2, "%lf", dResult);
		d= atof(pcResult);
		if(d == dResult)
			bFound= true;
		delete [] pcResult;
	}

	if(!bFound)
	{// search in all subroutines of folders
	 // where the name is equal to the current string
	 // and ask from the class of the subroutine the value
		string sFolder;
		string sSubroutine;
		string::size_type pos;
		string::size_type len;

		sSubroutine= pcCurrent;
		pos= sSubroutine.find(":");
		len= sSubroutine.length();
		if(pos < len)
		{
			sFolder= sSubroutine.substr(0, pos);
			sFolder= ConfigPropertyCasher::trim(sFolder);
			sSubroutine= sSubroutine.substr(pos + 1, sSubroutine.length() - pos);
			sSubroutine= ConfigPropertyCasher::trim(sSubroutine);
			pfolder= pStartFolder;
			while(pfolder != NULL)
			{
				if(pfolder->name == sFolder)
				{
					char* pcSub;

					pcSub= &sSubroutine[0];
					bFound= subroutineResult(pfolder, pcSub, dResult);
					break;
				}
				pfolder= pfolder->next;
			}
		}else
			bFound= subroutineResult(getFolder(actFolder, pStartFolder), pcCurrent, dResult);

		if(!bFound)
		{
			if(	!strcmp(pcCurrent, "true")
				||
				!strcmp(pcCurrent, "TRUE")	)
			{
				bFound= true;
				dResult= 1;
			}
			if(	!strcmp(pcCurrent, "false")
				||
				!strcmp(pcCurrent, "FALSE")	)
			{
				bFound= true;
				dResult= 0;
			}
			if(!bFound)
			{
				return false;
			}
		}
	}
	return true;
}

void switchClass::setValue(const double value)
{
	double bValue= 0;

	if(value)
		bValue= 1;
	portBase::setValue(bValue);
}

switchClass::~switchClass()
{
	if(m_pOn)
		delete m_pOn;
	if(m_pWhile)
		delete m_pWhile;
	if(m_pOff)
		delete m_pOff;
}
