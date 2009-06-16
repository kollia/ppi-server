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
#ifndef SSLCONNECT_H_
#define SSLCONNECT_H_

#include "../util/debug.h"
#ifdef _OPENSSLLIBRARY

#include <string>

#include "../pattern/server/IServerConnectArtPattern.h"
#include "../pattern/server/IFileDescriptorPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * initialication of connection over SSL/TCP.<br />
	 * this class need the ssl package libssl-dev (ubuntu)
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0
	 */
	class SSLConnect : public IServerConnectArtPattern
	{
		public:
			/**
			 * constructor to initial object
			 */
			SSLConnect();
			/**
			 * initial connection
			 */
			virtual bool init();
			/**
			 * listen on socket for new connection
			 *
			 * @return object of IFileDescriptorPattern for communicate with client
			 */
			virtual IFileDescriptorPattern* listen();
			/**
			 * return the address witch the comunication have reached after listen
			 *
			 * @return name of adress
			 */
			virtual string getLastDescriptorAddress();
			/**
			 * close all connections of sercer
			 */
			virtual void close() {};
			/**
			 * destructor of TcpConnect
			 */
			virtual ~SSLConnect();
	};

}

#endif //_OPENSSLLIBRARY
#endif /*SSLCONNECT_H_*/
