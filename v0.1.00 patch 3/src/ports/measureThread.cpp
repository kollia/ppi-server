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
#include <iostream>
#include <vector>
#include <sys/io.h>
#include <unistd.h>
#include <sstream>

#include "../util/debug.h"
#include "../util/Thread.h"

#include "../logger/LogThread.h"

#include "measureThread.h"

using namespace std;

meash_t* meash_t::firstInstance= NULL;
string meash_t::clientPath= "";

MeasureThread::MeasureThread(string threadname) :
Thread(threadname, /*defaultSleep*/0)
{
#ifdef DEBUG
	cout << "constructor of measurethread for folder " << getThreadName() << endl;
#endif // DEBUG
	m_VALUE= Thread::getMutex("VALUE");
	m_DEBUGLOCK= Thread::getMutex("DEBUGLOCK");
	m_bDebug= false;
	m_nDebugSleep= 3;
}

void MeasureThread::setDebug(bool bDebug, unsigned short sleep)
{
	int nMuch;

	LOCK(m_DEBUGLOCK);
	m_bDebug= bDebug;
	m_nDebugSleep= sleep;
	UNLOCK(m_DEBUGLOCK);

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
		(*m_pvtSubroutines)[n].portClass->setDebug(bDebug);
}

bool MeasureThread::isDebug()
{
	bool debug;

	LOCK(m_DEBUGLOCK);
	debug= m_bDebug;
	UNLOCK(m_DEBUGLOCK);

	return debug;
}

unsigned short MeasureThread::getSleepTime()
{
	unsigned short nTime= 3;

	LOCK(m_DEBUGLOCK);
	if(m_bDebug)
		nTime= m_nDebugSleep;
	UNLOCK(m_DEBUGLOCK);

	return nTime;
}

bool MeasureThread::init(void *arg)
{
	int nMuch;
	portBase* port;
	MeasureArgArray tArg= *((MeasureArgArray*)arg);

	/*if(ioperm(COM1, 8, 1))
	{
		LOG(LOG_ERROR, "cannot open port");
		return false;
	}*/
	m_pvlPorts= tArg.ports;
	m_pvtSubroutines= tArg.subroutines;
	m_vAfterContactPins= tArg.tAfterContactPins;
	// create in map all ports with offset
	// which are needed for after Contact
	for(set<portBase::Pins>::iterator i= m_vAfterContactPins.begin(); i!=m_vAfterContactPins.end(); ++i)
	{
		portBase::portpin_address_t pin= portBase::getPortPinAddress(*i, true);

		m_vAfterContactPorts[pin.nPort]= 0x00;
	}

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		if((*m_pvtSubroutines)[n].bCorrect)
		{
			port= (*m_pvtSubroutines)[n].portClass;
			port->setDebug(false);
			port->setAfterContact(m_vAfterContactPorts, m_vAfterContactPins);
		}
	}
	return true;
}

void MeasureThread::execute()
{
	bool bSleeped;

	bSleeped= measure();

	if(isDebug())
	{
		string thread(getThreadName());
		string msg("### DEBUGGING for folder ");

		msg+= thread + " is aktivated!";
		TIMELOG(LOG_WARNING, thread, msg);
	}
	if(!bSleeped)
	{
		unsigned short time= getSleepTime();
		//string msg("sleep ");
		ostringstream msg;

		msg << "sleep " << time << " seconds for folder ";
		msg << getThreadName();
		if(isDebug())
			cout << msg << endl;
		sleep(time);
		TIMELOG(LOG_DEBUG, "sleep" + getgid(), msg.str());
	}
}

void MeasureThread::ending()
{
}

bool MeasureThread::measure()
{
	bool bSleeped= false;
	int nMuch;
	sub subroutine;

	// fill all ports with function inb() from kernel
	// which are needed for after contact
	/*for(map<unsigned long, unsigned>::iterator i= m_vAfterContactPorts.begin(); i!=m_vAfterContactPorts.end(); ++i)
	{
		m_vAfterContactPorts[i->first]= inb(i->first);
	}*/
	//LOCK(m_VALUE);
	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		subroutine= (*m_pvtSubroutines)[n];
		if(subroutine.bCorrect)
		{
			string msg;

			if(isDebug())
			{
				cout << "execute subroutine '" << subroutine.name << "'" << endl;
			}
			subroutine.portClass->measure();
			if(stopping())
				break;

			//UNLOCK(m_VALUE);
			if(	subroutine.sleep
				||
				(	isDebug()
					&&
					subroutine.usleep	)	)
			{
				char time[10];
				unsigned short sleeptime;

				if(isDebug())
					sleeptime= getSleepTime();
				else
					sleeptime= subroutine.sleep;
				sprintf(time, "%d", sleeptime);
				msg+= "sleep ";
				msg+= time;
				msg+= " seconds";
				sleep(sleeptime);

			}
			if(	subroutine.usleep
				&&
				!isDebug()						)
			{
				char time[20];

				sprintf(time, "%li", subroutine.usleep);
				if(msg == "")
					msg+= "sleep ";
				else
					msg+= " and ";
				msg+= time;
				msg+= " mikroseconds";
				usleep(subroutine.usleep);
			}
			if(msg != "")
			{
				string thread(getThreadName());

				msg+= " for folder ";
				msg+= thread;
				TIMELOG(LOG_DEBUG, thread, msg);
				if(isDebug())
					cout << msg << endl;
				bSleeped= true;
			}
			if(isDebug())
				cout << "----------------------------------" << endl;
			//LOCK(m_VALUE);
		}
		//UNLOCK(m_VALUE);
		if(stopping())
			break;
	}
	if(isDebug())
		cout << endl << endl;
	return bSleeped;
}

portBase* MeasureThread::getPortClass(const string name, bool &bCorrect) const
{
	portBase* pRv= NULL;

	bCorrect= false;
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->name == name)
		{
			pRv= it->portClass;
			if(it->bCorrect)
				bCorrect= true;
			break;
		}
	}
	return pRv;
}

/*bool MeasureThread::setValue(string name, double value)
{
	bool bFound= false;
	unsigned int nSize;

	LOCK(m_VALUE);
	nSize= m_pvtSubroutines->size();
	for(unsigned int n= 0; n<nSize; ++n)
	{
		if((*m_pvtSubroutines)[n].name == name)
		{
			portBase *port= (*m_pvtSubroutines)[n].portClass;

			port->setValue(value);
			bFound= true;
			break;
		}
	}
	UNLOCK(m_VALUE);
	return bFound;
}

double MeasureThread::getValue(string name, bool &bFound)
{
	double nRv= 0;
	unsigned int nSize;

	bFound= false;
	LOCK(m_VALUE);
	nSize= m_pvtSubroutines->size();
	for(unsigned int n= 0; n<nSize; ++n)
	{
		if((*m_pvtSubroutines)[n].name == name)
		{
			portBase *port= (*m_pvtSubroutines)[n].portClass;

			if(port == NULL)
				break;
			nRv= port->getValue();
			bFound= true;
			break;
		}
	}
	UNLOCK(m_VALUE);
	return nRv;
}*/

MeasureThread::~MeasureThread()
{
#ifdef DEBUG
	cout << "destructure of measureThread for folder "<< getThreadName() << endl;
#endif // DEBUG
}
