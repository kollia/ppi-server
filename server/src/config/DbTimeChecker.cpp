/**
 *   This file 'DbTimeChecker.cpp' is part of ppi-server.
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

#include "DbTimeChecker.h"

#include "../database/lib/DatabaseFactory.h"

using namespace ppi_database;

int DbTimeChecker::execute(const ICommandStructPattern* params, InterlacedProperties* properties)
{
	/**
	 * server will be initialed
	 */
	bool bInitialing(false);
	/**
	 * whether new calculation beginning
	 * where reachend and runlength begin
	 * with null
	 */
	bool bNewBegin(false);
	bool bTimeDbLog(false);
	/**
	 * server do running
	 */
	bool brun(false);
	string stime;
	IPPIDatabasePattern* db;
	vector<vector<db_t> > dbVector;
	ppi_time tmStartReading;
	ppi_time tmStartingTime;
	ppi_time tmFetchReachend;
	ppi_time tmLastTime;
	ostringstream startstopOut;
	map<string, t_runlength > mSetRunlength;
	map<string, t_runlength >::iterator itRunlength;

	m_bListAll= params->hasOption("list");
	m_bStarting= params->hasOption("starting");
	if(	m_bListAll &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)," << endl;
		cout << "       and list all times (--list) are not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bExactStop= params->hasOption("exactstop");
	if(	m_bExactStop &&
		m_bStarting		)
	{
		cout << "ERROR: combination ofstarting times (--starting)," << endl;
		cout << "       and option --exactstop are not allowed" << endl;
		return EXIT_FAILURE;
	}
	if(m_bExactStop)
		m_bStarting= false;
	m_bReachend= params->hasOption("reachend");
	if(	m_bReachend &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)," << endl;
		cout << "       and list only new defined reaching end (--reachend) are not allowed" << endl;
		return EXIT_FAILURE;
	}
	stime= params->getOptionContent("from");
	if(stime != "")
	{
		m_oFromTime.read(stime, "%d.%m.%Y %H:%M:%S %N");
		if(m_oFromTime.error())
		{
			cout << "ERROR: cannot read time after --from option" << endl;
			cout << m_oFromTime.errorStr() << endl;
			return EXIT_FAILURE;
		}
		cout << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		cout << "reading times down from " << m_oFromTime.toString(true) << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	}
	stime= params->getOptionContent("to");
	if(stime != "")
	{
		m_oToTime.read(stime, "%d.%m.%Y %H:%M:%S %N");
		if(m_oToTime.error())
		{
			cout << "ERROR: cannot read time after --to option" << endl;
			cout << "       " << m_oToTime.errorStr() << endl;
			return EXIT_FAILURE;
		}
		cout << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		cout << "reading times down to " << m_oToTime.toString(true) << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	}

	db= DatabaseFactory::getChoosenDatabase(properties, NULL);
	db->readInsideSubroutine("ppi-server:starting", -1, 0, 0);
	cout << endl;
	db->read(/*show loading*/true);
	cout << endl << endl;
	dbVector= db->getReadSubBlock();
	for(vector<vector<db_t> >::iterator it= dbVector.begin(); it != dbVector.end(); ++it)
	{
		for(vector<db_t>::iterator line= it->begin(); line != it->end(); ++line)
		{
			bool bFoundValue(false);
			ppi_value nValue(0);

			if(!line->values.empty())
			{
				bFoundValue= true;
				nValue= line->values[0];
			}
			if(bFoundValue)
			{
				if( line->folder == "ppi-server" &&
					line->subroutine == "starting" &&
					line->identif == "starting"			)
				{

					ppi_time time;

					if(nValue == -1)
					{
						bInitialing= true;
						if(brun)
						{
							brun= false;
							startstopOut << "Server crashed on " << tmLastTime.toString(/*date*/true) << endl;
							startstopOut << "----------------------------------------------------------------------------------------------------------------------" << endl;
						}
						tmStartingTime= line->tm;
						string sStart(tmStartingTime.toString(true));

						if(sStart.substr(0, 6) == "11.05.")
							cout << flush;
						if(!tmStartReading.isSet())
							tmStartReading= tmStartingTime;
						if(needStatistic(bTimeDbLog))
							doStatistic(tmStartReading, tmLastTime);

					}else if(nValue == 1)
					{
						struct tm ttime;
						ostringstream timemsg;
						string sStart(tmStartingTime.toString(true));

						if(sStart.substr(0, 6) == "11.05.")
							cout << flush;
						bInitialing= false;
						if(bNewBegin)
						{
							bool bLog;

							// set wrong boolean value
							// to write always statistic when needed
							bLog= bTimeDbLog ? false : true;
							if(needStatistic(bLog))
								doStatistic(tmStartReading, tmLastTime);
							bNewBegin= false;
							mSetRunlength.clear();
						}
						if(brun)
						{
							startstopOut << "Server crashed on " << tmLastTime.toString(/*date*/true) << endl;
							startstopOut << "----------------------------------------------------------------------------------------------------------------------" << endl;
						}
						brun= true;
						startstopOut << "Server starting on " << tmStartingTime.toString(/*date*/true) << endl;
						time= line->tm;
						time-= tmStartingTime;
						if(localtime_r(&time.tv_sec, &ttime) == NULL)
							startstopOut << " cannot create correct localtime to calculate server starting time" << endl;
						else
						{
							if(ttime.tm_min)
							{
								timemsg << ttime.tm_min << " minute";
								if(ttime.tm_min > 1)
									timemsg << "s";
								timemsg << " and ";
							}
							timemsg << ttime.tm_sec << "." << time.tv_usec << " seconds";
							startstopOut << "  and running after " << timemsg.str() << endl;
						}
					}else
					{// nValue is 0 for stopping
						brun= false;
						time= line->tm;
						startstopOut << "Server stopping on " << time.toString(/*date*/true) << endl;
						startstopOut << "----------------------------------------------------------------------------------------------------------------------" << endl;
					}
				}else // ( 	line->folder == "ppi-server" &&
					//		line->subroutine == "starting" &&
					//		line->identif == "starting"			)
				if(	brun &&
					(	!m_oFromTime.isSet() ||
						m_oFromTime <= line->tm	) &&
					(	!m_oToTime.isSet() ||
						m_oToTime >= line->tm	)	)
				{
					t_reachend* pLastReachend;

					pLastReachend= getLastReachendValues(line->folder, line->subroutine);
					if(line->identif == "wanttime")
					{
						bTimeDbLog= true;
						pLastReachend->wanttime= nValue;

					}else if(line->identif == "runpercent")
					{
						bTimeDbLog= true;
						mSetRunlength[line->subroutine].runpercent= nValue;

					}else if(line->identif == "reachpercent")
					{
						bTimeDbLog= true;
						pLastReachend->reachpercent= nValue;

					}else if(line->identif == "reachlate")
					{
						bTimeDbLog= true;
						pLastReachend->reachlate= nValue;

					}else if(line->identif == "wrongreach")
					{
						bTimeDbLog= true;
						pLastReachend->wrongreach= nValue;

					}else if(line->identif == "informlate")
					{
						bTimeDbLog= true;
						pLastReachend->informlate= nValue;

					}else if(line->identif == "startlate")
					{
						bTimeDbLog= true;
						pLastReachend->startlate= nValue;

					}else if(line->identif == "maxcount")
					{
						if(line->folder == "folder")
							mSetRunlength[line->subroutine].maxcount= nValue;
						else
							pLastReachend->maxcount= nValue;

					}else if(line->identif == "runlength")
					{
						mSetRunlength[line->subroutine].runlength= nValue;

					}else if(line->identif.substr(0, 8) == "reachend")
					{
						string sRun;
						t_reachend tReachend;
						map<string, double>::iterator itP;

						if(needStatistic(bTimeDbLog))
							doStatistic(tmStartReading, tmLastTime);
						if(line->identif.length() > 8)
						{
							string::size_type nLen;
							ostringstream out;

							sRun= line->identif.substr(8);
							tReachend.ID= sRun;
							// calculte first how much caracter has
							// count of folder
							nLen= sRun.length();
							out << nLen;
							nLen= out.str().length();
							// create folder string
							out.str("");
							out << countFolders(sRun) << "-" << sRun;
							sRun= string("").append(nLen-1, ' ') + out.str();
						}else
							sRun= "-";
						itP= m_mPolicy.find(line->folder);
						if(itP != m_mPolicy.end())
							tReachend.policy= static_cast<int>(itP->second);
						itP= m_mPriority.find(line->folder);
						if(itP != m_mPriority.end())
							tReachend.priority= static_cast<int>(itP->second);
						tReachend.reachend= nValue;
						tReachend.maxcount= pLastReachend->maxcount;
						tReachend.reachpercent= pLastReachend->reachpercent;
						tReachend.wanttime= pLastReachend->wanttime;
						tReachend.informlate= pLastReachend->informlate;
						tReachend.startlate= pLastReachend->startlate;
						tReachend.reachlate= pLastReachend->reachlate;
						tReachend.wrongreach= pLastReachend->wrongreach;
						tReachend.tmStart= tmStartingTime;
						tReachend.tmFetch= line->tm;
						tReachend.runpercent= mSetRunlength[line->folder].runpercent;
						tReachend.runlength= mSetRunlength[line->folder].runlength;
						tReachend.runlengthcount= mSetRunlength[line->folder].maxcount;
						m_mReachend[sRun][line->folder][line->subroutine].push_back(tReachend);
						pLastReachend->informlate= 0;
						pLastReachend->startlate= 0;

					}// if(line->identif.substr(0, 8) == "reachend")

				}else // if(brun)
					if(bInitialing)
					{
						if(line->identif == "policy")
						{
							m_mPolicy[line->subroutine]= nValue;

						}else if(line->identif == "priority")
						{
							m_mPriority[line->subroutine]= nValue;

						}else if(line->identif.substr(0, 8) == "reachend")
						{
							if(nValue == 0)
								bNewBegin= true;
						}
					}// end else if(bInitialing)
			}// end if(bFoundValue)
			tmLastTime= line->tm;
		}
	}
	if(brun)
	{
		startstopOut << "Server crashed or last reading time on " << tmLastTime.toString(/*date*/true) << endl;
		startstopOut << "----------------------------------------------------------------------------------------------------------------------" << endl;
	}
	// set wrong boolean value
	// to write always statistic when needed
	bTimeDbLog= bTimeDbLog ? false : true;
	if(needStatistic(bTimeDbLog))
		doStatistic(tmStartReading, tmLastTime);
	if(m_bStarting)
	{
		cout << endl << endl;
		cout << startstopOut.str();
	}
	return EXIT_SUCCESS;
}

