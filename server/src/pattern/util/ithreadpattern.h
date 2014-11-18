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
#ifndef ITHREADPATTERN_H_
#define ITHREADPATTERN_H_

//#include <unistd.h>

#include <string>

#include "IErrorHandlingPattern.h"

using namespace std;

namespace design_pattern_world
{
	namespace util_pattern
	{
		/**
		 * pattern class for all threads
		 *
		 * @autor Alexander Kolli
		 * @version 1.0.0
		 */
		class IThreadPattern
		{
			public:
				/**
				 * start method to running the thread parallel
				 *
				 * @param args arbitrary optional defined parameter to get in initialization method init
				 * @param bHold should the caller wait of thread by ending.<br />
				 * 				default is false
				 * @return error handling object
				 */
				virtual EHObj start(void *args= NULL, bool bHold= false)= 0;
				/**
				 * to ask whether the thread should stopping.<br />
				 * This method should be call into running thread to know whether the thread should stop.
				 *
				 * @return 1 if the thread should stop otherwise 0 or -1 by error
				 */
				virtual short stopping()= 0;
				/**
				 * external query whether the thread is running
				 *
				 * @return 1 when thread running otherwise 0 or -1 by error
				 */
				virtual short running()= 0;
				/**
				 *  external command to stop thread
				 *
				 * @param bWait calling rutine should wait until the thread is stopping
				 * @return error handling object
				 */
				virtual EHObj stop(const bool bWait)= 0;
				/**
				 * returning current error handling object
				 *
				 * @return object of error handling
				 */
				virtual EHObj getErrorHandlingObj() const= 0;
				/**
				 * dummy destructor for pattern
				 */
				virtual ~IThreadPattern() {};

		};
	}
}

#endif /*ITHREADPATTERN_H_*/
