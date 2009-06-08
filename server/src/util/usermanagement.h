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

namespace user
{
	using namespace std;

	class UserManagement
	{
		public:
			/**
			 * static method to create single object of UserManagement
			 *
			 * @param file file whitch have configuration of access
			 * @return whether an new object is created
			 */
			static bool initial(const string file);
			/**
			 * static method to get instance of UserManagement
			 *
			 * @return object of UserManagement
			 */
			static UserManagement* instance();
			/**
			 * returns true if given user is root
			 *
			 * @param user name of user
			 * @return whether user is root
			 */
			bool isRoot(const string user) const;
			/**
			 * whether root can login as first user
			 *
			 * @return wethe3r root can login
			 */
			bool rootLogin() const;
			/**
			 * returns an user whitch is not root
			 *
			 * @return user name
			 */
			string getNoRootUser() const;
			/**
			 * return name of root
			 *
			 * @return name of root
			 */
			string getRootUser() const;
			/**
			 * return password from given user,
			 * otherwise if user not exist an null terminated string
			 *
			 * @param user name of user
			 * @return passord from user
			 */
			string getPassword(const string user) const;
			/**
			 * returns true if given user exists
			 *
			 * @param user name of user
			 * @return whether user exist
			 */
			bool isUser(const string user) const;
			/**
			 * returns true if given user has permission to system
			 * with given passord
			 *
			 * @param name of user
			 * @param password given password
			 * @param login whether access request is first login or change user
			 * @return whether user have permission
			 */
			bool hasAccess(const string user, const string password, const bool login) const;
			/**
			 * whether user has access to given groups
			 *
			 * @param user name of user
			 * @param groups name of groups seperatly with an double point ':'
			 * @param access look access for given param. can be 'read' or 'write'
			 * @return whether user has access
			 */
			bool hasPermission(string user, string groups, string access);
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
			 * wether root can login as first user
			 */
			bool m_bRootLogin;
			/**
			 * map from all user as key and password as value
			 */
			map<string, string> m_mUser;
			/**
			 * map with all groups show to permissions ("read" or "write")
			 */
			map<string, string> m_mGroup;
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
			 * constructor of creating an user-management
			 */
			UserManagement();
			/**
			 * private method to load the access properties from harddisk
			 *
			 * @param file file whitch have configuration of access
			 * @return whether the file loading was successful
			 */
			bool init(const string filename);
			/**
			 * private method do found for given group all real defined group with permission
			 * read or write
			 *
			 * @param group name of group whitch should be an original access group or a refered group
			 * @param groups set of groups whitch contains all original groups after call the method.<br />should be empty before
			 * @param exist allallocations from refered groups to an group whitch are readed before
			 */
			void referedGroups(const string group, set<string>* groups, const map<string, set<string> > exist);
	};

}

#endif /*USERMANAGEMENT_H_*/
