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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../logger/lib/LogInterface.h"

//#include "../util/ProcessStarter.h"

#include "LircClient.h"

//extern "C" {
#include "irsend.h"
//}

namespace ports
{
	using namespace util;
	using namespace boost;

	bool LircClient::init(const IPropertyPattern* properties)
	{
		return connect();
	}

	void LircClient::usePropActions(const IActionPropertyPattern* properties) const
	{
		bool bW1, bW2;

		properties->needValue("ID");
		properties->needValue("pin");
		properties->haveAction("receive");
		bW1= properties->haveAction("send");
		bW2= properties->haveAction("send_once");
		if(bW1 || bW2)
		{
			properties->getValue("priority", /*warning*/false);
			properties->getValue("begin", /*warning*/false);
			properties->getValue("while", /*warning*/false);
			properties->getValue("end", /*warning*/false);
		}
		properties->haveAction("db");
	}

	short LircClient::useChip(const IActionPropertyMsgPattern* properties, string& id, unsigned short& kernelmodule)
	{
		bool bread, bW1, bW2;
		short nRv= 0, set= 0;
		string remote, code, msgh, msg;

		kernelmodule= 0;
		remote= properties->needValue("ID");
		if(remote == "")
			return 0;
		code= properties->needValue("pin");
		if(code == "")
			return 0;
		msgh= "LIRC definition in folder ";
		msgh+= properties->getValue("folder", /*warning*/false);
		msgh+= " and subroutine ";
		msgh+= properties->getValue("name", /*warning*/false);
		msgh+= " by remote " + remote;
		msgh+= " and code " + code;
		msgh+= " isn't correct set\n";
		bread= properties->haveAction("receive");
		if(bread) ++set;
		bW1= properties->haveAction("send");
		if(bW1) ++set;
		bW2= properties->haveAction("send_once");
		if(bW2) ++set;
		if(set != 1)
		{
			if(set > 1)
				msg= "it can be set only one (receive|send|send_once) for action";
			else
				msg= "action receive, send or send_once have to be set";
			cerr << msg << endl;
			LOG(LOG_ERROR, msgh + msg);
			return 0;
		}
		id= remote + "/" + code;
		if(bW2)
			id+= "/SEND_ONCE";
		else if(bW1)
			id+= "/SEND";
		if(bread)
		{
			kernelmodule= 2;
			id= "read_" + id;
			//cout  << "'" << id  << "'" << endl;
			m_mset[id]= false;
			nRv= 1;
		}else
		{
			id= "write_" + id;
			nRv= 2;
			if(bW1)
				m_mSend[id]= false;
		}
		m_mvCodes[code].push_back(id);
		return nRv;
	}

	vector<string> LircClient::getUnusedIDs() const
	{
		vector<string> unused;

		return unused;
	}

	bool LircClient::connect()
	{
		char type[8];

		strncpy(type, "irexec", 6);
		if(lirc_init(type, 1) == -1)
		{
			cout << "### ERROR: cannot initial lirc" << endl;
			cout << "    ERRNO: " << strerror(errno) << endl;
			return false;
		}
		/*if(lirc_readconfig(NULL, &m_ptLircConfig, NULL) != 0)
		{
			cout << "### ERROR: cannot read lirc configuration" << endl;
			cout << "    ERRNO: " << strerror(errno) << endl;
			return false;
		}*/
		m_bConnected= true;
		return true;
	}

	void LircClient::disconnect()
	{
		lirc_freeconfig(m_ptLircConfig);
		lirc_deinit();
	}

	string LircClient::getChipType(string ID)
	{
		return "lirc";
	}

	string LircClient::getChipTypeID(const string pin)
	{
		return pin;
	}

	vector<string> LircClient::getChipIDs() const
	{
		vector<string> ids;

		for(map<string, vector<string> >::const_iterator it= m_mvCodes.begin(); it != m_mvCodes.end(); ++it)
			ids.insert(ids.end(), it->second.begin(), it->second.end());
		return ids;
	}

	bool LircClient::existID(const string type, string ID) const
	{
		bool bfound= false;
		vector<string>::const_iterator found;

		if(type != "LIRC")
			return false;
		for(map<string, vector<string> >::const_iterator it= m_mvCodes.begin(); it != m_mvCodes.end(); ++it)
		{
			found= find(it->second.begin(), it->second.end(), ID);
			if(found != it->second.end())
			{
				bfound= true;
				break;
			}
		}
		return true;
	}

	short LircClient::write(const string id, const double value)
	{
		string directive, remote, code;
		vector<string> vec;

		split(vec, id, is_any_of("/"));
		if(	vec.size() != 3 ||
			vec[0] == "" ||
			vec[0].substr(0, 6) != "write_" ||
			vec[1] == ""						)
		{
			TIMELOG(LOG_ERROR, id, "cannot define correctly ID '" + id + "' to send remote to LIRC");
			return -1;
		}
		directive= vec[2];
		remote= vec[0].substr(6);
		code= vec[1];
		if(!value && directive == "SEND_ONCE")
			return 0;
		if(directive == "SEND")
		{
			if(value)
			{
				if(m_mSend[id] == true)
					return 0; // do not send START command when START running
				directive+= "_START";
				m_mSend[id]= true;

			}else
			{
				directive+= "_STOP";
				if(m_mSend[id] == false)
					return 0; // do not send STOP command when no START running
				m_mSend[id]= false;
			}
		}
		//cout << "send to lirc '" << "irsend " << directive << " " << remote << " " << code << "'" << endl;
		irsend(directive.c_str(), remote.c_str(), code.c_str());
		return 0;
	}

	string LircClient::kernelmodule()
	{
		char *lircdef;
		string id("read_"), code, remote;
		map<string, bool>::iterator found;

		if(lirc_nextcode(&lircdef) == 0)
		{
			if(lircdef == NULL)
				return "";
			istringstream lirccode(lircdef);

			lirccode >> code; // hex number
			lirccode >> code; // count
			lirccode >> code; // code
			lirccode >> remote; // remote;
			free(lircdef);
			id+= remote + "/" + code;
			//cout << "get lirc code " << id << endl;
			LOCK(m_READMUTEX);
			found= m_mset.find(id);
			if(found != m_mset.end())
				found->second= true;
			UNLOCK(m_READMUTEX);
		}
		return "";
	}

	short LircClient::read(const string id, double &value)
	{
		short nRv= -1;
		map<string, bool>::iterator set;

		value= 0;
		LOCK(m_READMUTEX);
		set= m_mset.find(id);
		if(set != m_mset.end())
		{
			if(set->second == true)
			{
				value= 1;
				set->second= false;
			}
			nRv= 0;
		}
		UNLOCK(m_READMUTEX);
		return nRv;
	}

	void LircClient::range(const string id, double& min, double& max, bool &bfloat)
	{
		min= 0;
		max= 1;
		bfloat= false;
	}

	void LircClient::endOfCacheReading(const double cachetime)
	{

	}

	bool LircClient::reachAllChips()
	{
		return true;
	}

	LircClient::~LircClient()
	{
		disconnect();
	}
}
