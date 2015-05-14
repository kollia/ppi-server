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
#ifndef OWFSSERVER_H_
#define OWFSSERVER_H_

#include <sys/time.h>
#include <time.h>

#include <vector>
#include <queue>
#include <map>

#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/thread/Thread.h"

#include "../util/properties/configpropertycasher.h"

#include "../pattern/server/ichipaccesspattern.h"

#include "KernelModule.h"

using namespace util;

namespace server
{
	/**
	 * server to connect to owcapi from project owfs for all dallas semiconductor
	 * over adapter DS9490 for USB or DS9097U for seriell<br />
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class OWServer : public Thread
	{
	public:
		/**
		 * chip range getting from DbInterface
		 * and is saved in this structure
		 */
		struct chiprange_t {
			/**
			 * minimal value
			 */
			double min;
			/**
			 * maximal value
			 */
			double max;
			/**
			 * whether the chip can hold floating values
			 */
			bool bfloat;
		};

		/**
		 * server constructor to generate instance.<br/>
		 * By cancel this OWServer object, second parameter object will be also delete.
		 *
		 * @param ID id of server
		 * @param type type of server
		 * @param accessPattern pattern to access on the device
		 */
		OWServer(const unsigned short ID, const string& type,
						SHAREDPTR::shared_ptr<IChipAccessPattern> accessPattern);
		/**
		 * server description for external library port reader
		 *
		 * @return description string
		 */
		string getServerDescription()
		{ return m_poChipAccess->getServerDescription(); };
		/**
		 * all initialication of subroutines
		 * are done
		 *
		 * @param out whether result should be shown on command line
		 */
		void endOfInitialisation(bool out);
		/**
		 * whether server has type and chipID
		 *
		 * @param type type of server (OWFS, Vk8055, ...)
		 * @param chipID specific ID which the server should holded
		 * @return whether server has ID
		 */
		bool isServer(const string& type, const string& chipID);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the type
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter famaly code
		 * @return string of type from dallas semicontactor beginning with 'DS'
		 */
		string getChipType(const string &ID);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the family code
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter family code
		 * @return two Hexa character family code geted from chip
		 */
		string getChipFamily(const string& ID);
		/**
		 * returning all chip id's in an vector
		 *
		 * @return dallas chip id's
		 */
		vector<string> getChipIDs();
		/**
		 * should server show debug info
		 * on which cache be writing or reading
		 *
		 * @param debug whether debug messages should showen
		 */
		void setDebug(bool debug);
		/**
		 * get debug info to show
		 * on which cache be writing or reading
		 */
		bool isDebug();
		/**
		 * get debug info for benchmark
		 *
		 * @return vector of string for all devices
		 */
		vector<string> getDebugInfo();
		/**
		 * select all properties and actions which are used in interface
		 *
		 * @param properties reading properties from the main configuration file
		 */
		void usePropActions(const IActionPropertyPattern* properties) const
		{ m_poChipAccess->usePropActions(properties); };
		/**
		 * write to chip direct if pin is set to uncached,<br />
		 * or write into cache for writing later
		 *
		 * @param id unique ID of pin on chip
		 * @param value Value which should be writing
		 * @param adinfo additional info for chip
		 * @return whether writing on the device was by the last one correct
		 */
		bool write(const string& id, const double value, const string& addinfo);
		/**
		 * read from chipdirect if pin is set to uncached,<br />
		 * or read from cahe which be actualiced in defined time
		 *
		 * @param id ID of dallas conductor
		 * @param value Value which should be writing
		 * @return whether the reading on device was correct
		 */
		bool read(const string& id, double* value);
		/**
		 * primary command for chip access library.<br />
		 * If command is an null terminated string, this method should give back
		 * an result of all usable chip ID's.<br />
		 * For all other usable commands, please read the documentation of the library
		 *
		 * @param command specific command for library
		 * @param result output result of command
		 * @param more whether method has more result content for the next time
		 * @return returning error level of command, or 0 when it was done right
		 */
		int command_exec(const string& command, vector<string>& result, bool& more);
		/**
		 * set min and max parameter to the range which can be set for the pin.<br />
		 * If the pin is set from 0 to 1 for writing, in the config file can be set begin while and end.
		 * Otherwise the range is only for calibrate the max and min value if set from client outher range.
		 *
		 * @param pin the pin for whitch the range be asked
		 * @param min the minimal value
		 * @param max the maximal value
		 * @param bfloat whether the values can be float variables
		 */
		void range(const string& pin, double& min, double& max, bool &bfloat);
		/**
		 * whether chips for owserver have an default configuration file
		 * and have to be registered
		 *
		 * @return whether chip shoud be registered
		 */
		bool haveToBeRegistered()
		{ return (m_poChipAccess->getDefaultFileName() == "" ? false : true); };
		/**
		 * define dallas chip is used
		 *
		 * @param properties IActionProperyPattern from current subroutine
		 * @param return value of unique chip ID
		 * @param folder in which folder the chip is used
		 * @param subroutine in which subroutine the chip is used
		 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
		 */
		short useChip(IActionPropertyMsgPattern* properties, string& unique, const string& folder, const string& subroutine);
		/**
		 * check whether all exist id's are used,
		 * otherwise print unused on command-line
		 */
		void checkUnused();
		/**
		 * return true when an chip in measure.conf is not defined
		 */
		bool hasUnusedIDs();
		/**
		 * whether all reachable chips are defined for this interface,
		 * or one before. No more server with this interface must be start.
		 *
		 * @return true if all used, otherwise false
		 */
		virtual bool reachAllChips();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return OWServer::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling routine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * destructor to close connection
		 */
		virtual ~OWServer();

