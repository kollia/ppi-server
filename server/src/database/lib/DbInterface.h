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

#include "../../pattern/util/IMeasureSet.h"

#include "../../util/properties/measureStructures.h"

#include "../../server/libs/client/ProcessInterfaceTemplate.h"

#include "../logger/lib/LogInterface.h"

using namespace std;
using namespace util;
using namespace logger;

namespace ppi_database
{

	/**
	 * class representig connection to internal
	 * or an mySql database
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class DbInterface : public LogInterface,
						public ProcessInterfaceTemplate,
						public IMeasureSet
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
		 * method for starting ppi-server to define on which level
		 * position the server is
		 *
		 * @param sProcess name of process current starting.<br />when server is started string should be &quote;finished&quote;
		 * @param nPercent how much percent of process level be done
		 */
		virtual void setServerConfigureStatus(const string& sProcess, const short& nPercent);
		/**
		 * whether ppi-server is configured
		 *
		 * @param sProcess gives back name of process current starting
		 * @param nPercent gives back percent of process level be done
		 * @return whether server was started
		 */
		virtual bool isServerConfigured(string& sProcess, short& nPercent);
		/**
		 * Instantiate for DbProcess.<br/>
		 * By cancel this DbInterface object, second parameter object will be also delete in parent class.
		 *
		 * @param process name of process in which the log-interface running
		 * @param connection to which server this interface should connect to send messages
		 * @param identifwait how many seconds an log with an identif string should wait to write again into the log file
		 * @return return number of instance, elsewhere by error -1
		 */
		static short initial(const string& process, IClientConnectArtPattern* connection, const int identifwait);
		/**
		 * return instance of database object
		 *
		 * @param number created interface number (default 0 is first)
		 * @return database object
		 */
		static DbInterface* instance(const short number= 0);
		/**
		 * open the connection to server for sending questions
		 *
		 * @param toopen string for open question, otherwise by null the connection will be open with '<process>:<client> SEND' for connect with an ServerMethodTransaction
		 * @return object of error handling
		 */
		OVERWRITE EHObj openConnection(string toopen= "");
		/**
		 * return used connection for sending requests
		 *
		 * @return connection object
		 */
		IClientConnectArtPattern* getConnection()
		{ return ProcessInterfaceTemplate::getSendConnection(); };
		/**
		 * delete all objects of database interface
		 */
		static void deleteAll();
		/**
		 * sending method parameter directly to database process
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param answer whether client should wait for answer
		 * @return backward send return value from server if answer is true, elsewhere returning null string
		 */
		virtual string sendMethod(const string& toProcess, const OMethodStringStream& method, const bool answer= true)
		{ return ExternClientInputTemplate::sendMethod(toProcess, method, answer); };
		/**
		 * sending method parameter directly to database process
		 *
		 * @param toProcess for which process the method should be
		 * @param method object of method which is sending to server
		 * @param done on which getting string the answer should ending. Ending also when an ERROR or warning occurs
		 * @param answer whether client should wait for answer
		 * @return backward send return string vector from server if answer is true, elsewhere returning vector with no size
		 */
		virtual vector<string> sendMethod(const string& toProcess, const OMethodStringStream& method, const string& done, const bool answer= true)
		{ return ExternClientInputTemplate::sendMethod(toProcess, method, done, answer); };
		/**
		 * method to ask database whether database file is loaded
		 *
		 * @return whether file is loaded
		 */
		bool isDbLoaded();
		/**
		 * method to ask whether all owreader interface are finished
		 * by initialization
		 *
		 * @return string of &quote:done&quote: when all owreader initialed, name of owreader which is busy by initialing
		 * 			or string of &quote:false&quote: when owreader cannot initialed correctly
		 */
		string allOwreadersInitialed();
		/**
		 * function set, whether need value in database,
		 * or will be only set to inform clients for changes
		 *
		 * @param folder name of folder from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 * @param identif identification of writing value
		 */
		void writeIntoDb(const string& folder, const string& subroutine, const string& identif= "value");
		/**
		 * set double value into measure list
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
	 * @param value value which should write into database with last changing time when set, otherwise method iside list obect from ppi-server create own time
		 * @param account from which account over Internet the value will be set
		 * @return whether subroutine can be set correctly
		 */
		virtual bool setValue(const string& folder, const string& subroutine,
						const IValueHolderPattern& value, const string& account);
		/**
		 * get double value into measure list
		 *
		 * @param noexist returning error value<br />
		 *                 0 when value is ok, -1 when currently no access to value given, -1 when subroutine isn't define correctly,
		 *                -2 when subroutine not exist and -3 when folder not exist
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param account from which account over Internet the value will be set
		 * @return double value from subroutine
		 */
		virtual double getFolderValue(short& noexist, const string& folder, const string& subroutine, const string& account);
		/**
		 * fill double value into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param value value which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		void fillValue(const string& folder, const string& subroutine, const string& identif,
						double value, bool bNew= false);
		/**
		 * fill double values into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param values vector of value which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		void fillValue(const string& folder, const string& subroutine, const string& identif, const vector<double>& values, bool bNew= false);
		/**
		 * ask whether entry of folder:subroutine exist
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @return 5 when device exists correctly, 4 when no physical access exist, 3 when number of value count not exist, 2 when value with identifier not exist, 1 if subroutine not exist and 0 when folder not exist
		 */
		unsigned short existEntry(const string& folder, const string& subroutine, const string& identif,
																const vector<double>::size_type number= 0);
		/**
		 * ask whether folder exist
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return (true) when folder and subroutine exist or folder exist and subroutine be an null string, otherwise false
		 */
		bool existSubroutine(const string& folder, const string& subroutine= "");
		/**
		 * set hole folder to output state meassage
		 *
		 * @param debug whether folder:subroutine should debug or stop debugging
		 * @param bInform whether need by existing inform parameter this also by output
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 */
		void debugSubroutine(bool debug, bool bInform, const string& folder, const string& subroutine= "");
		/**
		 * show on command line which folder threads how often running
		 *
		 * @param seconds how much seconds server should wait for read count
		 * @param bClient whether server should wait before counting for client action
		 */
		void showThreads(int seconds, bool bClient);
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
		void registerPortID(const string& folder, const string& subroutine, const string& onServer, const string& chip);
		//void useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip);
		/**
		 * inform database for changed chip and database inform subroutine
		 *
		 * @param onServer on which server (owreader) the chip was changed
		 * @param chip which unique chip was changed
		 * @param value new value for chip
		 * @param device whether chip can be reach correctly
		 * @param reader from which ppi-reader chip was changed
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
		double getRegisteredDefaultChipCache(const string& server, const string& chip, bool& exist);
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
		chips_t getRegisteredDefaultChip(const string& server, const string& family, const string& type, const string& chip, const string& pin);
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
		double getDefaultCache(const double min, const double max, const bool bFloat, const string& folder= "", const string& subroutine= "");
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
		 * @param identifwait how many seconds an log with an identif string should wait to write again into the log file
		 */
		DbInterface(const string& process, IClientConnectArtPattern* connection, const int identifwait)
		: LogInterface("ppi-db-server", identifwait, true),
		  ProcessInterfaceTemplate(process, "DbInterface", "DatabaseServer", connection, NULL)
		{ };

	private:
		/**
		 * single instance of DbInterface
		 */
		static map<short, DbInterface*> _instance;

		/**
		 * return used connection for sending requests
		 *
		 * @return connection object
		 */
		IClientConnectArtPattern* getSendConnection()
		{ return ProcessInterfaceTemplate::getSendConnection(); };
		/**
		 * return used connection for getting requests
		 *
		 * @return connection object
		 */
		IClientConnectArtPattern* getGetConnection()
		{ return ProcessInterfaceTemplate::getGetConnection(); };
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
		chips_t getRegisteredDefaultChipA(bool bAll, const string& server, const string& family, const string& type, const string& chip, const string& pin);

	};

}

#endif /*DBINTERFACE_H_*/
