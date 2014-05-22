/**
 *   This file 'DbTimeChecker.h' is part of ppi-server.
 *   Created on: 05.05.2014
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

#ifndef DBTIMECHECKER_H_
#define DBTIMECHECKER_H_

#include "../pattern/util/ICommandStructPattern.h"
#include "../pattern/util/IPPIDatabasePattern.h"

#include "../util/stream/ppivalues.h"

class DbTimeChecker
{
public:
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
		 * starting time of Server
		 */
		ppi_time tmStart;
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
		  priority(0),
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
			m_nCount= 0;
			m_tmStarting.clear();
			m_dMinLength= 99999;
			m_dMaxLength= 0;
			m_dAverageLength= 0;
			m_dMinEstimate= 99999;
			m_dMaxEstimate= -99999;
			m_dLongEstimate= 0;
			m_dAverageEstimate= 0;
		};
		/**
		 * reset first folder setting to null
		 */
		void resetFirstFolders()
		{
			m_vLengthTimes.clear();
			m_vEstimateTimes.clear();
			m_vSetFolder.clear();
		};
		/**
		 * whether both values (length and estimate)
		 * be defined
		 */
		bool isSet() const
		{
			if(	!m_vLengthTimes.empty() &&
				!m_vEstimateTimes.empty()	)
			{
				return true;
			}
			return false;
		}
		/**
		 * set minimal and maximal of length time
		 * after stopping and estimated time
		 *
		 * @param dLength current length time
		 * @param estimate wrong estimated time
		 */
		void setTimes(const string& folder, const ppi_value dLength, const ppi_value dTime)
		{
			ppi_value ntime(dTime);
			vector<string>::iterator found;

			++m_nCount;
			m_vLengthTimes.push_back(dLength);
			m_vEstimateTimes.push_back(dTime);
			found= find(m_vSetFolder.begin(), m_vSetFolder.end(), folder);
			if(found == m_vSetFolder.end())
			{
				m_vSetFolder.push_back(folder);
				return;
			}
			if(dLength < m_dMinLength)
				m_dMinLength= dLength;
			if(dLength > m_dMaxLength)
				m_dMaxLength= dLength;
			m_dAverageLength= (m_dAverageLength + dLength) / 2;
			if(dTime < m_dMinEstimate)
				m_dMinEstimate= dTime;
			if(dTime > m_dMaxEstimate)
				m_dMaxEstimate= dTime;
			if(ntime < 0)
				ntime*= -1;
			if(ntime > m_dLongEstimate)
				m_dLongEstimate= ntime;
			m_dAverageEstimate= (m_dAverageEstimate + dTime) / 2;
		}
		/**
		 * get minimal length of time
		 * after stopping
		 *
		 * @return minimal length time
		 */
		ppi_value getMinLength() const
		{
			if(m_dMinLength == 99999)
			{
				if(m_vLengthTimes.size() == 1)
					return m_vLengthTimes.front();
				return 0;
			}
			return m_dMinLength;
		};
		/**
		 * get maximal length of time
		 * after stopping
		 *
		 * @return maximal length time
		 */
		ppi_value getMaxLength() const
		{
			if(m_dMaxLength == 0)
			{
				if(m_vLengthTimes.size() == 1)
					return m_vLengthTimes.front();
				return 0;
			}
			return m_dMaxLength;
		};
		/**
		 * get average of time length after stopping
		 *
		 * @return average time
		 */
		ppi_value getAverageLength() const
		{ return m_dAverageLength; };
		/**
		 * get minimal wrong estimated time
		 *
		 * @return minimal estimated time
		 */
		ppi_value getMinEstimate() const
		{
			if(m_dMinEstimate == 99999)
			{
				if(m_vEstimateTimes.size() == 1)
					return m_vEstimateTimes.front();
				return 0;
			}
			return m_dMinEstimate;
		};
		/**
		 * get maximal wrong estimated time
		 *
		 * @return maximal estimated time
		 */
		ppi_value getMaxEstimate() const
		{
			if(m_dMaxEstimate == -99999)
			{
				if(m_vEstimateTimes.size() == 1)
					return m_vEstimateTimes.front();
				return 0;
			}
			return m_dMaxEstimate;
		};
		/**
		 * get longest wrong estimated time
		 *
		 * @return longest estimated time
		 */
		ppi_value getLongestMiscalculated() const
		{
			if(m_nCount == 1)
			{
				ppi_value nRv;

				nRv= m_vEstimateTimes.front();
				if(nRv < 0)
					nRv*= -1;
				return nRv;
			}
			return m_dLongEstimate;
		};
		/**
		 * get average wrong estimated time
		 *
		 * @return average time
		 */
		ppi_value getAverageEstimation() const
		{ return m_dAverageEstimate; };
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
		unsigned long getCount() const
		{ return m_nCount; };
		/**
		 * add time lengths to current
		 *
		 * @param tmOthers minimal maximal times to subtract
		 */
		void add(const MinMaxTimes& tmOthers);

	private:
		/**
		 * whether new measuring session beginning
		 */
		bool m_bNewRun;
		/**
		 * count of different values read
		 */
		unsigned long m_nCount;
		/**
		 * starting time of server
		 */
		ppi_time m_tmStarting;
		/**
		 * minimal length of time after stopping
		 */
		ppi_value m_dMinLength;
		/**
		 * maximal length of time after stopping
		 */
		ppi_value m_dMaxLength;
		/**
		 * Average of length time after stopping
		 */
		ppi_value m_dAverageLength;
		/**
		 * minimal wrong estimated time
		 */
		ppi_value m_dMinEstimate;
		/**
		 * maximal wrong estimated time
		 */
		ppi_value m_dMaxEstimate;
		/**
		 * longest wrong estimated time
		 */
		ppi_value m_dLongEstimate;
		/**
		 * average of wrong time estimation
		 */
		ppi_value m_dAverageEstimate;
		/**
		 * all different setting length times
		 */
		vector<ppi_value > m_vLengthTimes;
		/**
		 * all different setting estimated times
		 */
		vector<ppi_value > m_vEstimateTimes;
		/**
		 * all defined folders:subroutines
		 * to now whether writing first time
		 */
		vector<string> m_vSetFolder;
	};

	/**
	 * constructor to set working directory
	 *
	 * @param workdir working directory
	 */
	DbTimeChecker(const string workdir) :
		m_sWorkDir(workdir),
		m_bTimeDbLog(false)
	{};
	/**
	 * reading of hole last database stuff
	 *
	 * @param params setting options and parameters by starting
	 * @param properties settings from server.conf properties
	 */
	int execute(const ICommandStructPattern* params, InterlacedProperties* properties);

