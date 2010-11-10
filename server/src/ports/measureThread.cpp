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
#include <sys/io.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <sstream>

#include "../util/debug.h"

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

void MeasureThread::setDebug(bool bDebug, const string& subroutine)
{
	vector<sub>::size_type count(0);
	vector<sub>::size_type nSize(m_pvtSubroutines->size());

	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->bCorrect)
		{
			if(	subroutine == "" ||
				it->name == subroutine	)
			{
				it->portClass->setDebug(bDebug);
				++count;
				if(subroutine != "")
					break;
			}
		}else
			++count;
	}
	LOCK(m_DEBUGLOCK);
	if(!bDebug)
	{
		if(nSize == count)
			m_bDebug= false;
	}else
		m_bDebug= true;
	//m_nDebugSleep= sleep;
	UNLOCK(m_DEBUGLOCK);
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
	SHAREDPTR::shared_ptr<IListObjectPattern> port;
	MeasureArgArray tArg= *((MeasureArgArray*)arg);

	m_pvlPorts= tArg.ports;
	m_pvtSubroutines= tArg.subroutines;

	nMuch= m_pvtSubroutines->size();
	for(int n= 0; n<nMuch; n++)
	{
		if((*m_pvtSubroutines)[n].bCorrect)
		{
			//cout << "define subroutine " << (*m_pvtSubroutines)[n].name << endl;
			port= (*m_pvtSubroutines)[n].portClass;
			port->setDebug(false);
			port->setObserver(this);
			port->setRunningThread(this);
		}
	}

	return 0;
}

void MeasureThread::changedValue(const string& folder, const string& from)
{
	LOCK(m_VALUE);
	m_vFolder.push_back(from);
	AROUSE(m_VALUECONDITION);
	UNLOCK(m_VALUE);
}

int MeasureThread::stop(const bool* bWait/*=NULL*/)
{
	int nRv;

	nRv= Thread::stop(false);
	//LOCK(m_VALUE);
	AROUSE(m_VALUECONDITION);
	//UNLOCK(m_VALUE);
	if(	bWait
		&&
		*bWait	)
	{
		nRv= Thread::stop(bWait);
	}
	return nRv;
}

struct time_sort : public binary_function<timeval, timeval, bool>
{
	bool operator()(timeval x, timeval y)
	{
		if(x.tv_sec < y.tv_sec) return true;
		return x.tv_usec < y.tv_usec;
	}
};

string MeasureThread::getUsecString(const suseconds_t usec)
{
	string::size_type nLen;
	string sRv;
	ostringstream stream;

	stream << usec;
	nLen= stream.str().length();
	for(string::size_type o= 6; o > nLen; --o)
		sRv+= "0";
	sRv+= stream.str();
	return sRv;
}

