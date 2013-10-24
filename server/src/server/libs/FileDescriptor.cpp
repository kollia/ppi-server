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

#include <iostream>
#include <utility>

#include "../../pattern/server/ITransferPattern.h"
#include "../../pattern/server/IClientHolderPattern.h"
#include "../../pattern/server/IServerCommunicationStarterPattern.h"

#include "../../util/debugtransaction.h"

#include "../../util/stream/OMethodStringStream.h"

#include "FileDescriptor.h"

using namespace std;
using namespace design_pattern_world::server_pattern;

namespace server
{
	void FileDescriptor::initial(IServerPattern* server, ITransferPattern* transfer, FILE* file, string address,
			const unsigned short port)
	{
		m_pTransfer= transfer;
		m_poServer= server;
		m_unConnID= 0;
		m_bFileAccess= true;
		m_sAddress= address;
		m_nPort= port;
		m_pFile= file;
		m_bEOF= false;
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");
		m_SENDSTRING= Thread::getMutex("SENDSTRING");
		m_THREADSAVEMETHODS= Thread::getMutex("THREADSAVEMETHODS");
		m_SENDSTRINGCONDITION= Thread::getCondition("SENDSTRINGCONDITION");
		m_GETSTRINGCONDITION= Thread::getCondition("GETSTRINGCONDITION");
	}

