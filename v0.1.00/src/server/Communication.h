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
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <sys/socket.h>

#include "../util/structures.h"
#include "../util/Thread.h"

#include "../ports/portbaseclass.h"
#include "../ports/measureThread.h"

#include "../pattern/server/IServerCommunicationStarterPattern.h"
#include "../pattern/server/IFileDescriptorPattern.h"

using namespace design_pattern_world::server_pattern;

typedef IServerCommunicationStarterPattern StarterPattern;

namespace server
{
	/**
	 * representing communication threads
	 * whitch are wating for client how connecting on server.
	 */
	class Communication : public Thread
	{
	public:
		/**
		 * next Communication object
		 */
		Communication* m_pnext;

		/**
		 * initial communication thread
		 *
		 * @param ID actual new connection id of communication
		 * @param starter thread which start communication threads
		 * @param first measure threads which are defined in measure.conf
		 * @param folderStart first folder defined in measure.conf
		 * @param clientPath path of client subdirectory
		 */
		Communication(unsigned int ID, StarterPattern* pStarter, meash_t* first, measurefolder_t *folderStart, string clientPath);
		/**
		 * commit an connection to an client
		 *
		 * @param access file descriptor whitch get from  IServerConnectArtPattern by listen
		 */
		void connection(IFileDescriptorPattern* access);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void *stop(const bool bWait)
		{ return stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void *stop(const bool *bWait= NULL);
		/**
		 * returning the default communication ID
		 * wich needet by starting an new connection.
		 * Set by creating
		 *
		 * @return default client ID
		 */
		unsigned int getDefaultID()
		{ return m_nDefaultID; };
		/**
		 * returning the actual communication ID of the thread
		 *
		 * @return communication ID
		 */
		unsigned int getConnectionID();
		/**
		 * whether an client is connected
		 *
		 * @return true if an client is conected
		 */
		bool hasClient();
		/**
		 * destroy instance of communication thread
		 */
		virtual ~Communication();

	protected:
		/**
		 * initial incomming variables from start method.<br />
		 * first running method of thread on starting
		 */
		virtual bool init(void *args);
		/**
		 * execute of communication.<br />
		 * polling while starter thread do not ending thread
		 */
		virtual void execute();
		/**
		 * protected initialization for given info points
		 * to write into the status information
		 *
		 * @param params parameter set by call getStatusInfo from main method
		 * @param pos position struct see pos_t
		 * @param elapsed seconds be elapsed since last position time pos_t.time
		 * @param time from last position pos_t.time converted in an string
		 */
		virtual string getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime);
		/**
		 * last running mehod of thread,
		 * if starter thread ending this thred
		 */
		virtual void ending();

	private:
		/**
		 * thread id of object
		 */
		//pid_t m_nthreadid;
		/**
		 * default connection ID set by creating object
		 * and will be the current ID if client set no other one
		 */
		unsigned int m_nDefaultID;
		/**
		 * mutex lock handle for have client
		 */
		pthread_mutex_t *m_HAVECLIENT;
		/**
		 * condition to waiting for client
		 */
		pthread_cond_t* m_CLIENTWAITCOND;
		/**
		 * whether communicator has any client
		 */
		bool m_bHasClient;
		/**
		 * whether the thread is only to send changes to client
		 */
		bool m_bSpeakerThread;
		/**
		 * FILE handle to client
		 */
		IFileDescriptorPattern* m_hFileAccess;
		/**
		 * whether client has an connection to server
		 * and has get an connection ID
		 */
		bool m_bConnected;
		/**
		 * whether user has access to server/application
		 */
		//bool m_bAccess;
		/**
		 * name of user whitch is connected
		 */
		string m_sUserName;
		/**
		 * Carage return for explicit operating system
		 * from client
		 */
		string m_sCR;
		/**
		 * whether varlible m_sCR is defined
		 */
		bool m_bSetCR;
		/**
		 * reference to starter thread of server
		 */
		StarterPattern* m_poStarter;
		/**
		 * first measure thread.<br />
		 * threads are defined in measure.conf to switch boolean or ports on LPT or COM,
		 * or measure on ports
		 */
		meash_t* m_pFirstMeasureThread;
		/**
		 * first folder name in an measure thread
		 */
		measurefolder_t *m_ptFolderStart;
		/**
		 * should server wait on client for more than one commands
		 */
		bool m_bWait;
		/**
		 * path of client subdirectory where the layout files
		 */
		string m_sClientRoot;

		//bool doConversation(FILE* fp, string input);
		/**
		 * whether one or more clients are connected
		 * with any communication thread
		 *
		 * @return true if an client is conected
		 */
		bool hasClients();
	};
}

//extern Communication* g_poFirstCommunication;

//Communication* getNextFreeCommunicationThread();
//Communication* searchNextFreeCommunicationThread();

#endif /*COMMUNICATION_H_*/
