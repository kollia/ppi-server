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
#ifndef ISERVERCONNECTARTPATTERN_H_
#define ISERVERCONNECTARTPATTERN_H_

#include <string>

#include "IClientConnectArtPattern.h"
#include "IFileDescriptorPattern.h"

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
		class IServerConnectArtPattern : public virtual IClientConnectArtPattern
		{
		public:
			/**
			 * set an instnace of the called server
			 *
			 * @param server instance of calling server
			 */
			virtual void setServerInstance(IServerPattern* server)= 0;
			/**
			 * listen on device for new connection
			 *
			 * @return object of ITransferPattern for communicate with client
			 */
			virtual IFileDescriptorPattern* listen()=0;
			/**
			 * return the address witch the comunication have reached after listen
			 *
			 * @return name of adress
			 */
			//virtual string getLastDescriptorAddress()=0;
			/**
			 * destructor for pattern
			 */
			virtual ~IServerConnectArtPattern() {};
		};
	}
}

#endif /*ISERVERCONNECTARTPATTERN_H_*/