bool DbTimeChecker::needStatistic(bool bTimeDbLog)
{
	if(bTimeDbLog == m_bTimeDbLog)
		return false;
	m_bTimeDbLog= bTimeDbLog;
	if(m_mReachend.empty())
		return false;
	if(m_bListAll)
		return true;
	if(m_bExactStop)
		return true;
	if(m_bReachend)
		return true;
	// when no option be set,
	// make also statistic
	if(!m_bStarting)
		return true;
	return false;
}

void DbTimeChecker::doStatistic(ppi_time& tmStartReading, const ppi_time& tmLastReading)
{
	double dCurrentReachendMaxCount(0);
	ppi_time tmStartingTime;
	MinMaxTimes tMinMax;

	cout << endl;
	cout << "BEGIN time statistic on " << tmStartReading.toString(/*as date*/true) << endl;
	cout << "----------------------------------------------------------------------------------------------------------------------" << endl;
	cout << endl;
	for(itReachendFolder::iterator itRun= m_mReachend.begin(); itRun != m_mReachend.end(); ++itRun)
	{
		cout << endl << endl;
		cout << "running folder ID " << itRun->first << ":" << endl;
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		for(itReachendSub itFolder= itRun->second.begin(); itFolder != itRun->second.end(); ++itFolder)
		{
			for(itReachendRun itSub= itFolder->second.begin(); itSub != itFolder->second.end(); ++itSub)
			{
				cout << endl;
				cout << "   " << itRun->first << " " << itFolder->first << ":" << itSub->first << ":" << endl;
				dCurrentReachendMaxCount= 0;
				tMinMax.reset();
				tMinMax.resetFirstFolders();
				for(itReachendValue itValue= itSub->second.begin(); itValue != itSub->second.end(); ++itValue)
				{
					bool bmore;
					double seconds;

					if(itValue == itSub->second.begin())
					{
						ostringstream info;

						info << "   " << "with scheduling policy ";
						switch(itValue->policy)
						{
						case SCHED_OTHER:
							info << "SCHED_OTHER";
							break;
						case SCHED_BATCH:
							info << "SCHED_BATCH";
							break;
						case SCHED_IDLE:
							info << "SCHED_IDLE";
							break;
						case SCHED_RR:
							info << "SCHED_RR";
							break;
						case SCHED_FIFO:
							info << "SCHED_FIFO";
							break;
						default:
							info << "unknown";
							break;
						}
						info << " and priority " << itValue->priority;
						if(itValue->priority != SCHED_OTHER)
							cout << info.str() << endl;
						cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
					}
					if(itValue->maxcount != dCurrentReachendMaxCount)
					{
						cout << "            on maximal count " << itValue->maxcount << endl;
						dCurrentReachendMaxCount= itValue->maxcount;
					}
					if(	m_bListAll &&
						itValue != itSub->second.begin()	)
					{
						cout << endl;
					}
					if(tmStartingTime != itValue->tmStart)
					{
						writeMinMax(tMinMax, /*last*/false);
						tMinMax.reset();
						tmStartingTime= itValue->tmStart;
						tMinMax.setStarting(tmStartingTime);
						cout << "new STARTING " << tmStartingTime.toString(/*as date*/true) << endl;
					}
					tMinMax.setTimes(itFolder->first+":"+itSub->first, itValue->reachlate, itValue->wrongreach);

					if(m_bListAll)
					{
						cout << endl;
						if(itValue->ID != "-")
						cout << "           running under ID " << itValue->ID << endl;
						cout << "       want measure time of " << fixed << itValue->wanttime << " seconds" << endl;
						cout << "   by ending calculation on " << fixed << itValue->tmFetch.toString(/*as date*/true) << endl;
						cout.precision(0);
						cout << "    fetch reachend value by " << itValue->reachpercent << "% CPU-time" << endl;
						cout.precision(6);
						cout << "            fetch runlength " << fixed << itValue->runlength;
						cout.precision(0);
						cout << " by " << dec << itValue->runpercent << "% CPU-time" << endl;
						cout.precision(6);
						if(itValue->informlate > 0)
							cout << "             subroutine was " << fixed << itValue->informlate << " seconds informed to late,"
										" to beginning time measure" << endl;
						if(itValue->startlate > 0)
							cout << "        subroutine starting " << fixed << itValue->startlate << " seconds to late" << endl;
					}
					if(	m_bListAll ||
						m_bExactStop	)
					{
						cout << "                       need " << fixed << itValue->reachlate
										<< " seconds after stopping TIMER subroutine with 0" << endl;
					/*	seconds= itValue->reachlate;
						if(seconds < 0)
						{
							seconds*= -1;
							bmore= false;
						}else
							bmore= true;
						cout << "                       need " << seconds;
						if(bmore)
							cout << " more ";
						else
							cout << " less ";
						cout << "seconds after want exact stopping" << endl;*/
					}
					if(	m_bListAll ||
						m_bExactStop	)
					{
						seconds= itValue->wrongreach;
						if(seconds < 0)
						{
							seconds*= -1;
							bmore= false;
						}else
							bmore= true;
						cout << "                       need " << fixed << seconds;
						if(bmore)
							cout << " more ";
						else
							cout << " less ";
						cout << "seconds than estimated" << endl;
					}
					if(	m_bListAll ||
						m_bReachend		)
					{
						cout << "               reach end in " << fixed << itValue->reachend << " seconds" << endl;
					}
				}
				writeMinMax(tMinMax, /*last*/true);
			}
		}
	}
	cout << endl;
	cout << "END time statistic by " << tmLastReading.toString(/*as date*/true) << endl;
	cout << "----------------------------------------------------------------------------------------------------------------------" << endl;
	cout << endl;
	m_mLastReachend.clear();
	m_mReachend.clear();
	tmStartReading= tmLastReading;
}

