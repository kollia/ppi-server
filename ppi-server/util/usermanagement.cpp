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
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "usermanagement.h"

#include "../logger/LogThread.h"

#include "../util/configpropertycasher.h"

using namespace std;
using namespace util;

namespace user
{

	UserManagement* UserManagement::_instance= NULL;

	UserManagement::UserManagement()
	{

	}

	bool UserManagement::initial(const string file)
	{
		if(!_instance)
		{
			_instance= new UserManagement();
			if(_instance->init(file))
				return true;
		}
		return false;
	}

	bool UserManagement::init(const string filename)
	{
		string param;
		Properties casher;
		set<string> fGroups;
		set<string>::iterator git;
		vector<string> split;
		vector<string>::size_type count, ecount;
		map<string, set<string> > msGroupGroup;
		map<string, set<string> >::iterator ggit;

		if(!casher.readFile(filename))
		{
			string msg("### fatal ERROR: cannot read access config file ");

			msg+= filename + "\n";
			msg+= "                 ";
			msg+= errno;
			msg+= " ";
			msg+= strerror(errno);
			cerr << msg;
			LOG(AKALERT, msg);
			return false;
		}

		// define root-name
		m_sroot= casher.needValue("root");
		if(m_sroot == "")
			return false;
		param= casher.getValue("rootlogin");
		if(param == "true")
			m_bRootLogin= true;
		else
		{
			m_bRootLogin= false;
			if(	param != ""
				&&
				param != "false"	)
			{
				string msg("### WARNING; rootlogin '");

				msg+= param + "' is undefined string, so set rootlogin as false";

				LOG(AKWARNING, msg);
				cout << msg << endl;
			}
		}
		if(m_sroot.find(":") < m_sroot.size())
		{
			string msg("### fatal ERROR: root name should not containe an double point ':'");

			LOG(AKALERT, msg);
			cerr << msg << endl;
			return false;
		}
		// read whitch user are defined
		count= casher.getPropertyCount("user");
		ecount= 0;
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("user", i, false);
			split= ConfigPropertyCasher::split(param, ":");
			if(split.size() != 2)
			{
				string msg("### WARNING: undefined user '");

				msg+= param + "'\n";
				msg+= "             an user must be defined with <username>:<password> seperatly with an double point ':'\n";
				msg+= "             this user will be ignored";
			}else
			{
				m_mUser[split[0]]= split[1];
				++ecount;
			}
		}
		if(ecount == 0)
		{
			string msg("## fatal ERROR: no user are defined in access.conf");

			LOG(AKALERT, msg);
			cerr << msg << endl;
			return false;
		}
		// read all groups with permission
		count= casher.getPropertyCount("group");
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("group", i, false);
			split= ConfigPropertyCasher::split(param, ":");
			if(split.size() != 2)
			{
				string msg("### WARNING: undefined group '");

				msg+= param + "'\n";
				msg+= "             an group must be defined with <groupname>:<permission> seperatly with an double point ':'\n";
				msg+= "             this user will be ignored";
			}else
				m_mGroup[split[0]]= split[1];
		}
		// read all allocation with user to group into m_msAllocation
		// or goup to group into msGroupGroup for read allocation in referedGroups
		count= casher.getPropertyCount("alloc");
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			bool ok= false;

			param= casher.getValue("alloc", i, false);
			split= ConfigPropertyCasher::split(param, ":");
			if(split.size() == 3)
			{
				if(split[0] == "u")
				{
					fGroups.clear();
					referedGroups(split[2], &fGroups, msGroupGroup);
					for(set<string>::iterator f= fGroups.begin(); f != fGroups.end(); ++f)
						m_msAllocation[split[1]].insert(*f);
					ok= true;
				}else if(split[0] == "g")
				{
					msGroupGroup[split[1]].insert(split[2]);
					ok= true;
				}
			}
			if(ok == false)
			{
				string msg("### WARNING: undefined allocation '");

				msg+= param + "'\n";
				msg+= "             an allocation must be defined with tree strings <u|g>:<user:group>:<group> seperatly with an double point ':'\n";
				msg+= "             this user will be ignored";
			}
		}
		return true;
	}

	string UserManagement::getNoRootUser() const
	{
		string sRv;

		for(map<string, string>::const_iterator it= m_mUser.begin(); it != m_mUser.end(); ++it)
		{
			sRv= it->first;
			if(sRv != m_sroot)
				break;
		}
		return sRv;
	}

	string UserManagement::getRootUser() const
	{
		return m_sroot;
	}

	string UserManagement::getPassword(const string user) const
	{
		map<string, string>::const_iterator fu;

		fu= m_mUser.find(user);
		if(fu != m_mUser.end())
			return fu->second;
		return "";
	}

	bool UserManagement::isUser(const string user) const
	{
		map<string, string>::const_iterator fu;

		fu= m_mUser.find(user);
		if(fu != m_mUser.end())
			return true;
		return false;
	}

	bool UserManagement::isRoot(const string user) const
	{
		if(m_sroot == user)
			return true;
		return false;
	}

	bool UserManagement::rootLogin() const
	{
		return m_bRootLogin;
	}

	bool UserManagement::hasAccess(const string user, const string password, const bool login) const
	{
		map<string, string>::const_iterator fu;

		if( !m_bRootLogin
			&&
			login
			&&
			user == m_sroot	)
		{
			return false;
		}
		fu= m_mUser.find(user);
		if(fu != m_mUser.end())
		{
			if(fu->second == password)
				return true;
		}
		return false;
	}

	bool UserManagement::hasPermission(string user, string groups, string access)
	{
		map<string, set<string> >::iterator fu;
		map<string, string>::iterator fg;
		set<string>::iterator fga;
		vector<string> split;

		if(user == m_sroot)
			return true;
		fu= m_msAllocation.find(user);
		if(fu == m_msAllocation.end())
			return false;
		split= ConfigPropertyCasher::split(groups, ":");
		for(vector<string>::iterator i= split.begin(); i != split.end(); ++i)
		{
			fga= fu->second.find(*i);
			if(fga != fu->second.end())
			{
				if(access == "read")
					return true;
				if(access == "write")
				{
					fg= m_mGroup.find(*i);
					if(	fg != m_mGroup.end()
						&&
						fg->second == "write"	)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	void UserManagement::referedGroups(const string group, set<string>* groups, const map<string, set<string> > exist)
	{
		map<string, string>::iterator fg;
		map<string, set<string> >::const_iterator fe;

		fg= m_mGroup.find(group);
		if(fg != m_mGroup.end())
		{
			groups->insert(group);
			return;
		}
		fe= exist.find(group);
		if(fe == exist.end())
			return;
		for(set<string>::iterator f= fe->second.begin(); f != fe->second.end(); ++f)
			referedGroups(*f, groups, exist);
	}

	UserManagement* UserManagement::instance()
	{
		return _instance;
	}

	UserManagement::~UserManagement()
	{
		_instance= NULL;
	}

}
