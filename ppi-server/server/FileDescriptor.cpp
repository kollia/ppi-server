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
#include "../util/Thread.h"

#include "../pattern/server/ITransferPattern.h"

#include "FileDescriptor.h"


namespace server
{
	FileDescriptor::FileDescriptor()
	{
		m_sAddress= "";
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");
	}

	FileDescriptor::FileDescriptor(ITransferPattern* transfer, FILE* file, string address, unsigned short port)
	{
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");

		m_pTransfer= transfer;
		m_pFile= file;
		m_sAddress= address;
		m_nPort= port;
	}

	bool FileDescriptor::init()
	{
		return m_pTransfer->init(*this);
	}

	void FileDescriptor::operator >>(string &reader)
	{
		char *res;
		char buf[501];

		res= fgets(buf, sizeof(buf), m_pFile);
		if(res == NULL)
		{
			reader= "";
			m_nEOF= EOF;
			return;
		}
		reader= buf;
	}

	void FileDescriptor::operator <<(string writer)
	{
		m_nEOF= fputs(writer.c_str(), m_pFile);
	}

	void FileDescriptor::endl()
	{
		m_nEOF= fputs("\n", m_pFile);
	}

	bool FileDescriptor::eof()
	{
		if(m_nEOF == EOF)
			return true;
		if(feof(m_pFile) != 0)
			return true;
		return false;
	}

	void FileDescriptor::flush()
	{
		if(ferror(m_pFile) != 0)
		{
			m_nEOF= EOF;
			return;
		}
		try{
			fflush(m_pFile);
		}catch(...)
		{
			// undefined error on stream
			// maybe connection to client was broken
			m_nEOF= EOF;
		}
	}

	string FileDescriptor::getHostAddressName()
	{
		return m_sAddress;
	}

	void FileDescriptor::setBoolean(string str, bool boolean)
	{
		m_mBoolean[str]= boolean;
	}

	bool FileDescriptor::getBoolean(string str)
	{
		bool bRv= m_mBoolean[str];

		return bRv;
	}

	void FileDescriptor::setString(string name, string value)
	{
		m_mString[name]= value;
	}

	string FileDescriptor::getString(string name)
	{
		return m_mString[name];
	}

	short FileDescriptor::getPort()
	{
		return m_nPort;
	}

	unsigned int FileDescriptor::getClientID()
	{
		unsigned int ID;

		LOCK(m_CONNECTIONIDACCESS);
		ID= m_unConnID;
		UNLOCK(m_CONNECTIONIDACCESS);
		return ID;
	}

	void FileDescriptor::setClientID(unsigned int ID)
	{
		LOCK(m_CONNECTIONIDACCESS);
		m_unConnID= ID;
		UNLOCK(m_CONNECTIONIDACCESS);
	}

	/*bool FileDescriptor::hasAccess()
	{
		bool bAccess;

		LOCK(m_HAVECLIENT);
		bAccess= m_bAccess;
		UNLOCK(m_HAVECLIENT);
		return bAccess;
	}*/

	//void FileDescriptor::setAccess(bool access/*=true*/)
	/*{
		LOCK(m_HAVECLIENT);
		m_bAccess= access;
		UNLOCK(m_HAVECLIENT);
	}*/

	bool FileDescriptor::transfer()
	{
		return m_pTransfer->transfer(*this);
	}

	FileDescriptor::~FileDescriptor()
	{
		DESTROYMUTEX(m_CONNECTIONIDACCESS);
		fclose(m_pFile);
	}

}
