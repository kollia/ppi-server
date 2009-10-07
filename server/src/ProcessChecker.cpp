/**
 *   This file is part of ppi-server.
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

#include "ProcessChecker.h"

#include "ports/measureThread.h"

#include "logger/lib/LogInterface.h"

#include "util/IMethodStringStream.h"

using namespace util;

inline int ProcessChecker::init(void *args)
{
	return openGetConnection();
}

int ProcessChecker::execute()
{
	int nRv= 0;
	string method;
	string question;

	question= getQuestion(m_sAnswer);
	IMethodStringStream object(question);

	method= object.getMethodName();
	if(method == "setValue")
	{
		bool bCorrect;
		double value;
		string folder, subroutine;
		meash_t* pCurMeas= meash_t::firstInstance;
		portBase* port= NULL;

		object >> folder;
		object >> subroutine;
		object >> value;
		while(pCurMeas)
		{
			if(pCurMeas->pMeasure->getThreadName() == folder)
				break;
			pCurMeas= pCurMeas->next;
		}
		if(pCurMeas)
		{
			port= pCurMeas->pMeasure->getPortClass(subroutine, bCorrect);
			if(port)
			{
				if(bCorrect)
				{
					port->setValue(value);
					m_sAnswer= "done";
				}else
					m_sAnswer= "nochipaccess";
			}else
				m_sAnswer= "nosubroutine";

		}else
			m_sAnswer= "nofolder";

	}else if(method == "stop-all")
	{
		static meash_t* pCurrent;

		switch(m_nEndPos)
		{
		case 0:
			// ending external port list
			cout << "stopping:" << endl;
			cout << "measureThreads " << flush;
			pCurrent= meash_t::firstInstance;
			while(pCurrent)
			{ // stopping all measure threads
				pCurrent->pMeasure->stop(/*wait*/false);
				pCurrent= pCurrent->next;
			}
			m_sAnswer= "measure threads";
			++m_nEndPos;
			pCurrent= meash_t::firstInstance;
			break;

		case 1:
			if(pCurrent)
			{
				cout << "." << flush;
				if(pCurrent->pMeasure->running())
				{
					usleep(1000000);
				}else
				{
					pCurrent= pCurrent->next;
					if(!pCurrent)
					{
						++m_nEndPos;
						cout << endl;
					}
				}
			}
			m_sAnswer= "measure threads";
			break;

		case 2:
			stop(/*wait*/false);
			getQuestion("done");
			break;
		}
	}else
	{
		string msg("### ERROR: undefined command '");

		msg+= question + "' was send to ProcessChecker";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		m_sAnswer= "ERROR 001";
	}
	return nRv;
}

inline void ProcessChecker::ending()
{
	closeGetConnection();
}
