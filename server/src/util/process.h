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

#include "StatusLogRoutine.h"
#include "Thread.h"

#include "../util/ExternClientInputTemplate.h"

#include "../pattern/util/ithreadpattern.h"
namespace util
{
	using namespace std;

	/**
	 * base class for all Process with fork.<br />
	 * <br />
	 * error codes:<br />
	 * (all codes over 0 be errors and under warnings)<br />
	 * <table>
	 * 	<tr>
	 * 		<td>
	 * 			0
	 * 		</td>
	 * 		<td>
	 * 			<table>
	 * 				<tr>
	 * 					<td>
	 * 						<code>start()</code> and<br />
	 * 						<code>stop()</code>
	 * 					</td>
	 * 					<td>
	 * 						no error occurred
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						<code>running()</code>
	 * 					</td>
	 * 					<td>
	 * 						process do not run
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						<code>stopping()</code>
	 * 					</td>
	 * 					<td>
	 * 						process run and do not stop in the next time
	 * 					</td>
	 * 				</tr>
	 * 			</table>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			1
	 * 		</td>
	 * 		<td>
	 * 			<table>
	 * 				<tr>
	 * 					<td>
	 * 						<code>running()</code>
	 * 					</td>
	 * 					<td>
	 * 						process is running
	 * 					</td>
	 * 				</tr>
	 * 				<tr>
	 * 					<td>
	 * 						<code>stopping()</code>
	 * 					</td>
	 * 					<td>
	 * 						process should stop in the next time
	 * 					</td>
	 * 				</tr>
	 * 			</table>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			2
	 * 		</td>
	 * 		<td>
	 * 			cannot fork process
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			3
	 * 		</td>
	 * 		<td>
	 * 			cannot correctly check initialization from new process,
	 * 			maybe connection was failed or server give back wrong answer (not 'done')
	 *
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			4
	 * 		</td>
	 * 		<td>
	 * 			cannot correctly check stopping from process,
	 * 			maybe connection was failed or server give back wrong answer (not 'done')
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			5
	 * 		</td>
	 * 		<td>
	 * 			ProcessStarter cannot start correctly application on harddisk
	 * 		</td>
	 * 	</tr>
	 * </table>
	 * <br />
	 * warning codes for <code>start()</code> and <code>stop()</code>:
	 * <table>
	 * 	<tr>
	 * 		<td>
	 * 			-1
	 * 		</td>
	 * 		<td>
	 * 			in constructor given no <code>IClientConnectArtPattern,
	 * 			so cannot check instruction
	 * 		</td>
	 * 	</tr>
	 * </table>
	 * <br />
	 * the return error codes from server should be ERROR or WARNING.
	 * If the given warnings are multiplied with -1 (become negative)
	 * and both will be by return of 10 count higher.
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class Process : virtual public IThreadPattern,
							public ExternClientInputTemplate,
							public StatusLogRoutine
	{
	public:
		/**
		 * creating instance of one process<br />
		 * object delete client connection by ending
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
		 * @return 0 for no error
		 */
		virtual int start(void *args= NULL, bool bHold= false);
		/**
		 * check only whether application is running
		 *
		 * @return 0 for process running
		 */
		virtual int check();
		/**
		 * start method to running the process without fork
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @return error code lower than 0
		 */
		virtual int run(void *args= NULL);
		/**
		 * abstract method to initial the thread
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return defined error code from extended class
		 */
		virtual int init(void *args)=0;
		/**
		 * abstract method to running process
		 * in the extended class.<br />
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @return defined error code from extended class
		 */
		virtual int execute()=0;
		/**
		 * external query whether the process is running.<br />
		 * This method should be call into running process to know whether the process should stop.<br />
		 * By calling from outside it ask over the server when connection given by constructor.
		 * If not given the return value is always false
		 *
		 * @return error or warning number see overview
		 */
		virtual int running();
		/**
		 *  external command to stop process
		 *
		 * @param bWait calling rutine should wait until the process is stopping
		 * @return error or warning number see overview
		 */
		virtual int stop(const bool bWait= true);
		/**
		 * to ask whether the process should stopping.<br />
		 * This method should be call into running process to know whether the process should stop.<br />
		 * By calling from outside it ask over the server when connection given by constructor.
		 * If not given the return value is always false
		 *
		 * @return error or warning number see overview
		 */
		virtual int stopping();
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
		 * return string describing error number
		 *
		 * @param error code number of error
		 * @return error string
		 */
		virtual string strerror(int error);
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
		 * @return error code lower than 0
		 */
		virtual int runprocess(void *args= NULL, bool bHold= false);

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
