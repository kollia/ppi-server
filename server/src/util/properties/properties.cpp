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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "properties.h"

#include "../GlobalStaticMethods.h"

#include "../stream/IParameterStringStream.h"
#include "../stream/OParameterStringStream.h"

#include "../../pattern/util/LogHolderPattern.h"


using namespace boost;
using namespace boost::algorithm;

namespace util {

	Properties::Properties(const bool byCheck/*= false*/)
	:	m_bByCheck(byCheck),
	 	m_nPropCount(0),
		m_sDelimiter("=")
	{
		m_itmvActRangeDoc= m_mssDocs.end();
	}

	Properties& Properties::copy(const Properties& x)
	{
		m_bByCheck= x.m_bByCheck;
		m_mErrorParams= x.m_mErrorParams;
		m_nPropCount= x.m_nPropCount;
		m_mvPropertyMap= x.m_mvPropertyMap;
		m_vPropOrder= x.m_vPropOrder;
		m_oNotAllowedParams= x.m_oNotAllowedParams;
		m_mDefault= x.m_mDefault;
		m_bRemoveLocal= x.m_bRemoveLocal;
		m_sBeginLocal= x.m_sBeginLocal;
		m_sEndLocal= x.m_sEndLocal;
		m_oPulled= x.m_oPulled;
		m_mFetchErrors= x.m_mFetchErrors;
		m_sDelimiter= x.m_sDelimiter;
		m_vsComms= x.m_vsComms;
		m_vsUnComms= x.m_vsUnComms;
		m_mvUncomProp= x.m_mvUncomProp;
		m_mssDocs= x.m_mssDocs;
		m_itmvActRangeDoc= m_mssDocs.end();
		return *this;
	}

	void Properties::setDelimiter(const string& delimiter)
	{
		m_sDelimiter= delimiter;
	}

	void Properties::setComment(const string& doc)
	{
		m_vsComms.push_back(doc);
	}

	void Properties::setUncomment(const string& undoc)
	{
		m_vsUnComms.push_back(undoc);
	}

	void Properties::setComment(const string& begin, const string& end)
	{
		m_mssDocs[begin].push_back(end);
		m_itmvActRangeDoc= m_mssDocs.end();
	}

	string Properties::wasCommented(const string& property)
	{
		string sRv;
		vector<string>::iterator found;

		for(map<string, vector<string> >::iterator it= m_mvUncomProp.begin(); it != m_mvUncomProp.end(); ++it)
		{
			found= find(it->second.begin(), it->second.end(), property);
			if(found != it->second.end())
			{
				sRv= it->first;
				break;
			}
		}
		return sRv;
	}

	void Properties::valueLocalization(const string& begin, const string& end, const bool remove/*= true*/)
	{
		m_sBeginLocal= begin;
		m_sEndLocal= end;
		m_bRemoveLocal= remove;
	}

	bool Properties::readFile(const string& filename)
	{
		string path, newfile;
		param_t param;
		string line;
		ifstream file(filename.c_str());

		if(file.is_open())
		{
			param.breadagain= false;
			param.bcontinue= false;
			param.correct= false;
			param.parameter= "";
			param.read= false;
			param.uncommented= "";
			param.value= "";
			param.filename= filename;
			param.line= 1;
			while(getline(file, line))
			{
				//cout << line << endl;
				do{
					read(line, &param);
					if(param.bcontinue)
						continue;
					//cout << "   param: '" << param.parameter << "'" << endl;
					//cout << "   value: '" << param.value << "'" << endl;
					if(param.correct)
					{
						if(param.read)
						{
							if(param.parameter == "file")
							{
								bool fsep= false, ssep= false;

								if(path == "")
								{
									string::size_type  pos= filename.find_last_of("/");

									if(pos != string::npos)
										path= filename.substr(0, pos);
									else
										path= "./";
								}
								fsep= path.substr(path.length()-1, 1) == "/" ? true : false;
								ssep= param.value.substr(0, 1) == "/" ? true : false;
								newfile= path;
								if(!fsep && !ssep)
									newfile+= "/";
								else if(fsep && ssep)
									newfile= newfile.substr(0, newfile.length()-1);
								newfile+= param.value;
								if(!readFile(newfile))
								{
									string msg;

									msg=  "### WARNING: cannot read new file: '";
									msg+= newfile + "'\n";
									msg+= "             in configuration file ";
									msg+= filename;
									cerr << msg << endl;
									LOG(LOG_ERROR, msg);
								}
							}else
								readLine(param);
						}
					}else
					{
						string msg("### WARNING: cannot read line: '");

						msg+= line + "'\n             in configuration file ";
						msg+= filename;
						cerr << msg << endl;
						LOG(LOG_WARNING, msg);
					}
					param.bcontinue= false;
					param.correct= false;
					param.parameter= "";
					param.read= false;
					param.uncommented= "";
					param.value= "";

				}while(param.breadagain);
				++param.line;
			}
		}else
			return false;
		file.close();
		return true;
	}

