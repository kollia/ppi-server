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

#ifndef LOG_DEBUG
#define LOG_DEBUG 0x000 // 00000000
#endif // LOG_DEBUG
#ifndef LOG_INFO
#define LOG_INFO 0x001 // 00000001
#endif // LOG_INFO
#ifndef LOG_WARNING
#define LOG_WARNING 0x002 // 00000010
#endif // LOG_WARNING
#ifndef LOG_ERROR
#define LOG_ERROR 0x003 // 00000011
#endif // LOG_ERROR
#ifndef LOG_ALERT
#define LOG_ALERT 0x004 // 00000100
#endif // LOG_ALERT

#ifndef LOG_SERVERDEBUG
#define LOG_SERVERDEBUG 0x008 // 00001000
#endif // LOG_SERVERDEBUG
#ifndef LOG_SERVERINFO
#define LOG_SERVERINFO 0x009 // 00001001
#endif // LOG_SERVERINFO
#ifndef LOG_SERVERWARNING
#define LOG_SERVERWARNING 0x00A // 00001010
#endif // LOG_SERVERWARNING
#ifndef LOG_SERVERERROR
#define LOG_SERVERERROR 0x00B // 00001011
#endif // LOG_SERVERERROR
#ifndef LOG_SERVERALERT
#define LOG_SERVERALERT 0x00C // 00001100
#endif // LOG_SERVERALERT

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
