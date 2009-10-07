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
 * properties.cpp
 *
 *  Created on: 19.03.2009
 *      Author: Alexander Kolli
 */

#include <iostream>
#include <fstream>

#include <boost/algorithm/string/trim.hpp>

#include "properties.h"
#include "URL.h"

#include "../logger/lib/LogInterface.h"

namespace util {

	Properties::Properties(const bool byCheck/*= false*/)
	:	m_bByCheck(byCheck)
	{
	}

	bool Properties::readFile(const string& filename)
	{
		string path, newfile;
		param_t param;
		string line;
		ifstream file(filename.c_str());

		if(file.is_open())
		{
			while(getline(file, line))
			{
				//cout << line << endl;
				param= read(line);
				if(param.correct)
				{
					if(param.read)
					{
						if(param.parameter == "file")
						{
							if(path == "")
								path= URL::getPath(filename);
							newfile= URL::addPath(path, param.value);
							if(!readFile(newfile))
							{
								string msg;

								msg=  "### WARNING: cannot read new file: '";
								msg+= newfile + "'\n";
								msg+= "             in configuration file ";
								msg+= filename;
								cerr << msg << endl;
								LOG(LOG_WARNING, msg);
							}
						}else
							readLine(line);
					}
				}else
				{
					string msg("### WARNING: cannot read line: '");

					msg+= line + "'\n             in configuration file ";
					msg+= filename;
					cerr << msg << endl;
					LOG(LOG_WARNING, msg);
				}
			}
		}else
			return false;
		file.close();
		return true;
	}

	bool Properties::readLine(const string& line)
	{
		param_t value;

		value= read(line);
		return saveLine(value);
	}

	bool Properties::saveLine(const Properties::param_t& parameter)
	{
		map<string, vector<string> >::iterator mContent;

		if(	!parameter.correct
			||
			!parameter.read		)
		{
			return false;
		}
		m_mvPropertyMap[parameter.parameter].push_back(parameter.value);
		return true;
		/*mContent= m_mvPropertyMap.find(type);
		if(mContent == m_mvPropertyMap.end())
		{
			vector<string> vContent;

			vContent.push_back(value);
			m_oPropertyMap[type]= vContent;
		}else
			m_oPropertyMap[type].push_back(value);
		return true;*/
	}

	Properties::param_t Properties::read(const string& line) const
	{
		param_t tRv;
		string::size_type len= line.length();
		string read(line);
		string::size_type pos;

		tRv.correct= false;
		tRv.read= false;
		pos= read.find("#");
		if(pos < len)
			read= read.substr(0, pos);
		boost::trim(read);
		if(read == "")
		{
			tRv.correct= true;
			return tRv;
		}
		pos= read.find("=");
		if(pos > len)
			return tRv;
		tRv.correct= true;
		tRv.read= true;
		tRv.parameter= read.substr(0, pos);
		boost::trim(tRv.parameter);
		tRv.value= read.substr(pos+1);
		boost::trim(tRv.value);
		return tRv;
	}

	void Properties::setDefault(const string& key, const string& value, const bool overwrite/*=true*/)
	{
		param_t param;

		param.correct= overwrite;
		param.parameter= key;
		param.value= value;
		m_mDefault[key]= param;
	}

	string Properties::needValue(const string property, vector<string>::size_type index/*= 0*/) const
	{
		bool byCheck= false;
		map<string, vector<string> >::const_iterator mContent;
		map<string, vector<vector<string>::size_type> >::const_iterator mPContent;
		string value("");

		mContent= m_mvPropertyMap.find(property);
		if(mContent != m_mvPropertyMap.end())
			value= mContent->second[index];

		if(value == "")
		{
			string msg;

			msg= getMsgHead(/*error*/true);
			msg+= "property ";
			msg+= property + " does not exist";
			if(byCheck)
				m_mFetchErrors[property]= true;
			else
				cerr << msg << endl;
			LOG(LOG_ALERT, msg);
		}
		mPContent= m_oPulled.find(property);
		if(mPContent == m_oPulled.end())
		{
			vector<vector<string>::size_type> vContent;

			vContent.push_back(index);
			m_oPulled[property]= vContent;
		}else
			m_oPulled[property].push_back(index);
		return value;
	}

