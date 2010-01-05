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

#include "../../util/IParameterStringStream.h"

#include "OWInterface.h"

namespace server {

using namespace logger;

map<unsigned short, OWInterface*> OWInterface::_instances;

inline OWInterface* OWInterface::getServer(const unsigned short serverID)
{
	return getServer("", NULL, serverID);
}

OWInterface* OWInterface::getServer(const string& process, IClientConnectArtPattern* connection, const unsigned short serverID)
{
	int err;
	ostringstream sID;
	map<unsigned short, OWInterface*>::iterator it;

	it= _instances.find(serverID);
	if(it != _instances.end())
	{
		if(connection)
			delete connection;
		return it->second;
	}
	sID << serverID;
	if(!connection)
	{
		string msg;

		msg=  "### ERROR: no instance for ID ";
		msg+= sID.str() + " exist! Create first with getServer(processName, connection, serverID).";
		cerr << msg << endl;
		LOG(LOG_ERROR, msg);
		return NULL;
	}
	_instances[serverID]= new OWInterface(process, sID.str(), connection);
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

OWInterface* OWInterface::getServer(const string& type, const string& chipID)
{
	for(map<unsigned short, OWInterface*>::iterator it= _instances.begin(); it != _instances.end(); ++it)
	{
		if(it->second->isServer(type, chipID))
			return it->second;
	}
	return NULL;
}

string OWInterface::getServerName()
{
	string sRv;
	OMethodStringStream method("getServerName");

	sRv= sendMethod(m_stoClient, method, true);
	return sRv;
}

void OWInterface::endOfInitialisation()
{
	OWInterface *pFirst;
	OMethodStringStream method("endOfInitialisation");

	pFirst= _instances.begin()->second;
	pFirst->sendMethod("ppi-db-server", method, false);
}

bool OWInterface::isServer(const string& type, const string& chipID)
{
	string res;
	OMethodStringStream method("isServer");

	method << type;
	method << chipID;
	res= sendMethod(m_stoClient, method, true);
	if(res == "true")
		return true;
	return false;
}

string OWInterface::getChipType(const string& ID)
{
	string sRv;
	OMethodStringStream method("getChipType");

	method << ID;
	sRv= sendMethod(m_stoClient, method, true);
	return sRv;
}

string OWInterface::getChipFamily(const string& ID)
{
	string sRv;
	OMethodStringStream method("getChipFamily");

	method << ID;
	sRv= sendMethod(m_stoClient, method, true);
	return sRv;
}

vector<string> OWInterface::getChipIDs()
{
	vector<string> vRv;
	OMethodStringStream method("getChipIDs");

	vRv= sendMethod(m_stoClient, method, "done", true);
	return vRv;
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

bool OWInterface::write(const string& id, const double value)
{
	string res;
	OMethodStringStream method("write");

	method << id;
	method << value;
	res= sendMethod(m_stoClient, method, true);
	if(res == "true")
		return true;
	return false;
}

bool OWInterface::read(const string& id, double* value)
{
	bool bRv;
	string res;
	OMethodStringStream method("read");

	method << id;
	method << *value;
	res= sendMethod(m_stoClient, method, true);

	IParameterStringStream result(res);

	result >> *value;
	result >> bRv;
	return bRv;
}

void OWInterface::range(const string& pin, double& min, double& max, bool& bfloat)
{
	string res;
	OMethodStringStream method("range");

	method << pin;
	res= sendMethod(m_stoClient, method, true);

	IParameterStringStream result(res);

	result >> min;
	result >> max;
	result >> bfloat;
}

bool OWInterface::haveToBeRegistered()
{
	string res;
	OMethodStringStream method("haveToBeRegistered");

	res= sendMethod(m_stoClient, method, true);
	if(res == "true")
		return true;
	return false;
}

short OWInterface::useChip(IActionPropertyPattern* properties, string& unique, const string& folder, const string& subroutine)
{
	short nRv;
	string res;
	OMethodStringStream method("useChip");

	method << properties->str();
	method << unique;
	method << folder;
	method << subroutine;
	res= sendMethod(m_stoClient, method, true);

	IParameterStringStream result(res);

	result >> nRv;
	result >> res;
	properties->pulled(res);
	result >> unique;
	return nRv;
}

void OWInterface::usePropActions(const IActionPropertyPattern* properties)
{
	string pulled;
	OMethodStringStream method("usePropActions");

	method << properties->str();
	pulled= sendMethod(m_stoClient, method, /*wait*/true);
	properties->pulled(pulled);
}

void OWInterface::checkUnused()
{
	OWInterface *pFirst;
	OMethodStringStream method("checkUnused");

	pFirst= _instances.begin()->second;
	pFirst->sendMethod("ppi-db-server", method, true);// wait for ending
}

bool OWInterface::reachAllChips()
{
	string res;
	OMethodStringStream method("reachAllChips");

	res= sendMethod(m_stoClient, method, true);
	if(res == "true")
		return true;
	return false;
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
