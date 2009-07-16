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
#ifndef ICLIENTHOLDERPATTERN_H_
#define ICLIENTHOLDERPATTERN_H_

#include <string>

#include "IClientPattern.h"

using namespace std;

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
		class IClientHolderPattern
		{
		public:
			/**
			 * whether one or more clients are connected
			 * with any communication thread
			 *
			 * @return cout of clients
			 */
			virtual short hasClients() const= 0;
			/**
			 * arouse condition for communication-thread-starter
			 * to search again for needed new communication threads
			 */
			virtual void arouseStarterThread() const= 0;
			/**
			 * send stop to all communication threads
			 *
			 * @param bWait if stopping process should wait for ending
			 * @param transactionName whether method is called from an transaction, there should be given the transaction name for no wait for this transaction. To get no dead lock
			 */
			virtual void stopCommunicationThreads(bool bWait= false, string transactionName= "")= 0;
			/**
			 * search client with given defined name
			 * and return this client
			 *
			 * @param definition defined name to find client
			 * @return return client
			 */
			virtual IClientPattern* getClient(const string& definition) const= 0;
			/**
			 * virtual destructor of pattern
			 */
			virtual ~IClientHolderPattern() {};
		};
	}
}

#endif /*ICLIENTHOLDERPATTERN_H_*/
