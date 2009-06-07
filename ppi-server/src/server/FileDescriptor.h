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
			FileDescriptor();
			/**
			 * constructor to initial FILE and current host address
			 *
			 * @param art object of holding transaction to client or server
			 * @param file FILE descriptor
			 * @param address incomming host address
			 */
			FileDescriptor(ITransferPattern* transfer, FILE* file, string address, unsigned short port);
			/**
			 * initial ITransferPattern with this FileDescriptor
			 *
			 * @return whether the initialization was correct
			 */
			virtual bool init();
			/**
			 * operator writing string value into the param,
			 * witch comming over the transaction
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
			 * creating an carrage return with flush
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
			virtual string getHostAddressName();
			/**
			 * transfer answer from geted string to client
			 *
			 * @return whether server should hold transaction
			 */
			virtual bool transfer();
			/**
			 * whether client has correct access to server
			 *
			 * @return has access
			 */
			//virtual bool hasAccess();
			/**
			 * set true access if client have access to server
			 */
			//virtual void setAccess(bool access= true);
			/**
			 * set boolean into object
			 *
			 * @param str name of boolean
			 * @param boolean value of boolean
			 */
			virtual void setBoolean(string str, bool boolean);
			/**
			 * read value of boolean
			 *
			 * @param str name of boolean
			 * @return value of boolean
			 */
			virtual bool getBoolean(string str);
			/**
			 * set string into object
			 *
			 * @param str name of variable
			 * @param value value of defiend string
			 */
			virtual void setString(string str, string value);
			/**
			 * read actual value of string
			 *
			 * @param str name of string variable
			 * @return value of defined name
			 */
			virtual string getString(string str);
			/**
			 * get actual port number
			 *
			 * @return port number
			 */
			virtual short getPort();
			/**
			 * get current client ID
			 *
			 * @return ID
			 */
			virtual unsigned int getClientID();
			/**
			 * set ID for client
			 *
			 * @param ID new ID for client
			 */
			virtual void setClientID(unsigned int ID);
			/**
			 * destructor to dereference file
			 */
			virtual ~FileDescriptor();

		private:
			/**
			 * mutex lock handle for changing or reading connection ID
			 */
			pthread_mutex_t *m_CONNECTIONIDACCESS;
			/**
			 * transaction from server to client or backward
			 */
			ITransferPattern* m_pTransfer;
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
	};

}

#endif /*FILEDESCRIPTOR_H_*/