	string Properties::getValue(const string property, bool warning) const
	{
		return getValue(property, 0, warning);
	}

	vector<string>::size_type Properties::getPropertyCount(string property) const
	{
		map<string, vector<string> >::const_iterator mContent;

		mContent= m_mvPropertyMap.find(property);
		if(mContent != m_mvPropertyMap.end())
			return mContent->second.size();
		return 0;
	}

	string Properties::getValue(const string property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		map<string, string>::const_iterator mNotAllowed;
		map<string, param_t>::const_iterator itDefault;
		map<string, vector<string> >::const_iterator mContent;
		map<string, vector<vector<string>::size_type> >::const_iterator mPContent;
		string value("");

		mNotAllowed= m_oNotAllowedParams.find(property);
		if(mNotAllowed != m_oNotAllowedParams.end())
			return mNotAllowed->second;
		mContent= m_mvPropertyMap.find(property);
		if(mContent == m_mvPropertyMap.end())
		{
			itDefault= m_mDefault.find(property);
			if(itDefault != m_mDefault.end())
				value= itDefault->second.value;
		}else
			value= mContent->second[index];

		if(	warning
			&&
			value == ""	)
		{
			string msg;

			msg= getMsgHead(/*error*/false);
			msg+= "property '";
			msg+= property + "' does not exist";
			if(m_bByCheck)
			{
				map<string, bool>::const_iterator found;

				found= m_mFetchErrors.find(property);
				if(found == m_mFetchErrors.end()) // write only an error into m_mFetchErrors with false if not exist,
					m_mFetchErrors[property]= false; // because maybe the error was written with true
			}else
				cerr << msg << endl;
			LOG(LOG_WARNING, msg);
		}

		mPContent= m_oPulled.find(property);
		if(mPContent == m_oPulled.end())
		{
			vector<vector<string>::size_type> vContent;

			vContent.push_back(index);
			m_oPulled[property]= vContent;
		}else
			m_oPulled[property].push_back(index);
		return value;
	}

	int Properties::needInt(string &property, vector<string>::size_type index/*= 0*/) const
	{
		string value;

		value= needValue(property, index);
		if(value == "")
		{
			property= "#ERROR";
			return 0;
		}
		return atoi(&value[0]);
	}

	int Properties::getInt(string &property, bool warning) const
	{
		return getInt(property, 0, warning);
	}

	int Properties::getInt(string &property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		string value;

		value= getValue(property, index, warning);
		if(value == "")
		{
			property= "#ERROR";
			return 0;
		}
		return atoi(&value[0]);
	}

	double Properties::needDouble(string &property, vector<string>::size_type index/*= 0*/) const
	{
		string value;
		string::size_type nLen;
		char* endptr;

		value= needValue(property, index);
		if(value == "")
		{
			property= "#ERROR";
			return 0;
		}
		nLen= value.length();
		endptr= &value[nLen-1];
		return strtod(&value[0], &endptr);
	}

	double Properties::getDouble(string &property, bool warning) const
	{
		return getDouble(property, 0, warning);
	}

	double Properties::getDouble(string &property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		string value;
		string::size_type nLen;
		char* endptr;

		value= getValue(property, index, warning);
		if(value == "")
		{
			property= "#ERROR";
			return 0;
		}
		nLen= value.length();
		endptr= &value[nLen-1];
		return strtod(&value[0], &endptr);
	}

	unsigned short Properties::needUShort(string &property, vector<string>::size_type index/*= 0*/) const
	{
		return (unsigned short)needInt(property, index);
	}

	unsigned short Properties::getUShort(string &property, bool warning) const
	{
		return (unsigned short)getInt(property, warning);
	}

	unsigned short Properties::getUShort(string &property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		return (unsigned short)getInt(property, index, warning);
	}

	unsigned int Properties::needUInt(string &property, vector<string>::size_type index/*= 0*/) const
	{
		return (unsigned int)needInt(property, index);
	}

	unsigned int Properties::getUInt(string &property, bool warning) const
	{
		return (unsigned int)getInt(property, warning);
	}

	unsigned int Properties::getUInt(string &property, vector<string>::size_type index/*= 0*/, bool warning/*= true*/) const
	{
		return (unsigned int)getInt(property, index, warning);
	}

