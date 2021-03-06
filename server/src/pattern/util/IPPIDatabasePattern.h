/**
 *   This file 'IppiDatabasePattern.h' is part of ppi-server.
 *   Created on: 26.03.2011
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


#ifndef IPPIDATABASEPATTERN_H_
#define IPPIDATABASEPATTERN_H_

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "../../util/properties/measureStructures.h"

#include "IDbgSessionPattern.h"

/*
 * define showSendingCount to 1
 * when want to see on command line
 * how much folders with subroutine #start
 * DbFiller send to database,
 * get database from ppi-server working list
 * and get ppi-internet-server from database
 * (seen by sending second hold or DEBUG from ppi-client)
 * define to 2
 * when want to see how much
 * debug sessions will be sending
 * currently info messages by one pass to database
 * and define to 3 by both
 */
#define __showSendingCount 0

using namespace std;

namespace design_pattern_world
{
	using namespace util_pattern;

	/**
	 * struct of nessesary items
	 * to write into database
	 */
	struct db_t
	{
		/**
		 * host on which measureing be done
		 */
		string measureHost;
		/**
		 * time of insert
		 */
		timeval tm;
		/**
		 * name of folder in which thread running
		 */
		string folder;
		/**
		 * subroutine in folder which had called instraction
		 */
		string subroutine;
		/**
		 * identifier of value
		 */
		string identif;
		/**
		 * whether subroutine has correct access to device by reading or writing
		 */
		bool device;
		/**
		 * values which should insert into database
		 */
		vector<double> values;
		/**
		 * whether database should write only new values
		 */
		bool bNew;
		/**
		 * greater operator
		 */
		bool operator > (const db_t* other) const
		{
			if(folder > other->folder)
				return true;
			if(subroutine > other->subroutine)
				return true;
			return identif > other->identif;
		}
		/**
		 * is same operator
		 */
		bool operator == (const db_t* other) const
		{
			return (	folder == other->folder &&
						subroutine == other->subroutine &&
						identif == other->identif			);
		}
	};

	class IPPIDatabasePattern
	{
	public:
		/*
		 * same three typedef definitions
		 * like IDbFillerPattern
		 */
		/**
		 * definition of debug session subroutine
		 * with subroutine name and time
		 */
		typedef pair<ppi_time, string > debugSessionSubroutine;
		/**
		 * definition of debug session map for subroutines
		 * with subroutine name, time and content
		 */
		typedef map<debugSessionSubroutine, IDbgSessionPattern::dbgSubroutineContent_t > debugSessionSubroutineMap;
		/**
		 * definition of debug session map for folders
		 * with folder name, subroutine name, time and content
		 */
		typedef map<string, debugSessionSubroutineMap> debugSessionFolderMap;

