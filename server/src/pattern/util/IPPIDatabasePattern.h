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

#include "../../util/structures.h"

using namespace std;

namespace design_pattern_world
{
	class IPPIDatabasePattern
	{
	public:
		/**
		 * read database
		 *
		 * @return whether can read or create database
		 */
		virtual bool read()= 0;
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
		 */
		virtual void writeIntoDb(const string folder, const string subroutine)= 0;
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
		virtual map<string, vector<string> >* getSubroutines(const string& onServer, const string& chip)= 0;
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
		 * @param subroutine mame of the subroutine in the folder
		 * @param values vector of value whitch should write into database
		 * @param bNew whether database should write only new values default= true
		 */
		virtual void fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew= true)= 0;
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
		 * @return error code for not correctly done
		 */
		virtual int execute()= 0;
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
