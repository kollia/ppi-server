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
	 * @param threadName name of template class
	 */
	CallbackTemplate()
	: Thread("callbackRoutine", 0)
	{};
	/**
	 * constructor for callback template
	 *
	 * @param threadName name of template class
	 */
	CallbackTemplate(string threadName)
	: Thread(threadName, 0)
	{};
	/**
	 * starting callback routine inside an new thread
	 */
	void initialStarting();
	/**
	 * for asking whether callback routine is finished
	 *
	 * @return 1 when callback is finished correctly, 0 when callback running, -1 when callback ending with warnings and -2 ending with errors
	 */
	short finished();
	/**
	 * dummy destructor
	 */
	virtual ~CallbackTemplate() {};

protected:
	/**
	 * abstract method running in thread.<br />
	 * This method starting again when method ending with return 0
	 * and stopping by all other values.<br />
	 * By calling external method finished()
	 * method gives back the return code.<br />
	 * In the most case the should be 1 for finished correctly, -1 finished with warnings
	 * or -2 with errors.
	 *
	 * @return defined error code from extended class
	 */
	virtual short runnable()=0;

private:
	/**
	 * return code of runnable method
	 */
	short m_nReturnCode;

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
	virtual int init(void *args) { return 0; };
	/**
	 * method to running thread
	 * in the extended class.<br />
	 * This method starting again when ending without an sleeptime
	 * if the method stop() isn't call.
	 *
	 * @return defined error code from extended class
	 */
	virtual int execute();
	/**
	 * abstract method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending() {};
};

#endif /* CALLBACKTEMPLATE_H_ */