void DbTimeChecker::writeMinMax(const MinMaxTimes tmMinMax, bool bLast)
{
	unsigned long nCount;
	static MinMaxTimes oFullTimes;
	double nMinLength, nMaxLength;
	double nMinEstimate, nMaxEstimate;

	if(	!m_bStarting &&
		!m_bListAll &&
		!m_bExactStop &&
		!m_bReachend	)
	{
		if(tmMinMax.isSet())
		{
			nCount= tmMinMax.getCount();
			nMinLength= tmMinMax.getMinLength();
			nMaxLength= tmMinMax.getMaxLength();
			nMinEstimate= tmMinMax.getMinEstimate();
			nMaxEstimate= tmMinMax.getMaxEstimate();
			cout << "        by reading " << nCount << " entries";
			if(	nCount > 1 &&
				!oFullTimes.isSet()	)
			{
				cout << " (do not calculate first time because that is mostly wrong)";
			}
			cout << endl;
			if(nMinLength != nMaxLength)
			{
				cout << "        reaching end differ from " << fixed << nMinLength << " to "
								<< fixed << nMaxLength << " seconds " << endl;
				cout << "            which is various differ of " << fixed << (nMaxLength - nMinLength) << " seconds" << endl;
				cout << "        wrong estimation differ from " << fixed << nMinEstimate << " to "
								<< fixed << nMaxEstimate << " seconds" << endl;
				cout << "            highest wrong estimation " << fixed << tmMinMax.getLongestMiscalculated() << " seconds" << endl;
			}else
			{
				cout << "        reaching end time of " << fixed << nMinLength << " seconds " << endl;
				cout << "              wrong estimate " << fixed << tmMinMax.getLongestMiscalculated() << " seconds" << endl;
			}
			oFullTimes.add(tmMinMax);
		}
		if(	bLast &&
			oFullTimes.isSet() &&
			tmMinMax.getCount() < oFullTimes.getCount()	)
		{
			nCount= oFullTimes.getCount();
			nMinLength= oFullTimes.getMinLength();
			nMaxLength= oFullTimes.getMaxLength();
			nMinEstimate= oFullTimes.getMinEstimate();
			nMaxEstimate= oFullTimes.getMaxEstimate();
			cout << "        -----------------------------------------------------------------------" << endl;
			cout << "        is result by reading " << nCount << " entries";
			if(nCount > 1)
				cout << " (do not calculate first time because that is mostly wrong)";
			cout << endl;
			if(nMinLength != nMaxLength)
			{
				cout << "        reaching end differ from " << fixed << nMinLength << " to "
								<< fixed << nMaxLength << " seconds " << endl;
				cout << "            which is various differ of " << fixed << (nMaxLength - nMinLength) << " seconds" << endl;
				cout << "        wrong estimation differ from " << fixed << nMinEstimate << " to "
								<< nMaxEstimate << " seconds" << endl;
				cout << "            highest wrong estimation " << fixed << oFullTimes.getLongestMiscalculated() << " seconds" << endl;
			}else
			{
				cout << "        reaching end time of " << fixed << nMinLength << " seconds " << endl;
				cout << "              wrong estimate " << fixed << oFullTimes.getLongestMiscalculated() << " seconds" << endl;
			}
			cout << "        -----------------------------------------------------------------------" << endl;
		}
	}
	if(bLast)
	{
		oFullTimes.reset();
		oFullTimes.resetFirstFolders();
	}
}

