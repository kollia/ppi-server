/**
 *   This file 'MeasureInformerCache.h' is part of ppi-server.
 *   Created on: 13.08.2014
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

#ifndef MEASUREINFORMERCACHE_H_
#define MEASUREINFORMERCACHE_H_

#include <string>

#include "../util/debug.h"

#include "../util/thread/Thread.h"

#include "../pattern/util/IInformerCachePattern.h"

#include "ListCalculator.h"

namespace util
{
	using namespace std;

	class MeasureInformerCache : public IInformerCachePattern
	{
	public:
		typedef pair<InformObject, ppi_time> inform_type;
		typedef vector<inform_type> informvec_type;
		typedef SHAREDPTR::shared_ptr<informvec_type> sharedinformvec_type;
		/**
		 * constructor of informer caches
		 * to inform folder threads to running
		 *
		 * @param forFolder for which folder cache should be hold starting values
		 * @param folderObj object of measuring folder thread
		 * @param informOutput object to informing output on Terminal
		 */
		MeasureInformerCache(const string& forFolder, IMeasurePattern* folderObj,
						SHAREDPTR::shared_ptr<IListObjectPattern> informOutput);
		/**
		 * returning name of folder for which cache be used
		 *
		 * @return name of folder
		 */
		OVERWRITE string getFolderName() const
		{ return m_sFolderName; };
		/**
		 * do debug output for informer ListCalculater
		 * when used
		 *
		 * @param bOut whether should show output
		 */
		void doOutput(bool bOut)
		{ m_oInformeThread.doOutput(bOut); };
		/**
		 * information by changed value in any subroutine
		 *
		 * @param folder which folder should be informed
		 * @param from from which folder comes information
		 */
		OVERWRITE void changedValue(const string& folder, const InformObject& from);
		/**
		 * whether folder list thread is defined for starting
		 *
		 * @param vStartTimes vector of starting ID`s and time when starting
		 * @return whether this list folder should starting
		 */
		OVERWRITE bool waitStarting(const vector<ppi_time>& vStartTimes) const;
		/**
		 * whether folder list thread should starting.<br />
		 * And remove also all old definitions.
		 *
		 * @param vStartTimes vector of starting ID`s and time when starting
		 * @param mInformed map of informing folders on which starting ID will be started.<br />
		 *                  will be filled from method
		 * @param bLocked whether content vector of cache be locked from other folder thread
		 * @param debug whether current session running in debugging mode.<br />
		 *              in this case, mInformed will be not filled
		 * @return whether own folder should starting
		 */
		OVERWRITE bool shouldStarting(const vector<ppi_time>& vStartTimes,
						map<short, vector<InformObject> >& mInformed,
						bool* bLocked, bool debug);
		/**
		 * destructor to destroy object
		 */
		virtual ~MeasureInformerCache()
		{	DESTROYMUTEX(m_CACHEVALUEMUTEX);	};

	private:
		/**
		 * folder thread object for which cache should store entrys
		 */
		IMeasurePattern* m_pFolderObject;
		/**
		 * folder name for which cache
		 */
		const string m_sFolderName;
		long m_lCount;
		/**
		 * object for informing output
		 */
		SHAREDPTR::shared_ptr<IListObjectPattern> m_oInformOutput;
		/**
		 * Calculate whether folder thread should be informed to start running
		 */
		ListCalculator m_oInformeThread;
		/**
		 * all changed folder
		 */
		sharedinformvec_type m_vFolderCache;
		/**
		 * mutex to write or read cache
		 */
		pthread_mutex_t* m_CACHEVALUEMUTEX;
		/**
		 * mutex to check only whether
		 * folder list should starting
		 */
		pthread_mutex_t* m_CHECKCACHE;
		/**
		 * mutex want to inform folder to running
		 */
		pthread_mutex_t *m_WANTINFORM;
		/**
		 * mutex for fill or erase new activate time
		 */
		pthread_mutex_t *m_ACTIVATETIME;
		/**
		 * condition for wait for new changing of any subroutine
		 */
		pthread_cond_t *m_VALUECONDITION;

		/**
		 * whether folder list thread has definitions to start
		 *
		 * @param vStartTimes vector of starting ID`s and time when starting
		 * @param mInformed map of informing folders on which starting ID will be started.<br />
		 *                  will be filled from method
		 * @param cache cache of all starting informations
		 * @param debug whether current session running in debugging mode.<br />
		 *              in this case, mInformed will be not filled
		 * @return whether own folder should starting
		 */
		bool hasToStart(const vector<ppi_time>& vStartTimes,
						map<short, vector<InformObject> >& mInformed,
						const sharedinformvec_type cache, bool debug) const;
	};

} /* namespace util */
#endif /* MEASUREINFORMERCACHE_H_ */
