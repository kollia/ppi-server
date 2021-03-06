/**
 *   This file 'CallbackTemplate.h' is part of ppi-server.
 *   Created on: 26.12.2011
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

#ifndef CALLBACKTEMPLATE_H_
#define CALLBACKTEMPLATE_H_

#include "Thread.h"

class CallbackTemplate : public Thread
{
public:
	/**
	 * constructor for callback template
	 *
	 * @param policy thread policy for scheduling
	 * @param priority new other scheduling priority for thread
	 * @param logger sending object over which logging should running
	 */
	CallbackTemplate(const int policy= -1, const int priority= -9999, IClientSendMethods* logger= NULL)
	: Thread("callbackRoutine", /*waitinit*/true, policy, priority, logger)
	{};
	/**
	 * constructor for callback template
	 *
	 * @param threadName name of template class
	 * @param policy thread policy for scheduling
	 * @param priority new other scheduling priority for thread
	 * @param logger sending object over which logging should running
	 */
	CallbackTemplate(string threadName, const int policy= -1, const int priority= -9999, IClientSendMethods* logger= NULL)
	: Thread(threadName, /*waitinit*/true, policy, priority, logger)
	{};
	/**
	 * starting callback routine inside an new thread
	 *
	 * @return error number when thread cannot be started, otherwise 0
	 */
	EHObj initialStarting();
	/**
	 * for asking whether callback routine is finished
	 *
	 * @param bWait whether method should wait for ending (default: false)
	 * @return 1 when callback is finished correctly, 0 when callback running,
	 *         -1 when callback ending with warnings and -2 ending with errors.<br />
	 *         by error/warning call <code>getErrorHandlingObj()</code> for more information
	 */
	short finished(bool bWait= false);
	/**
	 * dummy destructor
	 */
	virtual ~CallbackTemplate() {};

protected:
	/**
	 * abstract method running in thread.<br />
	 * This method starting again when method ending with true
	 * and stopping by false.
	 *
	 * @return whether thread should start again
	 */
	virtual bool runnable()=0;

private:
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
	OVERWRITE EHObj init(void *args) { return m_pError; };
	/**
	 * method to running thread
	 * in the extended class.<br />
	 * This method starting again when ending without an sleeptime
	 * if the method stop() isn't call.
	 *
	 * @return defined error code from extended class
	 */
	OVERWRITE bool execute();
	/**
	 * abstract method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() {};
};

#endif /* CALLBACKTEMPLATE_H_ */
