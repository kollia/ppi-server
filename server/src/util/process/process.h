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
#ifndef PROCESS_H_
#define PROCESS_H_

#include <sys/types.h>
#include <unistd.h>
#include <string>

#include "../stream/ErrorHandling.h"

#include "../thread/StatusLogRoutine.h"
#include "../thread/Thread.h"

#include "../../server/libs/client/ExternClientInputTemplate.h"

#include "../../pattern/util/ithreadpattern.h"

namespace util
{
	using namespace std;

	/**
	 * base class for all Process with forking.
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class Process : 		public ExternClientInputTemplate,
					virtual public IThreadPattern,
							public StatusLogRoutine
	{
	public:
		/**
		 * creating instance of one process<br />
		 * By cancel this Process object, third and fourth parameter objects will be also delete in parent class.
		 *
		 * @param ownProcess Name of process to identify in by server
		 * qparam toClient Name of Client to which connect
		 * @param sendConnection on which connection from outside the server to answer is reachable
		 * @param getConnection on which connection from outside the server is reachable to get questions
		 * @param wait whether the starting method should wait for <code>init()</code> method
		 */
		Process(const string& ownProcess, const string& toClient, IClientConnectArtPattern* sendConnection,
									IClientConnectArtPattern* getConnection, const bool wait= true);
		/**
		 * start method to fork the process parallel
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 * @return object of error handling
		 */
		virtual EHObj start(void *args= NULL, bool bHold= false);
		/**
		 * check only whether application is running
		 *
		 * @return object of error handling
		 */
		virtual EHObj check();
		/**
		 * start method to running the process without fork
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @return object of error handling
		 */
		virtual EHObj run(void *args= NULL);
		/**
		 * abstract method to initial the thread
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		virtual EHObj init(void *args)=0;
		/**
		 * abstract method to running process
		 * in the extended class.<br />
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @return whether should start process again
		 */
		virtual bool execute()=0;
		/**
		 * external query whether the process is running.<br />
		 * This method should be call into running process to know whether the process should stop.<br />
		 * By calling from outside it ask over the server when connection given by constructor.
		 * If not given the return value is always false
		 *
		 * @return 1 when thread running otherwise 0 or -1 by error
		 */
		virtual short running();
		/**
		 *  external command to stop process
		 *
		 * @param bWait calling rutine should wait until the process is stopping
		 * @return error or warning number see overview
		 */
		virtual EHObj stop(const bool bWait= true);
		/**
		 * to ask whether the process should stopping.<br />
		 * This method should be call into running process to know whether the process should stop.<br />
		 * By calling from outside it ask over the server when connection given by constructor.
		 * If not given the return value is always 0
		 *
		 * @return 1 if the thread should stop otherwise 0 or -1 by error
		 */
		virtual short stopping();
		/**
		 * abstract method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending()=0;
   		/**
   		 * method returning name of process
   		 *
   		 * @return name of process
   		 */
		virtual const string getProcessName() const
		{ return m_sProcessName; };
		/**
		 * returning current error handling object
		 *
		 * @return object of error handling
		 */
		OVERWRITE EHObj getErrorHandlingObj() const
		{ return ExternClientInputTemplate::getErrorHandlingObj(); };
		/**
		 * destructor of process
		 */
		virtual ~Process();

	protected:
		/**
		 * starting the process loop
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 * @return object for error handling
		 */
		virtual EHObj runprocess(void *args= NULL, bool bHold= false);

	private:
		/**
		 * whether starting routine should wait for <code>init()</code> method
		 */
		const bool m_bWaitInit;
		/**
		 * id of process by running,
		 * else where from outside the id is every time 0
		 */
		pid_t m_nProcessID;
		/**
		 * name of process
		 */
		const string m_sProcessName;
		/**
		 * name of client to which sending commands
		 */
		const string m_sToClient;
		/**
		 * whether process is running
		 */
		bool m_bRun;
		/**
		 * whether process should stop in the next time
		 */
		bool m_bStop;
		/**
		 * return value for start
		 */
		void* m_pvStartRv;
		/**
		 * return value for stop
		 */
		void* m_pvStopRv;
		/**
		 * mutex lock whether process running
		 */
		pthread_mutex_t* m_PROCESSRUNNING;
		/**
		 * mutex lock whether process should stopping
		 */
		pthread_mutex_t* m_PROCESSSTOPPING;
		/**
		 * condition to waiting for end
		 */
		pthread_cond_t* m_PROCESSSTOPCOND;


	};

}

#endif /*PROCESS_H_*/
