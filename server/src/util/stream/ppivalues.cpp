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

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "ppivalues.h"


IPPITimePattern& ppi_time::operator = (const IPPITimePattern& time)
{
	tv_sec= time.tv_sec;
	tv_usec= time.tv_usec;
	return *this;
}

IPPITimePattern& ppi_time::operator = (ppi_value value)
{
	tv_sec= static_cast<__time_t>(value);
	value-= tv_sec;
	value*= 1000000;
	tv_usec= static_cast<__suseconds_t>(value);
	return *this;
}

void ppi_time::operator >> (ppi_value& value) const
{
	value= static_cast<ppi_value>(tv_usec);
	value/= 1000000;
	value+= static_cast<ppi_value>(tv_sec);
}

IPPITimePattern& ppi_time::operator = (const ppi_time& time)
{
	/*
	 * implement also ppi_time object
	 * because when not
	 * gcc do not know which operator should taken
	 * take not for timeval ??? don't know why
	 */
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

	m_nErrno= 0;
	m_sError= "";
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
	m_sError= "";
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

bool ppi_time::read(const string& str, string format)
{
	struct tm ttm;
	const char* cstr(str.c_str());
	char* res;
	time_t tm_unix;
	string::size_type found;

	m_nErrno= 0;
	m_sError= "";
	found= format.find("%N");
	if(found != string::npos)
	{
		format= format.erase(found, found+1);
		boost::trim(format);
	}
	ttm.tm_isdst= -1;
	res= strptime(cstr, format.c_str(), &ttm);
	if(	res == NULL ||
		res == cstr		)// first character of string
	{
		m_nErrno= errno;
		if(m_nErrno == 0)
		{
			m_nErrno= -1;
			m_sError= "cannot convert date from string '" + str + "'";
		}
		clear();
		return false;
	}
	tm_unix= mktime(&ttm);
	if(tm_unix < 0)
	{
		m_nErrno= errno;
		if(m_nErrno == 0)
		{
			m_nErrno= -2;
			m_sError= "mktime() cannot make correct date from tm structure";
		}
		clear();
		return false;
	}
	tv_sec= tm_unix;
	if(*res != '\0')
	{
		istringstream instr;
		ostringstream outstr;
		__suseconds_t usec;
		string::size_type nLen, nCount(0);
		const string::size_type needDigs(6);

		while(	*res != '\0' &&
				!isdigit(*res)	)
		{
			++res;
		}
		instr.str(string(res));
		instr >> usec;
		if(usec == 0)
		{
			m_nErrno= -3;
			m_sError= "cannont read '<milliceconds[microseconds]>' after time string";
			return false;
		}
		outstr << usec;
		nLen= outstr.str().length();
		if(nLen < needDigs)
			nCount= needDigs - nLen;
		else if(nLen > needDigs)
			nCount= nLen - needDigs;
		for(string::size_type c= 0; c < nCount; ++c)
		{
			if(nLen < needDigs)
				usec*= 10;
			else
				usec/= 10;
		}
		tv_usec= usec;
	}
	return true;
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

    if(m_sError != "")
    {
    	sRv << "ERRNO[" << -1 << "]: ";
    	sRv << m_sError;

    }else
    {
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
				sRv << err;
				free(err);
			}else
				sRv << "cannot allocate enough character space for this ERROR description";
		}else
			sRv << "The value of ERRNO is not a valid error number.";
    }
	return sRv.str();
}

