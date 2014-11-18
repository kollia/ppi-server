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

#include <unistd.h>
#include <string.h>

#include <vector>

#include "ProcessStarter.h"

namespace util
{

	EHObj ProcessStarter::start(const string& file, const vector<string>& params)
	{
		m_sApp= file;
		m_vParams= params;
		m_pSocketError= Process::start(NULL, false);
		return m_pSocketError;
	}

	EHObj ProcessStarter::runprocess(void*, bool bHold)
	{
		int nRv;
		unsigned short count= 0;
		char* pstr;
		/**
		 * stored ellipse character strings
		 */
		vector<char*> vList;
		/**
		 * ellipse parameters of start method
		 */
		char **ppEllipse;

		count= m_vParams.size() + 2;
		ppEllipse= new char*[count];
		pstr= new char[m_sApp.size() + 2];
		std::copy(m_sApp.begin(), m_sApp.end(), pstr);
		pstr[m_sApp.size()]= '\0';
		vList.push_back(pstr);
		ppEllipse[0]= pstr;
		count= 1;
		if(m_bShowBinary)
			cout << "start inside forked process \"" << ppEllipse[0];
		for(vector<string>::iterator it= m_vParams.begin(); it != m_vParams.end(); ++it)
		{
			pstr= new char[it->size() + 2];
			std::copy(it->begin(), it->end(), pstr);
			pstr[it->size()]= '\0';
			vList.push_back(pstr);
			ppEllipse[count]= pstr;
			if(m_bShowBinary)
				cout << " " << ppEllipse[count];
			++count;
		}
		if(m_bShowBinary)
			cout << "\"" << endl;
		ppEllipse[count]= NULL;
		nRv= execv(m_sApp.c_str(), ppEllipse);
		if(nRv != 0)
		{
			int error(errno);
			string command(m_sApp + " ");

			for(vector<string>::iterator it= m_vParams.begin(); it != m_vParams.end(); ++it)
			{
				command+= *it + " ";
			}
			command= command.substr(0, command.length() - 1);
			m_pSocketError->setErrnoError("ProcessStarter", "execv", error, command);
		}
		for(vector<char*>::iterator it= vList.begin(); it != vList.end(); ++it)
		{
			char* p;

			if(*it == NULL)
				break;
			p= *it;
			delete[] p;
		}
		delete[] ppEllipse;
		return m_pSocketError;
	}

}
