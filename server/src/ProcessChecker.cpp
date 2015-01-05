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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "ProcessChecker.h"
#include "MeasureThreadCounter.h"

#include "ports/measureThread.h"

#include "database/lib/DbInterface.h"

#include "pattern/util/LogHolderPattern.h"

#include "util/GlobalStaticMethods.h"

#include "util/stream/ppivalues.h"
#include "util/stream/IMethodStringStream.h"

using namespace util;
using namespace boost;

inline EHObj ProcessChecker::init(void *args)
{
	return openGetConnection();
}

bool ProcessChecker::execute()
{
	static bool bfirst(false);
	static MeasureThreadCounter* counter(NULL);
	string method;
	string question;
	string from;

	if(!bfirst)
	{
		//cout << "--------------------------------------------------------------------------------------" << endl;
		//cout << "running first execute of ProcessChecker to set server status to finished" << endl;
		//cout << "--------------------------------------------------------------------------------------" << endl;
		ppi_database::DbInterface::instance()->setServerConfigureStatus("finished", -1);
		bfirst= true;
	}
	question= getQuestion(m_sAnswer);
	IMethodStringStream object(question);

	method= object.getMethodName();
	//cout << "get question '" << method << "'" << endl;
	if(	method == "setValue"
		||
		method == "changedChip"	)
	{
		bool bCorrect, device= true;
		int nPlace;
		InformObject::posPlace_e place;
		ValueHolder oValue;
		string folder, subroutine;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;
		SHAREDPTR::shared_ptr<IListObjectPattern> port;

		object >> folder;
		object >> subroutine;
		object >> oValue.value;
		if(method != "changedChip")
		{
			object >> oValue.lastChanging;
			object >> nPlace;
			place= (InformObject::posPlace_e)nPlace;
		}else
			object >> device;
		object >> from;
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
					IInformerCachePattern* informerCache;
					InformObject inform(InformObject::READER, from);

#ifdef __DEBUGPROCESSGETCHANGES
					out << " with reachable device " << boolalpha << device << endl;
					cout << out.str();
#endif // __DEBUGPROCESSGETCHANGES
					port->setDeviceAccess(device);
					port->setValue(oValue, inform);
					informerCache= pCurMeas->pMeasure->getInformerCache(from);
					informerCache->changedValue(folder, inform);
					m_sAnswer= "done";

				}else if(bCorrect)
				{
					if(counter)
						counter->clientAction(folder, subroutine, oValue.value, from);
#ifdef __DEBUGPROCESSGETCHANGES
					out << endl;
					cout << out.str();
#endif // __DEBUGPROCESSGETCHANGES
					port->setValue(oValue, InformObject(place, from));
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

	}else if(method == "getValue")
	{
		bool bCorrect;
		int nPlace;
		ValueHolder value;
		string folder, subroutine, account;
		ostringstream sval;
		InformObject::posPlace_e ePlace;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;
		SHAREDPTR::shared_ptr<IListObjectPattern> port;

		object >> folder;
		object >> subroutine;
		object >> nPlace;
		object >> account;
		ePlace= (InformObject::posPlace_e)nPlace;
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
					bCorrect= port->hasDeviceAccess();
					if(bCorrect)
					{
						value= port->getValue(InformObject(ePlace, account));
						sval << value.value;
						m_sAnswer= sval.str();
					}else
						m_sAnswer= "nochipaccess";
				}else
					m_sAnswer= "nocorrectsubroutine";
			}else
				m_sAnswer= "nosubroutine";
		}else
			m_sAnswer= "nofolder";

	}else if(method == "debugSubroutine")
	{
		bool bFound= false, debug, bInform;
		string folder, subroutine;
		SHAREDPTR::shared_ptr<meash_t> pCurMeas= meash_t::firstInstance;
		ostringstream msg;

		object >> debug;
		object >> bInform;
		object >> folder;
		object >> subroutine;
		if(folder == "")
		{
			subroutine= "#AllFolder";
			msg << "set all folder with all subroutines to debug session";
#if ( __DEBUGSESSIONOutput == debugsession_CLIENT || __DEBUGSESSIONOutput == debugsession_BOTH)
			IDbFillerPattern::dbgSubroutineContent_t content;

			/**
			 * place of new definition of content are:
			 * ProcessChecker::execute by method == "debugSubroutine"
			 * Informer::informing
			 * MeasureThread::setDebug on method end
			 * MeasureThread::execute by 2 times
			 * MeasureThread::doDebugStartingOutput
			 * MeasureThread::checkToStart
			 * portBase::writeDebugStream
			 * ServerDbTransaction::transfer by method == "fillDebugSession"
			 */
			content.folder= "all";
			content.subroutine= "#setDebug";
			content.value= 0;
			content.currentTime= SHAREDPTR::shared_ptr<IPPITimePattern>(new ppi_time);
			content.content= msg.str();
			pCurMeas->pMeasure->fillDebugSession(content);
#endif
#if ( __DEBUGSESSIONOutput == debugsession_SERVER || __DEBUGSESSIONOutput == debugsession_BOTH)
			tout << msg.str() << endl;
			TERMINALEND;
#endif
		}
		while(pCurMeas)
		{
			if( folder == "" ||
				pCurMeas->pMeasure->getThreadName() == folder	)
			{
				pCurMeas->pMeasure->setDebug(debug, bInform, subroutine);
				bFound= true;
				if(subroutine != "#AllFolder")
					break;
			}
			pCurMeas= pCurMeas->next;
		}
		if(!bFound)
			m_sAnswer= "nofolder";

	}else if(method == "showThreads")
	{
		//bool bOrder;
		bool bClient;
		int seconds;

		object >> seconds;
		//object >> bOrder;
		object >> bClient;
		if(counter)
		{
			counter->stop(true);
			delete counter;
		}
		counter= new MeasureThreadCounter(seconds, /*bOrder*/true, bClient);
		counter->initialStarting();
		m_sAnswer= "done";

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
			if(counter)
			{
				counter->stop(true);
				delete counter;
			}
			// ending external port list
			cout << "stopping:" << endl;
			cout << "measureThreads " << flush;
			pCurrent= meash_t::firstInstance;
			glob::stopMessage("ProcessChecker::execute(): sending stop to all measure threads");
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
				glob::stopMessage("ProcessChecker::execute(): stop measure thread " + pCurrent->pMeasure->getThreadName());
				if(pCurrent->pMeasure->running())
				{
					glob::stopMessage("ProcessChecker::execute(): thread was running, stopping again");
					pCurrent->pMeasure->stop(/*wait*/false);
					glob::stopMessage("ProcessChecker::execute(): thread was stopped again");
					USLEEP(1000000);
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
			glob::stopMessage("ProcessChecker::execute(): stop own process");
			stop(/*wait*/false);
			getQuestion("done");
			break;
		}
	}else if(method == "getStatusInfo")
	{
		static unsigned short step= 0;
		static vector<string> status;
		vector<string>::iterator pos;
		string param, msg;

		object >> param;
		switch(step)
		{
		case 0:
			msg= Thread::getStatusInfo(param);
			split(status, msg, is_any_of("\n"));
			pos= status.begin();
			m_sAnswer= *pos;
			//cout << "Log send: " << *pos << endl;
			status.erase(pos);
			++step;
			break;
		case 1:
			if(status.size() > 0)
			{
				do{
					pos= status.begin();
					msg= *pos;
					m_sAnswer= *pos;
					//cout << "Log send: " << *pos << endl;
					status.erase(pos);

				}while(	msg == "" &&
						!status.empty()	);
				if(msg != "")
					break;
			}
			//cout << "Log all be done" << endl;
			m_sAnswer= "done\n";
			step= 0;
			break;
		}

	}else
	{
		int log;
		string msg;

		m_pError->clear();
		m_pError->setErrorStr(question);
		if(m_pError->fail())
			m_pError->addMessage("ProcessChecker", "waitForQuestion");
		else
			m_pError->setError("ProcessChecker", "getWrongQuestion", question);
		msg= m_pError->getDescription();
		if(m_pError->hasError())
		{
			log= LOG_ERROR;
			cerr << glob::addPrefix("### ERROR: ", msg) << endl;
		}else
		{
			log= LOG_WARNING;
			cout << glob::addPrefix("### WARNING: ", msg) << endl;
		}
		LOG(log, msg);
	}
	return true;
}

inline void ProcessChecker::ending()
{
	closeGetConnection();
}
