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
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "owserver.h"
#include "OwServerQuestions.h"

#include "../util/exception.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

#include "../server/libs/client/SocketClientConnection.h"

using namespace server;
using namespace ppi_database;

namespace server
{
	OWServer::OWServer(const unsigned short ID, const string& type, IChipAccessPattern* accessPattern)
	:	Thread("OwfsServer", /*wait for init*/false),
		m_nServerID(ID),
		m_sServerType(type),
		m_bConnected(false),
		m_bAllInitial(false),
		m_poChipAccess(auto_ptr<IChipAccessPattern>(accessPattern)),
		m_bKernel(false),
		m_bKernelOnly(true),
		m_bPollRead(false)
	{
		m_WRITEONCHIP= getMutex("WRITEONCHIP");
		m_WRITECACHE= getMutex("READCACHE");
		m_PRIORITYCACHECOND= getCondition("PRIORITYCACHECOND");
		m_READCACHE= getMutex("READCACHE");
		m_PRIORITYCACHE= getMutex("PRIORITYCACHE");
		m_PRIORITY1CHIP= getMutex("PRIORITY1CHIP");
		m_CACHEWRITEENTRYS= getMutex("CACHEWRITEENTRYS");
		m_DEBUGINFO= getMutex("DEBUGINFO");
		m_EXECUTEMUTEX= getMutex("EXECUTEMUTEX");
		m_pKernelModule= auto_ptr<KernelModule>(new KernelModule(type, accessPattern, m_READCACHE, m_PRIORITYCACHECOND));
	}

	bool OWServer::readFirstChipState(bool out)
	{
		bool bCon;
		short res;
		double value;

		bCon= m_poChipAccess->isConnected();
		if(m_mvReadingCache.size() > 0)
		{
			if(out)
				cout << "read all defined input for first state from " << m_poChipAccess->getServerDescription() << " ..." << endl;
			for(map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > >::iterator it= m_mvReadingCache.begin(); it != m_mvReadingCache.end(); ++it)
			{
				if(out)
					cout << "   sequence for cache " << dec << it->first << " seconds" << endl;
				for(vector<SHAREDPTR::shared_ptr<chip_types_t> >::iterator chip= it->second.begin(); chip != it->second.end(); ++chip)
				{
					if(out)
						cout << "      " << (*chip)->id;
					if(bCon == true)
					{
						if(out)
							cout << " has value " << flush;
						value= 0;
						try{
							do{
								res= m_poChipAccess->read((*chip)->id, value);
							}while(res == 1);
						}catch(SignalException &ex)
						{
							string err;

							ex.addMessage("reading first state of chip '" + (*chip)->id + "'");
							err= ex.getTraceString();
							cout << endl << err << endl;
							LOG(LOG_ERROR, err);
							res= -1;
						}
						if(res < 0)
						{
							(*chip)->device= false;
							if(out)
								cout << "(cannot read correctly)" << endl;
						}else
						{
							(*chip)->device= true;
							(*chip)->value= value;
							if(out)
								cout << dec << value << endl;
						}
					}else
						cerr << " (owreader has no connection to any chip)" << endl;
				}
			}
		}
		if(!bCon)
			return false;
		return true;
	}

	void OWServer::endOfInitialisation(bool out)
	{
		readFirstChipState(out);
		if(m_bKernel == true)
		{
			if(m_bKernelOnly == false)
				m_pKernelModule->start();
		}else
		{
			m_bKernelOnly= false; // do not need kernel module
			m_pKernelModule= auto_ptr<KernelModule>();
		}
		m_bAllInitial= true;
	}

	bool OWServer::isServer(const string& type, const string& chipID)
	{
		if(chipID == "")
		{
			if(type == m_sServerType)
				return true;
			return false;
		}
		try{
			if(m_poChipAccess->existID(type, chipID))
				return true;

		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("asking external chip reader for chip " + chipID);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
		}
		return false;
	}

