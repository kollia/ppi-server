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
#ifndef EXTERNPORTS_H_
#define EXTERNPORTS_H_

#include "../util/debug.h"
#include "../util/structures.h"

#include <utility>
#include <string>
#include <map>

#include "../pattern/util/ipropertypattern.h"
#include "../pattern/server/ichipaccesspattern.h"

//* Variablenbelegung
#define ITIMERTYPE ITIMER_REAL
#define ITIMERSTARTSEC 120


#define COM1 0x3F8 //* Adresse COM1 *
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define LPT1 0x3BC // /dev/lp0 *
#define LPT2 0x378 // /dev/lp1 *
#define LPT3 0x278 // /dev/lp2 *

using namespace design_pattern_world;

namespace ports
{
	/**
	 * all ports as key which are opened
	 * and the current state as value
	 */
	extern map<unsigned long, int> global_mOpenedPorts;

	class ExternPorts : public IChipAccessPattern
	{
		public:
			enum Pin
			{
				NONE= 0,
				// COM-interface
				DTR,
				RTS,
				TXD,
				CTS,
				DSR,
				RI,
				DCD,

				// LPT-interface
				DATA1,
				DATA2,
				DATA3,
				DATA4,
				DATA5,
				DATA6,
				DATA7,
				DATA8,
				ERROR,
				SELECT,
				PAPEREMPTY,
				ACKNOWLEDGE,
				BUSY,
				STROBE,
				AUTOFEED,
				INIT,
				SELECTINPUT,

				// description
				SETPIN,
				GETPIN,
				GETSETPIN,

				// port
				COM,
				LPT
			};
			struct Pins
			{
				unsigned long nPort;
				Pin ePin;

				int operator == (const Pins &other) const
				{
					if(	nPort == other.nPort
						&&
						ePin == other.ePin	)
					{
						return 1;
					}
					return 0;
				};
				int operator<(const Pins &other) const
				{
					if(	nPort < other.nPort
						||
						(	nPort == other.nPort
							&&
							ePin < other.ePin	)	)
					{
						return 1;
					}
					return 0;
				};
			};
			struct portpin_address_t
			{
				unsigned long nPort;
				unsigned short nAdd;
				int nPin;
				Pin ePin;
				Pin eDescript;
				Pin ePort;
				bool bCacheWriting;

				// only for server type MPORT
				Pins eOut;
				Pins eNeg;
			};

