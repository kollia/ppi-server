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
#ifndef ISOCKETTRANSFERPATTERN_H_
#define ISOCKETTRANSFERPATTERN_H_

#include "ITransferPattern"

namespace design_pattern_world
{
	namespace server_pattern
	{
		/**
		 * abstract interface pattern for initial transaction over socket from server to client or backward
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0
		 */
		class ISocketTransferPattern : public ITransferPattern
		{
			virtual void socket(socketStruct* socket)=0;
		};
	}
}

#endif /*ISOCKETTRANSFERPATTERN_H_*/
