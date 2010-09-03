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
#include <algorithm>
#include <iostream>
#include <set>

#include "communicationthreadstarter.h"
#include "Communication.h"

using namespace std;

namespace server
{

	CommunicationThreadStarter* CommunicationThreadStarter::_instance= NULL;

	CommunicationThreadStarter::CommunicationThreadStarter(const string &threadName, const unsigned short& minThreads, const unsigned short& maxThreads)
	: 	Thread(threadName, 0),
		m_bWillStop(false),
		m_nNextFreeID(1),
		m_maxConnThreads(maxThreads),
		m_minConnThreads(minThreads)
	{
		m_NEXTCOMMUNICATION= getMutex("NEXTCOMMUNICATION");
		m_NEXTCOMMUNICATIONCOND= getCondition("NEXTCOMMUNICATIONCOND");
	}

	CommunicationThreadStarter::CommunicationThreadStarter(const unsigned short& minThreads, const unsigned short& maxThreads)
	: 	Thread("CommunicationThreadStarter", 0),
		m_bWillStop(false),
		m_nNextFreeID(1),
		m_maxConnThreads(maxThreads),
		m_minConnThreads(minThreads)
	{
		m_NEXTCOMMUNICATION= getMutex("NEXTCOMMUNICATION");
		m_NEXTCOMMUNICATIONCOND= getCondition("NEXTCOMMUNICATIONCOND");
	}

	int CommunicationThreadStarter::start(void *args/*= NULL*/, bool bHold/*= false*/)
	{
		if(m_minConnThreads == 0)
			return init(args);

		return Thread::start(args, bHold);
	}

	int CommunicationThreadStarter::init(void* args)
	{
		ICommunicationPattern* pCurrentCom;

		cout << "### start communication threads " << flush;
		pCurrentCom= getNewCommunicationThread(m_nNextFreeID);
		++m_nNextFreeID;
		//m_NEXTCOMMUNICATION= pCurrentCom->getMutex("NEXTCOMMUNICATION");
		pCurrentCom->start();
		m_poFirstCommunication= pCurrentCom;
		m_poNextFree= pCurrentCom;
		cout << "." << flush;
		for(unsigned short count= 1; count<m_maxConnThreads; ++count)
		{
			pCurrentCom->setNextComm(getNewCommunicationThread(m_nNextFreeID));
			++m_nNextFreeID;
			pCurrentCom= pCurrentCom->getNextComm();
			pCurrentCom->start();
			cout << "." << flush;
		}
		cout << endl;
		return 0;
	}

