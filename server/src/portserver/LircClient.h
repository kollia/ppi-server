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
#ifndef LIRCCLIENT_H_
#define LIRCCLIENT_H_

#include "../util/debug.h"

#include "lirc/lirc_client.h"

#include "../pattern/util/ipropertypattern.h"
#include "../pattern/server/ichipaccesspattern.h"

#include "../util/thread/Thread.h"

namespace ports
{
	/**
	 * class represent communication with LIRC.<br />
	 * to use compiling with lirc
	 * application need package liblircclient-dev
	 * and also library lirc_client with option -llirc_client
	 * by creating with make
	 *
	 * @autor Alexander Kolli
	 * @version 1.0.0.
	 */
	class LircClient : public IChipAccessPattern
	{
	public:
		/**
		 * constructor to declare object lirc
		 *
		 * @param ID port ID jumperd on K8055 platine
		 */
		LircClient() :
			m_bDebug(false),
			m_bConnected(false),
			m_READMUTEX(Thread::getMutex("READMUTEX")) {};
		/**
		 * display identification name for OWServer
		 *
		 * @return name of server
		 */
		virtual string getServerName()
		{ return "lircClient"; };
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
		 * return normaly the name of default config file for every chips or pins.<br />
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
		 * define which remote and code is used
		 *
		 * @param prop properties from current subroutine
		 * @param id this param get unique id of pin to identify reading or writing
		 * @param kernelmode give back whether owreader should read values over an kernel module. 0 for writing and 2 for reading
		 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
		 */
		virtual short useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmodule);
		/**
		 * function reading from chip
		 * and returning the type
		 *
		 * @param ID id from the chip which defined with getChipID
		 * @return string of type from chip
		 */
		virtual string getChipType(const string ID);
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
		virtual void setDebug(const bool debug)
		{ m_bDebug= debug; };
		/**
		 * get debug info to show
		 * on which cache be writing or reading
		 */
		virtual bool isDebug()
		{ return m_bDebug; };
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
		 * read values over an kernel module.<br />
		 * If some chips defined in method <code>useChip()</code> with kernelmode,
		 * owreader starts an second thread to call this method.
		 * This method should is thread save.
		 *
		 * @return changed id defined in <code>useChip()</code>
		 */
		virtual string kernelmodule();
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
		virtual void endOfLoop() {};
		/**
		 * desctuctor of object
		 */
		virtual ~LircClient();

	private:
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
		 * structure of main lirc configuration
		 */
		struct lirc_config *m_ptLircConfig;
		/**
		 * all defined codes in measure.conf
		 */
		map<string, vector<string> > m_mvCodes;
		/**
		 * map for id's whether server send actually an SEND command
		 */
		map<string, bool> m_mSend;
		/**
		 * all reading codes whether get from device
		 */
		map<string, bool> m_mset;
		/**
		 * mutex lock to set code
		 */
		pthread_mutex_t* m_READMUTEX;
	};
}

#endif /*LIRCCLIENT_H_*/
