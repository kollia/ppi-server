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
#include "../pattern/server/IClientHolderPattern.h"

#include "FileDescriptor.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	void FileDescriptor::initial(IServerPattern* server, ITransferPattern* transfer, FILE* file, string address, unsigned short port)
	{
		m_pTransfer= transfer;
		m_poServer= server;
		m_unConnID= 0;
		m_bAccess= false;
		m_sAddress= address;
		m_nPort= port;
		m_pFile= file;
		m_nEOF= 0;
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");
		m_SENDSTRING= Thread::getMutex("SENDSTRING");
		m_THREADSAVEMETHODS= Thread::getMutex("THREADSAVEMETHODS");
		m_SENDSTRINGCONDITION= Thread::getCondition("SENDSTRINGCONDITION");
		m_GETSTRINGCONDITION= Thread::getCondition("GETSTRINGCONDITION");
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

	string FileDescriptor::getHostAddressName() const
	{
		return m_sAddress;
	}

	void FileDescriptor::setBoolean(const string& str, const bool boolean)
	{
		m_mBoolean[str]= boolean;
	}

	bool FileDescriptor::getBoolean(const string& str) const
	{
		map<string, bool>::const_iterator it;

		it= m_mBoolean.find(str);
		if(it == m_mBoolean.end())
			return false;
		return it->second;
	}

	void FileDescriptor::setString(const string& name, const string& value)
	{
		m_mString[name]= value;
	}

	string FileDescriptor::getString(const string& name) const
	{
		map<string, string>::const_iterator it;

		it= m_mString.find(name);
		if(it == m_mString.end())
			return "";
		return it->second;
	}

	short FileDescriptor::getPort() const
	{
		return m_nPort;
	}

	string FileDescriptor::sendToOtherClient(const string& definition, const string& str)
	{
		string answer;
		IClientHolderPattern* holder= m_poServer->getCommunicationFactory();
		IClientPattern* client= holder->getClient(definition);

		if(client == NULL)
			return "ERROR 006";
		UNLOCK(m_THREADSAVEMETHODS);
		answer= client->sendString(str);
		LOCK(m_THREADSAVEMETHODS);
		return answer;
	}

	string FileDescriptor::sendString(const string& str)
	{
		string answer;

		LOCK(m_SENDSTRING);
		while(m_sSendString != "")
		{ // if SendString is not null,
		  // an other client wait for answer
			CONDITION(m_SENDSTRINGCONDITION, m_SENDSTRING);
		}
		m_sSendString= str;
		AROUSE(m_GETSTRINGCONDITION);
		CONDITION(m_SENDSTRINGCONDITION, m_SENDSTRING);
		answer= m_sClientAnswer;
		m_sSendString= "";
		AROUSE(m_SENDSTRINGCONDITION);
		UNLOCK(m_SENDSTRING);
		return answer;
	}

	string FileDescriptor::getOtherClientString(const bool wait/*= true*/)
	{
		string str;

		LOCK(m_SENDSTRING);
		if(m_sSendString == "")
		{
			if(!wait)
			{
				UNLOCK(m_SENDSTRING);
				return "";
			}
			UNLOCK(m_THREADSAVEMETHODS);
			CONDITION(m_GETSTRINGCONDITION, m_SENDSTRING);
			LOCK(m_THREADSAVEMETHODS);
		}
		str= m_sSendString;
		UNLOCK(m_SENDSTRING);
		return str;
	}

	void FileDescriptor::sendAnswer(const string& asw)
	{
		LOCK(m_SENDSTRING);
		m_sClientAnswer= asw;
		AROUSEALL(m_SENDSTRINGCONDITION);
		UNLOCK(m_SENDSTRING);
	}

	unsigned int FileDescriptor::getClientID() const
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

	bool FileDescriptor::isClient(const string& definition) const
	{
		bool bRv;

		LOCK(m_THREADSAVEMETHODS);
		bRv= m_pTransfer->isClient(*this, definition);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	string FileDescriptor::getTransactionName() const
	{
		string name;

		LOCK(m_THREADSAVEMETHODS);
		name= m_pTransfer->getTransactionName(*this);
		UNLOCK(m_THREADSAVEMETHODS);
		return name;
	}

	bool FileDescriptor::transfer()
	{
		bool bRv;

		LOCK(m_THREADSAVEMETHODS);
		bRv= m_pTransfer->transfer(*this);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	FileDescriptor::~FileDescriptor()
	{
		DESTROYMUTEX(m_SENDSTRING);
		DESTROYMUTEX(m_CONNECTIONIDACCESS);
		DESTROYMUTEX(m_THREADSAVEMETHODS);
		DESTROYCOND(m_SENDSTRINGCONDITION);
		DESTROYCOND(m_GETSTRINGCONDITION);
		fclose(m_pFile);
	}

}
