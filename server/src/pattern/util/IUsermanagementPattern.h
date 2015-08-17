/**
 *   This file 'IUsermanagementPattern.h' is part of ppi-server.
 *   Created on: 26.12.2011
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


#ifndef IUSERMANAGEMENTPATTERN_H_
#define IUSERMANAGEMENTPATTERN_H_

#include <string>

namespace design_pattern_world
{
	using namespace std;

	class IUserManagementPattern
	{
	public:
		/**
		 * enumerate for method hasAccess
		 * to know whether user login by first time,
		 * change user, or change password as hearing client
		 */
		enum login_t
		{
			LOGIN= 1,
			CHANGE,
			PWDCHECK
		};
		/**
		 * method to load the access properties from harddisk
		 *
		 * @param access file witch have configuration of access
		 * @param measure file with path for measure.conf,
		 * 			when permision groups of folder not be needed, this parameter can be an null string ("")
		 * @return whether the file loading was successful
		 */
		virtual bool init(const string& access, const string& measure)= 0;
		/**
		 * whether initialization of user-management is finished
		 *
		 * @return 1 if finished, 0 by running and -1 when some errors occurred
		 */
		virtual short finished()= 0;
		/**
		 * whether user can login as first
		 *
		 * @param user name of user
		 * @return wethe3r root can login
		 */
		virtual bool canLoginFirst(const string& user) const= 0;
		/**
		 * returns an user whitch is not root
		 *
		 * @return user name
		 */
		virtual string getNoRootUser() const= 0;
		/**
		 * return name of root
		 *
		 * @return name of root
		 */
		virtual string getRootUser() const= 0;
		/**
		 * return password from given user,
		 * otherwise if user not exist an null terminated string
		 *
		 * @param user name of user
		 * @return passord from user
		 */
		virtual string getPassword(const string& user) const= 0;
		/**
		 * returns true if given user exists
		 *
		 * @param user name of user
		 * @return whether user exist
		 */
		virtual bool isUser(const string& user) const= 0;
		/**
		 * returns true if given user has permission to system
		 * with given passord
		 *
		 * @param name of user
		 * @param password given password
		 * @param ID server access ID for client
		 * @param login whether access request is first login (LOGIN),
		 *              man client change user (CHANGE),
		 *              or hearing client check password to have same user as main client (PWDCHECK)
		 * @return whether user have permission
		 */
		virtual bool hasAccess(const string& user, const string& password,
						const unsigned int ID, const login_t login)= 0;
		/**
		 * clear server access ID for client
		 * when transaction will be closed
		 *
		 * @param ID access ID to remove
		 */
		virtual void clearAccessID(const unsigned int ID)= 0;
		/**
		 * return all set groups for subroutine in measure.conf
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return groups
		 */
		virtual string getAccessGroups(const string& folder, const string& subroutine)= 0;
		/**
		 * whether user has access to given groups
		 *
		 * @param user name of user
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param access look access for given param. can be 'read' or 'write'
		 * @return whether user has access
		 */
		virtual bool hasPermission(const string& user, const string& folder, const string& subroutine, const string& access)= 0;
		/**
		 * whether user has access to given groups
		 *
		 * @param user name of user
		 * @param groups name of groups seperatly with an double point ':'
		 * @param access look access for given param. can be 'read' or 'write'
		 * @return whether user has access
		 */
		virtual bool hasPermission(const string& user, string groups, const string& access)= 0;
		/**
		 * dummy destructor of user-management pattern
		 */
		virtual ~IUserManagementPattern() {};
	};
}
#endif /* IUSERMANAGEMENTPATTERN_H_ */
