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

#include "../logger/lib/LogInterface.h"

#include "../ports/OwfsPort.h"

#include "measureThread.h"

using namespace std;
using namespace ports;

SHAREDPTR::shared_ptr<meash_t> meash_t::firstInstance= SHAREDPTR::shared_ptr<meash_t>();
string meash_t::clientPath= "";

MeasureThread::MeasureThread(string threadname) :
Thread(threadname, /*defaultSleep*/0)
{
#ifdef DEBUG
	cout << "constructor of measurethread for folder " << getThreadName() << endl;
#endif // DEBUG
	m_DEBUGLOCK= Thread::getMutex("DEBUGLOCK");
	m_VALUE= Thread::getMutex("VALUE");
	m_VALUECONDITION= Thread::getCondition("VALUECONDITION");
	m_bDebug= false;
}

void MeasureThread::setDebug(bool bDebug, unsigned short sleep)
{
	int nMuch;

	LOCK(m_DEBUGLOCK);
	m_bDebug= bDebug;
	//m_nDebugSleep= sleep;
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

int MeasureThread::init(void *arg)
{
	int nMuch;
	SHAREDPTR::shared_ptr<portBase> port;
	MeasureArgArray tArg= *((MeasureArgArray*)arg);

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
			port->setObserver(this);
		}
	}

	return 0;
}

void MeasureThread::changedValue(const string& folder)
{
	LOCK(m_VALUE);
	m_qFolder.push(folder);
	AROUSE(m_VALUECONDITION);
	UNLOCK(m_VALUE);
}

int MeasureThread::stop(const bool* bWait/*=NULL*/)
{
	int nRv;

	nRv= Thread::stop(false);
	LOCK(m_VALUE);
	AROUSE(m_VALUECONDITION);
	UNLOCK(m_VALUE);
	if(	bWait
		&&
		*bWait	)
	{
		nRv= Thread::stop(bWait);
	}
	return nRv;
}

int MeasureThread::execute()
{
	bool bSleeped;

	bSleeped= measure();

	LOCK(m_VALUE);
	if(m_qFolder.empty())
		CONDITION(m_VALUECONDITION, m_VALUE);
	if(stopping())
		return 0;
	while(!m_qFolder.empty())
		m_qFolder.pop();
	UNLOCK(m_VALUE);

	if(isDebug())
	{
		string thread(getThreadName());
		string msg("### DEBUGGING for folder ");

		msg+= thread + " is aktivated!";
		TIMELOG(LOG_WARNING, thread, msg);
	}
	return 0;
}

void MeasureThread::ending()
{
}

bool MeasureThread::measure()
{
	bool bSleeped= false;
	int nMuch;
	sub subroutine;

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		subroutine= (*m_pvtSubroutines)[n];
		if(subroutine.bCorrect)
		{
			//string msg;

			if(isDebug())
			{
				cout << "execute subroutine '" << subroutine.name << "'" << endl;
			}
			subroutine.portClass->measure();

			if(isDebug())
				cout << "----------------------------------" << endl;
		}
		if(stopping())
			break;
	}
	if(isDebug())
		cout << endl << endl;
	return bSleeped;
}

SHAREDPTR::shared_ptr<portBase> MeasureThread::getPortClass(const string name, bool &bCorrect) const
{
	SHAREDPTR::shared_ptr<portBase> pRv;

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

MeasureThread::~MeasureThread()
{
#ifdef DEBUG
	cout << "destructure of measureThread for folder "<< getThreadName() << endl;
#endif // DEBUG
}
