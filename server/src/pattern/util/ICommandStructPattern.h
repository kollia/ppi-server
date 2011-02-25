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

#ifndef ICOMMANDSTRUCTPATTERN_H_
#define ICOMMANDSTRUCTPATTERN_H_

#include <string>

#include "IOptionStructPattern.h"

using namespace std;

class ICommandStructPattern : virtual public IOptionStructPattern
{
public:
	/**
	 * return name of actual command
	 *
	 * @return name of command
	 */
	virtual string command() const= 0;
	/**
	 * return content of actual command
	 *
	 * @return content behind command
	 */
	virtual string getCommandContent() const= 0;
	/**
	 * return content of actual command
	 * casted as integer number
	 *
	 * @param fault is '##ERROR' if content is no integer or '##NULL' when not set
	 * @return content behind command
	 */
	virtual int getCommandIntContent(string& fault) const= 0;
	/**
	 * return content of actual command
	 * casted as floating number
	 *
	 * @param fault is '##ERROR' if content is no float or '##NULL' when not set
	 * @return content behind command
	 */
	virtual float getCommandFloatContent(string& fault) const= 0;
	/**
	 * return pattern of next called command which inherit
	 * all other commands and also options
	 *
	 * @return next command
	 */
	virtual const ICommandStructPattern* getNextCommand() const= 0;
	/**
	 * dummy destructor
	 */
	virtual ~ICommandStructPattern() {};

};
#endif /* ICOMMANDSTRUCTPATTERN_H_ */
