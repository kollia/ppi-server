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

using namespace std;

namespace design_pattern_world
{
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
				 * @return whether initialization was correct
				 */
				virtual bool init()= 0;
				/**
				 * operator writing string value into the param,
				 * witch comming over the transaction
				 *
				 * @param reader contains after call the value
				 */
				virtual void operator >> (string &reader)= 0;
				/**
				 * operator value over the transaction
				 *
				 * @param writer write value to transaction file
				 */
				virtual void operator << (const string writer)= 0;
				/**
				 * creating an carrage return with flush
				 */
				virtual void endl()= 0;
				/**
				 * flush the string inside to client
				 */
				virtual void flush()= 0;
				/**
				 * whether end of file is reached
				 *
				 * @return end of file reached
				 */
				virtual bool eof()= 0;
				/**
				 * get name of host address from connected part (client or server)
				 *
				 * @return name of host address
				 */
				virtual string getHostAddressName()= 0;
				/**
				 * get actual port number
				 *
				 * @return port number
				 */
				virtual short getPort()= 0;
				/**
				 * transfer answer from geted string to client
				 *
				 * @return whether server should hold transaction
				 */
				virtual bool transfer()= 0;
				/**
				 * whether client has correct access to server
				 *
				 * @return has access
				 */
				//virtual bool hasAccess()= 0;
				/**
				 * set true access if client have access to server
				 */
				//virtual void setAccess(bool access= true)= 0;
				/**
				 * set boolean into object
				 *
				 * @param str name of boolean
				 * @param boolean value of boolean
				 */
				virtual void setBoolean(string str, bool boolean)= 0;
				/**
				 * read value of boolean
				 *
				 * @param str name of boolean
				 * @return value of boolean
				 */
				virtual bool getBoolean(string str)= 0;
				/**
				 * set string into object
				 *
				 * @param str name of variable
				 * @param value value of defiend string
				 */
				virtual void setString(string str, string value)= 0;
				/**
				 * read actual value of string
				 *
				 * @param str name of string variable
				 * @return value of defined name
				 */
				virtual string getString(string str)= 0;
				/**
				 * get current client ID
				 *
				 * @return ID
				 */
				virtual unsigned int getClientID()= 0;
				/**
				 * set ID for client
				 *
				 * @param ID new ID for client
				 */
				virtual void setClientID(unsigned int ID)= 0;
				/**
				 * whether client is an speaker thread
				 *
				 * @return is speaker
				 */
				//virtual bool isSpeaker()= 0;
				/**
				 * set client to an speaker thread if param speaker is true (default)
				 *
				 * @param speaker true if client shoud be an speaker thread
				 */
				//virtual void setToSpeaker(bool speaker= true)= 0;
				/**
				 * destructor to dereference file
				 */
				virtual ~IFileDescriptorPattern() {};
		};
	}
}

#endif /*IFILEDESCRIPTORPATTERN_H_*/
