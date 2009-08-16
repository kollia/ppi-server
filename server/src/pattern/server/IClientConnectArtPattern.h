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
#ifndef ICLIENTCONNECTARTPATTERN_H_
#define ICLIENTCONNECTARTPATTERN_H_

#include <string>

#include "ITransferPattern.h"
#include "IServerPattern.h"

using namespace std;

namespace design_pattern_world
{
	namespace server_pattern
	{
		/**
		 * abstract interface pattern for initial connection to any client
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0
		 */
		class IClientConnectArtPattern
		{
		public:
			/**
			 * set new transfer object and delete the old one if exist
			 *
			 * @param transfer pattern of ITransferPattern to cumunicate with server
			 * @param delOld whether the method should delete the transfer object set before
			 */
			virtual void newTranfer(ITransferPattern* transfer, const bool delOld)= 0;
			/**
			 * initial connection and call up transfer over ITransverPattern
			 *
			 * @return error code elsewhere 0 see overview from extended class
			 */
			virtual int init()=0;
			/**
			 * get host address to which client connect
			 *
			 * @return host address
			 */
			virtual const string getHostAddress() const= 0;
			/**
			 * return port address on which client connect
			 *
			 * @return port address
			 */
			virtual const unsigned short getPortAddress() const= 0;
			/**
			 * return string describing error number
			 *
			 * @param error code number of error
			 * @return error string
			 */
			virtual string strerror(int error) const= 0;
			/**
			 * close all connections of sercer
			 */
			virtual void close()=0;
			/**
			 * destructor for pattern
			 */
			virtual ~IClientConnectArtPattern() {};
		};
	}
}

#endif /*ICLIENTCONNECTARTPATTERN_H_*/
