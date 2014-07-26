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
		bool bNew;
		double value;
		sendingInfo_t sendInfo;
		string folder, subroutine, identif;
		vector<double> values;
		vector<string> vsRv;
		DbInterface* db;

		if(	m_bisRunn &&
			answer == false	)
		{
			vsRv.push_back("done");
			if(method.getMethodName() == "fillValue")
			{
				IMethodStringStream out(method.str());

				out >> folder;
				out >> subroutine;
				out >> identif;
				out >> bNew;
				while(!out.fail())
				{
					out >> value;
					values.push_back(value);
				}
				fillValue(folder, subroutine, identif, values, bNew);
				LOCK(m_SENDQUEUELOCK);
				vsRv.push_back(m_sNoWaitError);
				m_sNoWaitError= "";
				if(m_pHasContent)
					*m_pHasContent= true;
				UNLOCK(m_SENDQUEUELOCK);

			}else
			{
				sendInfo.toProcess= "ppi-db-server";
				sendInfo.method= method.str();
				sendInfo.done= done;
				LOCK(m_SENDQUEUELOCK);
				m_vsSendingQueue->push_back(sendInfo);
				vsRv.push_back(m_sNoWaitError);
				m_sNoWaitError= "";
				if(m_pHasContent)
					*m_pHasContent= true;
				UNLOCK(m_SENDQUEUELOCK);
			}
		}else
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
			LOCK(m_SENDQUEUELOCK);

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
				foundSub= m_apmtValueEntrys->find(subroutine);
				if(foundSub == m_apmtValueEntrys->end())
				{
					newEntry.folder= folder;
					newEntry.subroutine= subroutine;
					newEntry.identif= identif;
					newEntry.values= dvalues;
					newEntry.bNew= bNew;
					(*m_apmtValueEntrys)[subroutine]= newEntry;

				}else
				{
					foundSub->second.values= dvalues;
					foundSub->second.bNew= bNew;
				}
			}else
			{
				OMethodStringStream command("fillValue");

				sendInfo.toProcess= "ppi-db-server";
				sendInfo.done= "";
				command << folder;
				command << subroutine;
				command << identif;
				command << bNew;
				for(vector<double>::const_iterator it= dvalues.begin(); it != dvalues.end(); ++it)
					command << *it;
				sendInfo.method= command.str();
				m_vsSendingQueue->push_back(sendInfo);
			}
			if(m_pHasContent)
				*m_pHasContent= true;
			UNLOCK(m_SENDQUEUELOCK);
		}else
		{
			db= DbInterface::instance();
			db->fillValue(folder, subroutine, identif, dvalues, bNew);
		}
	}

	void DbFillerCache::informDatabase()
	{
		bool bEntrys(false);

		LOCK(m_SENDQUEUELOCK);
		if(	!m_vsSendingQueue->empty() ||
			!m_apmtValueEntrys->empty()		)
		{
			bEntrys= true;
		}
		UNLOCK(m_SENDQUEUELOCK);
		if(bEntrys)
			m_dbInform->informDatabase();
	}

	int DbFillerCache::remove()
	{
		/*
		 * doDo: method to remove cache from queue
		 * 		 only needed when property inside server.conf is ONE
		 */
		return 0;
	}

	void DbFillerCache::getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
					SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue)
	{
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > newMsgQueue(new vector<sendingInfo_t>());
		SHAREDPTR::shared_ptr<map<string, db_t>  > newValueEntrys(new map<string, db_t>());

		// set scheduling back to SCHED_OTHER
		// while thread inside SENDQUEUBLOCK
		// because there should have the same
		// running policy
		// and set to SCHED_BATCH again when outside
		// to run with lower priority
		//setSchedulingParameter(SCHED_OTHER, 0);
		LOCK(m_SENDQUEUELOCK);
		msgQueue= m_vsSendingQueue;
		m_vsSendingQueue= newMsgQueue;
		dbQueue= m_apmtValueEntrys;
		m_apmtValueEntrys= newValueEntrys;
		if(m_pHasContent)
			*m_pHasContent= false;
		UNLOCK(m_SENDQUEUELOCK);
		//setSchedulingParameter(SCHED_BATCH, 0);
	}

} /* namespace util */
