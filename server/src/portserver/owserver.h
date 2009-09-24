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

#include "../util/Thread.h"
#include "../util/configpropertycasher.h"

#include "../pattern/server/ichipaccesspattern.h"

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
		 * struct to writing debug information for time reading
		 * and writing.
		 */
		struct device_debug_t
		{
			/**
			 * count id of debug struct
			 */
			unsigned short id;
			/**
			 * whether actual time can reading gettimeofday
			 */
			bool btime;
			/**
			 * actual time of reading or writing
			 */
			timeval act_tm;
			/**
			 * how often device was read or write in the actual session
			 */
			unsigned short count;
			/**
			 * whether the device is for reading (true)
			 * or writing (false)
			 */
			bool read;
			/**
			 * whether device was reachable
			 */
			bool ok;
			/**
			 * needed time in micro seconds of reading or writing
			 */
			long utime;
			/**
			 * measured or written value
			 */
			double value;
			/**
			 * cache time for reading
			 */
			double cache;
			/**
			 * priority state for writing
			 */
			unsigned int priority;
			/**
			 * id of device
			 */
			string device;
			/**
			 * sort operator to comparing actual time whether own is greater
			 */
			bool operator > (const device_debug_t* other) const
			{
				if(act_tm.tv_sec > other->act_tm.tv_sec)
					return true;
				return act_tm.tv_usec > other->act_tm.tv_usec ? true : false;
			}
			/**
			 * find operator to comparing actual time wheteher both are the same
			 */
			int operator == (const device_debug_t* other)
			{
				return (device == other->device);
				/*if(	act_tm.tv_sec == other->act_tm.tv_sec
					&&
					act_tm.tv_usec == other->act_tm.tv_usec	)
				{
					return true;
				}
				return false;*/
			}
		};

		/**
		 * server constructor to generate instance
		 *
		 * @param ID id of server
		 * @param accessPattern pattern to access on the device
		 */
		OWServer(const unsigned short ID, IChipAccessPattern* accessPattern);
		/**
		 * display identification name for OWServer
		 *
		 * @return name of server
		 */
		string getServerName()
		{ return m_poChipAccess->getServerName(); };
		/**
		 * all initialication of subroutines
		 * are done
		 */
		static void endOfInitialisation();
		/**
		 * whether server has type and chipID
		 *
		 * @param type type of server (OWFS, Vk8055, ...)
		 * @param chipID specific ID which the server should holded
		 * @return whether server has ID
		 */
		bool isServer(const string type, const string chipID);
		/**
		 * return instance of server with given id
		 *
		 * @param ID id of server
		 */
		static OWServer* getServer(const unsigned short ID);
		/**
		 * method returning the right server for an specific chip ID.<br />
		 * All OWServer instaces be defined in an vector in the constructor
		 *
		 * @param type type of server (OWFS, Vk8055, ...)
		 * @param chipID specific ID which the server should holded
		 * @return the server instance
		 */
		static OWServer* getServer(const string type, const string chipID);
		/**
		 * delete all exist server
		 */
		static void delServers(OWServer* server= NULL);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the type
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter famaly code
		 * @return string of type from dallas semicontactor beginning with 'DS'
		 */
		string getChipType(string &ID);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the family code
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter family code
		 * @return two Hexa character family code geted from chip
		 */
		string getChipFamily(const string ID);
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
		 * given server shows debug info
		 * on which cache be writing or reading.<br />
		 * When server id is 0 all server be set to debug false.
		 * Every time only one server be set to debug true.
		 *
		 * @param ID id of server
		 */
		static void setDebug(const unsigned short ID);
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
		 * write to chip direct if pin is set to uncached,<br />
		 * or write into cache for writing later
		 *
		 * @param id unique ID of pin on chip
		 * @param value Value which should be writing
		 * @return whether writing on the device was by the last one correct
		 */
		bool write(const string id, const double value);
		/**
		 * read from chipdirect if pin is set to uncached,<br />
		 * or read from cahe which be actualiced in defined time
		 *
		 * @param id ID of dallas conductor
		 * @param value Value which should be writing
		 * @return whether the reading on device was correct
		 */
		bool read(const string id, double* value);
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
		void range(const string pin, double& min, double& max, bool &bfloat);
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
		 * check whether all exist id's are used
		 */
		static void checkUnused();
		/**
		 * return true when an chip in measure.conf is not defined
		 */
		bool hasUnusedIDs();
		/**
		 * print id's on screen if it is unused
		 */
		void printUnusedIDs();
		/**
		 * whether all reachable chips are defined for this interface,
		 * or one before. No more server with this interface must be start.
		 *
		 * @return true if all used, otherwise false
		 */
		virtual bool reachAllChips();
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
		 * struct of chip defined types,
		 * with an map for actions for all pins
		 */
		struct chip_types_t
		{
			/**
			 * unique ID of pin
			 */
			string id;
			/**
			 * unique ID for pins in an cache writing
			 */
			string wcacheID;
			/**
			 * whether value should read or write
			 */
			bool read;
			/**
			 * whether writing will be make in an cache
			 */
			bool writecache;
			/**
			 * value of pin from chip
			 * if it is an cached reading
			 */
			double value;
			/**
			 * priority of pin if it is for writing
			 */
			unsigned int priority;
			/**
			 * seconds for reading again as cache
			 */
			unsigned long sec;
			/**
			 * sequence of time to reading in an cache
			 */
			struct timeval timeSeq;
			/**
			 * whether reading or write on device was correct
			 */
			bool device;
			/**
			 * inline greater operator
			 */
			bool operator > (const chip_types_t* other)
			{
				return id > other->id ? true : false;
			}
			/**
			 * inline similar operator
			 */
			bool operator == (const chip_types_t* other)
			{
				return id == other->id ? true : false;
			}
		};

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
			 * next chip in sequence to read
			 */
			 chip_types_t* nextUnique;
		};
		/**
		 * id of server
		 */
		const unsigned short m_nServerID;
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
		map<string, chip_types_t*> m_mtConductors;
		/**
		 * cache of chips which should not write directly
		 */
		//map<string, map<string, string> > m_mmWriteCache;
		/**
		 * cache of chips with priority to actualizise
		 */
		map<int, queue<chip_types_t*> > m_mvPriorityCache;
		/**
		 * cache with reading secuences
		 */
		map<double, vector<chip_types_t*> > m_mvReadingCache;
		/**
		 * real timeval structure for every chace sequence
		 * to start measureing in cache again
		 */
		map<double, seq_t> m_mStartSeq;
		/**
		 * last writing value
		 * to now whether to write on the chip
		 */
		map<string, map<string, string> > m_mmLastWriten;
		/**
		 * is one chip with priority 1 written
		 * set this rwv_t to chip. No other chip with priority 1
		 * can write until the first pin of chip is unwritten
		 */
		chip_types_t* m_ptPriority1Chip;

		/**
		 * define method to initial the interface to dallas semiconductor.<br />
		 * this method will be called before running
		 * the method execute and also whether the connection was lost
		 *
		 * @see http://www.owfs.org
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual int init(void *args);
		/**
		 * reading all devices for first state
		 *
		 * @return whether OWServer is correct connected to external devices
		 */
		bool readFirstChipState();
		/**
		 * define method to running thread.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @param error code for not correctly done
		 */
		virtual int execute();
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
		 * all server instances as value
		 * and the ID as key
		 */
		static map<unsigned short, OWServer*> m_mOwServers;
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
		IChipAccessPattern* m_poChipAccess;
		/**
		 * thread for any questions to server
		 */
		Thread* m_oQuestions;

		/**
		 * measure time difference by debugging for reading or wirting on device
		 *
		 * @param device pointer to actual struct device_debug_t which hold time before
		 */
		void measureTimeDiff(device_debug_t* device) const;
	};


}

#endif /*OWFSSERVER_H_*/
