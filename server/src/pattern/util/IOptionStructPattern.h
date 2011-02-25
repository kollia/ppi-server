/**
 *   This file 'IOptionStructPattern.h' is part of ppi-server.
 *   Created on: 13.02.2011
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


#ifndef IOPTIONSTRUCTPATTERN_H_
#define IOPTIONSTRUCTPATTERN_H_

#include <string>

using namespace std;

class IOptionStructPattern
{
public:
	/**
	 * question for main option (as first) be called
	 *
	 * @param option name of option (no short defined option)
	 * @return whether option be called
	 */
	virtual bool hasOption(const string& option) const= 0;
	/**
	 * return content from option in actual command
	 *
	 * @param option name of option
	 * @return content of option
	 */
	virtual string getOptionContent(const string& option) const= 0;
	/**
	 * return content from option in actual command
	 * casted as integer number
	 *
	 * @param option name of option (is '##ERROR' if content is no integer or '##NULL' when not set)
	 * @return content of option
	 */
	virtual int getOptionIntContent(string& option) const= 0;
	/**
	 * return content from option in actual command
	 * cated as floating number
	 *
	 * @param option name of option (is '##ERROR' if content is no float or '##NULL' when not set)
	 * @return content of option
	 */
	virtual float getOptionFloatContent(string& option) const= 0;
	/**
	 * return how many options be set
	 *
	 * @return count of options
	 */
	virtual size_t optioncount() const= 0;
	/**
	 * dummy destructor
	 */
	virtual ~IOptionStructPattern() {};
};

#endif /* IOPTIONSTRUCTPATTERN_H_ */