private:
	/**
	 * type definition for hole reachend map folder/subroutine/..
	 */
	typedef map< string, map<string, map<string, vector<t_reachend> > > > itReachendFolder;
	/**
	 * type definition for subroutines from reachend map
	 */
	typedef map<string, map<string, vector<t_reachend> > >::iterator itReachendSub;
	/**
	 * type definition for together running folders inside reachend map
	 */
	typedef map<string, vector<t_reachend> >::iterator itReachendRun;
	/**
	 * type definition for vector with reachend values
	 */
	typedef vector<t_reachend>::iterator itReachendValue;
	/**
	 * type definition for current last values of reachend
	 */
	typedef map<string, map<string, t_reachend> > itLastReachendFolder;
	/**
	 * type definition of subroutines for current last values of reachend
	 * inside folders
	 */
	typedef map<string, t_reachend>::iterator itLastReachendSub;

	/**
	 * working directory
	 */
	const string m_sWorkDir;
	/**
	 * whether currently was logging
	 * more time database data
	 */
	bool m_bTimeDbLog;
	/**
	 * list all times defined inside database
	 */
	bool m_bListAll;
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
	 * show starting and ending times of server
	 */
	bool m_bStarting;
	/**
	 * display times after this time
	 */
	ppi_time m_oFromTime;
	/**
	 * display times to this time
	 */
	ppi_time m_oToTime;
	/**
	 * current kernel policy for folders
	 */
	map<string, double > m_mPolicy;
	/**
	 * current kernel priority for folders
	 */
	map<string, double > m_mPriority;
	/**
	 * all reaching end time of running TIMER subroutine.<br />
	 * Separated with together running folders / folder / subroutine / maxcount, reach end time
	 */
	itReachendFolder m_mReachend;
	/**
	 * all current reachend values of folder/subroutine
	 */
	itLastReachendFolder m_mLastReachend;

	/**
	 * whether running application
	 * need to output any statistic files
	 *
	 * @param bTimeDbLog currently state of more time logging
	 * @return whether need
	 */
	bool needStatistic(bool bTimeDbLog);
	/**
	 * method to display statistic
	 *
	 * @param tmStartReading start reading time of data and give back new start reading time
	 * @param tmLastReading last reading time of data
	 */
	void doStatistic(ppi_time& tmStartReading, const ppi_time& tmLastReading);
	/**
	 * return last values iterator from reachend
	 *
	 * @param folder searching name of folder
	 * @param subroutine searching name of subroutine
	 * @return iterator to last values
	 */
	t_reachend* getLastReachendValues(const string& folder, const string& subroutine);
	/**
	 * count running folder from string
	 *
	 * @param runningFolders string of running folders in binary form
	 * @return running folders
	 */
	unsigned short countFolders(const string& runningFolders) const;
	/**
	 * count digits of given value
	 *
	 * @param value integer of reaching time
	 * @return how much digits counted
	 */
	short countDigits(int value) const;
	/**
	 * write policy string with priority
	 *
	 * @param value current reachend structure
	 */
	string getPolicyString(itReachendValue value) const;
	/**
	 * display on command line, only when need depending on the options,
	 * minimal an maximal length of time needed after exact stopping
	 * and estimated difference time
	 *
	 * @param tmMinMax minimal and maximal of length and estimated times
	 * @param bLast whether calling of method will be the last of write statistics
	 */
	void writeMinMax(const MinMaxTimes tmMinMax, bool bLast);
};

#endif /* DBTIMECHECKER_H_ */
