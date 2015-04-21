/**
 *   This file 'ThreadErrorHandling.cpp' is part of ppi-server.
 *   Created on: 19.10.2014
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

#include "ThreadErrorHandling.h"

namespace util
{
	namespace thread
	{
		void ThreadErrorHandling::createMessages()
		{
			setDescription("en", "Thread", "start",
							"thread running currently");

			setDescription("en", "Thread", "start_exception",
							"exception by trying to start thread @1");

			setDescription("en", "Thread", "pthread_attr_init",
							"by creating attribute object of thread @1");
			setDescription("en", "Thread", "pthread_create",
							"by creating thread object @1");
			setDescription("en", "Thread", "pthread_join",
							"cannot join correctly to thread  @1 "
							"until waiting for running thread by stopping");
			setDescription("en", "Thread", "no_scheduling_set_possibe",
							"no new definition of scheduling policy/priority "
							"from running thread @1 by external thread possible");
			setDescription("en", "Thread", "read_scheduling",
							"cannot read current scheduling policy of thread @1");
			setDescription("en", "Thread", "set_scheduling",
							"cannot set new scheduling policy @1 with priority @2\n"
							"by thread @3[ID:@4] with user id (@5)\n"
							"(policy posibility for priority is from min:@6 to max:@7"	);

			setDescription("en", "UserManagementInitialization", "init",
							"cannot initial UserManagement");

			setDescription("en", "CommandExec", "init",
							"command string conversation throw exception");
			setDescription("en", "CommandExec", "popen",
							"can not open SHELL for @3 inside subroutine '@1::@2'");

			setDescription("en", "NoAnswerSender", "sendMethod",
							"By sending method @1 over NoAnswerSender Thread getting warning:");
			setDescription("en", "NoAnswerSender", "sendMethod",
							"By sending method @1 over NoAnswerSender Thread getting error:");

			setDescription("en", "NeedDbChanges", "start",
							"cannot start thread to looking for database changes");

			setDescription("en", "LogThread", "start",
							"logging thread is not defined for starting");
			setDescription("en", "LogThread", "stop",
							"logging thread is not defined for stopping");

			setDescription("en", "DatabaseThread", "init",
							"no database be defined");
		}

		bool ThreadErrorHandling::setPThreadError(const string& classname, const string& methodname,
						const string& error_string, int errno_nr, const string& decl/*= ""*/)
		{
			if(!setErrnoError(classname, error_string, errno_nr, decl))
				return false;
			m_tError.type= specific_error;
			m_tError.adderror= methodname;
			return true;
		}

		bool ThreadErrorHandling::setPThreadWarning(const string& classname, const string& methodname,
						const string& warn_string, int errno_nr, const string& decl/*= ""*/)
		{
			if(!setErrnoWarning(classname, warn_string, errno_nr, decl))
				return false;
			m_tError.type= specific_warning;
			m_tError.adderror= methodname;
			return true;
		}

		string ThreadErrorHandling::getErrorDescriptionString(errorVals_t error) const
		{
			bool bset(false);
			string err, sRv;

			if(	error.type == specific_error ||
				error.type == specific_warning	)
			{
				bset= true;
				switch(error.errno_nr)
				{
				case EAGAIN: // pthread_create
					if(error.adderror == "pthread_create")
					{
						err= "Insufficient  resources  to create another thread, "
							"or a system-imposed limit on the number of threads was encountered.";
					}else
						bset= false;
					break;
				case EDEADLK: // pthread_join
					if(error.adderror == "pthread_join")
					{
						err= "A deadlock was detected (e.g., two threads tried to join with each other); "
									"or thread specifies the calling thread.";
					}else
						bset= false;
					break;
				case EINVAL: // pthread_create / pthread_join / pthread_detach
					if(error.adderror == "pthread_create")
						err= "Invalid settings in thread attributes object (attr).";
					else if(	error.adderror == "pthread_join" ||
								error.adderror == "pthread_detach"	)
					{
						err= "thread is not a joinable thread";
						if(error.adderror == "pthread_join")
							err+= " or another thread is already waiting to join with this thread.";

					}else if(error.adderror == "pthread_setschedparam")
						err= "policy is not a recognized policy, or priority does not make sense for the policy.";
					else
						bset= false;
					break;
				case EPERM: // pthread_create
					if(error.adderror == "pthread_create")
					{
						err= "No permission to set the scheduling policy and parameters "
									"specified in thread attributes object (attr).";
					}else
						bset= false;
					break;
				case ESRCH: // pthread_join / pthread_detach
					if(	error.adderror == "pthread_join" ||
						error.adderror == "pthread_detach"	)
					{
						err= "No thread with the getting ID could be found.";

					}else if(	error.adderror == "pthread_setschedparam" ||
								error.adderror == "pthread_getschedparam"	)
					{
						err= "No thread with the given ID could be found.";

					}else
						bset= false;
					break;
				case ENOTSUP:
					if(error.adderror == "pthread_setschedparam")
					{
						err= "attempt was made to set the  policy  or  scheduling  "
										"parameters  to  an  unsupported  value";
					}else
						bset= false;
					break;
				default:
					bset= false;
					break;
				}
				if(!bset)
				{
					if(error.type == specific_error)
						error.type= errno_error;
					else
						error.type= errno_warning;
					err= "do not found specific error/warning description "
									"for function " + error.adderror +
									" inside class " + error.classname;
				}
			}
			sRv= BaseErrorHandling::getErrorDescriptionString(error);
			if(sRv == "") // when no correct description string found, return null string
				return "";// because then define BaseErrorHandling an message for not found
			if(err != "")
				sRv+= "\n" + err;
			return sRv;
		}

	} /* namespace thread */
} /* namespace util */
