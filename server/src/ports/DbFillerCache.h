/**
 *   This file 'DbFillerCache.h' is part of ppi-server.
 *   Created on: 16.07.2014
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

#ifndef DBFILLERCACHE_H_
#define DBFILLERCACHE_H_

#include <string>

#include "../pattern/util/IPPIDatabasePattern.h"
#include "../pattern/util/IDbFillerPattern.h"

#include "../util/thread/Thread.h"

namespace util
{
	using namespace std;

	class DbFillerCache : public IDbFillerPattern
	{
	public:
		/**
		 * constructor to create cache
		 *
		 * @param name name of cache for folder
		 * @param dbFiller database filler object
		 * @param queueLock mutex for lock filling entries
		 * @param hasContent whether cache has any content
		 */
		DbFillerCache(const string name, pthread_mutex_t* queueLock, bool* hasContent)
		: m_sName(name),
		  m_bisRunn(false),
		  m_bCreateLock(false),
		  m_pHasContent(hasContent),
		  m_vsSendingQueue1(new vector<sendingInfo_t>()),
		  m_apmtValueEntrys1(new map<string, db_t>()),
		  m_dbInform(NULL),
		  m_SENDQUEUELOCK1(queueLock)
		{};
		/**
		 * constructor to create cache
		 *
		 * @param name name of cache for folder
		 * @param dbFiller database filler object
		 * @param queueLock mutex for lock filling entries
		 * @param hasContent whether cache has any content
		 */
		DbFillerCache(const string name, IDbFillerInformPattern* dbInform)
		: m_sName(name),
		  m_bisRunn(false),
		  m_bCreateLock(true),
		  m_pHasContent(NULL),
		  m_vsSendingQueue1(new vector<sendingInfo_t>()),
		  m_vsSendingQueue2(new vector<sendingInfo_t>()),
		  m_apmtValueEntrys1(new map<string, db_t>()),
		  m_apmtValueEntrys2(new map<string, db_t>()),
		  m_dbInform(dbInform),
		  m_SENDQUEUELOCK1(Thread::getMutex("SENDQUEUELOCK1")),
		  m_SENDQUEUELOCK2(Thread::getMutex("SENDQUEUELOCK2"))
		{};
		/**
		 * return thread name of DbFiller
		 */
		virtual string getName()
		{ return m_sName; };
		/**
		 * cache running inside an DbFiller
		 * or used as more caches
		 */
		virtual void isRunning()
		{ m_bisRunn= true; };
		/**
		 * send message to given server in constructor
		 * or write into queue when no answer be needed
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param answer whether client should wait for answer
		 * @return backward send return value from server if answer is true, elsewhere returning null string
		 */
		virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true);
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
		virtual vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true);
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
						double value, bool bNew= false);
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
						const vector<double>& dvalues, bool bNew= false);
		/**
		 * informing thread to send entries to database
		 */
		virtual void informDatabase();
		/**
		 * return filled content from cache
		 *
		 * @param dbQueue database queue from cache
		 * @param msgQueue message queue from cache
		 */
		virtual void getContent(SHAREDPTR::shared_ptr<map<string, db_t> >& dbQueue,
						SHAREDPTR::shared_ptr<vector<sendingInfo_t> >& msgQueue);
		/**
		 * remove all content from DbFiller
		 * and stop thread when one running
		 */
		virtual int remove();
		/**
		 * destructor
		 */
		virtual ~DbFillerCache()
		{
			if(m_bCreateLock)
			{
				Thread::DESTROYMUTEX(m_SENDQUEUELOCK1);
				Thread::DESTROYMUTEX(m_SENDQUEUELOCK2);
			}
		};

	private:
		/**
		 * name of cache for which folder
		 */
		const string m_sName;
		/**
		 * whether own thread is running<br />
		 * this variable will be checked only the first time
		 * and should be thread safe
		 */
		bool m_bisRunn;
		/**
		 * whether SENDQUELOCK coming from external (false),
		 * or created self (true)
		 */
		bool m_bCreateLock;
		/**
		 * whether cache has any content<br />
		 * has to be locked inside SENDQUEUELOCK
		 */
		bool* m_pHasContent;
		/**
		 * first queue of question methods which need no answer
		 */
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > m_vsSendingQueue1;
		/**
		 * second queue of question methods which need no answer
		 */
		SHAREDPTR::shared_ptr<vector<sendingInfo_t> > m_vsSendingQueue2;
		/**
		 * first queue of all values for database
		 */
		SHAREDPTR::shared_ptr<map<string, db_t> > m_apmtValueEntrys1;
		/**
		 * second queue of all values for database
		 */
		SHAREDPTR::shared_ptr<map<string, db_t> > m_apmtValueEntrys2;
		/**
		 * last answer from sending question
		 * which need no answer.<br />
		 * could be an error/warning message
		 */
		string m_sNoWaitError1;
		/**
		 * last answer from sending question
		 * which need no answer.<br />
		 * could be an error/warning message
		 */
		string m_sNoWaitError2;
		/**
		 * inform DbFillerFactory to write values inside database.<br />
		 * only used when cache created from factory
		 */
		IDbFillerInformPattern* m_dbInform;
		/**
		 * mutex lock for write sending messages
		 * into an queue which are no answer needed
		 */
		pthread_mutex_t* m_SENDQUEUELOCK1;
		/**
		 * mutex lock for write sending messages
		 * into an queue which are no answer needed
		 */
		pthread_mutex_t* m_SENDQUEUELOCK2;
	};

} /* namespace util */
#endif /* DBFILLERCACHE_H_ */