			/**
			 * display identification name for OWServer
			 *
			 * @return name of server
			 */
			virtual string getServerName();
			/**
			 * constructor to declare object to read from all serial and parallel port interfaces
			 *
			 * @param openPorts all ports which should be opened
			 * @param type which type of port reading or writing is given
			 */
			ExternPorts(const vector<string>& openPorts, const PortTypes type);
			/**
			 * select all properties and actions whitch are uesed in interface
			 *
			 * @param properties reading properties from the main config file
			 */
			virtual void usePropActions(const IActionPropertyPattern* properties) const;
			/**
			 * first connection and initialization to the chips.<br />
			 *
			 * @param properties reading properties from the main config file
			 * @return whether the initialization was correct
			 */
			virtual bool init(const IPropertyPattern* properties);
			/**
			 * return normally the name of default config file for every chips or pins.<br />
			 * But method returning an null string, so no default config will be used
			 *
			 * @return name of file
			 */
			virtual string getDefaultFileName()
			{ return ""; };
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
			 * @param kernelmode give back whether owreader should read values over an kernel module (always 0)
			 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
			 */
			virtual short useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmode);
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
			 * @param type type of server (PORTS, OWFS, Vk8055, ...)
			 * @param id id of chip
			 * @return boolean whether id do exist
			 */
			virtual bool existID(const string servertype, const string id) const;
			/**
			 * calculate port address from given string COM1, COM2, ... or LPT1, LPT2, ...
			 * into defined address in server.conf
			 *
			 * @param sPortName port as string (COM1 or COM2 ...)
			 */
			unsigned long getPortAddress(const string& sPortName) const;
			/**
			 * return port type in an Pin enum (COM or LPT) if given string is an valid address.
			 * Elsewhere enum Pin is NONE
			 *
			 * @param sPortName port as string (COM1 or COM2 ...)
			 */
			Pin getPortType(const string& sPortName) const;
			/**
			 * return port type in an Pin enum (COM or LPT) if given string is an valid address.
			 * Elsewhere enum Pin is NONE
			 *
			 * @param port address of port
			 */
			Pin getPortType(const unsigned long port) const;
			/**
			 * return pin type in an Pin structure if given string is an valid pin name in an COM port.
			 * Elsewhere structure Pin is NONE
			 *
			 * @param sPinName pin as string (RTS, TXT, ...)
			 */
			Pin getCOMPinEnum(const string& sPinName) const;
			/**
			 * return pin type in an Pin structure if given string is an valid pin name in an LPT port.
			 * Elsewhere structure Pin is NONE
			 *
			 * @param sPinName pin as string (RTS, TXT, ...)
			 */
			Pin getLPTPinEnum(const string& sPinName) const;
			/**
			 * return structure of port address and pin enum as Pin, if valid names are given.
			 * Elsewhere in Pins structure address is 0 and Pin enum is NONE.
			 *
			 * @param sPortName port as string (COM1, COM2 ... or LPT1, LPT2 ...)
			 * @param sPinName pin as string (RTS, TXT, ...)
			 */
			Pins getPinsStruct(const string& sPort, const string& sPin) const;
			/**
			 * calculate from an Pins structure all hexa byte addresses from pins and ports
			 *
			 * @param tAdr defined Pins structure
			 * @param bSetAfter whether pin should be set or unset for an set pin (SETPIN) or should measure also after contact by GETPIN
			 */
			portpin_address_t getPortPinAddress(const Pins& tAdr, const bool bSetAfter) const;
			/**
			 * calculate from an Pins structure all hexa byte addresses from pins and ports
			 *
			 * @param sPortName port as string (COM1, COM2 ... or LPT1, LPT2 ...)
			 * @param sPinName pin as string (RTS, TXT, ...)
			 * @param bSetAfter whether pin should be set or unset for an set pin (SETPIN) or should measure also after contact by GETPIN
			 */
			portpin_address_t getPortPinAddress(const string& sPort, const string& sPin, const bool bSetAfter) const;
			/**
			 * define binary string
			 *
			 * @param value pin which should set as binary string
			 * @param bits count of bits set in string
			 * @return binary string
			 */
			string getBinString(const long value, const size_t bits) const;
			/**
			 * display on command line an binary string in 8 bits
			 * and also the port in an hex string
			 *
			 * @param value pin which should set as binary string
			 * @param nPort port value for display
			 */
			void printBin(const int* value, const unsigned long nPort) const;
			/**
			 * lock or unlock process to run only this thread alone on CPU
			 *
			 * @param bSet whether thread should lock or unlock
			 */
			void lockProcess(const bool bSet);
			/**
			 * return actual micro-time
			 *
			 * @return micro-time
			 */
			unsigned long getMikrotime();
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
			 * set or remove the given pin in the external port
			 *
			 * @param tPin port and pin address which should set or remove
			 * @param bSet whether pin should be set or remove
			 * @return whether writing was correct
			 */
			bool setPin(const portpin_address_t& tPin, const bool bSet);
			/**
			 * read given pin on external port
			 *
			 * @param tSet address of port and pin
			 * @return 1 if pin set 0 for removed or -1 by error
			 */
			short getPin(const portpin_address_t& tSet);
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
			 * this method is an dummy, because no kernelmode in <code>useChip()</code> be set.
			 *
			 * @return only an null string ("")
			 */
			virtual string kernelmodule() { return ""; };
			/**
			 * read the time from power send into pin of out and get in the defined pin
			 *
			 * @param id only for TIMELOG by overflow
			 * @return how long the power needs
			 */
			unsigned long getMeasuredTime(const string& id);
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
			virtual ~ExternPorts();

		private:
			/**
			 * mutex lock of show debug info
			 */
			pthread_mutex_t* m_DEBUGINFO;
			/**
			 * addresses of all defined parallel (COM) ports in server.conf
			 */
			map<string, pair<unsigned long, string> > m_mCOMPorts;
			/**
			 * addresses of all defined serial (LPT) ports in server.conf
			 */
			map<string, pair<unsigned long, string> > m_mLPTPorts;
			/**
			 * all ports and pins are used refered ID as key
			 */
			map<string, portpin_address_t> m_mUsedPins;
			/**
			 * all extern ports which should be open
			 */
			vector<string> m_vOpenPorts;
			/**
			 * whether should shown debug info
			 * for which cache be reading
			 */
			bool m_bDebug;
			/**
			 * whether an connection is made correctly
			 */
			bool m_bConnected;
			/**
			 * for which type of reading or writing the server pattern should exist
			 */
			PortTypes m_eType;
			/**
			 * whether the server is set to use an port.<br />
			 * This variable only be set for MPORT and RWPORT
			 */
			bool m_bSet;
			/**
			 * whether should freeze scheduling by measure on MPORT
			 */
			bool m_bFreeze;
			/**
			 * maximal measure time for port type MPORT
			 */
			unsigned long m_maxMeasuredTime;

	}; // class VellemannK8055

} // naqmespace ports

#endif /*EXTERNPORTS_H_*/
