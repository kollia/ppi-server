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
		 * structure of time inits
		 */
		enum time_e
		{
			seconds= 0,
			minutes,
			hours,
			days,
			weeks,
			months,
			years
		};
		/**
		 * Calculating the next time in seconds always on begin for the next hour, day, week, month or year
		 *
		 * @param newer whether should be the time in the feature (true), or past (false)
		 * @param acttime actual time
		 * @param more how much hours, days .. and so on the next time be calculated
		 * @param spez specification whether variable more is for hour, day, week, ...
		 * @return seconds for the next calculated time
		 */
		static time_t calcDate(const bool newer, const time_t acttime, const int more, const time_e& spez, struct tm* ttime= NULL);
		/**
		 * return time in seconds since epoch for setting date.<br />
		 * When any parameter has an negative value, it will be take the actual date
		 *
		 * @param year set time to this year
		 * @param month set time to this month
		 * @param day set time to this day
		 * @param hour set time to this hour
		 * @param min set time to this minute
		 * @param sec set time to this second
		 * @return time in seconds since epoch
		 */
		static time_t setDate(const int year, const int month, const int day, const int hour= 0, const int min= 0, const int sec= 0);
		/**
		 * add to actual time <code>from</code> one second
		 *
		 * @param from time from calculate next second
		 */
		static time_t nextSecond(time_t from)
		{ return from + 1; };
		/**
		 * add to actual time <code>from</code> seconds to the next minute
		 *
		 * @param from time from calculate next second
		 * @param ttime structor of local time. If not be set, method calculate self from from parameter
		 */
		static time_t nextMinute(time_t from, struct tm* ttime= NULL)
		{ return calcDate(/*newer*/true, from, 1, minutes, ttime); };
		/**
		 * add to actual time <code>from</code> seconds to the next hour
		 *
		 * @param from time from calculate next second
		 * @param ttime structor of local time. If not be set, method calculate self from from parameter
		 */
		static time_t nextHour(time_t from, struct tm* ttime= NULL)
		{ return calcDate(/*newer*/true, from, 1, hours, ttime); };
		/**
		 * add to actual time <code>from</code> seconds to the next day
		 *
		 * @param from time from calculate next second
		 * @param ttime structor of local time. If not be set, method calculate self from from parameter
		 */
		static time_t nextDay(time_t from, struct tm* ttime= NULL)
		{ return calcDate(/*newer*/true, from, 1, days, ttime); };
		/**
		 * add to actual time <code>from</code> seconds to the next Month
		 *
		 * @param from time from calculate next second
		 * @param ttime structor of local time. If not be set, method calculate self from from parameter
		 */
		static time_t nextMonth(time_t from, struct tm* ttime= NULL)
		{ return calcDate(/*newer*/true, from, 1, months, ttime); };
		/**
		 * add to actual time <code>from</code> seconds to the next Year
		 *
		 * @param from time from calculate next second
		 * @param ttime structor of local time. If not be set, method calculate self from from parameter
		 */
		static time_t nextYear(time_t from, struct tm* ttime= NULL)
		{ return calcDate(/*newer*/true, from, 1, years, ttime); };
	};

}

#endif /* CALENDAR_H_ */
