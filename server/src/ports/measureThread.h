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

#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/Thread.h"


struct MeasureArgArray
{
	vector<pair<string, PortTypes> > ports;
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
		SHAREDPTR::shared_ptr<portBase> getPortClass(const string subroutine, bool &bCorrect) const;
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
		/**
		 * this method will be called before running
		 * the method execute to initial class
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual int init(void *arg);
		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param error code for not correctly done
		 */
		virtual int execute();
		/**
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();
		void clear();
		bool measure();

	private:
		vector<pair<string, PortTypes> > m_pvlPorts;
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
		/**
		 * instance of first measure thread
		 */
		static SHAREDPTR::shared_ptr<meash_t> firstInstance;
		/**
		 * next instnace of meash_t
		 */
		SHAREDPTR::shared_ptr<meash_t> next;
		/**
		 * measure object
		 */
		SHAREDPTR::shared_ptr<MeasureThread> pMeasure;
		/**
		 * path in whitch be the layout files
		 * toDo: not the correct place
		 */
		static string clientPath;
};

#endif /*MEASURETHREAD_H_*/
