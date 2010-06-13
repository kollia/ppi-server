/*
 * KernelModule.cpp
 *
 *  Created on: 30.05.2010
 *      Author: root
 */

#include "KernelModule.h"

#include "../database/lib/DbInterface.h"

namespace server {

	using namespace ppi_database;

	KernelModule::KernelModule(const string& servertype, IChipAccessPattern* chipaccess, pthread_mutex_t* readcache)
	:	Thread("KernelModule", 0),
		m_sServerType(servertype),
		m_poChipAccess(chipaccess),
		m_READCACHE(readcache)
	{
	}

	int KernelModule::execute()
	{
		short endWork;
		double value;
		string readchip;
		vector<chip_types_t*>::iterator chip;

		readchip= m_poChipAccess->kernelmodule();
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
						chip_types_t* chip= *it;
						endWork= m_poChipAccess->read((*it)->id, value);
						readChip(endWork, value, *it);

					}while(endWork == 1);
				}
			}
			UNLOCK(m_READCACHE);
		}
		return 0;
	}

	bool KernelModule::readChip(const short endWork, const double value, chip_types_t* pActChip)
								//const bool bDebug, vector<device_debug_t>::iterator devIt)
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

	int KernelModule::stop(const bool bWait)
	{
		m_poChipAccess->disconnect();
		return Thread::stop(bWait);
	}

	KernelModule::~KernelModule()
	{
	}

}
