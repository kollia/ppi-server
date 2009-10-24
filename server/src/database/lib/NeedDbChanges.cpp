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

#include "NeedDbChanges.h"

namespace ppi_database {

NeedDbChanges* NeedDbChanges::_instance= NULL;

NeedDbChanges::NeedDbChanges(const string& process, IClientConnectArtPattern* connection, const bool bWait)
:	Thread("NeedChangesObject", 0, bWait),
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
		_instance= new NeedDbChanges(process, connection, bWait);
		if(_instance->start() > 0)
			return false;
	}
	return true;
}

int NeedDbChanges::init(void* args)
{
	short n;

	n= DbInterface::initial(m_sProcess, m_oConnection);
	m_oDb= DbInterface::instance(n);
	return 0;
}

int NeedDbChanges::execute()
{
	if(!m_oDb->hasOpenSendConnection())
	{
		int ret;

		ret= m_oDb->openSendConnection();
		if(ret > 0 && ret != 35)
		{
			cerr << m_oDb->strerror(ret) << endl;
			return ret;
		}
		if(ret == 35)
			return 0; // try again later
	}
	m_oDb->isEntryChanged();
	if(stopping())
	{
		AROUSEALL(m_CHANGEQUESTIONCOND);
		return 0;
	}
	LOCK(m_CHANGEQUESTION);
	++m_nChanged;
	if(m_nChanged > 1000)
		m_nChanged= 1;
	AROUSEALL(m_CHANGEQUESTIONCOND);
	UNLOCK(m_CHANGEQUESTION);
	return 0;
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
