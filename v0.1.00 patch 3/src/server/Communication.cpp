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
#include <sstream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <iostream>

#include "../logger/LogThread.h"

#include "../util/Thread.h"
#include "../util/XMLStartEndTagReader.h"
#include "../util/configpropertycasher.h"
#include "../util/usermanagement.h"

#include "../database/Database.h"

#include "../pattern/server/IFileDescriptorPattern.h"

#include "Communication.h"
#include "ServerThread.h"


using namespace std;
using namespace ppi_database;
using namespace util;
using namespace user;

namespace server
{
	Communication::Communication(unsigned int ID, StarterPattern* pStarter,
									meash_t* first, measurefolder_t *ptFolderStart, string clientPath) :
	Thread("communicationThread", /*defaultSleep*/0)
	{
		//m_SPEAKERVARACCESS= getMutex("SPEAKERVARACCESS");
		m_HAVECLIENT= getMutex("HAVECLIENT");
		m_CLIENTWAITCOND= getCondition("CLIENTWAITCOND");

		m_bConnected= false;
		m_pnext= NULL;
		m_bHasClient= false;
		m_nDefaultID= ID;
		m_poStarter= pStarter;
		m_pFirstMeasureThread= first;
		m_ptFolderStart= ptFolderStart;
		m_sClientRoot= clientPath;
		m_sCR= "\n";
		m_bSetCR= true;
		m_bSpeakerThread= false;
	}

	bool Communication::init(void *args)
	{
		// encrypt decrypt from webside http://www.daniweb.com/forums/thread23258.html
		/*char *message ="Test Message";
		RSA  *apub;
		RSA  *aprivate;
		FILE *f;
		int ret;
		unsigned char *buf;
		unsigned char *e_data;
		unsigned char *clear_text;


		//Get key
		//f= fopen("a_rsa_public","rb");
		f= fopen("ssl/certs/markus_key_vpn.pem","rb");
		if(f == NULL)
		{
			printf("\nError opening public key file");
			return -1;
		}else
			printf("\n Public key file opened");

		//load the key
		if ( fread(&apub,sizeof apub,1,f) != 1)
		{
			printf("\nError reading public key");
			return -1;
		}else
			printf("\nPublic key read");

		//close the key file
		fclose(f);

		buf = (unsigned char *) malloc(strlen(message));
		memcpy(buf,message,strlen(message));

		e_data = (unsigned char *) malloc(RSA_size(apub)); // THIS is where i get a run time error

		//encrypt data
		RSA_public_encrypt(strlen(message),buf, e_data, apub, RSA_PKCS1_OAEP_PADDING);

		//------------------decrypt
		//Get key
		//f= fopen("a_rsa_private","rb");
		f= fopen("ssl/certs/server_key_vpn.pem","rb");
		if(f == NULL)
		{
			printf("\nError opening private key file");
			return -1;
		}
		//load the key
		ret = fread(&aprivate,sizeof(aprivate),1,f);
		//close the key file
		fclose(f);

		//make sure we loaded ok
		if(ret != 1)
		{
			printf("\nError reading private key");
			return -1;
		}

		clear_text= (unsigned char *) malloc(strlen(message));
		RSA_private_decrypt(strlen((char*)e_data), e_data, clear_text, aprivate, RSA_PKCS1_OAEP_PADDING);
		cout << "read text: '" << clear_text << "'" << endl;*/
		return true;
	}

	void Communication::execute()
	{
		bool bHave;
		int conderror= 0;

		LOCK(m_HAVECLIENT);
		bHave= m_bHasClient;
		if(	!bHave
			&&
			!stopping()	)
		{
			conderror= CONDITION(m_CLIENTWAITCOND, m_HAVECLIENT);
			bHave= m_bHasClient;
		}
		UNLOCK(m_HAVECLIENT);
		//if(!bHave)
		//	usleep(10000);

		if(bHave)
		{
			while(bHave)
			{
				bHave= m_hFileAccess->transfer();
				if(!m_hFileAccess->eof())
					m_hFileAccess->flush();
			}
			string msg;

			msg= "Server stop connection to client:";
			msg+=  m_hFileAccess->getHostAddressName();
			delete m_hFileAccess;
			LOCK(m_HAVECLIENT);
			m_bHasClient= false;
			UNLOCK(m_HAVECLIENT);
			m_poStarter->arouseStarterThread();

#ifndef DEBUG
#ifdef SERVERDEBUG
			cout << msg << endl;
#endif // SERVERDEBUG
#else // DEBUG
			cout << msg << endl;
#endif // DEBUG
		}else if(conderror)
			usleep(500000);
	}

	string Communication::getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime)
	{
		bool bShow= false;
		bool bClient= hasClient();
		istringstream iparams(params);
		ostringstream id;
		string param, sRv;

		while(!iparams.eof())
		{
			iparams >> param;
			if(	param == "clients"
				||
				param.substr(0, 7) == "thread:"	)
			{
				bShow= true;
				break;
			}
		}
		if(bShow)
		{
			ostringstream oRv;

			oRv << "[";
			oRv.width(6);
			oRv << dec << pos.tid << "] ";
			oRv << pos.threadname << " ";

			if(pos.identif == "#client#wait-forQuestion")
			{
				oRv << "wait of question from client since " << lasttime;
				sRv= oRv.str();

			}else if(pos.identif == "#client#answer-question")
			{
				oRv << "treat request '" << pos.info2 << "' from client since " << lasttime;
				sRv= oRv.str();

			}else if(pos.identif == "#client#send-hearing")
			{
				oRv << "send chanched value '" << pos.info2 << "' to client since " << lasttime;
				sRv= oRv.str();

			}else
				sRv= Thread::getStatusInfo(params, pos, elapsed, lasttime);
			sRv+= "\n         with ";
			if(!bClient)
				sRv+= "no ";
			sRv+= "client";
			if(bClient)
			{
				id << getConnectionID();
				sRv+= " has connection-id ";
				sRv+= id.str();
			}
		}
		return sRv;
	}

	void* Communication::stop(const bool *bWait)
	{
		void* Rv;

		POS("x");
		LOCK(m_HAVECLIENT);
		Rv= Thread::stop();// do not detach thread
		AROUSE(m_CLIENTWAITCOND);
		UNLOCK(m_HAVECLIENT);
		if(	bWait
			&&
			*bWait	)
		{
			Rv= Thread::stop(bWait);
		}
		return Rv;
	}

	void Communication::ending()
	{

	}

	void Communication::connection(IFileDescriptorPattern* access)
	{
		LOCK(m_HAVECLIENT);
		m_hFileAccess= access;
		m_bHasClient= true;
		m_bConnected= false;
		access->setClientID(m_nDefaultID);
		AROUSE(m_CLIENTWAITCOND);
		UNLOCK(m_HAVECLIENT);
	}

	unsigned int Communication::getConnectionID()
	{
		if(!hasClient())
			return 0;
		return m_hFileAccess->getClientID();
	}

	bool Communication::hasClient()
	{
		bool bClient;

		LOCK(m_HAVECLIENT);
		bClient= m_bHasClient;
		UNLOCK(m_HAVECLIENT);
		return bClient;
	}

	bool Communication::hasClients()
	{
		return m_poStarter->hasClients();
	}

	//bool Communication::doConversation(FILE* fp, string input)

	//pthread_mutex_t* g_NEXTCOMMUNICATION;
	//Communication* g_poFirstCommunication;
	//Communication* g_freeCommunication= NULL;


	Communication::~Communication()
	{
		DESTROYMUTEX(m_HAVECLIENT);
		DESTROYCOND(m_CLIENTWAITCOND);
	}
}
