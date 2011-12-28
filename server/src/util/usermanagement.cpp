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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "usermanagement.h"

#include "../pattern/util/LogHolderPattern.h"

#include "properties/interlacedproperties.h"

using namespace std;
using namespace util;
using namespace boost;

namespace user
{

	UserManagement* UserManagement::_instance= NULL;

	UserManagement::UserManagement()
	: m_pInitial(NULL)
	{
	}

	bool UserManagement::initial(const string& accessfile, const string& measurefile)
	{
		if(!_instance)
		{
			_instance= new UserManagement();
			_instance->m_pInitial= new UserManagementInitialization(accessfile, measurefile, _instance);
			_instance->m_pInitial->initialStarting();
			return true;
		}
		return false;
	}

	void UserManagement::deleteObj()
	{
		delete _instance;
		_instance= NULL;
	}

	short UserManagement::finished()
	{
		short nRv;

		nRv= m_pInitial->finished();
		if(nRv < 0)
			nRv= -1;
		return nRv;
	}

	bool UserManagement::init(const string& accessfile, const string& measurefile)
	{
		bool bset;
		bool permission;
		string param, msg;
		Properties casher;
		InterlacedProperties mproperties;
		set<string> fGroups;
		set<string>::iterator git;
		vector<string> splitvec;
		vector<string>::size_type count, ecount;
		map<string, set<string> > mssClusterRef;
		map<string, set<string> > msGroupGroup;
		map<string, map<string, bool> >::iterator clgrIt, clgrIt2;
		map<string, bool>::iterator grIt;
		map<string, string>::iterator usIt;

		mproperties.modifier("folder");
		mproperties.setMsgParameter("folder");
		mproperties.modifier("name");
		mproperties.setMsgParameter("name", "subroutine");
		mproperties.valueLocalization("\"", "\"", /*remove*/true);
		if(measurefile != "")
		{
			if(!mproperties.readFile(measurefile))
			{
				msg=  "### fatal ERROR: cannot read correctly measure file '";
				msg+= measurefile + "\n";
				msg+= "                 ";
				msg+= errno;
				msg+= " ";
				msg+= strerror(errno);
				cerr << msg;
				LOG(LOG_ALERT, msg);
				return false;
			}
		}
		if(!casher.readFile(accessfile))
		{
			msg= "### fatal ERROR: cannot read access config file ";
			msg+= accessfile + "\n";
			msg+= "                 ";
			msg+= errno;
			msg+= " ";
			msg+= strerror(errno);
			cerr << msg;
			LOG(LOG_ALERT, msg);
			return false;
		}

		// define root-name
		m_sroot= casher.needValue("root");
		if(m_sroot == "")
		{
			msg= "### fatal ERROR: no root user be set";
			LOG(LOG_ALERT, msg);
			cerr << msg << endl;
			return false;
		}
		if(m_sroot.find(":") != string::npos)
		{
			msg= "### fatal ERROR: root name should not contain an colon ':'";
			LOG(LOG_ALERT, msg);
			cerr << msg << endl;
			return false;
		}
		// read which user are defined
		count= casher.getPropertyCount("user");
		ecount= 0;
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("user", i, false);
			split(splitvec, param, is_any_of(":"));
			if(splitvec.size() != 2)
			{
				msg= "### WARNING: undefined user '";
				msg+= param + "'\n";
				msg+= "             an user must be defined with <username>:<password> separately with an colon ':'\n";
				msg+= "             this user will be ignored";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
			}else
			{
				m_mUser[splitvec[0]]= splitvec[1];
				++ecount;
				//cout << "read user '" << splitvec[0] << "' with password '" << splitvec[1] << "'" << endl;
			}
		}
		if(ecount == 0)
		{
			msg=  "## fatal ERROR: no user are defined in access.conf";
			LOG(LOG_ALERT, msg);
			cerr << msg << endl;
			return false;
		}
		// read all cluster with groups and permission
		count= casher.getPropertyCount("cluster");
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("cluster", i, false);
			split(splitvec, param, is_any_of(":"));
			if(splitvec.size() == 3)
			{// definition of <clustername>:<groupname>:<permission>
				trim(splitvec[0]);
				trim(splitvec[1]);
				trim(splitvec[2]);
				if(splitvec[2] == "write")
					permission= true;
				else
				{
					permission= false;
					if(splitvec[2] != "read")
					{
						msg= "### WARNING: for cluster '";
						msg+= param + "' in access.conf file\n";
						msg+= "             at third position permission can only be the names of 'read' or 'write'\n";
						msg+= "             set access to the group '" + splitvec[1] + "' only to read.";
						LOG(LOG_WARNING, msg);
						cout << msg << endl;
					}
				}
				bset= false;
				if(permission == false)
				{
					clgrIt= m_mmCluster.find(splitvec[0]);
					if(clgrIt != m_mmCluster.end())
					{
						grIt= clgrIt->second.find(splitvec[1]);
						if(grIt == clgrIt->second.end())
							bset= true;
					}else
						bset= true;
				}else
					bset= true;
				if(bset == true)
				{
					m_mmCluster[splitvec[0]][splitvec[1]]= permission;
					//cout << "put to cluster '" << splitvec[0] << "' group '" << splitvec[1] << "' with permission '"
					//		<< splitvec[2] << "'" << endl;
				}
				if(permission == false)
				{
					grIt= m_msbGroups.find(splitvec[1]);
					if(grIt == m_msbGroups.end())
						m_msbGroups[splitvec[1]]= permission;
				}else
					m_msbGroups[splitvec[1]]= permission;

			}else if(splitvec.size() == 2)
			{// definition of <clustername>:<clustername>
				trim(splitvec[0]);
				trim(splitvec[1]);
				mssClusterRef[splitvec[0]].insert(splitvec[1]);

			}else
			{
				msg= "### WARNING: undefined cluster '";
				msg+= param + "' in access.conf file\n";
				msg+= "             an cluster must be defined with <clustername>:<groupname>:<permission> or <clustername>:<clustername>\n";
				msg+= "             separately with colon's ':'. this cluster will be ignored";
				LOG(LOG_WARNING, msg);
				cout << msg << endl;
			}
		}

		vector<string> vClusterOrder;
		vector<string>::iterator mfIt;
		map<string, set<string> >::iterator cl;

		// sort referred clusters
		for(map<string, set<string> >::iterator cl= mssClusterRef.begin(); cl != mssClusterRef.end(); ++cl)
		{
			//cout << "reference from " << cl->first << endl;
			mfIt= find(vClusterOrder.begin(), vClusterOrder.end(), cl->first);
			if(mfIt == vClusterOrder.end())
			{
				//cout << "insert " << cl->first << " as last" << endl;
				vClusterOrder.push_back(cl->first);
			}
			for(set<string>::iterator tocl= cl->second.begin(); tocl != cl->second.end(); ++tocl)
				insert(*tocl, vClusterOrder, cl->first, mssClusterRef);
			/*cout << "new order of clusters:" << endl;
			for(vector<string>::iterator it= vClusterOrder.begin(); it != vClusterOrder.end(); ++it)
				cout << "            " << *it << endl;
			cout << endl;*/
		}

		// write all cluster refer to an other in order from vClusterOrder
		for(vector<string>::iterator it= vClusterOrder.begin(); it != vClusterOrder.end(); ++it)
		{
			// search ordered cluster in referred cluster group to begin
			cl= mssClusterRef.find(*it);
			if(cl != mssClusterRef.end())
			{
				// by loop from all cluster to which referred:
				for(set<string>::iterator tocl= cl->second.begin(); tocl != cl->second.end(); ++tocl)
				{
					// search to refer cluster in exist cluster group where groups are written
					//cout << "cluster " << *it << " has reference to cluster " << *tocl << endl;
					clgrIt= m_mmCluster.find(*tocl);
					if(clgrIt != m_mmCluster.end())
					{
						// search whether cluster refer to cluster already exists with other groups in cluster group
						clgrIt2= m_mmCluster.find(cl->first);
						if(clgrIt2 != m_mmCluster.end())
						{
							for(map<string, bool>::iterator fromgr= clgrIt->second.begin(); fromgr != clgrIt->second.end(); ++fromgr)
							{
								if(fromgr->second == false)
								{	// if new filled in group has only an read permission
									// search whether group exit.
									// When exist, do not fill again,
									// because the exist group has maybe an write permission
									grIt= clgrIt2->second.find(fromgr->first);
									//cout << "need to insert into cluster group group " << fromgr->first
									//		<< " which has permission write:" << boolalpha << fromgr->second << "?" << endl;
									if(grIt == clgrIt2->second.end())
									{
										clgrIt2->second[fromgr->first]= fromgr->second;
										//cout << "put to cluster '" << clgrIt2->first << "' group '" << fromgr->first
										//		<< "' with permission write:" << boolalpha << fromgr->second << endl;
									}
									//else cout << "exist group " << grIt->first << " has permission write:" << boolalpha
									//			<< grIt->second << endl;

								}else
								{
									clgrIt2->second[fromgr->first]= fromgr->second;
									//cout << "put to cluster '" << clgrIt2->first << "' group '" << fromgr->first
									//		<< "' with permission write:" << boolalpha << fromgr->second << endl;
								}
							}
						}else
						{// cluster do not exist in cluster group,
						 // so insert hole reference
							m_mmCluster[cl->first]= clgrIt->second;
							//cout << "put to cluster '" << cl->first << "' reference to cluster '" << clgrIt->first << "'" << endl;
						}
					}else
					{
						msg= "### ERROR: for cluster reference'";
						msg+= cl->first + ":" + *tocl + "' in access.conf file\n";
						msg+= "             cannot find the cluster to refer. This cluster will be ignored";
						LOG(LOG_ERROR, msg);
						cerr << msg << endl;
					}
				}
			}// if(cl != mssClusterRef.end())
		}

		// read all allocation with user to cluster into m_msAllocation
		count= casher.getPropertyCount("alloc");
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("alloc", i, false);
			split(splitvec, param, is_any_of(":"));
			if(splitvec.size() == 2)
			{
				trim(splitvec[0]);
				trim(splitvec[1]);
				usIt= m_mUser.find(splitvec[0]);
				if(	usIt != m_mUser.end() ||
					splitvec[0] == m_sroot	)
				{
					clgrIt= m_mmCluster.find(splitvec[1]);
					if(clgrIt != m_mmCluster.end())
					{
						m_msAllocation[splitvec[0]].insert(splitvec[1]);
					}else
					{
						msg=  "### WARNING: undefined cluster in allocation '" + param + "'\n";
						msg+= "             an allocation must be defined with string <user>:<cluster> separately with an colon ':'\n";
						msg+= "             this cluster for user '" + splitvec[0] + "' will be ignored";
						cout << msg << endl;
						LOG(LOG_WARNING, msg);
					}
				}else
				{
					msg=  "### WARNING: undefined user in allocation '" + param + "'\n";
					msg+= "             an allocation must be defined with string <user>:<cluster> separately with an colon ':'\n";
					msg+= "             this user will be ignored";
					cout << msg << endl;
					LOG(LOG_WARNING, msg);
				}
			}
		}

		// read all user which should not login as first
		count= casher.getPropertyCount("noFirstLogin");
		for(vector<string>::size_type i= 0; i < count; ++i)
		{
			param= casher.getValue("noFirstLogin", i, false);
			trim(param);
			usIt= m_mUser.find(param);
			if(	usIt != m_mUser.end() ||
				param == m_sroot	)
			{
				m_sNoFirstLogin.insert(param);
			}else
			{
				msg=  "### WARNING: undefined user  '" + param + "' for no first login (noFirstLogin) found in access.conf";
				cerr << msg << endl;
				LOG(LOG_WARNING, msg);
			}
		}
		if(m_sNoFirstLogin.size() == m_mUser.size())
		{
			msg=  "### ERROR: no user can login, for server account, as first\n";
			msg+= "           same defined count of users be in noFirstLogin parameters.";
			cerr << msg << endl;
			LOG(LOG_ALERT, msg);
			return false;
		}

		if(measurefile != "")
		{
			// read all permission groups for folder:subroutines in file measure.conf
			typedef vector<IInterlacedPropertyPattern*>::iterator sit;

			string folder, subroutine, groups;
			vector<IInterlacedPropertyPattern*> fsection, ssection;

			fsection= mproperties.getSections();
			for(sit vfit= fsection.begin(); vfit != fsection.end(); ++vfit)
			{
				ssection= (*vfit)->getSections();
				for(sit vsit= ssection.begin(); vsit != ssection.end(); ++vsit)
				{
					folder= (*vfit)->getSectionValue();
					subroutine= (*vsit)->getSectionValue();
					groups= (*vsit)->getValue("perm", /*warning*/false);
					if(groups != "")
					{
						m_mmGroups[folder][subroutine]= groups;
						//cout << "allow group '" << groups << "' for subroutine " << folder << ":" << subroutine << endl;
					}
				}
			}
		}
		return true;
	}

	void UserManagement::insert(const string& cluster, vector<string>& inContainer, const string& reference,
			const map<string, set<string> > refClusters)
	{
		bool bInsert;
		vector<string>::iterator mfIt, fIt, sIt;
		map<string, set<string> >::const_iterator rfCIt;

		//cout << "insert " << cluster << endl;
		mfIt= find(inContainer.begin(), inContainer.end(), reference);
		fIt= find(inContainer.begin(), inContainer.end(), cluster);
		if(fIt == inContainer.end())
		{
			bInsert= true;
			sIt= inContainer.end();
			if(mfIt != inContainer.begin())
			{
				bInsert= false;
				for(sIt= inContainer.begin(); sIt != mfIt; ++sIt)
				{
					rfCIt= refClusters.find(*sIt);
					if(rfCIt != refClusters.end())
					{
						for(set<string>::const_iterator setIt= rfCIt->second.begin(); setIt != rfCIt->second.end(); ++setIt)
						{
							//cout << "found " << rfCIt->first << " refer to " << *setIt << endl;
							if(*setIt == cluster)
							{
								bInsert= true;
								break;
							}
						}
						if(bInsert)
							break;
					}
				}
				bInsert= true;
			}
			if(bInsert)
			{
				if(sIt == inContainer.end())
					inContainer.insert(mfIt, cluster);
				else
					inContainer.insert(sIt, cluster);
			}
		}else // if(fIt == inContainer.end())
		{// cluster exist in container
			if(fIt > mfIt)
			{
				string err;

				err=  "ERROR: user management cannot refer from cluster '" + *mfIt + "' to cluster '" + *fIt + "'\n";
				err+= "       cluster '" + *fIt + "' has an link back to cluster '" + *mfIt + "'";
				LOG(LOG_ERROR, err);
				cerr << err << endl;
			}
		}
	}

	string UserManagement::getNoRootUser() const
	{
		string sRv;

		for(map<string, string>::const_iterator it= m_mUser.begin(); it != m_mUser.end(); ++it)
		{
			sRv= it->first;
			if(	sRv != m_sroot &&
				canLoginFirst(sRv)	)
			{
				break;
			}
		}
		return sRv;
	}

	string UserManagement::getRootUser() const
	{
		return m_sroot;
	}

	string UserManagement::getPassword(const string& user) const
	{
		map<string, string>::const_iterator fu;

		fu= m_mUser.find(user);
		if(fu != m_mUser.end())
			return fu->second;
		return "";
	}

	bool UserManagement::isUser(const string& user) const
	{
		map<string, string>::const_iterator fu;

		fu= m_mUser.find(user);
		if(fu != m_mUser.end())
			return true;
		return false;
	}

	bool UserManagement::canLoginFirst(const string& user) const
	{
		set<string>::iterator found;

		found= m_sNoFirstLogin.find(user);
		if(found == m_sNoFirstLogin.end())
			return true;
		return false;
	}

	bool UserManagement::hasAccess(const string& user, const string& password, const bool login) const
	{
		map<string, string>::const_iterator fu;
		set<string>::iterator found;

		if(login == true)
		{
			found= m_sNoFirstLogin.find(user);
			if(found != m_sNoFirstLogin.end())
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

	string UserManagement::getAccessGroups(const string& folder, const string& subroutine)
	{
		map<string, map<string, string> >::iterator fit;
		map<string, string>::iterator sit;

		fit= m_mmGroups.find(folder);
		if(fit == m_mmGroups.end())
			return "";
		sit= fit->second.find(subroutine);
		if(sit == fit->second.end())
			return "";
		return sit->second;
	}

	bool UserManagement::hasPermission(const string& user, const string& folder, const string& subroutine, const string& access)
	{
		string groups;

		groups= getAccessGroups(folder, subroutine);
		return hasPermission(user, groups, access);
	}

	bool UserManagement::hasPermission(const string& user, string groups, const string& access)
	{
		bool bRoot(true);
		map<string, map<string, bool> >::iterator cl;
		map<string, set<string> >::iterator fu;
		map<string, string>::iterator fg;
		map<string, bool>::iterator aGr;
		vector<string> splitvec;

		trim(groups);
		if(groups == "")
			return false;
		if(user != m_sroot)
		{
			bRoot= false;
			fu= m_msAllocation.find(user);
			if(fu == m_msAllocation.end())
				return false;
		}
		split(splitvec, groups, is_any_of(":"));
		// search by all access groups for specific subroutine in measure.conf
		for(vector<string>::iterator i= splitvec.begin(); i != splitvec.end(); ++i)
		{
			trim(*i);
			if(bRoot)
			{
				aGr= m_msbGroups.find(*i);
				if(aGr != m_msbGroups.end())
				{
					if(access == "read")
						return true;
					if(	access == "write" &&
						aGr->second == true	)
					{
						return true;
					}else
						return false;
				}
			}else
			{
				// search by all clusters defined for user
				for(set<string>::iterator c= fu->second.begin(); c != fu->second.end(); ++c	)
				{
					cl= m_mmCluster.find(*c);
					if(cl != m_mmCluster.end())
					{
						// search for group in cluster
						aGr= cl->second.find(*i);
						if(aGr != cl->second.end())
						{
							if(access == "read")
								return true;
							if(	access == "write" &&
								aGr->second == true	)
							{
								return true;
							}
						}
					}
				}
			}
		}
		return false;
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
