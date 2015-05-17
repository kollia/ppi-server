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

#include "../../pattern/util/IErrorHandlingPattern.h"
#include "../../pattern/server/IFileDescriptorPattern.h"

/**
 * make debug check for
 * last writing and reading
 * when definition is 1
 */
#define __DEBUGLASTREADWRITECHECK 0

#include "../../util/smart_ptr.h"
#include "../../util/thread/Thread.h"
#include "../../util/stream/IMethodStringStream.h"
#if(__DEBUGLASTREADWRITECHECK)
#include "../../util/stream/ppivalues.h"
#endif // __DEBUGLASTREADWRITECHECK

using namespace std;
using namespace util;
using namespace design_pattern_world::util_pattern;
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
#if (__DEBUGLASTREADWRITECHECK)
		/**
		 * structure of last reading and writing content
		 */
		struct lastReadWrite_t
		{
			deque<pair<ppi_time, string> > lastRead;
			deque<pair<ppi_time, string> > lastWrite;
		};
#endif // __DEBUGLASTREADWRITECHECK

			/**
			 * stadard constructor to forwarding
			 */
			FileDescriptor()
			:	m_nTimeout(0),
			 	m_bAutoSending(true)
			{ initial(NULL, NULL, 0, "", 0); };
			/**
			 * constructor to initial FILE and current host address
			 *
			 * @param starter factory of all produced clients
			 * @param art object of holding transaction to client or server
			 * @param file FILE descriptor
			 * @param address incoming host address
			 * @param timeout waiting seconds if no second client thread waiting for answers
			 */
			FileDescriptor(IServerPattern* server, ITransferPattern* transfer, int file, string address,
					const unsigned short port, const unsigned int timeout)
			:	m_nTimeout(timeout),
			 	m_bAutoSending(true)
			{ initial(server, transfer, file, address, port); };
#if (__DEBUGLASTREADWRITECHECK)
			/**
			 * define last writing for signal SIGHUP
			 */
			static void setWriting(const string& write);
			/**
			 * define last reading for signal SIGHUP
			 */
			static void setReading(const string& read);
