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

#include "../../pattern/util/LogHolderPattern.h"

#include "../../util/GlobalStaticMethods.h"
#include "../../util/thread/ThreadErrorHandling.h"

#include "NeedDbChanges.h"

namespace ppi_database {

	using namespace util::thread;

	NeedDbChanges* NeedDbChanges::_instance= NULL;

NeedDbChanges::NeedDbChanges(const string& process, IClientConnectArtPattern* connection, const bool bWait)
:	Thread("NeedChangesObject", bWait),
	m_sProcess(process),
	m_oConnection(connection),
	m_oDb(NULL),
	m_nChanged(1)
{
	m_CHANGEQUESTION= getMutex("CHANGEQUESTION");
	m_CHANGEQUESTIONCOND= getCondition("CHANGEQUESTIONCOND");
}

bool NeedDbChanges::initial(const string& process, IClientConnectArtPattern* connection, const bool bWait/*= true*/)
{
	if(_instance == NULL)
	{
		ThreadErrorHandling oHandling;

		_instance= new NeedDbChanges(process, connection, bWait);
		oHandling= _instance->start();
		if(oHandling.fail())
		{
			int logging;
			string prefix, msg;

			if(oHandling.hasError())
			{
				prefix= "### ALERT: ";
				logging= LOG_ALERT;
			}else
			{
				prefix= "### WARNING: ";
				logging= LOG_WARNING;
			}
			oHandling.addMessage("NeedDbChanges", "start");
			msg= oHandling.getDescription();
			cout << glob::addPrefix(prefix, msg) << endl;
			LOG(logging, msg);
			if(oHandling.hasError())
				return false;
		}
	}
	return true;
}

void NeedDbChanges::deleteObj()
{
	if(_instance != NULL)
	{
		delete _instance;
		_instance= NULL;
	}
}

EHObj NeedDbChanges::init(void* args)
{
	short n;

	m_pError->clear();
	n= DbInterface::initial(m_sProcess, m_oConnection, 0/*do not need for second interface*/);
	m_oDb= DbInterface::instance(n);
	return m_pError;
}

bool NeedDbChanges::execute()
{
	if(!m_oDb->hasOpenSendConnection())
	{
		m_pError= m_oDb->openSendConnection();
		if(m_pError->hasError())
		{
			if(m_pError->fail(IEH::errno_error, ECONNREFUSED))
			{// create no ERROR try again later
				USLEEP(5000);
				return false;
			}else
			{
				cerr << glob::addPrefix("### ERROR: ", m_pError->getDescription()) << endl;
			}
			return false;

		}
	}
	m_oDb->isEntryChanged();
	if(stopping())
	{
		AROUSEALL(m_CHANGEQUESTIONCOND);
		return false;
	}
	LOCK(m_CHANGEQUESTION);
	++m_nChanged;
	if(m_nChanged > 1000)
		m_nChanged= 1;
	AROUSEALL(m_CHANGEQUESTIONCOND);
	UNLOCK(m_CHANGEQUESTION);
	return true;
}

unsigned short NeedDbChanges::isEntryChanged(unsigned short actualized)
{
	if(stopping())
		return 0;
	LOCK(m_CHANGEQUESTION);
	if(	actualized == m_nChanged
		&&
		!stopping()					)
	{
		CONDITION(m_CHANGEQUESTIONCOND, m_CHANGEQUESTION);
	}
	actualized= m_nChanged;
	UNLOCK(m_CHANGEQUESTION);
	return actualized;
}

}
