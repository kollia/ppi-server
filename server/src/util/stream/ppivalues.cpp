/**
 *   This file 'ppivalues.cpp' is part of ppi-server.
 *   Created on: 02.01.2014
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

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <sstream>

#include "ppivalues.h"


IPPITimePattern& ppi_time::operator = (const IPPITimePattern& time)
{
	tv_sec= time.tv_sec;
	tv_usec= time.tv_usec;
	return *this;
}

IPPITimePattern& ppi_time::operator = (const timeval& time)
{
	tv_sec= time.tv_sec;
	tv_usec= time.tv_usec;
	return *this;
}

bool ppi_time::operator == (const timeval& time) const
{
	if(!timercmp(this, &time, !=))
		return true;
	return false;
}

bool ppi_time::operator != (const timeval& time) const
{
	if(timercmp(this, &time, !=))
		return true;
	return false;
}

bool ppi_time::operator < (const timeval& time) const
{
	if(timercmp(this, &time, <))
		return true;
	return false;
}

bool ppi_time::operator > (const timeval& time) const
{
	if(timercmp(this, &time, >))
		return true;
	return false;
}

bool ppi_time::operator <= (const timeval& time) const
{
	if(!timercmp(this, &time, >))
		return true;
	return false;
}

bool ppi_time::operator >= (const timeval& time) const
{
	if(!timercmp(this, &time, <))
		return true;
	return false;
}

timeval ppi_time::operator + (const timeval& time)
{
	timeval tmRv;

	timeradd(this, &time, &tmRv);
	return tmRv;
}

IPPITimePattern& ppi_time::operator += (const timeval& time)
{
	string ich;
	timeradd(this, &time, this);
	return *this;
}

timeval ppi_time::operator - (const timeval& time)
{
	timeval tmRv;

	timersub(this, &time, &tmRv);
	return tmRv;
}

IPPITimePattern& ppi_time::operator -= (const timeval& time)
{
	timersub(this, &time, this);
	return *this;
}

bool ppi_time::setActTime()
{
	bool bRv(true);

	if(gettimeofday(this, NULL))
	{
		m_nErrno= errno;
		clear();
		bRv= false;
	}
	return bRv;
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

	m_nErrno= 0;
	if(bDate)
	{
		if(localtime_r(&tv_sec, &ttime) != NULL)
		{
			strftime(stime, 20, "%d.%m.%Y %H:%M:%S", &ttime);
			sRv << stime << " ";
		}else
		{
			m_nErrno= errno;
			sRv << "xx.xx.xxxx xx:xx:xx  ";
		}
	}else
		sRv << fixed << tv_sec << ".";

	stream << fixed << tv_usec;
	nLen= stream.str().length();
	for(string::size_type o= 6; o > nLen; --o)
		sRv << "0";
	sRv << stream.str();
	return sRv.str();
}

int ppi_time::error() const
{
	return m_nErrno;
}

string ppi_time::errorStr() const
{
	ostringstream sRv;
    char *err;
    size_t size;

    size = 1024;
    err = static_cast<char*>(malloc(size));
    if (err != NULL)
    {
		while (strerror_r(m_nErrno, err, size) != err && errno == ERANGE && errno != EINVAL)
		{
			size *= 2;
			err = static_cast<char*>(realloc(err, size));
			if (err == NULL)
				break;
		}
    }
	sRv << "ERRNO[" << m_nErrno << "]: ";
	if(errno != EINVAL)
	{
		if(err != NULL)
		{
			sRv << *err;
			free(err);
		}else
			sRv << "cannot allocate enough character space for this ERROR description";
	}else
		sRv << "The value of ERRNO is not a valid error number.";
	return sRv.str();
}

