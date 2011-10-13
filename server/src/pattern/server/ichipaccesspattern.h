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
#include <map>

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
		 * select all properties and actions whitch are uesed in interface
		 *
		 * @param properties reading properties from the main configure file
		 */
		virtual void usePropActions(const IActionPropertyPattern* properties) const= 0;
		/**
		 * first connection and initialization to the chips.<br />
		 * This method allocate the first connection and abort also with true if faild.
		 * On failed the owserver call the connect method in 1 second time.
		 * After them if one connection was ok, the server access this method again.
		 *
		 * @param properties reading properties from the main configure file
		 * @return wether the initialization was correct but not chure the connection
		 */
		virtual bool init(const IPropertyPattern* properties)=0;
		/**
		 * return the name of default configure file for every chips or pins.<br />
		 * If method returning an null string, no default configure will be used
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
		 * define which chip is used, defined in measure.conf.<br />
		 * If the third parameter kernelmode will be set in method to one or two,
		 * owreader starts an second thread to read values from an kernelmodule.
		 * By one, after reading over method <code>kernelmodule</code> the reading with method
		 * <code>read()</code> will be done only in this thread.
		 * By two the reading will be done in other thread by polling.<br />
		 * Read also description for <code>kernelmodule()</code>.
		 *
		 * @param prop properties from current subroutine in measure.conf
		 * @param id this param get unique id of pin to identify reading or writing
		 * @param kernelmode give back whether owreader should read values over an kernel module.<br />
		 * 			0 for normaly read over polling by owreader, 1 by read only over kernel module
		 * 			(no writing to chips) and 2 read over kernel but write also new state out
		 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
		 */
		virtual short useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmode)=0;
		/**
		 * read values over an kernel module.<br />
		 * If some chips defined in method <code>useChip()</code> with kernelmode,
		 * owreader starts an second thread to call this method.
		 * This method should be thread save with variables which read also in method <code>read()</code>
		 * from an other thread by set kernelmode to two in method <code>useChip</code>.
		 * if some reading codes defined with kernelmode null ('0') or two ('2'),
		 * because the method <vode>read()</code> will be called
		 * in two different threads.
		 * When the returned id is undefined (not null "") all id's will be asked with <code>read()</code>.
		 * If the returned id is null ("") owreader do nothing and start this method again.
		 * The developer should return also null ("") when he/she set the chip to read by polling
		 * (kernelmode to two ('2') in <code>useChip()</code>), because otherwise the reading will be done
		 * in two threads and the value can be inconsistent.<br />
		 * Please do not create an no ending loop in this method, because if owreader will stopping
		 * the module of kernel settings this method should be stopped also by <code>disconnect()</code>.
		 *
		 * @param read <code>map&lt;chip id, do reading&gt;</code> set chip id to reading over polling from owreader sequence.
		 * @return changed id defined in <code>useChip()</code>
		 */
		virtual string kernelmodule(map<string, bool>& read)= 0;
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
		 * @param addinfo additional info for chip
		 * @return 	-1 if an error occured,
		 * 			 0 if writing was correctly and the pin is finished (go to the next),
		 * 			 1 writing was also correctly but the next time should make the same pin,
		 * 			 2 when an entry was made but writing prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do (value the same as set time before -> go to the next pin)
		 */
		virtual short write(const string id, const double value, const string& addinfo)=0;
		/**
		 * read from chip or board.<br />
		 * If some of the reading id's defined in method <code>useChip()</code> are defined with the flag kernelmode
		 * but not all, the variables set in method <code>kernelmodule()</code> and will be read her should be thread save.
		 *
		 * @param id unique pin-id get from useChip
		 * @param value Result of reading
		 * @return 	-1 if an error occurred,
		 * 			 0 if reading was correctly and the pin is finished (go to the next),
		 * 			 1 reading was also correctly but the next time should make the same pin (value is not the last state),
		 * 			 2 when an entry was made but reading prime in an next time on an other pin (pin is finished -> go to the next)
		 * 			 3 if nothing to do, value param set, was reading before (chip wasn't read, value is correct) -> go to the next pin)
		 * 			 4 reading was correct but hang out for polling again
		 */
		virtual short read(const string id, double &value)=0;
		/**
		 * primary command for chip access library.<br />
		 * If command is an null terminated string, this method should give back
		 * an result of all usable chip ID's.<br />
		 * For all other usable commands, please read the documentation of the library.<br />
		 * This method coming thread save from the owserver object. This means only one thread
		 * can attach this method. When this method is blocking the thread, all subroutines are blocking
		 * which want to call this method (also from other folder's)
		 *
		 * @param command specific command for library
		 * @param result output result of command
		 * @param more whether method has more result content for the next time
		 * @return returning error level of command, or 0 when it was done right
		 */
		virtual int command_exec(const string& command, vector<string>& result, bool& more)= 0;
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
