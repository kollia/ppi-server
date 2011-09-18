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

#include "../pattern/util/LogHolderPattern.h"

#include "LircClient.h"

//extern "C" {
#include "irsend.h"
//}

namespace ports
{
	using namespace boost;

	bool LircClient::init(const IPropertyPattern* properties)
	{
		m_sLircSocket= properties->getValue("socket", /*warning*/false);
		if(m_sLircSocket == "")
			m_sLircSocket= "/var/run/lircd";
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
		id= remote + " " + code;
		if(bW2)
			id+= " SEND_ONCE";
		else if(bW1)
			id+= " SEND";
		if(bread)
		{
			kernelmodule= 2;
			id= "read " + id;
			//cout  << "'" << id  << "'" << endl;
			m_mset[id]= false;
			nRv= 1;
		}else
		{
			id= "write " + id;
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
			string msg("### ERROR: cannot initial LIRC\n");

			msg+= "    ERRNO: " + *strerror(errno);
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
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

	short LircClient::write(const string id, const double value, const string& addinfo)
	{
		int res;
		unsigned long num(1);
		vector<string> vec;
		vector<string>::size_type count;

		split(vec, id, is_any_of(" "));
		count= vec.size();
		if(	count != 4 ||
			vec[0] != "write" ||
			vec[1] == "" ||
			vec[2] == "" ||
			(	vec[3] != "SEND" &&
				vec[3] != "SEND_ONCE"	) )
		{
			TIMELOG(LOG_ERROR, id, "cannot define correctly ID '" + id + "' to send remote to LIRC");
			cerr << "### ERROR: cannot define correctly ID '" + id + "' to send remote to LIRC" << endl;
			return -2;
		}
		//cout << "write code with id '" << id << "' with value " << value << " send " << addinfo << " units" << endl;
		if(!value && vec[3] == "SEND_ONCE")
			return 0;
		if(	vec[3] == "SEND_ONCE" &&
			addinfo != ""				)
		{
			istringstream snum(addinfo);

			snum >> num;
			if(num == 0)
			{
				string err("undefined count (’");

				err+= addinfo + "') for LIRC irsend given. Set count to 1.";
				LOG(LOG_WARNING, err);
				cerr << "### ERROR: " << err << endl;
				num= 1;
			}
		}
		if(vec[3] == "SEND")
		{
			if(value)
			{
				if(m_mSend[id] == true)
					return 0; // do not send START command when START running
				vec[3]+= "_START";
				m_mSend[id]= true;

			}else
			{
				vec[3]+= "_STOP";
				if(m_mSend[id] == false)
					return 0; // do not send STOP command when no START running
				m_mSend[id]= false;
			}
		}
		//cout << "send to lirc '" << "irsend " << vec[3] << " " << vec[1] << " " << vec[2] << " " << num << "'" << endl;
		res= irsend(m_sLircSocket.c_str(), vec[3].c_str(), vec[1].c_str(), vec[2].c_str(), num);
		if(res == PACKETERROR)
		{
			TIMELOG(LOG_ERROR, "packaterror"+id, "wrong sending LIRC command with id '" + id + "'");
			//cerr << "wrong sending LIRC command with id '" + id + "'" << endl;
			return -2;
		}
		if(res == TRANSACTIONERROR)
		{
			TIMELOG(LOG_ERROR, "LIRC_TRANSACTION", "cannot create correctly transaction to LIRC");
			return -1;
		}
		return 0;
	}

	string LircClient::kernelmodule(map<string, bool>& read)
	{
		unsigned short num;
		char *lircdef;
		string id, code, remote;
		map<string, unsigned short>::iterator found;

		if(lirc_nextcode(&lircdef) == 0)
		{
			if(lircdef == NULL)
				return "";
			istringstream lirccode(lircdef);

			//cout << lircdef;
			lirccode >> code; // hex number
			//cout << "hex   :" << code << endl;
			lirccode >> hex >> num; // count
			//cout << "count: " << num << endl;
			lirccode >> code; // code
			lirccode >> remote; // remote;
			//cout << "code:  " << code << endl;
			//cout << "remote:" << remote << endl << endl;
			free(lircdef);
			id= "read " + remote + " " + code;
			//cout << "get value " << num << " on lirc code " << id << endl;
			read[id]= true;
			LOCK(m_READMUTEX);
			found= m_mset.find(id);
			if(found != m_mset.end())
				found->second= num+1;
			UNLOCK(m_READMUTEX);
		}else
		{
			string msg("### ERROR: by reading native lircd\n");

			msg+= "    LIRC:  " + *lircdef;
			if(errno)
				msg+= "\n    ERRNO: " + *strerror(errno);
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			free(lircdef);
		}
		return "";
	}

	short LircClient::read(const string id, double &value)
	{
		short nRv= -1;
		map<string, unsigned short>::iterator set;

		value= 0;
		LOCK(m_READMUTEX);
		set= m_mset.find(id);
		if(set != m_mset.end())
		{
			value= (double)set->second;
			set->second= 0;
			if(value == 0)
				nRv= 4;
			else
				nRv= 0;
		}
		UNLOCK(m_READMUTEX);
		return nRv;
	}

	void LircClient::range(const string id, double& min, double& max, bool &bfloat)
	{
		min= 0;
		max= 600;
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
