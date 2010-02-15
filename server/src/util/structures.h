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

#include "../ports/portbaseclass.h"

#include "../util/smart_ptr.h"
#include "../util/configpropertycasher.h"

using namespace std;
using namespace util;
using namespace ports;


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
	portBase::Pins out;
	portBase::Pins in;
	portBase::Pins negative;
	string sBegin;
	string sWhile;
	string sEnd;
	string sBeginComm;
	string sWhileComm;
	string sEndComm;
	double defaultValue;
	SHAREDPTR::shared_ptr<portBase> portClass;
	unsigned short sleep;
	unsigned long usleep;
	time_t tmlong;
	int nMax;
	int nMin;
	vector<ohm> resistor;
	vector<correction_t> correction;
	vector<unsigned short> ohmVector;
	unsigned short producerBValue;
	short measuredness;
	SHAREDPTR::shared_ptr<ConfigPropertyCasher> property;
};

struct measurefolder_t
{
	string name;
	bool bCorrect;
	set<portBase::Pins> afterContactPins;
	set<portBase::Pins> needInPorts;
	vector<sub> subroutines;
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
	MPORT, //!- measure from outgoing power to incomming pin
	RWPORT //!- read or write on hole port with open()
};

#endif /*STRUCTURES_H_*/
