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

#include <map>
#include <vector>
#include <sstream>

#include <pthread.h>

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
	static Terminal* instnace();
	/**
	 * input operator
	 *
	 * @param str value to display on screen
	 */
	void operator << (const ostream& str);
	/**
	 * create an new line like endl by cout
	 * but do not flush the output string
	 */
	void newline();
	/**
	 * destructor
	 */
	virtual ~Terminal();

private:
	/**
	 * single instance of Terminal
	 */
	static Terminal* _instance;
	/**
	 * map of autput strings for different threads
	 */
	map<pid_t, vector<string> > m_mvStrings;
	/**
	 * mutex to fill strings into map container
	 */
	pthread_mutex_t* m_PRINT;

	/**
	 * private constructor to create an singelton-pattern
	 */
	Terminal();
};

#endif /*TERMINAL_H_*/
