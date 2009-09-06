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

namespace util {

	time_t Calendar::calcDate(const bool newer, const time_t acttime, const int more, const char spez)
	{
		struct tm ttime;
		time_t nRv;

		if(more < 1)
			return acttime;
		ttime= *localtime(&acttime);
		//ttime= *readt;
		ttime.tm_sec= 0;
		ttime.tm_min= 0;
		if(spez == 'h')
		{
			if(newer)
				ttime.tm_hour+= more;
			else
				ttime.tm_hour-= more;
		}else if(spez == 'D')
		{
			ttime.tm_hour= 0;
			if(newer)
				ttime.tm_mday+= more;
			else
				ttime.tm_mday-= more;
			ttime.tm_yday= 0;
		}else if(spez == 'W')
		{
			int d= ttime.tm_wday;

			if(d == 0)
				d= 7;
			--d;
			ttime.tm_hour= 0;
			if(newer)
				ttime.tm_mday= (ttime.tm_mday - d + 7) + (7 * (more - 1));
			else
				ttime.tm_mday= (ttime.tm_mday - d + 7) - (7 * (more + 1));

		}else if(spez == 'M')
		{
			if( newer
				||
				more == 1	)
			{
				ttime.tm_hour= 0;
			}else // and what is this? only for the past when more month are calculated?
				ttime.tm_hour= 1;
			ttime.tm_mday= 1; // toDo: is bug? first day by calculating day is 0? why?
			if(newer)
				ttime.tm_mon+= more;
			else
				ttime.tm_mon-= more;
		}else if(spez == 'Y')
		{
			ttime.tm_hour= 1; // toDo: is bug? first hour by calculating day or month is 0? why?
			ttime.tm_mday= 1; // toDo: is bug? first day by calculating day is 0? why?
			ttime.tm_mon= 0;
			if(newer)
				ttime.tm_year+= more;
			else
				ttime.tm_year-= more;
		}
		nRv= mktime(&ttime);
#if 0
		char stime[18];
		strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&acttime));
		cout << "actual time: " << stime << endl;
		strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&nRv));
		cout << "write next:  " << stime << endl << endl;
#endif
		return nRv;
	}

}
