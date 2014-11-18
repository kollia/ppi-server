/**
 *   This file 'ISocketErrorHandlingPattern.h' is part of ppi-server.
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

#ifndef ISOCKETERRORHANDLINGPATTERN_H_
#define ISOCKETERRORHANDLINGPATTERN_H_

#include "IErrorHandlingPattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{

		class ISocketErrorHandlingPattern : public IErrorHandlingPattern
		{
		public:
			/**
			 * write error from getAddrInfo() routine routine into class
			 *
			 * @param classname name of class where error occurred
			 * @param error_string name of error string which error occurred
			 * @param error_nr error number from addAddrInfo routine
			 * @param errno_nr errno number needed when occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			virtual bool setAddrError(const string& classname, const string& error_string,
							int error_nr, int errno_nr, const string& decl= "");
		};


		/**
		 * type definition
		 * of socket error handling object
		 * inside an shared pointer
		 */
		typedef boost::shared_ptr<ISocketErrorHandlingPattern> STEHObj;

	} /* namespace util_pattern */
} /* namespace design_pattern_world */
#endif /* ISOCKETERRORHANDLINGPATTERN_H_ */
