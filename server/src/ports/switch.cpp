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
: portBase("SWITCH", folderName, subroutineName),
  m_bLastValue(false),
  m_oBegin(folderName, subroutineName, "begin", true),
  m_oWhile(folderName, subroutineName, "while", true),
  m_oEnd(folderName, subroutineName, "end", true)
{
}

switchClass::switchClass(string type, string folderName, string subroutineName)
: portBase(type, folderName, subroutineName),
  m_bLastValue(false),
  m_oBegin(folderName, subroutineName, "begin", true),
  m_oWhile(folderName, subroutineName, "while", true),
  m_oEnd(folderName, subroutineName, "end", true)
{
}

bool switchClass::init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	string on, sWhile, off, prop("default"), type;
	string sFolder= getFolderName();

	//m_pStartFolder= pStartFolder;
	on= properties.getValue("begin", /*warning*/false);
	sWhile= properties.getValue("while", /*warning*/false);
	off= properties.getValue("end", /*warning*/false);
	m_oBegin.init(pStartFolder, on);
	m_oWhile.init(pStartFolder, sWhile);
	m_oEnd.init(pStartFolder, off);
	if(!portBase::init(properties))
		return false;
	return true;
}

void switchClass::setObserver(IMeasurePattern* observer)
{
	string folder(getFolderName());
	string subroutine(getSubroutineName());

	m_oBegin.activateObserver(observer);
	m_oWhile.activateObserver(observer);
	m_oEnd.activateObserver(observer);
}

portBase* switchClass::getPort(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder,
								const string& folder, const string& subroutine, const string& cCurrent, const bool own, const string& addinfo)
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
	if(!own && (spl.size() != 2 || spl[0] == folder))
	{
		// information is for own folder
		// folder is activated and do not need again
		return NULL;

	}else if(spl.size() < 2)
	{
		sFolder= folder;
		sSubroutine= spl[0];

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

double switchClass::measure(const double actValue)
{
	setting set;

	return measure(actValue, set);
}

double switchClass::measure(const double actValue, setting& set)
{
	bool debug(isDebug());
	bool bDoOnOff= false;
	bool bResultTrue= false;
	bool bSwitched= false;
	bool bRemote= false;
	double dResult;

	/*string f("TRANSMIT_SONY"), s("first_touch");
	if(getFolderName() == f
		&& getSubroutineName() == s)
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << f << ":" << s << endl;
	}*/
	set= NONE;
	if(	(actValue < 0 || actValue > 0)	)
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
		if(debug)
			cout << "SWITCH value was enabled from remote access" << endl;
		set= BEGIN;

	}else if(bSwitched)
	{
		set= WHILE;

	}else if(	!bSwitched &&
				m_bLastValue	)
	{// if m_bSwitched is false
	 // but on the last session it was true
	 // the variable be set over the server from outside
		if(debug)
			cout << "SWITCH value was disabled from remote access" << endl;
		set= END;
	}


	if(!m_oBegin.isEmpty())
	{// if m_bSwitched is false
	 // and an begin result is set
	 // look for beginning
		if(!m_oBegin.calculate(dResult))
		{
			string msg("           could not resolve parameter 'begin= ");

			msg+= m_oBegin.getStatement() + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"begin", msg);
			if(debug)
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
		if(dResult)
		{
			bResultTrue= true;
			set= BEGIN;
		}else
			bResultTrue= false;
		bDoOnOff= true;
	}
	if(	bSwitched &&
		!m_oEnd.isEmpty() &&
		!bRemote			)
	{// else if m_bSwitched is true
	 // and an end result be set
	 // look for ending
	 // only in the session when m_bSwitched
	 // not set from outside
		if(!m_oEnd.calculate(dResult))
		{
			string msg("           could not resolve parameter 'end= ");

			msg+= m_oEnd.getStatement() + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"end", msg);
			if(debug)
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}

		if(dResult)
		{
			bResultTrue= false;
			set= END;
		}else
			bResultTrue= true;
		if(!bResultTrue)
			bDoOnOff= true;
	}
	if(	!m_oWhile.isEmpty() &&
		(	bSwitched ||
			m_oBegin.isEmpty()	) &&
		!bDoOnOff					)
	{
		if(!m_oWhile.calculate(dResult))
		{
			string msg("           could not resolve parameter 'while= ");

			msg+= m_oWhile.getStatement() + "'\n           in folder ";
			msg+= getFolderName() + " with subroutine ";
			msg+= getSubroutineName();
			TIMELOG(LOG_ERROR, "switchresolve"+getFolderName()+getSubroutineName()+"while", msg);
			if(debug)
				cerr << endl << "### ERROR: " << msg.substr(11) << endl;
			bResultTrue= false;
		}
		if(dResult)
		{
			bResultTrue= true;
			if(m_bLastValue)
				set= WHILE;
			else
				set= BEGIN;
		}else
		{
			bResultTrue= false;
			if(m_bLastValue)
				set= END;
			else
				set= NONE;
		}
	}
	if(	!m_oBegin.isEmpty() ||
		!m_oWhile.isEmpty() ||
		!m_oEnd.isEmpty()		)
	{
		// if nothing set for begin, while or end
		// bResultTrue is always false
		// so do not set bSwitched
		// because it can be set from outside true the server
		// see second if-sentence -> if(bSwitched && !m_bLastValue) <- inside this method
		bSwitched= bResultTrue;
	}
	m_bLastValue= bSwitched;

	if(debug)
	{
		cout << "result for SWITCH is " << boolalpha << bResultTrue << " (";
		switch(set)
		{
		case 1:
			cout << "BEGIN";
			break;
		case 2:
			cout << "WHILE";
			break;
		case 3:
			cout << "END";
			break;
		default:
			cout << "NONE";
		}
		cout << ")" << endl;
	}
	if(bResultTrue)
		return 1;
	return 0;
}

void switchClass::setDebug(bool bDebug)
{
	m_oBegin.doOutput(bDebug);
	m_oWhile.doOutput(bDebug);
	m_oEnd.doOutput(bDebug);
	portBase::setDebug(bDebug);
}

bool switchClass::range(bool& bfloat, double* min, double* max)
{
	bfloat= false;
	*min= 0;
	*max= 1;
	return true;
}


const SHAREDPTR::shared_ptr<measurefolder_t>  switchClass::getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
{
	SHAREDPTR::shared_ptr<measurefolder_t> current= pStartFolder;

	while(current && current->name != name)
		current= current->next;
	return current;
}

switchClass::~switchClass()
{
}
