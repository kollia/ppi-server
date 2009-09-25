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

namespace server {

using namespace util;
using namespace design_pattern_world::server_pattern;

class OWInterface : public ProcessInterfaceTemplate {
public:
	/**
	 * save parameter static to interface
	 *
	 * @param process name of process in which the interface running
	 * @param connection to which server this interface should connect to send messages
	 */
	static void initial(const string& process, IClientConnectArtPattern* connection);
	/**
	 * get instance for OWServer
	 *
	 * @param serverID id of OWSerever to want reach
	 * @return correct and checkd instance
	 */
	static OWInterface* instance(const unsigned short serverID);
	/**
	 * set all debugging parameters to false
	 */
	static void clearDebug();
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
	 * name of process in which started
	 */
	static string m_sProcess;
	/**
	 * ConnectionArtPattern to which interface connect
	 */
	static IClientConnectArtPattern* m_pCon;
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
