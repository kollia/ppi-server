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
#include <queue>

#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"

#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/thread/Thread.h"

using namespace design_pattern_world::util_pattern;

struct MeasureArgArray
{
	vector<pair<string, PortTypes> > ports;
	vector<sub> *subroutines;
	/**
	 * all subroutines for debugging from begin
	 */
	vector<string> debugSubroutines;
};

class MeasureThread : 	public Thread,
						virtual public IMeasurePattern
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
		SHAREDPTR::shared_ptr<IListObjectPattern> getPortClass(const string subroutine, bool &bCorrect) const;
		/**
		 * set debug session in subroutine or hole folder when subroutine not given
		 *
		 * @param bDebug wheter should debug session be set or unset
		 * @param subroutine name of subroutine
		 */
		void setDebug(bool bDebug, const string& subroutine= "");
		/**
		 * return actually count of current subroutine
		 *
		 * @param subroutine whitch count should be returned when set, elsewhere create new counts
		 * @return count number of subroutine
		 */
		virtual unsigned short getActCount(const string& subroutine);
		/**
		 * returning true if an client set this measurethread to debug
		 */
		bool isDebug();
		/**
		 *  external command to stop object of MeasureThread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return stop(&bWait); };
		/**
		 *  external command to stop object of MeasureThread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * if any client set debug to true, method returning sleeptime
		 * whitch has client set. Otherwise method returning default time from 3
		 */
		//unsigned short getSleepTime();
		/**
		 * information by changed value in any subroutine
		 *
		 * @param folder which folder should be informed
		 * @param from from which folder comes information
		 */
		virtual void changedValue(const string& folder, const string& from);
		/**
		 * from witch folder:subroutine thread was informed for new value
		 *
		 * @return vector of folder:subroutine which informed
		 */
		virtual vector<string> wasInformed();
		/**
		 * on which time the measure routine should start without any actions on extern ports
		 *
		 * @param folder name of folder
		 * @param time next beginning run time
		 */
		void nextActivateTime(const string& folder, const timeval& time)
		{ LOCK(m_ACTIVATETIME);m_vtmNextTime.push_back(time);UNLOCK(m_ACTIVATETIME); };
		/**
		 * searching whether folder was starting from an specific time condition.<br />
		 * This method will be called only from own thread
		 * so we need no LOCKING. <br />but WARNING:
		 * it's possible to call method also from other threads
		 *        ( method is not locking save !!! )
		 *
		 * @param timeConditionString string of time condition setting before
		 */
		bool hasActivatedTime(const string& timeConditionString);
		/**
		 * get time string of date with microseconds
		 *
		 * @param time structure of time with seconds and microseconds
		 * @param debug whether method should write an error by debugging mode
		 */
		static string getTimevalString(const timeval& time, const bool debug);
		/**
		 * get string with starting zeros from microseconds
		 *
		 * @param usec microseconds
		 */
		static string getUsecString(const suseconds_t usec);
		/**
		 * destructor of MeasureThread
		 */
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
		//set<portBase::Pins> m_vAfterContactPins;
		//map<unsigned long, unsigned> m_vAfterContactPorts;
		bool m_bDebug;
		/**
		 * actually count number of set subroutine
		 */
		unsigned short m_nActCount;
		/**
		 * from which folder:subroutine the thread was informed to change
		 */
		vector<string> m_vInformed;
		/**
		 * all changed folder
		 */
		vector<string> m_vFolder;
		/**
		 * next time to activate measure routine without action from extern
		 */
		vector<timeval> m_vtmNextTime;
		/**
		 * mutex by any changing of value
		 */
		pthread_mutex_t *m_VALUE;
		/**
		 * mutex for fill or erase new activate time
		 */
		pthread_mutex_t *m_ACTIVATETIME;
		/**
		 * mutex by setting debug output
		 */
		pthread_mutex_t *m_DEBUGLOCK;
		/**
		 * condition for wait for new changing of any subroutine
		 */
		pthread_cond_t *m_VALUECONDITION;

		/**
		 * private copy constructor for no allowed copy
		 *
		 * @param x object for coppy
		 */
		MeasureThread(const MeasureThread& x);
		/**
		 * private assignment operator for not allowed allocation
		 *
		 * @param x opbject for assignment
		 * @return own object
		 */
		MeasureThread& operator=(const MeasureThread&);
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
