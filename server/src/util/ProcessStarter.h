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
	 * 			E2BIG  The  number  of  bytes  used  by  the new process image’s argument list and environment list is greater than the system-imposed limit of {ARG_MAX} bytes.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			6
	 * 		</td>
	 * 		<td>
	 * 			EACCES Search permission is denied for a directory listed in the new process image file’s path prefix, or the new process  image  file  denies  execution
	 *				   permission, or the new process image file is not a regular file and the implementation does not support execution of files of its type.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			7
	 * 		</td>
	 * 		<td>
	 * 			EINVAL The new process image file has the appropriate permission and has a recognized executable binary format, but the system does not support execution
     *			         of a file with this format.
	 *
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			8
	 * 		</td>
	 * 		<td>
	 * 			ELOOP  A loop exists in symbolic links encountered during resolution of the path or file argument.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			9
	 * 		</td>
	 * 		<td>
	 * 			ENAMETOOLONG
     *         The length of the path or file arguments exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			10
	 * 		</td>
	 * 		<td>
	 * 			ENOENT A component of path or file does not name an existing file or path or file is an empty string.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			11
	 * 		</td>
	 * 			ENOTDIR
     *         A component of the new process image file’s path prefix is not a directory.
	 * 		<td>
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			12
	 * 		</td>
	 * 		<td>
	 * 			ENOEXEC
     *         The new process image file has the appropriate access permission but has an unrecognized format.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			13
	 * 		</td>
	 * 		<td>
	 * 			ENOMEM The new process image requires more memory than is allowed by the hardware or system-imposed memory management constraints.
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			14
	 * 		</td>
	 * 		<td>
	 * 			ETXTBSY
     *         The new process image file is a pure procedure (shared text) file that is currently open for writing by some process.
	 *
	 * 		</td>
	 * 	</tr>
	 * 	<tr>
	 * 		<td>
	 * 			20
	 * 		</td>
	 * 		<td>
	 * 			undefined error by start an extern application with execv()
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
	class ProcessStarter: public util::Process
	{
		public:
			/**
			 * creating instance to start external process.<br />
			 * By cancel this ProcessStarter object, third parameter object will be also delete in parent class Process.
			 *
			 * @param ownProcess Name of process to identify in by server
			 * qparam toClient Name of Client to which connect
			 * @param sendConnection on which connection from outside the server to answer is reachable
			 * @param wait whether the starting method should wait for <code>init()</code> method
			 */
			ProcessStarter(const string& ownProcess, const string& toClient, IClientConnectArtPattern* sendConnection, const bool wait= true)
			:	Process(ownProcess, toClient, sendConnection, NULL, wait)
			{};
			/**
			 * start external application in an fork with execev()
			 *
			 * @param file name of starting application with hole path
			 * @param arg1 the first parameter ...
			 * @return error code if not 0
			 */
			int start(const char* file, ...);
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error);
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
			 * @return error code lower than 0
			 */
			virtual int runprocess(void *args= NULL, bool bHold= false);

		private:
			/**
			 * name of external application
			 */
			string m_sApp;
			/**
			 * ellipse parameters of start method
			 */
			char **m_ppEllipse;
			/**
			 * not reachable dummy method to start an process inside application
			 *
			 * @param args arbitary optional defined parameter to get in initialisation method init
			 * @param bHold should the caller wait of thread by ending.<br />
			 * 				default is false
			 * @return error code lower than 0
			 */
			virtual int start(void *args= NULL, bool bHold= false) { return 0; };
			/**
			 * dummy method to initial process.<br />
			 * Is not needed for starter
			 *
			 * @param args user defined parameter value or array,<br />
			 * 				comming as void pointer from the external call
			 * 				method start(void *args).
			 * @return always 0
			 */
			virtual int init(void *args) { return 0; };
			/**
			 * dummy method for looping process.<br />
			 * Is not needed for starter
			 *
			 * @return always 0
			 */
			virtual int execute() { return 0; };
			/**
			 * dummy method for ending process.<br />
			 * Is not needed for starter
			 */
			virtual void ending() {};
	};

}

#endif /* PROCESSSTARTER_H_ */