	EHObj OWServer::init(void* arg)
	{
		string threadName("owreader[");
		string defaultConfig;

		try{
			defaultConfig= m_poChipAccess->getDefaultFileName();
			m_oServerProperties= static_cast<IPropertyPattern*>(arg);
			if(defaultConfig != "")
				DbInterface::instance()->define(m_sServerType, defaultConfig);
			if(!m_poChipAccess->init(m_oServerProperties))
			{
				m_pError->setError("owserver", "init_chip", m_sServerType);
				return m_pError;
			}
			threadName+= m_sServerType + "]";
			LogHolderPattern::instance()->setThreadName(threadName);
			if(!m_poChipAccess->isConnected())
			{
				string msg(" connection to device was failed, try to connect all seconds");

				LOG(LOG_INFO, msg);
				cout << "### WARNING: " << msg << endl;
			}else
				m_bConnected= true;

		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("initialing external chip reader " +  m_sServerType);
			m_pError->setError("owserver", "init_exception", m_sServerType);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ALERT, err);
			return m_pError;
		}
		return m_pError;
	}

	bool OWServer::reachAllChips()
	{
		return m_poChipAccess->reachAllChips();
	}

	short OWServer::useChip(IActionPropertyMsgPattern* properties, string& unique, const string& folder, const string& subroutine)
	{
		bool read= false;
		bool write;
		bool cacheWrite;
		//bool writecacheWrite;
		short res;
		unsigned short kernelmode= 0;
		unsigned int priority;
		double dCacheSeq;
		string prop;
		SHAREDPTR::shared_ptr<chip_types_t> pchip;
		device_debug_t tdebug;
		DbInterface *reader= NULL;

		prop= "priority";
		priority= properties->getInt(prop, /*warning*/false);
		cacheWrite= properties->haveAction("cache");
		//writecacheWrite= properties->haveAction("writecache");
		prop= "cache";
		dCacheSeq= properties->getDouble(prop, /*warning*/false);
		read= properties->haveAction("read");
		write= properties->haveAction("write");
		if(read && write)
		{
			string msg(properties->getMsgHead(true));

			msg+= "actions read and write cannot used both in same subroutine";
			LOG(LOG_ERROR, msg);
			cerr << msg << endl;
			return 0;
		}
		if(	write
			&&
			dCacheSeq	)
		{
			string msg(properties->getMsgHead(false));

			msg+= "cache as parameter for sequence is only for reading";
			cout << msg << endl;
			LOG(LOG_WARNING, msg);
			dCacheSeq= 0;
		}

		try{
			res= m_poChipAccess->useChip(properties, unique, kernelmode);
			if(m_poChipAccess->getDefaultFileName() == "")
			{// chip isn't registered,
			 // do it now
				bool fl;
				bool *pfl= &fl;
				double min, max;
				double *pmin= &min;
				double *pmax= &max;
				string chip, type, pin, family;

				range(unique, min, max, fl);
				chip= properties->getValue("ID");
				pin= properties->getValue("pin");
				type= m_poChipAccess->getChipType(pin);
				reader= DbInterface::instance();
				reader->registerChip(m_sServerType, /*chip*/unique, pin, type, "unknown", pmin, pmax, pfl);
			}
			if(res == 0)
				return 0;
			else if(res == 1)
			{// chip is for reading
				read= true;
				write= false;
				//*pCache= currentReading;
				if(dCacheSeq == 0)
				{
					bool bExist;
					double defaultCache;

					reader= DbInterface::instance();
					defaultCache= reader->getRegisteredDefaultChipCache(m_sServerType, unique, bExist);
					if(	bExist
						&&
						defaultCache != 0	)
					{
						dCacheSeq= defaultCache;
					}else
					{
						bool fl;
						double min, max;

						range(unique, min, max, fl);
						dCacheSeq= reader->getDefaultCache(min, max, fl, folder, subroutine);
						if(dCacheSeq == 0)
							dCacheSeq= 15;
					}
				}
				priority= 0;
			}else if(res == 2)
			{// chip is for writing
				read= false;
				write= true;
				if(priority == 0)
					priority= 9999;
				dCacheSeq= 0;
				m_bKernelOnly= false;
			}else // res is 3
			{
				read= false;
				write= false;
			}


			pchip= SHAREDPTR::shared_ptr<chip_types_t>(new chip_types_t);
			pchip->id= unique;
			if(cacheWrite)
				pchip->writecache= true;
			else
				pchip->writecache= false;

			pchip->wcacheID= m_poChipAccess->getChipTypeID(unique);
			pchip->read= read;
			pchip->value= 0;
			pchip->priority= priority;
			pchip->timeSeq.tv_sec= 0;
			pchip->timeSeq.tv_usec= 0;
			pchip->device= true;
			m_mtConductors[unique]= pchip;
			if(kernelmode)
			{ // chip is for reading over kernel module
				vector<string>::iterator found;

				if(!read)
				{
					string msg(properties->getMsgHead(true));

					msg+= "application can only read from kernelmodule. Don't set an writing chip with kernelmode";
					cout << msg << endl;
					LOG(LOG_ERROR, msg);
					return 0;
				}
				m_pKernelModule->fillChipType(pchip.get());
				m_bKernel= true;
				if(kernelmode == 2)
					m_bKernelOnly= false;
			}
			if(	dCacheSeq )
			{
				long tm;
				map<double, seq_t>::iterator found;

				tm= (long)dCacheSeq;
				pchip->timeSeq.tv_sec= (time_t)tm;
				tm= (long)((dCacheSeq - tm) * 1000000);
				pchip->timeSeq.tv_usec= (suseconds_t)tm;
				if(kernelmode != 1)
				{
					pchip->bPoll= true;
					m_bPollRead= true;
					m_bKernelOnly= false;
					m_mvTrueReadingCache[dCacheSeq].push_back(pchip);
				}else
					pchip->bPoll= false;
				m_mvReadingCache[dCacheSeq].push_back(pchip);

				found= m_mStartSeq.find(dCacheSeq);
				if(found == m_mStartSeq.end())
				{
					seq_t t;

					t.tm.tv_sec= pchip->timeSeq.tv_sec;
					t.tm.tv_usec= pchip->timeSeq.tv_usec;
					t.nextUnique= pchip;
					t.bPoll= true;
					m_mStartSeq[dCacheSeq]= t;
				}

			}
			// if owreader not be connected to any board or chip
			// set device access to false;
			if(!m_bConnected)
			{
				reader= DbInterface::instance();
				reader->changedChip(m_sServerType, unique, /*value*/0, /*access*/false);
				pchip->device= false;
			}
		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("initialing chip " + unique + " inside " +  m_sServerType
							+ " reader\nfor subroutine " + subroutine + " in folder " + folder);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
			return 0;
		}catch(std::exception &ex)
		{
			string msg;

			msg= "by initialing chip " + unique + " inside " +  m_sServerType
					+ " reader\nfor subroutine " + subroutine + " in folder " + folder + "\n";
			msg+= "throwing std exception: " + *ex.what();
			cerr << endl << endl << "### ERROR: " << msg << endl << endl;
			LOG(LOG_ERROR, msg);
			return 0;
		}catch(...)
		{
			string msg;

			msg= "by initialing chip " + unique + " inside " +  m_sServerType
					+ " reader\nfor subroutine " + subroutine + " in folder " + folder + "\n";
			msg+= "throwing unknown exception";
			cerr << endl << endl << "### ERROR: " << msg << endl << endl;
			LOG(LOG_ERROR, msg);
			return 0;
		}

		// define debug info to make benchmark
		// for this OWServer with client
		// > DEBUG -ow <Nr>
		tdebug.id= m_debugInfo.size() + 1;
		tdebug.btime= true;
		tdebug.act_tm.tv_sec= 0;
		tdebug.act_tm.tv_usec= 0;
		tdebug.count= 0;
		tdebug.read= read;
		tdebug.ok= true;
		tdebug.device= unique;
		tdebug.utime= 0;
		tdebug.value= 0;
		tdebug.cache= dCacheSeq;
		tdebug.priority= priority;
		m_debugInfo.push_back(tdebug);
		return res;
	}

	void OWServer::checkUnused()
	{
		vector<string> unused;

		// do not display unused chips
		return;
		unused= m_poChipAccess->getUnusedIDs();
		if(!unused.empty())
		{
			cout << "    unused id's of chips:" << endl;
			for(vector<string>::iterator i= unused.begin(); i != unused.end(); ++i)
			{
				string msg("          ");
				cout << "          ";
				cout << (*i) << "  type:";
				cout << m_poChipAccess->getChipType(*i) << endl;
			}
		}
	}

	bool OWServer::hasUnusedIDs()
	{
		vector<string> unused;

		unused= m_poChipAccess->getUnusedIDs();
		if(unused.size())
			return true;
		return false;
	}

	string OWServer::getChipType(const string &ID)
	{
		return m_poChipAccess->getChipType(ID);
	}

	void OWServer::setDebug(const bool debug)
	{
		m_poChipAccess->setDebug(debug);
	}

	bool OWServer::isDebug()
	{
		return m_poChipAccess->isDebug();
	}

	bool OWServer::execute()
	{
		typedef map<string, map<string, string> >::iterator cachemmiter;
		typedef map<string, string>::iterator cachemiter;
		typedef map<string, SHAREDPTR::shared_ptr<chip_types_t> >::iterator mmtype;
		typedef map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > >::iterator mdvReadingIt;
		typedef vector<SHAREDPTR::shared_ptr<chip_types_t> >::iterator vReadingIt;

		if(!m_bAllInitial)
		{
			USLEEP(500000);
			return true;
		}
		if(!m_bConnected)
		{
			SLEEP(1);
			if(stopping())
				return false;
			try{
				if(m_poChipAccess->connect())
				{
					if(!m_poChipAccess->init(m_oServerProperties))
						return false;
					m_bConnected= true;
				}else
					return true;
			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("trying to connect to external chip access " +  m_sServerType);
				err= ex.getTraceString();
				cout << endl << err << endl;
				LOG(LOG_ERROR, err);
				return false;
			}

		}
		if(m_bKernelOnly)
			return m_pKernelModule->execute();

		short endWork;
		DbInterface* db;
		map<int, queue<SHAREDPTR::shared_ptr<chip_types_t> > >::iterator priorityPos;
		bool bDebug= false;
		bool bDo= false;
		bool bHasStarterSeq;
		mmtype chipTypeIt;

		if(isDebug())
		{
			bDebug= true;
			LOCK(m_DEBUGINFO);
		}
		// in first case look about write to chips with priority
		LOCK(m_PRIORITYCACHE);
		priorityPos= m_mvPriorityCache.begin();
		while(priorityPos != m_mvPriorityCache.end())
		{
			if(m_mvPriorityCache.size() == 0)
				break;
			endWork= 3;// nothing to do
			while(!priorityPos->second.empty())
			{
				SHAREDPTR::shared_ptr<chip_types_t> chip;
				vector<device_debug_t>::iterator devIt;

				chip= priorityPos->second.front();
				if(bDebug)
				{
					device_debug_t debug;

					//cout << "write in priority cache " << priorityPos->first << " on unique ID:" << chip->id;
					//cout << " value:" << chip->value << endl;
					debug.device= chip->id;
					devIt= find(m_debugInfo.begin(), m_debugInfo.end(), &debug);
					if(gettimeofday(&devIt->act_tm, NULL))
						devIt->btime= false;
					else
						devIt->btime= true;
					devIt->value= chip->value;
					bDebug= true;
				}
#ifdef __OWSERVERREADWRITE
				ostringstream scout;

				scout << "write on chip id " << chip->id << " value " << chip->value << endl;
				cout << scout.str();
#endif // __OWSERVERREADWRITE
				try{
					endWork= m_poChipAccess->write(chip->id, chip->value, chip->addinfo);

				}catch(SignalException& ex)
				{
					string err;

					ex.addMessage("writing on chip " +  chip->id + "inside reader " + m_sServerType);
					err= ex.getTraceString();
					cout << endl << err << endl;
					LOG(LOG_ERROR, err);
					endWork= -1;
				}
#ifdef __OWSERVERREADWRITE
				ostringstream scout3;
				scout3 << "-- writing done by state " << endWork << endl;
				cout << scout3.str();
#endif // __OWSERVERREADWRITE
				if(bDebug)
					measureTimeDiff(&(*devIt));
				if(	endWork == -1 ||
					endWork == -2	)
				{// an error is occured
					chipTypeIt= m_mtConductors.find(chip->id);
					if(chipTypeIt != m_mtConductors.end())
						chipTypeIt->second->device= false;
					if(bDebug)
						devIt->ok= false;
					//if(endWork == -1)
					//	break;// try again the next time
					// -2 - kill from queue
				}else
				{
					chipTypeIt= m_mtConductors.find(chip->id);
					if(chipTypeIt != m_mtConductors.end())
						chipTypeIt->second->device= true;
					if(endWork == 1)
					{// reading was correctly but the next time should make the same pin,
						if(bDebug)
							devIt->ok= true;
						bDo= true;
						break;
					}
				}
				priorityPos->second.pop();
				if(priorityPos->second.size() == 0)
					m_mvPriorityCache.erase(priorityPos);
				if(endWork == 0)
				{// writing was correctly and the pin is finished (go to the next)
					if(bDebug)
						devIt->ok= true;
					bDo= true;
					break;
				}
				// if endWork is 2
				// an entry was made but writing prime in an next time on an other pin
				// stay in the loop and write on the next pin
			}
			if(endWork < 2)
				break;
			++priorityPos;
		}
		UNLOCK(m_PRIORITYCACHE);

		// if there by priority cache nothing to do
		// read sequences in cache
		if(!bDo)
		{
			/**
			 * actual device in an sequence of different caching times
			 */
			vector<SHAREDPTR::shared_ptr<chip_types_t> >::iterator pActChip;
			/**
			 * actual sequence of caches with next device
			 */
			map<double, seq_t>::iterator pActSeq= m_mStartSeq.begin();
			/**
			 * actual time
			 */
			timeval tv;

			LOCK(m_READCACHE);
			if(	m_pKernelModule.get() &&
				m_pKernelModule->changeReadPoll(m_mvReadingCache, m_mvTrueReadingCache) == true	)
			{
				m_bPollRead= true;
				for(pActSeq= m_mStartSeq.begin(); pActSeq != m_mStartSeq.end(); ++pActSeq)
					pActSeq->second.bPoll= true;
				pActSeq= m_mStartSeq.begin();
			}
			if(m_bPollRead)
			{
				// read actual time to now in which row of sequence should be measure
				if(gettimeofday(&tv, NULL))
				{
					string msg("WARNING: cannot get time of day,\n");

					msg+= "         so do not read sequence inside defined cache.\n";
					msg+= "         In this case measuring only in sequence back to back";
					TIMELOG(LOG_WARNING, "gettimeofday", msg);
					cerr << msg << endl;
					m_bReadSeq= false;
				}else
				{
					/**
					 * whether find any device for reading in all sequences
					 */
					bool bSeqFoundDevice(false);
					/**
					 * whether find any device for reading in one sequence
					 */
					bool bFoundDevice;

					//cout << "seconds: " << tv.tv_sec;
					//cout << " mikrosec:" << tv.tv_usec;
					//cout << " ACTUAL"<< endl;
					m_bReadSeq= true;
					// calculate in which row of sequence be measured
					while(pActSeq != m_mStartSeq.end())
					{
						//cout << "seconds: " << pActSeq->second.tm.tv_sec;
						//cout << " mikrosec:" << pActSeq->second.tm.tv_usec;
						//cout << " for cache "<< pActSeq->first << endl;
						if(pActSeq->second.bPoll)
						{
							if(	tv.tv_sec > pActSeq->second.tm.tv_sec
								||
								(	tv.tv_sec == pActSeq->second.tm.tv_sec
									&&
									tv.tv_usec > pActSeq->second.tm.tv_usec	)	)
							{
								// search for first reading device
								if(pActSeq->second.nextUnique.get() != NULL)
								{
									bFoundDevice= false;
									pActChip= find(m_mvTrueReadingCache[pActSeq->first].begin(), m_mvTrueReadingCache[pActSeq->first].end(),
													pActSeq->second.nextUnique);
									if(pActChip == m_mvTrueReadingCache[pActSeq->first].end())
									{
										pActChip= m_mvTrueReadingCache[pActSeq->first].begin();

									}else if(pActChip != m_mvTrueReadingCache[pActSeq->first].begin())
									{	// maybe one device before was OK
										// only if began from begin know whether device found
										bFoundDevice= true;
										bSeqFoundDevice= true;
									}
								}else
								{
									bFoundDevice= false;
									pActChip= m_mvTrueReadingCache[pActSeq->first].begin();
								}
								if(pActChip != m_mvTrueReadingCache[pActSeq->first].end())
								{
									pActSeq->second.nextUnique= *pActChip;
									//cout << "read chip for cache " << pActSeq->first << endl;
									break;
								}
								pActSeq->second.bPoll= bFoundDevice;

							}else	// sequence is defined for polling
							{		// but not in this time
									// maybe there is an device for reading
								bSeqFoundDevice= true;
							}
						}
						++pActSeq;
					}
					if(	!bSeqFoundDevice &&
						pActSeq == m_mStartSeq.end())
					{ // has not found any device for reading
						m_bPollRead= false;
					}//else
					//{
						//cout << "seconds: " << pActSeq->second.tm.tv_sec;
						//cout << " mikrosec:" << pActSeq->second.tm.tv_usec;
						//cout << " for next cache "<< pActSeq->first << endl;
						//cout << "------------------------------------------------" << endl;
					//}
				}
				while(pActSeq != m_mStartSeq.end())
				{
					pActChip= find(m_mvTrueReadingCache[pActSeq->first].begin(), m_mvTrueReadingCache[pActSeq->first].end(),
									pActSeq->second.nextUnique);
					while(pActChip != m_mvTrueReadingCache[pActSeq->first].end())
					{
						bool device;
						double value;
						vector<device_debug_t>::iterator devIt;

						if(bDebug)
						{
							device_debug_t debug;

							//cout << " for unique ID:" << (*pActChip)->id << endl;
							debug.device= (*pActChip)->id;
							devIt= find(m_debugInfo.begin(), m_debugInfo.end(), &debug);
							if(devIt == m_debugInfo.end())
							{
								debug.id= m_debugInfo.size() + 1;
								debug.read= true;
								debug.device= (*pActChip)->id;
								debug.count= 0;
								debug.cache= pActSeq->first;
								m_debugInfo.push_back(debug);
								devIt= find(m_debugInfo.begin(), m_debugInfo.end(), &debug);
							}
							if(gettimeofday(&devIt->act_tm, NULL))
								devIt->btime= false;
							else
								devIt->btime= true;
							bDebug= true;
						}
						value= 0;//(*pActChip)->value;
#ifdef __OWSERVERREADWRITE
						ostringstream scout;
						scout << "read external value on " << (*pActChip)->id << endl;
						cout << scout.str();
#endif // __OWSERVERREADWRITE
						UNLOCK(m_READCACHE);
						try{
							endWork= m_poChipAccess->read((*pActChip)->id, value);
							//cout << "server read from id " << ID << " value " << dec << value << " where value before was " << (*pActChip)->value << endl;
						}catch(SignalException& ex)
						{
							string err;

							ex.addMessage("read on chip " +  (*pActChip)->id + "inside reader " + m_sServerType);
							err= ex.getTraceString();
							cout << endl << err << endl;
							LOG(LOG_ERROR, err);
							endWork= -1;
						}
						LOCK(m_READCACHE);
#ifdef __OWSERVERREADWRITE
						ostringstream scout2;
						scout2 << "-- reading value " << value << " with state " << endWork << endl;
						cout << scout2.str();
#endif // __OWSERVERREADWRITE
						switch (endWork)
						{
						case -1:
							// an error occured
							device= false;
							if(bDebug)
							{
								devIt->ok= false;
								devIt->value= 0;
								measureTimeDiff(&(*devIt));
							}
							bDo= true;
							break;
						case 4:
							// reading was correctly and the pin is finished
							// in next time by polling this device shouldn't read (remove later from TrueReadingCache)
							(*pActChip)->bPoll= false;
						case 0:
							// reading was correctly and the pin is finished
							//cout << "server write id " << (*pActChip)->id << " to value " << dec << value << endl;
							device= true;
							if(bDebug)
							{
								devIt->value= value;
								devIt->ok= true;
								measureTimeDiff(&(*devIt));
							}
							bDo= true;
							break;
						case 1:
							// reading was also correctly but the next time should make the same pin (value is not the last state),
							device= true;
							if(bDebug)
								devIt->ok= true;
							bDo= true;
							break;
						case 3:
							// nothing was to do, value set,
							// was reading before (chip wasn't read,
							// value is correct) -> go to the next pin)
							device= true;
							if(bDebug)
							{
								devIt->value= value;
								devIt->ok= true;
								measureTimeDiff(&(*devIt));
							}
							break;
						default:
							// unknown result
							if(bDebug)
								devIt->ok= false;
							device= false;
							break;
						}
						if(	(*pActChip)->device != device
							||
							(*pActChip)->value != value		)
						{
							db= DbInterface::instance();
							db->changedChip(m_sServerType, (*pActChip)->id, value, device);
							(*pActChip)->device= device;
							(*pActChip)->value= value;
						}
						if(endWork == 1)
							break;
						if(endWork == 4)
							pActChip= m_mvTrueReadingCache[pActSeq->first].erase(pActChip);
						else
							++pActChip;
						while(pActChip != m_mvTrueReadingCache[pActSeq->first].end())
						{ // define next chip to read
							if((*pActChip)->bPoll == true)
								break;
							++pActChip;
						}
						if((pActChip == m_mvTrueReadingCache[pActSeq->first].end()))
							pActSeq->second.nextUnique= SHAREDPTR::shared_ptr<chip_types_t>();
						else
							pActSeq->second.nextUnique= *pActChip;
						if(bDo)
							break;
					}
					if(	pActSeq != m_mStartSeq.end()
						&&
						pActChip == m_mvTrueReadingCache[pActSeq->first].end())
					{
						try{
							m_poChipAccess->endOfCacheReading(pActSeq->first);
						}catch(SignalException& ex)
						{
							string err;

							ex.addMessage("sending endOfCacheReading to reader " + m_sServerType);
							err= ex.getTraceString();
							cout << endl << err << endl;
							LOG(LOG_ERROR, err);
						}
						pActSeq->second.nextUnique= SHAREDPTR::shared_ptr<chip_types_t>();
						if(m_mvTrueReadingCache[pActSeq->first].size() > 0)
						{// set next new time for reading
							if(gettimeofday(&tv, NULL))
							{
								string msg("WARNING: cannot get time of day,\n");

								msg+= "         so do not read sequence inside defined cache.\n";
								msg+= "         In this case measureing only in sequence back to back";
								TIMELOG(LOG_WARNING, "gettimeofday", msg);
								cout << msg << endl;
								m_bReadSeq= false;
							}else
							{
								timeval tmRepeat;
								SHAREDPTR::shared_ptr<chip_types_t> chip;

								//if(isDebug())
								//{
								/*	cout << "current seconds: " << tv.tv_sec;
									cout << " mikrosec:" << tv.tv_usec << endl;*/
								//}
								chip= *(--pActChip);
								tmRepeat= chip->timeSeq;
								tv.tv_sec+= tmRepeat.tv_sec;
								tv.tv_usec+= tmRepeat.tv_usec;
								while(tv.tv_usec > 999999)
								{
									tv.tv_sec+= 1;
									tv.tv_usec-= 1000000;
								}
								pActSeq->second.tm= tv;
								//if(isDebug())
								//{
								/*	cout << "set cache "<< pActSeq->first;
									cout << "  to seconds: " << pActSeq->second.tm.tv_sec;
									cout << " mikrosec:" << pActSeq->second.tm.tv_usec << endl;
									cout << "-------------------------------------------------------------------------------------------" << endl;
									*/
								//}
							}
						}// size of TrueReadingCache was more than 0
					}// end if(pActSeq->first != 9999)
					if(bDo)
						break;
				}// end while(pActSeq != m_mStartSeq.end())
			}// end if(m_bPollRead)
			UNLOCK(m_READCACHE);
		}// end if(!bDo)
		if(bDebug)
			UNLOCK(m_DEBUGINFO);
		if(!bDo)
		{// nothing was to do
		 // sleep to next measure
			timeval shorttm;
			timespec waittm;

			shorttm.tv_sec= -1;
			shorttm.tv_usec= -1;
			try{
				m_poChipAccess->endOfLoop();

			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("sending endOfLoop to reader " + m_sServerType);
				err= ex.getTraceString();
				cout << endl << err << endl;
				LOG(LOG_ERROR, err);
			}
			LOCK(m_READCACHE);
			if(m_mStartSeq.empty())
			{
				if(gettimeofday(&shorttm, NULL))
				{
					string msg("WARNING: cannot get time of day,\n");

					msg+= "         so do not read sequence inside defined cache.\n";
					msg+= "         In this case measureing only in sequence back to back";
					TIMELOG(LOG_WARNING, "gettimeofday", msg);
					cout << msg << endl;
					m_bReadSeq= false;
				}
				shorttm.tv_sec+= 1;
			}else
			{
				for(map<double, seq_t>::iterator it= m_mStartSeq.begin(); it != m_mStartSeq.end(); ++it)
				{
					if(	it->second.tm.tv_sec < shorttm.tv_sec
						||
						shorttm.tv_sec == -1
						||
						(	it->second.tm.tv_sec == shorttm.tv_sec
							&&
							it->second.tm.tv_usec < shorttm.tv_usec	)	)
					{
						shorttm.tv_sec= it->second.tm.tv_sec;
						shorttm.tv_usec= it->second.tm.tv_usec;
					}
				}
			}
			if(m_mStartSeq.size() == 0)
				bHasStarterSeq= false;
			else
				bHasStarterSeq= true;
			UNLOCK(m_READCACHE);
			waittm.tv_sec= shorttm.tv_sec;
			waittm.tv_nsec= shorttm.tv_usec * 1000;
			LOCK(m_PRIORITYCACHE);
			if(m_mvPriorityCache.size() == 0)
			{
				if(	bHasStarterSeq &&
					m_bPollRead			)
				{
					TIMECONDITION(m_PRIORITYCACHECOND, m_PRIORITYCACHE, &waittm);

				}else
					CONDITION(m_PRIORITYCACHECOND, m_PRIORITYCACHE);
			}
			UNLOCK( m_PRIORITYCACHE);
		}
		return true;
	}

	vector<string> OWServer::getDebugInfo()
	{
		//char buf[10];
		//long nsec, nmsec, nusec;
		//string sDo;
		vector<device_debug_t> debVect;
		vector<string> vRv;

		SLEEP(1);
#ifdef __OWSERVERREADWRITE
		ostringstream scout;
		scout << "~~~ want read debug info  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		cout << scout.str();
#endif // __OWSERVERREADWRITE
		LOCK(m_DEBUGINFO);
		debVect= m_debugInfo;
		for(vector<device_debug_t>::iterator o= m_debugInfo.begin(); o != m_debugInfo.end(); ++o)
		{
			o->count= 0;
			o->utime= 0;
		}
		UNLOCK(m_DEBUGINFO);
		for(vector<device_debug_t>::iterator o= debVect.begin(); o != debVect.end(); ++o)
		{
			ostringstream devString, dev1String;

			dev1String << dec;
			dev1String << o->id << " ";
			dev1String << o->btime << " ";
			dev1String << o->act_tm.tv_sec << " ";
			dev1String << o->act_tm.tv_usec << " ";
			dev1String << o->count << " ";
			dev1String << o->read << " ";
			dev1String << o->ok << " ";
			dev1String << o->utime << " ";
			dev1String << o->value << " ";
			dev1String << o->cache << " ";
			dev1String << o->priority << " ";
			dev1String << o->device;
			vRv.push_back(dev1String.str());
		}
#ifdef __OWSERVERREADWRITE
		ostringstream scout2;
		scout2 << "~~~ return debug info  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		cout << scout2.str();
#endif // __OWSERVERREADWRITE
		return vRv;
	}

	vector<string> OWServer::getChipIDs()
	{
		vector<string> vRv;

		try{
			vRv= m_poChipAccess->getChipIDs();
		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("asking for exist chip id's on reader " + m_sServerType);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
		}
		return vRv;
	}

	void OWServer::measureTimeDiff(device_debug_t* device) const
	{
		long sec, utime;
		timeval tm;

		if(	device->btime
			&&
			gettimeofday(&tm, NULL) == 0	)
		{
			if(tm.tv_sec > device->act_tm.tv_sec)
			{
				utime= 1000000 - device->act_tm.tv_usec;
				sec= device->act_tm.tv_sec + 1;
				utime+= (tm.tv_sec - sec) * 1000000;
				utime+= tm.tv_usec;
			}else
				utime= tm.tv_usec - device->act_tm.tv_usec;
			if(device->utime < utime)
				device->utime= utime;
		}else
			device->btime= false;
		++device->count;
	}

	bool OWServer::write(const string& id, const double value, const string& addinfo)
	{
	//	bool write= false;
		SHAREDPTR::shared_ptr<chip_types_t> chip;
		vector<string>::iterator inIt;
		map<string, SHAREDPTR::shared_ptr<chip_types_t> >::iterator chipIt;

		chipIt= m_mtConductors.find(id);
		if(chipIt == m_mtConductors.end())
			return false;
		chip= chipIt->second;

		// 2010/08/03 ppi@magnificat.at
		//				write always to server
		//				because maybe only by writing one
		//				server do output (like LIRC)
		//				and if set subroutine the second time one
		//				server should not ignore new output
		//			same for comment's before, not important
		/*if(	chip->value != value
			||
			write == true		)*/
		{
			if(chip->writecache)
			{
				LOCK(m_CACHEWRITEENTRYS);
				inIt= find(m_vChipTypeIDs.begin(), m_vChipTypeIDs.end(), chip->wcacheID);
				if(inIt == m_vChipTypeIDs.end())
					m_vChipTypeIDs.push_back(chip->wcacheID);
				UNLOCK(m_CACHEWRITEENTRYS);
			}
			LOCK(m_PRIORITYCACHE);
#ifdef SERVERTIMELOG
			timeval stltv;
			string msg;
			char stlbuf[20];
			ostringstream ovalue;

			if(gettimeofday(&stltv, NULL))
			{
				string msg("WARNING: cannot get time of day,\n");

				msg+= "         so do not read sequence inside defined cache.\n";
				msg+= "         In this case measureing only in sequence back to back";
				TIMELOG(LOG_WARNING, "gettimeofday", msg);
			}else
			{
				tm l;

				ovalue << value;
				msg= "order on " + id;
				msg+= " to write value ";
				msg+= ovalue.str() + "\n";
				msg+= "\n";
				if(localtime_r(&stltv.tv_sec, &l) == NULL)
					TIMELOG(LOG_ERROR, "localtime_r", "cannot create correct localtime");
				strftime(stlbuf, 15, "%H:%M:%S", &l);
				msg+= stlbuf;
				msg+= " mikrosec:";
				snprintf(stlbuf, 15, "%ld", stltv.tv_usec);
				msg+= stlbuf;
				LOG(LOG_INFO, msg);
			}
#endif // SERVERTIMELOG
			chip->value= value;
			chip->addinfo= addinfo;
			m_mvPriorityCache[chip->priority].push(chip);
			AROUSE(m_PRIORITYCACHECOND);
			UNLOCK(m_PRIORITYCACHE);
		}
		if(!chip->device)
			return false;
		return true;
	}

	void OWServer::range(const string& pin, double& min, double& max, bool &bfloat)
	{
		chiprange_t chip;
		map<string, chiprange_t>::iterator found;

		found= m_mRange.find(pin);
		if(found != m_mRange.end())
		{
			min= found->second.min;
			max= found->second.max;
			bfloat= found->second.bfloat;
		}else
		{
			try{
				m_poChipAccess->range(pin, min, max, bfloat);

			}catch(SignalException& ex)
			{
				string err;

				ex.addMessage("trying to get range from  external chip with server type " +  m_sServerType);
				err= ex.getTraceString();
				cout << endl << err << endl;
				LOG(LOG_ERROR, err);
				min= 1;
				max= 0;
				bfloat= true;
			}
			chip.min= min;
			chip.max= max;
			chip.bfloat= bfloat;
			m_mRange[pin]= chip;
		}
	}

	bool OWServer::read(const string& id, double* value)
	{
		bool bfloat, bCorrect;
		double min, max;
		map<string, SHAREDPTR::shared_ptr<chip_types_t> >::iterator chipIt;

		chipIt= m_mtConductors.find(id);
		range(chipIt->second->id, min, max, bfloat);
		LOCK(m_READCACHE);
		*value= chipIt->second->value;
		bCorrect= chipIt->second->device;
		//cout << "get for " << id << " " << boolalpha << bCorrect << " value " << *value << endl;
		if(	min == 0
			&&
			max == 1
			&&
			!bfloat	)
		{// set access bit (0010) to 0
			int bits= (int)chipIt->second->value;

			//cout << "between pin " << id << " is " << portBase::getBinString((long)bits, 2) << flush;
			bits&= 0x01;
			chipIt->second->value= (double)bits;
			//cout << " accessed " << portBase::getBinString((long)bits, 2) << flush;
			//cout << endl;
		}
		UNLOCK(m_READCACHE);
		//cout << "    with range min:" << min << ", max:" << max << " and floating is " << boolalpha << bfloat << " value is " << *value << endl;
		return bCorrect;
	}

	int OWServer::command_exec(const string& command, vector<string>& result, bool& more)
	{
		int nRv;

		LOCK(m_EXECUTEMUTEX);
		try{
			nRv= m_poChipAccess->command_exec(command, result, more);

		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("execute command on reader " + m_sServerType);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
			nRv= 127;
		}
		UNLOCK(m_EXECUTEMUTEX);
		return nRv;
	}

	void OWServer::ending()
	{
		try{
			if(m_poChipAccess.get() != NULL)
				m_poChipAccess->disconnect();

		}catch(SignalException& ex)
		{
			string err;

			ex.addMessage("trying to disconnect external chip access " +  m_sServerType);
			err= ex.getTraceString();
			cout << endl << err << endl;
			LOG(LOG_ERROR, err);
			return;
		}
		if(	m_bKernel == true &&
			m_bKernelOnly == false	)
		{
			m_pKernelModule->stop(false);
		}
	}

	EHObj OWServer::stop(const bool *bWait/*= NULL*/)
	{
		ErrorHandling errHandle;

		m_pError= Thread::stop(false);
		AROUSEALL(m_PRIORITYCACHECOND);
		if(m_pKernelModule.get() != NULL)
		{
			errHandle= m_pKernelModule->stop(bWait);
			if(!m_pError->hasError())
				(*m_pError)= errHandle;
		}
		if(	!m_pError->hasError() &&
			bWait != NULL &&
			*bWait == true	)
		{
			m_pError= Thread::stop(bWait);
		}
		return m_pError;
	}

	OWServer::~OWServer()
	{
		DESTROYMUTEX(m_WRITEONCHIP);
		DESTROYMUTEX(m_WRITECACHE);
		DESTROYMUTEX(m_READCACHE);
		DESTROYMUTEX(m_PRIORITYCACHE);
		DESTROYMUTEX(m_PRIORITY1CHIP);
		DESTROYMUTEX(m_CACHEWRITEENTRYS);
		DESTROYMUTEX(m_EXECUTEMUTEX);
		DESTROYCOND(m_PRIORITYCACHECOND);
	}

}
