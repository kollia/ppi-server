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

#ifndef PROCESSINTERFACETEMPLATE_H_
#define PROCESSINTERFACETEMPLATE_H_

/**
 * make debug check for
 * last writing and reading
 * when definition is 1
 * and also do not try to send
 * message which contain transaction error
 * again back into queue for sending
 * inside NoAnswerSender class
 * (this will be done that not the same
 *  error comes as loop )
 */
#define __DEBUGLASTREADWRITECHECK 0
/**
 * workaround by sending string over TCP connection
 * where inside the string 0 terminated characters be set
 * in this case client will be infor server
 * to send string again
 */
#define __WRONGSTRINGSEND_WORKAROUND 0
#if __WRONGSTRINGSEND_WORKAROUND
/**
 * when follow directive be defined
 * inside workaround will be displayed
 * wrong send string also on command line
 */
#define __DEBUGWRONGSTRING 1
#else // __WRONGSTRINGSEND_WORKAROUND
#define __DEBUGWRONGSTRING 0
#endif // __WRONGSTRINGSEND_WORKAROUND

#include <string>

#include "ExternClientInputTemplate.h"

using namespace std;

namespace util
{

	class ProcessInterfaceTemplate: public ExternClientInputTemplate
	{
		public:
			/**
			 * class extend from ExternClientinputTemplate and provide
			 * the <code>running()</code>, <code>stopping()</code> and <code>stop()</code> methods
			 * for sending to the running process of second parameter toProcess.<br/>
			 * By cancel this ProcessInterfaceTemplate object, 4. and 5. parameter will be also delete in parent object.
			 *
			 * @param process name of process to identify by server
			 * @param threadName name of thread in the process to identify by server
			 * @param toProcess name of process to which connect
			 * @param sendConnection on which connection from outside the server to answer is reachable
			 * @param getConnection on which connection from outside the server is reachable to get questions
			 */
			ProcessInterfaceTemplate(const string& process, const string&threadName, const string& toProcess,
						IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection)
			:	ExternClientInputTemplate(process, threadName, sendConnection, getConnection),
				m_sSendTo(toProcess)
				{};
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
			 * @return object of error handling
			 */
			OVERWRITE EHObj stop(const bool bWait= true);
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
			 * virtual destructor
			 */
			virtual ~ProcessInterfaceTemplate() {};
		private:
			/**
			 * to which running process the questions for running, stopping
			 * or the command stop should send
			 */
			const string m_sSendTo;
	};

}

#endif /* PROCESSINTERFACETEMPLATE_H_ */
