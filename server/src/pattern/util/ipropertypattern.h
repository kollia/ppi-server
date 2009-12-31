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
			 * parameter from line
			 */
			string parameter;
			/**
			 * value corresponding to parameter
			 */
			string value;
		};
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
		 */
		virtual void checkProperties(string* output= NULL, const bool head= true) const= 0;
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
