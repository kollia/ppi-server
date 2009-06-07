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
 * iinterlacedpropertypattern.h
 *
 *  Created on: 19.03.2009
 *      Author: Alexander Kolli
 */

#ifndef IINTERLACEDPROPERTYPATTERN_H_
#define IINTERLACEDPROPERTYPATTERN_H_

#include <string>

#include "ipropertypattern.h"

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for properties
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IInterlacedPropertyPattern : virtual public IPropertyPattern
	{
	public:
		/**
		 * read one line from and save paramer if exists
		 *
		 * @param struct of param_t with single parameter and value
		 * @return wheter method can read parameter
		 */
		virtual bool readLine(const param_t& param)= 0;
		/**
		 * to alter quantifier for interlace properties
		 *
		 * @param spez name of quantifier
		 * @param value current value of modifier
			 * @param pos position of modifier (default is 0 -> create position as access of method)
		 */
		virtual void modifier(const string& spez, const string& value= "", const unsigned short pos= 0)= 0;
		/**
		 * if method set to true, class allow also no regular order of modifier.<br />
		 * When method not be used, default usable is regular (like same as set method to <code>false</code>).
		 *
		 * @param reg whether need modifier in defined order by set false or also later modifier by true
		 */
		virtual void allowLaterModifier(const bool reg)= 0;
		/**
		 * whether given modifier is defined with method modifier()
		 *
		 * @param mod name of modifier
		 * @return whether modifier exist
		 */
		virtual bool isModifier(const string& mod) const= 0;
		/**
		 * return true if class found the modifier
		 *
		 * @param spez name of searched modifier
		 * @return whether class found an new subroutine or folder
		 */
		virtual bool foundModifier(const string& spez) const= 0;
		/**
		 * return all modified sections
		 *
		 * @return array of sections in this object
		 */
		virtual vector<IInterlacedPropertyPattern*> getSections() const= 0;
		/**
		 * return name of current section
		 *
		 * @return name of section
		 */
		virtual string getSectionModifier() const= 0;
		/**
		 * return value of current section if exist.<br />
		 * elsewhere return an null string ("")
		 *
		 * @return value of section
		 */
		virtual string getSectionValue() const= 0;
		/**
		 * virtual destructor of pattern
		 */
		virtual ~IInterlacedPropertyPattern() {};
	};
}

#endif /* IINTERLACEDPROPERTYPATTERN_H_ */
