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
#include <string.h>
#include "exception.h"

BaseException::BaseException(string heading, string error)
{
	m_sErrorHead= heading;
	m_sErrorText= error;
}

BaseException::~BaseException()
{
}

const string BaseException::getHeading()
{
	string sErrorString;

	sErrorString= "ERROR: ";
	sErrorString+= m_sErrorHead;
	return sErrorString;
}

const string BaseException::getErrorText()
{
	return m_sErrorText;
}

const string BaseException::getMessage()
{
	string sErrorString;

	sErrorString= getHeading() + "\n" + m_sErrorText;
	return sErrorString;
}

const string PortException::getMessage()
{
	string sErrorString(getHeading());

	sErrorString+= " by status of ";
	if(m_status==DEKLARATION)
		sErrorString+= "DEKLARATION";
	else if(m_status==UNKNOWN)
		sErrorString+= "UNKNOWN";
	sErrorString+= "\n";
	sErrorString+= getErrorText();
	return sErrorString;
}
