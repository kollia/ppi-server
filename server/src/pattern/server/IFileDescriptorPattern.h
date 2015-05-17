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
#ifndef IFILEDESCRIPTORPATTERN_H_
#define IFILEDESCRIPTORPATTERN_H_

#include <string>

#include "IServerPattern.h"

#include "../../util/stream/IMethodStringStream.h"

#include "../util/IErrorHandlingPattern.h"

using namespace std;
using namespace util;

namespace design_pattern_world
{
	using namespace util_pattern;

	namespace server_pattern
	{
		/**
		 * abstract interface pattern for initial file descriptor
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0
		 */
		class IFileDescriptorPattern
		{
			public:
				/**
				 * initial object of IFileDiscriptorPattern
				 *
				 * @return object of error handling
				 */
				virtual EHObj init()= 0;
				/**
				 * operator value write string into transaction
				 * for sending
				 *
				 * @param reader contains after call the value
				 */
				virtual void operator >> (string &reader)= 0;
				/**
				 * operator value over the transaction
				 *
				 * @param writer write value to transaction file
				 */
				virtual void operator << (const string& writer)= 0;
				/**
				 * sending request to client or server
				 */
				virtual void flush()= 0;
				/**
				 * whether descriptor is defined for automatic
				 * sending by every carriage return.<br />
				 * default is true
				 *
				 * @param automatic whether should be defined for automatic sending
				 */
				virtual void flushing(bool automatic)= 0;
				/**
				 * creating an carrage return with flush
				 */
				virtual void endl()= 0;
				/**
				 * whether other part of connection
				 * will be lost or connection end
				 * with an error
				 *
				 * @return end of file reached
				 */
				virtual bool eof() const= 0;
				/**
				 * whether current object has an error or warning
				 *
				 * @return whether error exist
				 */
				virtual bool fail() const= 0;
				/**
				 * whether current object has an error
				 *
				 * @return whether error exist
				 */
				virtual bool hasError() const= 0;
				/**
				 * whether current object has an warning
				 *
				 * @return whether warning exist
				 */
				virtual bool hasWarning() const= 0;
				/**
				 * return error / warning object
				 *
				 * @return error handling object
				 */
				virtual EHObj getErrorObj() const= 0;
				/**
				 * returning error or warning description
				 * form current object
				 *
				 * @return error description
				 */
				virtual string getErrorDescription() const= 0;
				/**
				 * get name of host address from connected part (client or server)
				 *
				 * @return name of host address
				 */
				virtual string getHostAddressName() const= 0;
				/**
				 * returning name of transaction
				 *
				 * @return name
				 */
				virtual string getTransactionName() const= 0;
				/**
				 * get actual port number
				 *
				 * @return port number
				 */
				virtual short getPort() const= 0;
				/**
				 * transfer answer from geted string to client
				 *
				 * @return whether server should hold transaction
				 */
				virtual bool transfer()= 0;
				/**
				 * unlock mutex of THREADSAVEMETHODS if transfer wait for any longer time.<br />
				 * Do not unlock before getting stream from descriptor, because this operation
				 * do also unlock this mutex.
				 */
				virtual void unlock()= 0;
				/**
				 * lock mutex of THREADSAVEMETHODS.<br />
				 * If you have unlock this mutex, lock it again for any getting stream from descriptor,
				 * or ending the transfer method.
				 */
				virtual void lock()= 0;
				/**
				 * set boolean into object
				 *
				 * @param str name of boolean
				 * @param boolean value of boolean
				 */
				virtual void setBoolean(const string& str, const bool boolean)= 0;
				/**
				 * read value of boolean
				 *
				 * @param str name of boolean
				 * @return value of boolean
				 */
				virtual bool getBoolean(const string& str) const= 0;
				/**
				 * set short into object
				 *
				 * @param str name of short
				 * @param value value of short
				 */
				virtual void setShort(const string& str, const short value)= 0;
				/**
				 * read value of short
				 *
				 * @param str name of short
				 * @return value of short
				 */
				virtual short getShort(const string& str) const= 0;
				/**
				 * set unsigned short into object
				 *
				 * @param str name of unsigned short
				 * @param value value of unsigned short
				 */
				virtual void setUShort(const string& str, const unsigned short value)= 0;
				/**
				 * read value of unsigned short
				 *
				 * @param str name of unsigned short
				 * @return value of unsigned short
				 */
				virtual unsigned short getUShort(const string& str) const= 0;
				/**
				 * set integer into object
				 *
				 * @param str name of integer
				 * @param value value of integer
				 */
				virtual void setInt(const string& str, const int value)= 0;
				/**
				 * read value of integer
				 *
				 * @param str name of integer
				 * @return value of integer
				 */
				virtual int getInt(const string& str) const= 0;
				/**
				 * set unsigned integer into object
				 *
				 * @param str name of unsigned integer
				 * @param value value of unsigned integer
				 */
				virtual void setUInt(const string& str, const unsigned int value)= 0;
				/**
				 * read value of unsigned integer
				 *
				 * @param str name of unsigned integer
				 * @return value of unsigned integer
				 */
				virtual unsigned int getUInt(const string& str) const= 0;
				/**
				 * read value of unsigned long integer
				 *
				 * @param str name of unsigned long integer
				 * @return value of unsigned long integer
				 */
				virtual unsigned long getULong(const string& str) const= 0;
				/**
				 * read value of unsigned double long integer
				 *
				 * @param str name of unsigned double long integer
				 * @return value of unsigned long integer
				 */
				virtual unsigned long long getULongLong(const string& str) const= 0;
				/**
				 * set unsigned long integer into object
				 *
				 * @param str name of unsigned long integer
				 * @param value value of unsigned long integer
				 */
				virtual void setULong(const string& str, const unsigned long value)= 0;
				/**
				 * set unsigned double long integer into object
				 *
				 * @param str name of unsigned double long integer
				 * @param value value of unsigned double long integer
				 */
				virtual void setULongLong(const string& str, const unsigned long long value)= 0;
				/**
				 * set float into object
				 *
				 * @param str name of float
				 * @param value value of float
				 */
				virtual void setFloat(const string& str, const float value)= 0;
				/**
				 * read value of float
				 *
				 * @param str name of float
				 * @return value of float
				 */
				virtual float getFloat(const string& str) const= 0;
				/**
				 * set double into object
				 *
				 * @param str name of double
				 * @param value value of double
				 */
				virtual void setDouble(const string& str, const double value)= 0;
				/**
				 * read value of double
				 *
				 * @param str name of double
				 * @return value of double
				 */
				virtual double getDouble(const string& str) const= 0;
				/**
				 * set string into object
				 *
				 * @param str name of variable
				 * @param value value of defiend string
				 */
				virtual void setString(const string& str, const string& value)= 0;
				/**
				 * read actual value of string
				 *
				 * @param str name of string variable
				 * @return value of defined name
				 */
				virtual string getString(const string& str) const= 0;
				/**
				 * get current client ID
				 *
				 * @return ID
				 */
				virtual unsigned int getClientID() const= 0;
				/**
				 * set ID for client
				 *
				 * @param ID new ID for client
				 */
				virtual void setClientID(unsigned int ID)= 0;
				/**
				 * search whether client with given defined name
				 * is the correct one
				 *
				 * @param definition defined name to find client
				 * @return whether client is correct with given definition
				 */
				virtual bool isClient(const string& definition) const= 0;
				/**
				 * send string to actual <code>ITransferPattern</code>
				 *
				 * @param str string which should send to client
				 * @param wait whether method should wait for an answer
				 * @param endString if sending client want an array, this is the last string for ending
				 * @return answer from client
				 */
				virtual vector<string> sendString(const IMethodStringStream& str, const bool& wait, const string& endString)= 0;
				/**
				 * method ask for string from other client
				 *
				 * @param doWait whether sending client wait for answer
				 * @param endString if sending client wait for an anser array, this is the last string for ending
				 * @param wait whether method should wait if no string was sending (default: true)
				 * @return string from other client
				 */
				virtual string getOtherClientString(bool& doWait, string& endString, const bool wait= true)= 0;
				/**
				 * send an answer of getting string with <code>getOtherClientString()</code>
				 *
				 * @param asw answer of getting string
				 */
				virtual void sendAnswer(const vector<string>& asw)= 0;
				/**
				 * send string to other client with defined definition name
				 * and return answer
				 *
				 * @param definition defined name from other client
				 * @param str string which should be sending
				 * @param wait whether method should wait for an answer
				 * @param endString string for ending by read an array
				 * @return answer from other client
				 */
				virtual vector<string> sendToOtherClient(const string& definition, const IMethodStringStream& str, const bool& wait, const string& endString)= 0;
				/**
				 * read setting answers from other client, when an end string be defined
				 *
				 * @param syncID string which should be sending
				 * @param endString string for ending by read an array
				 * @return answer from other client
				 */
				virtual vector<string> getMoreFromOtherClient(const unsigned long long syncID, const string& endString)= 0;
				/**
				 * read setting answers from last question, when an end string be defined
				 *
				 * @param syncID string which should be sending
				 * @param endString string for ending by read an array
				 * @return answer from other client
				 */
				virtual vector<string> getMoreAnswers(const unsigned long long syncID, const string& endString)= 0;
				/**
				 * return factory of ServerCommunicationStarter
				 *
				 * @return actual ServerCommunicationStarter
				 */
				virtual IServerPattern* getServerObject() const= 0;
				/**
				 * close connection to client
				 */
				virtual void closeConnection()= 0;
				/**
				 * destructor to dereference file
				 */
				virtual ~IFileDescriptorPattern() {};
		};
	}
}

#endif /*IFILEDESCRIPTORPATTERN_H_*/
