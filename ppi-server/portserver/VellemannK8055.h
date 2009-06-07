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
#ifndef VELLEMANNK8055_H_
#define VELLEMANNK8055_H_

#include "../util/debug.h"
#ifdef _K8055LIBRARY

#include <string>

#include "../pattern/util/ipropertypattern.h"
#include "../pattern/server/ichipaccesspattern.h"

using namespace design_pattern_world;

namespace ports
{

	class VellemannK8055 : public IChipAccessPattern
	{
		public:
			/**
			 * display identification name for OWServer
			 *
			 * @return name of server
			 */
			virtual string getServerName()
			{ return "k8055 USB port from Vellemann"; };
			/**
			 * konstructor to declare object with port ID on USB
			 *
			 * @param ID port ID jumperd on K8055 platine
			 */
			VellemannK8055(long ID);
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
			{ return "defaultvellemann.conf"; };
			/**
			 * access to physical port interface for the chips.<br />
			 * this method will be called by starting in init method
			 * and also if the connection to the chips was lost
			 *
			 * @return boolean whether the connection be upright
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
			 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
			 */
			virtual short useChip(const IActionPropertyMsgPattern* prop, string& id);
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
			virtual string getChipType(const string ID);
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
			virtual string getChipTypeID(const string pin);
			//string getConstChipTypeID(const string ID) const;
			/**
			 * returning all chip id's in an vector
			 *
			 * @return dallas chip id's
			 */
			virtual vector<string> getChipIDs() const;
			/**
			 * returning an vector of unused id's on port
			 *
			 * @return vector of unused id's
			 */
			virtual vector<string> getUnusedIDs() const;
			/**
			 * check whether id do exist
			 *
			 * @param type type of server (OWFS, Vk8055, ...)
			 * @param id id of chip
			 * @return boolean whether id do exist
			 */
			virtual bool existID(const string type, const string id) const;
			/**
			 * should server show debug info
			 * on which cache be writing or reading
			 */
			virtual void setDebug(const bool debug);
			/**
			 * get debug info to show
			 * on which cache be writing or reading
			 */
			virtual bool isDebug();
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
			virtual short write(const string id, const double value);
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
			virtual short read(const string id, double &value);
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
			 * all pins which are set for cache be read.
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
			 * method will be called if the end of the loop from reading and writing be reached
			 * to start an new loop
			 */
			virtual void endOfLoop();
			/**
			 * desctuctor of object
			 */
			virtual ~VellemannK8055();

		private:
			/**
			 * mutex lock of show debug info
			 */
			pthread_mutex_t* m_DEBUGINFO;
			/**
			 * whether any access on board is used
			 */
			bool m_bUsed;
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
			 * ID from Vellemann k8055 port on USB.<br />
			 * Can be from 0 to 3
			 */
			long m_nID;
			/**
			 * define which pin will be used on k8055<br />
			 * 	<table>
			 * 		<tr>
			 * 			<td>
			 * 				01 - 08
			 * 			</td>
			 * 			<td>
			 * 				&#160;&#160;-&#160;&#160;
			 * 			</td>
			 * 			<td>
			 * 				digital digital output
			 * 			</td>
			 * 		</tr>
			 * 		<tr>
			 * 			<td>
			 * 			</td>
			 * 			<td>
			 * 				&#160;&#160;-&#160;&#160;
			 * 			</td>
			 * 			<td>
			 * 			</td>
			 * 		</tr>
			 * 	</table>
			 */
			/**
			 * digital output from 01 to 08 in an 8 bit value
			 */
			int m_nDigitalOutput;
			/**
			 * analog output for chanel PWM1
			 */
			int m_nAnalogOutputC1;
			/**
			 * analog output for chanel PWM2
			 */
			int m_nAnalogOutputC2;
			/**
			 * digital input from I1 to I5 in an 5 bit value
			 */
			int m_nDigitalInput;
			/**
			 * analog input from chanel A1
			 */
			long m_nAnalogInputC1;
			/**
			 * analog input from chanel A2
			 */
			long m_nAnalogInputC2;
			/**
			 * digital input pins which was called since the last request
			 * displayed in an 5 bit value for 1 on and 0 off
			 */
			long m_ndRead;

	}; // class VellemannK8055

} // naqmespace ports

#endif //_K8055LIBRARY
#endif /*VELLEMANNK8055_H_*/