	bool Properties::readLine(const string& line)
	{
		param_t param;

		param.bcontinue= false;
		param.correct= false;
		param.parameter= "";
		param.read= false;
		param.uncommented= "";
		param.value= "";
		read(line, &param);
		return saveLine(param);
	}

	bool Properties::readLine(const Properties::param_t& parameter)
	{
		return saveLine(parameter);
	}

	bool Properties::saveLine(const Properties::param_t& parameter)
	{
		ostringstream nsline;
		map<string, vector<string> >::iterator mContent;

		if(	!parameter.correct
			||
			!parameter.read		)
		{
			return false;
		}
		m_vPropOrder.push_back(parameter.parameter);
		m_mvPropertyMap[parameter.parameter].push_back(parameter.value);
		nsline << parameter.line << " " << parameter.filename;
		m_mvPropertyLines[parameter.parameter].push_back(nsline.str());
		if(parameter.uncommented != "")
			m_mvUncomProp[parameter.uncommented].push_back(parameter.parameter);
		return true;
	}

	void Properties::read(const string& line, Properties::param_t* param) const
	{
		param_t tRv(*param);
		string::size_type len= line.length();
		string read(line), docline, comment, commented;
		string::size_type bpos, epos, blen, elen;

		//cout << read << endl;
		bpos= len + 1;
		epos= len + 1;
		if(m_itmvActRangeDoc == m_mssDocs.end())
		{
			// check whether line has begin of documented range
			for(map<string, vector<string> >::const_iterator o= m_mssDocs.begin(); o != m_mssDocs.end(); ++o)
			{
				bpos= read.find(o->first);
				if(bpos < len)
				{
					m_itmvActRangeDoc= o;
					break;
				}
			}
		}
		if(m_itmvActRangeDoc != m_mssDocs.end())
		{
			// check whether line has end of documented range
			for(vector<string>::const_iterator it= m_itmvActRangeDoc->second.begin(); it != m_itmvActRangeDoc->second.end(); ++it)
			{
				epos= read.find(*it);
				if(epos < len)
				{
					elen= it->length();
					break;
				}
			}
		}
		if(epos < len)
		{
			if(bpos < len)
			{
				read= read.substr(0, bpos) + read.substr(epos + elen);
			}else
				read= read.substr(epos + elen);
			m_itmvActRangeDoc= m_mssDocs.end();

		}else if(bpos < len)
			read= read.substr(0, bpos);
		if(	bpos > len &&
			m_itmvActRangeDoc != m_mssDocs.end() &&
			epos > len							)
		{
			tRv.correct= true;
			*param= tRv;
			return;
		}
		if(m_vsComms.size())
		{
			// check whether line has an commented string
			for(vector<string>::const_iterator it= m_vsComms.begin(); it != m_vsComms.end(); ++it)
			{
				bpos= read.find(*it);
				if(bpos != string::npos)
				{
					epos= bpos;
					if(!tRv.bcontinue)
					{
						while(	epos &&
								(	read[epos-1] == ' ' ||
									read[epos-1] == '\t'	)	)
						{
							--epos;
						}
					}
					commented= read.substr(epos);
					break;
				}
			}
		}else
		{
			// check whether line has an commented string with default character '#'
			bpos= read.find("#");
			if(bpos != string::npos)
			{
				epos= bpos;
				if(!tRv.bcontinue)
				{
					while(	epos &&
							(	read[epos-1] == ' ' ||
								read[epos-1] == '\t'	)	)
					{
						--epos;
					}
				}
				commented= read.substr(epos);
			}
		}
		if(	bpos != string::npos &&
			m_vsUnComms.size()		)
		{
			// check whether commented line should be uncommented
			for(vector<string>::const_iterator it= m_vsUnComms.begin(); it != m_vsUnComms.end(); ++it)
			{
				if(read.find(*it) == bpos)
				{// found uncommented string
					if(!tRv.breadagain)
					{
						// read first the range before the uncommented string
						// and than in next step the commented
						tRv.breadagain= true;

					}else
					{
						// now read the uncommented string
						tRv.breadagain= false;
						read= read.substr(bpos + it->length());
						bpos= len+1;
						comment= *it; // now comment is the uncommented string
					}
					break;
				}
			}
		}
		if(bpos != string::npos)
			read= read.substr(0, bpos);
		if(!tRv.bcontinue)
		{
			boost::trim(read);
			if(read == "")
			{
				tRv.correct= true;
				*param= tRv;
				return;
			}
			// check for delimiter between property and value
			for(string::const_iterator it= m_sDelimiter.begin(); it != m_sDelimiter.end(); ++it)
			{
				bpos= read.find(*it);
				if(bpos < len)
					break;
				if(*it == ' ')
				{
					bpos= read.find("\t");
					if(bpos < len)
						break;

				}else if(*it == '\t')
				{
					bpos= read.find(" ");
					if(bpos < len)
						break;
				}
			}
			if(bpos > len)
			{
				tRv.correct= false;
				*param= tRv;
				return;
			}
			tRv.correct= true;
			tRv.read= true;
			tRv.parameter= read.substr(0, bpos);
			boost::trim(tRv.parameter);
			read= read.substr(bpos+1);
			boost::trim(read);
			if(comment != "")
				tRv.uncommented= comment;
		}
		// check for localization of value string
		if(m_sBeginLocal != "")
		{
			bool bOk(true);
			bool bslash(false);
			string newread, space;
			string::size_type count(0), bcount(0), ecount(0);

			blen= m_sBeginLocal.length();
			elen= m_sEndLocal.length();
			bpos= string::npos;
			epos= string::npos;
			while(count < len)
			{
				if(!tRv.bcontinue)
				{
					// search begin of localization
					if(read[count] == m_sBeginLocal[bcount])
					{
						if(m_bRemoveLocal)
							bOk= false;
						if(bcount == 0)
							bpos= count;
						++bcount;
						if(bcount == blen)
						{
							tRv.bcontinue= true;
							bcount= 0;
						}

					}else if(bcount != 0)
					{
						bcount= 0;
						count= bpos;
						bOk= true;
					}
				}else
				{
					//search end of localization
					if(	read[count] == m_sEndLocal[ecount] &&
						!bslash									)
					{
						if(m_bRemoveLocal)
							bOk= false;
						if(ecount == 0)
							epos= count;
						++ecount;
						if(ecount == elen)
						{
							tRv.bcontinue= false;
							ecount= 0;
						}

					}else
					{
						if(ecount != 0)
						{
							ecount= 0;
							count= epos;
							bOk= true;
						}
						//search whether character is an back slash
						if(	!bslash &&
							read[count] == '\\' &&
							count+1 < len &&
							(	read[count+1] == '\\' ||
								read[count+1] == m_sEndLocal[0]	) )
						{
							bslash= true;
							if(m_bRemoveLocal)
								bOk= false;
						}else
							bslash= false;
					}
				}
				if(bOk)
				{
					// write spaces into 'space' variable when position outside of localization
					// because outside localization should not be an space on end
					if(	!tRv.bcontinue &&
						(	read[count] == ' ' ||
							read[count] == '\n'		)	)
					{
						space+= read[count];
					}else
					{
						newread+= space + read[count];
						space= "";
					}
				}
				bOk= true;
				++count;
				if(	count == len &&
					tRv.bcontinue &&
					commented != ""	)
				{
					read+= commented;
					if(m_vsComms.size())
					{
						// check for next position of commented string
						for(vector<string>::const_iterator it= m_vsComms.begin(); it != m_vsComms.end(); ++it)
						{
							bpos= read.find(*it, count+1);
							if(bpos != string::npos)
							{
								commented= read.substr(bpos);
								read= read.substr(0, bpos);
								break;
							}
						}
					}else
					{
						// check for next commented string with default character '#'
						bpos= read.find("#", count+1);
						if(bpos != string::npos)
						{
							commented= read.substr(bpos);
							read= read.substr(0, bpos);
						}
					}
				}
				len= read.size();
			}
			//cout << endl;
			tRv.value+= newread;

		}else if(m_sEndLocal != "")
		{
			string::size_type nlen(read.length() - m_sEndLocal.length());

			if(tRv.value.substr(nlen) == m_sEndLocal)
			{
				tRv.value+= read.substr(0, nlen);
				tRv.bcontinue= true;

			}else
			{
				tRv.value+= read;
				tRv.bcontinue= false;
			}
		}else
			tRv.value= read;
		*param= tRv;
		return;
	}

