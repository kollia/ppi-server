/**
 *   This file 'MeasureInformerCash.cpp' is part of ppi-server.
 *   Created on: 13.08.2014
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

#include <climits>

#include "../database/logger/lib/logstructures.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../util/properties/PPIConfigFileStructure.h"
#include "../util/thread/Terminal.h"

#include "MeasureInformerCache.h"

namespace util
{
	MeasureInformerCache::MeasureInformerCache(const string& forFolder, IMeasurePattern* folderObj,
					SHAREDPTR::shared_ptr<IListObjectPattern> informOutput)
	: m_pFolderObject(folderObj),
	  m_sFolderName(forFolder),
	  m_oInformOutput(informOutput),
	  m_oInformeThread(folderObj->getFolderName(), "#informe_thread", "inform",
					  false, true, informOutput.get()),
	  m_CACHEVALUEMUTEX(Thread::getMutex("CACHEVALUEMUTEX"))
	{
		IMeasurePattern::awakecond_t awake;
		string statement(folderObj->getInformeThreadStatement());

		if(statement != "")
			m_oInformeThread.init(PPIConfigFileStructure::instance()->getWorkingList(), statement);
		m_vFolderCache=
			SHAREDPTR::shared_ptr<vector<pair<string, ppi_time> > >(new vector<pair<string, ppi_time> >());
		awake= folderObj->getAwakeConditions();
		m_WANTINFORM= awake.wantinform;
		m_ACTIVATETIME= awake.activatetime;
		m_VALUECONDITION= awake.valuecondition;
	}

	void MeasureInformerCache::changedValue(const string& folder, const string& from)
	{
		bool debug, bReg;
		double inform;
		ppi_time time;

#ifdef __followSETbehaviorFromFolder
		if(	m_btimer &&
			__followSETbehaviorFrom <= 4 &&
			__followSETbehaviorTo >= 4		)
		{
			vector<string> spl;

			split(spl, from, is_any_of(":"));
			if(	string(__followSETbehaviorFromFolder) == "" ||
				boost::regex_match(spl[0], m_oToFolderExp)		)
			{
				if(	string(__followSETbehaviorFromSubroutine) == "" ||
					boost::regex_match(spl[1], m_oToSubExp)				)
				{
					cout << "[4] informing from " << from << endl;
				}
			}
		}
#endif // __followSETbehaviorFromFolder

		if(!time.setActTime())
		{
			string msg("### DEBUGGING for folder " + getFolderName());

			msg+= "\n    ERROR: cannot calculate time to informing";
			msg+= "\n    " + time.errorStr();
			TIMELOGEX(LOG_ERROR, "setActTime_"+m_sFolderName, msg, m_pFolderObject->getExternSendDevice());
			if(m_pFolderObject->isDebug())
				tout << " ERROR: cannot calculate time to informing" << endl;
			time.clear();

		}
		if(!m_oInformeThread.isEmpty())
		{
			debug= m_pFolderObject->isDebug();
			if(debug)
			{
				if(m_oInformeThread.doOutput())
				{
					bReg= Terminal::instance()->isRegistered(Thread::gettid());
					m_oInformOutput->out() << "--------------------------------------------------------------" << endl;
					m_oInformOutput->out() << "t:" << time.toString(/*ad date*/true) << endl;
					m_oInformOutput->out() << "INFORMED folder " << getFolderName() << " from " << from << ":" << endl;
				}else
					debug= false;
			}
			m_oInformeThread.calculate(inform);
			if(debug)
			{
				m_oInformOutput->out() << "--------------------------------------------------------------" << endl;
				m_oInformOutput->writeDebugStream();
				if(!bReg)
					TERMINALEND;
			}
			if(inform == 0)
				return;
		}
		LOCK(m_CACHEVALUEMUTEX);
		m_vFolderCache->push_back(pair<string, ppi_time>(from, time));
		UNLOCK(m_CACHEVALUEMUTEX);

		if(TRYLOCK(m_WANTINFORM) == 0)
		{// try to inform folder to start,
		 // but when lock given to an other thread
		 // this other one do the job
			LOCK(m_ACTIVATETIME);
			AROUSE(m_VALUECONDITION);
			/*
			 * unlock WANTINFORM before ACTIVATETIME
			 * because otherwise when one folder inform
			 * own folder to start and thread slice ending before
			 * unlock WANTINFORM, after that running
			 * hole own folder and stop again inside condition
			 * no other folder inform again to restart
			 * because WANTINFORM was locked
			 */
			UNLOCK(m_WANTINFORM);
			UNLOCK(m_ACTIVATETIME);
		}
	}

	bool MeasureInformerCache::shouldStarting(const vector<ppi_time>& vStartTimes,
							map<short, vector<string> >& mInformed, bool* bLocked, bool debug)
	{
		bool bRv, bFullTimes;
		sharedinformvec_type newFolderCache, currentCache;
		ppi_time lastrun;

		bFullTimes= vStartTimes.size() == (SHRT_MAX-1) ? true : false;
		newFolderCache= sharedinformvec_type(new informvec_type());
		if(bFullTimes)
		{
			/*
			 * when vStartTimes has to much values inside
			 * wait always for lock,
			 * because then on ending all caches read
			 * vStartTimes will be cleared
			 */
			LOCK(m_CACHEVALUEMUTEX);
		}else if(TRYLOCK(m_CACHEVALUEMUTEX) != 0)
		{
			/*
			 * when CACHEVALUEMUTEX be locked
			 * other folder thread for which this informer cache be
			 * will be inform own folder thread to start
			 */
			if(bLocked)
				*bLocked= true;
			return true;
		}
		currentCache= m_vFolderCache;
		m_vFolderCache= newFolderCache;
		UNLOCK(m_CACHEVALUEMUTEX);
		if(bLocked)
			*bLocked= false;
		if(!vStartTimes.empty())
			lastrun= vStartTimes.back();
		if(	vStartTimes.empty() ||
			!lastrun.isSet()		)
		{
			if(debug)
			{
				for(informvec_type::iterator it= currentCache->begin(); it != currentCache->end(); ++it)
					mInformed[SHRT_MAX].push_back(it->first);
			}
			return !currentCache->empty();

		}
		bRv= false;
		for(informvec_type::iterator it= currentCache->begin(); it != currentCache->end(); ++it)
		{
			if(it->second > lastrun)
			{
				bRv= true;
				if(!debug)
					break;
				mInformed[SHRT_MAX].push_back(it->first);

			}else
			{
				if(debug)
				{
					short count(1);

					for(vector<ppi_time>::const_iterator tit= vStartTimes.begin();
									tit != vStartTimes.end(); ++tit					)
					{
						if(*tit > it->second)
						{
							mInformed[count].push_back(it->first);
							break;
						}
						++count;
					}
				}
			}
		}// foreach(currentCache)
		return bRv;
	}

} /* namespace util */