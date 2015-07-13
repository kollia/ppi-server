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

#include "../../../util/structures.h"
#include "../../../util/thread/Thread.h"

#include "../../../pattern/server/IServerCommunicationStarterPattern.h"
#include "../../../pattern/server/IFileDescriptorPattern.h"
#include "../../../pattern/server/ICommunicationPattern.h"

using namespace design_pattern_world::server_pattern;

typedef IServerCommunicationStarterPattern StarterPattern;

namespace server
{
	/**
	 * representing communication threads
	 * which are waiting for client how connecting on server.
	 */
	class Communication :			public Thread,
							virtual public ICommunicationPattern
	{
	public:
		/**
		 * initial communication thread
		 *
		 * @param ID actual new connection id of communication
		 * @param starter thread which start communication threads
		 */
		Communication(unsigned int ID, const StarterPattern* pStarter);
		/**
		 * return actual file descriptor
		 *
		 * @return descriptor
		 */
		virtual const SHAREDPTR::shared_ptr<IFileDescriptorPattern> getDescriptor() const
		{ return m_hFileAccess; };
		/**
		 * set next communication object
		 *
		 * @param nextcomm new communication object
		 */
		virtual void setNextComm(ICommunicationPattern* nextcomm)
		{ m_pnext= nextcomm; };
		/**
		 * get next communication object
		 *
		 * @return next communication object
		 */
		virtual ICommunicationPattern* getNextComm() const
		{ return m_pnext; };
		/**
		 * commit an connection to an client.<br/>
		 * By Cancel this Communication object or ending an connection, the parameter access will be also delete
		 *
		 * @param access file descriptor whitch get from  IServerConnectArtPattern by listen
		 */
		virtual void connection(SHAREDPTR::shared_ptr<IFileDescriptorPattern>& access);
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool bWait)
		{ return stop(&bWait); };
		/**
		 *  external command to stop thread
		 *
		 * @param bWait calling rutine should wait until the thread is stopping
		 * @return object of error handling
		 */
		OVERWRITE EHObj stop(const bool *bWait= NULL);
		/**
		 * returning the default communication ID
		 * wich needet by starting an new connection.
		 * Set by creating
		 *
		 * @return default client ID
		 */
		virtual unsigned int getDefaultID() const
		{ return m_nDefaultID; };
		/**
		 * returning the actual communication ID of the thread
		 *
		 * @return communication ID
		 */
		virtual unsigned int getConnectionID() const;
		/**
		 * returning name of transaction
		 *
		 * @return name
		 */
		virtual string getTransactionName() const
		{ return m_hFileAccess.get() ? m_hFileAccess->getTransactionName() : ""; };
		/**
		 * whether an client is connected
		 *
		 * @return true if an client is conected
		 */
		virtual bool hasClient() const;
		/**
		 * search whether client with given defined name
		 * is the correct one
		 *
		 * @param process name of process in which client running
		 * @param client defined name to find client
		 * @return whether client is correct with given definition
		 */
		virtual bool isClient(const string& process, const string& client) const;
		/**
		 * send string to actual <code>ITransferPattern</code>
		 *
		 * @param str string which shold send to client
		 * @param wait whether method should wait for an answer
		 * @param endString if sending client want an array, this is the last string for ending
		 * @return answer from client
		 */
		virtual vector<string> sendString(const IMethodStringStream& str, const bool& wait, const string& endString)
		{ return m_hFileAccess->sendString(str, wait, endString); };
		/**
		 * read setting answers from last question, when an end string be defined
		 *
		 * @param syncID string which should be sending
		 * @param endString string for ending by read an array
		 * @return answer from other client
		 */
		virtual vector<string> getMoreAnswers(const unsigned long long syncID, const string& endString)
		{ return m_hFileAccess->getMoreAnswers(syncID, endString); };
		/**
		 * destroy instance of communication thread
		 */
		virtual ~Communication();

	protected:
		/**
		 * initial incomming variables from start method.<br />
		 * first running method of thread on starting
		 *
		 * @param args user defined parameter value or array,<br />
		 * 				coming as void pointer from the external call
		 * 				method start(void *args).
		 * @return object of error handling
		 */
		OVERWRITE EHObj init(void *args);
		/**
		 * This method starting again when ending with code 0 or lower for warnings
		 * and if the method stop() isn't called.
		 *
		 * @param whether should start thread again
		 */
		OVERWRITE bool execute();
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
		 * next Communication object
		 */
		ICommunicationPattern* m_pnext;
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
		SHAREDPTR::shared_ptr<IFileDescriptorPattern> m_hFileAccess;
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
		const StarterPattern* m_poStarter;
		/**
		 * should server wait on client for more than one commands
		 */
		bool m_bWait;

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