void DbTimeChecker::MinMaxTimes::add(const MinMaxTimes& tmOthers)
{
	m_nCount+= tmOthers.m_nCount;
	if(m_tmStarting > tmOthers.m_tmStarting)
		m_tmStarting= tmOthers.m_tmStarting;
	if(tmOthers.m_dMinLength < m_dMinLength)
		m_dMinLength= tmOthers.m_dMinLength;
	if(tmOthers.m_dMaxLength > m_dMaxLength)
		m_dMaxLength= tmOthers.m_dMaxLength;
	m_vLengthTimes.insert(m_vLengthTimes.end(), tmOthers.m_vLengthTimes.begin(), tmOthers.m_vLengthTimes.end());
	if(tmOthers.m_dMinEstimate < m_dMinEstimate)
		m_dMinEstimate= tmOthers.m_dMinEstimate;
	if(tmOthers.m_dMaxEstimate > m_dMaxEstimate)
		m_dMaxEstimate= tmOthers.m_dMaxEstimate;
	if(tmOthers.m_dLongEstimate > m_dLongEstimate)
		m_dLongEstimate= tmOthers.m_dLongEstimate;
	m_vEstimateTimes.insert(m_vEstimateTimes.end(), tmOthers.m_vEstimateTimes.begin(), tmOthers.m_vEstimateTimes.end());
}

DbTimeChecker::t_reachend* DbTimeChecker::getLastReachendValues(const string& folder,
                              const string& subroutine)
{
	t_reachend reachend;
	itLastReachendFolder::iterator fFold;
	itLastReachendSub fSub;

	fFold= m_mLastReachend.find(folder);
	if(fFold == m_mLastReachend.end())
	{
		m_mLastReachend[folder][subroutine]= reachend;
		fFold= m_mLastReachend.find(folder);
	}
	fSub= fFold->second.find(subroutine);
	if(fSub == fFold->second.end())
	{
		fFold->second.insert(pair<string, t_reachend>(subroutine, reachend));
		fSub= fFold->second.find(subroutine);
	}
	return &fSub->second;
}

unsigned short DbTimeChecker::countFolders(const string& runningFolders) const
{
	unsigned short nRv(0);

	for(string::const_iterator it= runningFolders.begin(); it != runningFolders.end(); ++it)
	{
		if(*it == '1')
			++nRv;
	}
	return nRv;
}



