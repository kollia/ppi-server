/**
 *   This file 'NoAnswerSender.h' is part of ppi-server.
 *   Created on: 02.11.2013
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

#ifndef DBFILLER_H_
#define DBFILLER_H_

#include <string>
#include <vector>

#include "../pattern/util/IPPIDatabasePattern.h"
#include "../pattern/util/IDbFillerPattern.h"
#include "../pattern/server/IClientSendMethods.h"

#include "../util/thread/Thread.h"

#include "DbFillerCache.h"

namespace util
{
	using namespace std;

	/**
	 * sending all questions which reach ExternClientInputTemplate
	 * and need no really answer
	 */
	class DbFiller : 	public Thread,
						public IDbFillerPattern
	{
	public:
		/**
		 * constructor of object
		 *
		 * @param threadName name of thread
		 */
		DbFiller(const string& threadName)
		: Thread("DbFillerThread_for_" + threadName, false, SCHED_BATCH, 0),
		  m_SENDQUEUELOCK(Thread::getMutex("SENDQUEUELOCK")),
		  m_SENDQUEUECONDITION(Thread::getCondition("SENDQUEUECONDITION")),
		  m_bHasContent(false),
		  m_oCache("DbFillerCache_for_" + threadName, m_SENDQUEUELOCK, &m_bHasContent),
		  m_nBeginSendingCount(1),
		  m_nSendingCount(m_nBeginSendingCount)
		{};
		/**
		 * return thread name of DbFiller
		 */
		virtual string getName()
		{ return getThreadName(); };
		/**
		 * send message to given server in constructor
		 * or write into queue when no answer be needed
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param answer whether client should wait for answer
		 * @return backward send return value from server if answer is true, elsewhere returning null string
		 */
		virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true)
		{ return m_oCache.sendMethod(toProcess, method, answer); };
		/**
		 * send message to given server in constructor
		 * or write into queue when no answer be needed
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param done on which getting string the answer should ending. Ending also when an ERROR or warning occurs
		 * @param answer whether client should wait for answer
		 * @return backward send return string vector from server if answer is true, elsewhere returning vector with no size
		 */
		virtual vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true)
		{ return m_oCache.sendMethod(toProcess, method, done, answer); };
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param value value which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
						double value, bool bNew= false)
		{ m_oCache.fillValue(folder, subroutine, identif, value, bNew); };
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param dvalues vector of more values which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
						const vector<double>& dvalues, bool bNew= false)
		{ m_oCache.fillValue(folder, subroutine, identif, dvalues, bNew); };
		/**
		 * fill debug session output from folder working list
		 * into database
		 *
		 * @param content structure of folder:subroutine data from debugging session
		 */
		OVERWRITE void fillDebugSession(const dbgSubroutineContent_t& content)
		{ m_oCache.fillDebugSession(content); }
		/**
		 * informing thread to send entries to database
		 */
		virtual void informDatabase();
		/**
		 * return filled content from cache<br />
		 * dummy method witch is'nt used,
		 * only used inside DbFillerCache
		 *
		 * @param dbQueue database queue from cache
		 * @param msgQueue message queue from cache
		 * @param debugQueue debug session output queue from cache
		 */
		virtual void getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
						SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
						SHAREDPTR::shared_ptr<debugSessionFolderMap>& debugQueue);
		/**
		 * sending database and message values from queue
		 * directly over interface to database-server
		 *
		 * @param dbQueue database queue from cache(es)
		 * @param msgQueue message queue from cache(es)
		 * @param debugQueue debug session output queue from cache(es)
		 */
		void sendDirect(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
						SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue,
						SHAREDPTR::shared_ptr<debugSessionFolderMap>& debugQueue);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return DbFiller::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * remove all content from DbFiller
		 * and stop thread when one running
		 *
		 * @return object of error handling
		 */
		OVERWRITE EHObj remove()
		{ bool bWait(true); return DbFiller::stop(&bWait); };
		/**
		 * destructor of object
		 */
		virtual ~DbFiller();

	protected:
		/**
		 * dummy initialization of thread
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		OVERWRITE EHObj init(void *args)
		{ m_oCache.isRunning(); return m_pError; };
		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return whether should start thread again
		 */
		OVERWRITE bool execute();
		/**
		 * dummy method to ending the thread
		 */
		virtual void ending()
		{};

	private:
		/**
		 * mutex lock for write sending messages
		 * into an queue which are no answer needed
		 */
		pthread_mutex_t* m_SENDQUEUELOCK;
		/**
		 * condition to wait for new sending messages
		 */
		pthread_cond_t* m_SENDQUEUECONDITION;
		/**
		 * whether cache has any content<br />
		 * has to be locked inside SENDQUEUELOCK
		 */
		bool m_bHasContent;
		/**
		 * cache of holding all database entries
		 * when DbFiller threads should running for every folder
		 */
		DbFillerCache m_oCache;
		/**
		 * begin sending count when object of DbFiller start
		 * or last passing was longer then 10 seconds
		 */
		const size_t m_nBeginSendingCount;
		/**
		 * sending count by send debug information
		 * by one pass of queue.<br />
		 * should be grow up when to much entries exist
		 */
		size_t m_nSendingCount;
		/**
		 * last sending time
		 * of debug session content
		 */
		ppi_time m_tLastDbgSend;

		/**
		 * cache running inside an DbFiller
		 * or used as more caches
		 */
		virtual void isRunning()
		{ /* dummy method, used only by DbFillerCache */ };
	};

} /* namespace util */
#endif /* DBFILLER_H_ */
