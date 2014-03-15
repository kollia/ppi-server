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
#ifndef IPROPERTYPATTERN_H_
#define IPROPERTYPATTERN_H_

#include <string>
#include <vector>

using namespace std;

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for properties
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IPropertyPattern
	{
	public:
		/**
		 * structure of parameter and value of one line
		 */
		struct param_t
		{
			/**
			 * whether the line can read correctly (true)
			 */
			bool correct;
			/**
			 * whether the line have an correct parameter with value
			 * and can read
			 */
			bool read;
			/**
			 * name of file
			 */
			string filename;
			/**
			 * line number in readed file
			 */
			unsigned long line;
			/**
			 * parameter from line
			 */
			string parameter;
			/**
			 * parameter length
			 * to beginning read of value
			 */
			string::size_type paramLen;
			/**
			 * value corresponding to parameter
			 */
			string value;
			/**
			 * whether the property was read inside of an comment
			 */
			string uncommented;
			/**
			 * whether line have also commented content to read again
			 */
			bool breadagain;
			/**
			 * whether to read value is localized and need more string lines to complete
			 */
			bool bcontinue;
		};
		/**
		 * set delimiter between properties and value.<br />
		 * The delimiter can be more than one char and delimits by one of them.
		 * If the delimiter string have an space, the delimiter can also be one or more spaces
		 * or an tabulator (\t). Default, when method not called is an equals sign ('=')
		 *
		 * @param delimiter set delimiter
		 */
		virtual void setDelimiter(const string& delimiter)= 0;
		/**
		 * set an begin- and end-delimiter for an property with specific name.<br />
		 * For example when you define <code>setDelimiter( "foo", "[". "]")</code>.
		 * You can write in the property file only <code>[true]</code> and the property
		 * foo has the value <code>"true"</code>.
		 *
		 * @param name specific name of property
		 * @param begindelimiter delimiter between property and value
		 * @param enddelimiter end-delimiter only if used
		 */
		virtual void setDelimiter(const string& name, const string& begindelimiter, const string& enddelimiter)= 0;
		/**
		 * set documentation string for hole lines.<br />
		 * default character is an hash ('#') if the method not called.
		 * When you need also an second character or string call this mehtod with an hash ('#')
		 * and in second time with the other wanted character or string
		 *
		 * @param doc documentation char
		 */
		virtual void setComment(const string& doc)= 0;
		/**
		 * neutralize an documented string or character.<br />
		 * As example when the documentation is an hash ('#') you can neutralize with this
		 * method the documentation when you set an hash and an exclamation mark ('#!')
		 *
		 * @param undoc neutralized string
		 */
		virtual void setUncomment(const string& undoc)= 0;
		/**
		 * return the uncommented string if the property is inside an comment
		 * otherwise returning an NULL string ('')
		 *
		 * @return uncommented string
		 */
		virtual string wasCommented(const string& property)= 0;
		/**
		 * allow values over more rows.<br />
		 * by define the localization, as example, with double quotes for begin and end,
		 * the value can reach over more rows. If begin is an null string ("")
		 * and the end maybe an back slash, when the end of an value have this character
		 * the value follow also the next row.<br />
		 * No Default be set
		 *
		 * @param begin beginning of localization
		 * @param end ending of localization in the same or any follow rows
		 * @param remove removing the localization on begin and end (default: true)
		 */
		virtual void valueLocalization(const string& begin, const string& end, const bool remove= true)= 0;
		/**
		 * set documentation for an range.<br />
		 * No default be set
		 *
		 * @param begin the begin of range
		 * @param end the end of range
		 */
		virtual void setComment(const string& begin, const string& end)= 0;
		/**
		 * read file from harddisk
		 *
		 * @param filename name of file
		 * @return whether file exist an is readable
		 */
		virtual bool readFile(const string& filename)= 0;
		/**
		 * read one line from and save paramer if exists
		 *
		 * @param character line to read
		 * @return wheter method can read an correct parameter with value
		 */
		virtual bool readLine(const string& line)= 0;
		/**
		 * default parameter values if they not be set in the file
		 *
		 * @param key name of parameter
		 * @param value value of parameter
		 * @param overwrite whether an set parameter can overwrite the default inside the next interlaced quantifier (default=true)
		 */
		virtual void setDefault(const string& key, const string& value, const bool overwrite= true)= 0;
		/**
		 * return the next property from <code>IPropertyPattern</code> object from begin to end
		 *
		 * @return property name
		 */
		virtual string nextProp() const= 0;
		/**
		 * reset the iterator from <code>IPropertyPattern</code> object to 0
		 * to get by next call from method <code>nextProp()</code> again the first property
		 */
		virtual void resetProp() const= 0;
		/**
		 * pull property from this class and write an error if not exist
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @return defined string for property
		 */
		virtual string needValue(const string property, vector<string>::size_type index= 0) const =0;
		/**
		 * pull first property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param warning flag to write warning
		 * @return defined string for property
		 */
		virtual string getValue(const string property, bool warning) const =0;
		/**
		 * pull property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @param warning flag to write warning. default: true
		 * @return defined string for property
		 */
		virtual string getValue(const string property, vector<string>::size_type index= 0, bool warning= true) const =0;
		/**
		 * return file name in witch property was found
		 *
		 * @param property from which property filename was needed
		 * @param index which number of same parameter. default: 0
		 * @return filename
		 */
		virtual string getPropertyFile(const string& property, vector<string>::size_type index= 0) const= 0;
		/**
		 * return number of line in which the property was written
		 *
		 * @param property from which property line number was needed
		 * @param index which number of same parameter. default: 0
		 * @return line number
		 */
		virtual unsigned long getPropertyLine(const string& property, vector<string>::size_type index= 0) const= 0;
		/**
		 * pull property from this class and write an error if not exist
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @return defined integer. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual int needInt(string &property, vector<string>::size_type index= 0) const =0;
		/**
		 * pull first property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param warning flag to write warning
		 * @return defined integer. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual int getInt(string &property, bool warning) const =0;
		/**
		 * pull property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @param warning flag to write warning. default: true
		 * @return defined integer. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual int getInt(string &property, vector<string>::size_type index= 0, bool warning= true) const =0;
		/**
		 * pull property from this class and write an error if not exist
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @return defined double. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual double needDouble(string &property, vector<string>::size_type index= 0) const =0;
		/**
		 * pull first property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param warning flag to write warning
		 * @return defined double. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual double getDouble(string &property, bool warning) const =0;
		/**
		 * pull property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @param warning flag to write warning. default: true
		 * @return defined double. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual double getDouble(string &property, vector<string>::size_type index= 0, bool warning= true) const =0;
		/**
		 * pull property from this class and write an error if not exist
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @return defined unsigned int. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned int needUInt(string &property, vector<string>::size_type index= 0) const =0;
		/**
		 * pull first property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param warning flag to write warning
		 * @return defined unsigned int. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned int getUInt(string &property, bool warning) const =0;
		/**
		 * pull property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @param warning flag to write warning. default: true
		 * @return defined unsigned int. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned int getUInt(string &property, vector<string>::size_type index= 0, bool warning= true) const =0;
		/**
		 * pull property from this class and write an error if not exist
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @return defined unsigned short. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned short needUShort(string &property, vector<string>::size_type index= 0) const =0;
		/**
		 * pull first property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param warning flag to write warning
		 * @return defined unsigned short. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned short getUShort(string &property, bool warning) const =0;
		/**
		 * pull property from this class and write an warning
		 * if not exist and flag 'warning' be set
		 *
		 * @param property value whitch needet
		 * @param index which number of same parameter. default: 0
		 * @param warning flag to write warning. default: true
		 * @return defined unsigned short. If occured an error, returnvalue is 0 and second parameter is '#ERROR'
		 */
		virtual unsigned short getUShort(string &property, vector<string>::size_type index= 0, bool warning= true) const =0;
		/**
		 * return how much values for the given propertys are set
		 *
		 * @param property name of property variable
		 */
		virtual vector<string>::size_type getPropertyCount(const string property) const =0;
		/**
		 * if this method be set, the parameter is not allowed.<br />
		 * When the parameter will be fetched in an base class,
		 * it get's the second parameter as default value
		 *
		 * @param property the parameter which is not allowed
		 * @param sDefault the value which gets the method how fetch this parameter
		 * @return true if the parameter was set
		 */
		virtual bool notAllowedParameter(const string property, const string& sDefault= "") =0;
		/**
		 * return an string of all properties
		 *
		 * @return string of properties
		 */
		virtual string str() const= 0;
		/**
		 * read properties from the created tag with method <code>str()</code>
		 *
		 * @param tag propertie tag
		 */
		virtual void tag(const string& tag)= 0;
		/**
		 * for streaming all pulled parameters.<br />
		 * (This method is <code>const</code>, because it make no changes in object
		 * other than fill all pulled parameters which are <code>mutable</code>)
		 *
		 * @param params pulled parameters in an string tag
		 */
		virtual void pulled(const string& params) const= 0;
		/**
		 * return all pulled parameters in an string tag
		 *
		 * @return pulled parameters
		 */
		virtual string pulled() const= 0;

		/**
		 * method write WARNINGS on command line if any action not nessered
		 * and if propertys be set, but not needet
		 *
		 * @param output method fill this string if set with WARNINGS of parameter and actions which are set but not allowed.<br />
		 * 				 Elsewhere if string not be set (NULL), WARNINGS will be writing on command line
		 * @param head whether should writing WARNING with message parameter defined in setMsgParameter()
		 * @return whether an error occurred, warnings can seen when ouptut isn't an null string ("")
		 */
		virtual bool checkProperties(string* output= NULL, const bool head= true) const= 0;
		/**
		 * show bei error or warning messages all defined parameter with values
		 *
		 * @param name parameter name
		 * @param as alias name to display in message
		 */
		virtual void setMsgParameter(const string& name, const string& as= "")= 0;
		/**
		 * returning an string with ### ERROR: or ### WARNING: for actual folder and subroutine
		 * to display in log-file or screen
		 *
		 * @param error if pointer to boolean be NULL,<br />
		 * 				in string display only for which folder and subroutine.<br />
		 * 				otherwise in string will be display error by true or warning by false
		 */
		virtual string getMsgHead(const bool *error= NULL) const= 0;
		/**
		 * returning an string with ### ERROR: or ### WARNING: for actual folder and subroutine
		 * to display in log-file or screen
		 *
		 * @param error displays error by true or warning by false
		 */
		virtual string getMsgHead(const bool error) const= 0;
		/**
		 * virtual destructor of pattern
		 */
		virtual ~IPropertyPattern() {};
	};
}  // namespace pattern-world
#endif /*IPROPERTYPATTERN_H_*/
