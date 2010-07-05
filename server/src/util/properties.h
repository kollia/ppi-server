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
 * properties.h
 *
 *  Created on: 19.03.2009
 *      Author: Alexander Kolli
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <string>
#include <vector>
#include <map>

#include "../pattern/util/ipropertypattern.h"

	using namespace std;

namespace util {


	class Properties: virtual public design_pattern_world::IPropertyPattern
	{
		public:
#if 0
			/**
			 * structure of parameter and value of one line
			 */
			struct param_t
			{
				/**
				 * whether the line have an correct parameter with value
				 */
				bool correct;
				/**
				 * parameter from line
				 */
				string parameter;
				/**
				 * value corresponding to parameter
				 */
				string value;
			};
#endif
			/**
			 * initialization of properties
			 *
			 * @param byCheck 	write error or warning by <code>checkProperties()</code>.<br />
			 * 					define whether by fetching an parameter
			 * 					the error or warning message is writing immediately on command line (false)
			 * 					or elsewhere by invoke <code>checkProperties()</code> (true) (default= false).
			 */
			Properties(const bool byCheck= false);
			/**
			 * set delimiter between properties and value.<br />
			 * The delimiter can be more than one char and delimits by one of them.
			 * If the delimiter string have an space, the delimiter can also be one or more spaces
			 * or an tabulator (\t). Default, when method not called is an equals sign ('=')
			 *
			 * @param delimiter set delimiter
			 */
			virtual void setDelimiter(const string& delimiter);
			/**
			 * set documentation string for hole lines.<br />
			 * default character is an hash ('#') if the method not called.
			 * When you need also an second character or string call this mehtod with an hash ('#')
			 * and in second time with the other wanted character or string
			 *
			 * @param doc documentation char
			 */
			virtual void setComment(const string& doc);
			/**
			 * set documentation for an range.<br />
			 * No default be set
			 *
			 * @param begin the begin of range
			 * @param end the end of range
			 */
			virtual void setComment(const string& begin, const string& end);
			/**
			 * neutralize an documented string or character.<br />
			 * As example when the documentation is an hash ('#') you can neutralize with this
			 * method the documentation when you set an hash and an exclamation mark ('#!')
			 *
			 * @param undoc neutralized string
			 */
			virtual void setUncomment(const string& undoc);
			/**
			 * return the uncommented string if the property is inside an comment
			 * otherwise returning an NULL string ('')
			 *
			 * @return uncommented string
			 */
			virtual string wasCommented(const string& property);
			/**
			 * read file from hard disk
			 *
			 * @param filename name of file
			 * @return whether file exist an is readable
			 */
			bool readFile(const string& filename);
			/**
			 * read line and save into variables
			 *
			 * @param character line
			 * @return whether line was an correct parameter with value
			 */
			virtual bool readLine(const string& line);
			/**
			 * read only line and split integer structure param_t
			 *
			 * @param line character line which should be read
			 * @return split line
			 */
			param_t read(const string& line) const;
			/**
			 * Return true if the property container contains no elements
			 *
			 * @return true if the property container contains no elements
			 */
			bool isEmpty()
			{
				if(!m_mvPropertyMap.size())
					return true;
				return false;
			};
			/**
			 * default parameter values if they not be set in the file
			 *
			 * @param key name of parameter
			 * @param value value of parameter
			 * @param overwrite whether an set parameter can overwrite the default inside the next interlaced quantifier (default=true)
			 */
			virtual void setDefault(const string& key, const string& value, const bool overwrite= true);
			/**
			 * return the next property from <code>Properties</code> object from begin to end
			 *
			 * @return property name
			 */
			virtual string nextProp();
			/**
			 * reset the iterator from <code>Properties</code> object to 0
			 * to get by next call from method <code>nextProp()</code> again the first property
			 */
			virtual void resetProp()
			{ m_nPropCount= 0; };
			/**
			 * pull property from this class and write an error if not exist
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @return defined string for property
			 */
			virtual string needValue(const string property, vector<string>::size_type index= 0) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined string for property
			 */
			virtual string getValue(const string property, bool warning) const;
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
			 * pull property from this class and write an error if not exist
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @return defined integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual int needInt(string &property, vector<string>::size_type index= 0) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual int getInt(string &property, bool warning) const;
			/**
			 * pull property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @param warning flag to write warning. default: true
			 * @return defined integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual int getInt(string &property, vector<string>::size_type index= 0, bool warning= true) const;
			/**
			 * pull property from this class and write an error if not exist
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @return defined double. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual double needDouble(string &property, vector<string>::size_type index= 0) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined double. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual double getDouble(string &property, bool warning) const;
			/**
			 * pull property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @param warning flag to write warning. default: true
			 * @return defined double. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual double getDouble(string &property, vector<string>::size_type index= 0, bool warning= true) const;
			/**
			 * pull property from this class and write an error if not exist
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @return defined unsigned integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned int needUInt(string &property, vector<string>::size_type index= 0) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined unsigned integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned int getUInt(string &property, bool warning) const;
			/**
			 * pull property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @param warning flag to write warning. default: true
			 * @return defined unsigned integer. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned int getUInt(string &property, vector<string>::size_type index= 0, bool warning= true) const;
			/**
			 * pull property from this class and write an error if not exist
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @return defined unsigned short. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned short needUShort(string &property, vector<string>::size_type index= 0) const;
			/**
			 * pull first property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param warning flag to write warning
			 * @return defined unsigned short. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned short getUShort(string &property, bool warning) const;
			/**
			 * pull property from this class and write an warning
			 * if not exist and flag 'warning' be set
			 *
			 * @param property value which needed
			 * @param index which number of same parameter. default: 0
			 * @param warning flag to write warning. default: true
			 * @return defined unsigned short. If occurred an error, return value is 0 and second parameter is '#ERROR'
			 */
			virtual unsigned short getUShort(string &property, vector<string>::size_type index= 0, bool warning= true) const;
			/**
			 * return how much values for the given properties are set
			 *
			 * @param property name of property variable
			 */
			vector<string>::size_type getPropertyCount(const string property) const;
			/**
			 * do not allow parameter in list.<br />
			 * This method can be used, if in an base class is defined an parameter
			 * but in the actually class this parameter should be not allowed
			 *
			 * @param property the parameter which is not allowed
			 * @param sDefault the value which gets the method how fetch this parameter
			 * @return true if the parameter was set
			 */
			virtual bool notAllowedParameter(const string property, const string& sDefault= "");
			/**
			 * return an string of all properties
			 *
			 * @return string of properties
			 */
			virtual string str() const;
			/**
			 * read properties from the created tag with method <code>str()</code>
			 *
			 * @param tag propertie tag
			 */
			virtual void tag(const string& tag);
			/**
			 * for streaming insert all pulled parameters.<br />
			 * (This method is <code>const</code>, because it make no changes in object
			 * other than fill all pulled parameters which are <code>mutable</code>)
			 *
			 * @param params pulled parameters in an string tag
			 */
			virtual void pulled(const string& params) const;
			/**
			 * return all pulled parameters in an string tag
			 *
			 * @return pulled parameters
			 */
			virtual string pulled() const;
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
			 * show bei error or warning messages all defined parameter with values
			 *
			 * @param name parameter name
			 * @param as alias name to display in message
			 */
			virtual void setMsgParameter(const string& name, const string& as= "");
			/**
			 * returning an string with ### ERROR: or ### WARNING: for actual folder and subroutine
			 * to display in log-file or screen
			 *
			 * @param error if pointer to boolean be NULL,<br />
			 * 				in string display only for which folder and subroutine.<br />
			 * 				otherwise in string will be display error by true or warning by false
			 */
			virtual string getMsgHead(const bool *error) const;
			/**
			 * returning an string with ### ERROR: or ### WARNING: for actual folder and subroutine
			 * to display in log-file or screen
			 *
			 * @param error displays error by true or warning by false
			 */
			virtual string getMsgHead(const bool error= NULL) const;
			/**
			 * destructor of properties
			 */
			virtual ~Properties();

