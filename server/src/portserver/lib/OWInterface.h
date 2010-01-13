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

#ifndef OWINTERFACE_H_
#define OWINTERFACE_H_

#include <vector>
#include <string>

#include "../../util/ProcessInterfaceTemplate.h"

#include "../../pattern/server/IClientConnectArtPattern.h"
#include "../../pattern/util/iactionpropertypattern.h"

namespace server {

using namespace util;
using namespace design_pattern_world;
using namespace design_pattern_world::server_pattern;

class OWInterface : public ProcessInterfaceTemplate {
public:
	/**
	 * get instance for OWServer.<br />
	 * Instnace have to be exist, otherwise create first the instance with <code>getServer(processName, connection, serverID)</code>.
	 *
	 * @param serverID id of OWSerever to want reach
	 * @return correct and checkd instance
	 */
	static OWInterface* getServer(const unsigned short serverID);
	/**
	 * get instance of OWServer and create if not exist.
	 *
	 * @param process name of process in which the interface running
	 * @param connection to which server this interface should connect to send messages
	 * @param serverID id of OWSerever to want reach
	 * @return correct and checkd instance
	 */
	static OWInterface* getServer(const string& process, IClientConnectArtPattern* connection, const unsigned short serverID);
	/**
	 * method returning the right server for an specific chip ID.<br />
	 * All OWServer instaces be defined in an vector in the constructor
	 *
	 * @param type type of server (OWFS, Vk8055, ...)
	 * @param chipID specific ID which the server should holded
	 * @return the server instance
	 */
	static OWInterface* getServer(const string& type, const string& chipID);
	/**
	 * display identification name for OWServer
	 *
	 * @return name of server
	 */
	string getServerName();
	/**
	 * all initialication of subroutines
	 * are done
	 *
	 * @param maxServer count of running one wire reader
	 */
	static void endOfInitialisation(const int maxServer);
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
	 * @param ID XX.XXXXXXXXXX id from the chip beginning with two character family code
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
	 * set debug parameter forOWServer benchmark
	 *
	 * @param set whether debug parameter should be set
	 */
	void setDebug(bool set);
	/**
	 * debug info for defined OWServer for benchmark
	 *
	 * @return benchmark strings
	 */
	vector<string> getDebugInfo();
	/**
	 * select all properties and actions which are used in interface
	 *
	 * @param properties reading properties from the main configuration file
	 */
	void usePropActions(const IActionPropertyPattern* properties);
	/**
	 * write to chip direct if pin is set to uncached,<br />
	 * or write into cache for writing later
	 *
	 * @param id unique ID of pin on chip
	 * @param value Value which should be writing
	 * @return whether writing on the device was by the last one correct
	 */
	bool write(const string& id, const double value);
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
	bool haveToBeRegistered();
	/**
	 * define dallas chip is used
	 *
	 * @param properties IActionProperyPattern from current subroutine
	 * @param return value of unique chip ID
	 * @param folder in which folder the chip is used
	 * @param subroutine in which subroutine the chip is used
	 * @return 0 if the pin is not correct, 1 if the pin is for reading, 2 for writing or 3 for unknown (reading/writing)
	 */
	short useChip(IActionPropertyPattern* properties, string& unique, const string& folder, const string& subroutine);
	/**
	 * check whether all exist id's are used
	 *
	 * @param maxServer count of running one wire reader
	 */
	static void checkUnused(const int maxServer);
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
	 * delete all created instances
	 */
	void deleteAll();
	/**
	 * dummy destructor of OWInterface
	 */
	virtual ~OWInterface() {};

protected:
	/**
	 * creating instance of OWInterface
	 *
	 * @param process name of process in which the interface running
	 * @param serverID id of OWServer to want reach
	 * @param connection to which server this interface should connect to send messages
	 */
	OWInterface(const string& process, const string& serverID, IClientConnectArtPattern* connection)
	:	ProcessInterfaceTemplate(process, "OWInterface", "OwServerQuestion-"+serverID, connection, NULL),
		m_stoClient("OwServerQuestion-"+serverID)
	{ };

private:
	/**
	 * all instances of OWInterface for created to OWServer with ID
	 */
	static map<unsigned short, OWInterface*> _instances;
	/**
	 * name of server to which should send questions
	 */
	string m_stoClient;
};

}

#endif /* OWINTERFACE_H_ */
