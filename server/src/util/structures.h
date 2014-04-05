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
#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <string>
#include <vector>
#include <set>

//#include "../ports/portbaseclass.h"
#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"

#include "../util/smart_ptr.h"

#include "../util/properties/configpropertycasher.h"

using namespace std;
using namespace util;
using namespace design_pattern_world::util_pattern;


struct convert_t
{
	float be;
	bool bSetTime;
	unsigned long nMikrosec;

};

struct correction_t : public convert_t
{
	double correction;
};

typedef convert_t ohm;

struct sub
{
	string name;
	bool bCorrect;
	bool bAfterContact;
	string type;
	//portBase::Pins out;
	//portBase::Pins in;
	//portBase::Pins negative;
	string sBegin;
	string sWhile;
	string sEnd;
	string sBeginComm;
	string sWhileComm;
	string sEndComm;
	double defaultValue;
	SHAREDPTR::shared_ptr<IListObjectPattern> portClass;
	time_t tmlong;
	int nMax;
	int nMin;
	vector<ohm> resistor;
	vector<correction_t> correction;
	vector<unsigned short> ohmVector;
	unsigned short producerBValue;
	short measuredness;
	SHAREDPTR::shared_ptr<IActionPropertyPattern> property;
};

struct measurefolder_t
{
	string name;
	bool bDefined;
	bool bCorrect;
	unsigned short nFolderID;
	unsigned short nObjectID;
	string sObject;
	vector<string> vsObjFolders;
	//set<portBase::Pins> afterContactPins;
	//set<portBase::Pins> needInPorts;
	vector<sub> subroutines;
	SHAREDPTR::shared_ptr<IMeasurePattern> runThread;
	SHAREDPTR::shared_ptr<IActionPropertyPattern> folderProperties;
	SHAREDPTR::shared_ptr<measurefolder_t> next;
};

struct timemap_t
{
	unsigned long time;
	unsigned short count;
	//double correction;

	timemap_t()
	{
		time= 0;
		count= 0;
		//correction= 0;
	}
	int operator==(const timemap_t &other) const
	{// sort only time
		if(this->time != other.time)
			return 0;
		return 1;
	};
	timemap_t &operator=(const timemap_t &other)
	{
		this->time= other.time;
		this->count= other.count;
		//this->correction= other.correction;
		return *this;
	};
	int operator<(const timemap_t &other) const
	{// sort only time
		if(this->time < other.time)
			return 1;
		return 0;
	};
};

enum PortTypes
{
	PORT, //!- set or read power on one pin
	MPORT, //!- measure time from outgoing power to incoming pin
	RWPORT //!- read or write on hole port with open()
};

//-------------------------------------------------------------------------------------------------
// debug device for owreader

/**
 * struct to writing debug information for time reading
 * and writing.
 */
struct device_debug_t
{
	/**
	 * count id of debug struct
	 */
	unsigned short id;
	/**
	 * whether actual time can reading gettimeofday
	 */
	bool btime;
	/**
	 * actual time of reading or writing
	 */
	timeval act_tm;
	/**
	 * how often device was read or write in the actual session
	 */
	unsigned short count;
	/**
	 * whether the device is for reading (true)
	 * or writing (false)
	 */
	bool read;
	/**
	 * whether device was reachable
	 */
	bool ok;
	/**
	 * needed time in micro seconds of reading or writing
	 */
	long utime;
	/**
	 * measured or written value
	 */
	double value;
	/**
	 * cache time for reading
	 */
	double cache;
	/**
	 * priority state for writing
	 */
	unsigned int priority;
	/**
	 * id of device
	 */
	string device;
	/**
	 * sort operator to comparing actual time whether own is greater
	 */
	bool operator > (const device_debug_t* other) const
	{
		if(act_tm.tv_sec > other->act_tm.tv_sec)
			return true;
		return act_tm.tv_usec > other->act_tm.tv_usec ? true : false;
	}
	/**
	 * find operator to comparing actual time whether both are the same
	 */
	int operator == (const device_debug_t* other)
	{
		return (device == other->device);
		/*if(	act_tm.tv_sec == other->act_tm.tv_sec
			&&
			act_tm.tv_usec == other->act_tm.tv_usec	)
		{
			return true;
		}
		return false;*/
	}
};

/**
 * struct of chip defined types for OWServer,
 * with an map for actions for all pins
 */
struct chip_types_t
{
	/**
	 * unique ID of pin
	 */
	string id;
	/**
	 * unique ID for pins in an cache writing
	 */
	string wcacheID;
	/**
	 * whether value should read or write
	 */
	bool read;
	/**
	 * whether should read chip by polling
	 */
	bool bPoll;
	/**
	 * whether writing will be make in an cache
	 */
	bool writecache;
	/**
	 * value of pin from chip
	 * if it is an cached reading
	 */
	double value;
	/**
	 * additional info for chip
	 */
	string addinfo;
	/**
	 * priority of pin if it is for writing
	 */
	unsigned int priority;
	/**
	 * seconds for reading again as cache
	 */
	unsigned long sec;
	/**
	 * sequence of time to reading in an cache
	 */
	struct timeval timeSeq;
	/**
	 * whether reading or write on device was correct
	 */
	bool device;
	/**
	 * inline greater operator
	 */
	virtual bool operator > (const chip_types_t* other)
	{
		cout << "chip_types_t '" << id << "' > '" << other->id << "'  ?" << endl;
		return id > other->id ? true : false;
	}
	/**
	 * inline greater operator
	 */
	virtual bool operator < (const chip_types_t* other)
	{
		cout << "chip_types_t '" << id << "' < '" << other->id << "'  ?" << endl;
		return id < other->id ? true : false;
	}
	/**
	 * inline similar operator
	 */
	virtual bool operator == (const chip_types_t* other)
	//bool operator == (const string& other)
	{
		cout << "chip_types_t '" << id << "' == '" << other->id << "'  ?" << endl;
		//return id == other->id ? true : false;
		return id == other->id ? true : false;
	}
	/**
	 * dummy destruktor
	 */
	virtual ~chip_types_t() {};
};

#endif /*STRUCTURES_H_*/
