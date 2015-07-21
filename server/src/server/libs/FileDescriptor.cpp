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
#include <csignal>

#include <iostream>
#include <utility>

#include <boost/algorithm/string/trim.hpp>

#include "../../pattern/util/LogHolderPattern.h"

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
#if (__DEBUGLASTREADWRITECHECK)
	pthread_mutex_t* FileDescriptor::m_SIGNALLOCK= Thread::getMutex("SIGNALLOCK");
	map<pid_t, FileDescriptor::lastReadWrite_t> FileDescriptor::m_mLastReadWrite=
					map<pid_t, FileDescriptor::lastReadWrite_t>();
#endif // __DEBUGLASTREADWRITECHECK

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
		m_nLockSM= 0;
		m_READWRITEMUTEX= Thread::getMutex("READWRITEMUTEX");
		m_CONNECTIONIDACCESS= Thread::getMutex("CONNECTIONIDACCESS");
		m_SENDSTRING= Thread::getMutex("SENDSTRING");
		m_THREADSAVEMETHODS= Thread::getMutex("THREADSAVEMETHODS");
		m_LOCKSM= Thread::getMutex("LOCKSM");
		m_SENDSTRINGCONDITION= Thread::getCondition("SENDSTRINGCONDITION");
		m_GETSTRINGCONDITION= Thread::getCondition("GETSTRINGCONDITION");

#if (__DEBUGLASTREADWRITECHECK)
		if(signal(SIGHUP, SIGHUPconverting) == SIG_ERR)
			glob::printSigError("SIGHUP", "SocketClientConnection");
#endif // __DEBUGLASTREADWRITECHECK
	}

#if (__DEBUGLASTREADWRITECHECK)
	void FileDescriptor::SIGHUPconverting(int signal)
	{
		ostringstream output;
		pid_t id(Thread::gettid());

		/*
		 * signal should be always SIGHUP
		 */
		output << "----------------------------------------------------------------------------" << std::endl;
		output << " thread [" << id << "]" << std::endl;
		LOCK(m_SIGNALLOCK);
		for(deque<pair<ppi_time, string> >::iterator it= m_mLastReadWrite[id].lastRead.begin();
						it != m_mLastReadWrite[id].lastRead.end(); ++it			)
		{
			output << "last read " << it->first.toString(/*date*/true) << ":" << std::endl;
			output << ">>'" << it->second << "'<<" << std::endl;
		}
		for(deque<pair<ppi_time, string> >::iterator it= m_mLastReadWrite[id].lastWrite.begin();
						it != m_mLastReadWrite[id].lastWrite.end(); ++it			)
		{
			output << "last write " << it->first.toString(/*date*/true) << ":" << std::endl;
			output << ">>'" << it->second << "'<<" << std::endl;
		}
		UNLOCK(m_SIGNALLOCK);
		output << "----------------------------------------------------------------------------" << std::endl;
		cout << output.str();
	}

	void FileDescriptor::setWriting(const string& write)
	{
		pid_t id(Thread::gettid());
		ppi_time tm;

		tm.setActTime();
		LOCK(m_SIGNALLOCK);
		m_mLastReadWrite[id].lastWrite.push_back(pair<ppi_time, string>(tm, write));
		if(m_mLastReadWrite[id].lastWrite.size() > 3)
			m_mLastReadWrite[id].lastWrite.pop_front();
		UNLOCK(m_SIGNALLOCK);
	}

	void FileDescriptor::setReading(const string& read)
	{
		pid_t id(Thread::gettid());
		ppi_time tm;

		tm.setActTime();
		LOCK(m_SIGNALLOCK);
		m_mLastReadWrite[id].lastRead.push_back(pair<ppi_time, string>(tm, read));
		if(m_mLastReadWrite[id].lastRead.size() > 10)
			m_mLastReadWrite[id].lastRead.pop_front();
		UNLOCK(m_SIGNALLOCK);
	}
