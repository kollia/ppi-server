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

#ifndef __BASETHREADEXCEPTION_H
#define __BASETHREADEXCEPTION_H

#define TRACELINES 25
#define SPACELEFT "      "

#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>

#include <csignal>
#include <cstdio>

#include <string>
#include <vector>
#include <exception>

using namespace std;

template <class SignalExceptionClass> class SignalTranslator
 {

 public:
     class SingleTonTranslator
     {
     public:
         SingleTonTranslator()
         {
             signal(SignalExceptionClass::GetSignalNumber(), SignalHandler);
         }

         static void SignalHandler(int sig)
         {
			sigset_t signal_set;

			sigemptyset(&signal_set);
			if(sigaddset(&signal_set, SignalExceptionClass::GetSignalNumber()) != 0)
				cout << "cannot add signal -" << SignalExceptionClass::GetSignalNumber() << endl;
			if(sigprocmask(SIG_UNBLOCK, &signal_set, NULL) != 0)
				cout << "cannot unblock signal -" << SignalExceptionClass::GetSignalNumber() << endl;
             throw SignalExceptionClass();
         }
     };

     SignalTranslator()
     {
         static SingleTonTranslator s_objTranslator;
     }
 };

class SignalException : public exception
{
	public:
		SignalException() throw();
		SignalException(const string& errortxt) throw();
		virtual ~SignalException() throw();

		const string getMessage() const { return m_sErrorText; };
		vector<string> getTraceVector() const;
		string getTraceString() const;
		void printTrace() const;
	    /** Returns a C-style character string describing the general cause
	     *  of the current error.  */
	    virtual const char* what() const throw();
	    virtual int getSignal() const { return 0; };
	    void addMessage(const string& message);

	protected:
		/**
		 * single exception messages,
		 * or static string holder for uncaught exceptions
		 */
		mutable string m_sErrorText;

		pid_t getThreadID() const { return m_nThreadID; };
		vector<string> getFirstTraceVector() const;
		virtual string getMainErrorMessage() const;
		virtual string getClassName() const { return "SignalException"; };
		virtual string getSignalName() const { return ""; };

	private:
		/**
		 * whether exception class was initialed with main error
		 */
		mutable bool m_bInit;
		/**
		 * id of actual running thread
		 */
		pid_t m_nThreadID;
		/**
		 * vector of stack trace
		 */
		vector<string> m_vStackTrace;

		/**
		 * return vector of stack trace
		 *
		 * @param del delete first rows of stack trace
		 * @return stack trace
		 */
		vector<string> trace(const unsigned short del) const;
};

 class SegmentationFault : public SignalException
 {
 public:
	 virtual ~SegmentationFault() throw() {};
     static int GetSignalNumber() {return SIGSEGV;}
	 virtual int getSignal() const { return GetSignalNumber(); };

 protected:
     virtual string getSignalName() const { return "SIGSEGV"; };
     virtual string getClassName() const { return "SegmentationFault"; };
 };


 class FloatingPointException : public SignalException
 {
 public:
	 virtual ~FloatingPointException() throw() {};
     static int GetSignalNumber() {return SIGFPE;}
	 virtual int getSignal() const { return GetSignalNumber(); };

 protected:
     virtual string getSignalName() const { return "SIGFPE"; };
     virtual string getClassName() const { return "FloatingPointException"; };
 };

#endif // __BASETHREADEXCEPTION_H


