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
	m_bStarting= params->hasOption("startingtime");
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
		cout << "ERROR: combination of starting times (--starting)" << endl;
		cout << "       and option --exactstop is not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bEstimated= params->hasOption("estimated");
	if(	m_bEstimated &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)" << endl;
		cout << "       and option --estimated is not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bReachend= params->hasOption("reachend");
	if(	m_bReachend &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)" << endl;
		cout << "       and list only new defined reaching end (--reachend) is not allowed" << endl;
		return EXIT_FAILURE;
	}
	if(	(	m_bExactStop ||
			m_bEstimated ||
			m_bReachend		) &&
		!m_bListAll					)
	{
		cout << "ERROR: option of ";
		if(m_bExactStop)
			cout << "--exactstop ";
		if(m_bEstimated)
			cout << "--estimated ";
		if(m_bReachend)
			cout << "--reachend ";
		cout << endl;
		cout << "       is only allowed with option --list" << endl;
		return EXIT_FAILURE;
	}
	m_bFolderSort= !params->hasOption("idsort");
	m_bExactStopSort= params->hasOption("exacttimesort");
	m_bEstimateTimeSort= params->hasOption("estimatetimesort");
	if(	m_bExactStopSort &&
		m_bEstimateTimeSort	)
	{
		cout << "ERROR: option --exacttimesort and --estimatetimesort" << endl;
		cout << "       can't be used in same time" << endl;
		return EXIT_FAILURE;
	}
	if(	(	m_bExactStopSort ||
			m_bEstimateTimeSort	) &&
		!m_bListAll					)
	{
		cout << "ERROR: option ";
		if(m_bExactStopSort)
			cout << "--exacttimesort ";
		if(m_bEstimateTimeSort)
			cout << "--estimatetimesort";
		cout << endl;
		cout << "       can only be used by list content (--list)" << endl;
	}
	stime= params->getOptionContent("begin");
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
	stime= params->getOptionContent("stop");
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
	if(	m_bExactStop ||
		m_bEstimated ||
		m_bReachend		)
	{// when one of this option be set
	 // do not list all other content
	 // is not same behavior then calling from user
	 // but easier to handle
		m_bListAll= false;
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
						if(m_bFolderSort)
							m_mReachend[line->folder][line->subroutine][sRun].push_back(tReachend);
						else
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
	ppi_value dLastReachend;
	MinMaxTimes tMinMax;

	cout << endl;
	cout << "BEGIN time statistic on " << tmStartReading.toString(/*as date*/true) << endl;
	cout << "----------------------------------------------------------------------------------------------------------------------" << endl;
	cout << endl;
	// when sort by folder (option --foldersort[m_bFolderSort]) itSort1 is folder
	// elsewhere itSort1 is running folder ID
	for(itReachendFolder::iterator itSort1= m_mReachend.begin(); itSort1 != m_mReachend.end(); ++itSort1)
	{
		if(!m_bFolderSort)
		{
			cout << endl << endl;
			cout << "running folder ID " << itSort1->first << ":" << endl;
			cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		}
		// when sort by folder (option --foldersort[m_bFolderSort]) itSort2 is subroutine
		// elsewhere itSort2 is folder
		for(itReachendSub itSort2= itSort1->second.begin(); itSort2 != itSort1->second.end(); ++itSort2)
		{
			// when sort by folder (option --foldersort[m_bFolderSort]) itSort3 is running folder ID
			// elsewhere itSort3 is subroutine
			for(itReachendRun itSort3= itSort2->second.begin(); itSort3 != itSort2->second.end(); ++itSort3)
			{
				unsigned int nRunningCount(0);
				short nMaxDigits;
				map<ppi_value, string> mTimeSort;

				if(!m_bFolderSort)
				{
					cout << endl;
					cout << "   " << itSort1->first << " " << itSort2->first << ":" << itSort3->first << ":" << endl;
				}
				dCurrentReachendMaxCount= 0;
				tMinMax.reset();
				tMinMax.resetFirstFolders();
				dLastReachend= 0;
				for(itReachendValue itValue= itSort3->second.begin(); itValue != itSort3->second.end(); ++itValue)
				{
					bool bmore;
					double seconds;
					ostringstream out;

					++nRunningCount;
					if(	m_bFolderSort &&
						itValue == itSort3->second.begin()	)
					{
						if(itSort3 == itSort2->second.begin())
						{
							cout << endl << endl;
							cout << "running folder '" << itSort1->first << ":" << itSort2->first << endl;
							cout << getPolicyString(itValue) << endl;
							cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
						}
						if(itSort2 == itSort1->second.begin())
						{
							cout << endl << endl;
							cout << "running folder ID " << itValue->ID << ":" << endl;
							cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
						}

					}else
					{
						if(itValue == itSort3->second.begin())
						{
							cout << "   " << getPolicyString(itValue) << endl;
							cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
						}
					}
					if(itValue->maxcount != dCurrentReachendMaxCount)
					{
						cout << "            on maximal count " << itValue->maxcount << endl;
						dCurrentReachendMaxCount= itValue->maxcount;
					}
					if(	m_bListAll &&
						itValue != itSort3->second.begin()	)
					{
						cout << endl;
					}
					if(tmStartingTime != itValue->tmStart)
					{
						writeMinMax(tMinMax, /*last*/false);
						tMinMax.reset();
						tmStartingTime= itValue->tmStart;
						tMinMax.setStarting(tmStartingTime);
						if(	!m_bExactStopSort &&
							!m_bEstimateTimeSort	)
						{
							cout << "new STARTING " << tmStartingTime.toString(/*as date*/true) << endl;
						}
					}
					tMinMax.setTimes(itSort2->first+":"+itSort3->first, itValue->reachlate, itValue->wrongreach);

					if(m_bListAll)
					{
						out << endl;
						if(	m_bExactStopSort ||
							m_bEstimateTimeSort	)
						{
							out << "           by " << nRunningCount << ". running" << endl;
						}
						if(itValue->ID != "-")
							out << "           running under ID " << itValue->ID << endl;
						out << "      for folder:subroutine ";
						if(m_bFolderSort)
							out << itSort1->first << ":" << itSort2->first << endl;
						else
							out << itSort2->first << ":" << itSort3->first << endl;
						out << "      " << getPolicyString(itValue) << endl;
						out << "       want measure time of " << itValue->wanttime << " seconds" << endl;
						out << "   by ending calculation on " << fixed << itValue->tmFetch.toString(/*as date*/true) << endl;
						out.precision(0);
						out << "    fetch reachend value by " << itValue->reachpercent << "% CPU-time" << endl;
						out.precision(6);
						out << "            fetch runlength " << fixed << itValue->runlength;
						out.precision(0);
						out << " by " << dec << itValue->runpercent << "% CPU-time" << endl;
						out.precision(6);
						if(itValue->informlate > 0)
							out << "             subroutine was " << fixed << itValue->informlate << " seconds informed to late,"
										" to beginning time measure" << endl;
						if(itValue->startlate > 0)
							out << "        subroutine starting " << fixed << itValue->startlate << " seconds to late" << endl;
					}
					if(	m_bListAll ||
						m_bExactStop	)
					{
						out << "                       ";
						if(	!m_bListAll &&
							(	m_bExactStopSort ||
								m_bEstimateTimeSort	)	)
						{
							out << nRunningCount << ". run ";
						}
						out << "need " << fixed << itValue->reachlate;
						out << " seconds after stopping TIMER subroutine" << endl;
					}
					if(	m_bListAll &&
						dLastReachend != 0 &&
						itValue->wanttime > itValue->reachlate	)
					{
						out << "                      minus ";
						out << dLastReachend << " seconds before estimated ending time" << endl;
					}
					if(	m_bListAll ||
						m_bEstimated	)
					{
						seconds= itValue->wrongreach;
						if(seconds < 0)
						{
							seconds*= -1;
							bmore= false;
						}else
							bmore= true;
						if(itValue->wanttime < itValue->reachlate)
						{
							if(	!m_bListAll &&
								(	m_bExactStopSort ||
									m_bEstimateTimeSort	)	)
							{
								out << "                   ";
							}else
								out << "            ";
							out << "desired time of " << itValue->wanttime;
							out << " seconds was overrun" << endl;
							if(itValue == itSort3->second.begin())
							{
								seconds-= itValue->wanttime;
								itValue->wrongreach= seconds;
							}
						}
						out << "                       ";
						if(	!m_bListAll &&
							(	m_bExactStopSort ||
								m_bEstimateTimeSort	)	)
						{
							if(!m_bExactStop)
								out << nRunningCount << ". run ";
							else
								out << "       ";
						}
						out << "need " << fixed << seconds;
						if(bmore)
							out << " more ";
						else
							out << " less ";
						out << "seconds";
						if(itValue->wanttime > itValue->reachlate)
							out << " than estimated";
						out << endl;
					}
					if(	m_bListAll ||
						m_bReachend		)
					{
						if(	!m_bListAll &&
							!m_bExactStop &&
							!m_bEstimated &&
							(	m_bExactStopSort ||
								m_bEstimateTimeSort	)	)
						{
							out << " by " << nRunningCount << ". running ";
						}
						out << " define new reaching end by " << fixed << itValue->reachend << " seconds" << endl;
					}
					dLastReachend= itValue->reachend;
					if(	m_bExactStopSort ||
						m_bEstimateTimeSort	)
					{
						short digit;
						ppi_value tm;

						if(m_bExactStopSort)
							tm= itValue->reachlate;
						else
							tm= itValue->wrongreach;
						digit= countDigits(static_cast<int>(tm));
						if(nMaxDigits < digit)
							nMaxDigits= digit;
						mTimeSort[tm]= out.str();

					}else
					{
						cout << out.str();
					}
				}
				if(	m_bExactStopSort ||
					m_bEstimateTimeSort	)
				{
					map<string, string> newTimeSort;

					for(map<ppi_value, string>::iterator it= mTimeSort.begin(); it != mTimeSort.end(); ++it)
					{
						short digit;
						ostringstream out;

						digit= countDigits(static_cast<int>(it->first));
						out << string().append((nMaxDigits - digit), '0');
						out << it->first;
						newTimeSort[out.str()]= it->second;
					}
					for(map<string, string>::iterator it= newTimeSort.begin(); it != newTimeSort.end(); ++it)
						cout << it->second;
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

string DbTimeChecker::getPolicyString(itReachendValue value) const
{
	ostringstream info;

	info << "with scheduling policy ";
	switch(value->policy)
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
	info << " and priority " << value->priority;
	return info.str();
}

short DbTimeChecker::countDigits(int value) const
{
	short nRv(1);

	while(value > 9)
	{
		++nRv;
		value/= 10;
	}
	return nRv;
}

void DbTimeChecker::writeMinMax(const MinMaxTimes tmMinMax, bool bLast)
{
	bool bNoOptionSet(false);
	unsigned long nCount;
	static MinMaxTimes oFullTimes;
	double nMinLength, nMaxLength;
	double nMinEstimate, nMaxEstimate;

	if(!m_bStarting)
	{
		if(	!m_bListAll &&
			!m_bExactStop &&
			!m_bEstimated &&
			!m_bReachend	)
		{// only sort option can be set
			bNoOptionSet= true;
		}
		if(tmMinMax.isSet())
		{
			nCount= tmMinMax.getCount();
			nMinLength= tmMinMax.getMinLength();
			nMaxLength= tmMinMax.getMaxLength();
			nMinEstimate= tmMinMax.getMinEstimate();
			nMaxEstimate= tmMinMax.getMaxEstimate();
			cout << endl;
			cout << "        by reading " << nCount << " entries";
			if(	nCount > 1 &&
				!oFullTimes.isSet()	)
			{
				cout << " (do not calculate first time because that is mostly wrong)";
			}
			cout << endl;
			if(nMinLength != nMaxLength)
			{
				if(	m_bListAll ||
					m_bExactStop ||
					bNoOptionSet	)
				{
					cout << "        reaching end differ from " << fixed << nMinLength << " to "
									<< fixed << nMaxLength << " seconds " << endl;
					cout << "            which is various differ of " << fixed << (nMaxLength - nMinLength) << " seconds" << endl;
					if(tmMinMax.getCount() > 1)
						cout << "            and has an average of " << tmMinMax.getAverageLength() << " seconds" << endl;
				}
				if(	m_bListAll ||
					m_bEstimated ||
					bNoOptionSet	)
				{
					cout << "        wrong estimation differ from " << fixed << nMinEstimate << " to "
									<< fixed << nMaxEstimate << " seconds" << endl;
					cout << "            highest wrong estimation " << fixed << tmMinMax.getLongestMiscalculated() << " seconds" << endl;
					if(tmMinMax.getCount() > 1)
						cout << "            and has an average of " << tmMinMax.getAverageEstimation() << " seconds" << endl;
				}
			}else
			{
				if(	m_bListAll ||
					m_bExactStop ||
					bNoOptionSet	)
				{
					cout << "        reaching end time of " << fixed << nMinLength << " seconds " << endl;
				}
				if(	m_bListAll ||
					m_bEstimated ||
					bNoOptionSet	)
				{
					cout << "              wrong estimate " << fixed << tmMinMax.getLongestMiscalculated() << " seconds" << endl;
				}
			}
			if(!bLast)
				cout << endl;
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
				if(	m_bListAll ||
					m_bExactStop ||
					bNoOptionSet	)
				{
					cout << "        reaching end differ from " << fixed << nMinLength << " to "
									<< fixed << nMaxLength << " seconds " << endl;
					cout << "            which is various differ of " << fixed << (nMaxLength - nMinLength) << " seconds" << endl;
					if(oFullTimes.getCount() > 1)
						cout << "            and has an average of " << oFullTimes.getAverageLength() << " seconds" << endl;
				}
				if(	m_bListAll ||
					m_bEstimated ||
					bNoOptionSet	)
				{
					cout << "        wrong estimation differ from " << fixed << nMinEstimate << " to "
									<< nMaxEstimate << " seconds" << endl;
					cout << "            highest wrong estimation " << fixed << oFullTimes.getLongestMiscalculated() << " seconds" << endl;
					if(oFullTimes.getCount() > 1)
						cout << "            and has an average of " << oFullTimes.getAverageEstimation() << " seconds" << endl;
				}
			}else
			{
				if(	m_bListAll ||
					m_bExactStop ||
					bNoOptionSet	)
				{
					cout << "        reaching end time of " << fixed << nMinLength << " seconds " << endl;
				}
				if(	m_bListAll ||
					m_bEstimated ||
					bNoOptionSet	)
				{
					cout << "              wrong estimate " << fixed << oFullTimes.getLongestMiscalculated() << " seconds" << endl;
				}
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
	if(m_dAverageLength == 0)
		m_dAverageLength= tmOthers.m_dAverageLength;
	else if(tmOthers.m_dAverageLength != 0)
		m_dAverageLength= (m_dAverageLength + tmOthers.m_dAverageLength) / 2;
	m_vLengthTimes.insert(m_vLengthTimes.end(), tmOthers.m_vLengthTimes.begin(), tmOthers.m_vLengthTimes.end());
	if(tmOthers.m_dMinEstimate < m_dMinEstimate)
		m_dMinEstimate= tmOthers.m_dMinEstimate;
	if(tmOthers.m_dMaxEstimate > m_dMaxEstimate)
		m_dMaxEstimate= tmOthers.m_dMaxEstimate;
	if(tmOthers.m_dLongEstimate > m_dLongEstimate)
		m_dLongEstimate= tmOthers.m_dLongEstimate;
	m_vEstimateTimes.insert(m_vEstimateTimes.end(), tmOthers.m_vEstimateTimes.begin(), tmOthers.m_vEstimateTimes.end());
	if(m_dAverageEstimate == 0)
		m_dAverageEstimate= tmOthers.m_dAverageEstimate;
	else if(tmOthers.m_dAverageEstimate != 0)
		m_dAverageEstimate= (m_dAverageEstimate + tmOthers.m_dAverageEstimate) / 2;
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



