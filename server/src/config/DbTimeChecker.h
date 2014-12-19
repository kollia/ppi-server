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

#include "MinMaxTimes.h"

class DbTimeChecker
{
public:
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

	typedef MinMaxTimes differDef;
	typedef map<string, MinMaxTimes> mdifferDef;
	typedef map<string, map<string, MinMaxTimes> > mmdifferDef;
	typedef vector<map<string, map<string, MinMaxTimes> > > vmmdifferDef;

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
	 * list only times when subroutine was informed
	 * after start measuring
	 */
	bool m_bInformLate;
	/**
	 * list only defined times after exact stopping
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
	 * last count of starting times
	 * when should show an difference
	 * from statistic of more starting times
	 */
	unsigned short m_nDiffer;
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
	 * all exist folders
	 */
	vector<string> m_vsExistFolders;
	/**
	 * current kernel policy for folders
	 */
	map<string, double > m_mPolicy;
	/**
	 * all folders which using
	 * an other policy
	 */
	set<string> m_vUsePolicy;
	/**
	 * current kernel priority for folders
	 */
	map<string, double > m_mPriority;
	/**
	 * TIMER status of different subroutines
	 * @see t_reachend.timerstat
	 */
	map<string, map<string, short> > m_mmTimerStat;
	/**
	 * all current reachend values of folder/subroutine
	 */
	itLastReachendFolder m_mLastReachend;

	/**
	 * read database for stored content
	 *
	 * @param properties settings from server.conf properties
	 * @return read blocks of MaxMinTimes objects
	 */
	vmmdifferDef readDatabase(InterlacedProperties* properties);
	/**
	 * whether running application
	 * need to output any statistic files
	 *
	 * @param bTimeDbLog currently state of more time logging
	 * @return whether need
	 */
	bool needStatistic(bool bTimeDbLog);
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
	 * whether all differ objects be on end
	 *
	 * @param first count of first starting time read inside <code>m_vmmDiffer object
	 * @param readBlock read block of all times
	 * @param pMinMaxFolders all folders with MinMaxTimes objects
	 * @return whether all on end
	 */
	bool allEnd(int first, vmmdifferDef& readBlock, mmdifferDef::iterator* pMinMaxFolders);
	/**
	 * whether all inner differ objects be on end
	 * @param pmmMinMaxFolders all outer folders with inner MinMaxTimes objects
	 * @param pmMinMaxFolders all inner folders with MinMaxTimes objects
	 * @return whether all on end
	 */
	bool allEnd(mmdifferDef::iterator* pmmMinMaxFolders, mdifferDef::iterator* pmMinMaxFolders);
	/**
	 * write ending time of starting period
	 * from server and all policies
	 *
	 * @param timeBlock of all reading times
	 * @param time ending time
	 * @param crashed whether ending time was by regular shut down
	 */
	void writeEnding(vmmdifferDef* timeBlock, const ppi_time& time, bool crashed);
};

#endif /* DBTIMECHECKER_H_ */
