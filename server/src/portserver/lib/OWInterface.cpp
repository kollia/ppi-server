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

#include "../../logger/lib/LogInterface.h"

#include "OWInterface.h"

namespace server {

using namespace logger;

map<unsigned short, OWInterface*> OWInterface::_instances;
IClientConnectArtPattern* OWInterface::m_pCon= NULL;
string OWInterface::m_sProcess;

void OWInterface::initial(const string& process, IClientConnectArtPattern* connection)
{
	m_sProcess= process;
	m_pCon= connection;
}

OWInterface* OWInterface::instance(const unsigned short serverID)
{
	int err;
	ostringstream sID;
	map<unsigned short, OWInterface*>::iterator it;

	if(m_sProcess == "")
	{
		string msg;

		msg=  "### ERROR: get instance is not OK before OWInterface::init()!";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		return NULL;
	}
	it= _instances.find(serverID);
	if(it != _instances.end())
		return it->second;
	sID << serverID;
	_instances[serverID]= new OWInterface(m_sProcess, sID.str(), m_pCon);
	err= _instances[serverID]->openSendConnection();
	if(err > 0)
	{
		string msg;
		msg=  "### ERROR: cannot open connection to one wire server " + sID.str() + "\n";
		msg+= "           " + _instances[serverID]->strerror(err);
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		delete _instances[serverID];
		_instances.erase(serverID);
		return NULL;
	}
	return _instances[serverID];
}

vector<string> OWInterface::getDebugInfo()
{
	vector<string> vRv;
	OMethodStringStream method("getDebugInfo");

	vRv= sendMethod(m_stoClient, method, "done", true);
	return vRv;
}

void OWInterface::setDebug(bool set)
{
	OMethodStringStream method("setDebug");

	method << set;
	sendMethod(m_stoClient, method, false);
}

void OWInterface::clearDebug()
{
	for(map<unsigned short, OWInterface*>::iterator it= _instances.begin(); it != _instances.end(); ++it)
		it->second->setDebug(false);
}

void OWInterface::deleteAll()
{
	for(map<unsigned short, OWInterface*>::iterator it= _instances.begin(); it != _instances.end(); ++it)
	{
		it->second->closeSendConnection();
		delete it->second;
		_instances.erase(it->first);
	}
}
}
