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
#include <unistd.h>
#include <limits.h>

#include <iostream>
#include <utility>

#include <boost/algorithm/string/trim.hpp>

#include "../../pattern/server/ITransferPattern.h"
#include "../../pattern/server/IClientHolderPattern.h"
#include "../../pattern/server/IServerCommunicationStarterPattern.h"

#include "../../util/debugtransaction.h"
#include "../../util/GlobalStaticMethods.h"

#include "../../util/stream/OMethodStringStream.h"

#include "SocketErrorHandling.h"
#include "FileDescriptor.h"

using namespace std;
using namespace design_pattern_world::server_pattern;
using namespace boost;

namespace server
{
	void FileDescriptor::initial(IServerPattern* server, ITransferPattern* transfer, int file, string address,
			const unsigned short port)
	{
		m_pTransfer= transfer;
		m_poServer= server;
		m_pHearingClient= NULL;
		m_unConnID= 0;
		m_bFileAccess= true;
		m_sAddress= address;
		m_nPort= port;
		m_nFd= file;
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");
		m_SENDSTRING= Thread::getMutex("SENDSTRING");
		m_THREADSAVEMETHODS= Thread::getMutex("THREADSAVEMETHODS");
		m_SENDSTRINGCONDITION= Thread::getCondition("SENDSTRINGCONDITION");
		m_GETSTRINGCONDITION= Thread::getCondition("GETSTRINGCONDITION");
	}

	EHObj FileDescriptor::init()
	{
		EHObj pRv(new SocketErrorHandling);

		LOCK(m_THREADSAVEMETHODS);
		if(m_pTransfer)
		{
			m_oSocketError= m_pTransfer->init(*this);
			(*pRv)= m_oSocketError;
		}
		UNLOCK(m_THREADSAVEMETHODS);
		return pRv;
	}

	EHObj FileDescriptor::getErrorObj() const
	{
		EHObj pRv(new SocketErrorHandling);

		(*pRv)= m_oSocketError;
		return pRv;
	}

	void FileDescriptor::operator >> (string &reader)
	{
		bool locked(true);
		ssize_t getLen(0);
		char buf[1026];
		string::size_type endPos;
		string::size_type bufLen(sizeof(buf)-2);
		string sread, process;
#if (__DEBUGLASTREADWRITECHECK)
		bool bbool, first(true);
		string sbuf;
		ostringstream lengths;
#endif // __DEBUGLASTREADWRITECHECK

		if(bufLen > SSIZE_MAX)
			bufLen= SSIZE_MAX;
		process= getString("process") + ":";
		process+= getString("client");
		sread= m_mLastRead[process];
		if(sread != "")
			getLen= sread.length();
		reader= "";
		if(eof())
			return;

		if(TRYLOCK(m_THREADSAVEMETHODS) == 0)
			locked= false;// lock done, so lock wasn't set
		UNLOCK(m_THREADSAVEMETHODS);
		do{
			if(getLen <= 0)
			{// otherwise an string was reading before
				getLen= read(m_nFd, buf, bufLen);
				if(getLen > 0)
				{
					buf[getLen]= '\0';
					sread= buf;
#if (__DEBUGLASTREADWRITECHECK)
					m_tLastRead.setActTime();
					if(first)
					{
						istringstream stream(sread);
						m_sReadLengths= "";
						m_sLastReadCommand= "";
						stream >> sbuf;// to process
						if(!stream.fail())
						{
							stream >> std::boolalpha >> bbool;// need answer
							if(!stream.fail())
							{
								stream >> std::boolalpha >> bbool;// more than one rows
								if(!stream.fail() && bbool)
									stream >> sbuf;// end command of more rows
								if(!stream.fail())
									stream >> m_sLastReadCommand;
							}
						}
						first= false;
					}
					lengths.str("");
					lengths << getLen << ", ";
					m_sReadLengths+= lengths.str();
					if(sread.length() > 25)
					{
						m_sLastRead= sread.substr(0, 5);
						m_sLastRead+= " < ... > ";
						m_sLastRead+= sread.substr(sread.length() - 20);
					}else
						m_sLastRead= sread;
#endif //__DEBUGLASTREADWRITECHECK

				}else
				{
					int nErrno(errno);
					ostringstream decl;

					decl << getHostAddressName() << "@";
					decl << getPort() << "@";
					decl << getTransactionName();
					if(getLen < 0)
					{
						if(	nErrno != EAGAIN &&
							nErrno != EWOULDBLOCK	)
						{
							m_oSocketError.setAddrError("FileDescriptor", "read", 0,
											nErrno, decl.str());
						}
					}else
					{// get null string, connection is broken
						m_oSocketError.setError("FileDescriptor", "noConnect", decl.str());
					}
					sread= "";
					if(	getLen == 0 ||
						(	nErrno != EAGAIN &&
							nErrno != EWOULDBLOCK	)	)
					{
						if(m_mLastRead[process] != "")
						{
							reader+= m_mLastRead[process];
							m_mLastRead[process]= "";
						}
						break;
					}
				}
			}
			endPos= sread.find("\n");
			if(	endPos != string::npos &&
				endPos < (sread.length() - 1)	)
			{
				reader+= sread.substr(0, endPos + 1);
				m_mLastRead[process]= sread.substr(endPos + 1);
#if 0
				ostringstream sout;
				ostringstream thread;
				size_t readerLen(reader.length());

				if(glob::getProcessName() != "ppi-client")
				{
					thread << "[" << glob::getProcessName() << " ("<< Thread::gettid() << ")] ";
					sout << "FileDescriptor cutting string of length " << reader.length() << " --\n";
					sout << "from |" + reader.substr(0, 10);
					if(readerLen > 10)
					{
						sout << " -- to -- ";
						if((readerLen - 10) > 10)
							sout << reader.substr(reader.length() - 10);
						else
							sout << reader.substr(10);
					}
					sout << "|cut by|" + m_mLastRead[process].substr(0, 10) + " ...\n";
					if(endPos > 10)
						endPos-= 10;
					else
						endPos= 0;
					sout << " original ... " + sread.substr(endPos, 20) + " ...\n";
					std::cout << glob::addPrefix(thread.str(), sout.str());
				}
#endif
			}else
			{
				reader+= sread;
				m_mLastRead[process]= "";
			}
			sread= "";
			getLen= 0;

		}while(endPos == string::npos && !eof());
		if(locked)
			LOCK(m_THREADSAVEMETHODS);
	}

