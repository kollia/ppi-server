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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <fstream>

#include <boost/algorithm/string/split.hpp>

#include "LogThread.h"

#include "../lib/DbInterface.h"

#include "../../util/URL.h"
#include "../../util/Calendar.h"

LogThread::LogThread(bool check, bool asServer)
:	Thread("LogThread", 0),
	m_pvtLogs(new vector<struct log_t>()),
	m_bAsServer(asServer),
	m_bIdentifCheck(check),
	m_nDeleteDays(0),
	m_nNextDeleteTime(0)
{
	m_READTHREADS= getMutex("READTHREADS");
	m_READLOGMESSAGES= getMutex("READLOGMESSAGES");
	m_READLOGMESSAGESCOND= getCondition("READLOGMESSAGESCOND");
}

int LogThread::init(void* arg)
{
	return 0;
}

int LogThread::start(void *args, bool bHold)
{
#ifndef SINGLETHREADING
	if(m_bAsServer)
		return Thread::start(args, bHold);
#endif // SINGLETHREADING

	return -11;
}

int LogThread::stop(bool bWait)
{
#ifndef SINGLETHREADING
	if(m_bAsServer)
	{
		int result;

		result= Thread::stop(/*wait*/false);
		stopping();
		LOCK(m_READLOGMESSAGES);
		AROUSE(m_READLOGMESSAGESCOND);
		UNLOCK(m_READLOGMESSAGES);
		if(bWait)
			return Thread::stop(/*wait*/true);
		return result;
	}
#endif // SINGLETHREADING

	return -12;
}

void LogThread::setProperties(string logFile, int minLogLevel, int logAllSec, int writeLogDays, const unsigned short nDeleteAfter)
{
	vector<string> splitVec;

	m_nMinLogLevel= minLogLevel;
	m_nTimeLogWait= logAllSec;
	m_tmWriteLogDays= (time_t)writeLogDays * 24 * 60 * 60;
	m_nDeleteDays= nDeleteAfter;

	if(logFile.substr(0, 1) == "/")
		m_sLogFilePath= "/";
	splitVec= boost::algorithm::split(splitVec, logFile, boost::algorithm::is_any_of("/"));
	for(vector<string>::iterator it= splitVec.begin(); it != (splitVec.end()-1); ++it)
		m_sLogFilePath+= *it + "/";
	m_sLogFilePrefix= *(splitVec.end()-1);
	setThreadName("LogThread");
	log(__FILE__, __LINE__, LOG_INFO, "Storage Log-files under " + m_sLogFilePath + " with prefix '" + m_sLogFilePrefix + "' and suffix '.log'");
}

bool LogThread::ownThread(string threadName, pid_t currentPid)
{
	string currentName= getThreadName(currentPid);

	if(threadName == currentName)
		return true;
	return false;
}

void LogThread::setThreadName(const string& threadName, const pthread_t threadID)
{
	unsigned int nCount= 0;
	unsigned int nSize;
	threadNames tThread;

	LOCK(m_READTHREADS);
	nSize= m_vtThreads.size();
	for(unsigned int n= 0; n<nSize; n++)
	{
		if(pthread_equal(m_vtThreads[n].thread, threadID))
		{
			m_vtThreads[n].name= threadName;
			UNLOCK(m_READTHREADS);
			return;
		}
		if(m_vtThreads[n].name == threadName)
			++nCount;
	}
	tThread.count= ++nCount;
	tThread.thread= threadID;
	tThread.name= threadName;
	m_vtThreads.push_back(tThread);
	UNLOCK(m_READTHREADS);
}

string LogThread::getThreadName(const pthread_t threadID/*= 0*/)
{
	string sThreadName("");
	unsigned int nSize;
	char caWord[40];
	pthread_t thread;

	if(threadID == 0)
		thread= pthread_self();
	else
		thread= threadID;
	LOCK(m_READTHREADS);
	nSize= m_vtThreads.size();
	for(unsigned int n= 0; n < nSize; n++)
	{
		if(pthread_equal(m_vtThreads[n].thread, thread))
		{
			sprintf(caWord, "%d", m_vtThreads[n].count);
			sThreadName= m_vtThreads[n].name;
			sThreadName+= "(" + string(caWord) + ")";
			break;
		}
	}
	UNLOCK(m_READTHREADS);
	if(sThreadName == "")
		sThreadName= "UNKNOWN";
	return sThreadName;
}

