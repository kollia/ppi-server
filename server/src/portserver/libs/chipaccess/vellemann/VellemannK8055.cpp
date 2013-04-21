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

#include "VellemannK8055.h"
#ifdef _K8055LIBRARY

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>

extern "C" {
#include <k8055.h>
}

#include "../../../../pattern/util/LogHolderPattern.h"

#include "../../../../util/thread/Thread.h"
#include "../../../../util/GlobalStaticMethods.h"


using namespace std;

namespace ports
{

	VellemannK8055::VellemannK8055(long ID)
	{
		m_nID= ID;
		m_bDebug= false;
		m_bConnected= false;
		m_DEBUGINFO= Thread::getMutex("MAXIMDEBUGINFO");
		m_nDigitalInput= 0x00;
		m_nAnalogOutputC1= 0;
		m_nAnalogOutputC2= 0;
		m_nDigitalOutput= 0x00;
		m_nAnalogInputC1= 0;
		m_nAnalogInputC2= 0;
	}

	bool VellemannK8055::init(const IPropertyPattern* properties)
	{
		if(!connect())
			return true;// no error try to connect all seconds
		return true;
	}

	void VellemannK8055::usePropActions(const IActionPropertyPattern* properties) const
	{
		bool read;
		string pin;

		properties->getValue("ID");
		pin= properties->getValue("pin").substr(0, 1);

		if(	pin == "I" // digital
			||
			pin == "A" // analog
			||
			pin == "d" // debounce
			||
			pin == "r" ) // reset
		{
			read= false;
		}else
			read= true;

		if(read)
		{
			properties->getValue("cache", /*warning*/false);
			properties->haveAction("current");

		}else
		{
			properties->getValue("priority", /*warning*/false);
			properties->haveAction("cache");
		}

		properties->haveAction("db");
		properties->haveAction("perm");
	}

	short VellemannK8055::useChip(const IActionPropertyMsgPattern* prop, string& id, unsigned short& kernelmode)
	{
		short nRv= 0;
		long def;
		string msg;
		string pin;
		string property;

		kernelmode= 0;
		property= "ID";
		def= (long)prop->getInt(property);
		if(property == "#ERROR")
			return -1;
		if(def != m_nID)
			return 0;
		pin= prop->needValue("pin");
		//cout << "use pin " << pin << endl;
		if(id == "")
			return -1;
		if(	pin == "01"
			||
			pin == "02"
			||
			pin == "03"
			||
			pin == "04"
			||
			pin == "05"
			||
			pin == "06"
			||
			pin == "07"
			||
			pin == "08"
			||
			pin == "PWM1"
			||
			pin == "PWM2"
			||
			pin == "debounce1"
			||
			pin == "debounce2"
			||
			pin == "reset1"
			||
			pin == "reset2"		)
		{
			nRv= 2;
			m_bUsed= true;

		}else if(	(	pin.substr(0, 1) == "I"
						&&
						(	pin.substr(1, 1) == "1"
							||
							pin.substr(1, 1) == "2"
							||
							pin.substr(1, 1) == "3"
							||
							pin.substr(1, 1) == "4"
							||
							pin.substr(1, 1) == "5"	)	)
					||
					(	pin.substr(0, 1) == "A"
						&&
						(	pin.substr(1, 1) == "1"
							||
							pin.substr(1, 1) == "2"	)	)
					||
					(	pin.substr(0, 7) == "counter"
						&&
						(	pin.substr(7, 1) == "1"
							||
							pin.substr(7, 1) == "2"	)	)	)
		{
			nRv= 1;
			m_bUsed= true;
		}else
		{
			msg= "pin ";
			msg+= pin;
			msg+= " for folder ";
			msg+= prop->getValue("folder", /*warning*/false);
			msg+= " and subroutine ";
			msg+= prop->getValue("name", /*warning*/false);
			msg+= " is not correct for the k8055 port";
			cerr << msg << endl;
			LOG(LOG_ERROR, msg);
			return 0;
		}
		id= getChipTypeID(pin);
		return nRv;
	}

	vector<string> VellemannK8055::getUnusedIDs() const
	{
		vector<string> unused;
		ostringstream id;

		if(!m_bUsed)
		{
			id << m_nID;
			unused.push_back(id.str());
		}
		return unused;
	}

