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

SHAREDPTR::shared_ptr<ostringstream> Terminal::out(pid_t threadID/*= 0*/)
{
	SHAREDPTR::shared_ptr<ostringstream> pRv;
	map<pid_t, SHAREDPTR::shared_ptr<ostringstream> >::iterator found;

	if(threadID == 0)
		threadID= Thread::gettid();
	LOCK(m_PRINT);
	if(m_pWorker.get() == NULL)
	{
		m_pWorker= std::auto_ptr<Terminal>(new Terminal());
		if(m_pWorker->start() != 0)
		{
			string err("cannot create Terminal thread to write output strings\n");

			cerr << "### WARNING: " << err;
			cerr << "              so writing strings directly, which cost by using more bad performance" << endl;
			TIMELOG(LOG_WARNING, "terminal", err +"so writing strings directly, which cost by using more bad performance");
		}
	}
	found= m_sLastMsgStream.find(threadID);
	if(found != m_sLastMsgStream.end())
	{
		if(	m_pWorker.get() != NULL &&
			m_pWorker->running()		)
		{
			m_pWorker->read(threadID, found->second->str());
			found->second->str("");
			UNLOCK(m_PRINT);
			return found->second;
		}else
		{
			string lastString(found->second->str());

			found->second->str("");
			if(lastString != "")
			{
				UNLOCK(m_PRINT);
				read(threadID, lastString);
				execute();
				LOCK(m_PRINT);
			}
		}
	}else
		m_sLastMsgStream[threadID]= SHAREDPTR::shared_ptr<ostringstream>(new ostringstream);
	pRv= m_sLastMsgStream[threadID];
	UNLOCK(m_PRINT);
	return pRv;
}

void Terminal::read(const pid_t& threadID, const string& msg)
{
	vector<string>::iterator it;
	map<pid_t, SHAREDPTR::shared_ptr<vector<string> > >::iterator found;

	if(	threadID == 0 ||
		msg == ""		)
	{
		return;
	}
	LOCK(m_PRINT);
	found= m_mqRunStrings.find(threadID);
	if(found == m_mqRunStrings.end())
	{
		m_mqRunStrings[threadID]= SHAREDPTR::shared_ptr<vector<string> >(new vector<string>());
		m_mqRunStrings[threadID]->push_back(msg);
	}else
		found->second->push_back(msg);
	if(find(m_qOrder.begin(), m_qOrder.end(), threadID) == m_qOrder.end())
		m_qOrder.push_back(threadID);
	if(m_qOrder.front() == threadID)
	{
		if(m_nActThreadID == 0)
			m_nActThreadID= threadID;
		AROUSE(m_WORKINGCONDITION);
	}
	UNLOCK(m_PRINT);
}

int Terminal::execute()
{
	pid_t threadID(0);
	finishedPtr_temp finished;
	SHAREDPTR::shared_ptr<vector<string> > pmsgs;

	LOCK(m_PRINT);
	while(	!stopping() &&
			m_pvFinishedStrings.get() == NULL &&
			(	m_qOrder.empty() ||
				m_mqRunStrings[m_qOrder.front()]->empty()	)	)
	{
		CONDITION(m_WORKINGCONDITION, m_PRINT);
	}
	if(	m_pvFinishedStrings.get() != NULL &&
		m_pvFinishedStrings->front().first == m_nActThreadID	)
	{
		finished= m_pvFinishedStrings;
		m_pvFinishedStrings= finishedPtr_temp();
		m_nActThreadID= 0;
	}
	if(!m_qOrder.empty())
	{
		threadID= m_qOrder.front();
		pmsgs= m_mqRunStrings[threadID];
		m_mqRunStrings[threadID]= SHAREDPTR::shared_ptr<vector<string> >(new vector<string>);
	}
	UNLOCK(m_PRINT);

	if(finished.get() != NULL)
	{
		for(finishedStrVec_temp::iterator it= finished->begin(); it != finished->end(); ++it)
		{
			for(vector<string>::iterator vit= it->second->begin(); vit != it->second->end(); ++vit)
				write(it->first, *vit);
			write(0, "");
		}
	}
	if(pmsgs.get() != NULL)
	{
		for(vector<string>::iterator it= pmsgs->begin(); it != pmsgs->end(); ++it)
			write(threadID, *it);
	}
	return 0;
}

