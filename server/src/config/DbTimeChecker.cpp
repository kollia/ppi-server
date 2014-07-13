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
#include "../database/logger/lib/logstructures.h"

#include "../pattern/util/LogHolderPattern.h"


using namespace ppi_database;

int DbTimeChecker::execute(const ICommandStructPattern* params, InterlacedProperties* properties)
{
	bool bDiffer;
	string stime;
	short nSort(0);
	vmmdifferDef readBlock;

	m_bListAll= params->hasOption("list");
	m_bStarting= params->hasOption("startingtime");
	if(	m_bListAll &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)[-s]," << endl;
		cout << "       and list all times (--list)[-l] are not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bInformLate= params->hasOption("inform");
	if(	m_bInformLate &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)[-s]" << endl;
		cout << "       and option --inform [-i] is not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bExactStop= params->hasOption("exactstop");
	if(	m_bExactStop &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)[-s]" << endl;
		cout << "       and option --exactstop [-E] is not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bEstimated= params->hasOption("estimated");
	if(	m_bEstimated &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)[-s]" << endl;
		cout << "       and option --estimated [-e] is not allowed" << endl;
		return EXIT_FAILURE;
	}
	m_bReachend= params->hasOption("reachend");
	if(	m_bReachend &&
		m_bStarting		)
	{
		cout << "ERROR: combination of starting times (--starting)[-s]" << endl;
		cout << "       and list only new defined reaching end (--reachend)[-r] is not allowed" << endl;
		return EXIT_FAILURE;
	}
	if(	(	m_bInformLate ||
			m_bExactStop ||
			m_bEstimated ||
			m_bReachend		) &&
		!m_bListAll					)
	{
		cout << "ERROR: option of ";
		if(m_bInformLate)
			cout << "--inform [-i] ";
		if(m_bExactStop)
			cout << "--exactstop [-E] ";
		if(m_bEstimated)
			cout << "--estimated [-e] ";
		if(m_bReachend)
			cout << "--reachend [-r] ";
		cout << endl;
		cout << "       is only allowed with option --list [-l]" << endl;
		return EXIT_FAILURE;
	}
	m_bFolderSort= !params->hasOption("idsort");
	m_bExactStopSort= params->hasOption("exacttimesort");
	if(m_bExactStopSort)
		++nSort;
	m_bEstimateTimeSort= params->hasOption("estimatetimesort");
	if(m_bEstimateTimeSort)
		++nSort;
	if(nSort > 1)
	{
		cerr << "ERROR: option of --exacttimesort [-T]" << endl;
		cerr << "             and --estimatetimesort [-t]" << endl;
		cout << "       can't be used in same time" << endl;
		return EXIT_FAILURE;
	}
	bDiffer= params->hasOption("difference");
	if(	bDiffer &&
		!m_bListAll	)
	{
		cerr << "ERROR: option --difference [-d] only usable with option --list [-l]" << endl;
		return EXIT_FAILURE;
	}
