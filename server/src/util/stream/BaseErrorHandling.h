/**
 *   This file 'BaseErrorHandling.h' is part of ppi-server.
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

#ifndef BASEERRORHANDLING_H_
#define BASEERRORHANDLING_H_

#include <string>
#include <vector>
#include <map>

#include "../debug.h"

#include "../../pattern/util/IErrorHandlingPattern.h"

#include "../thread/Thread.h"

namespace util
{

	using namespace std;
	using namespace design_pattern_world::util_pattern;

	class BaseErrorHandling : public IErrorHandlingPattern
	{
	public:

		/**
		 * constructor for null initialize class
		 * or initialized with given short error string
		 *
		 * @param ownClassName name of own class to know from where any error comes
		 * @param short_error short error string to predefine object
		 */
		BaseErrorHandling(const string& ownClassName, const string& short_error= "");
		/**
		 * constructor to initialize with foreign ErrorHandling class
		 *
		 * @param other other object of error handling class
		 */
		BaseErrorHandling(const string& ownClassName, IErrorHandlingPattern* other);

		/**
		 * set content of other object
		 * from error handling pattern
		 * into own
		 *
		 * @param other base error handling object
		 */
		void set(const IErrorHandlingPattern* other);
		/**
		 * operator to initialize with object
		 * from error handling pattern
		 *
		 * @param other base error handling object
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern* operator = (const BaseErrorHandling* other);
		/**
		 * operator to initialize with object
		 * from error handling pattern
		 *
		 * @param other base error handling object
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern& operator = (const BaseErrorHandling& other);
		/**
		 * operator to initialize with interface from object
		 * from error handling pattern
		 *
		 * @param other interface of error handling objects
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern& operator = (const IErrorHandlingPattern& other);
		/**
		 * operator to initialize with interface from object
		 * from error handling pattern
		 *
		 * @param other interface of error handling objects
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern* operator = (const IErrorHandlingPattern* other);
		/**
		 * operator to initialize with interface from object
		 * from error handling pattern
		 *
		 * @param other interface of error handling objects
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern* operator = (const std::auto_ptr<IErrorHandlingPattern> other);
		/**
		 * operator to initialize with interface from object
		 * from error handling pattern
		 *
		 * @param other interface of error handling objects
		 * @return own object
		 */
		OVERWRITE const IErrorHandlingPattern* operator = (const boost::shared_ptr<IErrorHandlingPattern> other);
		/**
		 * return class name definition
		 * of current object
		 */
		OVERWRITE string getErrorClassName() const
		{ return m_sOwnClassName; };
		/**
		 * set current object
		 * back to no error
		 */
		OVERWRITE void clear();
		/**
		 * add other error handling object
		 * to own only when not implemented before
		 *
		 * @param other other object of error handling class
		 * @return whether object was add to own
		 */
		virtual bool add(IErrorHandlingPattern* other);
		/**
		 * write normal error into class
		 *
		 * @param classname name of class where error occurred
		 * @param error_string name of error string defined inside translation reading file
		 * @param decl declaration of strings inside error description, separated with an '@'
		 * @return whether error be set.<br />when error before exist no new error will be set
		 */
		OVERWRITE bool setError(const string& classname, const string& error_string,
						const string& decl= "");
		/**
		 * write normal error into class
		 *
		 * @param classname name of class where error occurred
		 * @param warn_string name of warning string defined inside translation reading file
		 * @param decl declaration of strings inside error description, separated with an '@'
		 * @return whether warning be set.<br />when error or warning before exist no new warning will be set
		 */
		OVERWRITE bool setWarning(const string& classname, const string& warn_string,
						const string& decl= "");
		/**
		 * write errno error into class
		 *
		 * @param classname name of class where error occurred
		 * @param error_string name of error string defined inside translation reading file
		 * @param errno_nr error number where error occurred
		 * @param decl declaration of strings inside error description, separated with an '@'
		 * @return whether error be set.<br />when error before exist no new error will be set
		 */
		OVERWRITE bool setErrnoError(const string& classname, const string& error_string,
						int errno_nr, const string& decl= "");
		/**
		 * write errno warning into class
		 *
		 * @param classname name of class where error occurred
		 * @param warn_string name of warning string defined inside translation reading file
		 * @param errno_nr error number where error occurred
		 * @param decl declaration of strings inside error description, separated with an '@'
		 * @return whether warning be set.<br />when error or warning before exist no new warning will be set
		 */
		OVERWRITE bool setErrnoWarning(const string& classname, const string& warn_string,
						int errno_nr, const string& decl= "");
		/**
		 * add an message description before current error
		 *
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @param decl declaration of strings inside error description, separated with an '@'
		 */
		OVERWRITE void addMessage(const string& classname, const string& methodname,
						const string& decl= "");
		/**
		 * whether current object has an error or warning
		 *
		 * @return whether error exist
		 */
		OVERWRITE bool fail() const
		{ return (m_tError.type == NO ? false : true); };
		/**
		 * whether current object has specific error or warning
		 *
		 * @param type which type of error number next parameter will be
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @return whether error exist
		 */
		OVERWRITE bool fail(error_types type, const string& classname= "",
						const string& methodname= "") const
		{ return fail(type, m_tError.errno_nr, classname, methodname); };
		/**
		 * whether current object has errno error or warning
		 *
		 * @param num error number where error occurred
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @return whether error exist
		 */
		OVERWRITE bool fail(int num, const string& classname= "",
						const string& methodname= "") const
		{ return fail(IEH::UNKNOWN, num, classname, methodname); };
		/**
		 * whether current object has specific error or warning
		 *
		 * @param type which type of error number next parameter will be
		 * @param num error number where error occurred
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @return whether error exist
		 */
		OVERWRITE bool fail(error_types type, int num,
						const string& classname= "", const string& methodname= "") const;
		/**
		 * whether current object has an error.<br />
		 * can be differ between class name, method name or error type
		 *
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @param type which type of error number next parameter will be
		 * @return whether error exist
		 */
		OVERWRITE bool hasError(const string& classname= "", const string& methodname= "") const;
		/**
		 * whether current object has an warning
		 *
		 * @param classname name of class where error occurred
		 * @param methodname name of method where error occurred
		 * @param type which type of error number next parameter will be
		 * @return whether warning exist
		 */
		OVERWRITE bool hasWarning(const string& classname= "", const string& methodname= "") const;
		/**
		 * asking whether an error exist from all classes defined in the group
		 *
		 * @param groupname name of the group
		 * @return whether in the group classes was occurred an error
		 */
		OVERWRITE bool hasGroupError(const string& groupname) const;
		/**
		 * asking whether an error exist from all classes defined in the group
		 *
		 * @param groupname name of the group
		 * @return whether in the group classes was occurred an error
		 */
		OVERWRITE bool hasGroupWarning(const string& groupname) const;
		/**
		 * define intern-, specific- or errno-ERROR as WARNING
		 * when exist
		 */
		OVERWRITE void defineAsWarning();
		/**
		 * return type of error or warning
		 *
		 * @return error type
		 */
		OVERWRITE error_types getErrorType() const;
		/**
		 * create short error or warning string
		 * from existing error
		 *
		 * @return error string
		 */
		OVERWRITE string getErrorStr() const;
		/**
		 * create error object from short error string
		 *
		 * @param error short error string
		 * @return whether error string was correct
		 */
		OVERWRITE bool setErrorStr(const string& error);
		/**
		 * set standard language
		 * with two characters
		 * in ISO standard
		 *
		 * @param lang language
		 */
		OVERWRITE void setStdLang(const string& lang)
		{ m_sStdLang= lang; };
		/**
		 * set current translate language
		 * with two characters
		 * in ISO standard
		 */
		OVERWRITE void setLang(const string& lang)
		{ m_sTransLang= lang; };
		/**
		 * returning error description
		 * form current object
		 *
		 * @return error description
		 */
		OVERWRITE string getDescription() const;
		/**
		 * returning error description
		 * form predefined group
		 *
		 * @return error description
		 */
		OVERWRITE string getGroupErrorDescription() const;
		/**
		 * create errno string from error number
		 *
		 * @param errnoNr system errno number
		 * @return errno description
		 */
		static string getErrnoString(int errnoNr);
		/**
		 * define all internal error descriptions
		 */
		virtual void read()= 0;
		/**
		 * select from m_mmmsDescriptions map
		 * defined original description string
		 * with Declarations occurs from error
		 *
		 * @param error return description for this given error types
		 * @return error description with filled place-holder
		 */
		OVERWRITE string getErrorDescriptionString(errorVals_t error) const;

	protected:
		/**
		 * standard language
		 */
		string m_sStdLang;
		/**
		 * translated language
		 */
		string m_sTransLang;
		/**
		 * current error or warning values
		 */
		errorVals_t m_tError;
		/**
		 * additional messages for
		 * error or warning
		 */
		vector<base_errors_t> m_tGroups;

		/**
		 * set string of error description
		 * into current error object
		 *
		 * @param lang description language
		 * @param classname name of class where error occurred
		 * @param definition string of definition for error or warning
		 * @param descript description for the given error types
		 */
		void setDescription(const string& lang, const string& classname,
						const string& definition, const string& description);
		/**
		 * define group name for more classes.<br />
		 * this will be helpful when error asking question of <code>hasError()</code>
		 * should made over more classes, and maybe in the feature
		 * defined an other error handling class which should add to the question.
		 * Can used for <code>hasGroupError()</code> or <code>hasGroupWarning()</code>
		 *
		 * @param groupname name of group of classes, maybe the interface class name
		 * @param classnames name of all classes should add to the group
		 */
		virtual void addGroupErrorClasses(const string& groupname, const vector<string> classnames);
		/**
		 * replace string with leading number
		 * with declaration from vector
		 *
		 * @param str string to replace
		 * @param declarations vector of all declarations
		 * @return when found all place holders 0 otherwise -1
		 */
		short replaceFirstDeclaration(string& str, const vector<string>& declarations) const;
		/**
		 * create group message from short error string
		 *
		 * @param error short error string
		 * @return whether error string was correct
		 */
		void setGroupStr(const string& str);

	private:
		/**
		 * type definition for
		 * map< LANG, map< CLASS, map< definition, description > > >
		 */
		typedef  map<string, map<string, map<string, string> > > errlang_t;
		/**
		 * type definition for
		 * map< CLASS, map< definition, description > >
		 */
		typedef  map<string, map<string, string> > errclass_t;
		/**
		 * type definition for
		 * map< definition, description >
		 */
		typedef  map<string, string> errmethod_t;

		/**
		 * class name of own object
		 */
		const string m_sOwnClassName;
		/**
		 * all error/warning descriptions<br />
		 * split to map< LANG, map< CLASS, map< definition, description > > >
		 */
		errlang_t m_mmmsDescriptions;
		/**
		 * all other error handling classes
		 */
		static map<string, IErrorHandlingPattern*> m_msoHanlingObjects;
		/**
		 * all group definitions for classes
		 */
		static map<string, vector<string> > m_msvGroups;

		/**
		 * mutex to lock over static members
		 */
		static pthread_mutex_t* m_OBJECTSMUTEX;
		/**
		 * mutex to own lock by method strerror()
		 * for errno
		 */
		static pthread_mutex_t* m_ERRNOMUTEX;

		/**
		 * returning error description
		 * form current object
		 * with changing no values
		 *
		 * @param str description with place-holder
		 * @param error current error definition
		 * @return finished error description
		 */
		OVERWRITE string createDescription(string str, const errorVals_t& error) const;
		/**
		 * create short message
		 * from existing error/warning or group
		 *
		 * @param group create additional short group string when parameter exist,
		 *              elsewhere an short error or warning string
		 * @return error string
		 */
		string getErrorStr(const base_errors_t* group) const;

	};

} /* namespace util */
#endif /* BASEERRORHANDLING_H_ */
