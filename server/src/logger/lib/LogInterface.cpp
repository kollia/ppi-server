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

#include "../../util/thread/Thread.h"
#include "../../util/stream/OMethodStringStream.h"

using namespace util;

namespace logger
{
	LogInterface* LogInterface::_instance= NULL;

	LogInterface::LogInterface(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait)
	:	ProcessInterfaceTemplate(process, "LogInterface", "LogServer", connection, NULL),
		m_nOpen(0),
		m_nTimeLogWait(identifwait),
		m_funcUsable(NULL),
		m_poChecker(NULL)
	{
		m_WRITELOOP= Thread::getMutex("WRITELOOP");
	}

	bool LogInterface::initial(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait/*= true*/)
	{
		if(_instance == NULL)
		{
			_instance= new LogInterface(process, connection, identifwait, wait);
		}
		return true;
	}

	void LogInterface::deleteObj()
	{
		if(_instance)
		{
			if(_instance->m_funcUsable)
				_instance->m_funcUsable(false);
			if(_instance->m_poChecker)
			{
				_instance->m_poChecker->stop();
				_instance->m_poChecker= NULL;
			}
			delete _instance;
			_instance= NULL;
		}
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

	int LogInterface::openSendConnection(string toopen/*= ""*/)
	{
		int nRv;

		LOCK(m_WRITELOOP);
		if(m_nOpen == 2)
		{
			UNLOCK(m_WRITELOOP);
			return 0;
		}
		m_nOpen= 1;
		UNLOCK(m_WRITELOOP);
		nRv= ExternClientInputTemplate::openSendConnection(toopen);
		LOCK(m_WRITELOOP);
		if(nRv <= 0)
			m_nOpen= 2;
		else
			m_nOpen= 0;
		UNLOCK(m_WRITELOOP);
		return nRv;
	}

	bool LogInterface::openedConnection()
	{
		bool bRv;

		LOCK(m_WRITELOOP);
		switch(m_nOpen)
		{
		case 0:
			int err;

			m_nOpen= 1;
			UNLOCK(m_WRITELOOP);
			err= ExternClientInputTemplate::openSendConnection();
			LOCK(m_WRITELOOP);
			if(err > 0)
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

	void LogInterface::setThreadName(string threadName)
	{
		threadNames tName;

		tName.name= threadName;
		tName.thread= pthread_self();
		if(!openedConnection())
		{
			LOCK(m_WRITELOOP);
			m_vtThreads.push_back(tName);
			UNLOCK(m_WRITELOOP);
		}else
			writethread(tName);
	}

	string LogInterface::getThreadName(pthread_t threadID/*= 0*/)
	{
		OMethodStringStream command("getThreadName");

		if(!openedConnection())
			return "no connection to logging client";
		command << threadID;
		return sendMethod("LogServer", command, true);

	}

	void LogInterface::callback(void *(*usable)(bool))
	{
		m_funcUsable= usable;
	}

	void LogInterface::writethread(const threadNames& thread)
	{
		OMethodStringStream command("setThreadName");

		command << thread.name;
		command << thread.thread;
		sendMethod("LogServer", command, false);
	}

	void LogInterface::log(string file, int line, int type, string message, string sTimeLogIdentif/*= ""*/)
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
		sendMethod("LogServer", command, false);
	}
}
