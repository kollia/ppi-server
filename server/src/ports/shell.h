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
#ifndef SHELL_H_
#define SHELL_H_

#include "switch.h"

#include "../util/properties/configpropertycasher.h"

using namespace util;


class Shell : public switchClass
{
public:
	Shell(string folderName, string subroutineName)
	: switchClass(folderName, subroutineName) { };
	virtual bool init(ConfigPropertyCasher &properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
	/**
	 * measure new value for subroutine
	 * and trigger shell command
	 *
	 * @return return 0 for nothing done, 1 by made beginning command, 2 by while command and 3 by ending command
	 */
	virtual double measure();
	virtual ~Shell();

protected:
	/**
	 * this method is an dummy
	 * because the value can not write into database
	 * and be never set
	 *
	 * @param bfloat whether the values can be float variables
	 * @param min the minimal value
	 * @param max the maximal value
	 * @return whether the range is defined or can set all
	 */
	virtual bool range(bool& bfloat, double* min, double* max);

private:
	string m_sBeginCom;
	string m_sWhileCom;
	string m_sEndCom;

	void system(const char *command);
};

#endif /*SHELL_H_*/