void Terminal::ending()
{
	if(	!m_mqRunStrings.empty() ||
		m_pvFinishedStrings.get() != NULL	)
	{
		map<pid_t, SHAREDPTR::shared_ptr<vector<string> > >::iterator ths;

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "  WARNING: for Terminal object, some string exists inside buffer" << endl;
		cout << "--------------------------------------------------------------------------" << endl;
		if(m_pvFinishedStrings.get() != NULL)
		{
			for(finishedStrVec_temp::iterator it= m_pvFinishedStrings->begin(); it != m_pvFinishedStrings->end(); ++it)
			{
				for(vector<string>::iterator vit= it->second->begin(); vit != it->second->end(); ++vit)
					write(it->first, *vit);
				write(0, "");
			}
			if(!m_mqRunStrings.empty())
			{
				cout << endl;
				cout << "--------------------------------------------------------------------------" << endl;
			}
		}
		for(ths= m_mqRunStrings.begin(); ths != m_mqRunStrings.end(); ++ths)
			for(vector<string>::iterator it= ths->second->begin(); it != ths->second->end(); ++it)
				write(ths->first, *it);
		cout << endl;
		cout << "--------------------------------------------------------------------------" << endl;
		cout << "--------------------------------------------------------------------------" << endl;
	}
}

bool Terminal::isRegistered(pid_t threadID/*= 0*/)
{
	bool reg(false);

	if(threadID == 0)
		threadID= Thread::gettid();
	LOCK(m_PRINT);
	if(	m_pWorker.get() != NULL &&
		m_pWorker->running()		)
	{
		reg= m_pWorker->isRegistered(threadID);
		UNLOCK(m_PRINT);
		return reg;
	}
	if(find(m_qOrder.begin(), m_qOrder.end(), threadID) != m_qOrder.end())
		reg= true;
	UNLOCK(m_PRINT);
	return reg;
}

#ifdef TERMINALOUTPUT_ONLY_THRADIDS
void Terminal::end(string file, int line, const pid_t& threadID/*= 0*/)
#else
void Terminal::end(pid_t threadID/*= 0*/)
#endif
{
	bool bexec(false);
	map<pid_t, SHAREDPTR::shared_ptr<ostringstream> >::iterator it;
	map<pid_t, SHAREDPTR::shared_ptr<vector<string> > >::iterator found;
	vector<pid_t>::iterator foundTH;

	if(threadID == 0)
		threadID= Thread::gettid();
	LOCK(m_PRINT);
	if(	m_pWorker.get() != NULL &&
		m_pWorker->running()		)
	{
		it= m_sLastMsgStream.find(threadID);
		if(it != m_sLastMsgStream.end())
		{
			string msg(it->second->str());

			it->second->str("");
			if(msg != "")
				m_pWorker->read(threadID, msg);
		}
		m_pWorker->end(threadID);
		UNLOCK(m_PRINT);
		return;
	}
	found= m_mqRunStrings.find(threadID);
	it= m_sLastMsgStream.find(threadID);
	if(it != m_sLastMsgStream.end())
	{// can be only when no working thread created
		string msg(it->second->str());

		it->second->str("");
		if(msg != "")
		{
			if(found == m_mqRunStrings.end())
			{
				m_mqRunStrings[threadID]= SHAREDPTR::shared_ptr<vector<string> >(new vector<string>());
				found= m_mqRunStrings.find(threadID);
			}
			found->second->push_back(msg);
		}
	}
	if(found != m_mqRunStrings.end())
	{
		if(m_pvFinishedStrings.get() == NULL)
			m_pvFinishedStrings= finishedPtr_temp(new finishedStrVec_temp);
		if(	threadID == m_nActThreadID &&
			!m_pvFinishedStrings->empty() &&
			m_pvFinishedStrings->front().first != m_nActThreadID	)
		{
			// insert actual block on beginning
			m_pvFinishedStrings->push_front(
							pair<	pid_t,
								SHAREDPTR::shared_ptr<vector<string> > >
												(threadID, found->second)	);
		}else
		{
			m_pvFinishedStrings->push_back(
							pair<	pid_t,
								SHAREDPTR::shared_ptr<vector<string> > >
												(threadID, found->second)	);
		}
		m_mqRunStrings.erase(threadID);
		foundTH= find(m_qOrder.begin(), m_qOrder.end(), threadID);
		if(foundTH != m_qOrder.end())
			m_qOrder.erase(foundTH);
	}
	if(	_instance == this &&
		m_pvFinishedStrings.get() != NULL &&
		!m_qOrder.empty() &&
		!m_mqRunStrings[m_qOrder.front()]->empty()	)
	{// when no working thread created
		bexec= true;
	}
	if(	bexec == false &&
		m_nActThreadID == threadID	)
	{
		AROUSE(m_WORKINGCONDITION);
	}
	UNLOCK(m_PRINT);
	if(bexec)
		execute();// when no working thread created
}

int Terminal::stop(const bool *bWait/*= NULL*/)
{
	int res;

	res= Thread::stop(false);
	LOCK(m_PRINT);
	AROUSE(m_WORKINGCONDITION);
	UNLOCK(m_PRINT);
	if(	res != 0 ||
		bWait == NULL ||
		*bWait == false	)
	{
		return res;
	}
	return Thread::stop(bWait);
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
		if(_instance->m_pWorker.get() != NULL)
			_instance->m_pWorker->stop(true);
		_instance->ending();
		delete _instance;
		_instance= NULL;
	}
}