/*	if(	bDiffer &&
		nSort > 0	)
	{
		cerr << "ERROR: option --difference [-d] can't be used with any sorting option" << endl;
		cerr << "                                       --idsort [-I]" << endl;
		cerr << "                                       --exacttimesort [-T]" << endl;
		cerr << "                                       --estimatetimesort [-t]" << endl;
		return EXIT_FAILURE;
	}*/
	if(bDiffer)
	{
		string option("difference");
		float nDiffer;

		nDiffer= params->getOptionFloatContent(option);
		if(	option == "##ERROR" ||
			option == "##NULL"		)
		{
			cerr << "ERROR: last content for option --difference not be set" << endl;
			cerr << "       or can not read" << endl;
			return EXIT_FAILURE;
		}
		m_nDiffer= static_cast<unsigned short>(nDiffer);
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

	readBlock= readDatabase(properties);

	if(m_nDiffer > 0)
	{
		int start;
		size_t count;
		count= readBlock.size();
		start= static_cast<int>(count) - static_cast<int>(m_nDiffer);
		if(start < 0)
		{
			cerr << "ERROR: not enough starting times are available" << endl;
			return EXIT_FAILURE;
		}else
		{
			/*
			 * iterator array point to current inner map
			 * of map<'folder:subroutine', map<'folder ID', MinMaxTimes> >
			 * first strings can be converse
			 * when option --idsort be set
			 */
			mmdifferDef::iterator* pmmMinMaxFolders;
			/*
			 * iterator array point to current
			 * MinMaxTimes object of map<'folder ID', MinMaxTimes >
			 * when option --idsort be set
			 * first string will be 'folder:subroutine'
			 */
			mdifferDef::iterator* pmMinMaxFolders;
			set<string> vDefFolderIds;
			set<string> vDefIdFolders;
			MinMaxTimes nullObj;
			MinMaxTimes* last;
			ostringstream out;

			/*
			 * check first whether all objects
			 * has the same count of folders
			 */
			// read all defined folders
			for(size_t n= start; n < count; ++n)
			{
				for(mmdifferDef::iterator it= readBlock[n].begin();
								it != readBlock[n].end(); ++it)
				{
					vDefFolderIds.insert(it->first);
					for(mdifferDef::iterator it2= it->second.begin();
									it2 != it->second.end(); ++it2)
					{
						vDefIdFolders.insert(it2->first);
					}
				}
			}
			size_t ni, ci;

			ni= vDefIdFolders.size();
			out << "found follow folders:" << endl;
			for(set<string>::iterator it= vDefFolderIds.begin(); it != vDefFolderIds.end(); ++it)
			{
				string nullstr;
				size_t len(it->length() + 8);

				out << "      " << *it << " ";
				if(m_bFolderSort)
				{
					out << "with ";
					len+= 5;
				}else
				{
					out << "by ";
					len+= 3;
				}
				nullstr.append(len, ' ');
				ci= 1;
				for(set<string>::iterator it2= vDefIdFolders.begin(); it2 != vDefIdFolders.end(); ++it2)
				{
					out << *it2 << endl;
					if(ci < ni)
						out << nullstr;
					++ci;
				}
			}
			out << endl;
			LOG(LOG_INFO, out.str());
			out.str("");

			out << "found " << count << " difference objects" << endl;
			out << "      take " << (count - start) << " objects, from " << start << " to " << (count - 1) << endl;
			LOG(LOG_INFO, out.str());
			out.str("");
			// implement missing folders into time blocks
			LOG(LOG_DEBUG, "implement missing folders into time blocks:");
			for(size_t n= start; n < count; ++n)
			{
				mmdifferDef::iterator found1;
				mdifferDef::iterator found2;

				out << "    for " << (n - start + 1) << ". time block";
				LOG(LOG_DEBUG, out.str());
				out.str("");
				for(set<string>::iterator it= vDefFolderIds.begin(); it != vDefFolderIds.end(); ++it)
				{
					found1= readBlock[n].find(*it);
					if(found1 == readBlock[n].end())
					{
						readBlock[n][*it]= mdifferDef();
						out << "        implement new ";
					}else
						out << "        found ";
					out << "block for ";
					if(m_bFolderSort)
						out << "folder ";
					else
						out << "ID ";
					out << *it;
					LOG(LOG_DEBUG, out.str());
					out.str("");
					for(set<string>::iterator it2= vDefIdFolders.begin(); it2 != vDefIdFolders.end(); ++it2)
					{
						found2= readBlock[n][*it].find(*it2);
						if(found2 == readBlock[n][*it].end())
						{
							readBlock[n][*it][*it2]= nullObj;
							out << "            implement new ";
						}else
							out << "            found ";
						out << "block for ";
						if(!m_bFolderSort)
							out << "folder ";
						else
							out << "ID ";
						out << *it2;
						LOG(LOG_DEBUG, out.str());
						out.str("");
					}
				}
			}
			/*
			 * write than from all needed last object times
			 * always the first folder into an vector of iterators
			 */
			pmmMinMaxFolders= new mmdifferDef::iterator[readBlock[0].size()];
			pmMinMaxFolders= new mdifferDef::iterator[readBlock[0].begin()->second.size()];
			for(size_t n= start; n < count; ++n)
			{
				pmmMinMaxFolders[n - start]= readBlock[n].begin();
				pmMinMaxFolders[n - start]= pmmMinMaxFolders[n - start]->second.begin();
			}

			/*
			 * run thru all first objects from pmmMinMaxFolders->pmMinMaxFolders
			 * than second, third and so on
			 */
			while(!allEnd(start, readBlock, pmmMinMaxFolders))
			{
				last= NULL;
				for(size_t n= 0; n < m_nDiffer; ++n)
				{
					if(pmmMinMaxFolders[n] != readBlock[n + start].end())
					{
						out << "read from first " << pmmMinMaxFolders[n]->first;
						LOG(LOG_DEBUG, out.str());
						out.str("");
						if(pmMinMaxFolders[n] != pmmMinMaxFolders[n]->second.end())
						{
							if(pmMinMaxFolders[n]->second.isSet())
							{
								out << "    read second " << pmMinMaxFolders[n]->first;
								LOG(LOG_DEBUG, out.str());
								out.str("");
								pmMinMaxFolders[n]->second.writeDifference(static_cast<long>(n + 1), last);
								pmMinMaxFolders[n]->second.listEntries();
								last= &(pmMinMaxFolders[n]->second);
							}
							++pmMinMaxFolders[n];
						}
					}
				}
				if(allEnd(pmmMinMaxFolders, pmMinMaxFolders))
				{
					last= NULL;
					out << endl;
					LOG(LOG_DEBUG, out.str());
					out.str("");
					for(size_t i= 0; i < m_nDiffer; ++i)
					{
						++pmmMinMaxFolders[i];
						if(pmmMinMaxFolders[i] != readBlock[i + start].end())
							pmMinMaxFolders[i]= pmmMinMaxFolders[i]->second.begin();
					}
				}
			}
			delete[] pmmMinMaxFolders;
			delete[] pmMinMaxFolders;
		}
	}else //if(m_nDiffer > 0)
	{
		bool bStart;
		bool bOut;
		long nBlock(1);

		cout << endl << endl;
		for(vmmdifferDef::iterator vmm_it= readBlock.begin(); vmm_it != readBlock.end(); ++vmm_it)
		{
			bStart= true;
			bOut= false;
			for(mmdifferDef::iterator mm_it= vmm_it->begin(); mm_it != vmm_it->end(); ++mm_it)
			{
				bOut= false;
				for(mdifferDef::iterator m_it= mm_it->second.begin(); m_it != mm_it->second.end(); ++m_it)
				{
					bOut= true;
					if(bStart)
					{
						m_it->second.writeStarting();
						bStart= false;
					}
					m_it->second.writeDifference(nBlock, NULL);
					m_it->second.listEntries();
				}
			}
			if(bOut)
			{
				MinMaxTimes *last;

				last= &vmm_it->rbegin()->second.rbegin()->second;
				last->writeEnding();
			}
			++nBlock;
		}
	}
	return EXIT_SUCCESS;
}

