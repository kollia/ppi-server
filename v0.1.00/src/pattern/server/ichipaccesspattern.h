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
#ifndef ICHIPACCESSPATTERN_H_
#define ICHIPACCESSPATTERN_H_

#include <string>
#include <vector>

#include "../util/iactionpropertymsgpattern.h"

using namespace std;

namespace design_pattern_world
{
	/**
	 * abstract interface pattern for all chip access
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class IChipAccessPattern
	{
	public:
		/**
		 * display identification name for OWServer
		 *
		 * @return name of server
		 */
		virtual string getServerName()= 0;
		/**
		 * first connection and initialisation to the chips.<br />
		 * This method allocate the first connection and abort also with true if faild.
		 * On faild the owserver call the connect method in 1 second time.
		 * After them if one connection was ok, the server access this method again.
		 *
		 * @param properties reading properties from the main config file
		 * @return wether the initialisation was correct but not chure the connection
		 */
		virtual bool init(const IPropertyPattern* properties)=0;
		/**
		 * return the name of default config file for every chips or pins.<br />
		 * If method returning an null string, no default config will be used
		 *
		 * @return name of file
		 */
		virtual string getDefaultFileName()=0;
		/**
		 * access to physical port interface for the chips.<br />
		 * this method will be called by starting in init method
		 * and also if the connection to the chips was lost
		 *
		 * @return boolean whether the connection be upright
		 */
		virtual bool connect()=0;
		/**
		 * return whether server is connected to the device
		 *
		 * @return whether connected
		 */
		virtual bool isConnected() const =0;
		/**
		 * disconnect the exist connection
		 */
		virtual void disconnect()=0;
		/**
		 * define that dallas semicoductor is used
		 *
		 * @param prop properties from current subroutine
		 * @param id this param get unique id of pin to identify reading or writing
		 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
		 */
		virtual short useChip(const IActionPropertyMsgPattern* prop, string& id)=0;
		/**
		 * returning the ID which define the chip
		 *
		 * @param ID defined ID in subroutine of measure.conf
		 * @return chip id
		 */
		//virtual string getChipID(const string ID)=0;
		/**
		 * function reading from chip
		 * and returning the type
		 *
		 * @param ID id from the chip which defined with getChipID
		 * @return string of type from chip
		 */
		virtual string getChipType(const string ID)=0;
		/**
		 * function reading from chip
		 * and returning an family code
		 *
		 * @param ID id from the chip which defined with getChipID
		 * @return two Hexa character family code geted from chip
		 */
		//virtual string getChipFamily(const string ID)=0;
		/**
		 * chip id for writing in cache of more pins.<br />
		 * If subroutine have action cache and an changed value,
		 * subroutine with action writecache and same ChipTypeID
		 * must be always write
		 *
		 * @param ID unique id of pin
		 * @return chip-type-id
		 */
		virtual string getChipTypeID(const string ID)=0;
		/**
		 * returning all chip id's in an vector
		 *
		 * @return dallas chip id's
		 */
		virtual vector<string> getChipIDs()const=0;
		/**
		 * returning an vector of unused id's on port
		 *
		 * @return vector of unused id's
		 */
		virtual vector<string> getUnusedIDs() const =0;
		/**
		 * check whether id do exist
		 *
		 * @param type type of server (OWFS, Vk8055, ...)
		 * @param id id of chip
		 * @return boolean whether id do exist
		 */
		virtual bool existID(const string type, const string id) const=0;
		/**
		 * should server show debug info
		 * on which cache be writing or reading
		 */
		virtual void setDebug(const bool debug)=0;
		/**
		 * get debug info to show
		 * on which cache be writing or reading
		 */
		virtual bool isDebug()=0;
		/**
		 * write to chip or board
		 *
		 * @param id unique pin-id geted from useChip
		 * @param value Value which should be writing
		 * @return 	-1 if an error occured,
		 * 			 0 if writing was correctly and the pin is finished (go to the next),
		 * 			 1 writing was also correctly but the next time should make the same pin,
		 * 			 2 when an entry was made but writing prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do (value the same as set time before -> go to the next pin)
		 */
		virtual short write(const string id, const double value)=0;
		/**
		 * read from chip or board
		 *
		 * @param id unique pin-id geted from useChip
		 * @param value Result of reading
		 * @return 	-1 if an error occured,
		 * 			 0 if reading was correctly and the pin is finished (go to the next),
		 * 			 1 reading was also correctly but the next time should make the same pin (value is not the last state),
		 * 			 2 when an entry was made but reading prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do, value param set, was reading befor (chip wasn't read, value is correct) -> go to the next pin)
		 */
		virtual short read(const string id, double &value)=0;
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
		virtual void range(const string pin, double& min, double& max, bool &bfloat)= 0;
		/**
		 * all pins which are set for cahce be read.
		 *
		 * @param cachetime whiche cache is finished
		 */
		virtual void endOfCacheReading(const double cachetime)=0;
		/**
		 * whether all reachable chips are defined for this interface,
		 * or one before. No more server with this interface must be start.
		 *
		 * @return true if all used, otherwise false
		 */
		virtual bool reachAllChips()=0;
		/**
		 * method will be called if the end of the loop from reading and writing be reached
		 * to start an new loop
		 */
		virtual void endOfLoop()= 0;
		/**
		 * virtual destructor of pattern class
		 */
		virtual ~IChipAccessPattern() {};
	};
}  // namespace pattern-world

#endif /*ICHIPACCESSPATTERN_H_*/