#endif // __DEBUGLASTREADWRITECHECK

	EHObj FileDescriptor::init()
	{
		EHObj pRv(new SocketErrorHandling);

		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
		if(m_pTransfer)
		{
			m_oSocketError= m_pTransfer->init(*this);
			(*pRv)= m_oSocketError;
		}
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
		return pRv;
	}

	EHObj FileDescriptor::getErrorObj() const
	{
		EHObj pRv(new SocketErrorHandling);

		(*pRv)= m_oSocketError;
		return pRv;
	}

	bool FileDescriptor::unlockTHREADSAVEMETHODS(const string& file, int line)
	{
		bool locked;
		bool ownLock;

		if(Thread::mutex_trylock(file, line, m_THREADSAVEMETHODS) == EBUSY)
		{
			/*
			 * lock for THREADSAVEMETHODS is set
			 * so check no whether own thread
			 * has it locked
			 */
			Thread::mutex_lock(file, line, m_LOCKSM);
			if(m_nLockSM == Thread::gettid())
			{
				locked= true;
				ownLock= true;
			}else
			{
				locked= false;
				ownLock= false;
			}
			Thread::mutex_unlock(file, line, m_LOCKSM);
		}else
		{
			/*
			 * lock done from own thread,
			 * so lock wasn't set
			 */
			ownLock= true;
			locked= false;
		}
		if(ownLock)
		{
			Thread::mutex_lock(file, line, m_LOCKSM);
			m_nLockSM= 0;
			Thread::mutex_unlock(file, line, m_LOCKSM);
			Thread::mutex_unlock(file, line, m_THREADSAVEMETHODS);
		}
		return locked;
	}

	void FileDescriptor::operator >> (string &reader)
	{
		bool locked;
		ssize_t getLen(0);
		char buf[1026];
		//char buf[4];
		string::size_type endPos;
		string::size_type bufLen(sizeof(buf)-2);
		string sread;

		LOCK(m_READWRITEMUTEX);
		if(bufLen > SSIZE_MAX)
			bufLen= SSIZE_MAX;
//		process= getString("process") + ":";
//		process+= getString("client");
		sread= m_mLastRead;//[process];
		//m_mLastRead[process]= "";
		m_mLastRead= "";
		if(sread != "")
			getLen= sread.length();
		reader= "";
		if(fail())
		{
			/*
			 * when connection was fail
			 * give also last reading
			 * content back
			 */
			reader= sread;
#if(__DEBUGLASTREADWRITECHECK)
			cout << "[" << Thread::gettid() << "] ";
			cout << "return reading descriptor by fail '" << reader << "'" << std::endl;
			cout << "            '" << m_oSocketError.getErrorStr() << "'" << std::endl;
			cout << "            '" << m_oSocketError.getDescription() << std::endl;
#endif // __DEBUGLASTREADWRITECHECK
			UNLOCK(m_READWRITEMUTEX);
			return;
		}

		locked= UNLOCK_THREADSAVEMETHODS();
		do{
			if(getLen <= 0)
			{// otherwise an string was reading before
				getLen= read(m_nFd, buf, bufLen);
				if(getLen > 0)
				{
					buf[getLen]= '\0';
					sread= buf;
#if (__DEBUGLASTREADWRITECHECK)
					setReading(sread);
#endif //__DEBUGLASTREADWRITECHECK

				}else
				{
					int nErrno(errno);
					string errStr;
					ostringstream decl;

					decl << getHostAddressName() << "@";
					decl << getPort();
					if(m_pTransfer)
						decl << "@" << getTransactionName();
					if(	getLen < 0 ||
						nErrno != 0		)
					{
						if(	nErrno != EAGAIN &&
							nErrno != EWOULDBLOCK	)
						{
							if(m_pTransfer)
								errStr= "transRead";
							else
								errStr= "read";
							m_oSocketError.setAddrError("FileDescriptor", errStr, 0,
											nErrno, decl.str());
						}
					}else
					{// get null string, connection is broken
						if(m_pTransfer)
							errStr= "transNoConnect";
						else
							errStr= "noConnect";
						m_oSocketError.setError("FileDescriptor", errStr, decl.str());
					}
#if (__DEBUGLASTREADWRITECHECK)
					setReading(string("error reading:: ")+m_oSocketError.getDescription());
#endif //__DEBUGLASTREADWRITECHECK
					sread= "";
					if(	getLen == 0 ||
						(	nErrno != EAGAIN &&
							nErrno != EWOULDBLOCK	)	)
					{
						if(locked)
						{
							LOCK(m_THREADSAVEMETHODS);
							LOCK(m_LOCKSM);
							m_nLockSM= Thread::gettid();
							UNLOCK(m_LOCKSM);
							locked= false;
						}
						//if(m_mLastRead[process] != "")
						if(m_mLastRead != "")
						{
							//reader+= m_mLastRead[process];
							reader+= m_mLastRead;
							//m_mLastRead[process]= "";
							m_mLastRead= "";
						}
						break;
					}
				}
			}
			endPos= sread.find("\n");
			if(	endPos != string::npos &&
				endPos < (sread.length() - 1)	)
			{
				if(locked)
				{
					LOCK(m_THREADSAVEMETHODS);
					LOCK(m_LOCKSM);
					m_nLockSM= Thread::gettid();
					UNLOCK(m_LOCKSM);
					locked= false;
				}
				reader+= sread.substr(0, endPos + 1);
				m_mLastRead= sread.substr(endPos + 1);
			}else
			{
				reader+= sread;
			}
			sread= "";
			getLen= 0;

		}while(endPos == string::npos && !fail());
		if(locked)
		{
			LOCK(m_THREADSAVEMETHODS);
			LOCK(m_LOCKSM);
			m_nLockSM= Thread::gettid();
			UNLOCK(m_LOCKSM);
		}
		if(	fail() &&
			m_mLastRead != ""	)
		{
			/*
			 * when connection fail
			 * give also rest of string back
			 */
			reader+= m_mLastRead;
			cout << "[" << Thread::gettid() << "] ";
			cout << "return descriptor by 2. fail '" << reader << "'" << std::endl;
			m_mLastRead= "";
		}
		if(	!fail() &&
			(	reader.length() == 0 ||
				reader.substr(reader.length()-1) != "\n"	)	)
		{
			ostringstream out;

			out << "[" << Thread::gettid() << "] ";
			out << "read string with no carriage return on end" << std::endl;
			out << "'" << m_sSendTransaction << "'" << std::endl;
			cout << out.str();
		}
		UNLOCK(m_READWRITEMUTEX);
	}

	void FileDescriptor::operator << (const string& writer)
	{
		LOCK(m_READWRITEMUTEX);
		m_sSendTransaction+= writer;
		if(	m_bAutoSending &&
			writer.find('\n') != string::npos	)
		{
			Iflush();
		}
		UNLOCK(m_READWRITEMUTEX);
	}

	inline bool FileDescriptor::eof() const
	{
		if(	m_oSocketError.hasError("FileDescriptor", "noConnect") ||
			m_oSocketError.hasError("FileDescriptor", "transNoConnect")	)
		{
			return true;
		}
		return false;
	}

	void FileDescriptor::flush()
	{
		LOCK(m_READWRITEMUTEX);
		Iflush();
		UNLOCK(m_READWRITEMUTEX);
	}

	void FileDescriptor::Iflush()
	{
#ifdef DEBUG
		bool bSendLower(false);
#endif // DEBUG
		ssize_t writeLen;
		string::size_type len;

		if(eof())
			return;
		while(!m_sSendTransaction.empty())
		{
			len= m_sSendTransaction.length();
			/*ostringstream send;
			send << "send: '" << m_sSendTransaction << "'" << std::endl;
			cout << send.str();*/
			writeLen= write(m_nFd, m_sSendTransaction.c_str(), len);
#if(__DEBUGLASTREADWRITECHECK)
			if(writeLen >= 0)
			{
				string::size_type wlen(static_cast<string::size_type>(writeLen));

				if(	len > wlen  ||
					wlen == 0 ||
					m_sSendTransaction.substr(wlen-1, 1) != "\n"	)
				{
					ostringstream out;

					out << "[" << Thread::gettid() << "] ";
					if(len > wlen)
					{
						out << "write " << (len - wlen) << " chars shorter length than "
										<< len << std::endl << "'";
					}else
						out << "write string with no carriage return on end" << std::endl << "'";
					out << m_sSendTransaction << "'" << std::endl;
					cout << out.str();
				}
			}
#endif //__DEBUGLASTREADWRITECHECK
			if(writeLen < 0)
			{
				ostringstream decl;

				decl << getHostAddressName() << "@";
				decl << getPort();
				if(m_pTransfer)
				{
					bool locked;

					locked= UNLOCK_THREADSAVEMETHODS();
					decl << "@" << getITransactionName();
					m_oSocketError.setAddrError("FileDescriptor", "transWrite", 0,
									errno, decl.str());
					if(locked)
					{
						LOCK(m_THREADSAVEMETHODS);
						LOCK(m_LOCKSM);
						m_nLockSM= Thread::gettid();
						UNLOCK(m_LOCKSM);
					}
				}else
					m_oSocketError.setAddrError("FileDescriptor", "write", 0,
									errno, decl.str());
#if (__DEBUGLASTREADWRITECHECK)
				setWriting(string("writing ERROR:: ")+m_oSocketError.getDescription());
#endif //__DEBUGLASTREADWRITECHECK
				break;

			}else
			{
#if (__DEBUGLASTREADWRITECHECK)
				if(writeLen == 0)
					setWriting(string("write 0 should:: ")+m_sSendTransaction);
				else
					setWriting(m_sSendTransaction.substr(0, writeLen));
#endif //__DEBUGLASTREADWRITECHECK
				if(static_cast<size_t>(writeLen) < len)
				{
#ifdef DEBUG
					ostringstream out;

					out << "want to send string of " << len << " characters:" << std::endl;
					if(len > 100)
						out << m_sSendTransaction.substr(0, 50) << " ... " << m_sSendTransaction.substr(len - 50);
					else
						out << m_sSendTransaction;
					if(m_sSendTransaction.substr(len -1) != "\n")
						out << std::endl;
					out << "but sending only " << writeLen << " characters:" << std::endl;
					if(writeLen > 100)
						out << m_sSendTransaction.substr(0, 50) << " ... " << m_sSendTransaction.substr(writeLen - 50, 50);
					else
						out << m_sSendTransaction.substr(0, writeLen);
					out << std::endl;
					cout << glob::addPrefix("### WARNING: ", out.str()) << std::endl << std::endl;
					bSendLower= true;
#endif // DEBUG
					m_sSendTransaction= m_sSendTransaction.substr(writeLen);
				}else
				{
#ifdef DEBUG
					if(bSendLower)
					{
						ostringstream out;

						out << "sending end of string with " << len << " characters";
						if(m_sSendTransaction.substr(len -1) != "\n")
							out << " but without carriage return";
						out << ":" << std::endl;
						if(len > 100)
							out << m_sSendTransaction.substr(0, 50) << " ... " << m_sSendTransaction.substr(len - 50);
						else
							out << m_sSendTransaction;
						cout << glob::addPrefix("###: ", out.str()) << std::endl << std::endl;
					}
#endif // DEBUG
					m_sSendTransaction= "";
				}
			}
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

	IClientPattern* FileDescriptor::getOtherHearingClient(const string& process, const string& client,
												const unsigned int timeout, IClientPattern* after/*= NULL*/)
	{
		IServerCommunicationStarterPattern* starter;
		IClientHolderPattern* holder= m_poServer->getCommunicationFactory();
		IClientPattern* pClient;

		//std::cout << "sendToOtherClient search other client " << definition << std::endl;
		starter= dynamic_cast<IServerCommunicationStarterPattern*>(holder);
		pClient= starter->getClient(process, client, this, after);
		if(	pClient == NULL &&
			timeout > 0			)
		{
			time_t t, nt;
			ostringstream allocateOutput;

			allocateOutput << "no client found for " << client
							<< " search again for " << m_nTimeout << " seconds " << std::endl;
			time(&nt);
			time(&t);
			while(	pClient == NULL
					&&
					!eof()
					&&
					(t - nt) < (time_t)timeout	)
			{
				allocateOutput << "wait for client " << client << " since "
								<< (t - nt) << " seconds" << std::endl;
#ifdef ALLOCATEONMETHODSERVER
				cout << allocateOutput << std::endl;
#endif // ALLOCATEONMETHODSERVER
				LOG(LOG_DEBUG, allocateOutput.str());
				allocateOutput.str("");
				sleep(1);
				pClient= starter->getClient(process, client, this);
				time(&t);
			}
			if(pClient == NULL)
				return NULL;
		}
		return pClient;
	}

	vector<string> FileDescriptor::sendToOtherClient(const string& process, const string& client,
												const IMethodStringStream& str,
												const bool& wait, const string& endString)
	{
		vector<string> answer;
		string method(str.getMethodName());
		IMethodStringStream oInit("init");
		IClientPattern* pLast= NULL;

		if(method == UNEXPECTED_CLOSE)
			oInit.createSyncID(str.getSyncID());
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
		do{
			if(method == UNEXPECTED_CLOSE)
				pLast= getOtherHearingClient(process, pLast);
			else
				pLast= getOtherHearingClient(process, client, m_nTimeout);
			if(pLast == NULL)
			{
				SocketErrorHandling errHandle;

				errHandle.setError("FileDescriptor", "noHearingClient", client);
				LOCK(m_THREADSAVEMETHODS);
				LOCK(m_LOCKSM);
				m_nLockSM= Thread::gettid();
				UNLOCK(m_LOCKSM);
				answer.push_back(errHandle.getErrorStr());
				m_pHearingClient= NULL;
				return answer;
			}
	#ifdef __FOLLOWSERVERCLIENTTRANSACTION
			if(getBoolean("output") == true)
			{
				ostringstream out;

				out << "sendDescriptor " << getString("process") << "::" << getString("client");
				out << " send question '" << str.str(true);
				out << "' to client with found definition " << client << std::endl;
				cout << out.str();
			}
	#endif // __FOLLOWSERVERCLIENTTRANSACTION
			/*
			 * when server get an unexpected closing
			 * it should ask for all other clients with same process
			 * whether connection is holding
			 * do not send unexpected closing to other clients,
			 * but ask for initialization
			 */
			if(method == UNEXPECTED_CLOSE)
				answer= pLast->sendString(oInit, wait, endString);
			else
				answer= pLast->sendString(str, wait, endString);
		}while(method == UNEXPECTED_CLOSE);
		m_pHearingClient= pLast;
		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
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
		vector<string> vRv;

		if(m_pHearingClient == NULL)
		{
			if(endString != "")
				vRv.push_back(endString);
			return vRv;
		}
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
			if(fail())
			{
				ostringstream answ;

				if(syncID > 0)
					answ << "syncID " << syncID << " ";
				answ << m_oSocketError.getErrorStr();
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
				LOCK(m_LOCKSM);
				m_nLockSM= 0;
				UNLOCK(m_LOCKSM);
				UNLOCK(m_THREADSAVEMETHODS);
				CONDITION(m_GETSTRINGCONDITION, m_SENDSTRING);
				LOCK(m_THREADSAVEMETHODS);
				LOCK(m_LOCKSM);
				m_nLockSM= Thread::gettid();
				UNLOCK(m_LOCKSM);
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

	bool FileDescriptor::isClient(const string& process, const string& client) const
	{
		bool bRv;

		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
		bRv= m_pTransfer->hasNonAskingClient(*this, process, client);
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	string FileDescriptor::getITransactionName() const
	{
		string name;

		if(m_pTransfer)
			name= m_pTransfer->getTransactionName(*this);
		else
			name= "[no transfer set for descriptor]";
		return name;
	}

	string FileDescriptor::getTransactionName() const
	{
		string name;

		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
		name= getITransactionName();
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
		return name;
	}

	bool FileDescriptor::transfer()
	{
		bool bRv(true);

		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
		if(m_pTransfer)
		{
			bRv= m_pTransfer->transfer(*this);
			/*
			 * ppi@magnificat.at 23/05/2015:
			 * do not write getting transfer error
			 * into descriptor
			 * because when error comes from client
			 * should not be one
			 */
//			if(!bRv)
//				m_oSocketError= m_pTransfer->getErrorObj();
		}
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
		return bRv;
	}

	void FileDescriptor::closeConnection()
	{
		LOCK(m_THREADSAVEMETHODS);
		LOCK(m_LOCKSM);
		m_nLockSM= Thread::gettid();
		UNLOCK(m_LOCKSM);
		if(m_bFileAccess)
		{
			::close(m_nFd);
			m_bFileAccess= false;
			AROUSEALL(m_SENDSTRINGCONDITION);
			AROUSEALL(m_GETSTRINGCONDITION);
			LOCK(m_LOCKSM);
			m_nLockSM= 0;
			UNLOCK(m_LOCKSM);
			UNLOCK(m_THREADSAVEMETHODS);
			usleep(10000);	// to give foreign clients and own
			return;			// an chance to ending correctly
		}
		LOCK(m_LOCKSM);
		m_nLockSM= 0;
		UNLOCK(m_LOCKSM);
		UNLOCK(m_THREADSAVEMETHODS);
	}

	FileDescriptor::~FileDescriptor()
	{
		closeConnection();
		DESTROYMUTEX(m_READWRITEMUTEX);
		DESTROYMUTEX(m_SENDSTRING);
		DESTROYMUTEX(m_CONNECTIONIDACCESS);
		DESTROYMUTEX(m_THREADSAVEMETHODS);
		DESTROYMUTEX(m_LOCKSM);
		DESTROYCOND(m_SENDSTRINGCONDITION);
		DESTROYCOND(m_GETSTRINGCONDITION);
	}

}