DbTimeChecker::vmmdifferDef DbTimeChecker::readDatabase(InterlacedProperties* properties)
{
	/**
	 * read sorted blocks of MaxMinTimes object
	 */
	vmmdifferDef vmmRv;
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
	/**
	 * whether server started with option --timerdblog
	 * by current reading database entries
	 */
	bool bTimeDbLog(false);
	/**
	 * server do running
	 */
	bool brun(false);
	/**
	 * when server was new starting
	 * and no ending entry in database
	 * starting:starting -1 was defined
	 * server before was crashed
	 */
	bool bCrashed(false);
	/**
	 * last defined database
	 */
	IPPIDatabasePattern* db;
	vector<vector<db_t> > dbVector;
	ppi_time tmStartReading;
	ppi_time tmStartingTime;
	ppi_time tmFetchReachend;
	ppi_time tmLastTime;
	ostringstream startstopOut;
	map<string, t_runlength > mSetRunlength;
	map<string, t_runlength >::iterator itRunlength;

	vmmRv.push_back(mmdifferDef());
	db= DatabaseFactory::getChoosenDatabase(properties, NULL);
	db->readAllContent();
	//db->readInsideSubroutine("ppi-server:starting", -1, 0, 0);
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
						tmStartingTime= line->tm;
						string sStart(tmStartingTime.toString(true));

						if(!tmStartReading.isSet())
							tmStartReading= tmStartingTime;
						if(brun)
						{
							brun= false;
							bCrashed= true;
							writeEnding(&vmmRv, tmLastTime, /*crashed*/true);
						}
						bTimeDbLog= false;

					}else if(nValue == 1)
					{
						struct tm ttime;
						ostringstream timemsg;
						string sStart(tmStartingTime.toString(true));

						bInitialing= false;
						if(bNewBegin)
						{
							bNewBegin= false;
							mSetRunlength.clear();
						}
						if(brun)
						{
							LOG(LOG_WARNING, "found starting time 1 without first initiallining starting (-1)");
							writeEnding(&vmmRv, tmLastTime, /*crashed*/true);
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
						string sRunID;
						t_reachend tReachend;
						map<string, double>::iterator itP;

						m_vUsePolicy.insert(line->folder);
						if(line->identif.length() > 8)
						{
							string::size_type nLen;
							ostringstream out;

							sRunID= line->identif.substr(8);
							tReachend.ID= sRunID;
							// calculte first how much caracter has
							// count of folder
							nLen= sRunID.length();
							out << nLen;
							nLen= out.str().length();
							// create folder string
							out.str("");
							out << countFolders(sRunID) << "-" << sRunID;
							sRunID= string("").append(nLen-1, ' ') + out.str();
						}else
							sRunID= "-";
						itP= m_mPolicy.find(line->folder);
						if(itP != m_mPolicy.end())
							tReachend.policy= static_cast<int>(itP->second);
						itP= m_mPriority.find(line->folder);
						if(itP != m_mPriority.end())
							tReachend.priority= static_cast<int>(itP->second);
						tReachend.timerstat= m_mmTimerStat[line->folder][line->subroutine];
						tReachend.timeDbLog= bTimeDbLog;
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
						LOG(LOG_INFO, "read " + line->folder + ":" + line->subroutine +
										" with ID " + sRunID + "\n  use " +
										MinMaxTimes::getPolicyString(tReachend.policy, tReachend.priority)	);

						bool bFound(true);
						mmdifferDef::iterator found1;
						mdifferDef::iterator found2;
						mmdifferDef* block;
						string sSearch1(line->folder + ":" + line->subroutine);
						string sSearch2(sRunID);

						if(!m_bFolderSort)
						{
							sSearch2= sSearch1;
							sSearch1= sRunID;
						}
						block= &vmmRv.back();
						found1= block->find(sSearch1);
						if(found1 != block->end())
						{
							found2= found1->second.find(sSearch2);
							if(found2 != found1->second.end())
								found2->second.setTimes(tReachend);
							else
								bFound= false;
						}else
							bFound= false;
						if(!bFound)
						{
							MinMaxTimes newObj;

							newObj.setFolderSubroutine(line->folder + ":" + line->subroutine);
							newObj.setStarting(tmStartingTime);
							newObj.setOptions(m_bListAll, m_bInformLate, m_bExactStop,
												m_bEstimated, m_bReachend, m_bFolderSort,
												m_bExactStopSort, m_bEstimateTimeSort);
							newObj.setTimes(tReachend);
							(*block)[sSearch1][sSearch2]= newObj;
						}
						pLastReachend->informlate= 0;
						pLastReachend->startlate= 0;

					}// if(line->identif.substr(0, 8) == "reachend")

				}else // if(brun)
					if(bInitialing)
					{
						if(line->identif == "policy")
						{
							//cout << "found " << MinMaxTimes::getPolicyString((int)nValue, 1) << " for folder " << line->folder << " " << line->subroutine << endl;
							m_mPolicy[line->subroutine]= nValue;

						}else if(line->identif == "priority")
						{
							m_mPriority[line->subroutine]= nValue;

						}else if(line->identif == "timerstat")
						{
							m_mmTimerStat[line->folder][line->subroutine]= static_cast<short>(nValue);

						}else if(line->identif.substr(0, 8) == "reachend")
						{
							if(nValue == 0)
							{
								mmdifferDef* block;

								bNewBegin= true;
								block= &vmmRv.back();
								if(!block->empty())
								{
									writeEnding(&vmmRv, tmLastTime, bCrashed);
									vmmRv.push_back(mmdifferDef());
									/*
									 * remove used policies
									 * when server starting again
									 * for new initialization
									 */
									m_vUsePolicy.clear();
								}
							}
						}else if(	line->identif == "wanttime" ||
									line->identif == "runpercent" ||
									line->identif == "reachpercent" ||
									line->identif == "reachlate" ||
									line->identif == "wrongreach" ||
									line->identif == "informlate" ||
									line->identif == "startlate"		)
						{
							bTimeDbLog= true;
						}
					}// end else if(bInitialing)
			}// end if(bFoundValue)
			tmLastTime= line->tm;
		}
	}
	writeEnding(&vmmRv, tmLastTime, bCrashed);

	if(brun)
	{
		startstopOut << "Server crashed or last reading time on " << tmLastTime.toString(/*date*/true) << endl;
		startstopOut << "----------------------------------------------------------------------------------------------------------------------" << endl;
	}
	if(m_bStarting)
	{
		cout << endl << endl;
		cout << startstopOut.str();
	}
	return vmmRv;
}

