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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Terminal.h"

#include "../../pattern/util/LogHolderPattern.h"

Terminal* Terminal::_instance= NULL;

using namespace boost;

Terminal* Terminal::instance()
{
	if(_instance == NULL)
		_instance= new Terminal();
	return _instance;
}

/*
SHAREDPTR::shared_ptr<ostringstream> Terminal::out()
{
	pid_t pid;
	SHAREDPTR::shared_ptr<ostringstream> oRv;
	vector<pid_t>::iterator ending;
	vector<SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
	map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > >::iterator found;

	pid= Thread::gettid();
	oRv= SHAREDPTR::shared_ptr<ostringstream>(new ostringstream);
	LOCK(m_PRINT);
	cout << "[make " << pid << " msg] ";
	if(!m_qOrder.empty() && pid != m_qOrder.front())
		cout << m_qOrder.front() << " blocks";
	cout << endl;
	found= m_mqRunStrings.find(pid);
	if(found != m_mqRunStrings.end())
	{
		if(	m_qOrder.empty() ||
			pid == m_qOrder.front()	)
		{// if actual pid is same than first of order (m_qOrder)
		 // write out existing old strings
			for(it= found->second.begin(); it != found->second.end(); ++it)
				cout << (*it)->str();
			found->second.clear();
		}

	}
	if(find(m_qOrder.begin(), m_qOrder.end(), pid) == m_qOrder.end())
		m_qOrder.push_back(pid);
	ending= find(m_vEnding.begin(), m_vEnding.end(), pid);
	if(ending != m_vEnding.end())
		m_vEnding.erase(ending);

	m_mqRunStrings[pid].push_back(oRv);
	UNLOCK(m_PRINT);
	return oRv;
}*/
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
SHAREDPTR::shared_ptr<ostringstream> Terminal::out(string file, int line)
#else
SHAREDPTR::shared_ptr<ostringstream> Terminal::out()
#endif
{
	pid_t pid;
	SHAREDPTR::shared_ptr<ostringstream> oRv;
	vector<SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
	map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > >::iterator found;

	pid= Thread::gettid();
	oRv= SHAREDPTR::shared_ptr<ostringstream>(new ostringstream);
	LOCK(m_PRINT);
	found= m_mqRunStrings.find(pid);
	if(found != m_mqRunStrings.end())
	{// if thread is in m_mqRunStrings, also m_qOrder is not empty

		if(pid == m_qOrder.front())
		{// if aktual pid is same than first of order (m_qOrder)
		 // write out existing old strings
			for(it= found->second.begin(); it != found->second.end(); ++it)
				write(pid, (*it)->str());
			found->second.clear();
		}

	}else
	{
		if(found == m_mqRunStrings.end())
			m_qOrder.push_back(pid);
	}
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
	if(pid == m_qOrder.front())
	{
		if(s != pid)
			cout << "show [" << pid << "] " << file << " " << line << endl;
		s= pid;
	}else
	{
		if(b != pid)
		{
			cout << "block [" << pid << "]" << file << " " << line << endl;
			cout << "actual order:" << endl;
			for(vector<pid_t>::iterator o= m_qOrder.begin(); o != m_qOrder.end(); ++o)
				cout << *o << endl;
			cout << "------------------" << endl;
		}
		b= pid;
	}
#endif // TERMINALOUTPUT_ONLY_THRADIDS

	m_mqRunStrings[pid].push_back(oRv);
	UNLOCK(m_PRINT);
	return oRv;
}

bool Terminal::isRegistered()
{
	bool reg(false);

	LOCK(m_PRINT);
	if(find(m_qOrder.begin(), m_qOrder.end(), Thread::gettid()) != m_qOrder.end())
		reg= true;
	UNLOCK(m_PRINT);
	return reg;
}

