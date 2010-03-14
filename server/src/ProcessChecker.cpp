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
	if(	method == "setValue"
		||
		method == "changedChip"	)
	{
		bool bCorrect, device;
		double value;
		string folder, subroutine;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;
		SHAREDPTR::shared_ptr<portBase> port;

		object >> folder;
		object >> subroutine;
		object >> value;
		if(method == "changedChip")
			object >> device;
		else
			device= true;
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
#if 0
#define __DEBUGPROCESSGETCHANGES
				ostringstream out;
				out << "ProcessChecker reach " << method << " for " << folder << ":" << subroutine << " to set value " << value;
#endif
				if(method == "changedChip")
				{
#ifdef __DEBUGPROCESSGETCHANGES
					out << " with reachable device " << boolalpha << device << endl;
					cout << out.str();
#endif // __DEBUGPROCESSGETCHANGES
					port->setDeviceAccess(device);
					port->setValue(value);
					pCurMeas->pMeasure->changedValue(folder);
					m_sAnswer= "done";

				}else if(bCorrect)
				{
					double oldVal;

#ifdef __DEBUGPROCESSGETCHANGES
					out << endl;
					cout << out.str();
#endif // __DEBUGPROCESSGETCHANGES
					oldVal= port->getValue("i:"+folder);
					if(value != oldVal)
					{
						port->setValue(value);
						pCurMeas->pMeasure->changedValue(folder);
					}
					m_sAnswer= "done";

				}else
				{
#ifdef __DEBUGPROCESSGETCHANGES
					out << " but do not reach device" << endl;
					cout << out.str();
#endif // __DEBUGPROCESSGETCHANGES
					m_sAnswer= "nochipaccess";
				}
			}else
				m_sAnswer= "nosubroutine";

		}else
			m_sAnswer= "nofolder";

	}else if(method == "debugFolder")
	{
		bool bFound= false;
		string folder;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;

		object >> folder;
		while(pCurMeas)
		{
			if(pCurMeas->pMeasure->getThreadName() == folder)
			{
				pCurMeas->pMeasure->setDebug(true, 3);
				bFound= true;
			}else
				pCurMeas->pMeasure->setDebug(false, 0);
			pCurMeas= pCurMeas->next;
		}
		if(!bFound)
			m_sAnswer= "nofolder";

	}else if(method == "clearFolderDebug")
	{
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;

		while(pCurMeas)
		{
			pCurMeas->pMeasure->setDebug(false, 0);
			pCurMeas= pCurMeas->next;
		}

	}else if(method == "getOWMaxCount")
	{
		ostringstream count;

		count << m_nExistOW;
		m_sAnswer= count.str();

	}else if(method == "stop-all")
	{
		static SHAREDPTR::shared_ptr<meash_t> pCurrent;

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
			m_sAnswer= "stop measure threads";
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
			m_sAnswer= "stop measure threads";
			break;

		case 2:
			stop(/*wait*/false);
			getQuestion("done");
			break;
		}
	}else
	{
		int err;
		string msg("### ERROR: undefined command '");

		msg+= question + "' was send to ProcessChecker";
		err= error(question);
		if(err)
		{
			msg+= "\n           ";
			msg+= strerror(err, false);
		}else
			m_sAnswer= "ERROR 001";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
	}
	return nRv;
}

inline void ProcessChecker::ending()
{
	closeGetConnection();
}
