/**
 *   This file 'ThreadErrorHandling.h' is part of ppi-server.
 *   Created on: 19.10.2014
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

#ifndef THREADERRORHANDLING_H_
#define THREADERRORHANDLING_H_

#include <string>

#include "../debug.h"

#include "../stream/BaseErrorHandling.h"

namespace util
{
	namespace thread
	{
		using namespace std;

		class ThreadErrorHandling : public BaseErrorHandling
		{
		public:
			using BaseErrorHandling::operator =;
			/**
			 * constructor for null initialize class
			 * or initialized with given short error string
			 *
			 * @param short_error short error string to predefine object
			 */
			ThreadErrorHandling(const string& short_error= "")
			: BaseErrorHandling("TreadErrorHandling", short_error)
			{};
			/**
			 * constructor to initialize with foreign ErrorHandling class
			 *
			 * @param other other object of error handling class
			 */
			ThreadErrorHandling(const string& ownClassName, IErrorHandlingPattern* other)
			: BaseErrorHandling("TreadErrorHandling", other)
			{};
			/**
			 * define all internal error descriptions
			 */
			OVERWRITE void read();
			/**
			 * returning error description
			 * form current object
			 *
			 * @param error return description for this given error types
			 * @return error description
			 */
			OVERWRITE string getErrorDescriptionString(errorVals_t error) const;
			/**
			 * write error from any pthread routine into class.<br />
			 * this method need for second parameter the original
			 * pthread_(xxx) method name where the error occurred.
			 *
			 * @param classname name of class where error occurred
			 * @param methodname name of pthread routine
			 * @param error_string name of error string defined inside translation reading file
			 * @param error_nr error number getting from pthread routine
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			OVERWRITE bool setPThreadError(const string& classname, const string& methodname,
							const string& error_string, int error_nr, const string& decl= "");
			/**
			 * write error from any pthread routine into class as warning.<br />
			 * this method need for second parameter the original
			 * pthread_(xxx) method name where the error occurred.
			 *
			 * @param classname name of class where error occurred
			 * @param methodbane name of pthread routine
			 * @param warn_string name of error string defined inside translation reading file
			 * @param error_nr error number getting from pthread routine
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether warning be set.<br />when error or warning before exist no new warning will be set
			 */
			OVERWRITE bool setPThreadWarning(const string& classname, const string& method,
							const string& warn_string, int error_nr, const string& decl= "");
		};

	} /* namespace thread */
} /* namespace util */
#endif /* THREADERRORHANDLING_H_ */
