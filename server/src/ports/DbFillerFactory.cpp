/**
 *   This file 'DbFillerFactory.cpp' is part of ppi-server.
 *   Created on: 15.07.2014
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

#include "DbFillerFactory.h"
#include "DbFillerCache.h"

#include "../util/debug.h"
#include "../util/GlobalStaticMethods.h"

#include "../util/stream/ErrorHandling.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/logger/lib/logstructures.h"

namespace util
{
	DbFillerFactory* DbFillerFactory::__instance= NULL;

	SHAREDPTR::shared_ptr<IDbFillerPattern> DbFillerFactory::getInstance(const string& threadName, short threads)
	{
		DbFiller* pDbFiller(NULL);
		SHAREDPTR::shared_ptr<IDbFillerPattern> oRv;
		ErrorHandling errHandle;

		if(threads > 0)
		{
			/*
			 * when folder_db_threads inside server.conf
			 * defined with ONE (threads= 1)
			 * need an single pattern of DbFillerFactory
			 * and create for all measureThread objects
			 * only an DbFillerCache
			 */
			if(__instance == NULL)
			{
				__instance= new DbFillerFactory();
				if(__instance)
				{
					errHandle= __instance->start();
					if(errHandle.fail())
					{
						int log;
						string msg;

						errHandle.addMessage("DbFillerFactory", "start");
						msg= errHandle.getDescription();
						if(errHandle.hasError())
						{
							log= LOG_ERROR;
							cerr << glob::addPrefix("### ERROR: ", msg) << endl;
							delete __instance;
							__instance= NULL;
							threads= -1;
						}else
						{
							log= LOG_WARNING;
							cout << glob::addPrefix("### ERROR: ", msg) << endl;
						}
						LOG(log, msg);
					}
				}else
				{
					string msg;

					errHandle.setError("DbFillerFactory", "create_object");
					msg= errHandle.getDescription();
					cout << glob::addPrefix("### ERROR: ", msg) << endl;
					LOG(LOG_ERROR, msg);
					threads= -1;
				}
			}
			if(!errHandle.hasError())
			{
				oRv= __instance->createCache(threadName);
				return oRv;
			}
		}

		if(threads < 0)
		{
			/*
			 * when folder_db_threads inside server.conf
			 * defined with DIRECT (threads= -1)
			 * or EVERY (threads= 0)
			 * all measureThread objects need one DbFiller
			 */
			oRv= SHAREDPTR::shared_ptr<IDbFillerPattern>(new DbFiller(threadName));
			if(threads > -1)
			{
				/*
				 * when folder_db_threads inside server.conf
				 * defined with EVERY (threads= 0)
				 * DbFiller for all measureThread objects
				 * be an own thread instance and need to start
				 */
				pDbFiller= dynamic_cast<DbFiller*>(oRv.get());
				if(pDbFiller != NULL)
				{
					errHandle= pDbFiller->start();
					if(errHandle.fail())
					{
						int log;
						string msg;

						errHandle.addMessage("DbFiller", "start");
						msg= errHandle.getDescription();
						if(errHandle.hasError())
						{
							log= LOG_ERROR;
							cerr << glob::addPrefix("### ERROR: ", msg) << endl;
							delete __instance;
							__instance= NULL;
						}else
						{
							log= LOG_WARNING;
							cout << glob::addPrefix("### ERROR: ", msg) << endl;
							errHandle.clear();
						}
						LOG(log, msg);
					}
				}
			}
		}
		return oRv;
	}

	SHAREDPTR::shared_ptr<IDbFillerPattern> DbFillerFactory::createCache(const string& sfor)
	{
		string name("DbFillerCache_for_" + sfor);
		SHAREDPTR::shared_ptr<IDbFillerPattern> oRv;

		oRv= SHAREDPTR::shared_ptr<IDbFillerPattern>(new DbFillerCache(name, this));
		oRv->isRunning();
		LOCK(m_READCACHE);
		m_mCaches[name]= oRv;
		UNLOCK(m_READCACHE);
		return oRv;
	}

	void DbFillerFactory::informDatabase()
	{
		if(TRYLOCK(m_INFORMDATABASEMUTEX) == 0)
		{
			/*
			 *  when informer mutex already locked,
			 *  an other folder thread do this job
			 */
			AROUSE(m_INFORMDATABASECOND);
			UNLOCK(m_INFORMDATABASEMUTEX);
		}
	}

	bool DbFillerFactory::execute()
	{
		typedef map<string, SHAREDPTR::shared_ptr<IDbFillerPattern> >::iterator cacheIt;
		struct timespec wait;
		bool bWritten(true);
		bool bFirstOff(true);// <- set true because on first time should wait also 1 second
		SHAREDPTR::shared_ptr<map<string, db_t> > dbQueue;
		vector<SHAREDPTR::shared_ptr<map<string, db_t> > > allDbQueue;
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > msgQueue, allMsgQueue(new vector<sendingInfo_t>());

		wait.tv_sec= 1;
		wait.tv_nsec= 0;
		while(!stopping())
		{
			while(bWritten)
			{
				LOCK(m_READCACHE);
				for(cacheIt it= m_mCaches.begin(); it != m_mCaches.end(); ++it)
				{
					it->second->getContent(dbQueue, msgQueue);
					if(!dbQueue->empty())
						allDbQueue.push_back(dbQueue);
					if(!msgQueue->empty())
						allMsgQueue->insert(allMsgQueue->end(), msgQueue->begin(), msgQueue->end());
				}
				UNLOCK(m_READCACHE);
				if(	!allDbQueue.empty() ||
					!allMsgQueue->empty()	)
				{
					SHAREDPTR::shared_ptr<map<string, db_t> > wDbQueue(new map<string, db_t>());

					if(!allDbQueue.empty())
					{
						for(vector<SHAREDPTR::shared_ptr<map<string, db_t> > >::iterator it= allDbQueue.begin();
										it != allDbQueue.end(); ++it)
						{
							m_oDbFiller.sendDirect(*it, allMsgQueue);
							allMsgQueue->clear();
						}
					}else
						m_oDbFiller.sendDirect(wDbQueue, allMsgQueue);
					allMsgQueue->clear();
					allDbQueue.clear();
					bWritten= true;
					bFirstOff= true;
				}else
					bWritten= false;
			}// while(bWritten)
			if(stopping())
				return false;
			LOCK(m_INFORMDATABASEMUTEX);
			if(bFirstOff)
				RELTIMECONDITION(m_INFORMDATABASECOND, m_INFORMDATABASEMUTEX, &wait);
			else
				CONDITION(m_INFORMDATABASECOND, m_INFORMDATABASEMUTEX);
			UNLOCK(m_INFORMDATABASEMUTEX);
			bWritten= true;
		}
		return true;
	}

	EHObj DbFillerFactory::stop(const bool *bWait/*= NULL*/)
	{
		m_pError= Thread::stop(false);
		AROUSE(m_INFORMDATABASECOND);
		if(	bWait &&
			*bWait == true &&
			!m_pError->hasError()	)
		{
			(*m_pError)= Thread::stop(bWait);
		}
		return m_pError;
	}
}