	bool Properties::notAllowedParameter(const string property, const string& sDefault)
	{
		string value;

		m_oNotAllowedParams[property]= sDefault;
		value= getValue(property, /*warning*/false);
		if(value != "")
			return true;
		return false;
	}

	void Properties::checkProperties(string* output/*= NULL*/, const bool head/*= true*/) const
	{
		typedef map<string, bool>::const_iterator errorit;
		typedef map<string, vector<string> >::const_iterator miter;
		typedef map<string, vector<vector<string>::size_type> >::iterator mpiter;

		string msg, msg1, msg2;

		// write warning for all properties witch are set but not allowed
		for(miter c= m_mvPropertyMap.begin(); c != m_mvPropertyMap.end(); ++c)
		{
			string notex;
			vector<string>::size_type count= 0;
			vector<string>::size_type max= c->second.size();
			vector<vector<string>::size_type> vPContent;
			vector<string>::size_type PSize= vPContent.size();
			mpiter pulledIter=  m_oPulled.find(c->first);

			//cout << "for property " << c->first << endl;
			if(pulledIter != m_oPulled.end())
			{
				vPContent= m_oPulled.find(c->first)->second;
				while(count < max)
				{
					vector<string>::size_type vc= vPContent[count];
					if(vc > (PSize+1))
					{
						char val[20];

						sprintf(val, "%d., ", (int)(count+1));
						notex+= val;
					}
					++count;
				}
				if(notex != "")
				{
					msg1+= "\n                               ";
					if(count > 1)
					{
						notex= notex.substr(0, notex.size()-2);
						msg1+= notex + " value of " + c->first;
					}else
						msg1+= c->first;

				}
			}else
			{
				msg1+= "\n                               ";
				msg1+= c->first;
			}
		}
		if(msg1 != "")
		{
			if(head)
				msg= getMsgHead(false);
			msg+= "follow properties are set, but not allowed:" + msg1;
		}
		msg1= "";

		// write warning for all properties witch are fetched but not exists
		for(errorit o= m_mFetchErrors.begin(); o != m_mFetchErrors.end(); ++o)
		{
			if(!o->second)
			{
				msg1+= "                               ";
				msg1+= o->first + "\n";
			}
		}
		if(msg1 != "")
		{
			msg+= "\n             follow properties are fetched, but not exists:\n" + msg1;
			msg= msg.substr(0, msg.length()-1);
		}
		if(msg != "")
		{
			if(output)
				*output= msg;
			else
				cout << msg << endl;
			LOG(LOG_WARNING, msg);
		}
	}

	void Properties::setMsgParameter(const string& name, const string& as/*= ""*/)
	{
		string value(as);

		if(as == "")
			value= name;
		m_mErrorParams[name]= value;
	}

	string Properties::getMsgHead(const bool error) const
	{
		return getMsgHead(&error);
	}

	string Properties::getMsgHead(const bool *error/*= NULL*/) const
	{
		string msg, msg1, value;
		string::size_type n;
		map<string, string>::const_iterator o, l;

		if(error)
		{
			if(*error)
				msg= "### ERROR: ";
			else
				msg= "### WARNING: ";
		}
		if((n= m_mErrorParams.size()))
		{
			msg1= "";
			for(o= m_mErrorParams.begin(); o != m_mErrorParams.end(); ++o)
			{
				value= getValue(o->first, false);
				if(value != "")
				{
					l= o;
					if(	n != 1
						&&
						++l == m_mErrorParams.end()	)
					{
						msg1= msg1.substr(0, msg1.size()-1);
						msg1 += " and";
					}
					msg1+= " ";
					msg1+= o->second;
					msg1+= " '";
					msg1+= value;
					msg1+= "',";
				}
			}
			if(msg1 != "")
			{
				msg+= "in";
				msg+= msg1;
				msg= msg.substr(0, msg.size()-1);
				msg+= "\n";
				if(error)
				{
					msg+= "           ";
					if(!*error)
						msg+= "  ";
				}
			}
		}
		return msg;
	}

	Properties::~Properties()
	{
		// TODO Auto-generated destructor stub
	}

}
