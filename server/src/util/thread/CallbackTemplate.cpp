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

EHObj CallbackTemplate::initialStarting()
{
	return start(NULL, /*holding*/false);
}

bool CallbackTemplate::execute()
{
	return runnable();
}

short CallbackTemplate::finished(bool bWait/*= false*/)
{
	do{
		if( running() ||
			!stopping()	)
		{
			if(bWait)
				USLEEP(50);
			else
				return 0;
		}else
			break;
	}while(bWait);
	if(m_pError->fail())
	{
		if(m_pError->hasError())
			return -2;
		else // warning occurred
			return -1;
	}
	return 1;
}
