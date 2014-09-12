/**
 *   This file 'MeasureThreadCounter.h' is part of ppi-server.
 *   Created on: 05.02.2014
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

#ifndef MEASURETHREADCOUNTER_H_
#define MEASURETHREADCOUNTER_H_

#include "util/thread/CallbackTemplate.h"

/**
 * measure running folder threads
 * in an given time of seconds
 */
class MeasureThreadCounter : public CallbackTemplate
{
public:
	/**
	 * structure to set
	 * which client will be start counting
	 */
	struct who_t
	{
		string who;
		string folder;
		string subroutine;
		ppi_value value;
	};
	/**
	 * constructor to define seconds and whether should wait
	 * for some client action
	 *
	 * @param seconds how much seconds should wait for counting
	 * @param bClient whether thread should waiting for client action before begin counting
	 */
	MeasureThreadCounter(const int seconds, const bool bOrder, const bool bClient)
//					const SHAREDPTR::shared_ptr<measurefolder_t> pFolderStart)
	: CallbackTemplate("MeasureThreadCounter"),
	  m_CLIENTACTIONMUTEX(getMutex("CLIENTACTIONMUTEX")),
	  m_CLIENTACTIONCOND(getCondition("CLIENTACTIONCOND")),
	  m_nSeconds(seconds),
	  m_bShowOrder(bOrder),
	  m_bClientAction(bClient),
	  m_bClientActionDone(false),
	  m_bCounting(true)
//	  m_pFolderStart(pFolderStart)
	{};
	/**
	 * external action from outside
	 * when thread counter should wait for it
	 * to beginning
	 *
	 * @param folder for which folder activation be
	 * @param subroutine for which subroutine activation be
	 * @param value which value will be set
	 * @param from which client will be activate counting
	 */
	void clientAction(const string& folder, const string& subroutine, const ppi_value& value, const string& from);
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool bWait)
	{ return MeasureThreadCounter::stop(&bWait); };
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool *bWait= NULL);
	/**
	 * destructor to destroy mutex and condition
	 */
	~MeasureThreadCounter()
	{ 	DESTROYMUTEX(m_CLIENTACTIONMUTEX);
		DESTROYCOND(m_CLIENTACTIONCOND);	};

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
	virtual short runnable();
	/**
	 * define user readable second string
	 *
	 * @param seconds how many seconds should written inside string
	 * @param wait return value, how long should thread waiting for next output
	 * @return readable string
	 */
	string getTimeString(int seconds, int* wait= NULL);
	/**
	 * start thread run counting inside MeasureThread objects
	 */
	void beginCount();
	/**
	 * write info output which folder threads are running
	 * inside given time
	 *
	 * @param seconds how much seconds was running
	 */
	void outputCounting(int seconds);

private:
	/**
	 * mutex to wait for client action
	 */
	pthread_mutex_t* m_CLIENTACTIONMUTEX;
	/**
	 * condition to wait for client action
	 */
	pthread_cond_t* m_CLIENTACTIONCOND;
	/**
	 * in how much seconds
	 * thread should counting folder thread
	 */
	int m_nSeconds;
	/**
	 * whether result of counting should show
	 * in which order folders are starting
	 */
	bool m_bShowOrder;
	/**
	 * whether thread should waiting for client action
	 *  before begin counting
	 */
	bool m_bClientAction;
	/**
	 * whether any outside action be done
	 */
	bool m_bClientActionDone;
	/**
	 * whether object wait for counting.<br />
	 * Client action only will be set after MeasureThreadCounter
	 * has informed all MeasureThread's to count
	 */
	bool m_bCounting;
	/**
	 * which client start counting
	 */
	who_t m_tClient;
};

#endif /* MEASURETHREADCOUNTER_H_ */
