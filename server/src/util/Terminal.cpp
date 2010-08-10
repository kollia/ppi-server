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
#include <iostream>

#include "Terminal.h"

#include "thread/Thread.h"

Terminal* Terminal::_instance= NULL;

Terminal* Terminal::instnace()
{
	if(_instance == NULL)
		_instance= new Terminal();
	return _instance;
}

void Terminal::operator <<(const ostream& str)
{
	bool bout= false;
	ostringstream strout;
	string output;
	size_t pos;

	strout << str;
	output= strout.str();
	if(output.substr(output.length()-1) == "\n")
		bout= true;
	pos= output.find("\n");
	cout << dec << pos << endl;
}

void Terminal::deleteObj()
{
	delete _instance;
	_instance= NULL;
}

void Terminal::newline()
{
	pid_t tid= Thread::gettid();

	LOCK(m_PRINT);
	m_mvStrings[tid].push_back("");
	UNLOCK(m_PRINT);
}

Terminal::~Terminal()
{
	DESTROYMUTEX(m_PRINT);
}

Terminal::Terminal()
{
	m_PRINT= Thread::getMutex("PRINT");
}
