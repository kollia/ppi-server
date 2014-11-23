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
#ifndef IMEASUREPATTERN_H_
#define IMEASUREPATTERN_H_

#include <sys/time.h>

#include <string>
#include <vector>
#include <map>

#include "../../util/smart_ptr.h"

#include "../server/IClientSendMethods.h"

#include "IPPIValuesPattern.h"
#include "IInformerCachePattern.h"

namespace design_pattern_world
{
	namespace util_pattern
	{
		using namespace std;
		using namespace design_pattern_world::client_pattern;

		typedef map<short, pair<short, double> > percenttable_t;

		struct folderSpecNeed_t
		{
			bool needRun;
			bool fromCalc;
			bool isRun;
		};
		/**
		 * pattern class for MeasureThread to activate new pass by changed value
		 *
		 * @autor Alexander Kolli
		 * @version 1.0.0
		 */
		class IMeasurePattern
		{
			public:
				/**
				 * structure to measure in an algorithm maximal or middle length of time
				 */
				struct timeLen_t
				{
					/**
					 * name of reached finish ("reachend") or folder run length ("runlength")
					 * writing into database
					 */
					string stype;
					/**
					 * mane of maximal count writing into database
					 */
					string scount;
					/**
					 * count of max reading values to take for folder length
					 */
					short read;
					/**
					 * by witch count the max value reached
					 */
					short maxRadCount;
					/**
					 * count of reading length
					 * to write after timetype_t::maxVal into database
					 */
					short count;
					/**
					 * writing new actual value after follow counts
					 */
					short maxCount;
					/**
					 * maximal reading value for folder length
					 * or 100% length value for reach finished result
					 */
					double readValue;
					/**
					 * value which was written actually into database
					 */
					double dbValue;
					/**
					 * actual value which is more flexible than dbValue
					 */
					double actValue;
					/**
					 * how much values differ from the act Value
					 * and how often
					 */
					SHAREDPTR::shared_ptr<percenttable_t> reachedPercent;
					/**
					 * same as reachedPercent but only the last 10 value for runlength
					 * or last 3 for reachend
					 */
					SHAREDPTR::shared_ptr<percenttable_t> newReachedPercent;

					/**
					 * constructor to set default values
					 */
					timeLen_t()
					: read(0),
					  count(0),
					  maxCount(1),
					  readValue(0),
					  dbValue(0),
					  actValue(0),
					  reachedPercent(SHAREDPTR::shared_ptr<percenttable_t>(new percenttable_t)),
					  newReachedPercent(SHAREDPTR::shared_ptr<percenttable_t>(new percenttable_t))
					{};
				};
				/**
				 * static values for
				 * structure to measure in an algorithm maximal or middle length of time
				 */
				struct timetype_t
				{
					/**
					 * name of folder saved into database
					 */
					string folder;
					/**
					 * name of subroutine saved into database
					 */
					string subroutine;
					/**
					 * whether structure is for length running (true)
					 * or reached finished (false)
					 */
					bool runlength;
					/**
					 * inside how much values calculate middle value
					 */
					short maxVal;
					/**
					 * percent to which can differ
					 */
					short inPercent;
					/**
					 * synchronization ID of all running folder,
					 * implement for reading 'reachend' time
					 */
					string synchroID;
					/**
					 * map of time structures to calculate longest or middle time consider by CPU time,
					 * differ between synchronization ID of folders
					 */
					map< string, map<short, timeLen_t> > percentSyncDiff;
					/**
					 * log inside database for starting earlier or estimate possible finish time
					 * by which CPU percent time length be taken
					 */
					bool log;

					timetype_t()
					: runlength(false),
					  maxVal(5),
					  //percentDB(false),
					  inPercent(10),
					  log(false)
					{};
				};
				/**
				 * awake mutexe and conditions
				 * for informer cache
				 */
				struct awakecond_t
				{
					/**
					 * mutex want to inform folder to running
					 */
					pthread_mutex_t* wantinform;
					/**
					 * mutex for fill or erase new activate time
					 */
					pthread_mutex_t* activatetime;
					/**
					 * condition for wait for new changing of any subroutine
					 */
					pthread_cond_t* valuecondition;
				};

