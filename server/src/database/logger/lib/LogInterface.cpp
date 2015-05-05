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

#include "LogInterface.h"

#include "../../../util/thread/Thread.h"
#include "../../../util/stream/OMethodStringStream.h"

using namespace util;

namespace logger
{
	LogInterface::LogInterface(const string& toProcess, const int identifwait, const bool wait)
	:	m_sToProcess(toProcess),
	 	m_nOpen(0),
		m_nTimeLogWait(identifwait),
		m_funcUsable(NULL),
		m_poChecker(NULL)
	{
		m_WRITELOOP= Thread::getMutex("WRITELOOP");
		m_TIMECHECKMUTEX= Thread::getMutex("TIMECHECKMUTEX");
	}

	LogInterface::~LogInterface()
	{
		if(m_funcUsable)
			m_funcUsable(false);
		if(m_poChecker)
		{
			m_poChecker->stop(/*wait*/true);
			delete m_poChecker;
		}
		DESTROYMUTEX(m_WRITELOOP);
		DESTROYMUTEX(m_TIMECHECKMUTEX);
	}

	bool LogInterface::openedConnection()
	{
		bool bRv;
		EHObj err;

		LOCK(m_WRITELOOP);
		switch(m_nOpen)
		{
		case 0:
			m_nOpen= 1;
			UNLOCK(m_WRITELOOP);
			err= openConnection();
			LOCK(m_WRITELOOP);
			if(err->hasError())
			{
				bRv= false;
				m_nOpen= 0;
				if(m_poChecker == NULL)
				{
					m_poChecker= new LogConnectionChecker(this, m_WRITELOOP);
					UNLOCK(m_WRITELOOP); // unlock WRITELOOP because thread write LOG message
					m_poChecker->start();
					LOCK(m_WRITELOOP);
				}
			}else
			{
				bRv= true;
				m_nOpen= 2;
				writeVectors();
				if(m_poChecker)
				{
					m_poChecker->stop(/*wait*/true);
					delete m_poChecker;
					m_poChecker= NULL;
				}
			}
			break;

		case 1:
			bRv= false;
			break;

		case 2:
			bRv= true;
			break;
		}
		UNLOCK(m_WRITELOOP);
		return bRv;
	}

	bool LogInterface::writeVectors()
	{
		bool write= false;

		for(vector<threadNames>::iterator it= m_vtThreads.begin(); it != m_vtThreads.end(); ++it)
		{
			write= true;
			writethread(*it);
		}
		m_vtThreads.clear();
		for(vector<log_t>::iterator it= m_vtLogs.begin(); it != m_vtLogs.end(); ++it)
		{
			writelog(*it);
			write= true;
		}
		m_vtLogs.clear();
		return write;
	}

	void LogInterface::setThreadName(const string& threadName, IClientSendMethods* sendDevice/*= NULL*/)
	{
		threadNames tName;

		tName.name= threadName;
		tName.thread= pthread_self();
		if(sendDevice != NULL)
		{
			OMethodStringStream command("setThreadName");

			command << threadName;
			command << tName.thread;
			sendDevice->sendMethod(/*LogServer*/m_sToProcess, command, /*answer*/false);
			return;
		}
		if(!openedConnection())
		{
			LOCK(m_WRITELOOP);
			m_vtThreads.push_back(tName);
			UNLOCK(m_WRITELOOP);
		}else
			writethread(tName);
	}

	string LogInterface::getThreadName(const pthread_t threadID/*= 0*/, IClientSendMethods* sendDevice/*= NULL*/)
	{
		OMethodStringStream command("getThreadName");

		if(!openedConnection())
			return "no connection to logging client";
		command << threadID;
		if(sendDevice != NULL)
			return sendDevice->sendMethod(/*LogServer*/m_sToProcess, command, true);
		return sendMethod(/*LogServer*/m_sToProcess, command, true);

	}

	void LogInterface::callback(void (*usable)(bool))
	{
		m_funcUsable= usable;
	}

	void LogInterface::writethread(const threadNames& thread)
	{
		OMethodStringStream command("setThreadName");

		command << thread.name;
		command << thread.thread;
		sendMethod(/*LogServer*/m_sToProcess, command, false);
	}

	void LogInterface::log(const string& file, const int line, const int type, const string& message,
					const string& sTimeLogIdentif/*= ""*/, IClientSendMethods* sendDevice/*= NULL*/)
	{
		log_t log;

		log.file= file;
		log.line= line;
		log.type= type;
		log.message= message;
		log.identif= sTimeLogIdentif;
		log.thread= pthread_self();
		log.pid= getpid();
		log.tid= Thread::gettid();
		time(&log.tmnow);
		if(sendDevice != NULL)
		{
			OMethodStringStream command("log");

			command << file;
			command << line;
			command << type;
			command << message;
			command << log.pid;
			command << log.tid;
			command << log.thread;
			command << sTimeLogIdentif;
			command << log.tmnow;
			sendDevice->sendMethod(/*LogServer*/m_sToProcess, command, /*answer*/false);
			return;
		}
		if(!openedConnection())
		{
			LOCK(m_WRITELOOP);
			m_vtLogs.push_back(log);
			UNLOCK(m_WRITELOOP);
		}else
			writelog(log);
	}

	void LogInterface::writelog(const log_t& log)
	{
		bool bWrite= true;
		string message(log.message);
		OMethodStringStream command("log");

		if(log.identif != "")
		{
			bool bFound= false;
			unsigned int nTLSize= m_vtTimeLog.size();

			LOCK(m_TIMECHECKMUTEX);
			for(unsigned int count= 0; count < nTLSize; count++)
			{
				if(	log.thread == m_vtTimeLog[count].thread
					&&
					log.identif == m_vtTimeLog[count].identif
					&&
					log.file == m_vtTimeLog[count].file
					&&
					log.line == m_vtTimeLog[count].line	)
				{
					time_t newTime;

					bFound= true;
					newTime= m_vtTimeLog[count].tmold + (time_t)m_nTimeLogWait;
					if(log.tmnow < newTime)
					{
						bWrite= false;
					}else
					{
						m_vtTimeLog[count].tmold= log.tmnow;
						bWrite= true;
					}
					break;
				}
			}
			if(!bFound)
			{
				timelog_t tOld;

				tOld.thread= log.thread;
				tOld.identif= log.identif;
				tOld.file= log.file;
				tOld.line= log.line;
				tOld.tmold= log.tmnow;
				m_vtTimeLog.push_back(tOld);
			}
			UNLOCK(m_TIMECHECKMUTEX);
		}
		if(!bWrite)
			return;
		command << log.file;
		command << log.line;
		command << log.type;
		command << message;
		command << log.pid;
		command << log.tid;
		command << log.thread;
		command << log.identif;
		command << log.tmnow;
		sendMethod(/*LogServer*/m_sToProcess, command, false);
	}
}