void LogThread::log(const string& file, const int line, const int type, const string& message, const string& sTimeLogIdentif/*= ""*/)
{
	struct log_t logMessage;

	time(&logMessage.tmnow);
	logMessage.file= file;
	logMessage.line= line;
	logMessage.type= type;
	logMessage.message= message;
	logMessage.thread= pthread_self();
	logMessage.pid= getpid();
	logMessage.tid= Thread::gettid();
	logMessage.identif= sTimeLogIdentif;

	log(logMessage);
}

inline void LogThread::log(const log_t& messageStruct)
{
	if(!running())
		return;
	LOCK(m_READLOGMESSAGES);
	m_pvtLogs->push_back(messageStruct);
	AROUSE(m_READLOGMESSAGESCOND);
	UNLOCK(m_READLOGMESSAGES);
	if(!m_bAsServer)
		execute();
}

auto_ptr<vector<log_t> > LogThread::getLogVector()
{
	int conderror= 0;
	auto_ptr<vector<struct log_t> > vtRv;

	do{
		sleepDefaultTime();
		LOCK(m_READLOGMESSAGES);
		if(m_pvtLogs->size() != 0)
		{
			vtRv= m_pvtLogs;
			m_pvtLogs= auto_ptr<vector<struct log_t> >(new vector<struct log_t>);
		}else
		{
			if(!stopping())
			{
				//cout << endl << "wait for next logging messages" << endl;
				conderror= CONDITION(m_READLOGMESSAGESCOND, m_READLOGMESSAGES);
			}
			//cout << "awake from waiting" << endl;
		}
		UNLOCK(m_READLOGMESSAGES);
		if(conderror)
			usleep(500000);
	}while(	vtRv.get() == NULL
			&&
			!stopping()	);
	//cout << "write " << vtRv->size() << " log messages" << endl;
	return vtRv;
}

