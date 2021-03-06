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

#include "../../../util/GlobalStaticMethods.h"
#include "../../../util/XMLStartEndTagReader.h"
#include "../../../util/usermanagement.h"

#include "../../../util/properties/configpropertycasher.h"

#include "../../../pattern/util/LogHolderPattern.h"
#include "../../../pattern/server/IFileDescriptorPattern.h"

#include "Communication.h"
#include "ServerThread.h"


using namespace std;
using namespace util;
using namespace user;

namespace server
{
	Communication::Communication(unsigned int ID, const StarterPattern* pStarter) :
	Thread("communicationThread"),
	m_poStarter(pStarter)
	{
		//m_SPEAKERVARACCESS= getMutex("SPEAKERVARACCESS");
		m_HAVECLIENT= getMutex("HAVECLIENT");
		m_CLIENTWAITCOND= getCondition("CLIENTWAITCOND");

		m_bConnected= false;
		m_pnext= NULL;
		m_bHasClient= false;
		m_nDefaultID= ID;
		m_sCR= "\n";
		m_bSetCR= true;
		m_bSpeakerThread= false;
	}

	EHObj Communication::init(void *args)
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
		return m_pError;
	}

	bool Communication::execute()
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
				if(bHave && !m_hFileAccess->eof())
					m_hFileAccess->flush();
			}
			glob::threadStopMessage("Communication::execute(): one communication thread was ending");
#ifndef DEBUG
#ifdef SERVERDEBUG
			string msg;

			msg= "Server stop connection to client:";
			msg+=  m_hFileAccess->getHostAddressName();
			cout << msg << endl;
#endif // SERVERDEBUG
#else // DEBUG
			string msg;

			msg= "Server stop connection to client:";
			msg+=  m_hFileAccess->getHostAddressName();
			cout << msg << endl;
#endif // DEBUG
			LOCK(m_HAVECLIENT);
			if(m_hFileAccess.get())
			{
				m_hFileAccess->closeConnection();
				m_hFileAccess= auto_ptr<IFileDescriptorPattern>();
			}
			m_bHasClient= false;
			UNLOCK(m_HAVECLIENT);
			glob::threadStopMessage("Communication::execute(): arouse CommunicationThreadStarter to order all communication threads");
			m_poStarter->arouseStarterThread();

		}else if(conderror)
			USLEEP(500000);
		glob::threadStopMessage("Communication::execute(): ending CommunicationThread");
		return true;
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

	EHObj Communication::stop(const bool *bWait)
	{
		EHObj Rv;
		//bool client= hasClient();

		LOCK(m_HAVECLIENT);
		Rv= Thread::stop();// do not detach thread
		AROUSE(m_CLIENTWAITCOND);
		UNLOCK(m_HAVECLIENT);
		if(	bWait &&
			*bWait				)
		{
			/*if(*bWait && client && m_hFileAccess.get())
			{
				m_hFileAccess->closeConnection();
				m_hFileAccess= auto_ptr<IFileDescriptorPattern>();
			}*/
			Rv= Thread::stop(bWait);
		}
		return Rv;
	}

	void Communication::ending()
	{
		if(m_hFileAccess.get())
		{
			m_hFileAccess->closeConnection();
			m_hFileAccess= auto_ptr<IFileDescriptorPattern>();
		}
	}

	void Communication::connection(SHAREDPTR::shared_ptr<IFileDescriptorPattern>& access)
	{
		LOCK(m_HAVECLIENT);
		m_bHasClient= true;
		m_bConnected= false;
		m_hFileAccess= access;
		m_hFileAccess->setClientID(m_nDefaultID);
		AROUSE(m_CLIENTWAITCOND);
		UNLOCK(m_HAVECLIENT);
	}

	unsigned int Communication::getConnectionID() const
	{
		if(!hasClient())
			return 0;
		return m_hFileAccess->getClientID();
	}

	bool Communication::hasClient() const
	{
		bool bClient;

		LOCK(m_HAVECLIENT);
		if(m_hFileAccess.get() == NULL)
			bClient= false;
		else
			bClient= m_bHasClient;
		UNLOCK(m_HAVECLIENT);
		return bClient;
	}

	bool Communication::isClient(const string& process, const string& client) const
	{
		if(!hasClient())
			return false;
		return m_hFileAccess->isClient(process, client);
	}

	bool Communication::hasClients()
	{
		return m_poStarter->hasClients();
	}

	Communication::~Communication()
	{
		DESTROYMUTEX(m_HAVECLIENT);
		DESTROYCOND(m_CLIENTWAITCOND);
	}
}
