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

#include "../server/OutsideClientTransaction.h"
#include "../pattern/util/ithreadpattern.h"
#include "../pattern/server/IClientConnectArtPattern.h"

using namespace server;

namespace util
{
	using namespace std;
	using namespace design_pattern_world::util_pattern;
	using namespace design_pattern_world::server_pattern;

	/**
	 * base class for all Process with fork.<br />
	 * <br />
	 * error codes for <code>start()</code> and <code>stop()</code>:
	 * <table>
	 * 	<tr>
	 * 		<td>
	 * 			0 (EXIT_SUCCESS)
	 * 		</td>
	 * 		<td>
	 * 			no error occurred
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			1 (EXIT_FAILURE)
	 * 		</td>
	 * 		<td>
	 * 			undefined error occurred
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
							public StatusLogRoutine
	{
	public:
		/**
		 * creating instance of one process<br />
		 * object delete client connection by ending
		 *
		 * @param processName Name of process to identify in logmessages
		 * @param connection on which connection from outside the process is reachable
		 * @param wait whether the starting method should wait for <code>init()</code> method
		 */
		Process(const string& processName, IClientConnectArtPattern* connection= NULL, const bool wait= true);
		/**
		 * start method to running the process paralell
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 * @return error code lower than 0
		 */
		virtual int start(void *args= NULL, bool bHold= false);
		/**
		 * abstract method to initial the thread
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return boolean whether the method execute can start
		 */
		virtual bool init(void *args)=0;
		/**
		 * abstract method to running process
		 * in the extended class.<br />
		 * This method starting again when ending
		 * if the method stop() isn't called.
		 */
		virtual void execute()=0;
		/**
		 * external query whether the process is running.<br />
		 * This method should be call into running process to know whether the process should stop.<br />
		 * By calling from outside it ask over the server when connection given by constructor.
		 * If not given the return value is always false
		 *
		 * @return true if thread is running
		 */
		virtual bool running();
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
		virtual bool stopping();
		/**
		 * send message to given server in constructor
		 *
		 * @param toProcess for which process the method should be
		 * @param method name of method
		 * @param params all parameter in an string, splited with an colon
		 * @param hold whether the connection should ending after call
		 * @return backward send return value from server
		 */
		string sendMethod(const string& toProcess, const string& method, const string& params, const bool& hold= true);
   		/**
   		 * method returning name of process
   		 *
   		 * @return name of process
   		 */
		virtual const string getProcessName() const
		{ return m_sProcessName; };
		/**
		 * destructor of process
		 */
		virtual ~Process();

	private:
		/**
		 * whether starting routine should wait for <code>init()</code> method
		 */
		const bool m_bWaitInit;
		/**
		 * connection to process
		 */
		IClientConnectArtPattern* m_oConnect;
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
		 * transaction to server to send methods.<br />
		 * If not exist create an new one
		 */
		OutsideClientTransaction* m_pSendTransaction;
		/**
		 * transaction to server to get methods for questions from other processes
		 */
		OutsideClientTransaction* m_pGetTransaction;
		/**
		 * mutex lock whether process running
		 */
		pthread_mutex_t* m_PROCESSRUNNING;
		/**
		 * mutex lock whether process should stopping
		 */
		pthread_mutex_t* m_PROCESSSTOPPING;

		/**
		 * calculate the error code given back from server.<br />
		 * the return error codes from server should be ERROR or WARNING.
		 * If the given warnings are multiplied with -1 (become negative)
		 * and both will be by return of 10 count higher.
		 *
		 * @param input the return string from server
		 * @param err the calculated error code, elswhere 0
		 * @return whether the input string was an error or warning
		 */
		bool error(const string& input, int& err);

	};

}

#endif /*PROCESS_H_*/
