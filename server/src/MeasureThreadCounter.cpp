/**
 *   This file 'MeasureThreadCounter.cpp' is part of ppi-server.
 *   Created on: 05.02.2014
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

#include "util/thread/Terminal.h"

#include "ports/measureThread.h"

#include "MeasureThreadCounter.h"

string MeasureThreadCounter::getTimeString(int seconds, int* wait/*= NULL*/)
{
	short write(0);
	int secs, onlySec(0);
	string tmStr;
	ostringstream tm;

	secs= seconds % 60;
	if(secs > 0)
	{
		onlySec= secs;
		tm << secs << " second";
		if(secs > 1)
			tm << "s";
		tm << " ";
		tmStr= tm.str();
		tm.str("");
		++write;
		if(wait)
		{
			if(secs > 15)
			{
				*wait= 0;
				if(secs >= 30)
					*wait= 15;
				*wait+= (onlySec % 15);

			}else if(secs > 5)
			{
				*wait= onlySec - 5;
			}else
				*wait= 1;
		}
	}
	seconds-= secs;
	if(seconds > 0)
	{
		secs= seconds % 3600;
		if(secs > 0)
		{
			secs/= 60;
			if(wait)
			{
				if(secs == 1)
					*wait= 30 + (onlySec % 30);
				else
					*wait= 60 + (onlySec % 60);
			}
			tm << secs << " minute";
			if(secs > 1)
				tm << "s";
			tm << " ";
			if(write == 1)
				tm <<  "and ";
			tmStr= tm.str() + tmStr;
			tm.str("");
			++write;
			seconds-= (secs * 60);
		}
		if(seconds > 0)
		{
			secs= seconds / 3600;
			if(wait)
				*wait= 60 + (onlySec % 60);
			if(secs > 0)
			{
				tm << secs << " hour";
				if(secs > 1)
					tm << "s";
				tm << " ";
				if(write == 1)
					tm <<  "and ";
				tmStr= tm.str() + tmStr;
				tm.str("");
				++write;
				seconds-= (secs * 3600);
			}

		}
	}
	return tmStr;
}

void MeasureThreadCounter::beginCount()
{
	SHAREDPTR::shared_ptr<meash_t> pCurrent;

	pCurrent= meash_t::firstInstance;
	while(pCurrent)
	{
		pCurrent->pMeasure->beginCounting();
		pCurrent= pCurrent->next;
	}
}

void MeasureThreadCounter::outputCounting(int seconds)
{
	bool bCount(false);
	int nCount;
	SHAREDPTR::shared_ptr<meash_t> pCurrent;

	pCurrent= meash_t::firstInstance;
	tout << "inside time of " << getTimeString(seconds) << endl;
	while(pCurrent)
	{
		nCount= pCurrent->pMeasure->getRunningCount();
		if(nCount > 0)
		{
			bCount= true;
			tout << "folder " << pCurrent->pMeasure->getFolderName() <<
							" running in " << nCount << " times" << endl;
		}
		pCurrent= pCurrent->next;
	}
	if(!bCount)
		tout << "no folder running in given time" << endl;
	TERMINALEND;
}

void MeasureThreadCounter::clientAction()
{
	LOCK(m_CLIENTACTIONMUTEX);
	if(!m_bCounting)
	{// activate only when object do not wait for counting
	 // because by waiting for client action the action will be set
	 // only when MeasureThreadCounter has informed all MeasureThread's to count
		m_bClientActionDone= true;
		AROUSE(m_CLIENTACTIONCOND);
		CONDITION(m_CLIENTACTIONCOND, m_CLIENTACTIONMUTEX);
	}
	UNLOCK(m_CLIENTACTIONMUTEX);
}

short MeasureThreadCounter::runnable()
{
	int seconds(m_nSeconds), wait;

	if(m_bClientAction)
	{
		tout << "server wait for some action from any client" << endl;
		tout << "and count after that ";
	}else
		tout << "server count now ";
	tout << "between " << getTimeString(seconds, &wait) << "for running folder threads." << endl;
	TERMINALEND;
	if(m_bClientAction)
	{
		LOCK(m_CLIENTACTIONMUTEX);
		m_bCounting= false;
		while(!m_bClientActionDone)
		{
			CONDITION(m_CLIENTACTIONCOND, m_CLIENTACTIONMUTEX);
			if(stopping())
			{
				m_bCounting= true;
				UNLOCK(m_CLIENTACTIONMUTEX);
				return 1;
			}
		}
		beginCount();
		AROUSEALL(m_CLIENTACTIONCOND);
		m_bCounting= true;
		UNLOCK(m_CLIENTACTIONMUTEX);
		tout << "wait now for " << getTimeString(seconds, &wait) << endl;
		TERMINALEND;
	}else
		beginCount();
	do{
		//cout << "sleep for " << wait << " seconds" << endl;
		if(SLEEP(wait) != ETIMEDOUT)
		{
			outputCounting(m_nSeconds - seconds);
			return 1;
		}
		seconds-= wait;
		if(seconds > 0)
		{
			tout << "wait for " << getTimeString(seconds, &wait) << endl;
			TERMINALEND;
		}
	}while(seconds > 0);
	outputCounting(m_nSeconds);
	return 1;
}

int MeasureThreadCounter::stop(const bool *bWait)
{
	int nRv;

	nRv= Thread::stop(false);
	LOCK(m_CLIENTACTIONMUTEX);
	AROUSE(m_CLIENTACTIONCOND);
	UNLOCK(m_CLIENTACTIONMUTEX);
	if(	bWait &&
		*bWait	)
	{
		nRv= Thread::stop(bWait);
	}
	return nRv;
}










