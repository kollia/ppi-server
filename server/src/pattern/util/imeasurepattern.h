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
				 * subroutine signal whether can find the server for external measuring
				 *
				 * @param bfound whether server was finding
				 * @param server type of server will be found
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
