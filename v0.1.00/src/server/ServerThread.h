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
#ifndef SERVERTHREAD_H_
#define SERVERTHREAD_H_

#include <iostream>
#include <sys/io.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "../util/Thread.h"

#include "../ports/measureThread.h"

#include "../pattern/util/ipropertypattern.h"
#include "../pattern/server/IServerConnectArtPattern.h"

using namespace design_pattern_world::server_pattern;

namespace server
{
	/**
	 * structure to gibe server nesesary options
	 */
	struct serverArg_t
	{
		//string host;
		//unsigned short port;
		string clientFolder;
		meash_t *pFirstMeasureThreads;
		measurefolder_t *ptFirstFolder;
		map<string, string> mOutlineResults;
		/**
		 * properties from server configuration
		 */
		IPropertyPattern* pServerConf;
		/**
		 * main thread which starts communication threads
		 */
		//Server* pDistributor;
	};


	//typedef struct _SOCKET socketStruct;

	class ServerThread : public Thread
	{
		public:
			/**
			 * creating instance of ServerThread if not exist
			 * and retorning object.<br />
			 * ServerThread deliting by disturb object of IServerConnectArtPattern
			 *
			 * @param connect object of IServerConnectArtPattern to communicate with client
			 * @param args void pointer of any information get in init method
			 * @param bHold whether process should wait for ending thread
			 * @return single instance of ServerThread object
			 */
			static ServerThread *initial(IServerConnectArtPattern* connect, void *args, bool bHold= false);
			/**
			 * returning single instance of ServerThread
			 *
			 * @return single instance
			 */
			static ServerThread* instance()
			{ return _instance; };
			static int connectAsClient(const char *ip, unsigned short port, bool print= true);
			void close();
			bool getDirectory(string filter, string verz, vector<string> &list);
			virtual ~ServerThread();

		protected:
			string m_sClientRoot;
			meash_t *m_pFirstMeasureThread;
			/**
			 * actual communication ID of connection
			 */
			unsigned int m_connectionID;

			virtual bool init(void *args);
			virtual void execute();
			virtual void ending() {};
			bool doConversation(FILE* fp, string input);
			/**
			 * protected initialization for given info points
			 * to write into the status information
			 *
			 * @param params parameter set by call getStatusInfo from main method
			 * @param pos position struct see pos_t
			 * @param elapsed seconds be elapsed since last position time pos_t.time
			 * @param time from last position pos_t.time converted in an string
			 */
			virtual string getStatusInfo(string params, pos_t& pos, time_t elapsed, string lasttime);

		private:
			/**
			 * single instance of ServerThread
			 */
			static ServerThread *_instance;
			/**
			 * holder IServerConnectArtPattern to verify connection to client
			 */
			IServerConnectArtPattern* m_pConnect;

			/**
			 * initialization of class ServerThread
			 *
			 * @param connect object of IServerConnectArtPattern to communicate with client
			 * @param commStartSleep sleeping CommunicationThreadStarter for default time in microseconds
			 */
			ServerThread(IServerConnectArtPattern* connect);
	};
}

#endif /*SERVERTHREAD_H_*/
