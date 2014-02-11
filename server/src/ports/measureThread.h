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
#ifndef MEASURETHREAD_H_
#define MEASURETHREAD_H_


#include <set>
#include <queue>

#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"

#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/thread/Thread.h"
#include "../util/thread/Terminal.h"

#include "Informer.h"
#include "DbFiller.h"
#include "ListCalculator.h"

using namespace util;
using namespace design_pattern_world::util_pattern;

struct MeasureArgArray
{
	vector<pair<string, PortTypes> > ports;
	vector<sub> *subroutines;
	/**
	 * all subroutines for debugging from begin
	 */
	vector<string> debugSubroutines;
};

class MeasureThread : 	public Thread,
						virtual public IMeasurePattern
{
	public:
		/**
		 * creating instance of MeasureThread
		 *
		 * @param threadname name of running folder
		 * @param tArg structure of subroutine and ports
		 * @param pFolderStart all starting folders found in measure.conf
		 * @param how long thread should waiting to start folder list again when an external server wasn't found
		 * @param bNoDbRead whether should thread reading folder running length time from database
		 * @param folderCPUtime on which CPU time should differ by database writing for folder length
		 */
		MeasureThread(const string& threadname, const MeasureArgArray& tArg,
						const SHAREDPTR::shared_ptr<measurefolder_t> pFolderStart,
						const time_t& nServerSearch,
						bool bNoDbRead, short folderCPUlenth);
		/**
		 * return class of subroutine from this folder
		 *
		 * @param subroutine name of the subroutine
		 * @return class of subroutine
		 */
		SHAREDPTR::shared_ptr<IListObjectPattern> getPortClass(const string subroutine, bool &bCorrect) const;
		/**
		 * method returning name of folder,
		 * is same as thread name
		 *
		 * @return name of folder
		 */
		virtual string getFolderName() const
		{ return m_sFolder; };
		/**
		 * returning thread id in which thread folder object running
		 *
		 * @return thread id
		 */
		virtual pid_t getRunningThreadID()
		{ return Thread::getThreadID(); };
		/**
		 * returning external send device
		 *
		 * @return sending device
		 */
		virtual IClientSendMethods* getExternSendDevice()
		{ return &m_oDbFiller; };
		/**
		 * return run specification of folder
		 *
		 * @return all specification needed
		 */
		virtual vector<string> getAllSpecs() const
		{ return m_vsFolderSecs; };
		/**
		 * check whether this folder is running for work.<br />
		 * when first value <code>specs</code> has no content,
		 * method meaning that caller now that folder need.
		 * Otherwise method checking also whether one specification
		 * is also defined for own folder
		 *
		 * @param specs all specifications are allowed
		 * @return whether folder running
		 */
		virtual folderSpecNeed_t isFolderRunning(const vector<string>& specs);
		/**
		 * inform other folders and also own when necessary
		 * that an specific subroutine was changed
		 *
		 * @param folders map of folders which should informed
		 * @param from which subroutine (other or own) changing value
		 * @param as from which subroutine be informed
		 * @param debug whether subroutine which inform folders, running in debug session
		 * @param lock locking mutex for observers
		 */
		virtual void informFolders(const folders_t& folders, const string& from,
											const string& as, const bool debug, pthread_mutex_t *lock);
		/**
		 * set debug session in subroutine or hole folder when subroutine not given
		 *
		 * @param bDebug wheter should debug session be set or unset
		 * @param subroutine name of subroutine
		 */
		bool setDebug(bool bDebug, const string& subroutine= "");
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param value value which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		virtual void fillValue(const string& folder, const string& subroutine, const string& identif, double value, bool bNew= false)
		{ m_oDbFiller.fillValue(folder, subroutine, identif, value, bNew); };
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param dvalues vector of more values which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
						const vector<double>& dvalues, bool bNew= false)
		{ m_oDbFiller.fillValue(folder, subroutine, identif, dvalues, bNew); };
		/**
		 * return actually count of current subroutine
		 *
		 * @param subroutine whitch count should be returned when set, elsewhere create new counts
		 * @return count number of subroutine
		 */
		virtual unsigned short getActCount(const string& subroutine);
		/**
		 * returning true if an client set this measurethread to debug
		 *
		 * @return whether measure thread do output
		 */
		virtual bool isDebug();
		/**
		 *  external command to stop object of MeasureThread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return stop(&bWait); };
		/**
		 *  external command to stop object of MeasureThread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * if any client set debug to true, method returning sleeptime
		 * whitch has client set. Otherwise method returning default time from 3
		 */
		//unsigned short getSleepTime();
		/**
		 * information by changed value in any subroutine
		 *
		 * @param folder which folder should be informed
		 * @param from from which folder comes information
		 */
		virtual void changedValue(const string& folder, const string& from);
		/**
		 * from witch folder:subroutine thread was informed for new value
		 *
		 * @return vector of folder:subroutine which informed
		 */
		virtual vector<string> wasInformed();
		/**
		 * on which time the measure routine should start without any actions on extern ports
		 *
		 * @param folder name of folder
		 * @param time next beginning run time
		 */
		void nextActivateTime(const string& folder, const IPPITimePattern& time)
		{ LOCK(m_ACTIVATETIME);m_vtmNextTime.push_back(time);UNLOCK(m_ACTIVATETIME); };
		/**
		 * length time of folder running, by actual CPU time.<br />
		 * when get unset time back (<code>= !timerisset(<returnvalue>)</code>)
		 * no measuring be set for folder
		 *
		 * @param logPercent whether logging original percent into database
		 * @param debug whether call run in debug session
		 * @return longest measured length of folder time
		 */
		virtual IPPITimePattern& getLengthedTime(const bool& logPercent, const bool& debug);
		/**
		 * length time of given map by actual CPU time.<br />
		 * when get unset time back (<code>= !timerisset(<returnvalue>)</code>)
		 * no measuring be set for folder
		 *
		 * @param timelength all variables to calculate CPU time
		 * @param lengthRun time map differ by CPU time
		 * @param percent out comming percent of CPU from return value
		 * @param logPercent whether logging original percent into database
		 * @param debug whether call run in debug session
		 * @return longest measured length of folder time
		 */
		virtual IPPITimePattern& getLengthedTime(timetype_t* timelength, short *percent,
										const bool& logPercent, const bool& debug);
		/**
		 * sleep microseconds by consider stopping of running thread
		 *
		 * @param time sleeping time
		 * @return whether thread should stopping
		 */
		virtual bool usleep(const IPPITimePattern& time);
		/**
		 * set folder to calculating length of folder time
		 */
		virtual void calculateLengthTime()
		{ m_bNeedLength= true; };
		/**
		 * subroutine signal whether can find the server for external measuring
		 *
		 * @param bfound whether server was finding
		 * @param server type of server will be found
		 * @param id chip id searched inside server
		 */
		void foundPortServer(const bool bfound, const string& server, const string& id);
		/**
		 * searching where folder was starting from an specific time condition
		 * and change starting time
		 *
		 * @param folder name of folder
		 * @param time next beginning run time
		 * @param newtime new starting time
		 */
		virtual void changeActivationTime(const string& folder, const IPPITimePattern& time,
													const IPPITimePattern& newtime);
		/**
		 * searching whether folder was starting from an specific time condition and erase starting.
		 *
		 * @param folder name of folder
		 * @param time next beginning run time
		 */
		virtual void eraseActivateTime(const string& folder, const IPPITimePattern& time);
		/**
		 * write timeval time to display on console
		 *
		 * @param tvTime timeval time to display
		 * @param bDate whether should display given time as date with microseconds (true) or only seconds with microseconds
		 * @param bDebug whether should output error on console
		 * @return string object as date or seconds
		 */
		 static string getTimevalString(const timeval& tvtime, const bool& bDate, const bool& bDebug);
		 /**
		  * calculating length time for reached finished or starting late
		  * taking into account any outliers, save into database
		  * and consider also CPU time
		  *
		  * @param timelength all variables to measure CPU time
		  * @param length longer time of reached or starting late time
		  * @param debug whether subroutine running inside debug session
		  */
		 virtual void calcLengthDiff(timetype_t* timelength,
						 const IPPITimePattern& length, const bool& debug);
		 /**
		  * search inside timetype_t for correct map with synchronization ID
		  * and set also the nearest one by new creation to define the default value
		  *
		  * @param timelength all variables to measure CPU time
		  * @param nearest nearest map of syncronization ID for default value
		  * @param debug whether subroutine running inside debug session
		  */
		 map<short, timeLen_t>* getPercentDiff(timetype_t *timelength,
						 	 map<short, timeLen_t>* nearest, const bool&debug);
		/**
		 * set into given timetype the CPU times to begin measuring for <code>getCpuPercent</code>
		 *
		 * @param timetype all static variables to measure CPU time
		 */
		virtual void setCpuMeasureBegin(timetype_t *timetype);
		/**
		 * calculate CPU time in percent
		 *
		 * @param processor return CPU percent for processor.<br /> by 0 an average of all exist
		 * @param prev_idle last idle time from which should be created and give back new old one
		 * @param prev_total last total time from which  should be created and give back new old one
		 * @param old_usage last calculated CPU percent
		 * @param debug whether call is for debug session
		 * @return percent of CPU
		 */
		static int getCpuPercent(const vector<int>::size_type& processor, int *prev_idle,
										int *prev_total, int old_usage, const bool& debug);
		/**
		 * calculate double result whether should measure in second range
		 * or microsecond range
		 *
		 * @param tv current time
		 * @param secondcalc whether should calculate in seconds (true) or microseconds (false)
		 * @return result of subroutine
		 */
		static double calcResult(const timeval& tv, const bool& secondcalc);
		/**
		 * calculate timeval result whether should measure in second range
		 * or microsecond range
		 *
		 * @param tv current time
		 * @param secondcalc whether should calculate in seconds (true) or microseconds (false)
		 * @return result of subroutine
		 */
		static timeval calcResult(double seconds, const bool& secondcalc);
		/**
		 * begin counting of how much folder was running
		 */
		void beginCounting();
		/**
		 * return how often folder thread was running
		 *
		 * @return counting number
		 */
		int getRunningCount();
		/**
		 * destructor of MeasureThread
		 */
		virtual ~MeasureThread();


	protected:
		/**
		 * this method will be called before running
		 * the method execute to initial class
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual int init(void *arg);
		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param error code for not correctly done
		 */
		virtual int execute();
		/**
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();
		void clear();
		bool measure();

	private:
		/**
		 * changed times of subroutines
		 */
		struct tChangedTimes
		{
			/**
			 * all defined subroutines with last changing time
			 */
			map<string, timeval> subroutines;
			/**
			 * all subroutines from own folder
			 */
			vector<string> ownSubs;
			/**
			 * all subroutines needed from different TIMER subroutines
			 * and also differ by finished, begin, while and end parameter
			 */
			map<string, vector<pair<string, timeval*> > > subSubs;
		};

		/**
		 * time value from last <code>getLengthedTime()</code> method. inside DEBUGLOCK<br />
		 * need to hold as reference for return value
		 */
		ppi_time m_oInsideTime;
		/**
		 * time value from last <code>getLengthedTime()</code> method. outside DEBUGLOCK<br />
		 * need to hold as reference for return value
		 */
		ppi_time m_oOutsideTime;

		vector<pair<string, PortTypes> > m_pvlPorts;
		vector<sub> *m_pvtSubroutines;
		/**
		 * all subroutines which should be set in debug modus
		 * by starting
		 */
		vector<string> m_vStartDebugSubs;
		/**
		 * whether any subroutine is set as debug for output
		 */
		bool m_bDebug;
		/**
		 * name of folder
		 */
		string m_sFolder;
		/**
		 * whether thread should reading running length time from database
		 */
		bool m_bNoDbReading;
		/**
		 * whether any time subroutine inside folder
		 * needs length folder time to calculate precise starting
		 */
		bool m_bNeedLength;
		/**
		 * thread id in which measureThread object running
		 */
		pid_t m_tRunThread;
		/**
		 * Calculator whether thread running
		 */
		ListCalculator m_oRunnThread;
		/**
		 * all folder ID specifications which are defined for running folders
		 * to differ between reach time
		 */
		vector<string> m_vsFolderSecs;
		/**
		 * actually count number of set subroutine
		 */
		unsigned short m_nActCount;
		/**
		 * all changed folder
		 */
		vector<pair<string, ppi_time> > m_vFolder;
		/**
		 * all time activation
		 * which cannot write actualy into m_vFolder
		 */
		vector<string> m_vTimeFolder;
		/**
		 * next time to activate measure routine without any action from external client
		 */
		vector<ppi_time> m_vtmNextTime;
		/**
		 * start time of folder
		 */
		ppi_time m_tvStartTime;
		/**
		 * whether folder need to know whether folder is running.<br />
		 * this variable will be defined by initialization and need no mutex lock
		 * to be atomic
		 */
		bool m_bNeedFolderRunning;
		/**
		 * whether this folder running
		 */
		bool m_bFolderRunning;
		/**
		 * on which CPU time should differ by database writing for folder length
		 */
		short m_nFolderCPUtime;
		/**
		 * whether can read informed folders
		 */
		bool m_bReadInformations;
		/**
		 * vector of all started times
		 * when information folders not can be written
		 */
		vector<pair<short, ppi_time> > m_vStartTimes;
		/**
		 * structure definition to calculate length of folder running time
		 */
		timetype_t m_tLengthType;
		/**
		 * sleeping time of subroutine
		 * which should subtract from running length
		 */
		ppi_time m_tvSleepLength;
		/**
		 * object with server type's an id's for unknown server,
		 * to know whether folder list should starting all pre-defined seconds
		 * the list to searching again
		 */
		set<string> m_osUndefServers;
		/**
		 * pre-defined seconds to search for server
		 */
		time_t m_nServerSearchSeconds;
		/**
		 * count how often folder running
		 */
		int m_nRunCount;
		/**
		 * scheduling policy in which folder thread should running
		 */
		int m_nSchedPolicy;
		/**
		 * scheduling priority in which policy of folder thread should running
		 */
		int m_nSchedPriority;
		/**
		 * mutex by any changing of value
		 */
		pthread_mutex_t *m_VALUE;
		/**
		 * mutex for fill or erase new activate time
		 */
		pthread_mutex_t *m_ACTIVATETIME;
		/**
		 * mutex for check whether folder is running
		 * and also for counting how often run
		 */
		pthread_mutex_t *m_FOLDERRUNMUTEX;
		/**
		 * mutex by setting debug output
		 * or lengthed time of folder running
		 */
		pthread_mutex_t *m_DEBUGLOCK;
		/**
		 * mutex want to inform folder to running
		 */
		pthread_mutex_t *m_WANTINFORM;
		/**
		 * condition for wait for new changing of any subroutine
		 */
		pthread_cond_t *m_VALUECONDITION;
		/**
		 * thread to inform other and own folder when one subroutine changing
		 */
		Informer m_oInformer;
		/**
		 * database filler pool
		 */
		DbFiller m_oDbFiller;

		/**
		 * private copy constructor for no allowed copy
		 *
		 * @param x object for coppy
		 */
		MeasureThread(const MeasureThread& x);
		/**
		 * private assignment operator for not allowed allocation
		 *
		 * @param x opbject for assignment
		 * @return own object
		 */
		MeasureThread& operator=(const MeasureThread&);
		/**
		 * check whether inside folder is an new time to restart
		 * only when lock is given,
		 * and write info by debug session
		 *
		 * @param debug session of debug output
		 * @return whether folder should start
		 */
		bool checkToStart(const bool debug);
};


class meash_t
{
	public:
		/**
		 * instance of first measure thread
		 */
		static SHAREDPTR::shared_ptr<meash_t> firstInstance;
		/**
		 * next instnace of meash_t
		 */
		SHAREDPTR::shared_ptr<meash_t> next;
		/**
		 * measure object
		 */
		SHAREDPTR::shared_ptr<MeasureThread> pMeasure;
		/**
		 * path in whitch be the layout files
		 * toDo: not the correct place
		 */
		static string clientPath;
};

#endif /*MEASURETHREAD_H_*/
