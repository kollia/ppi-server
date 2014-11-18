/**
 *   This file 'BaseErrorHandling.cpp' is part of ppi-server.
 *   Created on: 10.10.2014
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

#include <cerrno>
#include <clocale>
#include <cstring>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "BaseErrorHandling.h"

namespace util
{
	using namespace boost;

	/**
	 * initializing of global static
	 * map for BaseErrorHandling
	 * to holding all different
	 * existing error handling objects
	 */
	map<string, IErrorHandlingPattern*> BaseErrorHandling::m_msoHanlingObjects= map<string, IErrorHandlingPattern*>();
	/**
	 * initializing of global static
	 * map for BaseErrorHandling
	 * to hold all defined groups
	 * for classes
	 */
	map<string, vector<string> > BaseErrorHandling::m_msvGroups= map<string, vector<string> >();

	pthread_mutex_t* BaseErrorHandling::m_OBJECTSMUTEX= Thread::getMutex("OBJECTSMUTEX");
	pthread_mutex_t* BaseErrorHandling::m_ERRNOMUTEX= Thread::getMutex("ERRNOMUTEX");

	BaseErrorHandling::BaseErrorHandling(const string& ownClassName, const string& error/*= ""*/)
	: m_sStdLang("en"),
	  m_sTransLang("en"),
	  m_sOwnClassName(ownClassName)
	{
		if(error != "")
			setErrorStr(error);
		else
			clear();
		add(this);
	}

	BaseErrorHandling::BaseErrorHandling(const string& ownClassName, IErrorHandlingPattern* other)
	: m_sStdLang("en"),
	  m_sTransLang("en"),
	  m_sOwnClassName(ownClassName)
	{
		set(other);
	}

	const IErrorHandlingPattern& BaseErrorHandling::operator = (const BaseErrorHandling& other)
	{
		set(&other);
		return *this;
	}

	const IErrorHandlingPattern* BaseErrorHandling::operator = (const BaseErrorHandling* other)
	{
		set(other);
		return this;
	}

	const IErrorHandlingPattern* BaseErrorHandling::operator = (const IErrorHandlingPattern* other)
	{
		if(other == NULL)
			clear();
		else
			set(other);
		return this;
	}

	const IErrorHandlingPattern& BaseErrorHandling::operator = (const IErrorHandlingPattern& other)
	{
		set(&other);
		return *this;
	}

	const IErrorHandlingPattern* BaseErrorHandling::operator = (const std::auto_ptr<IErrorHandlingPattern> other)
	{
		set(other.get());
		return this;
	}

	const IErrorHandlingPattern* BaseErrorHandling::operator = (const boost::shared_ptr<IErrorHandlingPattern> other)
	{
		set(other.get());
		return this;
	}

	void BaseErrorHandling::set(const IErrorHandlingPattern* other)
	{
		setErrorStr(other->getErrorStr());
	}

	bool BaseErrorHandling::add(IErrorHandlingPattern* other)
	{
		bool bRv(false);
		string otherClassName;

		otherClassName= other->getErrorClassName();
		LOCK(m_OBJECTSMUTEX);
		if(m_msoHanlingObjects.find(otherClassName) == m_msoHanlingObjects.end())
		{
			m_msoHanlingObjects.insert(pair<string, IErrorHandlingPattern*>(otherClassName, other));
			bRv= true;
		}
		UNLOCK(m_OBJECTSMUTEX);
		return bRv;
	}

	void BaseErrorHandling::clear()
	{
		m_tError.type= NO;
		m_tError.errno_nr= 0;
		m_tError.classname= "";
		m_tError.methodname= "";
		m_tError.declarations= "";
		m_tError.ERRORClass= m_sOwnClassName;
		m_tGroups.clear();
	}

	bool BaseErrorHandling::fail(error_types type, int num, const string& classname/*= ""*/,
					const string& methodname/*= ""*/) const
	{
		if(type != UNKNOWN)
		{
			if(m_tError.type != type)
				return false;
		}else
		{
			if(	num != 0 &&
				m_tError.type != errno_error &&
				m_tError.type != errno_warning	)
			{
				return false;
			}
		}
		if(num != m_tError.errno_nr)
			return false;
		if(	classname != "" &&
			classname != m_tError.classname &&
			!hasGroupError(classname)			)
		{
			return false;
		}
		if(	methodname != "" &&
			methodname != m_tError.methodname	)
		{
			return false;
		}
		return true;
	}

	bool BaseErrorHandling::hasError(const string& classname/*= ""*/,
					const string& methodname/*= ""*/) const
	{
		if(	m_tError.type == NO ||
			m_tError.type == errno_warning ||
			m_tError.type == intern_warning ||
			m_tError.type == specific_warning	)
		{
			return false;
		}
		if(	classname != "" &&
			classname != m_tError.classname	)
		{
			return false;
		}
		if( methodname != "" &&
			methodname != m_tError.methodname	)
		{
			return false;
		}
		return true;
	}

	bool BaseErrorHandling::hasWarning(const string& classname/*= ""*/,
					const string& methodname/*= ""*/) const
	{
		if(	m_tError.type == NO ||
			m_tError.type == UNKNOWN ||
			m_tError.type == errno_error ||
			m_tError.type == intern_error ||
			m_tError.type == specific_error	)
		{
			return false;
		}
		if(	classname != "" &&
			classname != m_tError.classname	)
		{
			return false;
		}
		if( methodname != "" &&
			methodname != m_tError.methodname	)
		{
			return false;
		}
		return true;
	}

	void BaseErrorHandling::defineAsWarning()
	{
		if(m_tError.type == intern_error)
			m_tError.type= intern_warning;
		else if(m_tError.type == specific_error)
			m_tError.type= specific_warning;
		else if(m_tError.type == errno_error)
			m_tError.type= errno_warning;
	}

	void BaseErrorHandling::addGroupErrorClasses(const string& groupname,
					const vector<string> classnames)
	{
		map<string, vector<string> >::iterator found;

		LOCK(m_OBJECTSMUTEX);
		found= m_msvGroups.find(groupname);
		if(found != m_msvGroups.end())
		{
			for(vector<string>::const_iterator it= classnames.begin();
							it != classnames.end(); ++it	)
			{
				if(	find(	found->second.begin(),
							found->second.end(),
							*it					) == found->second.end()	)
				{
					found->second.push_back(*it);
				}
			}
		}else
			m_msvGroups.insert(pair<string, vector<string> >(groupname, classnames));
		UNLOCK(m_OBJECTSMUTEX);
	}

	bool BaseErrorHandling::hasGroupError(const string& groupname) const
	{
		map<string, vector<string> >::iterator found;
		vector<string> classes;

		LOCK(m_OBJECTSMUTEX);
		found= m_msvGroups.find(groupname);
		if(found != m_msvGroups.end())
			classes= found->second;
		UNLOCK(m_OBJECTSMUTEX);

		for(vector<string>::iterator it= classes.begin();
						it != classes.end(); ++it	)
		{
			if(hasError(*it))
				return true;
		}
		return false;
	}

	bool BaseErrorHandling::hasGroupWarning(const string& groupname) const
	{
		map<string, vector<string> >::iterator found;
		vector<string> classes;

		LOCK(m_OBJECTSMUTEX);
		found= m_msvGroups.find(groupname);
		if(found != m_msvGroups.end())
			classes= found->second;
		UNLOCK(m_OBJECTSMUTEX);

		for(vector<string>::iterator it= classes.begin();
						it != classes.end(); ++it	)
		{
			if(hasWarning(*it))
				return true;
		}
		return false;
	}

	IEH::error_types BaseErrorHandling::getErrorType() const
	{
		return m_tError.type;
	}

	bool BaseErrorHandling::setError(const string& classname, const string& error_string,
					const string& decl/*= ""*/)
	{
		if(hasError())
			return false;
		m_tError.classname= classname;
		m_tError.methodname= error_string;
		m_tError.type= intern_error;
		m_tError.errno_nr= 0;
		m_tError.declarations= decl;
		m_tError.adderror= "";
		return true;
	}

	bool BaseErrorHandling::setWarning(const string& classname, const string& warn_string,
					const string& decl/*= ""*/)
	{
		if(fail())
			return false;
		m_tError.classname= classname;
		m_tError.methodname= warn_string;
		m_tError.type= intern_warning;
		m_tError.errno_nr= 0;
		m_tError.declarations= decl;
		m_tError.adderror= "";
		return true;
	}

	bool BaseErrorHandling::setErrnoError(const string& classname, const string& error_string,
					int errno_nr, const string& decl/*= ""*/)
	{
		if(hasError())
			return false;
		m_tError.classname= classname;
		m_tError.methodname= error_string;
		m_tError.type= errno_error;
		m_tError.errno_nr= errno_nr;
		m_tError.declarations= decl;
		m_tError.adderror= "";
		return true;
	}

	bool BaseErrorHandling::setErrnoWarning(const string& classname, const string& warn_string,
					int errno_nr, const string& decl/*= ""*/)
	{
		if(fail())
			return false;
		m_tError.classname= classname;
		m_tError.methodname= warn_string;
		m_tError.type= errno_warning;
		m_tError.errno_nr= errno_nr;
		m_tError.declarations= decl;
		m_tError.adderror= "";
		return true;
	}

	void BaseErrorHandling::addMessage(const string& classname,
					const string& methodname, const string& decl/*= ""*/)
	{
		base_errors_t group;

		if(m_tError.type == IEH::NO)
			return;
		group.classname= classname;
		group.methodname= methodname;
		group.declarations= decl;
		m_tGroups.push_back(group);
	}

	string BaseErrorHandling::getErrorStr() const
	{
		string sRv;

		sRv= getErrorStr(/*create error/warning*/NULL);
		for(vector<base_errors_t>::const_iterator it= m_tGroups.begin();
						it != m_tGroups.end(); ++it)
		{
			const base_errors_t* group;

			group= &(*it);
			sRv+= "|" + getErrorStr(group);
		}
		return sRv;
	}

	string BaseErrorHandling::getErrorStr(const base_errors_t* group) const
	{
		string sErrorClass(m_tError.ERRORClass);
		ostringstream oRv;

		if(group != NULL)
			sErrorClass= group->ERRORClass;
		if(group == NULL)
		{
			switch(m_tError.type)
			{
			case NO:
				oRv << "OK";
				break;
			case intern_warning:
			case errno_warning:
			case specific_warning:
				oRv << "WARNING:";
				break;
			default:
				oRv << "ERROR:";
				break;
			}
			if(m_tError.type != NO)
			{
				switch(m_tError.type)
				{
				case NO:
				case UNKNOWN:
					oRv << "UNKNOWN:";
					break;
				case errno_warning:
				case errno_error:
					oRv << "ERRNO:";
					break;
				case intern_warning:
				case intern_error:
					oRv << "INTERN:";
					break;
				case specific_warning:
				case specific_error:
					oRv << "SPECIFIC:";
					break;
				}
				oRv << m_tError.classname << ":";
				oRv << m_tError.methodname << ":";
				oRv << m_tError.errno_nr << ":";
				if(m_tError.adderror != "")
					oRv << m_tError.adderror;
				if(m_tError.declarations != "")
					oRv << ":" << m_tError.declarations;
			}
		}else
		{
			oRv << group->classname << ":";
			oRv << group->methodname;
			if(group->declarations != "")
				oRv << ":" << group->declarations;
		}
		return oRv.str();
	}

	bool BaseErrorHandling::setErrorStr(const string& error)
	{
		bool bIsErrWarn(true);
		vector<string> espl, gspl;
		errorVals_t setError;

		setError.ERRORClass= "";
		clear();
		if(	error == "" ||
			error == "OK" ||
			m_tError.type == intern_error ||
			m_tError.type == errno_error ||
			m_tError.type == specific_error	)
		{
			return false;
		}
		split(gspl, error, is_any_of("|"));
		split(espl, gspl[0], is_any_of(":"));
		if(	espl.size() != 6 &&
			espl.size() != 7		)
		{
			bIsErrWarn= false;
		}
		if(bIsErrWarn)
		{
			if(espl.size() == 7)
				setError.declarations= espl[6];
			if(espl[1] == "ERRNO")
			{
				if(espl[0] == "ERROR")
					setError.type= errno_error;
				else if(espl[0] == "WARNING")
					setError.type= errno_warning;
				else
					bIsErrWarn= false;

			}else if(espl[1] == "INTERN")
			{
				if(espl[0] == "ERROR")
					setError.type= intern_error;
				else if(espl[0] == "WARNING")
					setError.type= intern_warning;
				else
					bIsErrWarn= false;

			}else if(espl[1] == "SPECIFIC")
			{
				if(espl[0] == "ERROR")
					setError.type= specific_error;
				else if(espl[0] == "WARNING")
					setError.type= specific_warning;
				else
					bIsErrWarn= false;
			}else
				bIsErrWarn= false;
			if(bIsErrWarn)
			{
				istringstream nr;

				setError.classname= espl[2];
				setError.methodname= espl[3];
				nr.str(espl[4]);
				nr >> setError.errno_nr;
				if(nr.fail())
				{
					setError.errno_nr= 0;
					setError.type= IEH::UNKNOWN;
				}
				setError.adderror= espl[5];
			}// if(!bIsErrWarn)
		}// if(!bIsErrWarn)
		if(bIsErrWarn)
		{
			vector<string>::iterator it;

			if(	(	m_tError.type == intern_warning ||
					m_tError.type == errno_warning ||
					m_tError.type == specific_warning	) &&
				(	setError.type == intern_warning ||
					setError.type == errno_warning ||
					setError.type == specific_warning	)	)
			{
				return false;
			}
			m_tError.ERRORClass= "";
			m_tError.type= setError.type;
			m_tError.classname= setError.classname;
			m_tError.methodname= setError.methodname;
			m_tError.errno_nr= setError.errno_nr;
			m_tError.adderror= setError.adderror;
			m_tError.declarations= setError.declarations;
			it= gspl.begin(); // <- ERROR
			++it; // <- first group when exist
			while(it != gspl.end())
			{
				setGroupStr(*it);
				++it;
			}
		}
		return bIsErrWarn;
	}

	void BaseErrorHandling::setGroupStr(const string& str)
	{
		base_errors_t group;
		vector<string> spl;
		vector<string>::size_type nsize;

		if(str == "")
			return;
		if(	str.length() > 10 &&
			str.substr(0, 10) == "::unknown:"	)
		{
			group.classname= "::unknown";
			group.methodname= str.substr(10);
			return;
		}
		split(spl, str, is_any_of(":"));
		nsize= spl.size();
		if(	nsize < 2 ||
			nsize > 3	)
		{
			group.classname= "::unknown";
			group.methodname= str;
		}else
		{
			group.classname= spl[0];
			group.methodname= spl[1];
			if(nsize == 3)
				group.declarations= spl[2];
		}
		m_tGroups.push_back(group);
	}

	void BaseErrorHandling::setDescription(const string& lang, const string& classname,
					const string& definition, const string& description	)
	{

		m_mmmsDescriptions[lang][classname][definition]= description;
	}

	string BaseErrorHandling::getGroupErrorDescription() const
	{
		typedef map<string, IErrorHandlingPattern*> objects_typ;
		string sRv;
		errorVals_t error;

		error.type= NO;
		for(vector<base_errors_t>::const_iterator git= m_tGroups.begin();
						git != m_tGroups.end(); ++git	)
		{
			string add;

			if(git->classname != "::unknown")
			{
				error.classname= git->classname;
				error.methodname= git->methodname;
				error.declarations= git->declarations;
				if(git->ERRORClass != "")
				{
					objects_typ::iterator found;

					LOCK(m_OBJECTSMUTEX);
					found= m_msoHanlingObjects.find(m_tError.ERRORClass);
					if(found != m_msoHanlingObjects.end())
						add= found->second->getErrorDescriptionString(error);
					UNLOCK(m_OBJECTSMUTEX);
				}
				if(add == "")
				{
					LOCK(m_OBJECTSMUTEX);
					for(objects_typ::iterator it= m_msoHanlingObjects.begin();
									it != m_msoHanlingObjects.end(); ++it	)
					{
						add= it->second->getErrorDescriptionString(error);
						if(add != "")
							break;
					}
					UNLOCK(m_OBJECTSMUTEX);
					if(add == "")
						add= createDescription("", error);
				}
			}else // if(git->classname != "::unknown")
			{
				// found unknown error string
				add= "unknown additional message string found:\n";
				add+= " inside class " + error.classname;
				add+= " for definition '" + git->methodname + "'";
			}
			if(sRv != "")
				sRv= add + "\n" + sRv;
			else
				sRv= add;
		}
		return sRv;
	}

	string BaseErrorHandling::createDescription(string str, const errorVals_t& error) const
	{
		short correct(0);
		string replace;
		vector<string> declSpl, descSpl;

//		if(m_tError.type != UNKNOWN)
//			str= getDescriptionString(methodname, num);
		if(str == "")
		{
			if(error.type != UNKNOWN)
			{
				str= "found no specified ";
				if(hasError())
					str+= "ERROR";
				else
					str+= "WARNING";
			}else
				str+= "unknown ERROR";
			str+= " by defined string\n'" + getErrorStr(NULL) + "'\n";
			str+= "inside class " + error.classname;
			str+= " for definition '" + error.methodname + "'";
			return str;
		}
		if(error.declarations != "")
			split(declSpl, error.declarations, is_any_of("@"));
		if(str.substr(0, 1) == "@")
		{
			replace= str.substr(1);
			correct|= replaceFirstDeclaration(replace, declSpl);
			str= replace;
		}
		split(descSpl, str, is_any_of("@"));
		str= "";
		for(vector<string>::iterator it= descSpl.begin(); it != descSpl.end(); ++it)
		{
			if(it != descSpl.begin())
			{
				replace= *it;
				correct|= replaceFirstDeclaration(replace, declSpl);
				str+= replace;
			}else
				str= *it;
		}
		if(correct != 0)
		{
			str+= "\n (unknown placeholders ";
			str+= "inside class " + error.classname;
			str+= " for definition '" + error.methodname + "')";
		}
		return str;
	}

	short BaseErrorHandling::replaceFirstDeclaration(string& str,
					const vector<string>& declarations) const
	{
		bool bFound(true);
		vector<string>::size_type nNum;
		string declaration;
		string::size_type nLen;
		istringstream iNum(str);
		ostringstream oNum;

		iNum >> nNum;
		if(iNum.fail())
			nNum= 0;
		if(nNum != 0)
		{
			oNum << nNum;
			if(declarations.size() < nNum)
			{
				declaration= "(no declaration for place-holder @" + oNum.str() + " found)";
				bFound= false;
			}else
				declaration= declarations[nNum - 1];
			nLen= oNum.str().length();
		}else
		{
			if(str.length() == 0)
			{
				declaration= "(no place-holder number after @ be given)";
				bFound= false;
			}else
				declaration= "(unknown place-holder number '@" + str.substr(0, 1) + "' found)";
			nLen= 0;
		}
		str= declaration + str.substr(nLen);
		if(bFound)
			return 0;
		return -1;
	}

	string BaseErrorHandling::getDescription() const
	{
		typedef map<string, IErrorHandlingPattern*> objects_typ;
		string sRv, sGroups;

		if(m_tError.type == NO)
		{
			sRv= "no error occurred!";
			return sRv;
		}
		if(m_tError.ERRORClass != "")
		{
			objects_typ::iterator found;

			LOCK(m_OBJECTSMUTEX);
			found= m_msoHanlingObjects.find(m_tError.ERRORClass);
			if(found != m_msoHanlingObjects.end())
				sRv= found->second->getErrorDescriptionString(m_tError);
			UNLOCK(m_OBJECTSMUTEX);
		}
		if(sRv == "")
		{
			LOCK(m_OBJECTSMUTEX);
			for(objects_typ::iterator it= m_msoHanlingObjects.begin();
							it != m_msoHanlingObjects.end(); ++it	)
			{
				sRv= it->second->getErrorDescriptionString(m_tError);
				if(sRv != "")
					break;
			}
			UNLOCK(m_OBJECTSMUTEX);
		}
		if(sRv == "")
			sRv= createDescription("", m_tError);
		sGroups= getGroupErrorDescription();
		if(sGroups != "")
			sRv= sGroups + "\n" + sRv;
		return sRv;
	}

	string BaseErrorHandling::getErrorDescriptionString(errorVals_t error) const
	{
		errlang_t::const_iterator langIt;
		errclass_t::const_iterator classIt;
		errmethod_t::const_iterator methodIt;
		string lang, sRv;

		lang= m_sTransLang;
		while(lang != "")
		{
			langIt= m_mmmsDescriptions.find(lang);
			if(langIt != m_mmmsDescriptions.end())
			{
				classIt= langIt->second.find(error.classname);
				if(classIt != langIt->second.end())
				{
					methodIt= classIt->second.find(error.methodname);
					if(methodIt != classIt->second.end())
					{
						//found correct error description
						sRv= createDescription(methodIt->second, error);
						if(	error.type == errno_error ||
							error.type == errno_warning	)
						{
							sRv+= "\n" + getErrnoString(error.errno_nr);
						}
						return sRv;
					}
				}
			}
			if(lang != m_sStdLang)
				lang= m_sStdLang;
			else
				break;
		}// while(lang != "")
		return "";
	}

	string BaseErrorHandling::getErrnoString(int errnoNr)
	{
	    char *err;
	    string sRv;

	    /*
	     * make strerror own thread-safe
	     * because the gnu version of strerror_r
	     * differ to posix version
	     * and can make troubles by compiling
	     * when environment changing
	     * the local version strerror_l
	     * should be thread-safe
	     * but by the parameters
	     * for me it don't look so
	     */
	    LOCK(m_ERRNOMUTEX);
	    err= strerror(errnoNr);
	    sRv= err;
	    UNLOCK(m_ERRNOMUTEX);
	    return sRv;

	    /*
				setlocale(LC_ALL, "");
				loc = newlocale (LC_MESSAGES_MASK, "C", NULL);
				lerr= strerror_l(errnoNr, loc);
				perr= strerror_r(errnoNr, err, size);
				cout << "read for erno " << errnoNr << "  '" << perr << "'" << endl;
				cout << "                  '" << err << "'" << endl;
				cout << "                  '" << lerr << "'" << endl;
				nerr= errno;
				freelocale(loc);			*/
	}

} /* namespace util */
