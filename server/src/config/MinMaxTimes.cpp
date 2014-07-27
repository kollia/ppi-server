/**
 *   This file 'MinMaxTimes.cpp' is part of ppi-server.
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

#include "MinMaxTimes.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace boost;

void MinMaxTimes::setTimes(const t_reachend& reachend)
{
	vector<string>::iterator found;

	if(	!m_tmFirstStarting.isSet() ||
		(	reachend.tmStart.isSet() &&
			m_tmFirstStarting > reachend.tmStart	)	)
	{
		m_tmFirstStarting= reachend.tmStart;
	}
	m_vReachendTimes.push_back(reachend);
}

void MinMaxTimes::setPolicy(const map<string, double > policy, const map<string, double > priority, const set<string> used)
{
	set<string>::const_iterator found;
	map<string, double>::const_iterator prioIt;

	for(map<string, double>::const_iterator it= policy.begin(); it != policy.end(); ++it)
	{
		found= used.find(it->first);
		if(found == used.end())
		{
			double npolicy;
			double npriority(-1);
			pair<double, double> content;

			npolicy= it->second;
			prioIt= priority.find(it->first);
			if(prioIt != priority.end())
				npriority= prioIt->second;
			content= pair<double, double>(npolicy, npriority);
			m_mUnused[it->first]= content;
		}
	}
}

void MinMaxTimes::add(const MinMaxTimes& tmOthers)
{
	vector<string>::iterator foundFolder;

	if(	!m_tmFirstStarting.isSet() ||
		(	tmOthers.m_tmFirstStarting.isSet() &&
			m_tmFirstStarting > tmOthers.m_tmFirstStarting	)	)
	{
		m_tmFirstStarting= tmOthers.m_tmFirstStarting;
	}
	if(m_tmStarting > tmOthers.m_tmStarting)
		m_tmStarting= tmOthers.m_tmStarting;
	m_vReachendTimes.insert(m_vReachendTimes.end(), tmOthers.m_vReachendTimes.begin(), tmOthers.m_vReachendTimes.end());
}

void MinMaxTimes::writeStarting() const
{
	const t_reachend *first;

	first= &m_vReachendTimes.front();
	cout << "Server starting on " << first->tmStart.toString(/*date*/true) << endl;
	cout << "BEGIN time statistic on " << first->tmFetch.toString(/*date*/true) << endl;
	cout << "------------------------------------------------------------------------------------" << endl;
	cout << endl;
}

void MinMaxTimes::writeFolderSubroutine() const
{
	const t_reachend *first;

	first= &m_vReachendTimes.back();
	cout << "running folder '" << getFolderSubroutine() << endl;
	cout << getPolicyString(*first) << endl;
	cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	cout << endl;
}

void MinMaxTimes::writeID() const
{
	const t_reachend *first;

	first= &m_vReachendTimes.front();
	cout << "running folder ID " << first->ID << ":" << endl;
	cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	cout << endl;
}

void MinMaxTimes::writeEnding() const
{
	cout << endl;
	cout << "END time statistic by " << m_tmEndingTime.toString(/*date*/true) << endl;
	cout << "------------------------------------------------------------------------------------" << endl;
	cout << endl;
}