		protected:
			/**
			 * define whether by fetching an parameter
			 * the error or warning message is writing immediately on command line (false)
			 * or elsewhere by invoke <code>checkProperties()</code> (true).
			 */
			bool m_bByCheck;
			/**
			 * parameter which should shown for error or warning messages
			 * with an alias name for display as value
			 */
			map<string, string> m_mErrorParams;
			/**
			 * read line and save into variables
			 *
			 * @param parameter parameter with value which should saved in object
			 * @return whether line was an correct parameter with value
			 */
			virtual bool readLine(const param_t& parameter);
			/**
			 * save param_t integer member variables
			 *
			 * @param parameter parameter with value which should saved in object
			 * @return whether line was an correct parameter (param_t.correct was true)
			 */
			bool saveLine(const param_t& parameter);

		private:
			/**
			 * counter for property iterator
			 */
			unsigned int m_nPropCount;
			/**
			 * hole propertys for an subroutine
			 */
			map<string, vector<string> > m_mvPropertyMap;
			/**
			 * hole properties in order for method <code>nextProp()</code>
			 */
			vector<string> m_vPropOrder;
			/**
			 * inherits not allowed parameter.<br />
			 * if later in any base class the parameter will be fetched
			 * it gets the second value from map as default
			 */
			map<string, string> m_oNotAllowedParams;
			/**
			 * all default parameter if value not exist in the file
			 */
			map<string, param_t> m_mDefault;
			/**
			 * all fetched parameters
			 */
			mutable map<string, vector<vector<string>::size_type> > m_oPulled;
			/**
			 * all fetched parameter witch are not exist as key
			 * and whether write an error (true) or warning (false) as value
			 */
			mutable map<string, bool> m_mFetchErrors;
			/**
			 * one or more character for delimiter
			 */
			string m_sDelimiter;
			/**
			 * all documentation strings for lines
			 */
			vector<string> m_vsComms;
			/**
			 * all neutralize documentation strings for lines
			 */
			vector<string> m_vsUnComms;
			/**
			 * all uncommented properties
			 */
			map<string, vector<string> > m_mvUncomProp;
			/**
			 * all documentation strings for range with end of documentation
			 */
			map<string, vector<string> > m_mssDocs;
			/**
			 * when be found any begin of documentation range,
			 * there is saved the iterator for defined begin and end of range
			 */
			mutable map<string, vector<string> >::const_iterator m_itmvActRangeDoc;
	};

}

#endif /* PROPERTIES_H_ */
