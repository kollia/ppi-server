/**
 *   This file 'NoAnswerSender.cpp' is part of ppi-server.
 *   Created on: 02.11.2013
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

#include "../util/thread/Terminal.h"

#include "../util/exception.h"

#include "Informer.h"

using namespace std;
using namespace boost::algorithm;

namespace util
{
	void Informer::informFolders(const folders_t& folders, const string& from, const string& as,
											const bool debug, pthread_mutex_t *lock)
	{
		pid_t threadID(0);
		inform_t inform;
		SHAREDPTR::shared_ptr<folders_t> pfolders;

		if(m_bisRunn == false)
		{
			if(debug)
				threadID= Thread::gettid();
			informing(folders, from, as, threadID, lock);
			return;
		}
		LOCK(m_INFORMQUEUELOCK);
		pfolders= SHAREDPTR::shared_ptr<folders_t>(new folders_t);
		*pfolders= folders;
		inform.ownSubroutine= as;
		inform.from= from;
		inform.folders= pfolders;
		inform.debug= debug;
		if(debug)
			inform.threadID= Thread::gettid();
		inform.OBSERVERLOCK= lock;
		m_apvtFolders->push_back(inform);
		AROUSE(m_INFORMQUEUECONDITION);
		UNLOCK(m_INFORMQUEUELOCK);
	}

	void Informer::informing(const folders_t& folders, const string& from, const string& as,
										const pid_t& threadId, pthread_mutex_t *lock)
	{
		vector<string> spl;
		string sOwn(m_sFolder + ":" + as);
		ostringstream output;
		unsigned short count(m_poMeasurePattern->getActCount(as));

		split(spl, from, is_any_of(":"));
		LOCK(lock);
		if(	spl[0] == "e" ||
			(	spl[0] == "i" &&
				(	spl[1] != m_sFolder ||
					(	spl[1] == m_sFolder &&
						count < m_poMeasurePattern->getActCount(spl[2])	)	)	)	)
		{// inform own folder to restart
		 // when setting was from other folder, outside any folder,
		 // or in same folder, subroutine was from an later one
			m_poMeasurePattern->changedValue(m_sFolder, from.substr(2));
		}
		if(threadId > 0)
		{
			if(folders.size() > 0)
			{
				output << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\" << endl;
				output << "  " << sOwn << " was changed" << endl;
				if(	spl[0] != "i" ||
					from.substr(2) != sOwn		)
				{
					output << "  was informed from";
					if(from.substr(0, 1) == "e")
						output << " internet account ";
					else
						output << ": ";
					output << from.substr(2) << endl;
				}
			}
		}
		for(folders_t::const_iterator it= folders.begin(); it != folders.end(); ++it)
		{
			for(vector<string>::const_iterator fit= it->second.begin(); fit != it->second.end(); ++fit)
			{
				string::size_type pos;

				pos= fit->find(" ");
				if(	from.substr(0, 1) != "i" ||
					(	(	pos != string::npos &&
							from.substr(2) != fit->substr(0, pos)	) ||
						(	pos == string::npos &&
							from.substr(2) != *fit	) 					)	)
				{
					if(threadId > 0)
						output << "    inform " << *fit << endl;
					it->first->changedValue(*fit, sOwn);

				}else if(threadId > 0)
					output << "    do not inform " << *fit << " back" << endl;
				break;
			}
		}
		if(threadId > 0 && folders.size() > 0)
		{
			bool registeredThread;

			output << "////////////////////////////////////////" << endl;
			registeredThread= Terminal::instance()->isRegistered(threadId);
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
			*Terminal::instance()->out(threadId) << output.str();
			if(!registeredThread)
				Terminal::instance()->end(threadId);
#else
			*Terminal::instance()->out(__FILE__, __LINE__, threadId) << output.str();
			if(!registeredThread)
				Terminal::instance()->end(__FILE__, __LINE__, threadId);
#endif
		}
		UNLOCK(lock);
	}

	int Informer::execute()
	{
		pid_t threadId;
		std::auto_ptr<vector<inform_t> > pInform;

		LOCK(m_INFORMQUEUELOCK);
		if(m_apvtFolders->size() == 0)
		{
			CONDITION(m_INFORMQUEUECONDITION, m_INFORMQUEUELOCK);
		}
		pInform= m_apvtFolders;
		m_apvtFolders= std::auto_ptr<vector<inform_t> >(new vector<inform_t>);
		UNLOCK(m_INFORMQUEUELOCK);

		if(stopping())
			return 0;
		for(vector<inform_t>::iterator it= pInform->begin(); it != pInform->end(); ++it)
		{
			threadId= 0;
			if(it->debug)
				threadId= it->threadID;
			informing(*it->folders, it->from, it->ownSubroutine, threadId, it->OBSERVERLOCK);
		}
		return 0;
	}

	int Informer::stop(const bool *bWait/*= NULL*/)
	{
		int res;

		Thread::stop(/*wait*/false);
		AROUSE(m_INFORMQUEUECONDITION);
		res= Thread::stop(bWait);
		return res;
	}

	Informer::~Informer()
	{
		DESTROYMUTEX(m_INFORMQUEUELOCK);
		DESTROYCOND(m_INFORMQUEUECONDITION);
	}

} /* namespace ppi_database */