void MinMaxTimes::writeDifference(long nblock, MinMaxTimes* pLast)
{
	t_reachend* ownObj;
	t_reachend* foreignObj(NULL);
	vector<string>::iterator it;
	string ownFolders("by folder(s) ");
	string foreignFolders("by folder(s) ");
	string sOutStr;
	bool bunusedPolicy;
	string sUnusedPolicy;
	string sNewUsedPolicy;

	m_bDifference= true;
	ownObj= &m_vReachendTimes.front();
/*	it= m_vSetFolder.begin();
	if(it != m_vSetFolder.end())
	{
		ownFolders+= "'" + *it + "'\n";
		++it;
		while(it != m_vSetFolder.end())
			ownFolders+= "             '" + *it + "'\n";
	}else
		ownFolders= "no explicit folder(s) defined\n";*/
	if(pLast)
		foreignObj= &pLast->m_vReachendTimes.front();
	if(pLast == NULL)
		cout << endl << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	cout << "starting " << nblock << ". time block on " << m_tmFirstStarting.toString(/*as date*/true) << endl;
	if(	pLast == NULL ||
		pLast->m_mUnused.size() == 0	)
	{
		// show all unused policies
		bunusedPolicy= true;

	}else
	{
		// show only new unused policies
		// or removed
		if(m_mUnused.size() == 0)
		{
			bunusedPolicy= false;
			if(	pLast != NULL &&
				pLast->m_mUnused.size()	)
			{
				cout << "with all removed unused Policies" << endl;
			}
		}else
		{
			map<string, pair<double, double> >::iterator found;

			// show only new unused policies
			for(map<string, pair<double, double> >::iterator it= m_mUnused.begin();
							it != m_mUnused.end(); ++it)
			{
				bool bnew(false);

				found= pLast->m_mUnused.find(it->first);
				if(found != pLast->m_mUnused.end())
				{
					if(	found->second.first != it->second.first ||
						found->second.second != it->second.second	)
					{
						bnew= true;
					}
				}else
					bnew= true;
				if(bnew)
				{
					sUnusedPolicy+= "    '" + it->first + "' with ";
					sUnusedPolicy+= getPolicyString(static_cast<int>(it->second.first),
													static_cast<int>(it->second.second)) + "\n";
				}
			}// end of show only new unused policies

			// show removed policies
			for(map<string, pair<double, double> >::iterator it= pLast->m_mUnused.begin();
							it != pLast->m_mUnused.end(); ++it)
			{
				found= m_mUnused.find(it->first);
				if(found == m_mUnused.end())
				{
					sNewUsedPolicy+= "    '" + it->first + "' used by ";
					sNewUsedPolicy+= getPolicyString(static_cast<int>(it->second.first),
													static_cast<int>(it->second.second)) + "\n";
				}
			}
		}
	}
	if(bunusedPolicy)
	{
		if(m_mUnused.size())
		{
			for(map<string, pair<double, double> >::iterator it= m_mUnused.begin();
							it != m_mUnused.end(); ++it)
			{
				sUnusedPolicy+= "    '" + it->first + "' with ";
				sUnusedPolicy+= getPolicyString(static_cast<int>(it->second.first),
												static_cast<int>(it->second.second)) + "\n";
			}
		}
	}
	if(sUnusedPolicy != "")
	{
		cout << "where ";
		if(pLast != NULL)
			cout << "new ";
		cout << "running folder with Policy has no TIMER routine with ending data:" << endl;
		cout << sUnusedPolicy;
		if(sNewUsedPolicy == "")
			cout << endl;
	}
	if(sNewUsedPolicy != "")
	{
		cout << "where folder with policies use now TIMER routine with ending data:" << endl;
		cout << sNewUsedPolicy;
		cout << endl;
	}
	cout << "folder:subroutine '" << getFolderSubroutine() << "'" << endl;
	if(	pLast == NULL ||
		ownObj->timerstat != foreignObj->timerstat	)
	{
		cout << "";
		switch(ownObj->timerstat)
		{
		case 0:
			cout << "run as normally TIMER subroutine " << endl;
			break;
		case 1:
			cout << "run with exact stopping " << endl;
			break;
		case 2:
			cout << "run with exact stop and always waiting for end" << endl;
			break;
		case 3:
			cout << "start external subroutine when exact stopping" << endl;
			break;
		default:
			cout << "running unknown TIMER subroutine" << endl;
			break;
		}
	}
	if(	pLast == NULL ||
		ownObj->policy != foreignObj->policy ||
		ownObj->priority != foreignObj->priority	)
	{
		cout << getPolicyString(*ownObj) << endl;
	}
	if(	pLast == NULL ||
		ownObj->ID != foreignObj->ID	)
	{
		cout << "  as ID " << ownObj->ID << " " << endl;
	}
	cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	cout << endl;
}

string MinMaxTimes::getPolicyString(int policy, int priority)
{
	ostringstream info;

	info << "policy ";
	switch(policy)
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
	if(	policy == SCHED_RR ||
		policy == SCHED_FIFO	)
	{
		info << " and priority " << priority;
	}
	return info.str();
}

