/**
 *   This file 'CallbackTemplate.cpp' is part of ppi-server.
 *   Created on: 26.12.2011
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

#include "CallbackTemplate.h"

void CallbackTemplate::initialStarting()
{
	start(NULL, /*holding*/false);
}

int CallbackTemplate::execute()
{
	m_nReturnCode= runnable();
	if(m_nReturnCode != 0)
		stop(/*wait*/false);
	return static_cast<int>(m_nReturnCode);
}

short CallbackTemplate::finished()
{
	if( running() ||
		!stopping()	)
	{
		return 0;
	}
	return m_nReturnCode;
}
