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
#ifndef ISERVERCOMMUNICATIONSTARTERPATTERN_H_
#define ISERVERCOMMUNICATIONSTARTERPATTERN_H_

#include "IClientHolderPattern.h"
#include "IFileDescriptorPattern.h"
#include "../util/ithreadpattern.h"

namespace design_pattern_world
{
	namespace server_pattern
	{
		/**
		 * abstract interface pattern for communication thread starter
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0.
		 */
		class IServerCommunicationStarterPattern :	virtual public IClientHolderPattern,
													virtual public util_pattern::IThreadPattern
		{
		public:
			/**
			 * function locate the next free client id
			 *
			 * @return next free client id
			 */
			virtual unsigned int nextClientID()= 0;
			/**
			 * fill an free communication thread with an new FileDescriptor
			 *
			 * @param descriptor new client connection FileDescriptor
			 */
			virtual void setNewClient(IFileDescriptorPattern* descriptor)= 0;
			/**
			 * search client with given defined name
			 * and return this client
			 *
			 * @param definition defined name to find client
			 * @param own pointer of own called descriptor witch client is not needed
			 * @return return client
			 */
			virtual IClientPattern* getClient(const string& definition, IFileDescriptorPattern* own) const= 0;
			/**
			 * virtual destructor of pattern
			 */
			virtual ~IServerCommunicationStarterPattern() {};
		};
	}
}

#endif /*ISERVERCOMMUNICATIONSTARTERPATTERN_H_*/
