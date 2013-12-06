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
#ifndef TERMINAL_H_
#define TERMINAL_H_

/*****************************************************
 * this definition of TERMINALOUTPUT_ONLY_THRADIDS
 * can be used to show only the thread id's
 * by beginning of an string block
 * to see whether all strings will be writing
 * by all passes
 */
//#define TERMINALOUTPUT_ONLY_THRADIDS

#include <map>
#include <queue>
#include <deque>
#include <sstream>

#include <pthread.h>

#include "../smart_ptr.h"
#include "Thread.h"

using namespace std;

typedef deque<pair< pid_t, SHAREDPTR::shared_ptr<vector<string> > > > finishedStrVec_temp;
typedef std::auto_ptr< deque<pair< pid_t, SHAREDPTR::shared_ptr<vector<string> > > > > finishedPtr_temp;
/**
 * teminal class to display strings
 * for separate threads on screen
 *
 * @author Alexander Kolli
 * @version 1.0
 */
class Terminal : public Thread
{
public:
	/**
	 * return singel instance of terminal
	 */
	static Terminal* instance();
	/**
	 * write prefix before each row.<br />
	 * As default this Terminal object writing thread id as prefix.<br />
	 * By overload this method you can write also an other prefix.
	 *
	 * @param pid thread id of writing block
	 * @return prefix string
	 */
	virtual string output_prefix(const pid_t& pid);
	/**
	 * get ostringstream object which should written on right time
	 *
	 * @param threadID which thread ID should be registered, otherwise when 0, actual thread ID will be allocated
	 * @return string stream for writing by next pass or call end() method
	 */
	SHAREDPTR::shared_ptr<ostringstream> out(pid_t threadID= 0);
	/**
	 * write this string between all string blocks.<br />
	 * As default this Terminal object writing an empty row.<br />
	 * By overload this method you can write also an other string or nothing.
	 *
	 * @param pid thread id of writing block
	 * @return string for writing between blocks (threads)
	 */
	virtual string str_between(const pid_t& pid);
	/**
	 * notify whether before thread was registered.<br />
	 * This can be used to know whether an thread is known in order.
	 * Elsewhere it have to show end of block with <code>TERMINALEND</code>
	 *
	 * @param threadID which thread ID should be registered, otherwise when 0, actual thread ID will be allocated
	 * @return whether the thread was registered before
	 */
	bool isRegistered(pid_t threadID= 0);
	/**
	 * define end of field, where can write out also content of other threads
	 *
	 * @param threadID which thread ID should be registered, otherwise when 0, actual thread ID will be allocated
	 */
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
	void end(pid_t threadID= 0);
#else
	void end(string file, int line, const pid_t& threadID= 0);
#endif
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool bWait)
	{ return Terminal::stop(&bWait); };
	/**
	 *  external command to stop thread
	 *
	 * @param bWait calling rutine should wait until the thread is stopping
	 */
	virtual int stop(const bool *bWait= NULL);
	/**
	 * deleting of object
	 */
	static void deleteObj();
	/**
	 * destructor
	 */
	virtual ~Terminal()
	{ DESTROYMUTEX(m_PRINT);
	  DESTROYCOND(m_WORKINGCONDITION); };

protected:
	/**
	 * abstract method to initial the thread
	 * in the extended class.<br />
	 * this method will be called before running
	 * the method execute
	 *
	 * @param args user defined parameter value or array,<br />
	 * 				comming as void pointer from the external call
	 * 				method start(void *args).
	 * @return defined error code
	 */
	virtual int init(void *args)
	{ /* nothing to do */ return 0; };
	/**
	 * abstract method to running thread
	 * in the extended class.<br />
	 * This method starting again when ending when method ending with return value 0
	 * and the method stop() isn't called.
	 *
	 * @return defined error code
	 */
	virtual int execute();
	/**
	 * abstract method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending();
	/**
	 * reading output string inside created second thread
	 *
	 * @param threadID actually thread which was writing
	 * @param msg message of writing
	 */
	void read(const pid_t& threadID, const string& msg);
	/**
	 * write direct on command line.<br />
	 * This method is also for overloading when string should shown also on other positions.<br />
	 * The end of writing an block is signaled when first parameter pid is 0.
	 *
	 * @param pid actually thread which was writing
	 * @param msg message of writing
	 */
	void write(const pid_t& pid, const string& msg);

private:
	/**
	 * single instance of Terminal
	 */
	static Terminal* _instance;
	/**
	 * instance to working
	 */
	std::auto_ptr<Terminal> m_pWorker;
	/**
	 * last read string
	 */
	map<pid_t, SHAREDPTR::shared_ptr<ostringstream> > m_sLastMsgStream;
	/**
	 * queue of thread id's, ordered by inserting strings
	 */
	vector<pid_t> m_qOrder;
	/**
	 * the main thread ID which will be output
	 */
	pid_t m_nActThreadID;
	/**
	 * map of inserted strings in process for different threads
	 */
	map<pid_t, SHAREDPTR::shared_ptr<vector<string> > > m_mqRunStrings;
	/**
	 * map of finished inserted strings for different threads, wait for output
	 */
	finishedPtr_temp m_pvFinishedStrings;
	/**
	 * mutex to fill strings into map container
	 */
	pthread_mutex_t* m_PRINT;
	/**
	 * condition to write output strings
	 */
	pthread_cond_t* m_WORKINGCONDITION;

	/**
	 * private constructor to create an singelton-pattern
	 */
	Terminal()
	: Thread("Output-Terminal"),
	  m_pWorker(std::auto_ptr<Terminal>()),
	  m_nActThreadID(0),
	  m_PRINT(Thread::getMutex("PRINT")),
	  m_WORKINGCONDITION(Thread::getCondition("WORKINGCONDITION"))
	{ };
};

#ifndef TERMINALOUTPUT_ONLY_THRADIDS
#define tout *Terminal::instance()->out()
#define TERMINALEND Terminal::instance()->end()
#else
#define tout *Terminal::instance()->out(__FILE__, __LINE__)
#define TERMINALEND Terminal::instance()->end(__FILE__, __LINE__)
#endif

#endif /*TERMINAL_H_*/
