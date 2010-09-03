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

	//int ProcessStarter::start(const string& file, const string& arg0, ...)
	int ProcessStarter::start(const char* file, ...)
	{
		int nRv;
		size_t nFile= strlen(file);
		unsigned short count= 0;
		va_list list;
		char* cfile;
		char* value;
		vector<char*> vList;

		m_sApp= file;
		cfile= new char[nFile+2];
		strncpy(cfile, file, nFile+1);
		vList.push_back(cfile);
		va_start(list, file);
		do{
			value= va_arg(list, char*);
			vList.push_back(value);

		}while(value != NULL);
		va_end(list);
		m_ppEllipse= new char*[vList.size()];
		for(vector<char*>::iterator it= vList.begin(); it != vList.end(); ++it)
		{
			m_ppEllipse[count]= *it;
			++count;
		}
		nRv= Process::start(NULL, false);
		delete[] m_ppEllipse;
		delete[] cfile;
		return nRv;
	}

	int ProcessStarter::runprocess(void*, bool bHold)
	{
		int nRv;

		nRv= execv(m_sApp.c_str(), m_ppEllipse);
		if(nRv != 0)
		{
			switch(errno)
			{
			case E2BIG:
				nRv= 5;
				break;
			case EACCES:
				nRv= 6;
				break;
			case EINVAL:
				nRv= 7;
				break;
			case ELOOP:
				nRv= 8;
				break;
			case ENAMETOOLONG:
				nRv= 9;
				break;
			case ENOENT:
				nRv= 10;
				break;
			case ENOTDIR:
				nRv= 11;
				break;
			case ENOEXEC:
				nRv= 12;
				break;
			case ENOMEM:
				nRv= 13;
				break;
			case ETXTBSY:
				nRv= 14;
				break;
			default:
				nRv= 20;
				break;
			}
			// ToDo: send over transaction back the error
		}
		return nRv;
	}

	string ProcessStarter::strerror(int error)
	{
		string str;

		switch(error)
		{
		case 5:
			str= "E2BIG  The  number  of  bytes  used  by  the new process image’s argument list and environment list is greater than the system-imposed limit of {ARG_MAX} bytes.";
			break;
		case 6:
			str= "EACCES Search permission is denied for a directory listed in the new process image file’s path prefix, or the new process  image  file  denies  execution"
	 				   " permission, or the new process image file is not a regular file and the implementation does not support execution of files of its type.";
			break;
		case 7:
			str= "EINVAL The new process image file has the appropriate permission and has a recognized executable binary format, but the system does not support execution"
     			        " of a file with this format.";
			break;
		case 8:
			str= "ELOOP  A loop exists in symbolic links encountered during resolution of the path or file argument.";
			break;
		case 9:
			str= "ENAMETOOLONG"
				" The length of the path or file arguments exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.";
			break;
		case 10:
			str= "ENOENT A component of path or file does not name an existing file or path or file is an empty string.";
			break;
		case 11:
			str= "ENOTDIR"
              " A component of the new process image file’s path prefix is not a directory.";
			break;
		case 12:
			str= "ENOEXEC"
              " The new process image file has the appropriate access permission but has an unrecognized format.";
			break;
		case 13:
			str= "ENOMEM The new process image requires more memory than is allowed by the hardware or system-imposed memory management constraints.";
			break;
		case 14:
			str= "ETXTBSY"
              " The new process image file is a pure procedure (shared text) file that is currently open for writing by some process.";
			break;
		case 20:
			str= "undefined error by start an external application with execv()";
		default:
			str= Process::strerror(error);
			break;
		}
		return str;
	}
}
