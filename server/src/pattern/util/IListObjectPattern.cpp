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

#include "IListObjectPattern.h"


namespace design_pattern_world
{
	namespace util_pattern
	{
		timeval ppi_time::ntime= { 0, 0 };

		ppi_time& ppi_time::operator = (const timeval& time)
		{
			tv_sec= time.tv_sec;
			tv_usec= time.tv_usec;
			return *this;
		}

		bool ppi_time::operator == (const ppi_time& time)
		{
			if(!timercmp(this, &time, !=))
				return true;
			return false;
		}

		bool ppi_time::operator != (const ppi_time& time)
		{
			if(timercmp(this, &time, !=))
				return true;
			return false;
		}

		bool ppi_time::operator < (const ppi_time& time)
		{
			if(timercmp(this, &time, <))
				return true;
			return false;
		}

		bool ppi_time::operator > (const ppi_time& time)
		{
			if(timercmp(this, &time, >))
				return true;
			return false;
		}

		bool ppi_time::operator <= (const ppi_time& time)
		{
			if(!timercmp(this, &time, >))
				return true;
			return false;
		}

		bool ppi_time::operator >= (const ppi_time& time)
		{
			if(!timercmp(this, &time, <))
				return true;
			return false;
		}

		ppi_time ppi_time::operator + (const ppi_time& time)
		{
			ppi_time tmRv;

			timeradd(this, &time, &tmRv);
			return tmRv;
		}

		ppi_time& ppi_time::operator += (const ppi_time& time)
		{
			timeradd(this, &time, this);
			return *this;
		}

		ppi_time ppi_time::operator - (const ppi_time& time)
		{
			ppi_time tmRv;

			timersub(this, &time, &tmRv);
			return tmRv;
		}

		ppi_time& ppi_time::operator -= (const ppi_time& time)
		{
			timersub(this, &time, this);
			return *this;
		}

		bool ppi_time::setActTime()
		{
			if(gettimeofday(this, NULL))
				return false;
			return true;
		}

		bool ppi_time::isSet() const
		{
			if(timerisset(this))
				return true;
			return false;
		}

		void ppi_time::clear()
		{
			timerclear(this);
		}

		string ppi_time::toString(const bool& bDate) const
		{
			char stime[21];
			struct tm ttime;
			string::size_type nLen;
			ostringstream stream, sRv;

			if(bDate)
			{
				if(localtime_r(&tv_sec, &ttime) != NULL)
				{
					strftime(stime, 20, "%d.%m.%Y %H:%M:%S", &ttime);
					sRv << stime << " ";
				}else
					sRv << "xx.xx.xxxx xx:xx:xx  ";
			}else
				sRv << fixed << tv_sec << ".";

			stream << fixed << tv_usec;
			nLen= stream.str().length();
			for(string::size_type o= 6; o > nLen; --o)
				sRv << "0";
			sRv << stream.str();
			return sRv.str();
		}
	}
}

