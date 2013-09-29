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
#ifndef IMEASUREPATTERN_H_
#define IMEASUREPATTERN_H_

#include <sys/time.h>

#include <string>
#include <vector>
#include <map>

using namespace std;


namespace design_pattern_world
{
	namespace util_pattern
	{
		/**
		 * pattern class for MeasureThread to activate new pass by changed value
		 *
		 * @autor Alexander Kolli
		 * @version 1.0.0
		 */
		class IMeasurePattern
		{
			public:
				/**
				 * structure to measure in an algorithm maximal or middle length of time
				 */
				struct timeLen_t
				{
					/**
					 * name of reached finish ("reachend") or folder run length ("runlength")
					 * writing into database
					 */
					string stype;
					/**
					 * mane of maximal count writing into database
					 */
					string scount;
					/**
					 * count of max reading values to take for folder length
					 */
					short read;
					/**
					 * by witch count the max value reached
					 */
					short maxRadCount;
					/**
					 * count of reading length
					 * to write after timetype_t::maxVal into database
					 */
					short count;
					/**
					 * writing new actual value after follow counts
					 */
					short maxCount;
					/**
					 * maximal reading value for folder length
					 * or 100% length value for reach finished result
					 */
					double readValue;
					/**
					 * actual value which is 100%
					 */
					double actValue;
					/**
					 * how much values differ from the act Value
					 * and how often
					 */
					map<short, pair<short, double> > reachedPercent;
					/**
					 * same as reachedPercent but only the last 10 value for runlength
					 * or last 3 for reachend
					 */
					map<short, pair<short, double> > newReachedPercent;

					/**
					 * constructor to set default values
					 */
					timeLen_t()
					: read(0),
					  count(0),
					  maxCount(1),
					  readValue(0),
					  actValue(0)
					{};
				};
				/**
				 * static values for
				 * structure to measure in an algorithm maximal or middle length of time
				 */
				struct timetype_t
				{
					/**
					 * name of folder saved into database
					 */
					string folder;
					/**
					 * name of subroutine saved into database
					 */
					string subroutine;
					/**
					 * whether structure is for length running (true)
					 * or reached finished (false)
					 */
					bool runlength;
					/**
					 * inside how much values calculate middle value
					 */
					short maxVal;
					/**
					 * whether percent was saved into database
					 */
					//bool percentDB;
					/**
					 * percent to which can differ
					 */
					short inPercent;
					/**
					 * map of time structures to calculate longest or middle time consider by CPU time
					 */
					map<short, timeLen_t> percentDiff;
					/**
					 * preview idle time from last CPU creation
					 */
					int prev_idle;
					/**
					 * preview total time from last CPU creation
					 */
					int prev_total;
					/**
					 * CPU percent from last creation.<br />
					 * maybe CPU creation need when method <code>getCpuPercent()</code> was called to fast
					 */
					int old_usage;

					timetype_t()
					: runlength(false),
					  maxVal(5),
					  //percentDB(false),
					  inPercent(10),
					  prev_idle(0),
					  prev_total(0),
					  old_usage(0)
					{};
				};

				/**
				 * returning thread id in which thread folder object running
				 *
				 * @return thread id
				 */
				virtual pid_t getRunningThreadID()= 0;
				/**
				 * returning true if an client set this measurethread to debug
				 *
				 * @return whether measure thread do output
				 */
				virtual bool isDebug()= 0;
				/**
				 * set folder to calculating length of folder time
				 */
				virtual void calculateLengthTime()= 0;
				/**
				 * length time of folder running.<br />
				 * when get unset time back (<code>= !timerisset(<returnvalue>)</code>)
				 * no measuring be set for folder
				 *
				 * @param logPercent whether logging original percent into database
				 * @param debug whether call run in debug session
				 * @return longest measured length of folder time
				 */
				virtual timeval getLengthedTime(const bool& logPercent, const bool& debug)= 0;
				/**
				 * set into given timetype the CPU times to begin measuring for <code>getCpuPercent</code>
				 *
				 * @param timetype all static variables to measure CPU time
				 */
				virtual void setCpuMeasureBegin(timetype_t *timetype)= 0;
				/**
				 * sleep microseconds by consider stopping of running thread
				 *
				 * @param time sleeping time
				 * @return whether thread should stopping
				 */
				virtual bool usleep(timeval time)= 0;
				/**
				 * return actually count of current subroutine
				 *
				 * @param subroutine whitch count should be returned when set, elsewhere create new counts
				 * @return count number of subroutine
				 */
				virtual unsigned short getActCount(const string& subroutine= "")= 0;
				/**
				 * activate new pass of hole folder
				 *
				 * @param folder name of folder
				 */
				virtual void changedValue(const string& folder, const string& from)= 0;
				/**
				 * inform measure thread of folder that needing changing time of subroutine
				 *
				 * @param subroutine name of subroutine which need info
				 * @param from name of [folder]:<subroutine> which was changed
				 */
				virtual void needChangingTime(const string& subroutine, const string& from)= 0;
				/**
				 * return actual maximal changing time from before defined [folder:]<subroutine>
				 *
				 * @param subroutine name of subroutine which need information
				 * @param desc description of taking time by debug session otherwise null string
				 * @return highest time from changed subroutine value
				 */
				virtual timeval getMaxChangingTime(const string& subroutine, const string& desc)= 0;
				/**
				 * from witch folder:subroutine thread was informed for new value
				 *
				 * @return vector of folder:subroutine which informed
				 */
				virtual vector<string> wasInformed()= 0;
				/**
				 * on which time the measure routine should start without any actions on extern ports
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 */
				virtual void nextActivateTime(const string& folder, const timeval& time)= 0;
				/**
				 * searching where folder was starting from an specific time condition
				 * and change starting time
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 * @param newtime new starting time
				 */
				virtual void changeActivationTime(const string& folder, const timeval& time,
								const timeval& newtime)= 0;
				/**
				 * searching where folder was starting from an specific time condition and erase starting.
				 *
				 * @param folder name of folder
				 * @param time next beginning run time
				 */
				virtual void eraseActivateTime(const string& folder, const timeval& time)= 0;
				/**
				 * subroutine signal whether can find the server for external measuring
				 *
				 * @param bfound whether server was finding
				 * @param server stype of server will be found
				 * @param id chip id searched inside server
				 */
				virtual void foundPortServer(const bool bfound, const string& server, const string& id)= 0;
				/**
				 * dummy destructor for pattern
				 */
				virtual ~IMeasurePattern() {};

		};
	}
}

#endif /*IMEASUREPATTERN_H_*/
