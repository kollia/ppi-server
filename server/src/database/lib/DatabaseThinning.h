/**
 *   This file 'DatabaseThinning.h' is part of ppi-server.
 *   Created on: 21.04.2013
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

#ifndef DATABASETHINNING_H_
#define DATABASETHINNING_H_

#include "../../util/smart_ptr.h"
#include "../../util/Calendar.h"
#include "../../util/thread/Thread.h"

#include "../../pattern/util/IPPIDatabasePattern.h"
#include "../../pattern/util/IChipConfigReaderPattern.h"

namespace ppi_database
{
	/**
	 * structure for sorting timval keys in an map
	 */
	struct TimeSort
	{
	   bool operator()(const timeval x, const timeval y) const
	   {
			if(x.tv_sec < y.tv_sec) return true;
			if(x.tv_sec == y.tv_sec) return x.tv_usec < y.tv_usec;
			return false;
	   }
	};

	/**
	 * thinning database to hold memory space on hard disk closer
	 */
	class DatabaseThinning : public Thread
	{
	public:
		/**
		 * constructor to initial object
		 *
		 * @param sWorkDir directory path where database files are laying
		 * @param pChipReader DefaultChipConfigReader to define whether write value into new thinned database file
		 * @param nSleepAfter sleep time after every reading row to hold process performance lower
		 */
		DatabaseThinning(const string& sWorkDir, IChipConfigReaderPattern* pChipReader, __useconds_t nSleepAfter)
		: Thread("DatabaseThinning", false, SCHED_IDLE, 0),
		  m_THINNINGMUTEX(getMutex("THINNINGMUTEX")),
		  m_THINNINGWAITCONDITION(getCondition("THINNINGWAITCONDITION")),
		  m_sWorkDir(sWorkDir),
		  m_nSleepAfterRows(nSleepAfter),
		  m_pChipReader(pChipReader)
		{};
		/**
		 * whether object should thinning database files
		 */
		void startDatabaseThinning();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return DatabaseThinning::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * destructor to remove initialed member variables
		 */
		 virtual ~DatabaseThinning()
		 {	DESTROYMUTEX(m_THINNINGMUTEX);
		 	DESTROYCOND(m_THINNINGWAITCONDITION); };

	protected:
		/**
		 * initial the thread for thinning.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		virtual EHObj init(void *args);
		/**
		 * running thread for thinning database.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return whether should start thread again
		 */
		virtual bool execute();
		/**
		 * ending thin thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending() {};

	private:
		/**
		 * mutex lock for thinning condition
		 */
		pthread_mutex_t* m_THINNINGMUTEX;
		/**
		 * condition to waiting for next thinning
		 */
		pthread_cond_t* m_THINNINGWAITCONDITION;
		/**
		 * working directory for database
		 */
		string m_sWorkDir;
		/**
		 * sleep time after every reading row to hold process performance lower
		 */
		__useconds_t m_nSleepAfterRows;
		/**
		 * reference to DefaultConfigReader
		 */
		IChipConfigReaderPattern* m_pChipReader;
		/**
		 * current file for thin database
		 */
		string m_sThinFile;
		/**
		 * defined time for next thinning
		 */
		time_t m_nNextThinningTime;
		/**
		 * map of all filenames with the time to thin
		 */
		map<string, time_t> m_mOldest;
		/**
		 * new sorted database file content
		 */
		map<timeval, string, TimeSort> m_msNewFile;

		/**
		 * comb through database to kill older not needed entry's
		 *
		 * @return whether thinning should be done again
		 */
		bool thinDatabase();
		/**
		 * split an read line from database on harddisk into struct db_t
		 *
		 * @param line line from database on harddisk
		 * @return spliced values
		 */
		db_t splitDbLine(const string& line);
		/**
		 * write entry direct into database
		 *
		 * @param entry db_t structure of chip
		 */
		void writeEntry(const db_t& entry);
		/**
		 * calculate the new time which should thin the database files again.
		 *
		 * @param fromtime the time from which should calculated
		 * @param older pointer to older structure
		 */
		void calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<otime_t> &older);
	};

} /* namespace ppi_database */
#endif /* DATABASETHINNING_H_ */
