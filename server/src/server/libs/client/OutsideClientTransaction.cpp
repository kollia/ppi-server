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

#include "../../../util/ExternClientInputTemplate.h"
#include "../../../util/configpropertycasher.h"

using namespace std;
using namespace util;

namespace server
{
	bool OutsideClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		int err;
		string answer;
		string::size_type len= m_sCommand.size();

		m_vAnswer.clear();
		if(m_bHold)
		{
			if(len > 0)
			{
				if(m_sCommand.substr(len -1) != "\n")
					m_sCommand+= "\n";
			}else
				m_sCommand= "\n";
			do{
				if(descriptor.eof())
				{
					m_vAnswer.push_back("ERROR 001");
					return false;
				}
				descriptor << m_sCommand;
				descriptor.flush();
				if(descriptor.eof())
				{
					m_vAnswer.push_back("ERROR 001");
					return false;
				}
				descriptor >> answer;
				answer= ConfigPropertyCasher::trim(answer, " \t\r\n");
				err= ExternClientInputTemplate::error(answer);
				if(err != 0)
				{
					err+= (err > 0 ? getMaxErrorNums(/*error*/true) : (getMaxErrorNums(/*error*/false) * -1));
					answer= ExternClientInputTemplate::error(err);
				}
				m_vAnswer.push_back(answer);

			}while(	m_sAnswerEnding != ""
					&&
					answer != m_sAnswerEnding
					&&
					err == 0					);
			m_sCommand= "";
			m_bHold= true;
		}else
		{
			if(m_sEnding != "" && !descriptor.eof())
			{
				descriptor << m_sEnding;
				descriptor.endl();
				descriptor.flush();
				descriptor >> answer;
				m_vAnswer.push_back(answer);
			}
			m_bHold= true;//for new beginning
			return false;
		}
		return true;
	}

	string OutsideClientTransaction::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 1:
			str= "ERROR: connection is broken";
			break;
		default:
			str= "undefined socket client error";
		}
		return str;
	}

	inline unsigned int OutsideClientTransaction::getMaxErrorNums(const bool byerror) const
	{
		if(byerror)
			return 1;
		return 0;
	}

}
