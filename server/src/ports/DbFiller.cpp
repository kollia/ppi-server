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

#include "../util/GlobalStaticMethods.h"
#include "../util/exception.h"
#include "../database/lib/DbInterface.h"

#include "DbFiller.h"

using namespace std;
using namespace ppi_database;

namespace util
{
	string DbFiller::sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer/*= true*/)
	{
		vector<string> res;

		res= sendMethod(toProcess, method, "", answer);
		if(res.size() == 0)
			return "";
		return res[0];
	}
	vector<string> DbFiller::sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer/*= true*/)
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
				UNLOCK(m_SENDQUEUELOCK);

			}else
			{
				sendInfo.toProcess= "ppi-db-server";
				sendInfo.method= method.str();
				sendInfo.done= done;
				LOCK(m_SENDQUEUELOCK);
				m_vsSendingQueue.push_back(sendInfo);
				vsRv.push_back(m_sNoWaitError);
				AROUSE(m_SENDQUEUECONDITION);
				UNLOCK(m_SENDQUEUELOCK);
			}
		}else
		{
			db= DbInterface::instance();
			vsRv= db->sendMethod(toProcess, method, done, answer);
		}
		return vsRv;
	}
	void DbFiller::fillValue(const string& folder, const string& subroutine, const string& identif,
					double value, bool bNew/*= false*/)
	{
		vector<double> values;

		values.push_back(value);
		fillValue(folder, subroutine, identif, values, bNew);
	}

	void DbFiller::fillValue(const string& folder, const string& subroutine, const string& identif,
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
				m_vsSendingQueue.push_back(sendInfo);
			}
			AROUSE(m_SENDQUEUECONDITION);
			UNLOCK(m_SENDQUEUELOCK);
		}else
		{
			db= DbInterface::instance();
			db->fillValue(folder, subroutine, identif, dvalues, bNew);
		}
	}

	int DbFiller::execute()
	{
		typedef map<string, db_t>::iterator subIt;
		std::auto_ptr<map<string, db_t>  > pRv(new map<string, db_t>());
		DbInterface* db;
		vector<sendingInfo_t>::iterator msgPos;
		vector<sendingInfo_t> messages;
		vector<string> answer;
		string sError;
		int err;

		LOCK(m_SENDQUEUELOCK);
		if(	m_vsSendingQueue.size() == 0 &&
			m_apmtValueEntrys->size() == 0	)
		{
			CONDITION(m_SENDQUEUECONDITION, m_SENDQUEUELOCK);
		}
		messages= m_vsSendingQueue;
		pRv= m_apmtValueEntrys;
		m_apmtValueEntrys= auto_ptr<map<string, db_t> >(new map<string, db_t>());
		m_vsSendingQueue.clear();
		UNLOCK(m_SENDQUEUELOCK);

		db= DbInterface::instance();
		// write all values with identif 'value' from command fillValue()
		for(subIt sIt= pRv->begin(); sIt != pRv->end(); ++sIt)
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
					sError= *answ;
					break;
				}
			}
			if(sError != "") // toDo: error handling, because values by error lost here
				break;
		}
		if(sError == "")
		{
			for(msgPos= messages.begin(); msgPos != messages.end(); ++msgPos)
			{
				OMethodStringStream method(msgPos->method);

				answer= db->sendMethodD(msgPos->toProcess, method, msgPos->done, /*answer*/true);
				for(vector<string>::iterator answ= answer.begin(); answ != answer.end(); ++answ)
				{
					err= error(*answ);
					if(err > 0)
					{// ending only by errors (no warnings)
						sError= *answ;
						break;
					}
				}
				if(sError != "")
					break;
			}
		}
		if(stopping())
			return 0;
		LOCK(m_SENDQUEUELOCK);
		if(sError != "")
		{
			cerr << "DbFiller for NoAnswer method: " << msgPos->method << endl;
			cerr << " get Error code: " << sError << endl << endl;
			m_sNoWaitError= sError; // fill back into queue all sending methods from error
			m_vsSendingQueue.insert(m_vsSendingQueue.begin(), msgPos, messages.end());
		}else
			m_sNoWaitError= "";
		UNLOCK(m_SENDQUEUELOCK);
		return 0;
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
