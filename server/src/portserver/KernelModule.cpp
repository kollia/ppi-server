/*
 * KernelModule.cpp
 *
 *  Created on: 30.05.2010
 *      Author: root
 */

#include "KernelModule.h"

#include "../pattern/util/LogHolderPattern.h"

#include "../database/lib/DbInterface.h"

namespace server {

	using namespace ppi_database;

	KernelModule::KernelModule(const string& servertype, IChipAccessPattern* chipaccess,
			pthread_mutex_t* readcache, pthread_cond_t* prioritycond)
	:	Thread("KernelModule"),
		m_sServerType(servertype),
		m_poChipAccess(chipaccess),
		m_READCACHE(readcache),
		m_PRIORITYCACHECOND(prioritycond)
	{
		m_POLLREAD= getMutex("POLLREAD");
	}

	EHObj KernelModule::init(void *args)
	{
		string threadName("KernelModul[");

		threadName+= m_sServerType + ")";
		LogHolderPattern::instance()->setThreadName(threadName);
		return m_pError;
	}

	bool KernelModule::changeReadPoll(map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > >& sequences,
			map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > >& read)
	{
		bool bPoll(false);
		vector<SHAREDPTR::shared_ptr<chip_types_t> >::iterator found;
		map<double, vector<SHAREDPTR::shared_ptr<chip_types_t> > >::iterator chip;

		LOCK(m_POLLREAD);
		// first search for all devices in maybe littler TrueReadingCache named read
		for(map<string, bool>::iterator newChip= m_mPollRead.begin(); newChip != m_mPollRead.end(); ++newChip)
		{
			//cout << "search for '" << newChip->first << "'" << endl;
			for(chip= read.begin(); chip != read.end(); ++chip)
			{
				for(found= chip->second.begin(); found != chip->second.end(); ++found)
				{
					//cout << "     find '"<< (*found)->id << "'" << endl;
					if((*found)->id == newChip->first)
						break;
				}
				if(found != chip->second.end())
					break;
			}
			if(	chip != read.end()) // if chip isn't end
			{						// found also can not be end
				if(newChip->second == false)
				{
					(*found)->bPoll= false;
					read[chip->first].erase(found);

				}else if((*found)->bPoll == false)
				{
					(*found)->bPoll= true;
					bPoll= true;
				}
			}else
			{
				// elsewhere if not found search in ReadingCache sequences
				for(chip= sequences.begin(); chip != sequences.end(); ++chip)
				{
					for(found= chip->second.begin(); found != chip->second.end(); ++found)
					{
						//cout << "     find '"<< (*found)->id << "'" << endl;
						if((*found)->id == newChip->first)
							break;
					}
					if(found != chip->second.end())
					{
						(*found)->bPoll= newChip->second;
						if(newChip->second)
						{
							bPoll= true;
							read[chip->first].push_back(*found);
						}
						break;
					}
				}
			}
		}
		m_mPollRead.clear();
		UNLOCK(m_POLLREAD);
		return bPoll;
	}

	bool KernelModule::execute()
	{
		short endWork;
		double value;
		string readchip;
		vector<chip_types_t*>::iterator chip;
		map<string, bool> poll;

		readchip= m_poChipAccess->kernelmodule(poll);
		LOCK(m_READCACHE);
		LOCK(m_POLLREAD);
		if(poll.size())
		{
			AROUSE(m_PRIORITYCACHECOND);
			for(map<string, bool>::iterator it= poll.begin(); it != poll.end(); ++it)
				m_mPollRead[it->first]= it->second;
		}
		UNLOCK(m_POLLREAD);
		UNLOCK(m_READCACHE);
		if(readchip != "")
		{
			LOCK(m_READCACHE);
			for(chip= m_vkernelR.begin(); chip != m_vkernelR.end(); ++chip)
			{
				if((*chip)->id == readchip)
					break;
			}
			if(chip != m_vkernelR.end())
			{
				do{
					endWork= m_poChipAccess->read(readchip, value);
					readChip(endWork, value, *chip);

				}while(endWork == 1);
			}else
			{
				vector<device_debug_t>::iterator devIt;

				for(vector<chip_types_t*>::iterator it= m_vkernelR.begin(); it != m_vkernelR.end(); ++it)
				{
					do{
						endWork= m_poChipAccess->read((*it)->id, value);
						readChip(endWork, value, *it);

					}while(endWork == 1);
				}
			}
			UNLOCK(m_READCACHE);
		}
		return true;
	}

	bool KernelModule::readChip(const short endWork, const double value, chip_types_t* pActChip)
	{
		bool bDo= false;
		bool device= false;

		switch (endWork)
		{
		case -1:
			// an error occured
			device= false;
			bDo= true;
			break;
		case 0:
			// reading was correctly and the pin is finished
			device= true;
			bDo= true;
			break;
		case 1:
			// reading was also correctly but the next time should make the same pin (value is not the last state),
			device= true;
			bDo= true;
			break;
		case 3:
			// nothing was to do, value param set,
			// was reading befor (chip wasn't read,
			// value is correct) -> go to the next pin)
			device= true;
		case 4:
			// reading was correctly and the pin is finished
			device= true;
			bDo= true;
			break;
		default:
			// unknown result
			device= false;
			break;
		}
		if(	pActChip->device != device
			||
			pActChip->value != value		)
		{
			DbInterface::instance()->changedChip(m_sServerType, pActChip->id, value, device);
		}
		pActChip->device= device;
		pActChip->value= value;
		return bDo;
	}

	EHObj KernelModule::stop(const bool *bWait/*= NULL*/)
	{
		m_poChipAccess->disconnect();
		return Thread::stop(bWait);
	}

	KernelModule::~KernelModule()
	{
		DESTROYMUTEX(m_POLLREAD);
	}

}
