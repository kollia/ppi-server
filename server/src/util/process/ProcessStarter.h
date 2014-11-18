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

#ifndef PROCESSSTARTER_H_
#define PROCESSSTARTER_H_

#include <stdarg.h>

#include "process.h"

namespace util
{

	/**
	 * base class for all Process with fork.
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class ProcessStarter: public util::Process
	{
		public:
			/**
			 * creating instance to start external process.<br />
			 * This starts only an application with the command <code>start()</code>
			 * and gives no message back whether the running application have some problems.
			 */
			ProcessStarter(const bool show)
			: Process("nocommunicate", "nocommunicate", NULL, NULL, false),
			  m_bShowBinary(show)
			{};
			/**
			 * creating instance to start external process.<br />
			 * By cancel this ProcessStarter object, third parameter object will be also delete in parent class Process.
			 *
			 * @param ownProcess Name of process to identify in by server
			 * qparam toClient Name of Client to which connect
			 * @param sendConnection on which connection from outside the server to answer is reachable
			 * @param show whether should display starting binary on command line
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ProcessStarter(const string& ownProcess, const string& toClient, IClientConnectArtPattern* sendConnection, const bool show, const bool wait= true)
			: Process(ownProcess, toClient, sendConnection, NULL, wait),
			  m_bShowBinary(show)
			{};
			/**
			 * start external application in an fork with execev()
			 *
			 * @param file name of starting binary with hole path
			 * @param params vector of all parameter for binary
			 * @return object of error handling
			 */
			EHObj start(const string& file, const vector<string>& params);
			/**
			 * destructor of ProcessStarter
			 */
			virtual ~ProcessStarter() {};

		protected:
			/**
			 * starting the external application
			 *
			 * @param args arbitary optional defined parameter to get in initialisation method init
			 * @param bHold should the caller wait of thread by ending.<br />
			 * 				default is false
			 * @return object for error handling
			 */
			virtual EHObj runprocess(void *args= NULL, bool bHold= false);

		private:
			/**
			 * whether should display starting binary on command line
			 */
			bool m_bShowBinary;
			/**
			 * name of external application
			 */
			string m_sApp;
			/**
			 * vector of binary command parameters
			 */
			vector<string> m_vParams;
			/**
			 * not reachable dummy method to start an process inside application
			 *
			 * @param args arbitary optional defined parameter to get in initialisation method init
			 * @param bHold should the caller wait of thread by ending.<br />
			 * 				default is false
			 * @return always object for error handling with no error
			 */
			virtual EHObj start(void *args= NULL, bool bHold= false) { return m_pSocketError; };
			/**
			 * dummy method to initial process.<br />
			 * Is not needed for starter
			 *
			 * @param args user defined parameter value or array,<br />
			 * 				comming as void pointer from the external call
			 * 				method start(void *args).
			 * @return always object for error handling with no error
			 */
			virtual EHObj init(void *args) { return m_pSocketError; };
			/**
			 * dummy method for looping process.<br />
			 * Is not needed for starter
			 *
			 * @return always true
			 */
			virtual bool execute() { return true; };
			/**
			 * dummy method for ending process.<br />
			 * Is not needed for starter
			 */
			virtual void ending() {};
	};

}

#endif /* PROCESSSTARTER_H_ */
