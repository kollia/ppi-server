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

#include "../database/Database.h"

#include "communicationthreadstarter.h"
#include "Communication.h"

using namespace std;
using namespace ppi_database;

namespace server
{

	CommunicationThreadStarter* CommunicationThreadStarter::_instance= NULL;

	CommunicationThreadStarter::CommunicationThreadStarter(IServerConnectArtPattern* connect
													, serverArg_t* serverArg, useconds_t defaultSleep)
	: Thread("CommunicationThreadStarter", defaultSleep)
	{
		m_bWillStop= false;
		m_ptMeasureThread= serverArg->pFirstMeasureThreads;
		m_ptFirstFolder= serverArg->ptFirstFolder;
		m_poProperties= serverArg->pServerConf;

		m_NEXTCOMMUNICATION= getMutex("NEXTCOMMUNICATION");
		m_NEXTCOMMUNICATIONCOND= getCondition("NEXTCOMMUNICATIONCOND");
	}

	bool CommunicationThreadStarter::initial(IServerConnectArtPattern* connect
										, serverArg_t* serverArg, useconds_t defaultSleep)
	{
		if(!_instance)
		{
			_instance= new CommunicationThreadStarter(connect, serverArg, defaultSleep);
			if(!_instance)
				return false;
			return true;
		}
		return false;
	}

	bool CommunicationThreadStarter::init(void* args)
	{
		Communication* pCurrentCom;
		string property("minconnectionthreads");

		m_minConnThreads= m_poProperties->getUShort(property, /*warning*/true);
		/*if(	property == "#ERROR"
			||
			m_minConnThreads < 2		)
		{
			m_minConnThreads= 2;
		}*/
		property= "maxconnectionthreads";
		m_maxConnThreads= m_poProperties->getUShort(property, /*warning*/true);
		if(	property == "#ERROR"
			||
			m_maxConnThreads < 4		)
		{
			m_maxConnThreads= 4;
		}
		m_sClientPath= *(string*)args;
		cout << "### start communication threads " << flush;
		m_nNextFreeID= 1;
		pCurrentCom= new Communication(m_nNextFreeID, this, m_ptMeasureThread, m_ptFirstFolder, m_sClientPath);
		++m_nNextFreeID;
		//m_NEXTCOMMUNICATION= pCurrentCom->getMutex("NEXTCOMMUNICATION");
		pCurrentCom->start();
		m_poFirstCommunication= pCurrentCom;
		m_poNextFree= pCurrentCom;
		cout << "." << flush;
		for(unsigned short count= 1; count<m_maxConnThreads; ++count)
		{
			pCurrentCom->m_pnext= new Communication(m_nNextFreeID, this, m_ptMeasureThread,
														m_ptFirstFolder, m_sClientPath);
			++m_nNextFreeID;
			pCurrentCom= pCurrentCom->m_pnext;
			pCurrentCom->start();
			cout << "." << flush;
		}
		cout << endl;
		return true;
	}

	void CommunicationThreadStarter::execute()
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
	}

	void CommunicationThreadStarter::arouseStarterThread()
	{
		LOCK(m_NEXTCOMMUNICATION);
		AROUSE(m_NEXTCOMMUNICATIONCOND);
		UNLOCK(m_NEXTCOMMUNICATION);
	}

	unsigned int CommunicationThreadStarter::nextClientID()
	{
		unsigned int sid;
		unsigned int id= 1;
		Communication* cur= m_poFirstCommunication;
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
			cur= cur->m_pnext;
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

	void CommunicationThreadStarter::checkThreads(Communication* first)
	{
		unsigned short empty;
		unsigned int id;
		Communication* before= NULL;
		Communication* cur;
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
						if(cur->m_pnext)
						{
							if(before)
								before->m_pnext= cur->m_pnext;
							else
								before= cur->m_pnext;
						}else
							before->m_pnext= NULL;

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
					cur= cur->m_pnext;
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
					cur->m_pnext= new Communication(id, this, m_ptMeasureThread,
															m_ptFirstFolder, m_sClientPath);
					cur= cur->m_pnext;
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

	void CommunicationThreadStarter::setNewClient(IFileDescriptorPattern* descriptor)
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
					m_poNextFree= m_poNextFree->m_pnext;
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
		Communication* pCurrent= m_poFirstCommunication;

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
			pCurrent= pCurrent->m_pnext;
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

	short CommunicationThreadStarter::hasClients()
	{
		short nRv= 0;
		Communication *pCurrentCom= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			if(pCurrentCom->hasClient())
				++nRv;
			pCurrentCom= pCurrentCom->m_pnext;
		}
		UNLOCK(m_NEXTCOMMUNICATION);
		return nRv;
	}

	short CommunicationThreadStarter::hasThreads()
	{
		short nRv= 0;
		Communication *pCurrentCom= m_poFirstCommunication;

		LOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			++nRv;
			pCurrentCom= pCurrentCom->m_pnext;
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

	void CommunicationThreadStarter::stopCommunicationThreads(bool bWait)
	{
		Communication *pCurrentCom= m_poFirstCommunication;
		Communication *pDel;

		LOCK(m_NEXTCOMMUNICATION);
		m_bWillStop= true;
		UNLOCK(m_NEXTCOMMUNICATION);
		while(pCurrentCom != NULL)
		{
			if(pCurrentCom->running())
				pCurrentCom->stop(bWait);
			if(bWait)
			{
				pDel= pCurrentCom;
				pCurrentCom= pCurrentCom->m_pnext;
				delete pDel;
			}else
				pCurrentCom= pCurrentCom->m_pnext;
		}
		if(bWait)
			m_poFirstCommunication= NULL;
	}

	void* CommunicationThreadStarter::stop(const bool *bWait)
	{
		void* pRv;
		bool* wait= false;

		LOCK(m_NEXTCOMMUNICATION);
		pRv= Thread::stop(wait);
		AROUSEALL(m_NEXTCOMMUNICATIONCOND);
		UNLOCK(m_NEXTCOMMUNICATION);
		if(	bWait
			&&
			*bWait == true	)
		{
			pRv= Thread::stop(bWait);
		}
		return pRv;
	}

	void CommunicationThreadStarter::ending()
	{
	}

	CommunicationThreadStarter::~CommunicationThreadStarter()
	{
		Communication* del;
		Communication* pCurrentCom;

		pCurrentCom= m_poFirstCommunication;
		while(pCurrentCom)
		{
			del= pCurrentCom;
			pCurrentCom= pCurrentCom->m_pnext;
			delete del;
		}
		_instance= NULL;
	}

}
