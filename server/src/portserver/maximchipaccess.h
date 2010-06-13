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
#ifndef MAXIMCHIPACCESS_H_
#define MAXIMCHIPACCESS_H_

#include "../util/debug.h"
#ifdef _OWFSLIBRARY

#include <string>

#include "../util/Thread.h"
#include "../util/smart_ptr.h"

#include "../pattern/server/ichipaccesspattern.h"
#include "../pattern/util/iactionpropertymsgpattern.h"

using namespace std;
using namespace design_pattern_world;

namespace ports
{

	/**
	 * connection class to external chips
	 * from the company Maxim (old Dallas)
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class MaximChipAccess : public IChipAccessPattern
	{
	public:
		/**
		 * display identification name for OWServer
		 *
		 * @return name of server
		 */
		virtual string getServerName()
		{ return "OWFS device"; };
		/**
		 * constructor to initial class
		 * with main property management
		 *
		 * @param init 	initialitation string for dallas semiconductors.<br />
		 * 				If string not defined, init method fetch full maximinit string
		 * 				from properties
		 */
		MaximChipAccess(const string init= "", vector<string>* idsBefore= NULL);
		/**
		 * select all properties and actions whitch are uesed in interface
		 *
		 * @param properties reading properties from the main config file
		 */
		virtual void usePropActions(const IActionPropertyPattern* properties) const;
		/**
		 * first connection and initialisation to the chips.<br />
		 *
		 * @param properties reading properties from the main config file
		 * @return wether the initialisation was correct
		 */
		virtual bool init(const IPropertyPattern* properties);
		/**
		 * return the name of default config file for every chips or pins
		 *
		 * @return name of file
		 */
		virtual string getDefaultFileName()
		{ return "defaultmaxim.conf"; };
		/**
		 * define method to initial the interface to dallas semiconductor.<br />
		 * this method will be called before running
		 * the method execute and also whether the connection to the chips was lost
		 *
		 * @see http://www.owfs.org for init
		 * @return boolean whether the method execute can start
		 */
		virtual bool connect();
		/**
		 * return whether server is connected to the device
		 *
		 * @return whether connected
		 */
		virtual bool isConnected() const
					{ return m_bConnected; };
		/**
		 * disconnect the exist connection
		 */
		virtual void disconnect();
		/**
		 * define that dallas semicoductor is used
		 *
		 * @param prop properties from current subroutine
		 * @param id this param get unique id of pin to identify reading or writing
		 * @param kernelmode give back whether owreader should read values over an kernel module (give back always 0)
		 * @return 0 if the pin is not correct, 1 if the pin is for reading or 2 for writing
		 */
		virtual short useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmode);
		/**
		 * returning the ID which define the chip
		 *
		 * @param ID defined ID in subroutine of measure.conf
		 * @return chip id
		 */
		virtual string getChipID(const string ID);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the type
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter famaly code
		 * @return string of type from dallas semicontactor beginning with 'DS'
		 */
		virtual string getChipType(const string ID);
		/**
		 * function reading in one wire filesystem
		 * from the chip with the ID as subdirectory
		 * the family code
		 *
		 * @param ID XX.XXXXXXXXXX id from the chip beginning with two cahracter family code
		 * @return two Hexa character family code geted from chip
		 */
		virtual string getChipFamily(const string ID);
		/**
		 * chip id for writing in cache of more pins.<br />
		 * If subroutine have action cache and an changed value,
		 * subroutine with action writecache and same ChipTypeID
		 * must be always write
		 *
		 * @param ID unique id of pin
		 * @return chip-type-id
		 */
		virtual string getChipTypeID(const string ID);
		/**
		 * read all chip ID's
		 *
		 * @param vsIds all chip ID's whitch found
		 * @return whether an ERROR occured, error number defined in errno
		 */
		bool readChipIDs(vector<string>* vsIds) const;
		/**
		 * returning all chip id's in an vector
		 *
		 * @return dallas chip id's
		 */
		virtual vector<string> getChipIDs() const;
		/**
		 * check whether id do exist
		 *
		 * @param type type of server (OWFS, Vk8055, ...)
		 * @param id id of chip
		 * @return boolean whether id do exist
		 */
		virtual bool existID(const string type, const string id) const;
		/**
		 * returning an vector of unused id's on port
		 *
		 * @return vector of unused id's
		 */
		virtual vector<string> getUnusedIDs() const;
		/**
		 * should server show debug info
		 * on which cache be writing or reading
		 */
		virtual void setDebug(bool debug);
		/**
		 * get debug info to show
		 * on which cache be writing or reading
		 */
		virtual bool isDebug();
		/**
		 * write to chip directly
		 *
		 * @param id unique pin-id of dallas conductor
		 * @param value Value which should be writing
		 * @return 	-1 if an error occured,
		 * 			 0 if writing was correctly and the pin is finished (go to the next),
		 * 			 1 writing was also correctly but the next time should make the same pin,
		 * 			 2 when an entry was made but writing prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do (value the same as set time before -> go to the next pin)
		 */
		virtual short write(const string id, const double value);
		/**
		 * read from chip directly
		 *
		 * @param id unique pin-id of dallas conductor
		 * @param value Result of reading
		 * @return 	-1 if an error occured,
		 * 			 0 if reading was correctly and the pin is finished (go to the next),
		 * 			 1 reading was also correctly but the next time should make the same pin (value is not the last state),
		 * 			 2 when an entry was made but reading prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do, value param set, was reading befor (chip wasn't read, value is correct) -> go to the next pin)
		 */
		virtual short read(const string id, double &value);
		/**
		 * this method is an dummy, because no kernelmode in <code>useChip()</code> be set.
		 *
		 * @return only an null string ("")
		 */
		virtual string kernelmodule() { return ""; };
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
		virtual void range(const string pin, double& min, double& max, bool &bfloat);
		/**
		 * all pins which are set for cahce be read.<br />
		 *
		 * @param cachetime whiche cache is finished
		 */
		virtual void endOfCacheReading(const double cachetime);
		/**
		 * whether all reachable chips are defined for this interface,
		 * or one before. No more server with this interface must be start.
		 *
		 * @return true if all used, otherwise false
		 */
		virtual bool reachAllChips();
		/**
		 * destructor of MaximChipAccess class
		 */
		virtual ~MaximChipAccess();

	protected:
		/**
		 * ConfigPropertyCasher which was reading from server.conf
		 */
		const IPropertyPattern* m_pProp;
		/**
		 * count of this used interface
		 */
		static short m_nInterface;

	private:
		/**
		 * struct of an pin from an chip
		 */
		struct chip_pin_t
		{
			/**
			 * unique ID of pin
			 */
			string id;
			/**
			 * unique ID of chip
			 */
			string chipid;
			/**
			 * path which is reading or writing on chip
			 */
			string path;
			/**
			 * pin number if chip is extra defined in class
			 * to write or read on more than one pins
			 */
			short pin;
			/**
			 * value of pin from chip
			 * if it is an cached reading
			 */
			double value;
			/**
			 * reading in 2 steps and wirte one on DS2408
			 * if action after be set.<br />
			 * Otherwise reading only one.
			 */
			short steps;
			/**
			 * if pin is for writing,
			 * whether write all pins from chip to multitask
			 */
			bool cache;
			/**
			 * whether pin on chip is used
			 */
			bool used;
			/**
			 * subroutine position to write on screen
			 */
			string msg;
			/**
			 * subroutine position with warning to write on screen
			 */
			string warnmsg;
			/**
			 * subroutine position error to write on screen
			 */
			string errmsg;
		};
		/**
		 * struct of chip defined types,
		 * with an map for actions for all pins
		 */
		struct chip_type_t
		{
			/**
			 * ID of folder
			 */
			short folder;
			/**
			 * type of chip
			 */
			string type;
			/**
			 * family code of chip
			 */
			string family;
			/**
			 * value for all pins writing on chip
			 */
			string holeValue;
			/**
			 * map of actions for all pins in chip
			 */
			map<string, SHAREDPTR::shared_ptr<chip_pin_t> > pins;
			/**
			 * whether chip is used
			 */
			bool used;
			/**
			 * if the pin is for writing
			 * this flag is set whether
			 * writing should done in one of next
			 */
			bool cache;
		};
		/**
		 * whether should showen debug info
		 * for which cache be reading
		 */
		bool m_bDebug;
		/**
		 * whether an connection is made correctly
		 */
		bool m_bConnected;
		/**
		 * initialitation string for dallas conductors
		 */
		string m_sInit;
		/**
		 * mutex lock of show debug info
		 */
		pthread_mutex_t* m_DEBUGINFO;
		/**
		 * mutex lock for incorrect chips
		 */
		static pthread_mutex_t* m_INCORRECTCHIPIDS;
		/**
		 * map of all founded dallas conductor with chip ID as key
		 * and pointer to an chip structure as value
		 */
		map<string, SHAREDPTR::shared_ptr<chip_type_t> > m_mConductors;
		/**
		 * map of used dallas conductors with unigue pin ID as key
		 * and an pointer to the pin inside of m_mConductors as value
		 */
		map<string, SHAREDPTR::shared_ptr<chip_pin_t> > m_mUsedChips;
		/**
		 * map of all family codes for ID's
		 */
		map<string, string> m_mFamilyCodes;
		/**
		 * cache with reading secuences
		 */
		map<double, vector<SHAREDPTR::shared_ptr<chip_pin_t> > > m_mvReadingCache;
		/**
		 * all ids which are founded in an maxim server before
		 */
		mutable vector<string> m_vsBefore;
		/**
		 * founded id's of chip's
		 */
		mutable vector<string> m_vsIds;
		/**
		 * all incorrect or not founding chips
		 */
		static vector<string> m_vsIncorrect;

		/**
		 * fill into the m_vsIncorrect vector the chipID
		 * witch was incorrect by reading, writing or does not found.
		 * To polling for read the directory -> maybe it found later
		 *
		 * @param chipID ID from the incorrect chip
		 */
		static void incorrectChip(const string chipID);
		/**
		 * search in the m_vsIncorrect vector for the given chip
		 * to erase it from the vector
		 *
		 * @param chipID ID from new founded chip
		 */
		static void foundChip(const string chipID);
		/**
		 * request whether in one OWFS server is an incorrect chip
		 *
		 * @return whether one incorrect chip exists
		 */
		static bool hasIncorrectChips();
		/**
		 * method will be called if the end of the loop from reading and writing be reached
		 * to start an new loop
		 */
		virtual void endOfLoop();
		/**
		 * fill the chip into the chip map of m_mConductors
		 *
		 * @param chipID id of chip which should fill in
		 * @param whether chip must be inserted
		 */
		bool fillChip(const string chipID);
	};

}

#endif //_OWFSLIBRARY
#endif /*MAXIMCHIPACCESS_H_*/
