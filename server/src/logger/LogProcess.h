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
#ifndef LOGPROCESS_H_
#define LOGPROCESS_H_

//#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#include <string>
#include <vector>

#include "logstructures.h"
#include "LogThread.h"

#include "../util/process.h"

using namespace std;

class LogProcess : public Process
{
	public:
		/**
		 * creating instance of LogThread
		 *
		 * @param uid under witch user the log process should running
		 * @param getConnection on which connection from outside the server is reachable to get questions
		 * @param sendConnection on which connection from outside the server to answer is reachable (default= NULL)
		 * @param wait whether connection should wait for correct access (default= true)
		 */
		LogProcess(const uid_t uid, IClientConnectArtPattern* getConnection,
								IClientConnectArtPattern* sendConnection= NULL, const bool wait= true);

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
		 * this method will be called before running
		 * the method execute to initial class
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return error code for not right initialization
		 */
		virtual void ending();

	private:
		/**
		 * under witch user the process should be running
		 */
		uid_t m_uid;
		/**
		 * answer of last question
		 */
		string m_sAnswer;
		/**
		 * working thread which writing into files
		 */
		LogThread* m_pLogThread;
};

#endif /*LOGPROCESS_H_*/
