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
#ifndef MEASURETHREAD_H_
#define MEASURETHREAD_H_


#include <set>

#include "../util/structures.h"
#include "../util/Thread.h"


struct MeasureArgArray
{
	vector<unsigned long> ports;
	vector<sub> *subroutines;
	//set<unsigned long> afterContact;
	set<portBase::Pins> tAfterContactPins;
};

class MeasureThread : public Thread
{
	public:
		/**
		 * creating instance of MeasureThread
		 *
		 * @param threadname name of running folder
		 */
		MeasureThread(string threadname= "MeasureThread");
		/**
		 * return class of subroutine from this folder
		 *
		 * @param subroutine name of the subroutine
		 * @return class of subroutine
		 */
		portBase* getPortClass(const string subroutine, bool &bCorrect) const;
		//double getValue(string name, bool &bFound);
		//bool setValue(string name, double value);
		void setDebug(bool bDebug, unsigned short sleep);
		/**
		 * returning true if an client set this measurethread to debug
		 */
		bool isDebug();
		/**
		 * if any client set debug to true, method returning sleeptime
		 * whitch has client set. Otherwise method returning default time from 3
		 */
		unsigned short getSleepTime();
		virtual ~MeasureThread();


	protected:
		virtual bool init(void *arg);
		virtual void execute();
		virtual void ending();
		void clear();
		bool measure();

	private:
		vector<unsigned long> m_pvlPorts;
		vector<sub> *m_pvtSubroutines;
		set<portBase::Pins> m_vAfterContactPins;
		map<unsigned long, unsigned> m_vAfterContactPorts;
		bool m_bDebug;
		unsigned short m_nDebugSleep;
		pthread_mutex_t *m_VALUE;
		pthread_mutex_t *m_DEBUGLOCK;
};


class meash_t
{
	public:
		MeasureThread *pMeasure;
		meash_t *next;
		/**
		 * instance of first measure thread
		 */
		static meash_t* firstInstance;
		/**
		 * path in whitch be the layout files
		 * toDo: not the correct place
		 */
		static string clientPath;
};

#endif /*MEASURETHREAD_H_*/
