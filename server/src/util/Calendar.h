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

/*
 * Calendar.h
 *
 *  Created on: 05.09.2009
 *      Author: Alexander Kolli
 */

#ifndef CALENDAR_H_
#define CALENDAR_H_

#include <time.h>

namespace util {

	class Calendar
	{
	public:
		/**
		 * calcuĺating the next time in seconds always on begin for the next hour, day, week, month or year
		 *
		 * @param newer whether should be the time in the feature (true), or past (false)
		 * @param acttime actual time
		 * @param more how much hours, days .. and so on the next time be calculated
		 * @param spez specification whether variable more is for hour, day, week, ...
		 * @return seconds for the next calculated time
		 */
		static time_t calcDate(const bool newer, const time_t acttime, const int more, const char spez);
	};

}

#endif /* CALENDAR_H_ */
