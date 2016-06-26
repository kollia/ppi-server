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
#include <boost/regex.hpp>

#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"
#include "../pattern/util/IInformerCachePattern.h"
#include "../pattern/util/IDbFillerPattern.h"

#include "../util/debugsubroutines.h"
#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/thread/Thread.h"
#include "../util/thread/Terminal.h"

#include "Informer.h"
#include "MeasureInformerCache.h"
#include "DbFillerFactory.h"
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
#ifdef __followSETbehaviorFromFolder
		bool m_btimer;
		boost::regex m_oToFolderExp;
		boost::regex m_oToSubExp;
#endif // __followSETbehaviorFromFolder
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
		OVERWRITE string getFolderName() const
		{ return m_sFolder; };
		/**
		 * return ListCalculator for whether foder thread should be informed
		 *
		 * @return ListCalculator
		 */
		OVERWRITE string getInformeThreadStatement()
		{ return m_sInformeThreadStatement; };
		/**
		 * returning thread id in which thread folder object running
		 *
		 * @return thread id
		 */
		OVERWRITE pid_t getRunningThreadID()
		{ return Thread::getThreadID(); };
		/**
		 * returning external send device
		 *
		 * @return sending device
		 */
		OVERWRITE IClientSendMethods* getExternSendDevice()
		{ return m_oDbFiller.get(); };
		/**
		 * return run specification of folder
		 *
		 * @return all specification needed
		 */
		OVERWRITE vector<string> getAllSpecs() const
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
		OVERWRITE folderSpecNeed_t isFolderRunning(const vector<string>& specs);
		/**
		 * whether list object (folder) has the incoming sub-variable
		 *
		 * @param subvar name of sub-variable
		 * @return whether folder has this variable
		 */
		OVERWRITE bool hasSubVar(const string& subvar) const;
		/**
		 * return content of sub-variable from current list object (folder)
		 *
		 * @param who declare who need the value information
		 * @param subvar name of sub-variable
		 * @return value of sub-var
		 */
		OVERWRITE ppi_value getSubVar(const InformObject& who, const string& subvar) const;
		/**
		 * return cache to observer changing values
		 *
		 * @param folder name of folder for which cache used
		 * @return observer cache
		 */
		OVERWRITE SHAREDPTR::shared_ptr<IInformerCachePattern> getInformerCache(const string& folder);
		/**
		 * return as parameter mutex and conditions
		 * to awake folder thread for running
		 */
		OVERWRITE awakecond_t getAwakeConditions();
		/**
		 * return cache to observer only when exist
		 *
		 * @param folder name of folder for which cache used
		 * @return observer cache when exist, otherwise NULL
		 */
		OVERWRITE SHAREDPTR::shared_ptr<IInformerCachePattern> getUsedInformerCache(const string& folder);
		/**
		 * get setting scheduling parameters
		 * of policy and priority
		 *
		 * @param policy thread policy for scheduling
		 * @param priority scheduling priority
		 */
		OVERWRITE void getSchedulingParameter(int& policy, int& priority)
		{ Thread::getSchedulingParameter(policy, priority); };
		/**
		 * remove observer cache when no more needed
		 *
		 * @param folder name of folder for which cache was used
		 */
		OVERWRITE void removeObserverCache(const string& folder);
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
		OVERWRITE void informFolders(const IInformerCachePattern::memObserverVector& folders, const InformObject& from,
											const string& as, const bool debug, pthread_mutex_t *lock);
		/**
		 * returning output object to write output inside folder thread
		 *
		 * @return output object
		 */
		OVERWRITE SHAREDPTR::shared_ptr<IListObjectPattern> getTerminalOutputObject()
				{ return m_oInformOutput; };
		/**
		 * set debug session in subroutine or hole folder when subroutine not given
		 *
		 * @param bDebug whether should debug session be set
		 * @param bInform whether need by existing inform parameter this also by output
		 * @param subroutine name of subroutine
		 */
		bool setDebug(bool bDebug, bool bInform, const string& subroutine= "");
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param value value which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		OVERWRITE void fillValue(const string& folder, const string& subroutine, const string& identif, double value, bool bNew= false)
		{ m_oDbFiller->fillValue(folder, subroutine, identif, value, bNew); };
		/**
		 * fill double value over an queue into database
		 *
		 * @param folder folder name from the running thread
		 * @param subroutine name of the subroutine in the folder
		 * @param identif identification of which value be set
		 * @param dvalues vector of more values which should write into database
		 * @param bNew whether database should actualize value for client default= false
		 */
		OVERWRITE void fillValue(const string& folder, const string& subroutine, const string& identif,
						const vector<double>& dvalues, bool bNew= false)
		{ m_oDbFiller->fillValue(folder, subroutine, identif, dvalues, bNew); };
		/**
		 * fill debug session output from folder working list
		 * into database
		 *
		 * @param content structure of folder:subroutine data from debugging session
		 */
		OVERWRITE void fillDebugSession(const dbgSubroutineContent_t& content)
		{ m_oDbFiller->fillDebugSession(content); };
		/**
		 * return actually count of current subroutine
		 *
		 * @param subroutine whitch count should be returned when set, elsewhere create new counts
		 * @return count number of subroutine
		 */
		OVERWRITE unsigned short getActCount(const string& subroutine);
		/**
		 * returning true if an client set this measurethread to debug
		 *
		 * @return whether measure thread do output
		 */
		OVERWRITE bool isDebug();
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return MeasureThread::stop(&bWait); };
		/**
		 *  external command to stop object of MeasureThread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * from witch folder:subroutine thread was informed for new value
		 *
		 * @return vector of folder:subroutine which informed
		 */
		OVERWRITE vector<string> wasInformed();
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
		OVERWRITE IPPITimePattern& getLengthedTime(const bool& logPercent, const bool& debug);
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
		OVERWRITE IPPITimePattern& getLengthedTime(timetype_t* timelength, short *percent,
										const bool& logPercent, const bool& debug);
		/**
		 * sleep microseconds by consider stopping of running thread
		 *
		 * @param time sleeping time
		 * @return whether thread should stopping
		 */
		OVERWRITE bool usleep(const IPPITimePattern& time);
		/**
		 * set folder to calculating length of folder time
		 */
		OVERWRITE void calculateLengthTime()
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
		OVERWRITE void changeActivationTime(const string& folder, const IPPITimePattern& time,
													const IPPITimePattern& newtime);
		/**
		 * searching whether folder was starting from an specific time condition and erase starting.
		 *
		 * @param folder name of folder
		 * @param time next beginning run time
		 */
		OVERWRITE void eraseActivateTime(const string& folder, const IPPITimePattern& time);
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
		 OVERWRITE void calcLengthDiff(timetype_t* timelength,
						 const IPPITimePattern& length, const bool& debug);
		 /**
		  * search inside timetype_t for correct map with synchronization ID
		  * and set also the nearest one by new creation to define the default value
		  *
		  * @param timelength all variables to measure CPU time
		  * @param nearest whether result is nearest map of syncronization ID for default value
		  * @param debug whether subroutine running inside debug session
		  */
		 map<short, timeLen_t>* getPercentDiff(timetype_t *timelength,
						 	 bool& nearest, const bool&debug);
		/**
		 * calculate CPU time in percent
		 *
		 * @param processor return CPU percent for processor.<br /> by 0 an average of all exist
		 * @param debug whether call is for debug session
		 * @return percent of CPU
		 */
		static int getCpuPercent(const vector<int>::size_type& processor, const bool& debug);
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
		 * @param starts all starting times with information how inform
		 * @return counting number
		 */
		int getRunningCount(map<ppi_time, vector<InformObject> >& starts);
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
		 * @return object of error handling
		 */
		OVERWRITE EHObj init(void *arg);
		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param whether should start thread again
		 */
		OVERWRITE bool execute();
		/**
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		OVERWRITE void ending();
		void clear();
		bool measure();

	private:
		/**
		 * current measure of CPU time
		 */
		struct CpuTime_t
		{
			 /**
			  * last idle time
			  * from which should be created
			  * and give back new old one
			  */
			 int prev_idle;
			 /**
			  * last total time from which should
			  * be created and give back new old one
			  */
			 int prev_total;
			 /**
			  * last calculated CPU percent
			  */
			 int old_usage;
			 /**
			  * time after calculating
			  * new CPU time
			  */
			 ppi_time nextCall;

			 /**
			  * constructor to set all values to 0
			  */
			 CpuTime_t()
			 : prev_idle(0),
			   prev_total(0),
			   old_usage(0)
			 {};
		};
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
		 * whether should show by debug output
		 * also inform parameter
		 */
		bool m_bInformParam;
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
		 * object for informing output
		 */
		SHAREDPTR::shared_ptr<IListObjectPattern> m_oInformOutput;
		/**
		 * string to calculate whether folder thread should be informed to start running
		 */
		string m_sInformeThreadStatement;
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
		 * next time to activate measure routine without any action from external client
		 */
		vector<ppi_time> m_vtmNextTime;
		/**
		 * start time of folder thread
		 */
		ppi_time m_tvStartTime;
		/**
		 * time of measure ending
		 * from all subroutines
		 */
		ppi_time m_tvEndTime;
		/**
		 * whether folder need to know whether folder is running.<br />
		 * this variable will be defined by initialization and need no mutex lock
		 * to be atomic
		 */
		//bool m_bNeedFolderRunning;
		/**
		 * whether this folder running
		 */
		bool m_bFolderRunning;
		/**
		 * on which CPU time should differ by database writing for folder length
		 */
		short m_nFolderCPUtime;
		/**
		 * structure of
		 * created CPU time
		 */
		static CpuTime_t m_tCpuTime;
		/**
		 * vector of all started times
		 * when information folders not can be written
		 */
		vector<ppi_time> m_vStartTimes;
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
		 * starting times when running will be counted.<br />
		 * (ppi-client was started with parameter SHOW)<br />
		 * contain also who starting inside vector
		 */
		map<ppi_time, vector<InformObject> > m_vStartingCounts;
		/**
		 * from which folder:subroutine the thread
		 * was informed to restart
		 */
		vector<InformObject> m_vInformed;
		/**
		 * scheduling policy in which folder thread should running
		 */
		int m_nSchedPolicy;
		/**
		 * scheduling priority in which policy of folder thread should running
		 */
		int m_nSchedPriority;
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
		 * mutex to create or remove informer cache
		 */
		pthread_mutex_t *m_INFORMERCACHECREATION;
		/**
		 * condition for wait for new changing of any subroutine
		 */
		pthread_cond_t *m_VALUECONDITION;
		/**
		 * mutex of creating CPU time
		 */
		static pthread_mutex_t *m_CREATECPUTIMEMUTEX;
		/**
		 * mutex of reading CPU time
		 * when other thread will be creating
		 */
		static pthread_mutex_t *m_READCPUTIMEMUTEX;
		/**
		 * condition of waiting
		 * for new calculated CPU time
		 */
		static pthread_cond_t * m_NEWCPUTIMECONDITION;
		/**
		 * vector of all exist informer caches
		 */
		vector<SHAREDPTR::shared_ptr<MeasureInformerCache> > m_voInformerCaches;
		/**
		 * thread to inform other and own folder when one subroutine changing
		 */
		SHAREDPTR::shared_ptr<Informer> m_oInformer;
		/**
		 * database filler pool
		 */
		SHAREDPTR::shared_ptr<IDbFillerPattern> m_oDbFiller;

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
		 *
		 * @return whether folder should start
		 */
		bool waitForStart() const;
		/**
		 * check whether inside folder is an new time to restart
		 * only when lock is given,
		 * and write info by debug session
		 *
		 * @param vInformed vector of which folder or external clients are informed to start
		 * @param debug session of debug output
		 * @param bRemove whether should cache remove old informations
		 * @return whether folder should start
		 */
		bool checkToStart(vector<InformObject>& vInformed, const bool debug);
		/**
		 * write debugging output by start folder thread
		 *
		 * @param time current time of starting
		 */
		void doDebugStartingOutput(const ppi_time& time);
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