	bool VellemannK8055::connect()
	{
		static bool failt= false;

		errno= 0;
		if(OpenDevice(m_nID)<0 )
		{
			ostringstream os;

			os <<  "### ERROR: Could not open the k8055 (port:" << dec << m_nID << ")";
			if(errno != 0)
				   os << "\n    ERRNO(" << dec << errno << "): " << strerror(errno);
			TIMELOG(LOG_ERROR, m_nID+"Vellemannk8055", os.str());
			if(!failt)
			{
				cerr << os.str() << endl;
				failt= true;
			}
			m_bConnected= false;
			return false;
		}
		failt= false;
		m_bConnected= true;
		return true;
	}

	void VellemannK8055::disconnect()
	{
		CloseDevice();
	}

	string VellemannK8055::getChipType(string ID)
	{
		return "Vellemann k8055 port";
	}

	//string VellemannK8055::getConstChipTypeID(const string ID) const
	string VellemannK8055::getChipTypeID(const string pin)
	{
		ostringstream id;

		id << m_nID << ":" << pin;
		return id.str();
	}

	vector<string> VellemannK8055::getChipIDs() const
	{
		vector<string> ids;
		ostringstream id;

		id << m_nID;
		ids.push_back(id.str());
		return ids;
	}

	bool VellemannK8055::existID(const string type, string ID) const
	{
		if(type != "Vk8055")
			return false;
		if(atol(ID.c_str()) == m_nID)
			return true;
		return false;
	}

	bool VellemannK8055::isDebug()
	{
		bool debug;

		Thread::LOCK(m_DEBUGINFO);
		debug= m_bDebug;
		Thread::UNLOCK(m_DEBUGINFO);
		return debug;
	}

	void VellemannK8055::setDebug(const bool debug)
	{
		Thread::LOCK(m_DEBUGINFO);
		m_bDebug= debug;
		Thread::UNLOCK(m_DEBUGINFO);
	}

	short VellemannK8055::write(const string id, const double value, const string& addinfo)
	{
		static bool bfRead= true;
		int res;
		string pin(id.substr(2));

		//cout << "write on pin " << pin << " value " << value << endl;
		if(pin == "PWM1")
		{
			m_nAnalogOutputC1= (int)value;
			res= OutputAnalogChannel(1, m_nAnalogOutputC1);

		}else if(pin == "PWM2")
		{
			m_nAnalogOutputC2= (int)value;
			res= OutputAnalogChannel(2, m_nAnalogOutputC2);

		}else if(pin.substr(0, 5) == "reset")
		{
			long n= atoi(pin.substr(5, 1).c_str());

			res= ResetCounter(n);

		}else if(pin.substr(0, 8) == "debounce")
		{
			long n= atoi(pin.substr(8, 1).c_str());

			res= SetCounterDebounceTime(n, (long)value);

		}else
		{
			int binValue;

			//cout << "actual output " << m_nDigitalOutput << endl;
			//portBase::printBin(&m_nDigitalOutput, 0);
			binValue= 0x01; // pin is 01
			if(pin == "02") binValue<<= 1; else
			if(pin == "03") binValue<<= 2; else
			if(pin == "04") binValue<<= 3; else
			if(pin == "05") binValue<<= 4; else
			if(pin == "06") binValue<<= 5; else
			if(pin == "07") binValue<<= 6; else
			if(pin == "08") binValue<<= 7;
			if(value == 0)
			{
				binValue= ~binValue;
				//cout << " and:" << endl;
				//portBase::printBin(&binValue, 0);
				m_nDigitalOutput&= binValue;
			}else
			{
				//cout << " or:" << endl;
				//portBase::printBin(&binValue, 0);
				m_nDigitalOutput|= binValue;
			}
			//cout << "write output to " << m_nDigitalOutput << endl;
			//portBase::printBin(&m_nDigitalOutput, 0);
			res= WriteAllDigital((long)m_nDigitalOutput);
		}
		if(res < 0)
		{
			if(bfRead)
			{
				disconnect();
				if(connect())
				{
					short nRv;
					ostringstream oid;

					oid << m_nID;
					LOG(LOG_DEBUG, "reconnect successfully vellemann port "+ oid.str() + " by writing channels");
					bfRead= false;
					nRv= write(id, value, addinfo);
					bfRead= true;
					return nRv;
				}
			}
			return -1;
		}
		return 0;
	}