#endif // __DEBUGLASTREADWRITECHECK

			/**
			 * initial ITransferPattern with this FileDescriptor
			 *
			 * @return object of error handling
			 */
			virtual EHObj init();
			/**
			 * operator writing string value into the param,
			 * witch coming over the transaction
			 *
			 * @param reader contains after call the value
			 */
			virtual void operator >> (string &reader);
			/**
			 * operator value write string into transaction
			 * for sending
			 *
			 * @param writer write value to transaction file
			 */
			virtual void operator << (const string& writer);
			/**
			 * sending request to client or server
			 */
			virtual void flush();
			/**
			 * whether descriptor is defined for automatic
			 * sending by every carriage return.<br />
			 * default is true
			 *
			 * @param automatic whether should be defined for automatic sending
			 */
			virtual void flushing(bool automatic)
			{ m_bAutoSending= automatic; };
			/**
			 * creating an carriage return with flush
			 */
			virtual void endl();
			/**
			 * whether other part of connection
			 * will be lost or connection end
			 * with an error
			 *
			 * @return end of file reached
			 */
			virtual bool eof() const;
			/**
			 * whether current object has an error or warning
			 *
			 * @return whether error exist
			 */
			virtual bool fail() const
			{ return m_oSocketError.fail(); };
			/**
			 * whether current object has an error
			 *
			 * @return whether error exist
			 */
			virtual bool hasError() const
			{ return m_oSocketError.hasError(); };
			/**
			 * whether current object has an warning
			 *
			 * @return whether warning exist
			 */
			virtual bool hasWarning() const
			{ return m_oSocketError.hasWarning(); };
			/**
			 * returning error or warning description
			 * form current object
			 *
			 * @return error description
			 */
			virtual string getErrorDescription() const
			{ return m_oSocketError.getDescription(); };
			/**
			 * return error / warning object
			 *
			 * @return error handling object
			 */
			virtual EHObj getErrorObj() const;
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
			 * set unsigned long integer into object
			 *
			 * @param str name of unsigned long integer
			 * @param value value of unsigned long integer
			 */
			virtual void setULong(const string& str, const unsigned long value);
			/**
			 * set unsigned double long integer into object
			 *
			 * @param str name of unsigned double long integer
			 * @param value value of unsigned double long integer
			 */
			virtual void setULongLong(const string& str, const unsigned long long value);
			/**
			 * read value of unsigned long integer
			 *
			 * @param str name of unsigned long integer
			 * @return value of unsigned long integer
			 */
			virtual unsigned long getULong(const string& str) const;
			/**
			 * read value of unsigned double long integer
			 *
			 * @param str name of unsigned double long integer
			 * @return value of unsigned long integer
			 */
			virtual unsigned long long getULongLong(const string& str) const;
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
			 * @param doWait whether sending client wait for answer
			 * @param endString if sending client wait for an anser array, this is the last string for ending
			 * @param wait whether method should wait if no string was sending (default: true)
			 * @return string from other client
			 */
			virtual string getOtherClientString(bool& doWait, string& endString, const bool wait= true);
			/**
			 * send string to other client with defined definition name
			 *
			 * @param definition defined name from other client
			 * @param str string which should be sending
			 * @param wait whether method should wait for an answer
			 * @param endString string for ending by read an array
			 * @return answer from other client
			 */
			virtual vector<string> sendToOtherClient(const string& definition, const IMethodStringStream& str, const bool& wait, const string& endString);
			/**
			 * read setting answers from other client, when an end string be defined
			 *
			 * @param syncID string which should be sending
			 * @param endString string for ending by read an array
			 * @return answer from other client
			 */
			virtual vector<string> getMoreFromOtherClient(const unsigned long long syncID, const string& endString);
			/**
			 * read setting answers from last question, when an end string be defined
			 *
			 * @param syncID string which should be sending
			 * @param endString string for ending by read an array
			 * @return answer from other client
			 */
			virtual vector<string> getMoreAnswers(const unsigned long long syncID, const string& endString);
			/**
			 * send an answer of getting string with <code>getOtherClientString()</code>
			 *
			 * @param asw answer of getting string
			 */
			virtual void sendAnswer(const vector<string>& asw);
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
			 * destructor to dereference file
			 */
			virtual ~FileDescriptor();

		protected:
			/**
			 * returning name of transaction.<br />
			 * Method is not thread save.
			 *
			 * @return name
			 */
			string getITransactionName() const;
			/**
			 * unlock THREADSAVEMETHODS when own thread has locked.
			 * this will be done because operator of shifting in ('<<')
			 * and out ('>>') can be done also from outside
			 * with no transfer object set
			 *
			 * @return whether lock was set from own thread
			 */
			bool unlockTHREADSAVEMETHODS(const string& file, int line);

		private:
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
			 * mutex lock for knowing
			 * which thread locking currently
			 * lock of THREADSAVEMETHODS
			 */
			pthread_mutex_t *m_LOCKSM;
			/**
			 * condition handle to wait for answer while was sending an string from an other client
			 */
			pthread_cond_t *m_SENDSTRINGCONDITION;
			/**
			 * condition handle to wait for an string from an other client
			 */
			pthread_cond_t *m_GETSTRINGCONDITION;

			/**
			 * which thread locking currently
			 * THREADSAVEMETHODS
			 */
			mutable pid_t m_nLockSM;
			/**
			 * transaction from server to client or backward
			 */
			ITransferPattern* m_pTransfer;
			/**
			 * factory of all produced clients
			 */
			IServerPattern* m_poServer;
			/**
			 * other hearing client by server communication
			 */
			IClientPattern* m_pHearingClient;
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
			int m_nFd;
			/**
			 * cache for last reading
			 * when found some characters
			 * after carriage return.
			 * split for every process and client
			 */
			//map<string, string> m_mLastRead;
			string m_mLastRead;
			/**
			 * holds error number
			 */
			SocketErrorHandling m_oSocketError;
			/**
			 * boolean values set from ITransferPattern object
			 */
			map<string, bool> m_mBoolean;
			/**
			 * short values set from ITransferPattern object
			 */
			map<string, short> m_mShort;
			/**
			 * unsigned short values set from ITransferPattern object
			 */
			map<string, unsigned short> m_mUShort;
			/**
			 * integer values set from ITransferPattern object
			 */
			map<string, int> m_mInt;
			/**
			 * unsigned integer values set from ITransferPattern object
			 */
			map<string, unsigned int> m_mUInt;
			/**
			 * long values set from ITransferPattern object
			 */
			map<string, long> m_mLong;
			/**
			 * unsigned long values set from ITransferPattern object
			 */
			map<string, unsigned long> m_mULong;
			/**
			 * unsigned double long values set from ITransferPattern object
			 */
			map<string, unsigned long long> m_mULongLong;
			/**
			 * float values set from ITransferPattern object
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
			 * Whether sending client wait for answer
			 */
			bool m_bWait;
			/**
			 * whether sending client want an array.<br />
			 * By this case, variable is an string for ending
			 */
			string m_sEndingString;
			/**
			 * whether sending hearing client get an array, but sending
			 * client do not wait.
			 */
			queue<string> m_qsEndingStrings;
			/**
			 * string sending container from an other client,
			 * where the other clients do not wait for answer
			 */
			queue<string> m_qsSendStrings;
			/**
			 * answer string to return to other client
			 */
			vector< SHAREDPTR::shared_ptr<IMethodStringStream> > m_vsClientAnswer;
			/**
			 * timeout in seconds for waiting if no second client thread
			 * waiting for answers
			 */
			const unsigned int m_nTimeout;
			/**
			 * whether descriptor is defined for automatic
			 * sending by every carriage return
			 */
			bool m_bAutoSending;
			/**
			 * transaction cash to sending over connection
			 */
			string m_sSendTransaction;
			/**
			 * string transaction from an other client
			 * inside an other thread
			 * (no transfer over an connection)
			 */
			string m_sSendString;
