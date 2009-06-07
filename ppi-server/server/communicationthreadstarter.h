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
#ifndef COMMUNICATIONTHREADSTARTER_H_
#define COMMUNICATIONTHREADSTARTER_H_

#include <queue>

#include "../util/structures.h"
#include "../util/Thread.h"

#include "../ports/measureThread.h"

#include "../pattern/util/ipropertypattern.h"
#include "../pattern/server/IServerCommunicationStarterPattern.h"
#include "../pattern/server/IFileDescriptorPattern.h"

#include "ServerThread.h"
#include "Communication.h"


using namespace design_pattern_world::server_pattern;

namespace server
{

	/**
	 * objects starting and stopping communicationthreads
	 * and cloase also the remaining holes of communication ID's
	 *
	 * @author Alexander Kolli
	 * @version 1.0
	 */
	class CommunicationThreadStarter : public Thread, IServerCommunicationStarterPattern
	{
	public:
		/**
		 * creating new instance
		 *
		 * @param measureThread first object of measure thread
		 * @param folderStart first struct of folder to reache all subroutines
		 * @param properties <code>ConfigPropertyCasher</code> object from reading server.conf
		 * @param defaultSleep sleeping for default time in microseconds
		 */
		static bool initial(IServerConnectArtPattern* connect, serverArg_t* serverArg, useconds_t defaultSleep);
		/**
		 * abstract method to initial the thread
		 * in the extended class.<br />
		 * this method will be called before running
		 * the method execute
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				comming as void pointer from the external call
		 * 				method start(void *args).
		 * @return boolean whether the method execute can start
		 */
		virtual bool init(void *args);
		/**
		 * return single instance of CommunicationThreadStarter
		 * or NULL if not initialiciced
		 *
		 * @return instance
		 */
		static CommunicationThreadStarter* instance()
				{ return _instance; };
		/**
		 * function locate the next free client id
		 *
		 * @return next free client id
		 */
		virtual unsigned int nextClientID();
		/**
		 * set into free communication threads new connections from clients
		 * if any exists in NewClients pool
		 *
		 * @return whether all filling was done
		 */
		bool fillCommunicationThreads();
		/**
		 * fill an free communication thread with an new FileDescriptor
		 *
		 * @param descriptor new client connection FileDescriptor
		 */
		void setNewClient(IFileDescriptorPattern* descriptor);
		/**
		 * arouse condition NEXTCOMMUNICATIONCOND
		 * to search again for needed new communication threads
		 */
		virtual void arouseStarterThread();
		/**
		 * search the next communication thread with no client
		 *
		 * @return communication thread
		 */
		//Communication* searchNextFreeCommunicationThread();
		/**
		 * calculate how much communication-threads
		 * with or without clients running;
		 *
		 * @return count of threads
		 */
		short hasThreads();
		/**
		 * calculate how much threads has an communication with an client
		 *
		 * @return cout of clients
		 */
		short hasClients();
		/**
		 * send stop to all communication threads
		 *
		 * @param bWait if stopping process should wait for ending and also by true communication-threads will be deleted
		 */
		void stopCommunicationThreads(bool bWait= false);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void *stop(const bool bWait)
		{ return CommunicationThreadStarter::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual void *stop(const bool *bWait= NULL);

	protected:
		/**
		 * whether CommunicationThreadStarter will be stop in the next time
		 */
		bool m_bWillStop;
		/**
		 * next free communication ID for client
		 */
		unsigned int m_nNextFreeID;
		/**
		 * free communication ID's from clients which are deleted
		 * but be one or more ID's before the nNextFreeID
		 */
		vector<unsigned int> m_vFreeIDs;
		/**
		 * propertys from reading file server.conf
		 */
		IPropertyPattern* m_poProperties;
		/**
		 * first measure Thread
		 */
		meash_t* m_ptMeasureThread;
		/**
		 * first folder which need measureThread
		 * to reach all subroutines
		 */
		measurefolder_t* m_ptFirstFolder;
		/**
		 * all Communication objects
		 */
		//vector<Communication*> m_vpoCommunication;
		/**
		 * first Communication thread object
		 */
		Communication* m_poFirstCommunication;
		/**
		 * next free Communication thread object without any client
		 */
		Communication* m_poNextFree;
		/**
		 * mutex lock for write or read next communication thread
		 */
		pthread_mutex_t* m_NEXTCOMMUNICATION;
		/**
		 * condition to for waiting new Connection
		 */
		pthread_cond_t* m_NEXTCOMMUNICATIONCOND;

		/**
		 * abstract method to running thread
		 * in the extended class.<br />
		 * This method starting again when ending without an sleeptime
		 * if the method stop() isn't call.
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
		 * sort recursivly all communication thrads<br />
		 * and look whether enough or too much threads exists
		 *
		 * @param first beginning sorting on this first communication thread
		 */
		void checkThreads(Communication* first);
		/**
		 * abstract method to ending the thread.<br />
		 * This method will be called if any other or own thread
		 * calling method stop().
		 */
		virtual void ending();

	private:
		/**
		 * private single instance of own object
		 */
		static CommunicationThreadStarter* _instance;
		/**
		 * how much communication threads should be maximal open
		 */
		unsigned short m_maxConnThreads;
		/**
		 * how much communication threads should be minimal open
		 */
		unsigned short m_minConnThreads;
		/**
		 * path where all htm layout files for the client to display laying
		 */
		string m_sClientPath;
		/**
		 * new FileDescriptor pool filled from ServerThread
		 * if no NextFree communication-thread be set
		 */
		queue<IFileDescriptorPattern*> m_qpNewClients;

		/**
		 * constructor to create private object
		 * of CommunicationThreadStarter
		 *
		 * @param measureThread first object of measure thread
		 * @param folderStart first struct of folder to reache all subroutines
		 * @param properties <code>ConfigPropertyCasher</code> object from reading server.conf
		 * @param defaultSleep sleeping for default time in microseconds
		 */
		CommunicationThreadStarter(IServerConnectArtPattern* connect, serverArg_t* serverArg, useconds_t defaultSleep);
		/**
		 * destructor of CommunicationStarter
		 */
		virtual ~CommunicationThreadStarter();
	};

}

#endif /*COMMUNICATIONTHREADSTARTER_H_*/