	IClientPattern* CommunicationThreadStarter::getClient(const string& definition, IFileDescriptorPattern* own) const
	{
		ICommunicationPattern* pCurrentCom;

		LOCK(m_NEXTCOMMUNICATION);
		pCurrentCom= m_poFirstCommunication;
		while(pCurrentCom)
		{
			if(	own != pCurrentCom->getDescriptor().get()
				&&
				pCurrentCom->hasClient()
				&&
				pCurrentCom->isClient(definition)	)
			{
				break;
			}
			pCurrentCom= pCurrentCom->getNextComm();
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return pCurrentCom;
	}

	ICommunicationPattern* CommunicationThreadStarter::getNewCommunicationThread(unsigned int nextID) const
	{
		return new Communication(nextID, this);
	}

	int CommunicationThreadStarter::execute()
	{
		bool bWillStop;
		bool bAllFilled;
		int conderror;

		LOCK(m_NEXTCOMMUNICATION);
		bWillStop= m_bWillStop;
		UNLOCK(m_NEXTCOMMUNICATION);
		if(!bWillStop)
		{
			bAllFilled= fillCommunicationThreads();
			checkThreads(m_poFirstCommunication);
			sleepDefaultTime();
			if(bAllFilled)
			{
				LOCK(m_NEXTCOMMUNICATION);
				conderror= 0;
				if(	!stopping()
					&&
					m_qpNewClients.empty()	)
				{
					conderror= CONDITION(m_NEXTCOMMUNICATIONCOND, m_NEXTCOMMUNICATION);
				}
				UNLOCK(m_NEXTCOMMUNICATION);
				if(	conderror
					&&
					conderror != EINTR	)
				{
					sleep(1);
				}
			}
		}
		return 0;
	}

	void CommunicationThreadStarter::arouseStarterThread() const
	{
		LOCK(m_NEXTCOMMUNICATION);
		AROUSE(m_NEXTCOMMUNICATIONCOND);
		UNLOCK(m_NEXTCOMMUNICATION);
	}

	unsigned int CommunicationThreadStarter::nextClientID()
	{
		unsigned int sid;
		unsigned int id= 1;
		ICommunicationPattern* cur= m_poFirstCommunication;
		set<unsigned int> setIds;
		set<unsigned int>::iterator it;

		LOCK(m_NEXTCOMMUNICATION);
		while(cur)
		{
			sid= cur->getConnectionID();
			if(sid == id)
				++id;
			else if(sid > id)
				setIds.insert(sid);
			cur= cur->getNextComm();
		}
		for(it= setIds.find(id); it != setIds.end(); ++it)
		{
			if(*it == id)
				++id;
			else
				break;
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return id;
	}

	void CommunicationThreadStarter::checkThreads(ICommunicationPattern* first)
	{
		unsigned short empty;
		unsigned int id;
		ICommunicationPattern* before= NULL;
		ICommunicationPattern* cur;
		vector<unsigned int>::iterator freeIt;

		LOCK(m_NEXTCOMMUNICATION);// lock NEXTCOMMUNICATION for while command on the end of the loop
#ifdef SHOWCLIENTSONSHELL
		cout << "CommunicationThreadStarter::checkThreads() by defined SHOWCLIENTSONSHELL" << endl;
#endif // SHOWCLIENTSONSHELL
		do{
			cur= first;
			UNLOCK(m_NEXTCOMMUNICATION);
			empty= 0;
			while(cur)
			{
				LOCK(m_NEXTCOMMUNICATION);
#ifdef SHOWCLIENTSONSHELL
				cout << dec << cur->getThreadID() << " has ";
#endif // SHOWCLIENTSONSHELL
				id= cur->getConnectionID();
				if(	id == 0
					&&
					m_qpNewClients.empty()	)
				{// Communication class have no client
				 // and no client waits in list
#ifdef SHOWCLIENTSONSHELL
					cout << "no client" << endl;
#endif // SHOWCLIENTSONSHELL
					if(empty == m_maxConnThreads)
					{// more than enough Communication classes exists
						if(cur->getNextComm())
						{
							if(before)
								before->setNextComm(cur->getNextComm());
							else
								before= cur->getNextComm();
						}else
							before->setNextComm(NULL);

						if(cur == m_poNextFree) // have same address
							m_poNextFree= NULL;
#ifdef SHOWCLIENTSONSHELL
						cout << "stop communication thread " << dec << cur->getThreadID() << endl;
						ostringstream seethread;
						seethread << "thread:";
						seethread << cur->getThreadID();
						cout << Thread::getStatusInfo(seethread.str()) << endl;
#endif // SHOWCLIENTSONSHELL
						UNLOCK(m_NEXTCOMMUNICATION);
						cur->stop(/*wait*/true);
						LOCK(m_NEXTCOMMUNICATION);
						if(cur == NULL)
							cout << "communication thread after stop is NULL" << endl;
						id= cur->getDefaultID();
#ifdef SHOWCLIENTSONSHELL
						cout << "delete thread " << dec << cur->getThreadID() << " with default id " << id << endl;
#endif // SHOWCLIENTSONSHELL
						delete cur;
						cur= before;
						if(id == (m_nNextFreeID -1))
						{
							// deleted communication-thread has the ID before the new next free ID
							// so search also in the container from the free ID's which are deleted before
							// if there any exists
							if(m_vFreeIDs.size())
							{
								sort(m_vFreeIDs.begin(), m_vFreeIDs.end());
								freeIt= m_vFreeIDs.end();
								do{
									--freeIt;
									if(*freeIt == (id -1))
									{
										m_vFreeIDs.erase(freeIt);
										--id;
									}else
										break;
								}while(freeIt != m_vFreeIDs.begin());
							}
							m_nNextFreeID= id;
						}else
						{ // deleted communication-thread head not the ID before the next free ID
						  // so give ID into the vFreeIDs container
							m_vFreeIDs.push_back(id);
						}
#ifdef SHOWCLIENTSONSHELL
						cout << "thread is deleted" << endl;
#endif // SHOWCLIENTSONSHELL
					}else
					{
						if(!m_poNextFree)
							m_poNextFree= cur;
						++empty;
					}
				}else
				{
					if(!m_qpNewClients.empty())
					{
						cur->connection(m_qpNewClients.front());
						m_qpNewClients.pop();
#ifdef SHOWCLIENTSONSHELL
						cout << "client with id " << cur->getDefaultID() << " gets new connection" << endl;
#endif // SHOWCLIENTSONSHELL
					}
#ifdef SHOWCLIENTSONSHELL
					else
						cout << "client with id " << id << endl;
#endif // SHOWCLIENTSONSHELL
				}
				before= cur;
				if(cur)
					cur= cur->getNextComm();
				UNLOCK(m_NEXTCOMMUNICATION);
			}

			cur= before;
			if(empty < m_minConnThreads)
			{

				while(empty <= m_maxConnThreads)
				{
					LOCK(m_NEXTCOMMUNICATION);
					if(m_vFreeIDs.size())
					{
						sort(m_vFreeIDs.begin(), m_vFreeIDs.end());
						id= m_vFreeIDs[0];
						m_vFreeIDs.erase(m_vFreeIDs.begin());
					}else
					{
						id= m_nNextFreeID;
						++m_nNextFreeID;
					}
#ifdef SHOWCLIENTSONSHELL
					cout << "create communication thread with default id " << id << endl;
#endif // SHOWCLIENTSONSHELL
					cur->setNextComm(getNewCommunicationThread(m_nNextFreeID));
					cur= cur->getNextComm();
					cur->start();
					++empty;
					UNLOCK(m_NEXTCOMMUNICATION);
				}
			}

			LOCK(m_NEXTCOMMUNICATION);
		}while(!m_poNextFree);
#ifdef SHOWCLIENTSONSHELL
		if(m_poNextFree)
			cout << "next free communication-thread with default id " << m_poNextFree->getDefaultID() << endl;
		cout << dec << m_vFreeIDs.size() << " IDs in POOL" << endl;
		cout << "next new ID is " << dec << m_nNextFreeID << endl << endl;
#endif // SHOWCLIENTSONSHELL
		UNLOCK(m_NEXTCOMMUNICATION);
	}

	void CommunicationThreadStarter::setNewClient(SHAREDPTR::shared_ptr<IFileDescriptorPattern>& descriptor)
	{
		LOCK(m_NEXTCOMMUNICATION);
		if(m_poNextFree)
		{
			m_poNextFree->connection(descriptor);
			m_poNextFree= NULL;
		}else
		{
			if( m_minConnThreads == 0)
			{
				m_poNextFree= m_poFirstCommunication;
				while(m_poNextFree != NULL)
				{
					if(!m_poNextFree->hasClient())
						break;
					m_poNextFree= m_poNextFree->getNextComm();
				}
				if(m_poNextFree != NULL)
				{
					m_poNextFree->connection(descriptor);
					m_poNextFree= NULL;
				}else
				{
					(*descriptor) << "ERROR 018\n";
					descriptor->flush();
#ifdef SERVERDEBUG
					cout << "send: ERROR 018" << endl;
					cout << "      no free communication thread exist" << endl;
#endif // SERVERDEBUG
				}
			}else
				m_qpNewClients.push(descriptor);
		}
		AROUSE(m_NEXTCOMMUNICATIONCOND);
		UNLOCK(m_NEXTCOMMUNICATION);
	}

	bool CommunicationThreadStarter::fillCommunicationThreads()
	{
		bool bDone= false;
		ICommunicationPattern* pCurrent= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		if(	m_poNextFree
			&&
			m_qpNewClients.empty()	)
		{
			UNLOCK(m_NEXTCOMMUNICATION);
			return true;
		}

		while(pCurrent)
		{
			if(!pCurrent->hasClient())
			{
				if(!m_qpNewClients.empty())
				{
					pCurrent->connection(m_qpNewClients.front());
					m_qpNewClients.pop();
					if(	m_qpNewClients.empty()
						&&
						m_poNextFree			)
					{
						bDone= true;
						break;
					}

				}else if(m_poNextFree == NULL)
				{
					m_poNextFree= pCurrent;
					bDone= true;
					break;
				}else
				{
					bDone= true;
					break;
				}
			}
			pCurrent= pCurrent->getNextComm();
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return bDone;
	}

	/*Communication* CommunicationThreadStarter::searchNextFreeCommunicationThread()
	{
		Communication* pRv= NULL;
		Communication* pCurrent= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		while(pCurrent)
		{
			if(!pCurrent->hasClient())
			{
				pRv= pCurrent;
				break;
			}
			pCurrent= pCurrent->m_pnext;
		}
		UNLOCK(m_NEXTCOMMUNICATION);

		return pRv;
	}*/

	short CommunicationThreadStarter::hasClients() const
	{
		short nRv= 0;
		ICommunicationPattern *pCurrentCom= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			if(pCurrentCom->hasClient())
				++nRv;
			pCurrentCom= pCurrentCom->getNextComm();
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return nRv;
	}

	short CommunicationThreadStarter::hasThreads()
	{
		short nRv= 0;
		ICommunicationPattern *pCurrentCom= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			++nRv;
			pCurrentCom= pCurrentCom->getNextComm();
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return nRv;
	}

	string CommunicationThreadStarter::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
	{
		bool bShow= true;
		istringstream iparams(params);
		string param, sRv;

		while(!iparams.eof())
		{
			iparams >> param;
			if(param == "clients")
			{
				bShow= false;
				break;
			}
		}
		sRv= Thread::getStatusInfo(params, pos, elapsed, lasttime);
		if(bShow)
		{
			ostringstream threads, clients;

			threads << hasThreads();
			clients << hasClients();
			sRv+= "\n         run with ";
			sRv+= threads.str() + " threads, ";
			sRv+= clients.str() + " of which have a client";
		}
		return sRv;
	}

	bool CommunicationThreadStarter::stopCommunicationThreads(const unsigned int connectionID, bool bWait/*= false*/)
	{
		bool wait= bWait;
		ICommunicationPattern *pCurrentCom;
		ICommunicationPattern *pDel, *pBefore;

		LOCK(m_NEXTCOMMUNICATION);
		m_bWillStop= true;
		pBefore= m_poFirstCommunication;
		pCurrentCom= m_poFirstCommunication;
		UNLOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			pDel= NULL;
			if(connectionID != pCurrentCom->getConnectionID() || connectionID == 0 )
			{
				pCurrentCom->stop(false);
				if(	wait
					&&
					pCurrentCom->running()	)
				{
					for(short c= 0; c < 4; ++c)
					{// giving the client from outside an chance to stopping self
						if(!pCurrentCom->running())
							break;
						usleep(250000);
					}
					if(pCurrentCom->running())
						pCurrentCom->stop(true); // kill connection
					wait= false;
				}
				if(!pCurrentCom->running())
				{
					pDel= pCurrentCom;
					LOCK(m_NEXTCOMMUNICATION);
					if(pDel == m_poFirstCommunication)
					{
						m_poFirstCommunication= m_poFirstCommunication->getNextComm();
						pBefore= m_poFirstCommunication;
					}else
						pBefore->setNextComm(pCurrentCom->getNextComm());
					UNLOCK(m_NEXTCOMMUNICATION);
				}

			}else
			{
				pCurrentCom->stop(false);
				LOCK(m_NEXTCOMMUNICATION);
				if(m_poFirstCommunication != pCurrentCom)
				{
					pBefore->setNextComm(m_poFirstCommunication);
					m_poFirstCommunication->setNextComm(pCurrentCom->getNextComm());
					m_poFirstCommunication= pCurrentCom;
				}
				UNLOCK(m_NEXTCOMMUNICATION);
			}
			if(pDel == NULL)
				pBefore= pCurrentCom;
			pCurrentCom= pCurrentCom->getNextComm();
			if(pDel != NULL)
				delete pDel;
		}
		// if method be set to no waiting return true,
		// or all communication threads be stopped,
		// or also if connection ID be set and only the own does exist return true
		if(	!bWait ||
			!m_poFirstCommunication ||
			(	connectionID &&
				m_poFirstCommunication->getNextComm() == NULL	)	)
		{
			return true;
		}
		return false;
	}

	int CommunicationThreadStarter::stop(const bool *bWait)
	{
		int nRv;

		stopCommunicationThreads(0, false);
		LOCK(m_NEXTCOMMUNICATION);
		nRv= Thread::stop(false);
		AROUSEALL(m_NEXTCOMMUNICATIONCOND);
		UNLOCK(m_NEXTCOMMUNICATION);
		if(	nRv <= 0
			&&
			bWait
			&&
			*bWait == true	)
		{
			while(!stopCommunicationThreads(0, true))
			{};
			nRv= Thread::stop(true);
		}
		return nRv;
	}

	void CommunicationThreadStarter::ending()
	{
	}

	CommunicationThreadStarter::~CommunicationThreadStarter()
	{
		ICommunicationPattern* del;
		ICommunicationPattern* pCurrentCom;

		pCurrentCom= m_poFirstCommunication;
		while(pCurrentCom)
		{
			del= pCurrentCom;
			pCurrentCom= pCurrentCom->getNextComm();
			delete del;
		}
		_instance= NULL;
	}

}
