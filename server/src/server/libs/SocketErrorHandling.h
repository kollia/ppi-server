/**
 *   This file 'SocketErrorHandling.h' is part of ppi-server.
 *   Created on: 09.10.2014
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

#ifndef SOCKETERRORHANDLING_H_
#define SOCKETERRORHANDLING_H_

#include <string>

#include "../../util/debug.h"

#include "../../util/stream/BaseErrorHandling.h"

using namespace std;
using namespace util;
using namespace design_pattern_world::util_pattern;

class SocketErrorHandling : public BaseErrorHandling
{
public:
	using BaseErrorHandling::operator =;
	/**
	 * constructor for null initialize class
	 * or initialized with given short error string
	 *
	 * @param short_error short error string to predefine object
	 */
	SocketErrorHandling(const string& short_error= "")
	: BaseErrorHandling("SocketErrorHandling", short_error)
	{};
	/**
	 * constructor to initialize with foreign ErrorHandling class
	 *
	 * @param other other object of error handling class
	 */
	SocketErrorHandling(const string& ownClassName, IErrorHandlingPattern* other)
	: BaseErrorHandling("SocketErrorHandling", other)
	{};
	/**
	 * returning error description
	 * form current object
	 *
	 * @param error return description for this given error types
	 * @return error description
	 */
	OVERWRITE string getErrorDescriptionString(errorVals_t error) const;
	/**
	 * search inside an result array of strings
	 * for first error or warning
	 * and set into object
	 *
	 * @param result result of transaction strings
	 * @return whether found an error or warning,
	 *         or error/warning was set before.<br />
	 *         when an warning was set, an error overwrite this
	 */
	bool searchResultError(vector<string>& result);
	/**
	 * write error from getAddrInfo() routine routine into class
	 *
	 * @param classname name of class where error occurred
	 * @param error_string name of error string defined inside translation reading file
	 * @param error_nr error number from addAddrInfo routine
	 * @param errno_nr errno number needed when occurred
	 * @param decl declaration of strings inside error description, separated with an '@'
	 * @return whether error be set.<br />when error before exist no new error will be set
	 */
	OVERWRITE bool setAddrError(const string& classname, const string& error_string,
					int error_nr, int errno_nr, const string& decl= "");

protected:
	/**
	 * create new own object to read error/warning messages
	 */
	virtual IErrorHandlingPattern* createObj()
	{ return new SocketErrorHandling(); };
	/**
	 * define all internal error descriptions
	 */
	OVERWRITE void createMessages();
};

#endif /* SOCKETERRORHANDLING_H_ */
