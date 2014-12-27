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
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > pMsgQueue;
		SHAREDPTR::shared_ptr<map<string, db_t>  > pDbQueue;
		SHAREDPTR::shared_ptr<map<string, map<pair<ppi_time, string>, string > > > pDebugQueue;

		LOCK(m_SENDQUEUELOCK);
		while(!m_bHasContent)
			CONDITION(m_SENDQUEUECONDITION, m_SENDQUEUELOCK);
		UNLOCK(m_SENDQUEUELOCK);

		m_oCache.getContent(pDbQueue, pMsgQueue, pDebugQueue);
		// write all values with identif 'value' from command fillValue()
		if(	!pDbQueue->empty() ||
			!pMsgQueue->empty()	||
			!pDebugQueue->empty()	)
		{
			sendDirect(pDbQueue, pMsgQueue, pDebugQueue);
		}
		return true;
	}

	void DbFiller::sendDirect(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
					SHAREDPTR::shared_ptr<map<string, map<pair<ppi_time, string>, string > > >& debugQueue)
	{
		typedef map<string, db_t>::iterator subIt;
		typedef map<string, map<pair<ppi_time, string>, string > >::iterator debugIt;
		typedef map<pair<ppi_time, string>, string >::iterator debugInnerIt;
		bool bError(false);
		DbInterface* db;
		vector<sendingInfo_t>::iterator msgPos;
		vector<string> answer;
		ErrorHandling err;

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
			for(debugIt dIt= debugQueue->begin(); dIt != debugQueue->end(); ++dIt)
			{
				for(debugInnerIt dIIt= dIt->second.begin(); dIIt != dIt->second.end(); ++dIIt)
				{
					OMethodStringStream command("fillDebugSession");

					command << dIt->first;
					command << dIIt->first.second;
					command << dIIt->second;
					command << dIIt->first.first;
					answer= db->sendMethodD("ppi-db-server", command, "", /*answer*/true);
					for(vector<string>::iterator answ= answer.begin(); answ != answer.end(); ++answ)
					{
						err.setErrorStr(*answ);
						if(err.fail())
						{// ending only by errors (no warnings)
							int log;
							string msg;

							err.addMessage("DbFiller", "sendToDatabase", command.getMethodName()
											+ "@" + dIt->first + "@" + dIIt->first.second);
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
				if(bError)
					break;
			}
		}
		if(!bError)
		{
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
					SHAREDPTR::shared_ptr<map<string, map<pair<ppi_time, string>, string > > >& debugQueue)
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
