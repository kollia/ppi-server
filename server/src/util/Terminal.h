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
#include <sstream>

#include <pthread.h>

#include "smart_ptr.h"
#include "thread/Thread.h"

using namespace std;

/**
 * teminal class to display strings
 * for separate threads on screen
 *
 * @author Alexander Kolli
 * @version 1.0
 */
class Terminal
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
	 * @return string stream for writing by next pass or call end() method
	 */
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
	SHAREDPTR::shared_ptr<ostringstream> out();
#else
	SHAREDPTR::shared_ptr<ostringstream> out(string file, int line);
#endif
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
	 * @return whether the thread was registered before
	 */
	bool isRegistered();
	/**
	 * define end of field, where can write out also content of other threads
	 */
#ifndef TERMINALOUTPUT_ONLY_THRADIDS
	void end();
#else
	void end(string file, int line);
#endif
	/**
	 * deleting of object
	 */
	static void deleteObj();

protected:
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
	pid_t s;
	pid_t b;
	/**
	 * single instance of Terminal
	 */
	static Terminal* _instance;
	/**
	 * queue of thread id's, ordered by inserting strings
	 */
	vector<pid_t> m_qOrder;
	/**
	 * all string blocks which are marked as end
	 */
	//vector<pid_t> m_vEnding;
	/**
	 * map of inserted strings in process for different threads
	 */
	map<pid_t, vector<SHAREDPTR::shared_ptr<ostringstream> > > m_mqRunStrings;
	/**
	 * map of finished inserted strings for different threads, wait for output
	 */
	queue<pair<pid_t, string> > m_qStrings;
	/**
	 * mutex to fill strings into map container
	 */
	pthread_mutex_t* m_PRINT;

	/**
	 * private constructor to create an singelton-pattern
	 */
	Terminal()
	: s(0), b(0), m_PRINT(Thread::getMutex("PRINT"))
	{ };
	/**
	 * destructor
	 */
	virtual ~Terminal()
	{ DESTROYMUTEX(m_PRINT); };
};

#ifndef TERMINALOUTPUT_ONLY_THRADIDS
#define tout *Terminal::instance()->out()
#define TERMINALEND Terminal::instance()->end()
#else
#define tout *Terminal::instance()->out(__FILE__, __LINE__)
#define TERMINALEND Terminal::instance()->end(__FILE__, __LINE__)
#endif

#endif /*TERMINAL_H_*/