	bool FileDescriptor::init()
	{
		bool bRv= true;

		LOCK(m_THREADSAVEMETHODS);
		if(m_pTransfer)
			bRv= m_pTransfer->init(*this);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	void FileDescriptor::operator >>(string &reader)
	{
		char *res;
		char buf[501];
		string::size_type bufLen= (sizeof(buf)-2);
		string sread;

		if(eof())
			return;
		reader= "";
		UNLOCK(m_THREADSAVEMETHODS);
		do{
			res= fgets(buf, bufLen+1, m_pFile);
			sread= (res != NULL) ? string(res) : string("");
			reader+= sread;

		}while(!eof() && sread.length() == bufLen && sread.substr(bufLen-1, 1) != "\n");
		LOCK(m_THREADSAVEMETHODS);
		if(res == NULL)
		{
			reader= "";
			m_bEOF= true;
			return;
		}
	}

	void FileDescriptor::operator << (const string& writer)
	{
		if(eof())
			return;
		if(fputs(writer.c_str(), m_pFile) < 0)
		{
			error();
			m_bEOF= true;
		}
	}

	inline void FileDescriptor::endl()
	{
		(*this) << "\n";
	}

	inline bool FileDescriptor::eof() const
	{
		if(m_bEOF)
			return true;
		if(feof(m_pFile) != 0)
		{
			m_bEOF= true;
			return true;
		}
		return false;
	}

	inline int FileDescriptor::error() const
	{
		m_nErr= ferror(m_pFile);
		if(m_nErr != 0)
			m_bEOF= true;
		if(m_bEOF)
		{
			if(m_nErr == 0)
				m_nErr= EOF;
		}
		return m_nErr;
	}

	void FileDescriptor::flush()
	{
		if(m_bEOF)
			return;
		try{
			fflush(m_pFile);
		}catch(...)
		{
			// undefined error on stream
			// maybe connection to client was broken
			error();
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

	string FileDescriptor::strerror(int error) const
	{
		string str;

		switch(error)
		{
		case 1:
			str= "ERROR: no client found for spezified definition";
			break;
		case 2:
			str= "connection to hearing client is broken";
			break;
		default:
			str= m_pTransfer->strerror(error > 0 ? error + 10 : error - 10);
			break;
		}
		return str;
	}

	inline unsigned int FileDescriptor::getMaxErrorNums(const bool byerror) const
	{
		if(byerror)
			return 2;
		return 0;
	}

	bool FileDescriptor::getBoolean(const string& str) const
	{
		map<string, bool>::const_iterator it;

		it= m_mBoolean.find(str);
		if(it == m_mBoolean.end())
			return false;
		return it->second;
	}

	void FileDescriptor::setShort(const string& str, const short value)
	{
		m_mShort[str]= value;
	}

	short FileDescriptor::getShort(const string& str) const
	{
		map<string, short>::const_iterator it;

		it= m_mShort.find(str);
		if(it == m_mShort.end())
			return 0;
		return it->second;
	}

	void FileDescriptor::setUShort(const string& str, const unsigned short value)
	{
		m_mUShort[str]= value;
	}

	unsigned short FileDescriptor::getUShort(const string& str) const
	{
		map<string, unsigned short>::const_iterator it;

		it= m_mUShort.find(str);
		if(it == m_mUShort.end())
			return 0;
		return it->second;
	}

	void FileDescriptor::setInt(const string& str, const int value)
	{
		m_mInt[str]= value;
	}

	int FileDescriptor::getInt(const string& str) const
	{
		map<string, int>::const_iterator it;

		it= m_mInt.find(str);
		if(it == m_mInt.end())
			return 0;
		return it->second;
	}

	void FileDescriptor::setUInt(const string& str, const unsigned int value)
	{
		m_mUInt[str]= value;
	}

	void FileDescriptor::setULong(const string& str, const unsigned long value)
	{
		m_mULong[str]= value;
	}

	void FileDescriptor::setULongLong(const string& str, const unsigned long long value)
	{
		m_mULongLong[str]= value;
	}

	unsigned int FileDescriptor::getUInt(const string& str) const
	{
		map<string, unsigned int>::const_iterator it;

		it= m_mUInt.find(str);
		if(it == m_mUInt.end())
			return 0;
		return it->second;
	}

	unsigned long FileDescriptor::getULong(const string& str) const
	{
		map<string, unsigned long>::const_iterator it;

		it= m_mULong.find(str);
		if(it == m_mULong.end())
			return 0;
		return it->second;
	}

	unsigned long long FileDescriptor::getULongLong(const string& str) const
	{
		map<string, unsigned long long>::const_iterator it;

		it= m_mULongLong.find(str);
		if(it == m_mULongLong.end())
			return 0;
		return it->second;
	}

	void FileDescriptor::setFloat(const string& str, const float value)
	{
		m_mFloat[str]= value;
	}

	float FileDescriptor::getFloat(const string& str) const
	{
		map<string, float>::const_iterator it;

		it= m_mFloat.find(str);
		if(it == m_mFloat.end())
			return 0;
		return it->second;
	}

	void FileDescriptor::setDouble(const string& str, const double value)
	{
		m_mDouble[str]= value;
	}

	double FileDescriptor::getDouble(const string& str) const
	{
		map<string, double>::const_iterator it;

		it= m_mDouble.find(str);
		if(it == m_mDouble.end())
			return 0;
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

	string FileDescriptor::sendToOtherClient(const string& definition, const IMethodStringStream& str,
												const bool& wait, const string& endString)
	{
		string answer;
		IServerCommunicationStarterPattern* starter;
		IClientHolderPattern* holder= m_poServer->getCommunicationFactory();
		IClientPattern* client;

		//std::cout << "sendToOtherClient search other client " << definition << std::endl;
		starter= dynamic_cast<IServerCommunicationStarterPattern*>(holder);
		UNLOCK(m_THREADSAVEMETHODS);
		client= starter->getClient(definition, this);
		if(client == NULL)
		{
			time_t t, nt;

#ifdef ALLOCATEONMETHODSERVER
			std::cout << "no client found for " << definition << " search again for " << m_nTimeout << " seconds " << std::endl;
			std::cout << "  to send: " << str << std::endl;
#endif // ALLOCATEONMETHODSERVER
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		if(getBoolean("output") == true)
		{
			ostringstream out;

			out << "sendDescriptor " << getString("process") << "::" << getString("client");
			out << " want send question '" << str.str(true);
			out << "' but do not find client with definition " << definition << std::endl;
			cout << out.str();
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
			time(&nt);
			time(&t);
			while(	client == NULL
					&&
					!eof()
					&&
					(t - nt) < (time_t)m_nTimeout	)
			{
#ifdef ALLOCATEONMETHODSERVER
				std::cout << "wait for client " << definition << " since " << (t - nt) << " seconds" << std::endl;
#endif // ALLOCATEONMETHODSERVER
				sleep(1);
				client= starter->getClient(definition, this);
				time(&t);
			}
			if(client == NULL)
			{
				//std::cout << "found no client, return message ERROR 001" << std::endl;
				LOCK(m_THREADSAVEMETHODS);
				return "ERROR 001";
			}
		}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		if(getBoolean("output") == true)
		{
			ostringstream out;

			out << "sendDescriptor " << getString("process") << "::" << getString("client");
			out << " send question '" << str.str(true);
			out << "' to client with found definition " << definition << std::endl;
			cout << out.str();
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		answer= client->sendString(str, wait, endString);
		LOCK(m_THREADSAVEMETHODS);
		return answer;
	}

	string FileDescriptor::sendString(const IMethodStringStream& str, const bool& wait, const string& endString)
	{
		typedef vector< SHAREDPTR::shared_ptr<IMethodStringStream> >::iterator MethodIter;
		bool bRightAnswer;
		string answer;
		unsigned long long qSyncID(str.getSyncID());

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		bool boutput(true);
#ifndef __FOLLOW_FROMROCESS
#ifndef __FOLLOW_FROMCLIENT
#ifndef __FOLLOW_SENDMESSAGE
#ifdef __FOLLOW_TOPROCESS
		if(descriptor.getString("process") != __FOLLOW_TOPROCESS)
			boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_TOCLIENT
		if(descriptor.getString("client") != __FOLLOW_TOCLIENT)
			boutput= false;
#endif // __FOLLOW_FROMCLIENT
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_TOCLIENT
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOPROCESS
		if(descriptor.getString("process") != __FOLLOW_TOPROCESS)
			boutput= false;
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOCLIENT
		if(descriptor.getString("client") != __FOLLOW_TOCLIENT)
			boutput= false;
#endif // __FOLLOW_TOCLIENT
#ifdef __FOLLOW_SENDMESSAGE
		string sendmsg(__FOLLOW_SENDMESSAGE);
		string actmsg(str.str());

		if(actmsg.substr(0, sendmsg.length()) != sendmsg)
			boutput= false;
#endif // __FOLLOW_SENDMASSAGE
		if(boutput == true)
		{
			ostringstream out;

			out << "sendDescriptor " << getString("process") << "::" << getString("client");
			out << " try to send question '" << str.str(true);
			out << "' to client" << std::endl;
			cout << out.str();
		}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		LOCK(m_SENDSTRING);
		if(wait)
		{
			while(m_sSendString != "")
			{ // if SendString is not null,
			  // an other client wait for answer
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(boutput == true)
				{
					ostringstream out;

					out << "sendDescriptor " << getString("process") << "::" << getString("client");
					out << " want send question '" << str.str(true);
					out << "' to " << getServerObject()->getName();
					out << ", but other question '" << m_sSendString << "' wait also for answer" << std::endl;
					out << " so wait before until this answer was given" << std::endl;
					cout << out.str();
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
				CONDITION(m_SENDSTRINGCONDITION, m_SENDSTRING);
			}
			m_bWait= wait;
			m_sEndingString= endString;
			m_sSendString= str.str(true);
			AROUSEALL(m_GETSTRINGCONDITION);

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(	boutput == true &&
				!m_vsClientAnswer.empty()			)
			{
				ostringstream out;

				out << "sendDescriptor " << getString("process") << "::" << getString("client");
				out << " want send question '" << str.str(true);
				out << "' to " << getServerObject()->getName();
				out << ", follow answers be set:" << std::endl;
				for(MethodIter it= m_vsClientAnswer.begin();
								it != m_vsClientAnswer.end(); ++it)
				{
					out << "          '" << (*it)->str(true) << "'" << std::endl;
				}
				cout << out.str();
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			do{
				RELTIMECONDITION(m_SENDSTRINGCONDITION, m_SENDSTRING, 3);
				if(eof())
				{
					ostringstream answ;

					if(qSyncID > 0)
						answ << "syncID " << qSyncID << " ";
					answ << "ERROR 002";
					answer= answ.str();
					break;
				}
				bRightAnswer= false;
				if(!m_vsClientAnswer.empty())
				{
					for(MethodIter it= m_vsClientAnswer.begin();
									it != m_vsClientAnswer.end(); ++it)
					{
						if(qSyncID == (*it)->getSyncID())
						{
							bRightAnswer= true;
							answer= (*it)->str(true);
							m_vsClientAnswer.erase(it);
							break;
						}
					}

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
					if(	!bRightAnswer &&
						boutput == true	)
					{
						ostringstream out;

						out << "sendDescriptor " 	<< getString("process") << "::" << getString("client");
						out << " wait for answer with ";
						if(qSyncID > 0)
							out << "syncID(" << qSyncID << ") ";
						else
							out << "no syncID ";
						out << "but only follow answers be set:" << std::endl;
						for(MethodIter it= m_vsClientAnswer.begin();
										it != m_vsClientAnswer.end(); ++it)
						{
							out << "          '" << (*it)->str(true) << "'" << std::endl;
						}
						cout << out.str();
					}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

				}
			}while(!bRightAnswer);
			m_sSendString= "";
			AROUSE(m_SENDSTRINGCONDITION);
		}else
		{// write questions with need no answer only into a queue
		 // witch can complete answer client by time
			m_qsEndingStrings.push(endString);
			m_qsSendStrings.push(str.str(true));
			AROUSE(m_GETSTRINGCONDITION);
			answer= endString;
		}

		UNLOCK(m_SENDSTRING);
		return answer;
	}

	string FileDescriptor::getOtherClientString(bool& doWait, string& endString, const bool wait/*= true*/)
	{
		const unsigned short nDo= 5;
		static unsigned short count= 0;
		string str;
		string waitEnding;
		string waitStr;

		LOCK(m_SENDSTRING);
		if(m_sSendString == "")
		{
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(getBoolean("output") == true)
			{
				cout << getString("process") << "::" << getString("client");
				cout << " no send-string for new question be set,";
				if(m_qsSendStrings.empty())
				{
					cout << " and none inside queue,";
					if(!wait)
						cout << " and no client wait for answering";
					else
						cout << " wait for new string";
					cout << std::endl;
				}else
				{
					queue<string> output(m_qsSendStrings);

					endString= m_qsEndingStrings.front();
					cout << " take first string from queue as question";
					if(!m_qsEndingStrings.empty())
						cout << ", with end-string '" << m_qsEndingStrings.front() << "'";
					else
						cout << " (no end-string be set)";
					cout << std::endl;
					while(!output.empty())
					{
						cout << "       " << output.front() << std::endl;
						output.pop();
					}
				}
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			if(m_qsSendStrings.empty())
			{
				count= 0;
				if(!wait)
				{
					UNLOCK(m_SENDSTRING);
					return "";
				}
				UNLOCK(m_THREADSAVEMETHODS);
				CONDITION(m_GETSTRINGCONDITION, m_SENDSTRING);
				LOCK(m_THREADSAVEMETHODS);
			}
		}
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		else
			cout << "current send-string is '" << m_sSendString << "'" << std::endl;
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		if(	m_sSendString == ""
			||
			count > nDo			)	// when sending client always trigger SendString's
									// but there are also strings in the SendString queue
		{							// send some times also strings from there
			waitStr= m_sSendString;
			waitEnding= m_sEndingString;
			if(m_qsSendStrings.size())
			{
				m_sSendString= m_qsSendStrings.front();
				m_qsSendStrings.pop();
				m_bWait= false; // from the queue only no waiting strings
				m_sEndingString= m_qsEndingStrings.front();
				m_qsEndingStrings.pop();

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
				if(getBoolean("output") == true)
				{
					queue<string> output(m_qsSendStrings);

					cout << getString("process") << "::" << getString("client");
					cout << " trigger from queue first string";
					if(m_sEndingString != "")
						cout << ", with end-string '" << m_qsEndingStrings.front() << "'";
					else
						cout << " (no end-string be set)";
					cout << std::endl;
					while(!output.empty())
					{
						cout << "       " << output.front() << std::endl;
						output.pop();
					}
				}
#endif // __FOLLOWSERVERCLIENTTRANSACTION

			}else
				m_sSendString= "";
			count= 0;
		}else
			++count;
		str= m_sSendString;

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(getBoolean("output") == true)
			{
				cout << getString("process") << "::" << getString("client");
				cout << " take setting send-string from other client '" << str << "'" << std::endl;
			}
#endif // __FOLLOWSERVERCLIENTTRANSACTION
		doWait= m_bWait;
		endString= m_sEndingString;
		m_sSendString= waitStr;
		m_bWait= doWait;
		m_sEndingString= waitEnding;
		UNLOCK(m_SENDSTRING);
		return str;
	}

	void FileDescriptor::sendAnswer(const string& asw)
	{
		SHAREDPTR::shared_ptr<IMethodStringStream> sharedMethod;

		sharedMethod= SHAREDPTR::shared_ptr<IMethodStringStream>(new IMethodStringStream(asw));
		LOCK(m_SENDSTRING);
		m_vsClientAnswer.push_back(sharedMethod);
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

	void FileDescriptor::closeConnection()
	{
		LOCK(m_THREADSAVEMETHODS);
		if(m_bFileAccess)
		{
			fclose(m_pFile);
			m_bFileAccess= false;
			AROUSEALL(m_SENDSTRINGCONDITION);
			AROUSEALL(m_GETSTRINGCONDITION);
			UNLOCK(m_THREADSAVEMETHODS);
			usleep(10000);	// to give foreign clients and own
			return;			// an chance to ending correctly
		}
		UNLOCK(m_THREADSAVEMETHODS);
	}

	FileDescriptor::~FileDescriptor()
	{
		closeConnection();
		DESTROYMUTEX(m_SENDSTRING);
		DESTROYMUTEX(m_CONNECTIONIDACCESS);
		DESTROYMUTEX(m_THREADSAVEMETHODS);
		DESTROYCOND(m_SENDSTRINGCONDITION);
		DESTROYCOND(m_GETSTRINGCONDITION);
	}

}
