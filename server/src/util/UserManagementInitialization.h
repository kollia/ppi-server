/**
 *   This file 'UserManagementInitialization.h' is part of ppi-server.
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

#ifndef USERMANAGEMENTINITIALIZATION_H_
#define USERMANAGEMENTINITIALIZATION_H_

#include <string>

#include "../pattern/util/IUsermanagementPattern.h"
#include "thread/CallbackTemplate.h"

using namespace std;
using namespace design_pattern_world;

class UserManagementInitialization : public CallbackTemplate
{
public:
	UserManagementInitialization(const string& access, const string& measure, IUserManagementPattern* usermanagement)
	:	CallbackTemplate("UserManagementInitialization"),
	 	m_sAccess(access),
	 	m_sMeasure(measure),
		m_pUserManagement(usermanagement)
		{};
	virtual ~UserManagementInitialization() {};

private:
	string m_sAccess;
	string m_sMeasure;
	IUserManagementPattern* m_pUserManagement;

	OVERWRITE bool runnable()
	{ 	if(!m_pUserManagement->init(m_sAccess, m_sMeasure))
			m_pError->setError("UserManagementInitialization", "init");
		return false; /* do not start runnable again*/                                           }
};

#endif /* USERMANAGEMENTINITIALIZATION_H_ */
