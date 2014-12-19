/**
 *   This file 'MinMaxTimes.h' is part of ppi-server.
 *   Created on: 05.06.2014
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

#ifndef MINMAXTIMES_H_
#define MINMAXTIMES_H_

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>

#include "../util/stream/ppivalues.h"

/*
 * by setting this definition
 * do not output any
 * regular folder, id, policy , times, ...
 * which made by methods:
 *           writeStarting()
 *           writeDifference()
 *           writeFolderSubroutine()  (not used now)
 *           writeID()                (not used now)
 *           listEntries()
 *           writeStatistic()
 *           writeEnding()
 *  this only maybe helpful
 *  for develop and understand
 *  behavior of mconfig application
 */
//#define __NOOUTPUT_BYRUNNING


using namespace std;

	/**
	 * structure of reading values
	 * stored inside database
	 * around 'runlength'
	 */
	struct t_runlength
	{
		/**
		 * for 7/8 longest folder run time
		 * to estimate length
		 */
		double runlength;
		/**
		 * currently fetching runlength by CPU-time
		 */
		double runpercent;
		/**
		 * measure 'runnlength' always by this maximal count
		 * to write into database
		 */
		double maxcount;

		/**
		 * constructor to set all values to 0
		 */
		t_runlength()
		: runlength(0),
		  runpercent(0),
		  maxcount(0)
		{};
	};
	/**
	 * structure of reading values
	 * stored inside database
	 * around 'reachend'
	 */
	struct t_reachend
	{
		/**
		 * kernel policy of current running folder
		 */
		int policy;
		/**
		 * kernel priority of current running folder
		 */
		int priority;
		/**
		 * run status of TIMER subroutine.<br />
		 * <table>
		 *   <tr>
		 *     <td>
		 *       0
		 *     </td>
		 *     <td>
		 *       -
		 *     </td>
		 *       normally
		 *     <td>
		 *     </td>
		 *   </tr>
		 *   <tr>
		 *     <td>
		 *       1
		 *     </td>
		 *     <td>
		 *       -
		 *     </td>
		 *       exact stopping
		 *     <td>
		 *     </td>
		 *   </tr>
		 *   <tr>
		 *     <td>
		 *       2
		 *     </td>
		 *     <td>
		 *       -
		 *     </td>
		 *       every wait
		 *     <td>
		 *     </td>
		 *   </tr>
		 *   <tr>
		 *     <td>
		 *       3
		 *     </td>
		 *     <td>
		 *       -
		 *     </td>
		 *       start external
		 *     <td>
		 *     </td>
		 *   </tr>
		 * </table><br />
		 * (synchronized with database entry timerstat, defined on end of <code>init()</code> method from TIMER object)
		 */
		short timerstat;
		/**
		 * starting time of Server
		 */
		ppi_time tmStart;
		/**
		 * whether server was started
		 * with option --timerdblog
		 * for current entry
		 */
		bool timeDbLog;
		/**
		 * running ID from reachend
		 */
		string ID;
		/**
		 * for middle length of reaching late finish-position
		 */
		ppi_value reachend;
		/**
		 * current time of fetching reachend
		 */
		ppi_time tmFetch;
		/**
		 * store 'reachend' always by this maximal count
		 */
		ppi_value maxcount;
		/**
		 * cpu percent taking estimated finish time 'reachend'
		 */
		ppi_value reachpercent;
		/**
		 * which time want to measure
		 */
		ppi_value wanttime;
		/**
		 * when subroutine was informed to late for exact calculation
		 */
		ppi_value informlate;
		/**
		 * when starting time of subroutine was to late
		 */
		ppi_value startlate;
		/**
		 * reach currently late finish-position
		 */
		ppi_value reachlate;
		/**
		 * wrong difference to reach finish-position dependent to 'reachend'
		 */
		ppi_value wrongreach;
		/**
		 * by which cpu percent taking 'runlength' time
		 */
		ppi_value runpercent;
		/**
		 * current runlength time
		 */
		ppi_value runlength;
		/**
		 * by which current maximal count
		 * new runlength time was safed
		 */
		ppi_value runlengthcount;

		/**
		 * constructor to set all values to 0
		 */
		t_reachend()
		: policy(SCHED_OTHER),
		  priority(-1),
		  timerstat(0),
		  reachend(0),
		  maxcount(0),
		  reachpercent(0),
		  wanttime(0),
		  informlate(0),
		  startlate(0),
		  reachlate(0),
		  wrongreach(0),
		  runpercent(0),
		  runlength(0),
		  runlengthcount(0)
		{};
	};
	/**
	 * calculated average values
	 */
	struct t_averageVals
	{
		/**
		 * count of entries to calculate average
		 */
		unsigned int nCount;
		/**
		 * minimal length of time after stopping
		 */
		ppi_value dMinLength;
		/**
		 * maximal length of time after stopping
		 */
		ppi_value dMaxLength;
		/**
		 * how often the exact stopping time
		 * of TIMER routine wasen't reached
		 */
		unsigned int nStopOverrun;
		/**
		 * how often the wanted time of measure
		 * was overrun
		 */
		unsigned int nOverrun;
		/**
		 * Average of length time after stopping
		 */
		ppi_value dAverageLength;
		/**
		 * minimal wrong estimated time
		 */
		ppi_value dMinEstimate;
		/**
		 * maximal wrong estimated time
		 */
		ppi_value dMaxEstimate;
		/**
		 * longest wrong estimated time
		 */
		ppi_value dLongEstimate;
		/**
		 * average of wrong time estimation
		 */
		ppi_value dAverageEstimate;
		/**
		 * average of first information of TIMER routine
		 */
		ppi_value dAverageInforming;
		/**
		 * shortest time of fist informint from TIMER routine
		 */
		ppi_value dMinInforming;
		/**
		 * longest time of first informing from TIMER routine
		 */
		ppi_value dMaxInforming;

		/**
		 * constructor to set all values to 0
		 */
		t_averageVals()
		: nCount(0),
		  dMinLength(99999),
		  dMaxLength(0),
		  nStopOverrun(0),
		  nOverrun(0),
		  dAverageLength(0),
		  dMinEstimate(99999),
		  dMaxEstimate(-99999),
		  dLongEstimate(0),
		  dAverageEstimate(0),
		  dAverageInforming(0),
		  dMinInforming(99999),
		  dMaxInforming(0)
		{};
	};
	/**
	 * structure of minimal, maximal time values
	 * and also some starting times
	 */
	class MinMaxTimes
	{
	public:
		/**
		 * constructor to set null values
		 */
		MinMaxTimes()
		{ m_bNewRun= true; reset(); };
		/**
		 * set inherit values to null
		 */
		void reset()
		{
			m_sFolderSubroutine= "";
			m_bListAll= false;
			m_bInformLate= false;
			m_bExactStop= false;
			m_bReachend= false;
			m_bEstimated= false;
			m_bFolderSort= false;
			m_bExactStopSort= false;
			m_bEstimateTimeSort= false;
			m_bDifference= false;
			m_bCrashed= false;
			m_tmStarting.clear();
		};
		/**
		 * reset first folder setting to null
		 */
		void resetFirstFolders()
		{
			m_tmFirstStarting.clear();
			m_vReachendTimes.clear();
		};
		/**
		 * whether both values (length and estimate)
		 * be defined
		 */
		bool isSet() const
		{
			if(!m_vReachendTimes.empty())
				return true;
			return false;
		}
		/**
		 * set minimal and maximal of length time
		 * after stopping and estimated time
		 *
		 * @param reachend object of all read reaching end values from database
		 */
		void setTimes(const t_reachend& reachend);
		/**
		 * return folder name with subroutine name
		 *
		 * @return folder:subroutine
		 */
		string getFolderSubroutine() const
		{ return m_sFolderSubroutine; };
		/**
		 * set folder and subroutine name
		 *
		 * @param folderSub name of folder with subroutine
		 */
		void setFolderSubroutine(const string& folderSub)
		{ m_sFolderSubroutine= folderSub; }
		/**
		 * write policy string with priority
		 *
		 * @param value current reachend structure
		 */
		static string getPolicyString(const t_reachend value)
		{ return "with scheduling " + getPolicyString(value.policy, value.priority); };
		/**
		 * write policy string with priority
		 *
		 * @param policy number of policy
		 * @param priority number of priority
		 */
		static string getPolicyString(int policy, int priority);
		/**
		 * calculate average of current values
		 *
		 * @param current new values of current measure
		 * @param average current calculated average of values
		 */
		void calculateAverage(const t_reachend& current, t_averageVals* average)
		{
			ppi_value ntime(current.wrongreach);

			++average->nCount;
			if(current.reachlate < average->dMinLength)
				average->dMinLength= current.reachlate;
			if(current.reachlate > average->dMaxLength)
				average->dMaxLength= current.reachlate;
			average->dAverageLength= (average->dAverageLength + current.reachlate) / 2;
			if(current.wrongreach < average->dMinEstimate)
				average->dMinEstimate= current.wrongreach;
			if(current.wrongreach > average->dMaxEstimate)
				average->dMaxEstimate= current.wrongreach;
			if(ntime < 0)
				ntime*= -1;
			if(ntime > average->dLongEstimate)
				average->dLongEstimate= ntime;
			if(m_vReachendTimes.size() == 2)
			{// the first reachend values will be calculated
				average->dAverageEstimate= current.wrongreach;
				average->dAverageInforming= current.informlate;
			}else
			{
				average->dAverageEstimate= (average->dAverageEstimate + current.wrongreach) / 2;
				average->dAverageInforming= (average->dAverageInforming + current.informlate) / 2;
			}
			if(average->dMinInforming > current.informlate)
				average->dMinInforming= current.informlate;
			if(average->dMaxInforming < current.informlate)
				average->dMaxInforming= current.informlate;
			if(current.startlate > 0)
				++average->nStopOverrun;
			if((current.informlate + current.reachlate + current.startlate) > current.wanttime)
				++average->nOverrun;
		}
		/**
		 * calculate average of some average
		 *
		 * @param current some average calculated from current part
		 * @param average current calculated average of values
		 */
		void calculateAverage(const t_averageVals* current, t_averageVals& average)
		{
			average.nCount+= current->nCount;
			if(current->dMinLength < average.dMinLength)
				average.dMinLength= current->dMinLength;
			if(current->dMaxLength > average.dMaxLength)
				average.dMaxLength= current->dMaxLength;
			average.dAverageLength= (average.dAverageLength + current->dAverageLength) / 2;
			if(current->dMinEstimate < average.dMinEstimate)
				average.dMinEstimate= current->dMinEstimate;
			if(current->dMaxEstimate > average.dMaxEstimate)
				average.dMaxEstimate= current->dMaxEstimate;
			if(current->dLongEstimate > average.dLongEstimate)
				average.dLongEstimate= current->dLongEstimate;
			if(m_vReachendTimes.size() == 2)
			{// the first reachend values will be calculated
				average.dAverageEstimate= current->dAverageEstimate;
				average.dAverageInforming= current->dAverageInforming;
			}else
			{
				average.dAverageEstimate= (average.dAverageEstimate + current->dAverageEstimate) / 2;
				average.dAverageInforming= (average.dAverageInforming + current->dAverageInforming) / 2;
			}
			if(average.dMinInforming > current->dMinInforming)
				average.dMinInforming= current->dMinInforming;
			if(average.dMaxInforming < current->dMaxInforming)
				average.dMaxInforming= current->dMaxInforming;
			average.nStopOverrun+= current->nStopOverrun;
			average.nOverrun+= current->nOverrun;
		}
		/**
		 * get shortest informing time
		 * of TIMER subroutine
		 *
		 * @param values calculated average values
		 * @return shortest informing time
		 */
		ppi_value getMinInforming(const t_averageVals& values) const
		{
			if(values.dMinInforming == 99999)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().informlate;
				return 0;
			}
			return values.dMinInforming;
		};
		/**
		 * get maximal time form first informing time of TIMER routine
		 *
		 * @param values calculated average values
		 * @return maximal time
		 */
		ppi_value getMaxInforming(const t_averageVals& values) const
		{
			if(values.dMaxInforming == 0)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().informlate;
				return 0;
			}
			return values.dMaxInforming;
		};
		/**
		 * get minimal length of time
		 * after stopping
		 *
		 * @param values calculated average values
		 * @return minimal length time
		 */
		ppi_value getMinLength(const t_averageVals& values) const
		{
			if(values.dMinLength == 99999)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().reachlate;
				return 0;
			}
			return values.dMinLength;
		};
		/**
		 * get maximal length of time
		 * after stopping
		 *
		 * @param values calculated average values
		 * @return maximal length time
		 */
		ppi_value getMaxLength(const t_averageVals& values) const
		{
			if(values.dMaxLength == 0)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().reachlate;
				return 0;
			}
			return values.dMaxLength;
		};
		/**
		 * get average of time length after stopping
		 *
		 * @param values calculated average values
		 * @return average time
		 */
		ppi_value getAverageLength(const t_averageVals& values) const
		{ return values.dAverageLength; };
		/**
		 * get minimal wrong estimated time
		 *
		 * @param values calculated average values
		 * @return minimal estimated time
		 */
		ppi_value getMinEstimate(const t_averageVals& values) const
		{
			if(values.dMinEstimate == 99999)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().wrongreach;
				return 0;
			}
			return values.dMinEstimate;
		};
		/**
		 * get maximal wrong estimated time
		 *
		 * @param values calculated average values
		 * @return maximal estimated time
		 */
		ppi_value getMaxEstimate(const t_averageVals& values) const
		{
			if(values.dMaxEstimate == -99999)
			{// only one time be set
				if(m_vReachendTimes.size() == 1)
					return m_vReachendTimes.front().wrongreach;
				return 0;
			}
			return values.dMaxEstimate;
		};
		/**
		 * get longest wrong estimated time
		 *
		 * @param values calculated average values
		 * @return longest estimated time
		 */
		ppi_value getLongestMiscalculated(const t_averageVals& values) const
		{
			if(m_vReachendTimes.size() == 1)
			{// only one time be set
				ppi_value nRv;

				nRv= m_vReachendTimes.front().wrongreach;
				if(nRv < 0)
					nRv*= -1;
				return nRv;
			}
			return values.dLongEstimate;
		};
		/**
		 * get average wrong estimated time
		 *
		 * @param values calculated average values
		 * @return average time
		 */
		ppi_value getAverageEstimation(const t_averageVals& values) const
		{ return values.dAverageEstimate; };
		/**
		 * get average form first informing time of TIMER routine
		 *
		 * @param values calculated average values
		 * @return average time
		 */
		ppi_value getAverageInforming(const t_averageVals& values) const
		{ return values.dAverageInforming; };
		/**
		 * set current starting time of server
		 *
		 * @param time starting time
		 */
		void setStarting(const ppi_time& time)
		{ m_tmStarting= time; };
		/**
		 * get how often reading different
		 * length times after stopping
		 */
		size_t getCount() const
		{ return m_vReachendTimes.size(); };
		/**
		 * get first starting time of MinMaxTimes object
		 *
		 * @return first starting time
		 */
		ppi_time getFirstStarting()
		{
			return m_tmFirstStarting;
		}
		/**
		 * add time lengths to current
		 *
		 * @param tmOthers minimal maximal times to subtract
		 */
		void add(const MinMaxTimes& tmOthers);
		/**
		 * set writing options
		 *
		 * @param bListAll whether should list all times defined inside database
		 * @param bInformLate whether should list all first information of subroutine TIMER
		 * @param bExactStop whether should list only defined times after exact stopping
		 * @param bEstimated whether should list only fault estimated times
		 * @param bReachend whether should list only defined reachend values
		 * @param bFolderSort sorting output by folder/subroutine, otherwise by running folder ID
		 */
		void setOptions(bool bListAll, bool bInformLate, bool bExactStop, bool bEstimated, bool bReachend,
						bool bFolderSort, bool bExactStopSort, bool bEstimateTimeSort)
		{
			m_bListAll= bListAll;
			m_bInformLate= bInformLate;
			m_bExactStop= bExactStop;
			m_bEstimated= bEstimated;
			m_bReachend= bReachend;
			m_bFolderSort= bFolderSort;
			m_bExactStopSort= bExactStopSort;
			m_bEstimateTimeSort= bEstimateTimeSort;
		}
		/**
		 * set policy of different folder:subroutines
		 * and calculate which are not used
		 *
		 * @param policy all available policy which founded for folders
		 * @param priority all available priority founded for folders
		 * @param used all policy which are used
		 */
		void setPolicy(const map<string, double > policy, const map<string, double > priority, const set<string> used);
		/**
		 * set ending time of starting period
		 * from server
		 *
		 * @param time ending time
		 * @param crashed whether ending time was by regular shut down
		 */
		void setEndingTime(const ppi_time& time, bool crashed)
		{	m_tmEndingTime= time;
			m_bCrashed= crashed; 	};
		/**
		 * count digits of given value
		 *
		 * @param value integer of reaching time
		 * @return how much digits counted
		 */
		short countDigits(int value) const;
		/**
		 * display on command line starting time
		 */
		void writeStarting() const;
		/**
		 * display current folder with subroutine
		 */
		void writeFolderSubroutine() const;
		/**
		 * display current running folder ID
		 */
		void writeID() const;
		/**
		 * write all difference to last MinMaxTimes object.<br />
		 * when object is NULL, write all preferences
		 *
		 * @param nblock block number increments every new starting time
		 * @param pLast last MinMaxTimes object
		 */
		void writeDifference(long nblock, MinMaxTimes* pLast);
		/**
		 * write statistic of all read values
		 *
		 * @param bfirst whether statistic output is for first starting time block
		 * @param bResult whether statistic output is an result of more than one time started ppi-server
		 * @param values calculated average values
		 */
		void writeStatistic(bool bfirst, bool bResult, const t_averageVals* values) const;
		/**
		 * list separate specific entries
		 */
		void listEntries();
		/**
		 * display on command line ending time
		 */
		void writeEnding() const;

	private:
		/**
		 * name of folder with subroutine
		 */
		string m_sFolderSubroutine;
		/**
		 * first starting time of object
		 */
		ppi_time m_tmFirstStarting;
		/**
		 * whether new measuring session beginning
		 */
		bool m_bNewRun;
		/**
		 * list all times defined inside database
		 */
		bool m_bListAll;
		/**
		 * list only times when subroutine was informed
		 * after start measuring
		 */
		bool m_bInformLate;
		/**
		 * list only defined times after exact stopping
		 * and fault estimated times
		 */
		bool m_bExactStop;
		/**
		 * list only fault estimated times
		 */
		bool m_bEstimated;
		/**
		 * list only defined reachend values
		 */
		bool m_bReachend;
		/**
		 * whether output should show difference
		 * between starting times
		 */
		bool m_bDifference;
		/**
		 * sorting output by folder:subroutine (true)<br>
		 * or running folder ID (false)
		 */
		bool m_bFolderSort;
		/**
		 * sorting output by exact stopping time
		 */
		bool m_bExactStopSort;
		/**
		 * sorting output by wrong estimated times
		 */
		bool m_bEstimateTimeSort;
		/**
		 * starting time of server
		 */
		ppi_time m_tmStarting;
		/**
		 * ending time of starting period of server
		 */
		ppi_time m_tmEndingTime;
		/**
		 * whether ending of server
		 * shutdown on regular process
		 */
		bool m_bCrashed;
		/**
		 * all different setting reaching end times
		 */
		vector<t_reachend> m_vReachendTimes;
		/**
		 * all folders with other policy than SCHED_OTHER
		 * but has no exact TIMER subroutine
		 */
		map<string, pair<double, double> > m_mUnused;
	};

#endif /* MINMAXTIMES_H_ */