	void FileDescriptor::operator << (const string& writer)
	{
		m_sSendTransaction+= writer;
		if(	m_bAutoSending &&
			writer.find('\n') != string::npos	)
		{
			flush();
		}
	}

	inline bool FileDescriptor::eof() const
	{
		if(m_oSocketError.hasError("FileDescriptor", "noConnect"))
			return true;
		return false;
	}

	void FileDescriptor::flush()
	{
		ssize_t writeLen;
		string::size_type len;
#if (__DEBUGLASTREADWRITECHECK)
		bool bbool, first(true);
		string sbuf;
		ostringstream lengths;
#endif // __DEBUGLASTREADWRITECHECK

		if(eof())
			return;
		while(!m_sSendTransaction.empty())
		{
			len= m_sSendTransaction.length();
			writeLen= write(m_nFd, m_sSendTransaction.c_str(), len);
#if (__DEBUGLASTREADWRITECHECK)
			string buf(m_sSendTransaction.substr(0, writeLen));

			m_tLastWrite.setActTime();
			if(first)
			{
				istringstream stream(m_sSendTransaction);

				m_sWriteLengths= "";
				m_sLastWriteCommand= "";
				stream >> sbuf;// to process
				if(!stream.fail())
				{
					stream >> std::boolalpha >> bbool;// need answer
					if(!stream.fail())
					{
						stream >> std::boolalpha >> bbool;// more than one rows
						if(!stream.fail() && bbool)
							stream >> sbuf;// end command of more rows
						if(!stream.fail())
							stream >> m_sLastWriteCommand;
					}
				}
				first= false;
			}
			lengths.str("");
			lengths << writeLen << ":" << len << ", ";
			m_sWriteLengths+= lengths.str();
			if(writeLen > 0)
			{
				if(buf.length() > 25)
				{
					m_sLastWrite= buf.substr(0, 5);
					m_sLastWrite+= " < ... > ";
					m_sLastWrite+= buf.substr(buf.length() - 20);
				}else
					m_sLastWrite= buf;
			}else
			{
				ostringstream out;

				out << "error by length(" << writeLen << ") " << strerror(errno);
				m_sLastWrite= out.str();
			}
#endif //__DEBUGLASTREADWRITECHECK
			if(writeLen < 0)
			{
				ostringstream decl;

				decl << getHostAddressName() << "@";
				decl << getPort() << "@";
				decl << getTransactionName();
				m_oSocketError.setAddrError("FileDescriptor", "write", 0,
								errno, decl.str());
				break;

			}else if(static_cast<size_t>(writeLen) < len)
				m_sSendTransaction= m_sSendTransaction.substr(writeLen);
			else
				m_sSendTransaction= "";
		}
	}

