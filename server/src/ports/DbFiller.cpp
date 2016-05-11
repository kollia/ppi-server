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

#include "../pattern/util/LogHolderPattern.h"

#include "../util/GlobalStaticMethods.h"
#include "../util/exception.h"

#include "../util/stream/ErrorHandling.h"

#include "../database/lib/DbInterface.h"

#include "DbFiller.h"

using namespace std;
using namespace ppi_database;

namespace util
{
	bool DbFiller::execute()
	{
		SHAREDPTR::shared_ptr<map<string, db_t>  > pDbQueue;
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > pMsgQueue;
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > allMsgQueue(new vector<sendingInfo_t>());
		SHAREDPTR::shared_ptr<debugSessionFolderMap> pDebugQueue;
		SHAREDPTR::shared_ptr<IDbFillerPattern::debugSessionFolderMap> allDebugQueue;

		allDebugQueue= SHAREDPTR::shared_ptr<IDbFillerPattern::debugSessionFolderMap>
						(new IDbFillerPattern::debugSessionFolderMap());
		LOCK(m_SENDQUEUELOCK);
		while(!m_bHasContent)
			CONDITION(m_SENDQUEUECONDITION, m_SENDQUEUELOCK);
		UNLOCK(m_SENDQUEUELOCK);

		do{
			m_oCache.getContent(pDbQueue, pMsgQueue, pDebugQueue);
			if(!pMsgQueue->empty())
				allMsgQueue->insert(allMsgQueue->end(), pMsgQueue->begin(), pMsgQueue->end());
			if(!pDebugQueue->empty())
				allDebugQueue->insert(pDebugQueue->begin(), pDebugQueue->end());
			// write all values with identif 'value' from command fillValue()
			sendDirect(pDbQueue, allMsgQueue, allDebugQueue);

		}while(	!pDbQueue->empty() ||
				!allMsgQueue->empty()	||
				!allDebugQueue->empty()	);
		return true;
	}