#if (__DEBUGLASTREADWRITECHECK)
			/**
			 * mutex lock to write last reading or writing
			 * from FileDescriptor
			 */
			static pthread_mutex_t* m_SIGNALLOCK;
#endif //__DEBUGLASTREADWRITECHECK

			/**
			 * constructor initialization for object
			 *
			 * @param starter factory of all produced clients
			 * @param art object of holding transaction to client or server
			 * @param file FILE descriptor
			 * @param address incoming host address
			 * @param timeout waiting seconds if no second client thread waiting for answers
			 */
			void initial(IServerPattern* server, ITransferPattern* transfer, int file, string address,
					const unsigned short port);
			/**
			 * search other client which hearing to give answer
			 *
			 * @param definition defined name from other client
			 * @return client
			 */
			IClientPattern* getOtherHearingClient(const string& definition);
			/**
			 * send string to actual <code>ITransferPattern</code>
			 *
			 * @param str string which should send to client
			 * @param wait whether method should wait for an answer
			 * @param endString if sending client want an array, this is the last string for ending
			 * @return answer from client
			 */
			virtual vector<string> sendString(const IMethodStringStream& str,
							const bool& wait, const string& endString);
#if (__DEBUGLASTREADWRITECHECK)
			/**
			 * map of last writing and reading content
			 * sorted by thread ID
			 */
			static map<pid_t, lastReadWrite_t> m_mLastReadWrite;
			/**
			 * signal converting for SIGHUP
			 */
			static void SIGHUPconverting(int nSignal);
#endif // __DEBUGLASTREADWRITECHECK
	};

}

#define UNLOCK_THREADSAVEMETHODS() unlockTHREADSAVEMETHODS(__FILE__, __LINE__)

#endif /*FILEDESCRIPTOR_H_*/
