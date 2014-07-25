/**
 *   This file 'DbFillerFactory.h' is part of ppi-server.
 *   Created on: 15.07.2014
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

#ifndef DBFILLERFACTORY_H_
#define DBFILLERFACTORY_H_

#include <string>
#include <utility>
#include <map>

#include "../util/smart_ptr.h"
#include "../util/thread/Thread.h"

#include "../pattern/util/IDbFillerPattern.h"

#include "DbFiller.h"

namespace util
{
	class DbFillerFactory : public Thread,
							public IDbFillerInformPattern
	{
	public:
		/**
		 * create instance of DbFiller.<br />
		 *  DbFiller object for direct information when threads defined as -1,
		 *  to write directly into database.<br />
		 *  also DbFiller instance when threads defined as 0,
		 *  to write into database with an extra thread for all folders.<br />
		 *  DbFillerCache object when threads defined as 1,
		 *  to write into database with one extra thread for all folders.
		 *
		 * @param thredName name of folder instance
		 * @param threads instance creating object, described before
		 * @return DbFiller or DbFilllerCache depends on parameter thread
		 */
		static SHAREDPTR::shared_ptr<IDbFillerPattern> getInstance(const string& threadName, short threads);
		/**
		 * informing thread to send entries to database
		 */
		virtual void informDatabase();
		/**
		 * destructor to ending instance
		 */
		virtual ~DbFillerFactory()
		{
			DESTROYMUTEX(m_READCACHE);
			DESTROYMUTEX(m_INFORMDATABASEMUTEX);
			DESTROYCOND(m_INFORMDATABASECOND);
			__instance= NULL;
		}

	protected:
		/**
		 * abstract method to initial the thread
		 * in this extended class DbFillerFactory.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return defined error code from extended class
		 */
		virtual int init(void *args)
		{ return 0; };
		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending when method ending with return value 0
		 * and the method stop() isn't called.
		 *
		 * @return defined error code from extended class
		 */
		virtual int execute();
		/**
		 * abstract method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending()
		{ /* dummy method */ };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return DbFillerFactory::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);

	private:
		/**
		 * instance of DbFillerFactory
		 */
		static DbFillerFactory* __instance;
		/**
		 * DbFiller object to write over interface to database
		 */
		DbFiller m_oDbFiller;
		/**
		 * holder map of all caches
		 */
		map<string, SHAREDPTR::shared_ptr<IDbFillerPattern> > m_mCaches;
		/**
		 * mutex to create, read or remove caches
		 */
		pthread_mutex_t* m_READCACHE;
		/**
		 * mutex to lock database inform array
		 */
		pthread_mutex_t* m_INFORMDATABASEMUTEX;
		/**
		 * condition to inform thread
		 * to write into database
		 */
		pthread_cond_t* m_INFORMDATABASECOND;

		/**
		 * constructor to create instance
		 */
		DbFillerFactory()
		: Thread("DbFillerFactory", false, SCHED_BATCH, 0),
		  m_oDbFiller("DbFillerInstance"),
		  m_READCACHE(Thread::getMutex("READCACHE")),
		  m_INFORMDATABASEMUTEX(Thread::getMutex("INFORMDATABASEMUTEX")),
		  m_INFORMDATABASECOND(Thread::getCondition("INFORMDATABASE"))
		{};
		/**
		 * create cache for given folder
		 *
		 * @param sfor name of folder
		 */
			SHAREDPTR::shared_ptr<IDbFillerPattern> createCache(const string& sfor);
			/**
			 * cache running inside an DbFiller
			 * or used as more caches
			 */
			virtual void isRunning()
			{ /* dummy method, used only by DbFillerCache */ };
	};
} /* namespace util */

#endif /* DBFILLERFACTORY_H_ */
