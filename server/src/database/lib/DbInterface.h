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
#ifndef DBINTERFACE_H_
#define DBINTERFACE_H_

#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "../../util/structures.h"
#include "../../util/ProcessInterfaceTemplate.h"

using namespace std;
using namespace util;

namespace ppi_database
{

	/**
	 * class representig connection to internal
	 * or an mySql database
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class DbInterface : public ProcessInterfaceTemplate
	{
	public:
		/**
		 * structure of chip id's
		 * from DefaultChipConfigReader without pointer variable to otime_t
		 */
		struct chips_t {
			bool exists;
			string server;
			string family;
			string type;
			string id;
			string pin;
			double dmin;
			double dmax;
			bool bFloat;
			double dCache;
			bool bWritable;
			vector<double> errorcode;
		};

		/**
		 * Instantiate for DbProcess.<br/>
		 * By cancel this DbInterface object, second parameter object will be also delete in parent class.
		 *
		 * @param process name of process in which the log-interface running
		 * @param connection to which server this interface should connect to send messages
		 * @return return number of instance, elsewhere by error -1
		 */
		static short initial(const string& process, IClientConnectArtPattern* connection);
		/**
		 * return instance of database object
		 *
		 * @param number created interface number (default 0 is first)
		 * @return database object
		 */
		static DbInterface* instance(const short number= 0);
		/**
		 * delete all objects of database interface
		 */
		static void deleteAll();
		/**
		 * method to ask database whether database file is loaded
		 *
		 * @return wheter file is loaded
		 */
		bool isDbLoaded();
		/**
		 * function set, whether need value in database,
		 * or will be only set to inform clients for changes
		 *
		 * @param folder name of folder from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 */
		void writeIntoDb(const string folder, const string subroutine);
		/**
		 * set double value into measure list
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 * @param value value whitch should write into database
		 */
		void setValue(string folder, string subroutine, double value);
		/**
		 * fill double value into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param value value whitch should write into database
		 * @param bNew whether database should write only new values default= true
		 */
		void fillValue(string folder, string subroutine, string identif, double value, bool bNew= true);
		/**
		 * fill double values into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 * @param values vector of value whitch should write into database
		 * @param bNew whether database should write only new values default= true
		 */
		void fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew= true);
		/**
		 * ask whether entry of folder:subroutine exist
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @return 4 when all exists, 3 when number of value not exist, 2 when value with identifier not exist, 1 if subroutine not exist and 0 when folder not exist
		 */
		unsigned short existEntry(const string& folder, const string& subroutine, const string& identif,
																const vector<double>::size_type number= 0);
		/**
		 * ask whether folder exist
		 *
		 * @param folder name of folder
		 * @return whether folder exist
		 */
		bool existFolder(const string& folder);
		/**
		 * set hole folder to output state meassage
		 *
		 * @param folder name of folder
		 */
		void debugFolder(const string& folder);
		/**
		 * clear all debug states from any folders,
		 * which should write output messages
		 */
		void clearFolderDebug();
		/**
		 * returns actual value of subroutine in folder
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @param exist whether asking value for subroutine exist in the database
		 * @return current value of given parameters
		 */
		double getActEntry(bool& exist, const string& folder, const string& subroutine, const string& identif,
																	const vector<double>::size_type number= 0);
		/**
		 * returns two convert values which have between the given value
		 *
		 * @param subroutine for which folder:subroutine the two values should be
		 * @param definition defined name in the database
		 * @param value the value which should be between the returned two values
		 * @return vector of two convert values
		 */
		vector<convert_t> getNearest(string subroutine, string definition, double value);
		/**
		 * database service for which changes in subroutines are needed.<br />
		 * this method define for any specific client which subroutines in folder it does
		 * need to know it is changed.
		 *
		 * @param connection id of server connection with client
		 * @param name folder and subroutine which are needed, or string 'newentry' if need an new pool of subroutines
		 * @return whether the subroutine exists
		 */
		bool needSubroutines(unsigned long connection, string name);
		/**
		 * whether one or more entrys are changed
		 * and hold thread in process
		 *
		 * @return strings of changed, stop or error number
		 */
		string isEntryChanged();
		/**
		 * database service for all changes in subroutines.<br />
		 * this method returning all changes which are defined with <code>needSubroutines()</code>
		 *
		 * @param connection id of server connection with client
		 * @return all changed values
		 */
		vector<string> getChangedEntrys(unsigned long connection);
		/**
		 * change the communication ID to an new one
		 *
		 * @param oldId old communication ID
		 * @param newId new communication ID
		 */
		void changeNeededIds(unsigned long oldId, unsigned long newId);
		/**
		 * describe whether all chips for folder and subroutine are defined to allow thin older database
		 *
		 * @param defined whether chips defined
		 */
		void chipsDefined(const bool defined);
		/**
		 * define for all OWServer the name of the default-file in DefaultChipConfigReader
		 *
		 * @param server name of OWServer
		 * @param config name of default config-file
		 */
		void define(const string& server, const string& config);
		/**
		 * register the used chip in DefaultChipConfigReader from defined chip out of default.conf if set,
		 * elswhere if pdmin be defined, create an new chip
		 *
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param pin witch pin inside the chip is used
		 * @param type specified type of chip
		 * @param family specified family code of chip
		 * @param pdmin minimal value witch chip can has
		 * @param pdmax maximal value witch chip can has
		 * @param pbFloat whether chip can has floating values
		 * @param pdCache which default cache the pin of chip have
		 */
		void registerChip(const string& server, const string& chip, const string& pin, const string& type, const string& family,
							const double* pdmin= NULL, const double* pdmax= NULL, const bool* pbFloat= NULL, const double* pdCache= NULL);
		/**
		 * define which chip OwPort will be use to inform when content change value
		 *
		 * @param folder name of folder in which use
		 * @param subroutine name of subroutine in which use
		 * @param onServer name of server from which is using chip
		 * @param chip unique definition of chip
		 */
		void useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip);
		/**
		 * inform database for changed chip and database inform subroutine
		 *
		 * @param onServer on which server (owreader) the chip was changed
		 * @param chip which unique chip was changed
		 * @param value new value for chip
		 * @param device whether chip can be reach correctly
		 */
		void changedChip(const string& onServer, const string& chip, const double value, const bool device);
		/**
		 * register chip-id with pin for folder and subroutine
		 * in DefaultChipConfigReader
		 *
		 * @param subroutine name of subroutine
		 * @param folder name of folder
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param pin witch pin inside the chip is used (default is standard pin defined in default.conf of server)
		 */
		void registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip);
		/**
		 * return cache value from registered default chip in DefaultChipConfigReader of unique chip ID and server.
		 *
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param whether default chip exists in DefaultChipConfigReader
		 */
		const double getRegisteredDefaultChipCache(const string& server, const string& chip, bool& exist);
		/**
		 * return structure from registered default chip in DefaultChipConfigReader of unique chip ID and server.
		 *
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param whether default chip exists in DefaultChipConfigReader
		 */
		chips_t getRegisteredDefaultChip(const string& server, const string& chip);
		/**
		 * return registered default chip from DefaultChipConfigReader if exist.
		 * beginning search on backward parameter chip, type and than family
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		chips_t getRegisteredDefaultChip(const string& server, const string& family, const string& type, const string& chip);
		/**
		 * return the defined default cache from default.conf.<br />
		 * folder and subroutine set not be, or else both.<br />
		 * Method search the default values inside of given range.
		 * It found when the difference of <code>min</code> and <code>max</code> be smaller than in the default.conf.
		 * If <code>float</code> is false, the default must be also false or can be true if no false be set.
		 *
		 * @param min minimum of range
		 * @param max maximum of range
		 * @param bFloat whether the value in the subroutine can be an floating point variable
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return default values with defined older_t structure
		 */
		const double getDefaultCache(const double min, const double max, const bool bFloat, const string& folder= "", const string& subroutine= "");
		/**
		 * return status message from all running processes without internet-server
		 *
		 * @param param parameter for which message
		 * @return status message
		 */
		vector<string> getStatusInfo(const string& param);
		/**
		 * check whether one wire server does exist
		 *
		 * @param sID ID of one wire server
		 * @return whether reader exist
		 */
		bool existOWServer(const unsigned short ID);
		/**
		 * set one wire server to read debug info
		 *
		 * @param serverID ID of one wire server
		 * @param connectionID ID of connection in internet-server to hold debugging open for more users
		 * @param set whether debug should set or not
		 */
		void setOWDebug(const unsigned short serverID, const unsigned int connectionID, const bool set);
		/**
		 * lift all one wire debugging for the given connection
		 *
		 * @param connectionID ID of connection in internet-server to know which all debugging should lift
		 */
		void clearOWDebug(const unsigned int connectionID);
		/**
		 * debug info for defined OWServer for benchmark
		 *
		 * @param ID ID of OWServer
		 * @return benchmark strings
		 */
		vector<string> getOWDebugInfo(const unsigned short ID);
		/**
		 * send stop-all command to ProcessChecker and stop also own database
		 */
		string stopall();
		/**
		 * destruct of Database
		 */
		virtual ~DbInterface()
		{ closeSendConnection(); };

	protected:
		/**
		 * creating instance of DbInterface.<br/>
		 * By cancel this DbInterface object, second parameter object will be also delete in parent class.
		 *
		 * @param process name of process in which the log-interface running
		 * @param connection to which server this interface should connect to send messages
		 */
		DbInterface(const string& process, IClientConnectArtPattern* connection)
		: ProcessInterfaceTemplate(process, "DbInterface", "DatabaseServer", connection, NULL)
		{ };

	private:
		/**
		 * single instance of DbInterface
		 */
		static map<short, DbInterface*> _instance;
		/**
		 * return registered default chip from DefaultChipConfigReader if exist.
		 * beginning search on backward parameter chip, type and than family
		 *
		 * qparam bAll whether should ask DefaultChipConfigReader only with values from server and chip by false, or with all values by true
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		chips_t getRegisteredDefaultChipA(bool bAll, const string& server, const string& family, const string& type, const string& chip);

	};

}

#endif /*DBINTERFACE_H_*/
