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
#include "../pattern/server/ICommunicationPattern.h"

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
	class CommunicationThreadStarter :	virtual	public Thread,
										virtual public IServerCommunicationStarterPattern
	{
	public:
		/**
		 * constructor to create object
		 * from CommunicationThreadStarter
		 *
		 * @param minThreads how much communication threads should be started at least.<br /> If this value is <code>NULL</code> the CommunicationThreadStarter starts no thread to control how much communication threads are running
		 * @param maxThreads how much communication threads should be running at most.<br />This count will be starts on beginning
		 */
		CommunicationThreadStarter(const unsigned short& minThreads, const unsigned short& maxThreads);
		/**
		 * constructor to create object
		 * of CommunicationThreadStarter
		 *
		 * @param threadName new name of thread
		 * @param minThreads how much communication threads should be started at least.<br /> If this value is <code>NULL</code> the CommunicationThreadStarter starts no thread to control how much communication threads are running
		 * @param maxThreads how much communication threads should be running at most.<br />This count will be starts on beginning
		 */
		CommunicationThreadStarter(const string &threadName, const unsigned short& minThreads, const unsigned short& maxThreads);
		/**
		 * start method to running the thread parallel only if minThreads in constructor was greater than 0.<br />
		 * Elswhere the object only running the <code>init()</code> method and is present for the server to set an new connection
		 * into an communication thread.
		 *
		 * @param args arbitrary optional defined parameter to get in initialization method init
		 * @param bHold should the caller wait of thread by ending.<br />
		 * 				default is false
		 */
		virtual int start(void *args= NULL, bool bHold= false);
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
		virtual void setNewClient(IFileDescriptorPattern* descriptor);
		/**
		 * arouse condition NEXTCOMMUNICATIONCOND
		 * to search again for needed new communication threads
		 */
		virtual void arouseStarterThread() const;
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
		virtual short hasClients() const;
		/**
		 * search client with given defined name
		 * and return this client
		 *
		 * @param definition defined name to find client
		 * @param own pointer of own called descriptor witch client is not needed
		 * @return return client
		 */
		virtual IClientPattern* getClient(const string& definition, IFileDescriptorPattern* own) const;
		/**
		 * send stop to all communication threads
		 *
		 * @param bWait if stopping process should wait for ending and also by true communication-threads will be deleted
		 * @param transactionName whether method is called from an transaction, there should be given the transaction name for no wait for this transaction. To get no dead lock
		 */
		void stopCommunicationThreads(bool bWait= false, string transactionName= "");
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool bWait)
		{ return CommunicationThreadStarter::stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 */
		virtual int stop(const bool *bWait= NULL);
		/**
		 * destructor of CommunicationStarter
		 */
		virtual ~CommunicationThreadStarter();

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
		 * first Communication thread object
		 */
		ICommunicationPattern* m_poFirstCommunication;
		/**
		 * next free Communication thread object without any client
		 */
		ICommunicationPattern* m_poNextFree;
		/**
		 * mutex lock for write or read next communication thread
		 */
		pthread_mutex_t* m_NEXTCOMMUNICATION;
		/**
		 * condition to for waiting new Connection
		 */
		pthread_cond_t* m_NEXTCOMMUNICATIONCOND;

		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param error code for not correctly done
		 */
		virtual int execute();
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
		 * return next free ID
		 *
		 * @return next free ID
		 */
		unsigned int getNextFreeID()
		{ return m_nNextFreeID; };
		/**
		 * create new communication object and return them
		 *
		 * @param nextID next default ID for communication object
		 * @return new communication object
		 */
		virtual ICommunicationPattern* getNewCommunicationThread(unsigned int nextID) const;
		/**
		 * sort recursivly all communication thrads<br />
		 * and look whether enough or too much threads exists
		 *
		 * @param first beginning sorting on this first communication thread
		 */
		void checkThreads(ICommunicationPattern* first);
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
		const unsigned short m_maxConnThreads;
		/**
		 * how much communication threads should be minimal open
		 */
		const unsigned short m_minConnThreads;
		/**
		 * new FileDescriptor pool filled from ServerThread
		 * if no NextFree communication-thread be set
		 */
		queue<IFileDescriptorPattern*> m_qpNewClients;

	};

}

#endif /*COMMUNICATIONTHREADSTARTER_H_*/