	protected:
		/**
		 * vector for debug which containes sorted device_debug_t structs
		 */
		vector<device_debug_t> m_debugInfo;

		/**
		 * sequence struct
		 */
		struct seq_t
		{
			/**
			 * time in sec and usec for next access
			 */
			timeval tm;
			/**
			 * whether should read sequence by polling
			 */
			bool bPoll;
			/**
			 * next chip in sequence to read
			 */
			 SHAREDPTR::shared_ptr<chip_types_t> nextUnique;
		};
		/**
		 * id of server
		 */
		const unsigned short m_nServerID;
		/**
		 * type of server
		 */
		const string m_sServerType;
		/**
		 * mutex log to write or read on chips
		 */
		pthread_mutex_t* m_WRITEONCHIP;
		/**
		 * mutex log for variable m_mmWriteCache
		 */
		pthread_mutex_t* m_WRITECACHE;
		/**
		 * mutex log for variable m_mmReadCache
		 */
		pthread_mutex_t* m_READCACHE;
		/**
		 * condition to wait for next cache-reading
		 * or into prioritycache something written
		 */
		pthread_cond_t* m_PRIORITYCACHECOND;
		/**
		 * mutex lock for priority cache map
		 */
		pthread_mutex_t* m_PRIORITYCACHE;
		/**
		 * mutex lock for priority 1
		 */
		pthread_mutex_t* m_PRIORITY1CHIP;
		/**
		 * mutex lock of changed cache entrys
		 */
		pthread_mutex_t* m_CACHEWRITEENTRYS;
		/**
		 * mutex lock for changing debug info
		 */
		pthread_mutex_t* m_DEBUGINFO;
		/**
		 * mutex lock to execute <code>command_exec()</code>
		 */
		pthread_mutex_t* m_EXECUTEMUTEX;
		/**
		 * whether an connection is made correctly
		 */
		bool m_bConnected;
		/**
		 * IPropertyPattern object from server.conf for initialitation
		 */
		IPropertyPattern* m_oServerProperties;
		/**
		 * seconds for aktualice cached reading writing
		 */
		unsigned int m_cache;
		/**
		 * whether timer is set correctly
		 * and server can read chips also in an sequence
		 */
		bool m_bReadSeq;
		/**
		 * map of all chips for hole information.<br />
		 * Key is set as unique pin-id, value globaly information of type, family
		 * and an map member called actions with all used pins.<br />
		 * This variable is used for all information if will be reading or writing
		 * to the priority-cache or used in an cacheReader.<br />
		 * Otherwise if chip not be set in one of them, server reading or writing
		 * only when nothing to do.
		 */
		map<string, SHAREDPTR::shared_ptr<chip_types_t> > m_mtConductors;
		/**
		 * cache of chips which should not write directly
		 */
		//map<string, map<string, string> > m_mmWriteCache;
		/**
		 * cache of chips with priority to actualizise
		 */
		map<int, queue<SHAREDPTR::shared_ptr<chip_types_t> > > m_mvPriorityCache;
		/**
		 * cache with all reading sequences
		 */
		map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > > m_mvReadingCache;
		/**
		 * cache of realy reading sequences
		 */
		map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > > m_mvTrueReadingCache;
		/**
		 * real timeval structure for every cache sequence
		 * to start measuring in cache again
		 */
		map<double, seq_t> m_mStartSeq;
		/**
		 * last writing value
		 * to now whether to write on the chip
		 */
		map<string, map<string, string> > m_mmLastWriten;

		/**
		 * define method to initial the interface to dallas semiconductor.<br />
		 * this method will be called before running
		 * the method execute and also whether the connection was lost
		 *
		 * @see http://www.owfs.org
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		OVERWRITE EHObj init(void *args);
		/**
		 * reading all devices for first state
		 *
		 * @param out whether result should be shown on command line
		 * @return whether OWServer is correct connected to external devices
		 */
		bool readFirstChipState(bool out);
		/**
		 * define method to running thread.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return object of error handling
		 */
		OVERWRITE bool execute();
		/**
		 * define method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();

	private:
		/**
		 * whether all initialication of subroutines are done
		 */
		bool m_bAllInitial;
		/**
		 * chace writing every time if chip-type-id in this vector
		 */
		vector<string> m_vChipTypeIDs;
		/**
		 * chip range getting from DbInterface
		 * and is saved in this structure
		 */
		map<string, chiprange_t> m_mRange;
		/**
		 * ConfigPropertyCasher which was reading from server.conf
		 */
		ConfigPropertyCasher* m_pProp;
		/**
		 * access pattern interface to extern chip
		 */
		SHAREDPTR::shared_ptr<IChipAccessPattern> m_poChipAccess;
		/**
		 * whether owreader should read any chip from m_vkernelR;
		 */
		bool m_bKernel;
		/**
		 * whether need only to ask for reading the kernelmodul
		 * and have not start and second thread
		 */
		bool m_bKernelOnly;
		/**
		 * whether owserver should polling over read sequence
		 */
		bool m_bPollRead;
		/**
		 * KernelModule class reading the kernelmodule from <code>IChipAccessPattern</code>
		 */
		auto_ptr<KernelModule> m_pKernelModule;

		/**
		 * measure time difference by debugging for reading or wirting on device
		 *
		 * @param device pointer to actual struct device_debug_t which hold time before
		 */
		void measureTimeDiff(device_debug_t* device) const;
	};


}

#endif /*OWFSSERVER_H_*/
