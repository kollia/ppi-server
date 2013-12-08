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

#include "LogHolderPattern.h"

LogHolderPattern* LogHolderPattern::_instance= NULL;

void LogHolderPattern::init(const int loglevel)
{
	if(LogHolderPattern::_instance == NULL)
		LogHolderPattern::_instance= new LogHolderPattern;
	LogHolderPattern::_instance->m_nLogLevel= loglevel & 0x007; // & 00000111
}

void LogHolderPattern::init(ILogPattern* loggingobject)
{
	if(LogHolderPattern::_instance)
		delete LogHolderPattern::_instance;
	LogHolderPattern::_instance= new LogHolderPattern;
	LogHolderPattern::_instance->m_oLogging= loggingobject;
	LogHolderPattern::_instance->m_bExists= true;
	loggingobject->callback(LogHolderPattern::usable);
}

void LogHolderPattern::log(const string& file, int line, int type, const string& message,
				const string& sTimeLogIdentif/*= ""*/, IClientSendMethods* sendDevice/*= NULL*/)
{
	if(m_bExists)
	{
		m_oLogging->log(file, line, type, message, sTimeLogIdentif, sendDevice);
		return;
	}

	type= type & 0x007; // & 00000111
	if(type < m_nLogLevel)
		return;
	switch(type)
	{
		case LOG_DEBUG:
			cout << "DEBUG info";
			break;
		case LOG_INFO:
			cout << "INFO";
			break;
		case LOG_WARNING:
			cout << "WARNING";
			break;
		case LOG_ERROR:
			cout << "ERROR";
			break;
		case LOG_ALERT:
			cout << "ALERT error";
			break;
		default:
			cout << "UNKNOWN logmessage";
			break;
	}
	cout << ": " << message << endl;
}


