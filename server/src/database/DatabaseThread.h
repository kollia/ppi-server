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
#ifndef DATABASETHREAD_H_
#define DATABASETHREAD_H_

#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "DefaultChipConfigReader.h"

#include "../util/Thread.h"
#include "../util/structures.h"

using namespace std;
using namespace ports;

namespace ppi_database
{

	/**
	 * struct of nessesary items
	 * to write into database
	 */
	struct db_t
	{
		/**
		 * time of insert
		 */
		time_t tm;
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
			return subroutine > other->subroutine;
		}
		/**
		 * is same operator
		 */
		bool operator == (const db_t* other) const
		{
			return (	folder == other->folder
						&&
						subroutine == other->subroutine	);
		}
	};

	/**
	 * class representig connection to internal
	 * or an mySql database
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class DatabaseThread : public Thread
	{
	public:
		using Thread::stop;

		/**
		 * instanciate class of database
		 *
		 * @param dbDir working directory of database where write the files
		 * @param confDir path of configuration files for default chips settings
		 * @param measureName setting measure-name in file mesure.conf
		 * @param mbyte write after MB an new database file default(15)
		 * @param defaultSleep sleeping for default time in microseconds default(0)
		 * @return instance of database
		 */
		static void initial(string dbDir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep= 0);
		/**
		 * method to ask database whether database file is loaded
		 *
		 * @return wheter file is loaded
		 */
		bool isDbLoaded() const;
		/**
		 * return instance of database object
		 *
		 * @return database object
		 */
		static DatabaseThread* instance()
					{ return _instance; };
		/**
		 * function set, whether need value in database,
		 * or will be only set to inform clients for changes
		 *
		 * @param folder name of folder from the running thread
		 * @param subroutine mame of the subroutine in the folder
		 */
		void writeIntoDb(const string folder, const string subroutine);
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
		void fillValue(string folder, string subroutine, string identif, vector<double> values, bool bNew= true);
		/**
		 * returns actual value of subroutine in folder
		 *
		 * @param folder measure folder defined in measure.conf
		 * @param subroutine subroutine inside of given folder
		 * @param identif identification of which value be set
		 * @param number count of double value which is required
		 * @return current value of given parameters
		 */
		double* getActEntry(const string folder, const string subroutine, const string identif,
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
		 * whether one or more entrys are changed.<br />
		 * method hold thread in process
		 */
		void isEntryChanged();
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
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * destruct of Database
		 */
		virtual ~DatabaseThread();

	protected:
		/**
		 * method to initial the thread .<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual int init(void *args= NULL);
		/**
		 * method to running thread .<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
		 *
		 * @return error code for not correctly done
		 */
		virtual int execute();
		/**
		 * method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();
		/**
		 * write entrys into database with asking bevore DefaultChipConfigReader whether is allowed
		 *
		 * @param entry db_t structure of chip
		 * @param dbfile if dbentrys, handle of file descriptor, is set
		 * 					do not ask on DefaultChipConfigReader whether should writing
		 * 					and open no new handle
		 */
		void writeDb(db_t entry, ofstream* dbentrys= NULL);
		/**
		 * write entry direct into database
		 *
		 * @param entry db_t structure of chip
		 * @param dbfile file handle to write
		 */
		void writeEntry(const db_t& entry, ofstream *dbfile);

		/**
		 * write after MB an new database file
		 */
		unsigned int m_nAfter;

	private:
		/**
		 * single instance of database
		 */
		static DatabaseThread* _instance;
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
		 * mutex lock to know whether database file is loaded
		 */
		pthread_mutex_t* m_DBLOADED;
		/**
		 * condition for client wating of new changes
		 */
		pthread_cond_t* m_CHANGINGPOOLCOND;
		/**
		 * vector with all new entrys
		 */
		vector<db_t>* m_ptEntrys;
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
		 * name of database file on harddisk
		 */
		string m_sDbFile;
		/**
		 * whether database file is loaded
		 */
		bool m_bDbLoaded;
		/**
		 * current file for thin database
		 */
		string m_sThinFile;
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
		 * time on database file created
		 */
		time_t m_tmDb;
		/**
		 * name of measure file to write in database
		 */
		string m_sMeasureName;
		/**
		 * working directory
		 */
		string m_sWorkDir;
		/**
		 * path of configuration files for default chips settings
		 */
		string m_sConfDir;
		/**
		 * variable is set if the database file cannot write.<br />
		 * The base of case is to write the errormessage only on one time.
		 */
		bool m_bError;
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
		 * private initialization of Database
		 *
		 * @param dbDir working directory of database where write the files
		 * @param confDir path of configuration files for default chips settings
		 * @param measureName setting measure-name in file mesure.conf
		 * @param mbyte write after MB an new database file
		 * @param defaultSleep sleeping for default time in microseconds
		 */
		DatabaseThread(string dbdir, string confDir, IPropertyPattern* properties, useconds_t defaultSleep);
		/**
		 * asking for cached entrys
		 *
		 * @param bWait wait for condition DBENTRYITEMSCOND if nothing to write
		 * @return vector of db_t entrys structures
		 */
		vector<db_t>* getDbEntryVector(bool bWait);
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
		 * set the actual entry of an measured value
		 * in the private map
		 *
		 * @param entry structure of current db_t
		 * @return true if an new value was set
		 */
		bool setActEntry(const db_t entry);
		/**
		 * set the entry to an measure curve which be
		 * defined in db_t::identif as <code>def:&lt;curvetype;:&lt;folder&gt;:&lt;subroutine;</code>
		 *
		 * @param entry db_t value
		 * @param doSort on set true, vector of values will be sorted after fill in (default)
		 */
		void fillMeasureCurve(const db_t entry, bool doSort= true);
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
		void calcNewThinTime(time_t fromtime, const DefaultChipConfigReader::otime_t* older);
		/**
		 * split an read line from database on harddisk into struct db_t
		 *
		 * @param line line from database on harddisk
		 * @return spliced values
		 */
		db_t splitDbLine(const string& line);
	};

}

#endif /*DATABASETHREAD_H_*/
