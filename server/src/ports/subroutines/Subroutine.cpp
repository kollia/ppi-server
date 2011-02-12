/**
 *   This file 'Subroutine.cpp' is part of ppi-server.
 *   Created on: 27.01.2011
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

#include <sstream>

#include "Subroutine.h"

namespace subroutines
{

	Subroutine::Subroutine(ofstream& out, const string& name, const string& type)
	:	m_bWritten(false),
	 	m_sName(name),
	 	m_sType(type),
	 	m_oOutput(out)
	{
	}

	void Subroutine::pbegin(const string& content, const string& desc)
	{
		writeParam("begin", content, desc);
	}

	void Subroutine::pwhile(const string& content, const string& desc)
	{
		writeParam("while", content, desc);
	}

	void Subroutine::pend(const string& content, const string& desc)
	{
		writeParam("end", content, desc);
	}

	void Subroutine::pfrom(const double& content, const string& desc)
	{
		writeParam("from", content, desc);
	}

	void Subroutine::pfrom(const string& content, const string& desc)
	{
		writeParam("from", content, desc);
	}

	void Subroutine::pfrom(const vector<string>& content)
	{
		for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("from", *it);
	}

	void Subroutine::pfrom(const vector<double>& content)
	{
		for(vector<double>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("from", *it);
	}

	void Subroutine::pset(const double& content, const string& desc)
	{
		writeParam("set", content, desc);
	}

	void Subroutine::pset(const string& content, const string& desc)
	{
		writeParam("set", content, desc);
	}

	void Subroutine::pset(const vector<string>& content)
	{
		for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("set", *it);
	}

	void Subroutine::pvalue(const double& content, const string& desc)
	{
		writeParam("value", content, desc);
	}

	void Subroutine::pvalue(const string& content, const string& desc)
	{
		writeParam("value", content, desc);
	}

	void Subroutine::pvalue(const vector<string>& content)
	{
		for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("value", *it);
	}

	void Subroutine::pvalue(const vector<double>& content)
	{
		for(vector<double>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("value", *it);
	}

	void Subroutine::pstring(const string& content, const string& desc)
	{
		writeParam("string", content, desc);
	}

	void Subroutine::pstring(const vector<string>& content)
	{
		for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("string", *it);
	}

	void Subroutine::plink(const string& content, const string& desc)
	{
		writeParam("link", content, desc);
	}

	void Subroutine::plink(const vector<string>& content)
	{
		for(vector<string>::const_iterator it= content.begin(); it != content.end(); ++it)
			writeParam("link", *it);
	}

	void Subroutine::plwhile(const string& content, const string& desc)
	{
		writeParam("lwhile", content, desc);
	}

	void Subroutine::pdefault(const double& content, const string& desc)
	{
		writeParam("default", content, desc);
	}

	void Subroutine::pdefault(const string& content, const string& desc)
	{
		writeParam("default", content, desc);
	}

	void Subroutine::pperm(const string& content, const string& desc)
	{
		writeParam("perm", content, desc);
	}
#if 0
	void Subroutine::pmin(const long& content, const string& desc)
	{
		writeParam("min", content, desc);
	}
#endif
	void Subroutine::pmin(const double& content, const string& desc)
	{
		writeParam("min", content, desc);
	}

	void Subroutine::pmax(const double& content, const string& desc)
	{
		writeParam("max", content, desc);
	}

	void Subroutine::pday(const long& content, const string& desc)
	{
		writeParam("day", content, desc);
	}

	void Subroutine::phour(const long& content, const string& desc)
	{
		writeParam("hour", content, desc);
	}

	void Subroutine::psec(const long& content, const string& desc)
	{
		writeParam("sec", content, desc);
	}

	void Subroutine::pmillisec(const long& content, const string& desc)
	{
		writeParam("millisec", content, desc);
	}

	void Subroutine::pmicrosec(const long& content, const string& desc)
	{
		writeParam("microsec", content, desc);
	}

	void Subroutine::pmtime(const string& content, const string& desc)
	{
		writeParam("mtime", content, desc);
	}

	void Subroutine::psetnull(const string& content, const string& desc)
	{
		writeParam("setnull", content, desc);
	}

	void Subroutine::pbegincommand(const string& content, const string& desc)
	{
		writeParam("begincommand", content, desc);
	}

	void Subroutine::pwhilecommand(const string& content, const string& desc)
	{
		writeParam("whilecommand", content, desc);
	}

	void Subroutine::pendcommand(const string& content, const string& desc)
	{
		writeParam("endcommand", content, desc);
	}

	void Subroutine::pfile(const string& content, const string& desc)
	{
		writeParam("file", content, desc);
	}

	void Subroutine::action(const string& content, const string& desc)
	{
		writeParam("action", content, desc);
	}

	void Subroutine::writeParam(const string& parameter, const long& content, const string& desc)
	{
		ostringstream cont;

		cont << content;
		writeParam(parameter, cont.str());
	}

	void Subroutine::writeParam(const string& parameter, const double& content, const string& desc)
	{
		ostringstream cont;

		cont << content;
		writeParam(parameter, cont.str());
	}

	void Subroutine::writeParam(const string& parameter, const string& content, const string& desc)
	{
		if(!m_bWritten) flush();
		m_oOutput << parameter << "= " << content;
		if(desc != "")
			m_oOutput << " # " << desc;
		m_oOutput << endl;
	}

	void Subroutine::flush()
	{
		if(!m_bWritten)
		{
			m_oOutput << "name= " << m_sName << endl;
			m_oOutput << "type= " << m_sType << endl;
			m_bWritten= true;
		}
	}

	void Subroutine::description(const string& content)
	{
		if(content != "")
			m_oOutput << "# " << content;
		m_oOutput << endl;
	}

}
