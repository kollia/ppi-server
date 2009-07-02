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

BaseException::BaseException(char* heading, char* error)
{
	size_t nHeadLen= strlen(heading);
	m_sErrorHead= new char[nHeadLen+1];
	strcpy(m_sErrorHead, heading);
	size_t nTextLen= strlen(error);
	m_sErrorText= new char[nTextLen+1];
	strcpy(m_sErrorText, error);
}

BaseException::~BaseException()
{
	delete m_sErrorHead;
	delete m_sErrorText;
}

const char* BaseException::getHeading()
{
	size_t nLen= 9;
	char *sErrorString;

	nLen+= strlen(m_sErrorHead);
	sErrorString= new char[nLen];
	strcpy(sErrorString, "ERROR: ");
	strcat(sErrorString, m_sErrorHead);
	return sErrorString;
}

const char* BaseException::getErrorText()
{
	int nLen= 30;
	char *sErrorString;

	nLen+= strlen(m_sErrorText);
	sErrorString= new char[nLen];
	strcpy(sErrorString, m_sErrorText);
	return sErrorString;
}

const char* BaseException::getMessage()
{
	int nLen= 30;
	char *sErrorString;
	const char *sHeading= getHeading();
	const char *sError= getErrorText();

	nLen+= strlen(sHeading);
	nLen+= strlen(sError);
	sErrorString= new char[nLen];
	strcpy(sErrorString, sHeading);
	strcat(sErrorString, "\n");
	strcat(sErrorString, sError);
	//delete sHeading;
	//delete sError;
	return sErrorString;
}

const char* PortException::getMessage()
{
	int nLen= 30;
	char *sErrorString;
	const char *sHeading= getHeading();
	const char *sError= getErrorText();

	nLen+= strlen(sHeading);
	nLen+= strlen(sError);
	sErrorString= new char[nLen];
	strcpy(sErrorString, sHeading);
	strcat(sErrorString, " by status of ");
	if(m_status==DEKLARATION)
		strcat(sErrorString, "DEKLARATION");
	else if(m_status==UNKNOWN)
		strcat(sErrorString, "UNKNOWN");
	strcat(sErrorString, "\n");
	strcat(sErrorString, sError);
	delete sHeading;
	delete sError;
	return sErrorString;
}