void MinMaxTimes::listEntries()
{
	t_averageVals allAverage;
	std::auto_ptr<t_averageVals> pCurrentAverage(new t_averageVals());
	bool bFirstContent(true);
	bool bFirstStatistic(true);
	bool bNoOptionSet(false);
	double dCurrentReachendMaxCount(0);
	ppi_time tmStartingTime;
	ostringstream out;
	unsigned int nRunningCount(0);
	ppi_value dLastReachend;
	bool bmore;
	double seconds;
	short nMaxDigits;
	map<ppi_value, string> mTimeSort;

	if(	!m_bListAll &&
		!m_bInformLate &&
		!m_bExactStop &&
		!m_bEstimated &&
		!m_bReachend	)
	{// only sort option can be set
		bNoOptionSet= true;
	}
	for(vector<t_reachend>::iterator itValue= m_vReachendTimes.begin();
					itValue != m_vReachendTimes.end(); ++itValue)
	{
		++nRunningCount;
		if(itValue->maxcount != dCurrentReachendMaxCount)
		{
			cout << "            on maximal count " << itValue->maxcount << endl;
			dCurrentReachendMaxCount= itValue->maxcount;
		}
		if(	m_bListAll &&
			itValue != m_vReachendTimes.begin()	)
		{
			cout << endl;
		}
		if(tmStartingTime != itValue->tmStart)
		{
			calculateAverage(pCurrentAverage.get(), allAverage);
			if(tmStartingTime.isSet())
			{// when tmStatingTime not be set, listing of entrys only begin
				writeStatistic(bFirstStatistic, /*result*/false, pCurrentAverage.get());
				pCurrentAverage= std::auto_ptr<t_averageVals>(new t_averageVals());
				bFirstStatistic= false;
			}
			tmStartingTime= itValue->tmStart;
			if(	!m_bExactStopSort &&
				!m_bEstimateTimeSort		)
			{
				cout << "new STARTING " << tmStartingTime.toString(/*as date*/true) << endl;
				cout << "----------------------------------------" << endl;
			}
		}
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
			out << "      for folder:subroutine " << getFolderSubroutine() << endl;
			out << "      " << getPolicyString(*itValue) << endl;
			out << "       want measure time of " << itValue->wanttime << " seconds" << endl;
			out << "   by ending calculation on " << fixed << itValue->tmFetch.toString(/*as date*/true) << endl;
			out.precision(0);
			out << "    fetch reachend value by " << itValue->reachpercent << "% CPU-time" << endl;
			if(itValue->runlength > 0)
			{
				out.precision(6);
				out << "            fetch runlength " << fixed << itValue->runlength;
				out.precision(0);
				out << " by " << dec << itValue->runpercent << "% CPU-time" << endl;
			}
		}
		if( (	m_bListAll ||
				m_bInformLate	) &&
			itValue->informlate > 0		)
		{
			out.precision(6);
			out << "       timer subroutine was " << fixed << itValue->informlate << " seconds informed "
							"after starting time measure" << endl;
			if(itValue->informlate >= itValue->wanttime)
				out << "               cannot reache exact time by late informing" << endl;
		}
		if(	m_bListAll &&
			itValue->startlate > 0	)
		{
			out << "        subroutine starting " << fixed << itValue->startlate << " seconds to late" << endl;
		}
		if(	m_bListAll ||
			m_bExactStop	)
		{
			if(	!m_bListAll &&
				(	m_bExactStopSort ||
					m_bEstimateTimeSort	)	)
			{
				out << "                ";
				out << nRunningCount << ". run ";
			}else
				out << "                       ";
			out << "need " << fixed << itValue->reachlate;
			out << " seconds after stopping TIMER subroutine" << endl;
		}
		if(	(	m_bListAll ||
				m_bReachend		) &&
			dLastReachend > 0.0000000009 &&
			itValue->wanttime > itValue->reachlate	)
		{
			if(	!m_bListAll &&
				!m_bExactStop &&
				!m_bEstimated &&
				(	m_bExactStopSort ||
					m_bEstimateTimeSort	)	)
			{
				out << "           ";
				out << " by " << nRunningCount << ". running ";
			}else
				out << "                      ";
			out << "minus ";
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
				out << "desired time was overrun for ";
				out << (itValue->reachlate - itValue->wanttime) << " seconds"<< endl;
				if(itValue == m_vReachendTimes.begin())
				{
					seconds-= itValue->wanttime;
					//itValue->wrongreach= seconds;
				}
			}
			out << "                ";
			if(	!m_bListAll &&
				(	m_bExactStopSort ||
					m_bEstimateTimeSort	)	)
			{
				if(!m_bExactStop)
					out << nRunningCount << ". run ";
				else
					out << "       ";
			}
			else
				out << "       ";
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
		if(m_bListAll)
			out << " define new reaching end by " << fixed << itValue->reachend << " seconds" << endl;
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
			cout << out.str();

		out.str("");
		if(!bFirstContent)
			calculateAverage(*itValue, pCurrentAverage.get());
		else
		{
			pCurrentAverage->nCount= 1;
			bFirstContent= false;
		}
	}// end of for(itValue= m_vReachendTimes.begin(); itValue != m_vReachendTimes.end(); ++itValue)

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
	}// end of if(	m_bExactStopSort || m_bEstimateTimeSort	)
	cout << endl;
	writeStatistic(bFirstStatistic, /*result*/bFirstStatistic, pCurrentAverage.get());
	if(!bFirstStatistic)
	{
		calculateAverage(pCurrentAverage.get(), allAverage);
		writeStatistic(bFirstStatistic, /*result*/true, &allAverage);
	}
}