	void DbFiller::sendDirect(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
					SHAREDPTR::shared_ptr<debugSessionFolderMap>& debugQueue)
	{
		typedef map<string, db_t>::iterator subIt;
		typedef debugSessionFolderMap::iterator debugIt;
		typedef debugSessionSubroutineMap::iterator debugInnerIt;
		bool bError(false);
		/**
		 * when follow seconds
		 * getting no new debug session
		 * for sending
		 * set count to send by one pass (m_nSendingCount)
		 * back to beginning sending (m_nBeginSendingCount)
		 */
		const time_t nEmptySetBack(10);
		/**
		 * grow sending count (m_nSendingCount)
		 * with follow counts
		 * when debug session count
		 * greater than nGrowByMore
		 */
		const size_t nGrowCount(3);
		/**
		 * grow sending count (m_nSendingCount)
		 * when debug session count
		 * greater then follow counts
		 */
		const size_t nGrowByMore(50);
		DbInterface* db;
		size_t nDebugQueueSize;
		ppi_time currentTime;
		vector<sendingInfo_t>::iterator msgPos;
		vector<vector<sendingInfo_t>::iterator> sendedMsgs;
		vector<debugIt> sendedDebug;
		vector<string> answer;
		ErrorHandling err;

		/*
		 * secure option when to much entries
		 * inside debugQueue
		 * when more than 100 add 3 to do more by one pass
		 */
		currentTime.setActTime();
		nDebugQueueSize= debugQueue->size();
		if(nDebugQueueSize < msgQueue->size())
			nDebugQueueSize= msgQueue->size();
#if(__showSendingCount > 1)
		ostringstream out;
#endif // if(__showSendingCount > 1)

		if(nDebugQueueSize > 0)
		{
			if(nDebugQueueSize > nGrowByMore)
			{
#if(__showSendingCount > 1)
				out << "++ grow sending for " << nGrowCount << " counts";
				out << " because " << nGrowByMore << " was under run" << endl;
#endif // if(__showSendingCount > 1)
				m_nSendingCount+= nGrowCount;

			}else if(	m_nSendingCount > m_nBeginSendingCount &&
						nDebugQueueSize < nGrowByMore						)
			{
				ppi_time tcheck;

				tcheck.tv_sec= nEmptySetBack;
				if(ppi_time(currentTime - m_tLastDbgSend) >= tcheck)
				{
#if(__showSendingCount > 1)
					out << "-- set sending count back to begin, because longer then ";
					out << tcheck.toString(false) << " seconds no message come in" << endl;
#endif // if(__showSendingCount > 1)
					m_nSendingCount= m_nBeginSendingCount;

				}else
				{
					if(m_nSendingCount < (m_nBeginSendingCount + nGrowCount + 1))
					{
#if(__showSendingCount > 1)
						out << "-- set sending count back to begin by low count\n";
#endif // if(__showSendingCount > 1)
						m_nSendingCount= m_nBeginSendingCount;
					}else
					{
#if(__showSendingCount > 1)
						out << "-- make sending count lower of ";
						out << nGrowCount << " counts";
						out << " because " << nGrowByMore << " was overrun" << endl;
#endif // if(__showSendingCount > 1)
						m_nSendingCount-= nGrowCount;
					}
				}
			}// if( nDebugQueueSize < nGrowByMore )
		}// if(currentTime >= tGrowCheck)
		if(nDebugQueueSize > 0)
			m_tLastDbgSend= currentTime;
#if(__showSendingCount > 1)
		if(nDebugQueueSize > 0)
		{
			out << "exist " << nDebugQueueSize << " msg entries";
			out << ", sending " << m_nSendingCount;
			out << " [" << Thread::gettid() << "] ";
			out << currentTime.toString(true) << endl;
			cout << out.str();
		}
#endif // if(__showSendingCount > 1)


		/*
		 * fill into database
		 */
		db= DbInterface::instance();
		for(subIt sIt= dbQueue->begin(); sIt != dbQueue->end(); ++sIt)
		{
			OMethodStringStream command("fillValue");

			command << sIt->second.folder;
			command << sIt->second.subroutine;
			command << sIt->second.identif;
			command << sIt->second.bNew;
			for(vector<double>::const_iterator it= sIt->second.values.begin(); it != sIt->second.values.end(); ++it)
				command << *it;
			answer= db->sendMethodD("ppi-db-server", command, "", /*answer*/true);
			for(vector<string>::iterator answ= answer.begin(); answ != answer.end(); ++answ)
			{
				err.setErrorStr(*answ);
				if(err.fail())
				{// ending only by errors (no warnings)
					int log;
					string msg;

					err.addMessage("DbFiller", "sendToDatabase", msgPos->method
									+ "@" + sIt->second.folder + "@" + sIt->second.subroutine);
					msg= err.getDescription();
					if(err.hasError())
					{
						log= LOG_ERROR;
						cerr << glob::addPrefix("### ERROR: ", msg) << endl;
					}else
					{
						log= LOG_WARNING;
						cout << glob::addPrefix("### WARNING: ", msg) << endl;
					}
					LOGEX(log, msg, &m_oCache);
					if(log == LOG_ERROR)
					{
						bError= true;
						break;
					}
				}
			}
			if(bError)
				break;
		}
		if(!bError)
		{
			dbQueue->clear();
			for(msgPos= msgQueue->begin(); msgPos != msgQueue->end(); ++msgPos)
			{
				OMethodStringStream method(msgPos->method);

				answer= db->sendMethodD(msgPos->toProcess, method, msgPos->done, /*answer*/true);
				for(vector<string>::iterator answ= answer.begin(); answ != answer.end(); ++answ)
				{
					err.setErrorStr(*answ);
					if(err.fail())
					{// ending only by errors (no warnings)
						int log;
						string msg;

						err.addMessage("DbFiller", "sendDatabase", msgPos->method);
						msg= err.getDescription();
						if(err.hasError())
						{
							log= LOG_ERROR;
							cerr << glob::addPrefix("### ERROR: ", msg) << endl;
						}else
						{
							log= LOG_WARNING;
							cout << glob::addPrefix("### WARNING: ", msg) << endl;
						}
						LOGEX(log, msg, &m_oCache);
						if(log == LOG_ERROR)
						{
							bError= true;
							break;
						}
					}
				}
				if(bError)
					break;
				/*
				 * send only nSendingCount messages
				 * because database entries from dbQueue
				 * are more important
				 */
				sendedMsgs.push_back(msgPos);
				if(sendedMsgs.size() >= m_nSendingCount)
					break;
			}
			for(vector<vector<sendingInfo_t>::iterator>::iterator it= sendedMsgs.begin();
							it != sendedMsgs.end(); ++it								)
			{
				msgQueue->erase(*it);
			}
		}
		if(!bError)
		{
			for(debugIt dIt= debugQueue->begin(); dIt != debugQueue->end(); ++dIt)
			{
				for(debugInnerIt dIIt= dIt->second.begin(); dIIt != dIt->second.end(); ++dIIt)
				{
#if( __showSendingCount == 1 || __showSendingCount == 3 )
					if(dIIt->second.subroutine == "#setDebug")
					{
						for(map<string, short>::iterator it= m_mFolderCount.begin();
										it != m_mFolderCount.end(); ++it				)
						{
							cout << "sending " << it->second << " "
											<< it->first << " folder #start to database" << endl;
						}
						m_mFolderCount.clear();

					}else if(dIIt->second.subroutine == "#start")
					{
						short count(0);

						if(m_mFolderCount.find(dIIt->second.folder) != m_mFolderCount.end())
							count= m_mFolderCount[dIIt->second.folder];
						++count;
						m_mFolderCount[dIIt->second.folder]= count;
					}
#endif // #if(__showSendingCount == 1 || __showSendingCount == 3)
					if(!db->fillDebugSession(dIIt->second, /*answer*/true))
					{
						bError= true;
						break;
					}
				}
				if(bError)
					break;
				/*
				 * send only nSendingCount debug information
				 * because database entries from dbQueue
				 * are more important
				 */
				sendedDebug.push_back(dIt);
				if(sendedDebug.size() >= m_nSendingCount)
					break;
			}
			for(vector<debugIt>::iterator it= sendedDebug.begin();
							it != sendedDebug.end(); ++it		)
			{
				debugQueue->erase(*it);
			}
		}
	}

	void DbFiller::informDatabase()
	{
		LOCK(m_SENDQUEUELOCK);
		if(m_bHasContent)
			AROUSE(m_SENDQUEUECONDITION);
		UNLOCK(m_SENDQUEUELOCK);
	}

	void DbFiller::getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
					SHAREDPTR::shared_ptr<debugSessionFolderMap>& debugQueue)
	{
		m_oCache.getContent(dbQueue, msgQueue, debugQueue);
	}

	EHObj DbFiller::stop(const bool *bWait/*= NULL*/)
	{
		m_pError= Thread::stop(/*wait*/false);
		AROUSE(m_SENDQUEUECONDITION);
		if(	bWait &&
			*bWait == true &&
			!m_pError->hasError()	)
		{
			(*m_pError)= Thread::stop(bWait);
		}
		return m_pError;
	}

	DbFiller::~DbFiller()
	{
		DESTROYMUTEX(m_SENDQUEUELOCK);
		DESTROYCOND(m_SENDQUEUECONDITION);
	}

} /* namespace ppi_database */
