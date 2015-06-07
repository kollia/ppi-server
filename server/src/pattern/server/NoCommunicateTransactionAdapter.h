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
#ifndef NOCOMMUNICATETRANSACTIONADAPTER_H_
#define NOCOMMUNICATETRANSACTIONADAPTER_H_

#include "ITransferPattern.h"

namespace design_pattern_world
{
	namespace server_pattern
	{
		/**
		 * Adapter to initial transaction from server to client
		 * with the dummy method <code>isClient()</code> only needed for transaction between
		 * more clients in an server
		 *
		 * @author Alexander Kolli
		 * @version 1.0.0
		 */
		class NoCommunicateTransferAdapter : virtual public ITransferPattern
		{
			public:
				/**
				 * dummy method to get name of transaction
				 *
				 * @param descriptor file handle to get variables
				 * @return null name
				 */
				virtual string getTransactionName(const IFileDescriptorPattern& descriptor) const
				{ return "NoCommunicateTransferAdapter"; };
				/**
				 * dummy method for no search whether client with given defined name
				 * is the correct one
				 *
				 * @param descriptor file handle to get variables
				 * @param definition defined name to find client
				 * @return always false
				 */
				virtual bool isClient(const IFileDescriptorPattern& descriptor, const string& definition) const
				{ return false; };
		};
	}
}

#endif /*NOCOMMUNICATETRANSACTIONADAPTER_H_*/
