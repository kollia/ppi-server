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
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <string.h>

#include "LogThread.h"

LogThread::LogThread(bool asServer) :
Thread("LogThread", 0)
{
	m_pvtLogs= new vector<struct log>;
	m_READTHREADS= getMutex("READTHREADS");
	m_READLOGMESSAGES= getMutex("READLOGMESSAGES");
	m_READLOGMESSAGESCOND= getCondition("READLOGMESSAGESCOND");
	m_bAsServer= asServer;
	time(&m_tmbegin);
}

bool LogThread::init(void* arg)
{
	return true;
}

LogThread *LogThread::instance(bool asServer)
{
	static LogThread *_instance= NULL;

	if(_instance == NULL)
	{
		_instance= new LogThread(asServer);
	}
	return _instance;
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

void LogThread::setProperties(string logFile, int minLogLevel, int logAllSec, int writeLogDays)
{
	LOG(LOG_INFO, "Storage Log-files " + logFile);
	m_sConfLogFile= logFile;
	m_nMinLogLevel= minLogLevel;
	m_nTimeLogWait= logAllSec;
	m_tmWriteLogDays= (time_t)writeLogDays * 24 * 60 * 60;
}

bool LogThread::ownThread(string threadName, pid_t currentPid)
{
	string currentName= getThreadName(currentPid);

	if(threadName == currentName)
		return true;
	return false;
}

void LogThread::setThreadName(string threadName)
{
	pthread_t thread= pthread_self();
	unsigned int nCount= 0;
	unsigned int nSize;
	threadNames tThread;

	LOCK(m_READTHREADS);
	nSize= m_vtThreads.size();
	for(unsigned int n= 0; n<nSize; n++)
	{
		if(pthread_equal(m_vtThreads[n].thread, thread))
		{
			m_vtThreads[n].name= threadName;
			UNLOCK(m_READTHREADS);
			return;
		}
		if(m_vtThreads[n].name == threadName)
			++nCount;
	}
	tThread.count= ++nCount;
	tThread.thread= thread;
	tThread.name= threadName;
	m_vtThreads.push_back(tThread);
	UNLOCK(m_READTHREADS);
}

string LogThread::getThreadName(pthread_t threadID)
{
	string sThreadName("");
	unsigned int nSize;
	char caWord[40];

	if(threadID == 0)
		threadID= pthread_self();
	LOCK(m_READTHREADS);
	nSize= m_vtThreads.size();
	for(unsigned int n= 0; n < nSize; n++)
	{
		if(pthread_equal(m_vtThreads[n].thread, threadID))
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

void LogThread::log(string file, int line, int type, string message, string sTimeLogIdentif)
{
	struct log logMessage;

	time(&logMessage.tmnow);
	logMessage.file= file;
	logMessage.line= line;
	logMessage.type= type;
	logMessage.message= message;
	logMessage.thread= pthread_self();
	logMessage.pid= getpid();
	logMessage.tid= Thread::gettid();
	logMessage.identif= sTimeLogIdentif;

/*#ifdef DEBUG
	if(sTimeLogIdentif == "")
		cout << message << endl;
#endif*/

	LOCK(m_READLOGMESSAGES);
	m_pvtLogs->push_back(logMessage);
	AROUSE(m_READLOGMESSAGESCOND);
	UNLOCK(m_READLOGMESSAGES);
	if(!m_bAsServer)
		execute();
}

vector<log> *LogThread::getLogVector()
{
	int conderror= 0;
	vector<struct log> *vtRv= NULL;

	do{
		sleepDefaultTime();
		LOCK(m_READLOGMESSAGES);
		if(m_pvtLogs->size() != 0)
		{
			vtRv= m_pvtLogs;
			m_pvtLogs= new vector<struct log>;
		}else
			conderror= CONDITION(m_READLOGMESSAGESCOND, m_READLOGMESSAGES);
		UNLOCK(m_READLOGMESSAGES);
		if(conderror)
			usleep(500000);
	}while(	vtRv == NULL
			&&
			!stopping()	);
	return vtRv;
}

void LogThread::execute()
{
	vector<struct log> *pvLogVector= getLogVector();

	ofstream logfile;
	string sThreadName;
	unsigned int nSize;
	time_t tmnow;

	if(pvLogVector != NULL)
	{
		time(&tmnow);
		if(	m_sCurrentLogFile == ""
			||
			tmnow >= (m_tmbegin + m_tmWriteLogDays)	)
		{
			char timeString[15];

			strftime(timeString, 11, "%Y-%m-%d", gmtime(&tmnow));
			m_sCurrentLogFile= m_sConfLogFile;
			m_sCurrentLogFile+= timeString;
			m_sCurrentLogFile+= ".";
			sprintf(timeString, "%d", getpid());
			m_sCurrentLogFile+= timeString;
			m_sCurrentLogFile+=  ".log";
		}
		logfile.open(m_sCurrentLogFile.c_str(), ios::app);
		if(logfile.fail())
		{
			cout << "### ERROR: cannot open file "<< m_sLogFile << endl;
			cout << "           so cannot write any logfile" << endl;
			cout << "    ERRNO: " << strerror(errno) << endl;
			delete pvLogVector;
			return;
		}
		nSize= pvLogVector->size();
		for(unsigned int n= 0; n < nSize; n++)
		{
			struct log result= (*pvLogVector)[n];
			if(m_nMinLogLevel <= (*pvLogVector)[n].type)
			{
				bool bWrite= true;

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
				if(bWrite)
				{
					char datetime[30];
					tm *tmnow;

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
						case LOG_SERVER:
							logfile << "SERVER";
							break;
						case LOG_ERROR:
							logfile << "ERROR";
							break;
						case LOG_ALERT:
							logfile << "ALERT error";
							break;
						default:
							logfile << "UNKNOWN logmessage";
							break;
					}
					sThreadName= getThreadName((*pvLogVector)[n].thread);
					tmnow= localtime(&(*pvLogVector)[n].tmnow);
					sprintf(datetime, "  %02d.%02d.%2d %02d:%02d:%02d", 	tmnow->tm_mday, tmnow->tm_mon,
																			tmnow->tm_year, tmnow->tm_hour,
																			tmnow->tm_min, tmnow->tm_sec	);
					logfile << " level for thread " << sThreadName << " ID:" << dec << (*pvLogVector)[n].tid;
					logfile << " on process ID:" << dec << (*pvLogVector)[n].pid;
					logfile << datetime << endl;
					logfile << "*  file:" << (*pvLogVector)[n].file;
					logfile << " line:" << (*pvLogVector)[n].line << endl << endl;
					logfile << (*pvLogVector)[n].message << endl << endl << endl;
				}
			}
		}
		logfile.close();
		delete pvLogVector;
	}
	if(!stopping())
		usleep(10000);
}

void LogThread::ending()
{
	execute();
	delete m_pvtLogs;
}
