/**
 *   This file 'IThreadErrorHandlingPattern.h' is part of ppi-server.
 *   Created on: 08.11.2014
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

#ifndef ITHREADERRORHANDLINGPATTERN_H_
#define ITHREADERRORHANDLINGPATTERN_H_

#include "IErrorHandlingPattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{

		class IThreadErrorHandlingPattern : public IErrorHandlingPattern
		{
		public:
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
			virtual bool setPThreadError(const string& classname, const string& methodname,
							const string& error_string, int error_nr, const string& decl= "")= 0;
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
			virtual bool setPThreadWarning(const string& classname, const string& method,
							const string& warn_string, int error_nr, const string& decl= "")= 0;
		};


		/**
		 * type definition
		 * of thread error handling object
		 * inside an shared pointer
		 */
		typedef boost::shared_ptr<IThreadErrorHandlingPattern> TEHObj;

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* ITHREADERRORHANDLINGPATTERN_H_ */
