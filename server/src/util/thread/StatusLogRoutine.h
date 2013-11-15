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
#ifndef STATUSLOGROUTINE_H_
#define STATUSLOGROUTINE_H_

#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>

#include "../debug.h"

#include "../../pattern/server/IClientSendMethods.h"
#include "../../pattern/util/ithreadpattern.h"
#include "../../pattern/util/IStatusLogPattern.h"

using namespace design_pattern_world;
using namespace design_pattern_world::util_pattern;
using namespace design_pattern_world::client_pattern;
using namespace std;

/**
 * base class to logging all actual status.
 *
 * @autor Alexander Kolli
 * @version 1.0.0
 */
class StatusLogRoutine : public virtual IStatusLogPattern
{
	public:
		/**
		 * constructor of StatusLogRoutine
		 */
		StatusLogRoutine();
		/**
		 * set last position for status information
		 *
		 * @param file in which file the poisiton be set
		 * @param line on which line in the file the position set
		 * @param identif any named position identification
		 */
		static void position(const string file, const int line, const string identif)
		{ positionA(file, line, identif, NULL, NULL); };
		/**
		 * set last position for status information
		 *
		 * @param file in which file the poisiton be set
		 * @param line on which line in the file the position set
		 * @param identif any named position identification
		 * @param info2 varios information as an string
		 */
		static void position(const string file, const int line, const string identif, const string info2)
		{ positionA(file, line, identif, &info2, NULL); };
		/**
		 * set last position for status information
		 *
		 * @param file in which file the poisiton be set
		 * @param line on which line in the file the position set
		 * @param identif any named position identification
		 * @param ninfo2 variaos information as an integer
		 */
		static void position(const string file, const int line, const string identif, const int ninfo2)
		{ positionA(file, line, identif, NULL, &ninfo2); };
		/**
		 * set last position for status information
		 *
		 * @param file in which file the poisiton be set
		 * @param line on which line in the file the position set
		 * @param identif any named position identification
		 * @param info2 varios information as an string
		 * @param ninfo2 variaos information as an integer
		 */
		static void position(const string file, const int line, const string identif, const string info2, const int ninfo2)
		{ positionA(file, line, identif, &info2, &ninfo2); };
		/**
		 * set last position for status information
		 *
		 * @param file in which file the poisiton be set
		 * @param line on which line in the file the position set
		 * @param identif any named position identification
		 * @param info2 varios information as an string
		 * @param ninfo2 variaos information as an integer
		 */
		static void positionA(const string file, const int line, const string identif, const string* info2, const int* ninfo2);
		/**
		 * first initialization from running thread
		 *
		 * @param threadName name of the thread
		 * @param thread object of an thread which contains the IStatusLogPattern
		 */
		void initstatus(const string threadName, IStatusLogPattern* thread);
		/**
		 * set status for information
		 *
		 * @param thread object of an thread which contains the IStatusLogPattern
		 */
		void statusattrib(IStatusLogPattern* thread)
		{ statusattrib(thread, NULL, NULL); };
		/**
		 * set status for information
		 *
		 * @param ninfo1 integer information
		 */
		void statusattrib(int ninfo1)
		{ statusattrib(NULL, NULL, &ninfo1); };
		/**
		 * set status for information
		 *
		 * @param info1 string information
		 */
		void statusattrib(string info1)
		{ statusattrib(NULL, &info1, NULL); };
		/**
		 * set status for information
		 *
		 * @param info1 string information
		 * @param ninfo1 integer information
		 */
		void statusattrib(string info1, int ninfo1)
		{ statusattrib(NULL, &info1, &ninfo1); };
		/**
		 * set status for information
		 *
		 * @param thread object of an thread which contains the IStatusLogPattern
		 * @param info1 string information
		 * @param ninfo1 integer information
		 */
		void statusattrib(IStatusLogPattern* thread, string* info1, int* ninfo1);
		/**
		 * get status information for last reached position with time
		 * for all running threads.<br />
		 * <br />
		 * On starting static method with no params, method returning only an number of running threads<br />
		 * elsewhere by any params it calls for all threads the defined virtual getStatusInfo(params, pos, ...)<br />
		 * if in the params be set 'threads:<system thread number pid_t>' this method calling only the virtual
		 * getStatusInfo for the given thread.
		 *
		 * @param params to show which status info
		 * @return status information
		 */
		static string getStatusInfo(string params);
		/**
		 * remove thread position from status list
		 *
		 * @param threadid id of thread which should be removed
		 */
		void removestatus(const pid_t threadid);
		/**
		 * destructor of class Thread
		 */
		virtual ~StatusLogRoutine();

	protected:
		/**
		 * protected initialization for given info points
		 * to write into the status information
		 *
		 * @param params parameter set by call getStatusInfo from main method
		 * @param pos position struct see pos_t
		 * @param elapsed seconds be elapsed since last position time pos_t.time
		 * @param time from last position pos_t.time converted in an string
		 */
		virtual string getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime);
		/**
		 * this method defining the name whitch be showen in the log-files.
		 *
		 * @param threadName name which should be showen
		 * @param sendDevice sending object over which logging should running
		 */
		void setThreadLogName(string threadName, IClientSendMethods* sendDevice);




	private:
		/**
		 * map of position information for all threads
		 */
		static map<pid_t, pos_t> m_mStatus;
		/**
		 * mutex lock for set or reading status
		 */
		static pthread_mutex_t* m_POSITIONSTATUS;
		/**
		 * private static initialization for given info points
		 * to write into the status information
		 *
		 * @param params parameter set by call getStatusInfo from main method
		 * @param pos position struct see pos_t
		 * @param elapsed seconds be elapsed since last position time pos_t.time
		 * @param time from last position pos_t.time converted in an string
		 */
		static string getStatus(string params, pos_t& pos, time_t elapsed, string lasttime);

};

#define POS(identif) StatusLogRoutine::position(__FILE__, __LINE__, identif)
#define POSS(identif, info2) StatusLogRoutine::position(__FILE__, __LINE__, identif, info2)
#define POSN(identif, ninfo2) StatusLogRoutine::position(__FILE__, __LINE__, identif, ninfo2)
#define POSSN(identif, info2, ninfo2) StatusLogRoutine::position(__FILE__, __LINE__, identif, info2, ninfo2)

#endif /*STATUSLOGROUTINE_H_*/