				/**
				 * method returning name of folder
				 *
				 * @return name of folder
				 */
				virtual string getFolderName() const= 0;
				/**
				 * return cache to observer changing values
				 *
				 * @param folder name of folder for which cache used
				 * @return observer cache
				 */
				virtual IInformerCachePattern* getInformerCache(const string& folder)= 0;
				/**
				 * return as parameter mutex and conditions
				 * to awake folder thread for running
				 */
				virtual awakecond_t getAwakeConditions()= 0;
				/**
				 * return cache to observer only when exist
				 *
				 * @param folder name of folder for which cache used
				 * @return observer cache when exist, otherwise NULL
				 */
				virtual IInformerCachePattern* getUsedInformerCache(const string& folder)= 0;
				/**
				 * remove observer cache when no more needed
				 *
				 * @param folder name of folder for which cache was used
				 */
				virtual void removeObserverCache(const string& folder)= 0;
				/**
				 * return ListCalculator for whether foder thread should be informed
				 *
				 * @return ListCalculator
				 */
				virtual string getInformeThreadStatement()= 0;
				/**
				 * get setting scheduling parameters
				 * of policy and priority
				 *
				 * @param policy thread policy for scheduling
				 * @param priority scheduling priority
				 */
				virtual void getSchedulingParameter(int& policy, int& priority)= 0;
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
				virtual void informFolders(const vector<pair<IInformerCachePattern*, vector<string> > >& folders,
								const InformObject& from,
								const string& as, const bool debug, pthread_mutex_t *lock)= 0;
				/**
				 * returning thread id in which thread folder object running
				 *
				 * @return thread id
				 */
				virtual pid_t getRunningThreadID()= 0;
				/**
				 * returning external send device
				 *
				 * @return sending device
				 */
				virtual IClientSendMethods* getExternSendDevice()= 0;
				/**
				 * return run specification of folder
				 *
				 * @return all specification needed
				 */
				virtual vector<string> getAllSpecs() const= 0;
				/**
				 * check whether this folder is running for work
				 *
				 * @param specs all specifications are allowed
				 * @return whether folder running
				 */
				virtual folderSpecNeed_t isFolderRunning(const vector<string>& specs)= 0;
				/**
				 * returning true if an client set this measurethread to debug
				 *
				 * @return whether measure thread do output
				 */
				virtual bool isDebug()= 0;
				/**
				 * set folder to calculating length of folder time
				 */
				virtual void calculateLengthTime()= 0;
				/**
				 * length time of folder running.<br />
				 * when get unset time back (<code>= !timerisset(<returnvalue>)</code>)
				 * no measuring be set for folder
				 *
				 * @param logPercent whether logging original percent into database
				 * @param debug whether call run in debug session
				 * @return longest measured length of folder time
				 */
				virtual IPPITimePattern& getLengthedTime(const bool& logPercent, const bool& debug)= 0;
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
												const bool& logPercent, const bool& debug)= 0;
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
								 const IPPITimePattern& length, const bool& debug)= 0;
				/**
				 * sleep microseconds by consider stopping of running thread
				 *
				 * @param time sleeping time
				 * @return whether thread should stopping
				 */
				virtual bool usleep(const IPPITimePattern& time)= 0;
				/**
				 * return actually count of current subroutine
				 *
				 * @param subroutine whitch count should be returned when set, elsewhere create new counts
				 * @return count number of subroutine
				 */
				virtual unsigned short getActCount(const string& subroutine= "")= 0;
				/**
				 * from witch folder:subroutine thread was informed for new value
				 *
				 * @return vector of folder:subroutine which informed
				 */
				virtual vector<string> wasInformed()= 0;
				/**
				 * on which time the measure routine should start without any actions on extern ports
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 */
				virtual void nextActivateTime(const string& folder, const IPPITimePattern& time)= 0;
				/**
				 * searching where folder was starting from an specific time condition
				 * and change starting time
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 * @param newtime new starting time
				 */
				virtual void changeActivationTime(const string& folder, const IPPITimePattern& time,
								const IPPITimePattern& newtime)= 0;
				/**
				 * searching where folder was starting from an specific time condition and erase starting.
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 */
				virtual void eraseActivateTime(const string& folder, const IPPITimePattern& time)= 0;
				/**
				 * subroutine signal whether can find the server for external measuring
				 *
				 * @param bfound whether server was finding
				 * @param server stype of server will be found
				 * @param id chip id searched inside server
				 */
				virtual void foundPortServer(const bool bfound, const string& server, const string& id)= 0;
				/**
				 * fill double value over an queue into database
				 *
				 * @param folder folder name from the running thread
				 * @param subroutine name of the subroutine in the folder
				 * @param identif identification of which value be set
				 * @param value value which should write into database
				 * @param bNew whether database should actualize value for client default= false
				 */
				virtual void fillValue(const string& folder, const string& subroutine, const string& identif,
								double value, bool bNew= false)= 0;
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
								const vector<double>& dvalues, bool bNew= false)= 0;
				/**
				 * dummy destructor for pattern
				 */
				virtual ~IMeasurePattern() {};

		};
	}
}

#endif /*IMEASUREPATTERN_H_*/
