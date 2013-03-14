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

#include <boost/algorithm/string/replace.hpp>

#include "OParameterStringStream.h"

using namespace boost::algorithm;

namespace util {

string OParameterStringStream::str() const
{
	return m_sStream.str();
}

OParameterStringStream* OParameterStringStream::operator << (const OParameterStringStream& value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value.m_sStream;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const OParameterStringStream* value) {
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value->m_sStream;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const bool value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << boolalpha << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const bool* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << boolalpha << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const short value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned short value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const short* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned short* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const int value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned int value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const int* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned int* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const long value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned long value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const long* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const unsigned long* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const double value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	m_sStream << value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const double* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
		m_sStream << "NULL";
	else
		m_sStream << *value;
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (const char* const value)
{
	string sValue(value);

	return *this << sValue;
}

OParameterStringStream* OParameterStringStream::operator << (string value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	replace_all(value, "\\", "\\\\");
	replace_all(value, "\n", "\\n");
	replace_all(value, "\"", "\\\"");
	m_sStream << "\"" << value << "\" ";
	return this;
}

OParameterStringStream* OParameterStringStream::operator << (string* value)
{
	if(!m_sStream.eof())
		m_sStream << " ";
	if(value == NULL)
	{
		m_sStream << "NULL";
		return this;
	}
	replace_all(*value, "\n", "\\n");
	replace_all(*value, "\"", "\\\"");
	replace_all(*value, "\\", "\\\\");
	m_sStream << "\"" << *value << "\" ";
	return this;
}

OParameterStringStream::~OParameterStringStream() {}

}// namespace util
