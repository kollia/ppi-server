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
#ifndef ISTATUSLOGPATTERN_H_
#define ISTATUSLOGPATTERN_H_

#include <sstream>

using namespace std;

namespace design_pattern_world
{
	/**
	 * structure of status information
	 * for any last position
	 */
	struct pos_t
	{
		/**
		 * object of IStatusLogPattern
		 * to call method
		 */
		class IStatusLogPattern* thread;
		/**
		 * name of thread
		 */
		string threadname;
		/**
		 * thread id
		 */
		pid_t tid;
		/**
		 * in which file the poisiton be set
		 */
		string file;
		/**
		 * on which line in the file the position set
		 */
		int line;
		/**
		 * any named position identification
		 */
		string identif;
		/**
		 * last time when position was reached
		 */
		time_t time;
		/**
		 * status string information as an string by initialization
		 */
		string info1;
		/**
		 * status information for position as an string
		 */
		string info2;
		/**
		 * status information as an integer, set by initialization
		 */
		int ninfo1;
		/**
		 * status information for postition as an integer
		 */
		int ninfo2;

		/**
		 * public constructor to initial member variables
		 */
		pos_t()
		{
			thread= NULL;
			tid= 0;
		}
	};

	/**
	 * pattern to initial status on threadx
	 *
	 * @author Alexander Kolli
	 * @version 1.0
	 */
	class IStatusLogPattern
	{
	public:
		/**
		 * returning status information for an thread
		 *
		 * @param params parameter set by call getStatusInfo from main method
		 * @param pos position struct see pos_t
		 * @param elapsed seconds be elapsed since last position time pos_t.time
		 * @param time from last position pos_t.time converted in an string
		 * @return status info
		 */
		virtual string getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)= 0;
		/**
		 * destructor of pattern
		 */
		virtual ~IStatusLogPattern() {};
	};
}  // namespace design_pattern_world

#endif /*ISTATUSLOGPATTERN_H_*/