int LogThread::execute()
{
	auto_ptr<vector<struct log_t> > pvLogVector;

	bool bOpenedLogF= false, bAnyW= false;
	pvLogVector= getLogVector();
	ofstream logfile;
	string sThreadName;
	unsigned int nSize;
	time_t timenow;

	if(pvLogVector.get() != NULL)
	{
		time(&timenow);
		if(	m_sCurrentLogFile == "" ||
			timenow >= (m_tmbegin + m_tmWriteLogDays)	)
		{
			struct tm tmtime;
			char timeString[17];

			localtime_r(&timenow, &tmtime);
			strftime(timeString, 16, "%Y%m%d.%H%M%S", &tmtime);
			tmtime.tm_sec= 0;
			tmtime.tm_min= 0;
			tmtime.tm_hour= 0;
			m_tmbegin= mktime(&tmtime);
			m_sCurrentLogFile= URL::addPath(m_sLogFilePath, m_sLogFilePrefix, /*always*/true);
			m_sCurrentLogFile+= timeString;
			m_sCurrentLogFile+= ".";
			sprintf(timeString, "%d", getpid());
			m_sCurrentLogFile+= timeString;
			m_sCurrentLogFile+=  ".log";
		}
		nSize= pvLogVector->size();
		for(unsigned int n= 0; n < nSize; n++)
		{
			struct log_t result= (*pvLogVector)[n];
			if(m_nMinLogLevel <= ((*pvLogVector)[n].type & 0x007 /*00000111*/))
			{
				bool bWrite= true;

				if(m_bIdentifCheck)
				{
					if((*pvLogVector)[n].identif != "")
					{
						bool bFound= false;
						unsigned int nTLSize= m_vtTimeLog.size();

						for(unsigned int count= 0; count < nTLSize; count++)
						{
							if(	(*pvLogVector)[n].thread == m_vtTimeLog[count].thread
								&&
								(*pvLogVector)[n].identif == m_vtTimeLog[count].identif
								&&
								(*pvLogVector)[n].file == m_vtTimeLog[count].file
								&&
								(*pvLogVector)[n].line == m_vtTimeLog[count].line	)
							{
								time_t newTime;

								bFound= true;
								newTime= m_vtTimeLog[count].tmold + (time_t)m_nTimeLogWait;
								if((*pvLogVector)[n].tmnow < newTime)
								{
									bWrite= false;
								}else
								{
									m_vtTimeLog[count].tmold= (*pvLogVector)[n].tmnow;
									bWrite= true;
								}
								break;
							}
						}
						if(!bFound)
						{
							timelog_t tOld;

							tOld.thread= (*pvLogVector)[n].thread;
							tOld.identif= (*pvLogVector)[n].identif;
							tOld.file= (*pvLogVector)[n].file;
							tOld.line= (*pvLogVector)[n].line;
							tOld.tmold= (*pvLogVector)[n].tmnow;
							m_vtTimeLog.push_back(tOld);
						}
					}
				}
				if(bWrite)
				{
					string datetime;
					tm tmnow;

					bAnyW= true;
					if(bOpenedLogF == false)
					{
						logfile.open(m_sCurrentLogFile.c_str(), ios::app);
						if(logfile.fail())
						{
							cerr << "### ERROR: cannot open file '" << m_sCurrentLogFile << "'" << endl;
							cerr << "           so cannot write any log file" << endl;
							cerr << "    ERRNO: " << strerror(errno) << endl;
							return 1;
						}
						bOpenedLogF= true;
					}
					logfile << "*****************************************************************************************" << endl;
					logfile << "*  ";
					switch((*pvLogVector)[n].type)
					{
						case LOG_DEBUG:
							logfile << "DEBUG info";
							break;
						case LOG_INFO:
							logfile << "INFO";
							break;
						case LOG_WARNING:
							logfile << "WARNING";
							break;
						case LOG_ERROR:
							logfile << "ERROR";
							break;
						case LOG_ALERT:
							logfile << "ALERT error";
							break;
						case LOG_SERVERDEBUG:
							logfile << "SERVER-DEBUG info";
							break;
						case LOG_SERVERINFO:
							logfile << "SERVER-INFO";
							break;
						case LOG_SERVERWARNING:
							logfile << "SERVER-WARNING";
							break;
						case LOG_SERVERERROR:
							logfile << "SERVER-ERROR";
							break;
						case LOG_SERVERALERT:
							logfile << "SERVER-ALERT";
							break;
						default:
							logfile << "UNKNOWN logmessage";
							break;
					}
					sThreadName= getThreadName((*pvLogVector)[n].thread);
					tmnow.tm_isdst= -1;
					if(localtime_r(&(*pvLogVector)[n].tmnow, &tmnow) != NULL)
					{
						char formattime[21];

						strftime(formattime, 20, "%d.%m.%Y %H:%M:%S", &tmnow);
						datetime= formattime;
					}else
						datetime= "(no correct time creation)";

					logfile << " level for thread " << sThreadName << " ID:" << dec << (*pvLogVector)[n].tid;
					logfile << " on process ID:" << dec << (*pvLogVector)[n].pid;
					logfile << "  " << datetime << endl;
					logfile << "*  file:" << (*pvLogVector)[n].file;
					logfile << " line:" << (*pvLogVector)[n].line << endl << endl;
					logfile << (*pvLogVector)[n].message << endl << endl << endl;
				}
			}
		}
		if(bOpenedLogF == true)
			logfile.close();
	}
	if(bAnyW)
	{
		time(&timenow);
		if(	m_nDeleteDays > 0 &&
			(	m_nNextDeleteTime == 0 ||
				m_nNextDeleteTime < timenow	)	)
		{
			struct stat fileStat;
			map<string, string>::size_type allcount, count(0);
			map<string, string> files;
			time_t older;
			string file;

			older= Calendar::calcDate(/*newer*/false, timenow, m_nDeleteDays, Calendar::days);
			files= URL::readDirectory(m_sLogFilePath, m_sLogFilePrefix, ".log");
			allcount= files.size() - static_cast<map<string, string>::size_type>(m_nDeleteDays);
			for(map<string, string>::iterator it= files.begin(); it != files.end(); ++it)
			{
				++count;			// when not enough log files be exist,
				if(count > allcount)// maybe days between no logging content,
					break;			// break before the deletion to have always the count of m_nDeleteDays log files
				file= URL::addPath(m_sLogFilePath, it->second, /*always*/true);
				//cout << "read file " << file << endl;
				if(stat(file.c_str(), &fileStat) != 0)
				{
					char cerrno[20];
					string error("   cannot read date of log file ");

					error+= file + "\n   ERRNO(";
					sprintf(cerrno, "%d", errno);
					error+= cerrno;
					error+= "): ";
					error+= strerror(errno);
					log(__FILE__, __LINE__, LOG_WARNING, error);
				}else
				{
					if(fileStat.st_mtime < older)
					{
						if(unlink(file.c_str()) < 0)
						{
							char cerrno[20];
							string error("   cannot delete log file ");

							error+= file + "\n   ERRNO(";
							sprintf(cerrno, "%d", errno);
							error+= cerrno;
							error+= "): ";
							error+= strerror(errno);
							log(__FILE__, __LINE__, LOG_ERROR, error);
						}
					}
				}
			}
			m_nNextDeleteTime= Calendar::calcDate(/*newer*/true, timenow, m_nDeleteDays, Calendar::days);
		}
	}
	return 0;
}

void LogThread::ending()
{
	execute();
}
