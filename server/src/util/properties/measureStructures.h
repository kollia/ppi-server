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
#ifndef MEASURESTRUCTURES_H_
#define MEASURESTRUCTURES_H_

#include <string>
#include <vector>
#include <set>

#include "../../pattern/util/IListObjectPattern.h"
#include "../../pattern/util/imeasurepattern.h"

#include "../smart_ptr.h"

#include "configpropertycasher.h"

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

/**
 * structure of main working list properties
 */
struct measurefolder_t
{
	string name;
	bool bDefined;
	bool bCorrect;
	unsigned short nFolderID;
	unsigned short nFolderArrayID;
	string sFolderArray;
	vector<string> vsArrayFolders;
	//set<portBase::Pins> afterContactPins;
	//set<portBase::Pins> needInPorts;
	vector<sub> subroutines;
	SHAREDPTR::shared_ptr<IMeasurePattern> runThread;
	SHAREDPTR::shared_ptr<IActionPropertyPattern> folderProperties;
	SHAREDPTR::shared_ptr<measurefolder_t> next;
};

enum PortTypes
{
	PORT, //!- set or read power on one pin
	MPORT, //!- measure time from outgoing power to incoming pin
	RWPORT //!- read or write on hole port with open()
};

#endif /*MEASURESTRUCTURES_H_*/
