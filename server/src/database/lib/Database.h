/**
 *   This file 'Database.h' is part of ppi-server.
 *   Created on: 22.03.2011
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

#ifndef DATABASE_H_
#define DATABASE_H_

#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include "../../pattern/util/ipropertypattern.h"
#include "../../pattern/util/IPPIDatabasePattern.h"
#include "../../pattern/util/IChipConfigReaderPattern.h"

#include "../../util/structures.h"
#include "../../util/debug.h"
#include "../../util/URL.h"
#include "../../util/thread/Thread.h"

using namespace std;
using namespace design_pattern_world;

namespace ppi_database
{
	using namespace design_pattern_world;

	class Database : public IPPIDatabasePattern
	{
	public:
		/**
		 * constructor to set working directory of database
		 *
		 * @param properties server properties (server.conf) to know which database is to create
		 * @param chipreader reference to default chip reader to know on which time the value can be deleted from database.<br />
		 * 					 If only needed actually state of database (no execute for working will be needed), this reference can be NULL.
		 */
		Database(IPropertyPattern* properties, IChipConfigReaderPattern* chipreader);
		/**
		 * read database on hard disk
		 *
		 * @return whether can read or create database
		 */
		virtual bool read();
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
						const double to, const unsigned short ncount);
		/**
		 * return read block of subroutine from before defined reading of begin and end.
		 *
		 * @return block of subroutines
		 */
		virtual vector<vector<db_t> > getReadSubBlock()
				{ return m_vvDbEntrys; };
		/**
		 * read all actually state of subroutine inside value from and to
		 *
		 * @param sub subroutine for definition to begin end ending
		 * @param from value of subroutine to begin
		 * @param to value of subroutine to end
		 * @param ncount size of needed blocks
		 */
		virtual void readInsideSubroutine(const string& sub, const double from, const double to, const unsigned short ncount)
		{ readInsideSubroutine(sub, from, sub, to, ncount); };
		/**
		 * define which chip OwPort will be use to inform when content on server (owreader) change value
		 *
		 * @param folder name of folder in which use
		 * @param subroutine name of subroutine in which use
		 * @param onServer name of server from which is using chip
		 * @param chip unique definition of chip
		 */
		virtual void useChip(const string& folder, const string& subroutine, const string& onServer, const string& chip);
		/**
		 * function set, whether need value in database,
		 * or will be only set to inform clients for changes
		 *
		 * @param folder name of folder from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 */
		virtual void writeIntoDb(const string folder, const string subroutine);
		/**
		 * database service for which changes in subroutines are needed.<br />
		 * this method define for any specific client which subroutines in folder it does
		 * need to know it is changed.
		 *
		 * @param connection id of server connection with client
		 * @param name folder and subroutine which are needed, or string 'newentry' if need an new pool of subroutines
		 * @return whether the subroutine exists
		 */
		virtual bool needSubroutines(unsigned long connection, string name);
		/**
		 * database service for all changes in subroutines.<br />
		 * this method returning all changes which are defined with <code>needSubroutines()</code>
		 *
		 * @param connection id of server connection with client
		 * @return all changed values
		 */
		virtual vector<string> getChangedEntrys(unsigned long connection);
		/**
		 * set the entry to an measure curve which be
		 * defined in db_t::identif as <code>def:&lt;curvetype;:&lt;folder&gt;:&lt;subroutine;</code>
		 *
		 * @param entry db_t value
		 * @param doSort on set true, vector of values will be sorted after fill in (default)
		 */
		void fillMeasureCurve(const db_t entry, bool doSort= true);
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
																			const vector<double>::size_type number= 0);
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
																const vector<double>::size_type number= 0);
		/**
		 * change the communication ID to an new one
		 *
		 * @param oldId old communication ID
		 * @param newId new communication ID
		 */
		virtual void changeNeededIds(unsigned long oldId, unsigned long newId);
		/**
		 * whether one or more entry's are changed.<br />
		 * method hold thread in process and return without any result
		 * when any entry was changed
		 */
		virtual void isEntryChanged();
		/**
		 * arouse all condition witch wait of any changes in database.<br />
		 * This method will be called when any connection from Internet process will be break,
		 * to ending also this asking connection.
		 */
		virtual void arouseChangingPoolCondition();
		/**
		 * returns two convert values which have between the given value
		 *
		 * @param subroutine for which folder:subroutine the two values should be
		 * @param definition defined name in the database
		 * @param value the value which should be between the returned two values
		 * @return vector of two convert values
		 */
		virtual vector<convert_t> getNearest(string subroutine, string definition, double value);
		/**
		 * return all folder with subroutines which are use the defined chip
		 *
		 * @param onServer on which server the changed chip will be
		 * @param chip the unique define chip
		 * @return map of folder with subroutines
		 */
		virtual map<string, vector<string> >* getSubroutines(const string& onServer, const string& chip);
		/**
		 * fill double value into database
		 *
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
		virtual void fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew= true);
		/**
		 * method to running thread .<br />
		 * This method starting again when method stop() wasn't call.
		 *
		 * @return error code for not correctly done
		 */
		virtual int execute();
		/**
		 * stop working of database,
		 * arose all conditions
		 */
		virtual bool stop();
		/**
		 * method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();
		/**
		 * destruktor of database
		 */
		virtual ~Database();

	protected:
		/**
		 * shows whether database should stopping
		 */
		bool stopping()
		{	bool bRv;
			LOCK(m_STOPDB);
			bRv= m_bDbStop;
			UNLOCK(m_STOPDB);
			return bRv;			};

	private:
		/**
		 * reference to DefaultConfigReader
		 */
		IChipConfigReaderPattern* m_pChipReader;
		/**
		 * working directory
		 */
		string m_sWorkDir;
		/**
		 * name of current database file
		 */
		string m_sDbFile;
		/**
		 * condition for new items filled in
		 */
		pthread_cond_t* m_DBENTRYITEMSCOND;
		/**
		 * mutex for locking database entrys to fill
		 */
		pthread_mutex_t* m_DBENTRYITEMS;
		/**
		 * mutex for locking actual database entrys
		 */
		pthread_mutex_t* m_DBCURRENTENTRY;
		/**
		 * mutex for locking measure curve data
		 */
		pthread_mutex_t* m_DBMEASURECURVES;
		/**
		 * mutex lock to fill changes
		 */
		pthread_mutex_t* m_CHANGINGPOOL;
		/**
		 * condition for client wating of new changes
		 */
		pthread_cond_t* m_CHANGINGPOOLCOND;
		/**
		 * mutex lock for stopping status
		 */
		pthread_mutex_t* m_STOPDB;
		/**
		 * whether database should stopping
		 */
		bool m_bDbStop;
		/**
		 * vector with all new entrys
		 */
		std::auto_ptr<vector<db_t> > m_sptEntrys;
		/*
		 * subroutines which should be written into database
		 */
		vector<db_t> m_vtDbValues;
		/**
		 * map with all measure curves
		 */
		map<string, map<string, map<double, double> > > m_mmmMeasureCurves;
		/**
		 * vector of all db_t structs which are in m_mmmMeasureCurves
		 * for writing an new database file
		 */
		vector<db_t> m_vMeasureCurves;
		/**
		 * current state of values
		 */
		map<string, map<string, map<string, db_t> > > m_mCurrent;
		/**
		 * current file for thin database
		 */
		string m_sThinFile;
		/**
		 * variable is set if the database file cannot write.<br />
		 * The base of case is to write the errormessage only on one time.
		 */
		bool m_bError;
		/**
		 * wait this seconds on condition before read older db files to thin
		 */
		unsigned short m_nCondWait;
		/**
		 * to thin older db files read always this count of rows before wait again in condition for new db entrys
		 */
		unsigned int m_nReadRows;
		/**
		 * how much byte read from file to thin
		 */
		ostream::pos_type m_nReadPos;
		/**
		 * write after MB an new database file
		 */
		unsigned int m_nAfter;
		/**
		 * name of measure file to write in database
		 */
		string m_sMeasureName;
		/**
		 * this variable fill and delete an client over the server
		 * which need all actual values from port-list
		 */
		map<unsigned long, vector<db_t> > m_mvoChanges;
		/**
		 * whether any value is changed in database
		 */
		bool m_bAnyChanged;
		/**
		 * map of all filenames with the time to thin
		 */
		map<string, time_t> m_mOldest;
		/**
		 * all subroutines witch need new content of any chips.<br/>
		 * saving in map<onServer, map<chip, map<folder, vector<subroutine> > > >
		 */
		map<string, map<string, map<string, vector<string> > > > m_mmmvServerContent;
		/**
		 * in which case reading information of saved subroutine result
		 */
		vector<pair<pair<pair<string, string>, double>, pair<pair<string, string>, double> > > m_vReadBlockDefs;
		/**
		 * subroutine and value to ending read of block
		 */
		pair<pair<string, string>, double>* m_pEndReading;
		/**
		 * read blocks
		 */
		vector<vector<db_t> > m_vvDbEntrys;
		/**
		 * all read subroutine lines from database
		 */
		vector<vector<db_t> > m_vReadBlocks;
		/**
		 * actual reading block
		 */
		vector<vector<db_t> >::size_type m_nReadBlock;

		/**
		 * write entrys into database with asking bevore DefaultChipConfigReader whether is allowed
		 *
		 * @param entry db_t structure of chip
		 * @param dbfile if dbentrys, handle of file descriptor, is set
		 * 					do not ask on DefaultChipConfigReader whether should writing
		 * 					and open no new handle
		 */
		void writeDb(db_t entry, ofstream* dbfile= NULL);
		/**
		 * write entry direct into database
		 *
		 * @param entry db_t structure of chip
		 * @param dbfile file handle to write
		 */
		void writeEntry(const db_t& entry, ofstream &dbfile);
		/**
		 * comb through database to kill older not needed entry's
		 *
		 * @param ask whether call is only for asking
		 * @return whether need to comb through again
		 */
		bool thinDatabase(const bool ask);
		/**
		 * calculate the new time which should thin the database files again.
		 *
		 * @param fromtime the time from which should calculated
		 * @param older pointer to older structure
		 */
		void calcNewThinTime(time_t fromtime, const SHAREDPTR::shared_ptr<otime_t> &older);
		/**
		 * asking for cached entrys
		 *
		 * @param bWait wait for condition DBENTRYITEMSCOND if nothing to write
		 * @return vector of db_t entrys structures
		 */
		std::auto_ptr<vector<db_t> > getDbEntryVector(bool bWait);
		/**
		 * searching in given directory for the latest
		 * file which beginning with filter, date
		 * and ending with extension .dat
		 *
		 * @param path in which directory should be find the file
		 * @param filter with which characters the name of the file beginning
		 * @param size size of dbfile. If no file found, size is 0
		 * @return name of last dbfile if any exist
		 */
		string getLastDbFile(string path, string filter, off_t &size);
		/**
		 * create an new database file and fill in all actual entrys
		 *
		 * @param bCheck whether function should check for old database file whether to greate (over 15000000 byte)
		 * @return true if an new file is created
		 */
		bool createNewDbFile(bool bCheck);
		/**
		 * split an read line from database on harddisk into struct db_t
		 *
		 * @param line line from database on harddisk
		 * @return spliced values
		 */
		db_t splitDbLine(const string& line);
		/**
		 * set the actual entry of an measured value
		 * in the private map
		 *
		 * @param entry structure of current db_t
		 * @return true if an new value was set
		 */
		bool setActEntry(const db_t entry);
	};

}

#endif /* DATABASE_H_ */