short MinMaxTimes::countDigits(int value) const
{
	short nRv(1);

	while(value > 9)
	{
		++nRv;
		value/= 10;
	}
	return nRv;
}

void MinMaxTimes::writeStatistic(bool bfirst, bool bResult, const t_averageVals* values) const
{
	bool bNoOptionSet(false);
	ppi_value nMinLength, nMaxLength;
	ppi_value nMinEstimate, nMaxEstimate;

	if(	!m_bListAll &&
		!m_bInformLate &&
		!m_bExactStop &&
		!m_bEstimated &&
		!m_bReachend	)
	{// only sort option can be set
		bNoOptionSet= true;
	}

	nMinLength= getMinLength(*values);
	nMaxLength= getMaxLength(*values);
	nMinEstimate= getMinEstimate(*values);
	nMaxEstimate= getMaxEstimate(*values);
	if(bResult)
	{
		cout << "------------------------------------------------"
						"---------------------------------------------------" << endl;
		cout << "        is result by reading " << values->nCount << " entries";
	}else
	{
		cout << "        by reading " << values->nCount << " entries";
	}
	if(	values->nCount > 1 &&
		(	bfirst ||
			bResult		)		)
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
			if(values->nCount > 1)
				cout << "            and has an average of " << getAverageLength(*values) << " seconds" << endl;
		}
		if(	m_bListAll ||
			m_bEstimated ||
			bNoOptionSet	)
		{
			cout << "        wrong estimation differ from " << fixed << nMinEstimate << " to "
							<< fixed << nMaxEstimate << " seconds" << endl;
			cout << "            highest wrong estimation " << fixed << getLongestMiscalculated(*values) << " seconds" << endl;
			if(values->nCount > 1)
			{
				ppi_value average(getAverageEstimation(*values));

				if(average < 0)
					average*= -1;
				cout << "            and has an average of " << average << " seconds" << endl;
			}
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
			cout << "              wrong estimate " << fixed << getLongestMiscalculated(*values) << " seconds" << endl;
		}
	}
	cout << "        the highest minimal time of measured seconds will be " << (nMaxLength + getMaxInforming(*values)) << endl;
	if(nMinLength != nMaxLength)
	{
		cout << "                               which should be in most times "
					<< (getAverageLength(*values) + getAverageInforming(*values)) << " seconds" << endl;
	}
	if(values->nStopOverrun)
		cout << "        do not reach TIMER routine for exact stopping in << "
					<< values->nStopOverrun << " times" << endl;
	if(values->nOverrun)
		cout << "        overrun wanted measure time for " << values->nOverrun << " times" << endl;
	cout << endl;

/*	for later use
	if(m_tmEndingTime.isSet())
	{
		cout << endl;
		cout << "Server crashed on " << m_tmEndingTime.toString(/date/true) << endl;
		cout << "----------------------------------------------------------------------------------------------------------------------" << endl;
	}*/
}

