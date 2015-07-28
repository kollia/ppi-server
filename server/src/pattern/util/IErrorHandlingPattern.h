/**
 *   This file 'IErrorHanlingPattern.h' is part of ppi-server.
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

#ifndef IERRORHANDLINGPATTERN_H_
#define IERRORHANDLINGPATTERN_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace design_pattern_world
{
	namespace util_pattern
	{
		using namespace std;
		/**
		 * Error handling pattern
		 * to use for all specific
		 * error results
		 */
		class IErrorHandlingPattern
		{
		public:
			/**
			 * enum of existing errors
			 */
			enum error_types
			{
				/**
				 * no error occurred
				 */
				NO= 0,
				/**
				 * unknown error occurred
				 */
				UNKNOWN,
				/**
				 * error exist inside errno number
				 */
				errno_error,
				/**
				 * error exist inside errno number,
				 * but will be only an warning
				 * no possible error occured
				 */
				errno_warning,
				/**
				 * internal warning number from
				 * defined error object
				 */
				intern_warning,
				/**
				 * internal error number from
				 * defined error object
				 */
				intern_error,
				/**
				 * specific internal warning number from
				 * defined error object
				 */
				specific_warning,
				/**
				 * specific internal error number from
				 * defined error object
				 */
				specific_error
			};
			/**
			 * base of error values,
			 * for direct errors and
			 * also additional group errors
			 */
			struct base_errors_t
			{
				/**
				 * inside which error handling object
				 * current error stored
				 */
				string ERRORClass;
				/**
				 * name of specific class
				 * where error occurred
				 */
				string classname;
				/**
				 * name of method
				 * where error occurred
				 */
				string errorstring;
				/**
				 * declaration of strings
				 * inside error description,
				 * separated with an '@'
				 */
				string declarations;
			};
			/**
			 * error values
			 */
			struct errorVals_t : public base_errors_t
			{
				/**
				 * current error type
				 */
				error_types type;
				/**
				 * current error number
				 */
				int errno_nr;
				/**
				 * additional error string for description
				 */
				string adderror;

			};

			/**
			 * operator to initialize with interface from object
			 * from error handling pattern
			 *
			 * @param other interface of error handling objects
			 * @return own object
			 */
			virtual const IErrorHandlingPattern& operator = (const IErrorHandlingPattern& other)= 0;
			/**
			 * operator to initialize with interface from object
			 * from error handling pattern
			 *
			 * @param other interface of error handling objects
			 * @return own object
			 */
			virtual const IErrorHandlingPattern* operator = (const IErrorHandlingPattern* other)= 0;
			/**
			 * operator to initialize with interface from object
			 * from error handling pattern
			 *
			 * @param other interface of error handling objects
			 * @return own object
			 */
			virtual const IErrorHandlingPattern* operator = (std::auto_ptr<IErrorHandlingPattern> other)= 0;
			/**
			 * operator to initialize with interface from object
			 * from error handling pattern
			 *
			 * @param other interface of error handling objects
			 * @return own object
			 */
			virtual const IErrorHandlingPattern* operator = (const boost::shared_ptr<IErrorHandlingPattern> other)= 0;
			/**
			 * return class name definition
			 * of current object
			 */
			virtual string getErrorClassName() const= 0;
			/**
			 * set standard language
			 * with two characters
			 * in ISO standard
			 */
			virtual void setStdLang(const string& lang)= 0;
			/**
			 * set current translate language
			 * with two characters
			 * in ISO standard
			 */
			virtual void setLang(const string& lang)= 0;
			/**
			 * set current object
			 * back to no error
			 */
			virtual void clear()= 0;
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
			virtual void addGroupErrorClasses(const string& groupname, const vector<string> classnames)= 0;
			/**
			 * write error into class, for explicit classname
			 *
			 * @param classname name of class where error occurred
			 * @param error_string name of error string defined inside translation reading file
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			virtual bool setError(const string& classname, const string& error_string,
							const string& decl= "")= 0;
			/**
			 * write warning into class for explicit classname
			 *
			 * @param classname name of class where error occurred
			 * @param warn_string name of warning string defined inside translation reading file
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether warning be set.<br />when error or warning before exist no new warning will be set
			 */
			virtual bool setWarning(const string& classname, const string& warn_string,
							const string& decl= "")= 0;
			/**
			 * write errno error into class
			 * and add error from translation file
			 *
			 * @param classname name of class where error occurred
			 * @param error_string name of error string defined inside translation reading file
			 * @param errno_nr error number where error occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			virtual bool setErrnoError(const string& classname, const string& error_string,
							int errno_nr, const string& decl= "")= 0;
			/**
			 * write errno warning into class
			 * and add error from translation file
			 *
			 * @param classname name of class where error occurred
			 * @param warn_string name of warning string defined inside translation reading file
			 * @param errno_nr error number where error occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether warning be set.<br />when error or warning before exist no new warning will be set
			 */
			virtual bool setErrnoWarning(const string& classname, const string& warn_string,
							int errno_nr, const string& decl= "")= 0;
			/**
			 * write error for specific method with ERRNO number
			 * which has an light difference of real ERRNO number
			 * and add the error string for classname.<br />
			 * This method search first for ERRNO the explicit error/warning description
			 * of &lt;errorstring&gt;_&lt;ERRNO_STR&gt; where classname is ##standard_errno##,
			 * when not found search with defined classname,
			 * otherwise it will be give back the original error string for ERRNO number.<br />
			 * ERRNO_STR is the pre-defined string for ERRNO number inside errno.h
			 *
			 * @param classname name of class where error occurred
			 * @param errorstring specific method inside which error occures
			 * @param error_string name of error string defined inside translation reading file
			 * @param errno_nr errno number needed when occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			virtual bool setMethodError(const string& classname, const string& methodname,
							const string& error_string, int errno_nr, const string& decl= "")= 0;
			/**
			 * write warning for specific method with ERRNO number
			 * which has an light difference of real ERRNO number
			 * and add the warning string for classname.<br />
			 * See description of errno string by method <code>setMethodError()</code>
			 *
			 * @param classname name of class where error occurred
			 * @param errorstring specific method inside which error occures
			 * @param warn_string name of error string defined inside translation reading file
			 * @param errno_nr errno number needed when occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 * @return whether error be set.<br />when error before exist no new error will be set
			 */
			virtual bool setMethodWarning(const string& classname, const string& methodname,
							const string& warn_string, int errno_nr, const string& decl= "")= 0;
			/**
			 * add an message description before current error
			 *
			 * @param classname name of class where error occurred
			 * @param errorstring name of method where error occurred
			 * @param decl declaration of strings inside error description, separated with an '@'
			 */
			virtual void addMessage(const string& classname,
							const string& errorstring, const string& decl= "")= 0;
			/**
			 * change any error, when exist, to warning
			 */
			virtual void changeToWarning()= 0;
			/**
			 * asking whether an error exist from all classes defined in the group
			 *
			 * @param groupname name of the group
			 * @return whether in the group classes was occurred an error
			 */
			virtual bool hasGroupError(const string& groupname) const= 0;
			/**
			 * asking whether an error exist from all classes defined in the group
			 *
			 * @param groupname name of the group
			 * @return whether in the group classes was occurred an error
			 */
			virtual bool hasGroupWarning(const string& groupname) const= 0;
			/**
			 * whether current object has an error or warning
			 *
			 * @return whether error exist
			 */
			virtual bool fail() const= 0;
			/**
			 * whether current object has specific error or warning
			 *
			 * @param type which type of error number next parameter will be
			 * @param classname name of class where error occurred
			 * @param errorstring name of method where error occurred
			 * @return whether error exist
			 */
			virtual bool fail(error_types type, const string& classname= "",
							const string& errorstring= "") const= 0;
			/**
			 * whether current object has errno error or warning
			 *
			 * @param num error number where error occurred
			 * @param classname name of class where error occurred
			 * @param errorstring name of method where error occurred
			 * @return whether error exist
			 */
			virtual bool fail(int num, const string& classname= "",
							const string& methodname= "") const= 0;
			/**
			 * whether current object has specific error or warning
			 *
			 * @param type which type of error number next parameter will be
			 * @param num error number where error occurred
			 * @param classname name of class where error occurred
			 * @param errorstring name of method where error occurred
			 * @return whether error exist
			 */
			virtual bool fail(error_types type, int num,
							const string& classname= "", const string& errorstring= "") const= 0;
			/**
			 * whether current object has an error.<br />
			 * can be differ between class name, method name or error type
			 *
			 * @param classname name of class where error occurred
			 * @param errno_nr errno number on which error occurred
			 * @return whether error exist
			 */
			virtual bool hasError(const string& classname, const int errno_nr) const= 0;
			/**
			 * whether current object has an error.<br />
			 * can be differ between class name, method name or error type
			 *
			 * @param classname name of class where error occurred
			 * @param methodname name of method where error occurred
			 * @param errno_nr errno number on which error occurred
			 * @return whether error exist
			 */
			virtual bool hasError(const string& classname= "", const string& methodname= "",
							const int errno_nr= 0) const= 0;
			/**
			 * whether current object has an warning
			 *
			 * @param classname name of class where warning occurred
			 * @param errno_nr errno number on which warning occurred
			 * @return whether warning exist
			 */
			virtual bool hasWarning(const string& classname, const int errno_nr) const= 0;
			/**
			 * whether current object has an warning
			 *
			 * @param classname name of class where warning occurred
			 * @param errno_nr errno number on which warning occurred
			 * @return whether warning exist
			 */
			virtual bool hasWarning(const string& classname= "", const string& methodname= "",
							const int errno_nr= 0) const= 0;
			/**
			 * define intern-, specific- or errno-ERROR as WARNING
			 * when exist
			 */
			virtual void defineAsWarning()= 0;
			/**
			 * return type of error or warning
			 *
			 * @return error type
			 */
			virtual error_types getErrorType() const= 0;
			/**
			 * create short error string
			 * from existing error
			 *
			 * @return error string
			 */
			virtual string getErrorStr() const= 0;
			/**
			 * create error object from short error string
			 *
			 * @param error short error string
			 * @return whether error string was correct
			 */
			virtual bool setErrorStr(const string& error)= 0;
			/**
			 * returning error description
			 * form current object
			 *
			 * @return error description
			 */
			virtual string getDescription() const= 0;
			/**
			 * dummy destructor of interface
			 */
			virtual ~IErrorHandlingPattern()
			{};
			/**
			 * select from m_mmmsDescriptions map
			 * defined original description string
			 * with Declarations occurs from error
			 *
			 * @param error return description for this error types
			 * @param groups with this additional groups
			 * @return error description with filled place-holder
			 */
			virtual string getErrorDescriptionString(errorVals_t error) const= 0;
		};
		/**
		 * short type definition
		 * of IErrorHandlingPattern
		 */
		typedef IErrorHandlingPattern IEH;
		/**
		 * type definition
		 * of error handling object
		 * inside an shared pointer
		 */
		typedef boost::shared_ptr<IErrorHandlingPattern> EHObj;


	}// namespace util_pattern
}// namespace design_pattern_world

#endif /* IERRORHANDLINGPATTERN_H_ */
