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
#ifndef HEARINGTHREAD_H_
#define HEARINGTHREAD_H_

#include <string>

#include "../util/Thread.h"

namespace server
{

/**
 * length of reading char on file handle
 */
#define BUFLENGTH 100

	/**
	 * This class create an second connection
	 * for the client to hear about changes
	 *
	 * @author Alexander Kolli
	 * @version 1.0
	 */
	class HearingThread : public Thread
	{
	public:
		/**
		 * Constructor to create hearing-instance
		 *
		 * @param host on which host should be connected
		 * @param port over which port should be connect
		 * @param communicationID ID which should send to server
		 * @param user user for second connection
		 * @param pwd password for second connection
		 * @param bOwDebug whether the session is set for debug an OWServer
		 */
		HearingThread(string host, unsigned short port, string communicationID, string user, string pwd, bool bOwDebug);
		/**
		 * Destructor of HearingThread
		 */
		virtual ~HearingThread();

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
		virtual int init(void *args);
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

	private:
		/**
		 * user for second connection
		 */
		string m_sUser;
		/**
		 * password for second connection
		 */
		string m_sPwd;
		/**
		 * on which host should be connected
		 */
		string m_shost;
		/**
		 * over which port should connect
		 */
		unsigned short m_nPort;
		/**
		 * with which ID the client should communicate
		 * whith the server
		 */
		string m_sCommunicationID;
		/**
		 * whether the session is set for debug an OWServer
		 */
		bool m_bOwDebug;
	};

}

#endif /*HEARINGTHREAD_H_*/