/*void Terminal::end()
{
	pid_t pid, fpid;
	vector<pid_t>::iterator ending;
	vector<SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
	map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > >::iterator found;

	pid= Thread::gettid();
	LOCK(m_PRINT);
	found= m_mqRunStrings.find(pid);
	cout << "[ending " << pid << " msgs] " << endl;
	if(found != m_mqRunStrings.end())
	{
		if(	m_qOrder.empty() ||
			pid == m_qOrder.front()	)
		{// if actual pid is same than first of order (m_qOrder)
		 // write out existing old strings
			for(it= found->second.begin(); it != found->second.end(); ++it)
				cout << (*it)->str();
			m_mqRunStrings.erase(found);
			if(!m_qOrder.empty())
				m_qOrder.erase(m_qOrder.begin());
			ending= find(m_vEnding.begin(), m_vEnding.end(), pid);
			if(ending != m_vEnding.end())
				m_vEnding.erase(ending);

		}else if(find(m_vEnding.begin(), m_vEnding.end(), pid) == m_vEnding.end())
			m_vEnding.push_back(pid);
	}
	if(!m_qOrder.empty())
	{
		fpid= m_qOrder.front();
		found= m_mqRunStrings.find(fpid);
		while(	found != m_mqRunStrings.end()	)
		{
			for(it= found->second.begin(); it != found->second.end(); ++it)
				cout << (*it)->str();
			m_mqRunStrings.erase(found);
			ending= find(m_vEnding.begin(), m_vEnding.end(), fpid);
			if(ending == m_vEnding.end())
				break;
			m_vEnding.erase(ending);
		}
	}
	UNLOCK(m_PRINT);
}*/
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
void Terminal::end(string file, int line)
#else
void Terminal::end()
#endif
{
	pid_t pid;
	pair<pid_t, string> buf;
	vector<SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
	map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > >::iterator found;

	pid= Thread::gettid();
	LOCK(m_PRINT);
	found= m_mqRunStrings.find(pid);
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
	cout << "ending [" << pid << "] " << file << " " << line << endl;
#endif
	if(found != m_mqRunStrings.end())
	{// if thread is in m_mqRunStrings, also m_qOrder is not empty
		if(pid == m_qOrder.front())
		{// if actual pid is same than first of order (m_qOrder)
		 // write out existing old strings
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
			cout << "write last strings of [" << pid << "]" << endl;
#endif
			for(it= found->second.begin(); it != found->second.end(); ++it)
				write(pid, (*it)->str());
			write(0, "");
			m_mqRunStrings.erase(found);
			m_qOrder.erase(m_qOrder.begin());
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
			cout << "new order:" << endl;
			for(vector<pid_t>::iterator o= m_qOrder.begin(); o != m_qOrder.end(); ++o)
				cout << *o << endl;
			cout << "------------------" << endl;
			pid= 0;
			cout << "write blocked buffer" << endl;
#endif
			s= 0;
			b= 0;
			// write now all strings which are finished in meantime
			while(!m_qStrings.empty())
			{
				buf= m_qStrings.front();
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
				if(buf.first != 0 && pid != buf.first)
					cout << "show [" << buf.first << "] block" << endl;
				pid= buf.first;
#endif
				write(buf.first, buf.second);
				m_qStrings.pop();
			}
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
			cout << "--------------" << endl;
#endif

		}else
		{
#ifdef TERMINALOUTPUT_ONLY_THRADIDS
			cout << "write [" << pid << "] into blocked buffer" << endl;
#endif
			for(it= found->second.begin(); it != found->second.end(); ++it)
				m_qStrings.push(pair<pid_t, string>(pid, (*it)->str()));
			m_qStrings.push(pair<pid_t, string>(0, ""));
			m_mqRunStrings.erase(found);
			m_qOrder.erase(find(m_qOrder.begin(), m_qOrder.end(), pid));
		}
	}
	UNLOCK(m_PRINT);
}

string Terminal::output_prefix(const pid_t& pid)
{
	ostringstream oRv;

	oRv << "[" << pid << "] ";
	return oRv.str();
}

string Terminal::str_between(const pid_t& pid)
{
	return "\n";
}

void Terminal::write(const pid_t& pid, const string& msg)
{
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
	static bool end(true);
	string sth;
	vector<string> spl;
	vector<string>::size_type size, count(0);

	if(pid)
	{
		sth= output_prefix(pid);
		split(spl, msg, is_any_of("\n"));
		size= spl.size();
		for(vector<string>::iterator it= spl.begin(); it != spl.end(); ++it)
		{
			++count;
			if(	end &&
				(	count != size ||
					*it != ""		)	)
			{
				cout << sth;
			}
			cout << *it;
			if(count != size)
				cout << endl;
			if(	count == size &&
				*it != "" )
//				(	*it != "" ||
//					size == 1	)	)
			{
				end= false;
			}else
			{
				end= true;
			}
		}
	}else
		cout << str_between(pid);
#endif // TERMINALOUTPUT_ONLY_THRADIDS
}

void Terminal::deleteObj()
{
	if(_instance)
	{
		if(	!_instance->m_mqRunStrings.empty() ||
			!_instance->m_qStrings.empty()			)
		{
			vector<SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
			map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > >::iterator ths;

			cout << "--------------------------------------------------------------------------" << endl;
			cout << "  WARNING: for Terminal object, some string exists inside buffer" << endl;
			cout << "--------------------------------------------------------------------------" << endl;
			while(!_instance->m_qStrings.empty())
			{
				pair<pid_t, string> buf;

				buf= _instance-> m_qStrings.front();
				cout << buf.second;
				_instance->m_qStrings.pop();
			}
			cout << endl;
			cout << "--------------------------------------------------------------------------" << endl;
			for(ths= _instance->m_mqRunStrings.begin(); ths != _instance->m_mqRunStrings.end(); ++ths)
				for(it= ths->second.begin(); it != ths->second.end(); ++it)
					cout << (*it)->str();
			cout << endl;
			cout << "--------------------------------------------------------------------------" << endl;
			cout << "--------------------------------------------------------------------------" << endl;
		}
		delete _instance;
		_instance= NULL;
	}
}
