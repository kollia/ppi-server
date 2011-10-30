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

#include "Calendar.h"

#include <iostream>

#include "../pattern/util/LogHolderPattern.h"

namespace util {

	time_t Calendar::calcDate(const bool newer, const time_t acttime, const int more, const time_e& spez, struct tm* ttime/*= NULL*/)
	{
		int d;
		struct tm* actStruct;
		time_t nRv;

		if(more < 0)
			return acttime;
		if(ttime == NULL)
			actStruct= new tm();
		else
			actStruct= ttime;
		if(localtime_r(&acttime, actStruct) == NULL)
		{
			TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
			return -1;
		}
		// no daylight saving time
		actStruct->tm_isdst= -1;
		switch(spez)
		{
		case seconds:
			if(newer)
				actStruct->tm_sec+= more;
			else
				actStruct->tm_sec-= more;
			break;

		case minutes:
			if(newer)
				actStruct->tm_min+= more;
			else
				actStruct->tm_min-= more;
			actStruct->tm_sec= 0;
			break;

		case hours:
			if(newer)
				actStruct->tm_hour+= more;
			else
				actStruct->tm_hour-= more;
			actStruct->tm_sec= 0;
			actStruct->tm_min= 0;
			break;

		case days:
			if(newer)
				actStruct->tm_mday+= more;
			else
				actStruct->tm_mday-= more;
			actStruct->tm_sec= 0;
			actStruct->tm_min= 0;
			actStruct->tm_hour= 0;
			actStruct->tm_wday= -1;
			actStruct->tm_yday= -1;
			break;

		case weeks:
			d= actStruct->tm_wday;
			if(d == 0)
				d= 7;
			--d;
			if(newer)
				actStruct->tm_mday= (actStruct->tm_mday - d + 7) + (7 * (more - 1));
			else
				actStruct->tm_mday= (actStruct->tm_mday - d + 7) - (7 * (more + 1));
			actStruct->tm_sec= 0;
			actStruct->tm_min= 0;
			actStruct->tm_hour= 0;
			actStruct->tm_wday= -1;
			actStruct->tm_yday= -1;
			break;

		case months:
			if(newer)
				actStruct->tm_mon+= more;
			else
				actStruct->tm_mon-= more;
			actStruct->tm_sec= 0;
			actStruct->tm_min= 0;
			actStruct->tm_hour= 0;
			actStruct->tm_wday= -1;
			actStruct->tm_mday= 1;
			actStruct->tm_yday= -1;
			break;

		case years:
			if(newer)
				actStruct->tm_year+= more;
			else
				actStruct->tm_year-= more;
			actStruct->tm_sec= 0;
			actStruct->tm_min= 0;
			actStruct->tm_hour= 0;
			actStruct->tm_wday= -1;
			actStruct->tm_mday= 1;
			actStruct->tm_yday= -1;
			actStruct->tm_mon= 0;
			break;
		}
		nRv= mktime(actStruct);
#if 0
		char stime[18];
		strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&acttime));
		std::cout << "actual time: " << stime << std::endl;
		std::cout << "next time ";
		if(newer)
			std::cout << "in ";
		else
			std::cout << "before ";
		std::cout << more << " ";
		switch(spez)
		{
		case seconds:
			std::cout << "seconds";
			break;
		case minutes:
			std::cout << "minutes";
			break;
		case hours:
			std::cout << "hours";
			break;
		case days:
			std::cout << "days";
			break;
		case weeks:
			std::cout << "weeks";
			break;
		case months:
			std::cout << "months";
			break;
		case years:
			std::cout << "years";
			break;
		}
		std::cout << " is" << std::endl;
		strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&nRv));
		std::cout << "             " << stime << " >> " << asctime(localtime(&nRv));
		std::cout << "change to values:" << std::endl;
		std::cout << "year        :" << actStruct->tm_year << std::endl;
		std::cout << "month       :" << actStruct->tm_mon << std::endl;
		std::cout << "day of year :" << actStruct->tm_yday << std::endl;
		std::cout << "day of month:" << actStruct->tm_mday << std::endl;
		std::cout << "day of week :" << actStruct->tm_wday << std::endl;
		std::cout << "hour        :" << actStruct->tm_hour << std::endl;
		std::cout << "minute      :" << actStruct->tm_min << std::endl;
		std::cout << "second      :" << actStruct->tm_sec << std::endl;
		std::cout << "daylight sav:" << actStruct->tm_isdst << std::endl;
		std::cout << "---------------------------------------------" << std::endl << std::endl;
#endif
		if(ttime == NULL)
			delete actStruct;
		return nRv;
	}

	time_t Calendar::setDate(const int year, const int month, const int day, const int hour/*= 0*/, const int min/*= 0*/, const int sec/*= 0*/)
	{
		struct tm time;

		time.tm_year= year - 1900;
		time.tm_mon= month -1;
		time.tm_mday= day;
		time.tm_yday= -1;
		time.tm_wday= -1;
		time.tm_hour= hour;
		time.tm_min= min;
		time.tm_sec= sec;
		time.tm_isdst= -1;
		return mktime(&time);
	}

}
