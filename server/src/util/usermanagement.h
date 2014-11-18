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
#ifndef USERMANAGEMENT_H_
#define USERMANAGEMENT_H_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "../pattern/util/IUsermanagementPattern.h"

#include "UserManagementInitialization.h"

namespace user
{
	using namespace std;
	using namespace design_pattern_world;

	class UserManagement : public IUserManagementPattern
	{
		public:
			/**
			 * static method to create single object of UserManagement
			 *
			 * @param access file witch have configuration of access
			 * @param measure file with path for measure.conf,
			 * 			when permision groups of folder not be needed, this parameter can be an null string ("")
			 * @return whether an new object is created
			 */
			static bool initial(const string& access, const string& measure);
			/**
			 * method to load the access properties from harddisk
			 *
			 * @param access file witch have configuration of access
			 * @param measure file with path for measure.conf,
			 * 			when permision groups of folder not be needed, this parameter can be an null string ("")
			 * @return whether the file loading was successful
			 */
			OVERWRITE bool init(const string& access, const string& measure);
			/**
			 * whether initialization of user-management is finished
			 *
			 * @return 1 if finished, 0 by running and -1 when some errors occurred
			 */
			OVERWRITE short finished();
			/**
			 * delete single object of UserManagement
			 */
			static void deleteObj();
			/**
			 * static method to get instance of UserManagement
			 *
			 * @return object of UserManagement
			 */
			static UserManagement* instance();
			/**
			 * whether user can login as first
			 *
			 * @param user name of user
			 * @return wethe3r root can login
			 */
			OVERWRITE bool canLoginFirst(const string& user) const;
			/**
			 * returns an user whitch is not root
			 *
			 * @return user name
			 */
			OVERWRITE string getNoRootUser() const;
			/**
			 * return name of root
			 *
			 * @return name of root
			 */
			OVERWRITE string getRootUser() const;
			/**
			 * return password from given user,
			 * otherwise if user not exist an null terminated string
			 *
			 * @param user name of user
			 * @return passord from user
			 */
			OVERWRITE string getPassword(const string& user) const;
			/**
			 * returns true if given user exists
			 *
			 * @param user name of user
			 * @return whether user exist
			 */
			OVERWRITE bool isUser(const string& user) const;
			/**
			 * returns true if given user has permission to system
			 * with given passord
			 *
			 * @param name of user
			 * @param password given password
			 * @param login whether access request is first login or change user
			 * @return whether user have permission
			 */
			OVERWRITE bool hasAccess(const string& user, const string& password, const bool login) const;
			/**
			 * return all set groups for subroutine in measure.conf
			 *
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 * @return groups
			 */
			OVERWRITE string getAccessGroups(const string& folder, const string& subroutine);
			/**
			 * whether user has access to given groups
			 *
			 * @param user name of user
			 * @param folder name of folder
			 * @param subroutine name of subroutine
			 * @param access look access for given param. can be 'read' or 'write'
			 * @return whether user has access
			 */
			OVERWRITE bool hasPermission(const string& user, const string& folder, const string& subroutine, const string& access);
			/**
			 * whether user has access to given groups
			 *
			 * @param user name of user
			 * @param groups name of groups seperatly with an double point ':'
			 * @param access look access for given param. can be 'read' or 'write'
			 * @return whether user has access
			 */
			OVERWRITE bool hasPermission(const string& user, string groups, const string& access);
			/**
			 * destructor of user-management
			 */
			virtual ~UserManagement();

		protected:
			/**
			 * name of root
			 */
			string m_sroot;
			/**
			 * vector of all users which can not login as first
			 */
			set<string> m_sNoFirstLogin;
			/**
			 * map from all user as key and password as value
			 */
			map<string, string> m_mUser;
			/**
			 * map with all groups show to permissions ("read" or "write")
			 */
			map<string, map<string, bool> > m_mmCluster;
			/**
			 * map with user names and accesible groups
			 */
			map<string, set<string> > m_msAllocation;

		private:
			/**
			 * instance of own single object
			 */
			static UserManagement* _instance;
			/**
			 * running initialization in seperate thread
			 */
			UserManagementInitialization* m_pInitial;
			/**
			 * all exist groups for root permission.<br />
			 * <code>map&lt;group, write&gt;</code>
			 */
			map<string, bool> m_msbGroups;
			/**
			 * permission groups for folder and subroutines
			 */
			map<string, map<string, string> > m_mmGroups;

			/**
			 * constructor of creating an user-management
			 */
			UserManagement();
			/**
			 * insert cluster into sorted vector
			 *
			 * @param cluster name of cluster which should be inserted
			 * @param inContainer container in which should be inserted for sorted order
			 * @param reference name of reference cluster
			 */
			void insert(const string& cluster, vector<string>& inContainer, const string& reference,
					const map<string, set<string> > refClusters);
			/**
			 * private method do found for given group all real defined group with permission
			 * read or write
			 *
			 * @param group name of group whitch should be an original access group or a refered group
			 * @param groups set of groups whitch contains all original groups after call the method.<br />should be empty before
			 * @param exist allallocations from refered groups to an group whitch are readed before
			 */
			void referedGroups(const string& group, set<string>* groups, const map<string, set<string> > exist);
	};

}

#endif /*USERMANAGEMENT_H_*/
