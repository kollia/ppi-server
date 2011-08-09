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

#include "logstructures.h"
#include "LogConnectionChecker.h"

namespace logger {

int LogConnectionChecker::execute()
{
	sleep(1);
	if(m_poStarter->openConnection() <= 0)
	{
		LOCK(m_WRITELOOP);
		if(!m_poStarter->writeVectors())
		{
			// stop thread by the next loop
			// because maybe LogInterface write any log-messages
			// more into the vectors
			stop();// stop own thread
			m_poStarter->connectionCheckerStops();
		}
		UNLOCK(m_WRITELOOP);
	}
	return 0;
}

}
