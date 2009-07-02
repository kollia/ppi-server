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

namespace thread
{
	using namespace std;

	/**
	 * base class for all Process with fork
	 *
	 * @autor Alexander Kolli
	 * @version 1.0.0
	 */
	class Process
	{
	public:
		/**
		 * creating instance of one process
		 *
		 * @param processName Name of process to identify in logmessages
		 * @param user user id in which the process should running. By 0 (default) process running as calling user
		 * @param if flag is true (default), starting thread waiting until this thread initial whith method init()
		 */
		Process(const string processName, uid_t user= 0, bool waitInit= true);
		/**
		 * method returning name of process
		 *
		 * @return name of process
		 */
		string getName();
		/**
		 * start method to running the process paralell
		 *
		 * @param args arbitary optional defined parameter to get in initialisation method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 */
		virtual void *start(void *args= NULL, bool bHold= false);
		/**
		 * to ask whether the process should stopping.<br />
		 * This method should be call into running process to know whether the thread should stop.
		 *
		 * @return true if the thread should stop
		 */
		bool stopping();
		/**
		 * external query whether the thread is running
		 *
		 * @return true if thread is running
		 */
		bool running();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		//virtual void *stop(bool bWait= true);
		/**
		 * destructor of process
		 */
		virtual ~Process();

	protected:
		/**
		 * abstract method to initial the process
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return boolean whether the method execute can start
		 */
//		virtual bool init(void *args)=0;
		/**
		 * abstract method to running process
		 * in the extended class.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 */
//		virtual void execute()=0;
		/**
		 * abstract method to ending the process.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
//		virtual void ending()=0;
		/**
		 * this method defining the name whitch be showen in the log-files.
		 *
		 * @param threadName name which should be showen
		 */
		void setProcessLogName(string threadName);

	};

}

#endif /*PROCESS_H_*/
