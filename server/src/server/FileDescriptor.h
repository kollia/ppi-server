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
#ifndef FILEDESCRIPTOR_H_
#define FILEDESCRIPTOR_H_

#include <string>
#include <queue>
#include <map>

#include "../pattern/server/IFileDescriptorPattern.h"
#include "../pattern/server/IClientConnectArtPattern.h"

using namespace std;
using namespace design_pattern_world::server_pattern;


namespace server
{

	/**
	 * description of file to writing or reading
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class FileDescriptor : public IFileDescriptorPattern
	{
		public:
			/**
			 * stadard constructor to forwarding
			 */
			FileDescriptor()
			:	m_nTimeout(0)
			{ initial(NULL, NULL, NULL, "", 0); };
			/**
			 * constructor to initial FILE and current host address
			 *
			 * @param starter factory of all produced clients
			 * @param art object of holding transaction to client or server
			 * @param file FILE descriptor
			 * @param address incoming host address
			 * @param timeout waiting seconds if no second client thread waiting for answers
			 */
			FileDescriptor(IServerPattern* server, ITransferPattern* transfer, FILE* file, string address,
					const unsigned short port, const unsigned int timeout)
			:	m_nTimeout(timeout)
			{ initial(server, transfer, file, address, port); };
			/**
			 * initial ITransferPattern with this FileDescriptor
			 *
			 * @return whether the initialization was correct
			 */
			virtual bool init();
			/**
			 * operator writing string value into the param,
			 * witch coming over the transaction
			 *
			 * @param reader contains after call the value
			 */
			virtual void operator >> (string &reader);
			/**
			 * operator value over the transaction
			 *
			 * @param writer write value to transaction file
			 */
			virtual void operator << (const string writer);
			/**
			 * creating an carriage return with flush
			 */
			virtual void endl();
			/**
			 * flush the string inside to client
			 */
			virtual void flush();
			/**
			 * whether end of file is reached
			 *
			 * @return end of file reached
			 */
			virtual bool eof();
			/**
			 * get name of host address from client
			 *
			 * @return name of host address
			 */
			virtual string getHostAddressName() const;
			/**
			 * transfer answer from getting string to client.<br />
			 * Method is thread save.
			 *
			 * @return whether server should hold transaction
			 */
			virtual bool transfer();
			/**
			 * set boolean into object
			 *
			 * @param str name of boolean
			 * @param boolean value of boolean
			 */
			virtual void setBoolean(const string& str, const bool boolean);
			/**
			 * read value of boolean
			 *
			 * @param str name of boolean
			 * @return value of boolean
			 */
			virtual bool getBoolean(const string& str) const;
			/**
			 * set string into object
			 *
			 * @param str name of variable
			 * @param value value of defined string
			 */
			virtual void setString(const string& str, const string& value);
			/**
			 * read actual value of string
			 *
			 * @param str name of string variable
			 * @return value of defined name
			 */
			virtual string getString(const string& str) const;
			/**
			 * get actual port number
			 *
			 * @return port number
			 */
			virtual short getPort() const;
			/**
			 * get current client ID
			 *
			 * @return ID
			 */
			virtual unsigned int getClientID() const;
			/**
			 * method ask for string from other client
			 *
			 * @param wait whether method should wait if no string was sending (default: true)
			 * @return string from other client
			 */
			virtual string getOtherClientString(const bool wait= true);
			/**
			 * send string to other client with defined definition name
			 *
			 * @param definition defined name from other client
			 * @param str string which should be sending
			 * @param wait whether method should wait for an answer
			 * @return answer from other client
			 */
			virtual string sendToOtherClient(const string& definition, const string& str, const bool& wait);
			/**
			 * send an answer of getting string with <code>getOtherClientString()</code>
			 *
			 * @param asw answer of getting string
			 */
			virtual void sendAnswer(const string& asw);
			/**
			 * set ID for client
			 *
			 * @param ID new ID for client
			 */
			virtual void setClientID(unsigned int ID);
			/**
			 * search whether client with given defined name
			 * is the correct one.<br />
			 * Method is thread save.
			 *
			 * @param definition defined name to find client
			 * @return whether client is correct with given definition
			 */
			virtual bool isClient(const string& definition) const;
			/**
			 * returning name of transaction.<br />
			 * Method is thread save.
			 *
			 * @return name
			 */
			virtual string getTransactionName() const;
			/**
			 * return factory of ServerCommunicationStarter
			 *
			 * @return actual ServerCommunicationStarter
			 */
			virtual IServerPattern* getServerObject() const
			{ return m_poServer; };
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error) const;
			/**
			 * destructor to dereference file
			 */
			virtual ~FileDescriptor();

		private:
			/**
			 * send string to actual <code>ITransferPattern</code>
			 *
			 * @param str string which should send to client
			 * @param wait whether method should wait for an answer
			 * @return answer from client
			 */
			virtual string sendString(const string& str, const bool& wait);

			/**
			 * mutex lock handle for changing or reading connection ID
			 */
			pthread_mutex_t *m_CONNECTIONIDACCESS;
			/**
			 * mutex lock handle for string from other client
			 */
			pthread_mutex_t *m_SENDSTRING;
			/**
			 * mutex lock for thread save methods between transfer, isClient and getTransferName
			 */
			pthread_mutex_t *m_THREADSAVEMETHODS;
			/**
			 * condition handle to wait for answer while was sending an string from an other client
			 */
			pthread_cond_t *m_SENDSTRINGCONDITION;
			/**
			 * condition handle to wait for an string from an other client
			 */
			pthread_cond_t *m_GETSTRINGCONDITION;
			/**
			 * transaction from server to client or backward
			 */
			ITransferPattern* m_pTransfer;
			/**
			 * factory of all produced clients
			 */
			IServerPattern* m_poServer;
			/**
			 * connection ID of communicattion thread.<br />
			 * This ID can be chanced, if the client have more then one
			 * connection, to all other conncetion ID of communication threads
			 */
			unsigned int m_unConnID;
			/**
			 * whether client has access to server
			 */
			bool m_bAccess;
			/**
			 * address from witch incomming values be
			 */
			string m_sAddress;
			/**
			 * number of port
			 */
			unsigned short m_nPort;
			/**
			 * file handle to connection over IP
			 */
			FILE* m_pFile;
			/**
			 * signal end of file.<br />
			 * Value is an non-negative number, otherwise EOF
			 */
			int m_nEOF;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, bool> m_mBoolean;
			/**
			 * string values set from ITransferPattern object
			 */
			map<string, string> m_mString;
			/**
			 * string sending from an other client
			 */
			string m_sSendString;
			/**
			 * string sending container from an other client,
			 * where the other clients do not wait for answer
			 */
			queue<string> m_qsSendStrings;
			/**
			 * answer string to return to other client
			 */
			string m_sClientAnswer;
			/**
			 * timeout in seconds for waiting if no second client thread
			 * waiting for answers
			 */
			const unsigned int m_nTimeout;

			/**
			 * constructor initialization for object
			 *
			 * @param starter factory of all produced clients
			 * @param art object of holding transaction to client or server
			 * @param file FILE descriptor
			 * @param address incoming host address
			 * @param timeout waiting seconds if no second client thread waiting for answers
			 */
			void initial(IServerPattern* server, ITransferPattern* transfer, FILE* file, string address,
					const unsigned short port);
	};

}

#endif /*FILEDESCRIPTOR_H_*/
