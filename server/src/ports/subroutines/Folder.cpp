/**
 * Folder::  This file 'Folder.cpp' is part of ppi-server.
 * Folder::  Created on: 28.01.2011
 *
 * Folder::  ppi-server is free software: you can redistribute it and/or modify
 * Folder::  it under the terms of the Lesser GNU General Public License as published by
 * Folder::  the Free Software Foundation, either version 3 of the License, or
 * Folder::  (at your option) any later version.
 *
 * Folder::  ppi-server is distributed in the hope that it will be useful,
 * Folder::  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * Folder::  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Folder::  Lesser GNU General Public License for more details.
 *
 * Folder::  You should have received a copy of the Lesser GNU General Public License
 * Folder::  along with ppi-server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "/home/kollia/development/C/workspace/ppi-server/src/ports/subroutines/Folder.h"

namespace subroutines
{

	Folder::Folder(ofstream& out, const string& name)
	:	m_bWritten(false),
	 	m_sName(name),
	 	m_oOutput(out)
	{
	}

	Switch* Folder::getSwitch(const string& name)
	{
		Switch* pRv= new Switch(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Value* Folder::getValue(const string& name)
	{
		Value* pRv= new Value(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Set* Folder::getSet(const string& name)
	{
		Set* pRv= new Set(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Timer* Folder::getTimer(const string& name)
	{
		Timer* pRv= new Timer(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Shell* Folder::getShell(const string& name)
	{
		Shell* pRv= new Shell(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Debug* Folder::getDebug(const string& name)
	{
		Debug* pRv= new Debug(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	Lirc* Folder::getLirc(const string& name)
	{
		Lirc* pRv= new Lirc(m_oOutput, name);

		if(!m_bWritten) flush();
		m_vpObjs.push_back(pRv);
		return pRv;
	}

	void Folder::flush()
	{
		if(!m_bWritten)
		{
			m_oOutput << "folder= " << m_sName << endl;
			m_bWritten= true;
		}
	}

	void Folder::description(const string& content)
	{
		if(content != "")
			m_oOutput << "# " << content;
		m_oOutput << endl;
	}

	Folder::~Folder()
	{
		for(vector<Subroutine*>::iterator it= m_vpObjs.begin(); it != m_vpObjs.end(); ++it)
			delete *it;
	}

}