	inline void FileDescriptor::endl()
	{
		(*this) << "\n";
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

	IClientPattern* FileDescriptor::getOtherHearingClient(const string& definition)
	{
		IServerCommunicationStarterPattern* starter;
		IClientHolderPattern* holder= m_poServer->getCommunicationFactory();
		IClientPattern* client;

		//std::cout << "sendToOtherClient search other client " << definition << std::endl;
		starter= dynamic_cast<IServerCommunicationStarterPattern*>(holder);
		client= starter->getClient(definition, this);
		if(client == NULL)
		{
			time_t t, nt;

#ifdef ALLOCATEONMETHODSERVER
			std::cout << "no client found for " << definition << " search again for " << m_nTimeout << " seconds " << std::endl;
#endif // ALLOCATEONMETHODSERVER
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
				return NULL;
		}
		return client;
	}

	vector<string> FileDescriptor::sendToOtherClient(const string& definition, const IMethodStringStream& str,
												const bool& wait, const string& endString)
	{
		vector<string> answer;

		UNLOCK(m_THREADSAVEMETHODS);
		m_pHearingClient= getOtherHearingClient(definition);
		if(m_pHearingClient == NULL)
		{
			//std::cout << "found no client, return message ERROR 001" << std::endl;
			LOCK(m_THREADSAVEMETHODS);
			answer.push_back("ERROR 001");
			return answer;
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
		answer= m_pHearingClient->sendString(str, wait, endString);
		LOCK(m_THREADSAVEMETHODS);
		return answer;
	}

	vector<string> FileDescriptor::sendString(const IMethodStringStream& str, const bool& wait, const string& endString)
	{
		typedef vector< SHAREDPTR::shared_ptr<IMethodStringStream> >::iterator MethodIter;
		vector<string> answer;

#ifdef __FOLLOWSERVERCLIENTTRANSACTION
		bool boutput(true);
#ifndef __FOLLOW_FROMROCESS
#ifndef __FOLLOW_FROMCLIENT
#ifndef __FOLLOW_SENDMESSAGE
#ifdef __FOLLOW_TOPROCESS
		if(getString("process") != __FOLLOW_TOPROCESS)
			boutput= false;
#endif // __FOLLOW_FROMPROCESS
#ifdef __FOLLOW_TOCLIENT
		if(getString("client") != __FOLLOW_TOCLIENT)
			boutput= false;
#endif // __FOLLOW_FROMCLIENT
#endif // __FOLLOW_SENDMESSAGE
#endif // __FOLLOW_TOCLIENT
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOPROCESS
		if(getString("process") != __FOLLOW_TOPROCESS)
			boutput= false;
#endif // __FOLLOW_TOPROCESS
#ifdef __FOLLOW_TOCLIENT
		if(getString("client") != __FOLLOW_TOCLIENT)
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

			UNLOCK(m_SENDSTRING);
			answer= getMoreAnswers(str.getSyncID(), endString);
			LOCK(m_SENDSTRING);
			m_sSendString= "";
			AROUSE(m_SENDSTRINGCONDITION);
		}else
		{// write questions with need no answer only into a queue
		 // witch can complete answer client by time
			m_bWait= wait;
			m_qsEndingStrings.push(endString);
			m_qsSendStrings.push(str.str(true));
			AROUSE(m_GETSTRINGCONDITION);
			answer.push_back(endString);
		}

		UNLOCK(m_SENDSTRING);
		return answer;
	}

	vector<string> FileDescriptor::getMoreFromOtherClient(const unsigned long long syncID, const string& endString)
	{
		return m_pHearingClient->getMoreAnswers(syncID, endString);
	}

	vector<string> FileDescriptor::getMoreAnswers(const unsigned long long syncID, const string& endString)
	{
		typedef vector< SHAREDPTR::shared_ptr<IMethodStringStream> >::iterator MethodIter;
		bool bRightAnswer;
		string answer;
		vector<string> answers;

		LOCK(m_SENDSTRING);
		do{
			bRightAnswer= false;
			if(!m_vsClientAnswer.empty())
			{
				bool bErase;

				do{
					bErase= false;
					for(MethodIter it= m_vsClientAnswer.begin();
									it != m_vsClientAnswer.end(); ++it)
					{
						if(syncID == (*it)->getSyncID())
						{
							bRightAnswer= true;
							answers.push_back((*it)->str(true));
							answer= (*it)->str();
							trim(answer);
							m_vsClientAnswer.erase(it);
							bErase= true;
							break;
						}
					}
					if(	endString == "" ||
						endString == answer	)
					{
						break;
					}
				}while(bErase);
			}
			if(eof())
			{
				ostringstream answ;

				if(syncID > 0)
					answ << "syncID " << syncID << " ";
				answ << "ERROR 002";
				answers.push_back(answ.str());
				break;
			}
			if(!bRightAnswer)
				RELTIMECONDITION(m_SENDSTRINGCONDITION, m_SENDSTRING, 3);
		}while(!bRightAnswer);
		UNLOCK(m_SENDSTRING);
		return answers;
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

	void FileDescriptor::sendAnswer(const vector<string>& asw)
	{
		SHAREDPTR::shared_ptr<IMethodStringStream> sharedMethod;

		LOCK(m_SENDSTRING);
		for(vector<string>::const_iterator it= asw.begin(); it != asw.end(); ++it)
		{
			sharedMethod= SHAREDPTR::shared_ptr<IMethodStringStream>(new IMethodStringStream(*it));
			m_vsClientAnswer.push_back(sharedMethod);
		}
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
		if(m_pTransfer)
			name= m_pTransfer->getTransactionName(*this);
		else
			name= "[no transfer set for descriptor]";
		UNLOCK(m_THREADSAVEMETHODS);
		return name;
	}

	bool FileDescriptor::transfer()
	{
		bool bRv(true);

		LOCK(m_THREADSAVEMETHODS);
		if(m_pTransfer)
			bRv= m_pTransfer->transfer(*this);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	void FileDescriptor::closeConnection()
	{
		LOCK(m_THREADSAVEMETHODS);
		if(m_bFileAccess)
		{
			::close(m_nFd);
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
