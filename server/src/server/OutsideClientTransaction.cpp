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
#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <termios.h>
//#include <errno.h>
#include <string.h>

#include <iostream>
#include <sstream>

#include "OutsideClientTransaction.h"

using namespace std;
using namespace util;

namespace server
{
	bool OutsideClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		string::size_type len= m_sCommand.size();

		if(m_sCommand != "")
		{
			if(m_sCommand.substr(len -1) != "\n")
				m_sCommand+= "\n";
			descriptor << m_sCommand;
			descriptor.flush();
			if(descriptor.eof())
			{
				m_sAnswer= "#ERROR 001";
				return false;
			}
			descriptor >> m_sAnswer;
			m_sAnswer= ConfigPropertyCasher::trim(m_sAnswer, " \t\r\n");
		}
		if(m_bHold == false)
		{
			descriptor << "ending\n";
			descriptor.flush();
			return false;
		}
		return true;
	}


}
