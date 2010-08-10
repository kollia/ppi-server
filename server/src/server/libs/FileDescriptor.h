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

#include "../../pattern/server/IFileDescriptorPattern.h"
#include "../../pattern/server/IClientConnectArtPattern.h"

#include "../../util/thread/Thread.h"

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
			 * operator value write string into transaction
			 * when line be ending (with carriage return) or by flush command.
			 *
			 * @param writer write value to transaction file
			 */
			virtual void operator << (const string& writer);
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
			virtual bool eof() const;
			/**
			 * test file handle of error
			 */
			virtual int error() const;
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
			 * unlock mutex of THREADSAVEMETHODS if transfer wait for any longer time.<br />
			 * Do not unlock before getting stream from descriptor, because this operation
			 * do also unlock this mutex.
			 */
			virtual void unlock()
			{ UNLOCK(m_THREADSAVEMETHODS); };
			/**
			 * lock mutex of THREADSAVEMETHODS.<br />
			 * If you have unlock this mutex, lock it again for any getting stream from descriptor,
			 * or ending the transfer method.
			 */
			virtual void lock()
			{ LOCK(m_THREADSAVEMETHODS); }
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
			 * set short into object
			 *
			 * @param str name of short
			 * @param value value of short
			 */
			virtual void setShort(const string& str, const short value);
			/**
			 * read value of short
			 *
			 * @param str name of short
			 * @return value of short
			 */
			virtual short getShort(const string& str) const;
			/**
			 * set unsigned short into object
			 *
			 * @param str name of unsigned short
			 * @param value value of unsigned short
			 */
			virtual void setUShort(const string& str, const unsigned short value);
			/**
			 * read value of unsigned short
			 *
			 * @param str name of unsigned short
			 * @return value of unsigned short
			 */
			virtual unsigned short getUShort(const string& str) const;
			/**
			 * set integer into object
			 *
			 * @param str name of integer
			 * @param value value of integer
			 */
			virtual void setInt(const string& str, const int value);
			/**
			 * read value of integer
			 *
			 * @param str name of integer
			 * @return value of integer
			 */
			virtual int getInt(const string& str) const;
			/**
			 * set unsigned integer into object
			 *
			 * @param str name of unsigned integer
			 * @param value value of unsigned integer
			 */
			virtual void setUInt(const string& str, const unsigned int value);
			/**
			 * read value of unsigned integer
			 *
			 * @param str name of unsigned integer
			 * @return value of unsigned integer
			 */
			virtual unsigned int getUInt(const string& str) const;
			/**
			 * set float into object
			 *
			 * @param str name of float
			 * @param value value of float
			 */
			virtual void setFloat(const string& str, const float value);
			/**
			 * read value of float
			 *
			 * @param str name of float
			 * @return value of float
			 */
			virtual float getFloat(const string& str) const;
			/**
			 * set double into object
			 *
			 * @param str name of double
			 * @param value value of double
			 */
			virtual void setDouble(const string& str, const double value);
			/**
			 * read value of double
			 *
			 * @param str name of double
			 * @return value of double
			 */
			virtual double getDouble(const string& str) const;
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
			 * close connection to client
			 */
			virtual void closeConnection();
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error) const;
			/**
			 * get maximal error or warning number in positive values from own class
			 *
			 * @param byerror whether needs error number (true) or warning number (false)
			 * @return maximal error or warning number
			 */
			virtual unsigned int getMaxErrorNums(const bool byerror) const;
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
			 * whether object have an opened file handle
			 */
			bool m_bFileAccess;
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
			 * signal end of file.
			 */
			mutable bool m_bEOF;
			/**
			 * holds error number
			 */
			mutable int m_nErr;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, bool> m_mBoolean;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, short> m_mShort;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, unsigned short> m_mUShort;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, int> m_mInt;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, unsigned int> m_mUInt;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, long> m_mLong;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, unsigned long> m_mULong;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, float> m_mFloat;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, double> m_mDouble;
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
