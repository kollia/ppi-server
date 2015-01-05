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

#include "../pattern/util/IDbgSessionPattern.h"

#include "../util/stream/ppivalues.h"

#include "../util/thread/Terminal.h"

#include "../util/exception.h"
#include "../util/debugsubroutines.h"

#include "Informer.h"

using namespace std;
using namespace boost::algorithm;
using namespace design_pattern_world::util_pattern;

namespace util
{
	void Informer::informFolders(const folders_t& folders, const InformObject& from,
					const string& as, const bool debug, pthread_mutex_t *lock)
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
		pfolders= SHAREDPTR::shared_ptr<folders_t>(new folders_t);
		*pfolders= folders;
		inform.ownSubroutine= as;
		inform.from= from;
		inform.folders= pfolders;
		inform.debug= debug;
		if(debug)
			inform.threadID= Thread::gettid();
		inform.OBSERVERLOCK= lock;
		LOCK(m_INFORMQUEUELOCK);
		m_apvtFolders->push_back(inform);
		UNLOCK(m_INFORMQUEUELOCK);
	}

	void Informer::arouseInformerThread()
	{
		if(m_bisRunn == false)
			return;
		LOCK(m_INFORMQUEUELOCK);
		if(!m_apvtFolders->empty())
			AROUSE(m_INFORMQUEUECONDITION);
		UNLOCK(m_INFORMQUEUELOCK);
	}

	void Informer::informing(const folders_t& folders, const InformObject& from,
					const string& as, const pid_t& threadId, pthread_mutex_t *lock)
	{
		vector<string> spl;
		string sOwn(m_sFolder + ":" + as);
		string sFromWho(from.getWhoDescription());
		ostringstream output;
		InformObject::posPlace_e place(from.getDirection());
		unsigned short count(m_poMeasurePattern->getActCount(as));

		split(spl, sFromWho, is_any_of(":"));
		LOCK(lock);
		if(	place != InformObject::INTERNAL ||
			spl[0] != m_sFolder ||
			count < m_poMeasurePattern->getActCount(spl[1])	)
		{// inform own folder to restart
		 // when setting was from other folder, outside own folder,
		 // or in same folder, but subroutine was from an later one
			m_pOwnInformerCache->changedValue(m_sFolder, from);
		}
		if(	threadId > 0 &&
			folders.size() > 0	)
		{// thread id only set by debugging session
			output << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\" << endl;
			output << "  " << sOwn << " was changed" << endl;
			if(	place == InformObject::INTERNAL ||
				from.getWhoDescription() != sOwn	)
			{
				output << "  was informed from ";
				output << from.toString() << ":" << endl;
			}
		}
		for(folders_t::const_iterator it= folders.begin(); it != folders.end(); ++it)
		{
			for(vector<string>::const_iterator fit= it->second.begin(); fit != it->second.end(); ++fit)
			{
				string::size_type pos;

				pos= fit->find(" ");
				if(	place == InformObject::INTERNAL ||
					(	(	pos != string::npos &&
							sFromWho != fit->substr(0, pos)	) ||
						(	pos == string::npos &&
							sFromWho != *fit		)				)	)
				{
					if(threadId > 0)
						output << "    inform " << *fit << endl;
					it->first->changedValue(*fit, InformObject(place, sOwn));

				}else if(threadId > 0)
					output << "    do not inform " << *fit << " back" << endl;
				break;
			}
		}
		if(	threadId > 0 &&
			folders.size() > 0	)
		{
			output << "////////////////////////////////////////" << endl;
#if ( __DEBUGSESSIONOutput == debugsession_SERVER || __DEBUGSESSIONOutput == debugsession_BOTH)
			bool registeredThread;

			registeredThread= Terminal::instance()->isRegistered(threadId);
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
			*Terminal::instance()->out(threadId) << output.str();
			if(!registeredThread)
				Terminal::instance()->end(threadId);
#else // TERMINALOUTPUT_ONLY_THRADIDS
			*Terminal::instance()->out(__FILE__, __LINE__, threadId) << output.str();
			if(!registeredThread)
				Terminal::instance()->end(__FILE__, __LINE__, threadId);
#endif // TERMINALOUTPUT_ONLY_THRADIDS
#endif
#if ( __DEBUGSESSIONOutput == debugsession_CLIENT || __DEBUGSESSIONOutput == debugsession_BOTH)
			IDbgSessionPattern::dbgSubroutineContent_t content;

			/**
			 * place of new definition of content are:
			 * ProcessChecker::execute by method == "debugSubroutine"
			 * Informer::informing
			 * MeasureThread::setDebug on method end
			 * MeasureThread::execute by 2 times
			 * MeasureThread::doDebugStartingOutput
			 * MeasureThread::checkToStart
			 * portBase::writeDebugStream
			 * ServerDbTransaction::transfer by method == "fillDebugSession"
			 */
			content.folder= m_sFolder;
			content.subroutine= as;
			content.value= 0;
			content.currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time);
			content.content= output.str();
			m_poMeasurePattern->fillDebugSession(content);
#endif
		}
		UNLOCK(lock);
	}

	bool Informer::execute()
	{
		pid_t threadId;
		std::auto_ptr<vector<inform_t> > pInform;

		LOCK(m_INFORMQUEUELOCK);
		while(	m_apvtFolders->size() == 0 &&
				!stopping()						)
		{
			CONDITION(m_INFORMQUEUECONDITION, m_INFORMQUEUELOCK);
		}
		pInform= m_apvtFolders;
		m_apvtFolders= std::auto_ptr<vector<inform_t> >(new vector<inform_t>);
		UNLOCK(m_INFORMQUEUELOCK);

		if(stopping())
			return false;
		for(vector<inform_t>::iterator it= pInform->begin(); it != pInform->end(); ++it)
		{
			threadId= 0;
			if(it->debug)
				threadId= it->threadID;
			informing(*it->folders, it->from, it->ownSubroutine, threadId, it->OBSERVERLOCK);
		}
		return true;
	}

	EHObj Informer::stop(const bool *bWait/*= NULL*/)
	{
		m_pError= Thread::stop(/*wait*/false);
		AROUSE(m_INFORMQUEUECONDITION);
		if(	!m_pError->hasError() &&
			bWait &&
			*bWait == true			)
		{
			(*m_pError)= Thread::stop(bWait);
		}
		return m_pError;
	}

	Informer::~Informer()
	{
		DESTROYMUTEX(m_INFORMQUEUELOCK);
		DESTROYCOND(m_INFORMQUEUECONDITION);
	}

} /* namespace ppi_database */
