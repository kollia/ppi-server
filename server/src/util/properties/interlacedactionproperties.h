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

/*
 * interlacedactionproperties.h
 *
 *  Created on: 22.03.2009
 *      Author: Alexander Kolli
 */

#ifndef INTERLACEDACTIONPROPERTIES_H_
#define INTERLACEDACTIONPROPERTIES_H_

#include <iostream>

#include <string>

#include "interlacedproperties.h"
#include "actionproperties.h"

#include "../../pattern/util/iinterlacedactionpropertypattern.h"

namespace util {

	using namespace design_pattern_world;

	class InterlacedActionProperties :	virtual public InterlacedProperties,
										virtual public ActionProperties,
										virtual public IInterlacedActionPropertyPattern
	{
		public:
			/**
			 * constructor to initial member variables
			 *
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedActionProperties(const bool byCheck= false)
			:	Properties(byCheck),
				InterlacedProperties(byCheck),
				ActionProperties(byCheck)
			{ };
			/**
			 * constructor to initial member variables
			 *
			 * @param modifier name of modifier from the defined object
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedActionProperties(const string& modifier, const bool byCheck= false)
			:	Properties(byCheck),
				InterlacedProperties(modifier, byCheck),
				ActionProperties(byCheck)
			{ };
			/**
			 * constructor to initial member variables
			 *
			 * @param modifier name of modifier from the defined object
			 * @param value value from modifier of the defined object
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedActionProperties(const string& modifier, const string& value, const bool byCheck= false)
			:	Properties(byCheck),
				InterlacedProperties(modifier, value, byCheck),
				ActionProperties(byCheck)
			{ };
			/**
			 * constructor to initial member variables
			 *
			 * @param modifier name of modifier from the defined object
			 * @param value value from modifier of the defined object
			 * @param level current modifier count of deepness in objects beginning with 1
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedActionProperties(const string& modifier, const string& value, const unsigned short level, const bool byCheck= false)
			:	Properties(byCheck),
				InterlacedProperties(modifier, value, level, byCheck),
				ActionProperties(byCheck)
			{ };
			/**
			 * read one line from and save paramer if exists
			 *
			 * @param character line to read
			 * @return struct of param_t with single parameter and value
			 */
			virtual bool readLine(const string& line);
			/**
			 * method write WARNINGS on command line if any action not necessary
			 * and if properties be set, but not needed
			 *
			 * @param output method fill this string if set with WARNINGS of parameter and actions which are set but not allowed.<br />
			 * 				 Elsewhere if string not be set (NULL), WARNINGS will be writing on command line
			 * @param head whether should writing WARNING with message parameter defined in setMsgParameter() (default true)
			 */
			virtual void checkProperties(string* output= NULL, const bool head= true) const;
			/**
			 * destructor of object
			 */
			virtual ~InterlacedActionProperties();

		protected:
			/**
			 * create new object of interlaced properties
			 *
			 * @param modifier name of modifier from the defined object
			 * @param value value from modifier of the defined object
			 * @param level current modifier count of deepness in objects beginning with 1
			 * @return new object
			 */
			virtual IInterlacedPropertyPattern* newObject(const string modifier, const string value, const unsigned short level);
			/**
			 * read line and save into variables
			 *
			 * @param param parameter with value which should saved in object
			 * @return whether line was an correct parameter with value
			 */
			virtual bool readLine(const param_t& param);
	};

}

#endif /* INTERLACEDACTIONPROPERTIES_H_ */
