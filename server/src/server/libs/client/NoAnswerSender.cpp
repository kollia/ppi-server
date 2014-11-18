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

#include "NoAnswerSender.h"

#include "../SocketErrorHandling.h"

#include "../../../util/GlobalStaticMethods.h"

using namespace std;

namespace util
{
	vector<string> NoAnswerSender::sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done)
	{
		sendingInfo_t sendInfo;
		vector<string> beforeAnsw;

		sendInfo.toProcess= toProcess;
		sendInfo.method= method.str(/*with sync ID*/true);
		sendInfo.done= done;
		LOCK(m_SENDQUEUELOCK);
		m_vsSendingQueue.push_back(sendInfo);
		beforeAnsw.push_back(m_sNoWaitError);
		AROUSE(m_SENDQUEUECONDITION);
		UNLOCK(m_SENDQUEUELOCK);
		return beforeAnsw;
	}

	bool NoAnswerSender::execute()
	{
		vector<sendingInfo_t>::iterator msgPos;
		vector<sendingInfo_t> messages;
		vector<string> answer;

		m_pError->clear();
		LOCK(m_SENDQUEUELOCK);
		if(m_vsSendingQueue.size() == 0)
			CONDITION(m_SENDQUEUECONDITION, m_SENDQUEUELOCK);
		messages= m_vsSendingQueue;
		m_vsSendingQueue.clear();
		UNLOCK(m_SENDQUEUELOCK);

		for(msgPos= messages.begin(); msgPos != messages.end(); ++msgPos)
		{
			OMethodStringStream method(msgPos->method);
			SocketErrorHandling oHandling;

			// this NoAnswer thread waiting for answer,
			// because otherwise sending running in an loop
			answer= m_pExternClient->sendMethodD(msgPos->toProcess, method, msgPos->done, /*answer*/true);
			oHandling.searchResultError(answer);
			if(oHandling.fail())
			{
				(*m_pError)= oHandling;
				if(m_pError->hasError())
					break;
			}
		}
		if(stopping())
			return false;
		LOCK(m_SENDQUEUELOCK);
		if(m_pError->fail())
		{
			string prefix;

			if(m_pError->hasError())
				prefix= "### ERROR: ";
			else
				prefix= "### WARNING: ";
			m_pError->addMessage("NoAnswerSender", "sendMethod", msgPos->method);
			cerr << glob::addPrefix(prefix, m_pError->getDescription()) << endl;
			// fill back into queue all sending methods from error
			if(m_pError->hasError())
			{
				m_sNoWaitError= m_pError->getErrorStr();
				m_vsSendingQueue.insert(m_vsSendingQueue.begin(), msgPos, messages.end());
			}else
				m_sNoWaitError= "";
		}else
			m_sNoWaitError= "";
		UNLOCK(m_SENDQUEUELOCK);
		return true;
	}

	EHObj NoAnswerSender::stop(const bool *bWait/*= NULL*/)
	{
		EHObj res;

		res= Thread::stop(/*wait*/false);
		if(!res->hasError())
		{
			AROUSE(m_SENDQUEUECONDITION);
			res= Thread::stop(bWait);
		}
		return res;
	}

	NoAnswerSender::~NoAnswerSender()
	{
		DESTROYMUTEX(m_SENDQUEUELOCK);
		DESTROYCOND(m_SENDQUEUECONDITION);
	}

} /* namespace ppi_database */
