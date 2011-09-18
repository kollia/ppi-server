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
 * interlacedproperties.h
 *
 *  Created on: 20.03.2009
 *      Author: Alexander Kolli
 */

#ifndef INTERLACEDPROPERTIES_H_
#define INTERLACEDPROPERTIES_H_

#include <iostream>

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "properties.h"

#include "../../pattern/util/iinterlacedpropertypattern.h"

namespace util {

	using namespace std;
	using namespace design_pattern_world;

	class InterlacedProperties: virtual public Properties,
								virtual public IInterlacedPropertyPattern
	{
		public:
			/**
			 * position of modifier
			 */
			struct pos_t
			{
				/**
				 * position
				 */
				unsigned short pos;
				/**
				 * current value of modifier
				 */
				string currentval;
				/**
				 * operator to compare in an find function
				 */
				int operator==(const string value) const
				{// sort only time
					if(this->currentval != value)
						return 0;
					return 1;
				};
				/**
				 * operator to know whether value is lower than the other
				 */
				int operator<(const pos_t &other) const
				{// sort only time
					if(this->currentval < other.currentval)
						return 1;
					return 0;
				};
			};
			/**
			 * constructor to initial member variables
			 *
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedProperties(const bool byCheck= false)
			:	Properties(byCheck)
			{ init("", "", 0); };
			/**
			 * constructor to initial member variables
			 *
			 * @param modifier name of modifier from the defined object
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			InterlacedProperties(const string& modifier, const bool byCheck= false)
			:	Properties(byCheck)
			{ init(modifier, "", 0); };
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
			InterlacedProperties(const string& modifier, const string& value, const bool byCheck= false)
			:	Properties(byCheck)
			{ init(modifier, value, 0); };
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
			InterlacedProperties(const string& modifier, const string& value, const unsigned short level, const bool byCheck= false)
			:	Properties(byCheck)
			{ init(modifier, value, level); };
			/**
			 * copy constructor for object
			 *
			 * @param x object to copy
			 */
			InterlacedProperties(const InterlacedProperties& x)
			:	Properties(x)
			{ copy(x, /*constructor*/true); };
			/**
			 * assignment operator to copy
			 *
			 * @param x object to copy
			 */
			virtual InterlacedProperties& operator=(const InterlacedProperties& x)
			{ return copy(x, /*constructor*/false); };
			/**
			 * if method set to true, class allow also no regular order of modifier.<br />
			 * When method not be used, default usable is regular (like same as set method to <code>false</code>).
			 *
			 * @param reg whether need modifier in defined order by set false or also later modifier by true
			 */
			virtual void allowLaterModifier(const bool reg);
			/**
			 * read one line from and save paramer if exists
			 *
			 * @param character line to read
			 * @return struct of param_t with single parameter and value
			 */
			virtual bool readLine(const string& line);
			/**
			 * to alter quantifier for interlace properties
			 *
			 * @param spez name of quantifier
			 * @param pos position of modifier (default is 0 -> create position as access of method)
			 */
			virtual void modifier(const string& spez, const unsigned short pos= 0)
			{ modifier(spez, "", pos); };
			/**
			 * to alter quantifier for interlace properties
			 *
			 * @param spez name of quantifier
			 * @param value if modifier should have an specific value to be a modifier
			 * @param pos position of modifier (default is 0 -> create position as access of method)
			 */
			virtual void modifier(const string& spez, const string& value, const unsigned short pos= 0);
			/**
			 * whether given modifier is defined with method modifier()
			 *
			 * @param mod name of modifier
			 * @return whether modifier exist
			 */
			virtual bool isModifier(const string& mod) const;
			/**
			 * return true if class found the modifier
			 *
			 * @param spez name of searched modifier
			 * @return whether class found an new subroutine or folder
			 */
			virtual bool foundModifier(const string& spez) const;
			/**
			 * method write WARNINGS on command line if any action not necessary
			 * and if properties be set, but not needed
			 *
			 * @param output method fill this string if set with WARNINGS of parameter and actions which are set but not allowed.<br />
			 * 				 Elsewhere if string not be set (NULL), WARNINGS will be writing on command line
			 * @param head whether should writing WARNING with message parameter defined in setMsgParameter() (default true)
			 * @return whether an error occurred, warnings can seen when ouptut isn't an null string ("")
			 */
			virtual bool checkProperties(string* output= NULL, const bool head= true) const;
			/**
			 * return all modified sections
			 *
			 * @param modifier when this parameter set, only sections of this modifier will be returned
			 * @return array of sections in this object
			 */
			virtual const vector<IInterlacedPropertyPattern*> getSections(const string& modifier= "") const;
			/**
			 * return specific section of modifier and value
			 *
			 * @param modifier name of specific modifier
			 * @param value specific value of modifier
			 * @param index for more than one modifier with the same value, you can set also the index
			 * @return defined section of modifier and value
			 */
			virtual const IInterlacedPropertyPattern* getSection(const string& modifier, const string& value,
																vector<IInterlacedPropertyPattern*>::size_type index= 0) const;
			/**
			 * calculate count of all sections
			 *
			 * @param modifier name of specific modifier
			 * @param value specific value of modifier
			 * @return index of sections
			 */
			virtual vector<IInterlacedPropertyPattern*>::size_type getSectionCount(const string& modifier= "", const string& value= "") const;
			/**
			 * return name of current section
			 *
			 * @return name of section
			 */
			virtual string getSectionModifier() const
							{ return m_sModifier; };
			/**
			 * return value of current section if exist.<br />
			 * elsewhere return an null string ("")
			 *
			 * @return value of section
			 */
			virtual string getSectionValue() const
							{ return m_sValue; };
			/**
			 * return how much values for the given properties are set
			 *
			 * @param property name of property variable
			 */
			vector<string>::size_type getPropertyCount(const string property) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined string for property
			 */
			virtual string getValue(const string property, bool warning) const
							{ return getValue(property, 0, warning); };
			/**
			 * pull property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @param warning flag to write warning. default: true
			 * @return defined string for property
			 */
			virtual string getValue(const string property, vector<string>::size_type index= 0, bool warning= true) const;
			/**
			 * destructor for object
			 */
			virtual ~InterlacedProperties();

		protected:
			/**
			 * all interlaced property classes witch in the specific level
			 */
			vector<IInterlacedPropertyPattern*> m_vSections;

			/**
			 * method to copy for constructor and operator
			 *
			 * @param x object to copy
			 * @param constructor whether copy is for constructor
			 */
			InterlacedProperties& copy(const InterlacedProperties& x, bool constructor);
			/**
			 * method check whether parameter is an interlaced modifier
			 * and create in this case an new interlaced property object
			 *
			 * @param param structure of parameter with value
			 * @return whether the parameter is an modifier
			 */
			virtual bool readLine(const param_t& param); // throw(runtime_error);
			/**
			 * create new object of interlaced properties
			 *
			 * @param modifier name of modifier from the defined object
			 * @param level current modifier count of deepness in objects beginning with 1
			 * @return new object
			 */
			virtual IInterlacedPropertyPattern* newObject(const string modifier, const unsigned short level);
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
			 * add all defined modifier in the new opject
			 */
			void addDefinitions(IInterlacedPropertyPattern* obj) const;
			/**
			 * check also all interlaced properties
			 * @see checkProperties
			 *
			 * @param output method fill this string if set with WARNINGS of parameter and actions which are set but not allowed.<br />
			 * 				 Elsewhere if string not be set (NULL), WARNINGS will be writing on command line
			 * @param head whether should writing WARNING with message parameter defined in setMsgParameter() (default true)
			 * @return whether an error occurred, warnings can seen when ouptut isn't an null string ("")
			 */
			virtual bool checkInterlaced(string* output= NULL, const bool head= true) const;

		private:
			/**
			 * the level from this property-class, position in m_vModifier
			 */
			unsigned short m_nLevel;
			/**
			 * whether use only regular order of modifier
			 */
			bool m_bRegOrder;
			/**
			 * modifier of this object if set.<br />
			 * Elswhere this variable is ""
			 */
			string m_sModifier;
			/**
			 * value from modifier of this object
			 */
			string m_sValue;
			/**
			 * map of all modifier with actual values and positions
			 */
			map<string, vector<pos_t> > m_mvModifier;

			/**
			 * initial member variables
			 *
			 * @param modifier name of modifier from the defined object
			 * @param value value from modifier of the defined object
			 * @param level current modifier count of deepness in objects beginning with 1
			 */
			void init(const string& modifier, const string& value, const unsigned short level);
	};

}

#endif /* INTERLACEDPROPERTIES_H_ */
