/**
 *   This file 'NoAnswerSender.h' is part of ppi-server.
 *   Created on: 24.11.2013
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

#ifndef INFORMER_H_
#define INFORMER_H_

#include <string>
#include <vector>

//#include "../pattern/util/IPPIDatabasePattern.h"

#include "../util/thread/Thread.h"

#include "../pattern/util/imeasurepattern.h"
#include "../pattern/util/IInformerCachePattern.h"

namespace util
{
	using namespace std;

	typedef map<IInformerCachePattern*, vector<string> > folders_t;

	/**
	 * inform own or other folders over an external pool thread
	 */
	class Informer : 	public Thread
	{
	public:
		struct inform_t
		{
			/**
			 * changed subroutine
			 */
			string ownSubroutine;
			/**
			 * from which folder:subroutine be changed
			 */
			string from;
			/**
			 * map of folders which should be informed
			 */
			SHAREDPTR::shared_ptr<folders_t> folders;
			/**
			 * whether subroutine <code>ownSubroutine</code> running
			 * in debug session
			 */
			bool debug;
			/**
			 * thread ID of subroutine which running in debug session
			 */
			pid_t threadID;
			/**
			 * mutex to set or remove observers
			 */
			pthread_mutex_t *OBSERVERLOCK;
		};
		/**
		 * constructor of object
		 *
		 * @param threadName name of thread
		 * @param objCaller class object of ExternClientInputTemplate which need this sender thread
		 */
		Informer(const string& threadName, IMeasurePattern* measureThread)
		: Thread("InformerThread_for_" + threadName, false, SCHED_BATCH, 0),
		  m_bisRunn(false),
		  m_poMeasurePattern(measureThread),
		  m_sFolder(threadName),
		  m_pOwnInformerCache(measureThread->getInformerCache(m_sFolder)),
		  m_apvtFolders(auto_ptr<vector<inform_t> >(new vector<inform_t>)),
		  m_INFORMQUEUELOCK(getMutex("INFORMQUEUELOCK")),
		  m_INFORMQUEUECONDITION(getCondition("INFORMQUEUECONDITION"))
		{};
		/**
		 * inform over pool other folders and also own when necessary
		 * that an specific subroutine was changed
		 *
		 * @param folders map of folders which should informed
		 * @param from which subroutine (other or own) changing value
		 * @param as from which subroutine be informed
		 * @param debug whether subroutine which inform folders, running in debug session
		 * @param lock locking mutex for observers
		 */
		void informFolders(const folders_t& folders, const string& from, const string& as,
													const bool debug, pthread_mutex_t *lock);
		/**
		 * arouse informer pool when running
		 * to inform all other folders and maybe own
		 * that any subroutine was changing
		 */
		void arouseInformerThread();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return Informer::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * destructor of object
		 */
		virtual ~Informer();

	protected:
		/**
		 * dummy initialization of thread
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return defined error code from extended class
		 */
		virtual int init(void *args)
		{ m_bisRunn= true; return 0; };
		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return defined error code from extended class
		 */
		virtual int execute();
		/**
		 * inform directly other folders and also own when necessary
		 * that an specific subroutine was changed
		 *
		 * @param folders map of folders which should informed
		 * @param from which subroutine (other or own) changing value
		 * @param as from which subroutine be informed
		 * @param threadId when subroutine which inform folders, running in debug session
		 * @param lock locking mutex for observers
		 */
		void informing(const folders_t& folders, const string& from,
						const string& as, const pid_t& threadId, pthread_mutex_t *lock);
		/**
		 * dummy method to ending the thread
		 */
		virtual void ending()
		{};

	private:
		/**
		 * whether own thread is running<br />
		 * this variable will be checked only the first time
		 * and should be thread safe
		 */
		bool m_bisRunn;
		/**
		 * measure thread of own folder
		 */
		IMeasurePattern* m_poMeasurePattern;
		/**
		 * name of folder in which Informer running
		 */
		string m_sFolder;
		/**
		 * informer cache for own folder thread
		 */
		IInformerCachePattern* m_pOwnInformerCache;
		/**
		 * queue of all values for database
		 */
		std::auto_ptr<vector<inform_t> > m_apvtFolders;
		/**
		 * last answer from sending question
		 * which need no answer.<br />
		 * could be an error/warning message
		 */
		string m_sNoWaitError;
		/**
		 * mutex lock for write sending messages
		 * into an queue which are no answer needed
		 */
		pthread_mutex_t* m_INFORMQUEUELOCK;
		/**
		 * condition to wait for new sending messages
		 */
		pthread_cond_t* m_INFORMQUEUECONDITION;
	};

} /* namespace ppi_database */
#endif /* INFORMER_H_ */
