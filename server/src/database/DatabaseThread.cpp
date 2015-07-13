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

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include "DatabaseThread.h"
#include "DefaultChipConfigReader.h"

#include "lib/DatabaseFactory.h"

#include "../server/libs/client/ExternClientInputTemplate.h"

#include "../pattern/server/IClientPattern.h"

#include "../util/thread/ThreadErrorHandling.h"

using namespace util;
using namespace util::thread;
using namespace server;
using namespace ports;

namespace ppi_database
{

DatabaseThread* DatabaseThread::_instance= NULL;

DatabaseThread::DatabaseThread(string dbDir, string confDir, IPropertyPattern* properties)
:	Thread("database", false),
 	m_bDbLoaded(false)
{
	bool bUseRegex;
	string res;
	IChipConfigReaderPattern* reader;

	bUseRegex= false;
	res= properties->getValue("usefsdefault_regex");
	if(	res == "true" ||
		res == "TRUE"		)
	{
		bUseRegex= true;
	}
	// initial configuration path to reading default configuration for any chips
	DefaultChipConfigReader::init(confDir, bUseRegex);
	reader= DefaultChipConfigReader::instance();
	m_pDatabase= DatabaseFactory::getChoosenDatabase(properties, reader);
	m_DBLOADED= getMutex("DBLOADED");

}

void DatabaseThread::initial(string workDir, string confDir, IPropertyPattern* properties)
{
	if(_instance == NULL)
	{
		_instance= new DatabaseThread(workDir, confDir, properties);
		_instance->start(NULL, false);
	}
}

void DatabaseThread::deleteObj()
{
	delete _instance;
	_instance= NULL;
}

bool DatabaseThread::isDbLoaded() const
{
	bool bRv;

	LOCK(m_DBLOADED);
	bRv= m_bDbLoaded;
	UNLOCK(m_DBLOADED);
	return bRv;
}

EHObj DatabaseThread::init(void *args)
{
	if(m_pDatabase == NULL)
	{
		m_pError->setError("DatabaseThread", "init");
		return m_pError;
	}
	m_pDatabase->read();

	LOCK(m_DBLOADED);
	m_bDbLoaded= true;
	UNLOCK(m_DBLOADED);
	return m_pError;
}

vector<string> DatabaseThread::getDebugInfo(const unsigned short server)
{
	ThreadErrorHandling errHandle;
	IClientPattern* client;
	ostringstream definition;
	vector<string> vRv, vanswer;
	string answer;

	definition << __EXTERNALPORT_CLIENT_BEGINSTR << server;
	client= m_pStarter->getClient(__EXTERNALPORT_PROCESSES, definition.str(), NULL);
	if(client != NULL)
	{
		do{
			vanswer= client->sendString(IMethodStringStream("getinfo"), true, "done");
			if(vanswer.size())
				answer= vanswer.front();
			else
				answer= "";
			errHandle.setErrorStr(answer);
			vRv.push_back(answer);

		}while(answer != "done" && !errHandle.hasError());
	}
	return vRv;
}

EHObj DatabaseThread::stop(const bool *bWait)
{
	bool stopped;

	m_pError= Thread::stop(false);
	stopped= m_pDatabase->stop();

	if(	!m_pError->hasError() &&
		stopped &&
		bWait		)
	{
		(*m_pError)= Thread::stop(/*wait*/bWait);
	}
	return m_pError;
}

DatabaseThread::~DatabaseThread()
{
	DESTROYMUTEX(m_DBLOADED);
	delete m_pDatabase;
}

}