		/**
		 * method for starting ppi-server to define on which level
		 * position the server is
		 *
		 * @param sProcess name of process current starting.<br />when server is started string should be &quote;finished&quote;
		 * @param nPercent how much percent of process level be done
		 */
		virtual void setServerConfigureStatus(const string& sProcess, const short& nPercent)= 0;
		/**
		 * whether ppi-server is configured
		 *
		 * @param sProcess gives back name of process current starting
		 * @param nPercent gives back percent of process level be done
		 * @return whether server was started
		 */
		virtual bool isServerConfigured(string& sProcess, short& nPercent)= 0;
		/**
		 * read database
		 *
		 * @param load show loading progress on command line
		 * @return whether can read or create database
		 */
		virtual bool read(bool load= false)= 0;
		/**
		 * read all actually state of subroutine inside given subroutines fromsub with value from
		 * to subroutine tosub and value to
		 *
		 * @param fromsub subroutine from which should be read
		 * @param from value from subroutine by begin of reading
		 * @param tosub subroutine by which should ending
		 * @param to value of subroutine by ending
		 * @param ncount size of needed blocks
		 */
		virtual void readInsideSubroutine(const string& fromsub, const double from, const string& tosub,
						const double to, const unsigned short ncount)= 0;
		/**
		 * read all content from database
		 * to get with method <code>getReadSubBlock()</code>
		 */
		virtual void readAllContent()= 0;
		/**
		 * read all actually state of subroutine inside value from and to
		 *
		 * @param sub subroutine for definition to begin end ending
		 * @param from value of subroutine to begin
		 * @param to value of subroutine to end
		 * @param ncount size of needed blocks
		 */
		virtual void readInsideSubroutine(const string& sub, const double from, const double to, const unsigned short ncount)= 0;
		/**
		 * return read block of subroutine from before defined reading of begin and end
		 *
		 * @return block of subroutines
		 */
		virtual vector<vector<db_t> > getReadSubBlock()= 0;
		/**
		 * define which chip OwPort will be use to inform when content on server (owreader) change value
		 *
		 * @param folder name of folder in which use
		 * @param subroutine name of subroutine in which use
		 * @param onServer name of server from which is using chip
		 * @param chip unique definition of chip
		 */
		virtual void useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip)= 0;
		/**
		 * function set, whether need value in database,
		 * or will be only set to inform clients for changes
		 *
		 * @param folder name of folder from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 * @param identif identification of writing value
		 */
		virtual void writeIntoDb(const string& folder, const string& subroutine, const string& identif= "value")= 0;
		/**
		 * database service for which changes in subroutines are needed.<br />
		 * this method define for any specific client which subroutines in folder it does
		 * need to know it is changed.
		 *
		 * @param connection id of server connection with client
		 * @param name folder and subroutine which are needed, or string 'newentry' if need an new pool of subroutines
		 * @return whether the subroutine exists
		 */
		virtual bool needSubroutines(unsigned long connection, string name)= 0;
		/**
		 * return all folder with subroutines which are use the defined chip
		 *
		 * @param onServer on which server the changed chip will be
		 * @param chip the unique define chip
		 * @return map of folder with subroutines
		 */
		virtual map<string, set<string> >* getSubroutines(const string& onServer, const string& chip)= 0;
		/**
		 * change the communication ID to an new one
		 *
		 * @param oldId old communication ID
		 * @param newId new communication ID
		 */
		virtual void changeNeededIds(unsigned long oldId, unsigned long newId)= 0;
		/**
		 * ask whether entry of folder:subroutine exist
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @return 4 when all exists, 3 when number of value not exist, 2 when value with identifier not exist, 1 if subroutine not exist and 0 when folder not exist
		 */
		virtual unsigned short existEntry(const string& folder, const string& subroutine, const string& identif,
																		const vector<double>::size_type number= 0)= 0;
		/**
		 * returns actual value of subroutine in folder
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @return current value of given parameters
		 */
		virtual auto_ptr<double> getActEntry(const string& folder, const string& subroutine, const string& identif,
																			const vector<double>::size_type number= 0)= 0;
		/**
		 * fill double values into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param values vector of value which should write into database
		 * @param bNew whether database should write only new values default= true
		 */
		virtual void fillValue(string folder, string subroutine, string identif,
						vector<double> values, bool bNew= true)= 0;
		/**
		 * fill debug session output from folder working list
		 * into database
		 *
		 * @param content structure of folder:subroutine data from debugging session
		 */
		virtual void fillDebugSession(const IDbgSessionPattern::dbgSubroutineContent_t& content)= 0;
		/**
		 * return queue of hole written debug sessions
		 *
		 * @return debug sessions
		 */
		virtual std::auto_ptr<debugSessionFolderMap> getDebugSessionQueue()= 0;
		/**
		 * whether one or more entry's are changed.<br />
		 * method hold thread in process and return without any result
		 * when any entry was changed
		 */
		virtual void isEntryChanged()= 0;
		/**
		 * arouse all condition witch wait of any changes in database.<br />
		 * This method will be called when any connection from Internet process will be break,
		 * to ending also this asking connection.
		 */
		virtual void arouseChangingPoolCondition()= 0;
		/**
		 * database service for all changes in subroutines.<br />
		 * this method returning all changes which are defined with <code>needSubroutines()</code>
		 *
		 * @param connection id of server connection with client
		 * @return all changed values
		 */
		virtual vector<string> getChangedEntrys(unsigned long connection)= 0;
		/**
		 * returns two convert values which have between the given value
		 *
		 * @param subroutine for which folder:subroutine the two values should be
		 * @param definition defined name in the database
		 * @param value the value which should be between the returned two values
		 * @return vector of two convert values
		 */
		virtual vector<convert_t> getNearest(string subroutine, string definition, double value)= 0;
		/**
		 * method to running thread .<br />
		 * This method starting again when method stop() wasn't call.
		 *
		 * @return whether thread should starting again
		 */
		virtual bool execute()= 0;
		/**
		 * stop working of database.<br />
		 * Arose all conditions to ending method <code>execute()</code>
		 */
		virtual bool stop()= 0;
		/**
		 * method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending()= 0;
		/**
		 * dummy destruktor for design pattern
		 */
		virtual ~IPPIDatabasePattern() {};
	};
}

#endif /* IPPIDATABASEPATTERN_H_ */
