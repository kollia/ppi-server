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

#include <boost/algorithm/string/trim.hpp>

#include "OutsideClientTransaction.h"
#include "ExternClientInputTemplate.h"


using namespace std;
using namespace util;

namespace server
{
	bool OutsideClientTransaction::transfer(IFileDescriptorPattern& descriptor)
	{
		/**
		 * error number when connection be failt
		 */
		int err;
		/**
		 * current answer of server
		 */
		string answer;
		/**
		 * get answer with more rows
		 * where this variable should be
		 * the ending string
		 */
		string endString;
		/**
		 * length of command
		 * when object not be setting
		 * for answering
		 */
		string::size_type len= m_sCommand.size();

		m_vAnswer.clear();
		if(m_bHold)
		{
			if(m_vsAnswerBlock.size())
			{// client is defined for answering question
				for(vector<string>::iterator it= m_vsAnswerBlock.begin(); it != m_vsAnswerBlock.end(); ++it)
				{
					if(descriptor.eof())
					{
						m_vAnswer.push_back("ERROR 001");
						return false;
					}
					descriptor << *it;
					if((*it).substr((*it).size(), -1) != "\n")
						descriptor.endl();
					descriptor.flush();
				}
			}else
			{// client is defined to send questions
				string::size_type pos, start;

				// look first whether client get an answer
				// over more rows
				pos= m_sCommand.find(" ");
				if(pos != string::npos)
				{
					pos= m_sCommand.find(" ", pos + 1);
					if(pos != string::npos)
					{
						if(m_sCommand.substr(pos + 1, 4) == "true")
						{
							start= pos + 6;
							pos= m_sCommand.find(" ", start);
							if(pos != string::npos)
								endString= m_sCommand.substr(start, pos - start);
						}
					}
				}
				if(len > 0)
				{
					if(m_sCommand.substr(len -1) != "\n")
						m_sCommand+= "\n";
				}else
					m_sCommand= "\n";
				descriptor << m_sCommand;
			}
			do{ // while(endString != "")

				if(descriptor.eof())
				{
					m_vAnswer.push_back("ERROR 001");
					return false;
				}
				descriptor >> answer;
				boost::trim(answer);
				err= ExternClientInputTemplate::error(answer);
				if(err != 0)
				{
					err+= (err > 0 ? getMaxErrorNums(/*error*/true) : (getMaxErrorNums(/*error*/false) * -1));
					answer= ExternClientInputTemplate::error(err);
				}
				m_vAnswer.push_back(answer);
				if(err > 0)
					break;
				if(	endString != "" &&
					endString == answer	)
				{
					endString= "";
				}
			}while(endString != "");
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

	void OutsideClientTransaction::setAnswer(const vector<string>& answer)
	{
		m_sCommand= "";
		m_vsAnswerBlock= answer;
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
