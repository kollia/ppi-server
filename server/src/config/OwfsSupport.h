/**
 *   This file 'OwfsSupport.h' is part of ppi-server.
 *   Created on: 13.11.2011
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

#ifdef _OWFSLIBRARY
#ifndef OWFSSUPPORT_H_
#define OWFSSUPPORT_H_

#include "../pattern/util/ICommandStructPattern.h"

class OwfsSupport
{
public:
	/**
	 * constructor to set working directory
	 *
	 * @param workdir working directory
	 */
	OwfsSupport(const string workdir) :
		m_sWorkDir(workdir)
	{};
	/**
	 * configuration for lirc settings
	 *
	 * @param argc count of values
	 * @param argv values in an pointer array
	 */
	int execute(const ICommandStructPattern* params);

private:
	/**
	 * working directory
	 */
	const string m_sWorkDir;
};

#endif /* OWFSSUPPORT_H_ */
#endif /* _OWFSLIBRARY */