int MeasureThread::execute()
{
	bool debug(isDebug());
	string folder(getThreadName());
	static timeval start_tv;
	timeval end_tv, cond;
	timespec waittm;
	vector<timeval>::iterator akttime;

	/*string stopfolder("TRANSMIT_SONY_choice");
	if(folder == stopfolder)
	{
		cout << __FILE__ << __LINE__ << endl;
		cout << "by starting folder " << stopfolder << endl;
		//setDebug(true, 0);
	}*/
	measure();
	if(debug)
	{
		folder= getThreadName();
		if(gettimeofday(&end_tv, NULL))
			cout << " ERROR: cannot calculate time of ending" << endl;
		else
		{
			char stime[18];

			strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&end_tv.tv_sec));
			cout << " folder '" << folder << "' STOP (" << stime << " ";
			cout << getUsecString(end_tv.tv_usec) << ")";

			timersub(&end_tv, &start_tv, &end_tv);
			cout << "  running time (" << end_tv.tv_sec << ".";
			cout << getUsecString(end_tv.tv_usec) << ")" << endl;
		}
		cout << "----------------------------------" << endl << endl << endl;
		timerclear(&cond);
	}
	LOCK(m_VALUE);
	m_vInformed.clear();
	if(m_vFolder.empty())
	{
		if(!m_vtmNextTime.empty())
		{
			sort(m_vtmNextTime.begin(), m_vtmNextTime.end(), time_sort());

			akttime= m_vtmNextTime.begin();
			waittm.tv_sec= akttime->tv_sec;
			waittm.tv_nsec= akttime->tv_usec * 1000;
			if(TIMECONDITION(m_VALUECONDITION, m_VALUE, &waittm) == ETIMEDOUT)
			{
				if(debug || folder == "TRANSMIT_SONY")
					cond= *akttime;
				m_vtmNextTime.erase(akttime);
			}
		}else
			CONDITION(m_VALUECONDITION, m_VALUE);
	}
	if(stopping())
		return 0;
	m_vInformed= m_vFolder;
	m_vFolder.clear();

	if(debug)
	{
		string msg("### DEBUGGING for folder ");

		msg+= folder + " is aktivated!";
		TIMELOG(LOG_INFO, folder, msg);

		cout << "----------------------------------" << endl;
		if(gettimeofday(&start_tv, NULL))
			cout << " ERROR: cannot calculate time of beginning" << endl;
		else
		{
			char stime[18];

			strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&start_tv.tv_sec));
			cout << " folder '" << folder << "' START (" << stime << " ";
			cout << getUsecString(start_tv.tv_usec) << ")" << endl;
			if(timerisset(&cond))
			{
				strftime(stime, 16, "%Y%m%d:%H%M%S", localtime(&cond.tv_sec));
				cout << "      awaked from setting time " << stime << " ";
				cout << getUsecString(cond.tv_usec) << ")" << endl;
			}
			for(vector<string>::iterator i= m_vInformed.begin(); i != m_vInformed.end(); ++i)
			{
				cout << "    informed ";
				if(i->substr(0, 1) == "|")
				{
					if(i->substr(1, 1) == "|")
						cout << "from ppi-reader '" << i->substr(2) << "'" << endl;
					else
						cout << "over Internet connection account '" << i->substr(1) << "'" << endl;
				}else
					cout << "from " << *i << " because value was changed" << endl;
			}
		}
		cout << "----------------------------------" << endl;
	}
	UNLOCK(m_VALUE);
	return 0;
}

vector<string> MeasureThread::wasInformed()
{
	vector<string> vRv;

	LOCK(m_VALUE);
	vRv= m_vInformed;
	UNLOCK(m_VALUE);
	return vRv;
}

void MeasureThread::ending()
{
}

bool MeasureThread::measure()
{
	bool debug(isDebug()), notime(false), classdebug;
	string folder("i:"+getThreadName());
	timeval tv, tv_end;

	if(debug)
	{
		if(gettimeofday(&tv, NULL))
		{
			cout << " ERROR: cannot calculate time of ending" << endl;
			notime= true;
		}
	}
	for(vector<sub>::iterator it= m_pvtSubroutines->begin(); it != m_pvtSubroutines->end(); ++it)
	{
		if(it->bCorrect)
		{
			double result;

			/*string stopfolder("TRANSMIT_SONY");
			string stopsub("correct_group");
			if(	getThreadName() == stopfolder &&
				( 	stopsub == "" ||
					it->name == stopsub	)	)
			{
				cout << __FILE__ << __LINE__ << endl;
				cout << stopfolder << ":" << it->name << endl;
			}*/
			if( debug &&
				it->portClass->isDebug())
			{
				classdebug= true;
				cout << "execute subroutine " << it->portClass->getType() << " '" << it->name << "' ";
				if(notime || gettimeofday(&tv_end, NULL))
					cout << " (no length)" << endl;
				else
				{
					timersub(&tv_end, &tv, &tv_end);
					cout << " (" << tv_end.tv_sec << ".";
					cout << getUsecString(tv_end.tv_usec) << ")" << endl;
				}
			}else
				classdebug= false;
			result= it->portClass->measure(it->portClass->getValue(folder));
			it->portClass->setValue(result, folder+":"+it->name);


		}else if(debug)
			cout << "Subroutine " << it->name << " is not correct initialized" << endl;
		if(classdebug)
			cout << "----------------------------------" << endl;
		if(stopping())
			break;
	}
	return true;
}

SHAREDPTR::shared_ptr<IListObjectPattern> MeasureThread::getPortClass(const string name, bool &bCorrect) const
{
	SHAREDPTR::shared_ptr<IListObjectPattern> pRv;

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