	void Properties::setDefault(const string& key, const string& value, const bool overwrite/*=true*/)
	{
		param_t param;

		param.correct= overwrite;
		param.parameter= key;
		param.value= value;
		m_mDefault[key]= param;
	}

	string Properties::nextProp()
	{
		unsigned int count= 0;
		string sRv;
		map<string, vector<string> >::size_type size;

		size= m_vPropOrder.size();
		if(size < m_nPropCount+1)
		{
			m_nPropCount= 0;
			return "";
		}
		++m_nPropCount;
		for(vector<string>::iterator it= m_vPropOrder.begin(); it != m_vPropOrder.end(); ++it)
		{
			++count;
			sRv= *it;
			if(m_nPropCount == count)
				break;
		}
		return sRv;
	}

	string Properties::needValue(const string property, vector<string>::size_type index/*= 0*/) const
	{
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
			if(m_bByCheck)
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

	string Properties::getPropertyFile(const string& property, vector<string>::size_type index/*= 0*/) const
	{
		unsigned long line;
		string file;
		map<string, vector<string> >::const_iterator found;

		found= m_mvPropertyLines.find(property);
		if(	found == m_mvPropertyLines.end() ||
			index > (found->second.size()-1))
		{
			return "";
		}
		istringstream str(found->second[index]);

		str >> line >> file;
		return file;
	}

	unsigned long Properties::getPropertyLine(const string& property, vector<string>::size_type index/*= 0*/) const
	{
		unsigned long line;
		map<string, vector<string> >::const_iterator found;

		found= m_mvPropertyLines.find(property);
		if(	found == m_mvPropertyLines.end() ||
			index > (found->second.size()-1))
		{
			return 0;
		}
		istringstream str(found->second[index]);

		str >> line;
		return line;
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
		{
			if(index < mContent->second.size())
				value= mContent->second[index];
			else
				value= "";
		}

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

	string Properties::str() const
	{
		string stream("<properties "), sProp;
		map<string, param_t>::const_iterator itDefault;
		map<string, vector<string> >::const_iterator mContent;
		map<string, string>::const_iterator mNotAllowed;
		OParameterStringStream props;

		for(mContent= m_mvPropertyMap.begin(); mContent != m_mvPropertyMap.end(); ++mContent)
		{
			mNotAllowed= m_oNotAllowedParams.find(mContent->first);
			if(mNotAllowed == m_oNotAllowedParams.end())
			{
				for(vector<string>::const_iterator it= mContent->second.begin(); it != mContent->second.end(); ++it)
				{
					sProp= mContent->first + "=" + *it;
					props << sProp;
				}
			}else
			{
				if(mNotAllowed->second != "")
				{
					sProp= mNotAllowed->first + "=" + mNotAllowed->second;
					props << sProp;
				}

			}
		}
		for(itDefault= m_mDefault.begin(); itDefault != m_mDefault.end(); ++itDefault)
		{
			mNotAllowed= m_oNotAllowedParams.find(mContent->first);
			if(mNotAllowed == m_oNotAllowedParams.end())
			{
				mContent= m_mvPropertyMap.find(itDefault->first);
				if(mContent == m_mvPropertyMap.end())
				{
					sProp= itDefault->first + "=" + itDefault->second.value;
					props << sProp;
				}
			}
		}
		stream+= props.str() + " />";
		return stream;
	}

	void Properties::tag(const string& tag)
	{
		string param;
		string::size_type nLen= tag.length();
		vector<string> spl;

		if(nLen < 15 || tag.substr(0, 12) != "<properties ")
			return;

		IParameterStringStream params(tag.substr(12, nLen - 15));
		while(!params.empty())
		{
			params >> param;
			readLine(param);
		}
	}

	string Properties::pulled() const
	{
		string stream("<pulledproperties "), sProp;
		vector<vector<string>::size_type>::const_iterator i;
		map<string, vector<vector<string>::size_type> >::const_iterator mPC;
		OParameterStringStream props;

		for(mPC= m_oPulled.begin(); mPC != m_oPulled.end(); ++mPC)
		{
			OParameterStringStream p;

			sProp= mPC->first + "='";
			for(i= mPC->second.begin(); i != mPC->second.end(); ++i)
				p << *i;
			sProp+= p.str() + "'";
			props << sProp;
		}
		stream+= props.str() + " />";
		return stream;
	}

	void Properties::pulled(const string& params) const
	{
		string nParams;
		vector<string>::size_type index;
		string parameter, value;
		string::size_type nLen= params.length();

		if(nLen < 20 || params.substr(0, 18) != "<pulledproperties ")
			return;
		nParams= params.substr(18, nLen - 20);

		IParameterStringStream ps(nParams);

		while(!ps.empty())
		{
			ps >> parameter;
			nLen= parameter.find("=");
			if(nLen < parameter.length())
			{
				value= parameter.substr(nLen + 2, parameter.length() - (nLen + 2) - 1);
				parameter= parameter.substr(0, nLen);

				IParameterStringStream oIdx(value);
				while(!oIdx.empty())
				{
					oIdx >> index;
					m_oPulled[parameter].push_back(index);
					if(oIdx.fail())
						break;	// an error is occured
								// take only this one fault value (=0)
								// maybe the value was no number for double (perhaps string or boolean)
				}
			}
		}
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
			vector<string>::size_type PSize;
			mpiter pulledIter=  m_oPulled.find(c->first);

			//cout << "for property " << c->first << endl;
			if(pulledIter != m_oPulled.end())
			{
				vPContent= m_oPulled.find(c->first)->second;
				PSize= vPContent.size();
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
