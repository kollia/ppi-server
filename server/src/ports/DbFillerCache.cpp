/**
 *   This file 'DbFillerCache.cpp' is part of ppi-server.
 *   Created on: 16.07.2014
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

#include "DbFillerCache.h"

#include "../util/stream/ErrorHandling.h"

#include "../database/lib/DbInterface.h"

namespace util
{
	using namespace ppi_database;

	string DbFillerCache::sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer/*= true*/)
	{
		vector<string> res;

		res= sendMethod(toProcess, method, "", answer);
		if(res.size() == 0)
			return "";
		return res[0];
	}
	vector<string> DbFillerCache::sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer/*= true*/)
	{
		sendingInfo_t sendInfo;
		vector<string> vsRv;
		DbInterface* db;

		if(	m_bisRunn &&
			answer == false	)
		{
			bool bFillFirstContainer(true), bFilledError(false);

			if(method.getMethodName() == "fillValue")
			{
				IMethodStringStream out(method.str());
				string folder, subroutine, identif;

				out >> folder;
				out >> subroutine;
				out >> identif;
				if(identif == "value")
				{
					bool bNew;
					double value;
					vector<double> values;

					out >> bNew;
					while(!out.fail())
					{
						out >> value;
						values.push_back(value);
					}
					fillValue(folder, subroutine, identif, values, bNew);
					if(TRYLOCK(m_SENDQUEUELOCK2) == 0)
					{
						if(m_sNoWaitError2 != "")
						{
							vsRv.push_back(m_sNoWaitError2);
							m_sNoWaitError2= "";
							bFilledError= true;
						}
						UNLOCK(m_SENDQUEUELOCK2);
					}
					if(!bFilledError)
					{
						if(TRYLOCK(m_SENDQUEUELOCK1) == 0)
						{
							if(m_sNoWaitError1 != "")
							{
								vsRv.push_back(m_sNoWaitError1);
								m_sNoWaitError1= "";
								bFilledError= true;
							}
							UNLOCK(m_SENDQUEUELOCK1);
						}
					}
					if(!bFilledError)
						vsRv.push_back("done");
					return vsRv;
				}

			}
			sendInfo.toProcess= "ppi-db-server";
			sendInfo.method= method.str();
			sendInfo.done= done;
			if(m_pHasContent == NULL)
			{
				/*
				 * cache running with dbFiller ONE
				 * set inside server.conf
				 *
				 * when dbFiller thread lock currently SENDQUELOCK2
				 * dbFiller thread was before inside SENDQUELOCK1
				 * and this locking array should be free now
				 * fill into m_vsSendingQueue1 container
				 * otherwise into m_vsSendingQueue2
				 */
				if(TRYLOCK(m_SENDQUEUELOCK2) == 0)
				{
					m_vsSendingQueue2->push_back(sendInfo);
					if(m_sNoWaitError2 != "")
					{
						vsRv.push_back(m_sNoWaitError2);
						m_sNoWaitError2= "";
						bFilledError= true;
					}
					UNLOCK(m_SENDQUEUELOCK2);
					bFillFirstContainer= false;
				}
			}
			if(bFillFirstContainer)
			{
				LOCK(m_SENDQUEUELOCK1);
				m_vsSendingQueue1->push_back(sendInfo);
				if(m_sNoWaitError1 != "")
				{
					vsRv.push_back(m_sNoWaitError1);
					m_sNoWaitError1= "";
					bFilledError= true;
				}
				if(m_pHasContent)
					*m_pHasContent= true;
				UNLOCK(m_SENDQUEUELOCK1);
			}
			if(!bFilledError)
				vsRv.push_back("done");
			if(m_dbInform)
				m_dbInform->informDatabase();

		}else // if(m_bisRunn && answer == false)
		{
			db= DbInterface::instance();
			vsRv= db->sendMethod(toProcess, method, done, answer);
		}
		return vsRv;
	}
	void DbFillerCache::fillValue(const string& folder, const string& subroutine, const string& identif,
					double value, bool bNew/*= false*/)
	{
		vector<double> values;

		values.push_back(value);
		fillValue(folder, subroutine, identif, values, bNew);
	}

	void DbFillerCache::fillDebugSession(const IDbgSessionPattern::dbgSubroutineContent_t& content)
	{
		DbInterface* db;
		OMethodStringStream command("fillValue");
		map<string, db_t>::iterator foundSub;
		sendingInfo_t sendInfo;
		db_t newEntry;
#if( __showSendingCount == 1 || __showSendingCount == 3 )
		map<string, pair<short, short> >::iterator foundFolderCount;

		if(content.subroutine == "#setDebug")
		{
			for(map<string, pair<short, short> >::iterator it= m_mFolderCount.begin();
							it != m_mFolderCount.end(); ++it				)
			{
				cout << "get debug sessions content " << it->second.first
								<< " in queue 1 and " << it->second.second
								<< " in queue 2 for folder " << it->first << endl;
			}
			m_mFolderCount.clear();

		}else if(content.subroutine == "#start")
		{
			foundFolderCount= m_mFolderCount.find(content.folder);
			if(foundFolderCount == m_mFolderCount.end())
			{
				m_mFolderCount[content.folder]= pair<short, short>(0, 0);
				foundFolderCount= m_mFolderCount.find(content.folder);
			}
		}
#endif // #if( __showSendingCount == 1 || __showSendingCount == 3 )
		if(m_bisRunn)
		{
//#define __showLOCK
#ifdef __showLOCK
			if(getThreadName() == "DbFillerThread_for_Raff1_Zeit")
			{
				ostringstream out;
				out << " Raff1_Zeit LOCK for filling " << identif << " ";
				for(vector<double>::const_iterator i= dvalues.begin(); i != dvalues.end(); ++i)
					out << *i << " ";
				out << " into " << folder << ":" << subroutine << endl;
				cout << out.str();
			}
#endif // __showLOCK

			bool bFillFirstContainer(true);
			debugSessionSubroutine timeSub(*content.currentTime, content.subroutine);
			debugSessionSubroutineMap::iterator foundTimer;
			debugSessionFolderMap::iterator foundFolder;

			if(m_pHasContent == NULL)
			{
				/*
				 * cache running with dbFiller ONE
				 * defined inside server.conf
				 *
				 * when dbFiller thread lock currently SENDQUELOCK2
				 * dbFiller thread was before inside SENDQUELOCK1
				 * and this locking array should be free now
				 * fill into m_apmtDebugSession1 container
				 * otherwise into m_apmtDebugSession2
				 */
				if(TRYLOCK(m_SENDQUEUELOCK2) == 0)
				{
#if( __showSendingCount == 1 || __showSendingCount == 3 )
					if(content.subroutine == "#start")
						++foundFolderCount->second.second;
#endif // #if( __showSendingCount == 1 || __showSendingCount == 3 )
					foundFolder= m_apmtDebugSession2->find(content.folder);
					if(foundFolder != m_apmtDebugSession2->end())
					{
						foundTimer= foundFolder->second.find(timeSub);
						if(foundTimer != foundFolder->second.end())
						{
							foundTimer->second.content+= content.content;
						}else
							(*m_apmtDebugSession2)[content.folder][timeSub]= content;
					}else
					{
						(*m_apmtDebugSession2)[content.folder][timeSub]= content;
					}
					UNLOCK(m_SENDQUEUELOCK2);
					bFillFirstContainer= false;
				}
			}
			if(bFillFirstContainer)
			{
				LOCK(m_SENDQUEUELOCK1);
#if( __showSendingCount == 1 || __showSendingCount == 3 )
				if(content.subroutine == "#start")
					++foundFolderCount->second.first;
#endif // #if( __showSendingCount == 1 || __showSendingCount == 3 )
				foundFolder= m_apmtDebugSession1->find(content.folder);
				if(foundFolder != m_apmtDebugSession1->end())
				{
					foundTimer= foundFolder->second.find(timeSub);
					if(foundTimer != foundFolder->second.end())
					{
						foundTimer->second.content+= content.content;
					}else
						(*m_apmtDebugSession1)[content.folder][timeSub]= content;
				}else
					(*m_apmtDebugSession1)[content.folder][timeSub]= content;
				if(m_pHasContent)
					*m_pHasContent= true;
				UNLOCK(m_SENDQUEUELOCK1);
			}
			if(m_dbInform)
				m_dbInform->informDatabase();
		}else //v if(m_bisRunn)
		{
			db= DbInterface::instance();
			db->fillDebugSession(content, /*answer*/false);
		}
	}

	void DbFillerCache::fillValue(const string& folder, const string& subroutine, const string& identif,
					const vector<double>& dvalues, bool bNew/*= false*/)
	{
		DbInterface* db;
		OMethodStringStream command("fillValue");
		map<string, db_t>::iterator foundSub;
		sendingInfo_t sendInfo;
		db_t newEntry;

		if(m_bisRunn)
		{
//#define __showLOCK
#ifdef __showLOCK
			if(getThreadName() == "DbFillerThread_for_Raff1_Zeit")
			{
				ostringstream out;
				out << " Raff1_Zeit LOCK for filling " << identif << " ";
				for(vector<double>::const_iterator i= dvalues.begin(); i != dvalues.end(); ++i)
					out << *i << " ";
				out << " into " << folder << ":" << subroutine << endl;
				cout << out.str();
			}
#endif // __showLOCK
			if(identif == "value")
			{
				bool bFillFirstContainer(true);

				if(m_pHasContent == NULL)
				{
					/*
					 * cache running with dbFiller ONE
					 * defined inside server.conf
					 *
					 * when dbFiller thread lock currently SENDQUELOCK2
					 * dbFiller thread was before inside SENDQUELOCK1
					 * and this locking array should be free now
					 * fill into m_apmtValueEntrys1 container
					 * otherwise into m_apmtValueEntrys2
					 */
					if(TRYLOCK(m_SENDQUEUELOCK2) == 0)
					{
						ppi_time tm;

						tm.setActTime();
						foundSub= m_apmtValueEntrys2->find(subroutine);
						if(foundSub == m_apmtValueEntrys2->end())
						{
							newEntry.folder= folder;
							newEntry.subroutine= subroutine;
							newEntry.identif= identif;
							newEntry.tm.tv_sec= tm.tv_sec;
							newEntry.tm.tv_usec= tm.tv_usec;
							newEntry.values= dvalues;
							newEntry.bNew= bNew;
							(*m_apmtValueEntrys2)[subroutine]= newEntry;

						}else
						{
							if(tm > foundSub->second.tm)
							{
								foundSub->second.values= dvalues;
								foundSub->second.tm.tv_sec= tm.tv_sec;
								foundSub->second.tm.tv_usec= tm.tv_usec;
								foundSub->second.bNew= bNew;
							}
						}
						UNLOCK(m_SENDQUEUELOCK2);
						bFillFirstContainer= false;
					}
				}
				if(bFillFirstContainer)
				{
					ppi_time tm;

					LOCK(m_SENDQUEUELOCK1);
					tm.setActTime();
					foundSub= m_apmtValueEntrys1->find(subroutine);
					if(foundSub == m_apmtValueEntrys1->end())
					{
						newEntry.folder= folder;
						newEntry.subroutine= subroutine;
						newEntry.identif= identif;
						newEntry.tm.tv_sec= tm.tv_sec;
						newEntry.tm.tv_usec= tm.tv_usec;
						newEntry.values= dvalues;
						newEntry.bNew= bNew;
						(*m_apmtValueEntrys1)[subroutine]= newEntry;

					}else
					{
						if(tm > foundSub->second.tm)
						{
							foundSub->second.values= dvalues;
							foundSub->second.tm.tv_sec= tm.tv_sec;
							foundSub->second.tm.tv_usec= tm.tv_usec;
							foundSub->second.bNew= bNew;
						}
					}
					if(m_pHasContent)
						*m_pHasContent= true;
					UNLOCK(m_SENDQUEUELOCK1);
				}
				if(m_dbInform)
					m_dbInform->informDatabase();
			}else
			{
				OMethodStringStream command("fillValue");

				command << folder;
				command << subroutine;
				command << identif;
				command << bNew;
				for(vector<double>::const_iterator it= dvalues.begin(); it != dvalues.end(); ++it)
					command << *it;
				sendMethod("ppi-db-server", command, /*answer*/false);
			}
		}else
		{
			db= DbInterface::instance();
			db->fillValue(folder, subroutine, identif, dvalues, bNew);
		}
	}

	void DbFillerCache::informDatabase()
	{
		bool bEntrys(false);

		LOCK(m_SENDQUEUELOCK1);
		if(	!m_vsSendingQueue1->empty() ||
			!m_apmtValueEntrys1->empty()		)
		{
			bEntrys= true;
		}
		UNLOCK(m_SENDQUEUELOCK1);
		if(bEntrys)
			m_dbInform->informDatabase();
	}

	EHObj DbFillerCache::remove()
	{
		EHObj handle(EHObj(new ErrorHandling));
		/*
		 * toDo: method to remove cache from queue
		 * 		 only needed when property inside server.conf is ONE
		 */
		return handle;
	}

	void DbFillerCache::getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& valQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
					SHAREDPTR::shared_ptr<debugSessionFolderMap>& debugQueue)
	{
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > newMsgQueue1(new vector<sendingInfo_t>());
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > newMsgQueue2(new vector<sendingInfo_t>());
		SHAREDPTR::shared_ptr<map<string, db_t>  > newValueEntrys1(new map<string, db_t>());
		SHAREDPTR::shared_ptr<map<string, db_t>  > newValueEntrys2(new map<string, db_t>());
		SHAREDPTR::shared_ptr<debugSessionFolderMap>
									newDebugQueue1(new debugSessionFolderMap());
		SHAREDPTR::shared_ptr<debugSessionFolderMap>
									newDebugQueue2(new debugSessionFolderMap());
		SHAREDPTR::shared_ptr<map<string, db_t> > valQueue2;
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > msgQueue1;
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > msgQueue2;
		SHAREDPTR::shared_ptr<debugSessionFolderMap> debugQueue1;
		SHAREDPTR::shared_ptr<debugSessionFolderMap> debugQueue2;
		map<string, db_t>::iterator found;

		LOCK(m_SENDQUEUELOCK1);
		valQueue= m_apmtValueEntrys1;
		m_apmtValueEntrys1= newValueEntrys1;
		msgQueue1= m_vsSendingQueue1;
		m_vsSendingQueue1= newMsgQueue1;
		debugQueue1= m_apmtDebugSession1;
		m_apmtDebugSession1= newDebugQueue1;
		if(m_pHasContent)
			*m_pHasContent= false;
		UNLOCK(m_SENDQUEUELOCK1);

		msgQueue->insert(msgQueue->end(), msgQueue1->begin(), msgQueue1->end());
		addDebugSession(debugQueue1, debugQueue);
		if(m_pHasContent != NULL)
		{
			/*
			 * cache running with dbFiller ONE
			 * defined inside server.conf
			 * and content was only written into
			 * m_vsSendingQueue1 and m_apmtValueEntrys1
			 * (mutex SENDQUEUELOCK2 wasn't defined)
			 */
			return;
		}
		LOCK(m_SENDQUEUELOCK2);
		msgQueue2= m_vsSendingQueue2;
		m_vsSendingQueue2= newMsgQueue2;
		valQueue2= m_apmtValueEntrys2;
		m_apmtValueEntrys2= newValueEntrys2;
		debugQueue2= m_apmtDebugSession2;
		m_apmtDebugSession2= newDebugQueue2;
		UNLOCK(m_SENDQUEUELOCK2);

		for(map<string, db_t>::iterator it= valQueue2->begin(); it != valQueue2->end(); ++it)
		{
			found= valQueue->find(it->first);
			if(found != valQueue->end())
			{	// overwrite entry only when have newer time
				if(ppi_time(it->second.tm) > found->second.tm)
					(*valQueue)[it->first]= it->second;
			}else // or do not exist
				(*valQueue)[it->first]= it->second;
		}
		msgQueue->insert(msgQueue->end(), msgQueue2->begin(), msgQueue2->end());
		addDebugSession(debugQueue2, debugQueue);
	}

	void DbFillerCache::addDebugSession(SHAREDPTR::shared_ptr<debugSessionFolderMap>& from,
									SHAREDPTR::shared_ptr<debugSessionFolderMap>& toExist	)
	{
		typedef debugSessionFolderMap::iterator folderQueueIt;
		typedef debugSessionSubroutineMap::iterator timerQueueIt;

		folderQueueIt foundFolder;
		timerQueueIt foundTimer;

		for(folderQueueIt it= from->begin(); it != from->end(); ++it)
		{
			foundFolder= toExist->find(it->first);
			if(foundFolder != toExist->end())
			{
				for(timerQueueIt tit= it->second.begin(); tit != it->second.end(); ++tit)
				{
					foundTimer= foundFolder->second.find(tit->first);
					if(foundTimer != foundFolder->second.end())
					{
						foundTimer->second.content+= tit->second.content;
					}else
						(*toExist)[it->first][tit->first]= tit->second;
				}
			}else
				toExist->insert(*it);
		}
	}

} /* namespace util */
