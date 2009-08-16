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

#ifndef LOGSTRUCTURES_H_
#define LOGSTRUCTURES_H_

#include <string>

using namespace std;

#define LOG_DEBUG 0
#define LOG_INFO  1
#define LOG_SERVER 2
#define LOG_WARNING 3
#define LOG_ERROR 4
#define LOG_ALERT 5

struct log_t
{
	time_t tmnow;
	string file;
	int line;
	pthread_t thread;
	pid_t pid;
	pid_t tid;
	int type;
	string message;
	string identif;
};

struct timelog_t
{
	pthread_t thread;
	string identif;
	string file;
	int line;
	time_t tmold;
};

struct threadNames
{
	unsigned int count;
	pthread_t thread;
	string name;
};

#endif /* LOGSTRUCTURES_H_ */
