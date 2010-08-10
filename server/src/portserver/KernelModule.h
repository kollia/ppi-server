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

#ifndef KERNELMODULE_H_
#define KERNELMODULE_H_

#include "../util/structures.h"
#include "../util/thread/Thread.h"

#include "../pattern/server/ichipaccesspattern.h"

namespace server {

	using namespace design_pattern_world;

	/**
	 * class definition of KernelModule
	 * will be create if defined any reading chip in method <code>useChip</code>
	 * from an <code>IChipAccessPattern</code> with the flag kernelmode
	 */
	class KernelModule : public Thread
	{
	public:
		using Thread::stop;
		/**
		 * constructor of class
		 *
		 * @param servertype type of server for which <code>IChipAccessPattern</code> running
		 * @param chipaccess module to have access to extern chips or board
		 * @param readcache mutex variable to lock reading chips
		 */
		KernelModule(const string& servertype, IChipAccessPattern* chipaccess, pthread_mutex_t* readcache);
		/**
		 * fill with chip's which should read in <code>kernelmodule()</code> from <code>IChipAccessPattern</code>
		 *
		 * @param cip defined chip
		 */
		void fillChipType(chip_types_t* chip)
		{ m_vkernelR.push_back(chip); };
		/**
		 * method to running thread
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return defined error code from extended class
		 */
		virtual int execute();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait);
		/**
		 * destructor of class
		 */
		virtual ~KernelModule();

	protected:
		/**
		 * method to initial the thread
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return defined error code from extended class
		 */
		virtual int init(void *args) { return 0; };
		/**
		 * read direct from chip
		 *
		 * @param endWork status of chip (-1 fault, 0 finished, 1 read again)
		 * @param value read value from <code>IChipAccessPattern</code>
		 * @param pActChip in which chip the results will be writing
		 */
		bool readChip(const short endWork, const double value, chip_types_t* pActChip);
		/**
		 * method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending() {};

	private:
		/**
		 * type of server for which <code>IChipAccessPattern</code> running
		 */
		string m_sServerType;
		/**
		 * module to have access to extern chips or board
		 * need methods <code>kernelmodule()</code> and <code>read()</code>
		 */
		IChipAccessPattern* m_poChipAccess;
		/**
		 * all chips should be read over an kernel moule
		 */
		vector<chip_types_t*> m_vkernelR;
		/**
		 * mutex to lock reading variables
		 */
		pthread_mutex_t* m_READCACHE;
	};

}

#endif /* KERNELMODULE_H_ */
