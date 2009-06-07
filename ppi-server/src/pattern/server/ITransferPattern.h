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
#ifndef ITRANSFERPATTERN_H_
#define ITRANSFERPATTERN_H_

#include "IFileDescriptorPattern.h"

namespace design_pattern_world
{
	namespace server_pattern
	{
		/**
		 * abstract interface pattern to sending answer or command to server or client
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0
		 */
		class ITransferPattern
		{
			public:
				/**
				 * initial all values for transaction
				 *
				 * @param descriptor file handle to set start values
				 * @return whether initialization was correct
				 */
				virtual bool init(IFileDescriptorPattern& descriptor)= 0;
				/**
				 * transaction to client or server
				 *
				 * @param descriptor file handle to get command's and send answer
				 * @return wether need to hold the connection
				 */
				virtual bool transfer(IFileDescriptorPattern& descriptor)= 0;
				/**
				 * destructor to dereference file
				 */
				virtual ~ITransferPattern() {};
		};
	}
}

#endif /*ITRANSFERPATTERN_H_*/