	short VellemannK8055::read(const string id, double &value)
	{
		short nRv(0);
		string spin(id.substr(2));

		if(spin.substr(0, 7) == "counter")
		{
			long n= atoi(spin.substr(7, 1).c_str());

			if(n > 0 && n <= 2)
			{
				value= (double)ReadCounter(n);
			}else
			{
				value= 0;
				nRv= -1;
			}

		}else if(spin.substr(0, 1) == "I")
		{
			long count= 0;
			long oldcount= 0;
			int pin= atoi(&spin[1]);
			int res;
			int nvalue= (int)value;

			if(pin > 0 && pin <= 5)
			{
				res= ReadDigitalChannel(pin);
				//cout << "read pin " << id << " is single result " << res << endl;
				if(res < 0)
				{
					disconnect();
					if(connect())
					{
						ostringstream id;

						id << m_nID;
						LOG(LOG_DEBUG, "reconnect successfully vellemann port "+ id.str() + " by reading digital channels");
						res= ReadDigitalChannel(pin);
					}
					if(res < 0)
					{
						nRv= -1;
						value= 0;
					}
				}
				if(nRv >= 0)
				{
					if(pin > 0 && pin <= 2)
					{
						map<int, long>::iterator found;

						count= ReadCounter(pin);
						found= m_mnCount.find(pin);
						if(found != m_mnCount.end())
						{
							oldcount= found->second;
							found->second= count;
						}else
						{
							m_mnCount[pin]= count;
							oldcount= count;
						}
					}else
					{
						count= 0;
						oldcount= 0;
					}
					if(	res || count != oldcount)
						nvalue= res ? 0x03 : 0x02;
					else
						nvalue&= 0x02;
					value= (double)nvalue;
					//cout << " is result " << glob::getBinString(static_cast<long>(nvalue), 2) << endl;
					if(value < 0)
					{
						value= 0;
						nRv= -1;
					}
				}
			}else
			{
				value= 0;
				nRv= -1;
			}

			//cout << " direkt pin " << id;
			//cout << " is act " << res << " and set to " << dec << value << endl;
		}else
		{
			long channel;

			//cout << "pin " << id << endl;
			if(spin.substr(1, 1) == "1")
				channel= 1;
			else if(spin.substr(1, 1) == "2")
				channel= 2;
			else
			{
				value= 0;
				nRv= -1;
			}
			if(nRv == 0)
			{
				value= (double)ReadAnalogChannel(channel);
				if(value < 0)
				{
					disconnect();
					if(connect())
					{
						ostringstream id;

						id << m_nID;
						LOG(LOG_DEBUG, "reconnect successfully vellemann port "+ id.str() + " by reading analog channels");
						value= (double)ReadAnalogChannel(channel);
					}
					if(value < 0)
					{
						value= 0;
						nRv= -1;
					}
				}
			}
			//cout << "value is " << dec << value << endl;
		}
		return nRv;
	}

	int VellemannK8055::command_exec(const string& command, vector<string>& result, bool& more)
	{
		string pin;

		more= false;
		result.clear();
		if(command != "")
			return -1;
		result.push_back("01");
		result.push_back("02");
		result.push_back("03");
		result.push_back("04");
		result.push_back("05");
		result.push_back("06");
		result.push_back("07");
		result.push_back("08");
		result.push_back("PWM1");
		result.push_back("PWM2");
		result.push_back("debounce1");
		result.push_back("debounce2");
		result.push_back("reset1");
		result.push_back("reset2");
		result.push_back("I1");
		result.push_back("I2");
		result.push_back("I3");
		result.push_back("I4");
		result.push_back("I5");
		result.push_back("A1");
		result.push_back("A2");
		result.push_back("counter1");
		result.push_back("counter2");
		return 0;
	}

	void VellemannK8055::range(const string id, double& min, double& max, bool &bfloat)
	{
		string pin(id.substr(2));

		bfloat= false;
		min= 0;
		if(pin.substr(0, 7) == "counter")
		{
			min= 0;
			max= -1;
			return;
		}
		if(	pin.substr(0, 1) == "A"
			||
			pin.substr(0, 3) == "PWM"	)
		{
			max= 255;

		}else if(pin.substr(0, 8) == "debounce")
		{
			max= 7450;
		}else// pin 01 - 08 and I1 - I5
			max= 1;
	}

	void VellemannK8055::endOfCacheReading(const double cachetime)
	{

	}

	bool VellemannK8055::reachAllChips()
	{
		if(m_bUsed)
			return true;
		return false;
	}

	void VellemannK8055::endOfLoop()
	{
		// nothing to do
	}

	VellemannK8055::~VellemannK8055()
	{
		DESTROYMUTEX(m_DEBUGINFO);
	}

}
#endif //_K8055LIBRARY
