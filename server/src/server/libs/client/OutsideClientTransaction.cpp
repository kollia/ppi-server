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

		m_pSocketError->clear();
		m_vAnswer.clear();
		if(m_bHold)
		{
			if(m_vsAnswerBlock.size())
			{// client is defined for answering question
				for(vector<string>::iterator it= m_vsAnswerBlock.begin(); it != m_vsAnswerBlock.end(); ++it)
				{
					if(descriptor.eof())
					{
						m_pSocketError->setError("OutsideClientTransaction", "descriptor");
						m_vAnswer.push_back(m_pSocketError->getErrorStr());
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
				{
					cout << "WARNING: sending string with no content" << endl;
					m_sCommand= "\n";
				}
#if 0
				/*
				 * debug output by over length
				 * of command by fill debug session
				 */
				if(	m_sCommand.substr(0, 41) == "ppi-db-server true false fillDebugSession" &&
					m_sCommand.length() > 3041	)
				{
					IMethodStringStream method(m_sCommand.substr(25));
					ostringstream out;
					size_t pos;
					string folder, subroutine;

					method >> folder;
					method >> subroutine;
					//m_sCommand= "ppi-db-server true false fillDebugSession   \"power_switch\"  \"#inform\"  0 1424286923.137149 \"--------------------------------------------------------------\\nINFORM power_switch:port_switch while from INTERNAL 'Raff2_Zeit:closed'\\ncalculate inform parameter (' all_started = 0 & ( Raff_Zeit_alle:do_on_grad_pressed | Raff_Alle:Auf | Raff_Alle:Zu ) ?\\n                                         (  ( Raff1:Auf | Raff1:Zu | Raff1_Zeit:use_Raff = 0 |\\n                                              (Raff1_Zeit:do_on_grad_pressed & \\n                                               Raff1_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff1_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff1_Zeit:closed)                   ) & \\n                                            ( Raff2:Auf | Raff2:Zu | Raff2_Zeit:use_Raff = 0 |\\n                                              (Raff2_Zeit:do_on_grad_pressed & \\n                                               Raff2_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff2_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff2_Zeit:closed)                   ) &\\n                                            ( Raff3:Auf | Raff3:Zu | Raff3_Zeit:use_Raff = 0 |\\n                                              (Raff3_Zeit:do_on_grad_pressed & \\n                                               Raff3_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff3_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff3_Zeit:closed)                   ) &  \\n                                            ( Raff4:Auf | Raff4:Zu | Raff4_Zeit:use_Raff = 0 |\\n                                              (Raff4_Zeit:do_on_grad_pressed & \\n                                               Raff4_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff4_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff4_Zeit:closed)                   ) &  \\n                                            ( Raff5:Auf | Raff5:Zu | Raff5_Zeit:use_Raff = 0 |\\n                                              (Raff5_Zeit:do_on_grad_pressed & \\n                                               Raff5_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff5_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff5_Zeit:closed)                   ) &  \\n                                            ( Raff6:Auf | Raff6:Zu | Raff6_Zeit:use_Raff = 0 |\\n                                              (Raff6_Zeit:do_on_grad_pressed & \\n                                               Raff6_Zeit:make_grad_can_start = 0 ) |\\n                                              (Raff_Alle:Auf & Raff6_Zeit:schliessen=0) |\\n                                              (Raff_Alle:Zu & Raff6_Zeit:closed)                   )    ) : true    ')\\n\\nif: [all_started=0] = 0 & ([Raff_Zeit_alle:do_on_grad_pressed=0]  | [Raff_Alle:Auf=0]  | [Raff_Alle:Zu=1]  {result TRUE})  {result TRUE}\\n  then: (([Raff1:Auf=0]  | [Raff1:Zu=0]  | [Raff1_Zeit:use_Raff=1] = 0 | ([Raff1_Zeit:do_on_grad_pressed=0] {break by FALSE})  | ([Raff_Alle:Auf=0] {break by FALSE})  | ([Raff_Alle:Zu=1]  & [Raff1_Zeit:closed=1]  {result TRUE})  {result TRUE})  & ([Raff2:Auf=0]  | [Raff2:Zu=0]  | [Raff2_Zeit:use_Raff=1] = 0 | ([Raff2_Zeit:do_on_grad_pressed=0] {break by FALSE})  | ([Raff_Alle:Auf=0] {break by FALSE})  | ([Raff_Alle:Zu=1]  & [Raff2_Zeit:closed=1]  {result TRUE})  {result TRUE})  & ([Raff3:Auf=0]  | [Raff3:Zu=1] {break by TRUE})  & ([Raff4:Auf=0]  | [Raff4:Zu=1] {break by TRUE})  & ([Raff5:Auf=0]  | [Raff5:Zu=1] {break by TRUE})  & ([Raff6:Auf=0]  | [Raff6:Zu=1] {break by TRUE})  {result TRUE})  {result TRUE}\\n--------------------------------------------------------------\\n\" \n";
					pos= m_sCommand.find('\n');
					out << "[" << Thread::gettid() << "] fill debug session for " << folder << ":" << subroutine << " with ";
					if(endString == "")
						out << "no ";
					out << "end string ";
					if(endString != "")
						out << "'" << endString << "' ";
					out << "and length " << m_sCommand.length();
					out << " \\n ";
					if(pos == string::npos)
						out << "with no pos" << endl;
					else
						out << "with pos " << pos << endl;
		//			out << ">> '" << m_sCommand << "' <<" << endl;
					cout << out.str();
				}
#endif // debug
				descriptor << m_sCommand;
				descriptor.flush();
			}
			do{ // while(endString != "")

				if(descriptor.eof())
				{
					m_pSocketError->setError("OutsideClientTransaction", "descriptor");
					m_vAnswer.push_back(m_pSocketError->getErrorStr());
					return false;
				}
				descriptor >> answer;
				boost::trim(answer);
				m_pSocketError->setErrorStr(answer);
				m_vAnswer.push_back(answer);
				if(m_pSocketError->hasError())
					break;
				if(	endString != "" &&
					endString == answer	)
				{
					endString= "";
				}
			}while(endString != "");

			m_sCommand= "";
			m_bHold= true;
		}else // if(m_bHold)
		{
			if(m_sEnding != "" && !descriptor.eof())
			{
				descriptor << m_sEnding;
				descriptor.endl();
				descriptor.flush();
				descriptor >> answer;
				m_pSocketError->setErrorStr(answer);
				m_vAnswer.push_back(answer);
			}
			m_bHold= true;//for new beginning
			return false;
		}// if(m_bHold)
		return true;
	}

	EHObj OutsideClientTransaction::init(IFileDescriptorPattern& descriptor)
	{
		//m_pSocketError->setError("OutsideClientTransaction", "init");
		return m_pSocketError;
	}

	void OutsideClientTransaction::setAnswer(const vector<string>& answer)
	{
		m_sCommand= "";
		m_vsAnswerBlock= answer;
	}

}
