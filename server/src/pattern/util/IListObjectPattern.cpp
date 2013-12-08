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

		bool ppi_time::isSet()
		{
			if(timerisset(this))
				return true;
			return false;
		}

		void ppi_time::clear()
		{
			timerclear(this);
		}
	}
}

