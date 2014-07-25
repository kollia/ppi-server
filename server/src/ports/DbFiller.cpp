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

#include "../database/lib/DbInterface.h"

#include "DbFiller.h"

using namespace std;
using namespace ppi_database;

namespace util
{
	int DbFiller::execute()
	{
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > pMsgQueue;
		SHAREDPTR::shared_ptr<map<string, db_t>  > pDbQueue;

		LOCK(m_SENDQUEUELOCK);
		while(!m_bHasContent)
			CONDITION(m_SENDQUEUECONDITION, m_SENDQUEUELOCK);
		UNLOCK(m_SENDQUEUELOCK);

		m_oCache.getContent(pDbQueue, pMsgQueue);
		// write all values with identif 'value' from command fillValue()
		if(	!pDbQueue->empty() ||
			!pMsgQueue->empty()		)
		{
			sendDirect(pDbQueue, pMsgQueue);
		}
		return 0;
	}

	void DbFiller::sendDirect(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue)
	{
		typedef map<string, db_t>::iterator subIt;
		bool bError(false);
		DbInterface* db;
		vector<sendingInfo_t>::iterator msgPos;
		vector<string> answer;
		int err;

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
				err= error(*answ);
				if(err > 0)
				{// ending only by errors (no warnings)
					ostringstream oErr;

					oErr << "DbFiller for NoAnswer method: " << msgPos->method << endl;
					oErr << " get Error code: " << *answ;
					cerr << oErr.str() << endl << endl;
					LOGEX(LOG_ERROR, oErr.str(), &m_oCache);
					bError= true;
					break;
				}
			}
			if(bError)
				break;
		}
		if(!bError)
		{
			for(msgPos= msgQueue->begin(); msgPos != msgQueue->end(); ++msgPos)
			{
				OMethodStringStream method(msgPos->method);

				answer= db->sendMethodD(msgPos->toProcess, method, msgPos->done, /*answer*/true);
				for(vector<string>::iterator answ= answer.begin(); answ != answer.end(); ++answ)
				{
					err= error(*answ);
					if(err > 0)
					{// ending only by errors (no warnings)
						ostringstream oErr;

						oErr << "DbFiller for NoAnswer method: " << msgPos->method << endl;
						oErr << " get Error code: " << *answ;
						cerr << oErr.str() << endl << endl;
						LOGEX(LOG_ERROR, oErr.str(), &m_oCache);
						bError= true;
						break;
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
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue)
	{
		m_oCache.getContent(dbQueue, msgQueue);
	}

	inline int DbFiller::error(const string& input)
	{
		int number;
		string answer;
		istringstream out(input);

		out >> answer >> number;
		if(answer == "ERROR")
			return number;
		if(answer == "WARNING")
			return number * -1;
		return 0;
	}

	int DbFiller::stop(const bool *bWait/*= NULL*/)
	{
		int res;

		Thread::stop(/*wait*/false);
		AROUSE(m_SENDQUEUECONDITION);
		res= Thread::stop(bWait);
		return res;
	}

	DbFiller::~DbFiller()
	{
		DESTROYMUTEX(m_SENDQUEUELOCK);
		DESTROYCOND(m_SENDQUEUECONDITION);
	}

} /* namespace ppi_database */
