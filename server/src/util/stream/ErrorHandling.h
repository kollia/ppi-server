/**
 *   This file 'ErrorHandling.h' is part of ppi-server.
 *   Created on: 14.10.2014
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

#ifndef ERRORHANDLING_H_
#define ERRORHANDLING_H_

#include <string>

#include "../debug.h"

#include "BaseErrorHandling.h"

namespace util
{
	using namespace std;

	class ErrorHandling : public BaseErrorHandling
	{
	public:
		using BaseErrorHandling::operator =;
		/**
		 * constructor for null initialize class
		 * or initialized with given short error string
		 *
		 * @param short_error short error string to predefine object
		 */
		ErrorHandling(const string& short_error= "")
		: BaseErrorHandling("ErrorHandling", short_error)
		{};
		/**
		 * constructor to initialize with foreign ErrorHandling class
		 *
		 * @param other other object of error handling class
		 */
		ErrorHandling(const string& ownClassName, IErrorHandlingPattern* other)
		: BaseErrorHandling("ErrorHandling", other)
		{};

	protected:
		/**
		 * create new own object to read error/warning messages
		 */
		virtual IErrorHandlingPattern* createObj()
		{ return new ErrorHandling(); };
		/**
		 * define all internal error descriptions
		 */
		OVERWRITE void createMessages();
	};

} /* namespace util */
#endif /* ERRORHANDLING_H_ */