bool DbTimeChecker::allEnd(int first, vmmdifferDef& readBlock, mmdifferDef::iterator* pMinMaxFolders)
{
	for(size_t n= 0; n < m_nDiffer; ++n)
	{
		if(pMinMaxFolders[n] != readBlock[n + first].end())
			return false;
	}
	return true;
}

bool DbTimeChecker::allEnd(mmdifferDef::iterator* pmmMinMaxFolders, mdifferDef::iterator* pmMinMaxFolders)
{
	for(size_t n= 0; n < m_nDiffer; ++n)
	{
		if(pmMinMaxFolders[n] != pmmMinMaxFolders[n]->second.end())
			return false;
	}
	return true;
}

void DbTimeChecker::writeEnding(vmmdifferDef* timeBlock, const ppi_time& time, bool bCrashed)
{
	mmdifferDef* block;

	if(timeBlock->empty())
		return;
	block= &timeBlock->back();
	for(mmdifferDef::iterator oit= block->begin(); oit != block->end(); ++oit)
	{
		for(mdifferDef::iterator iit= oit->second.begin(); iit != oit->second.end(); ++iit)
		{
			iit->second.setPolicy(m_mPolicy, m_mPriority, m_vUsePolicy);
			iit->second.setEndingTime(time, bCrashed);
		}
	}
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

t_reachend* DbTimeChecker::getLastReachendValues(const string& folder,
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



