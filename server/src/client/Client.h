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
#ifndef CLIENT_H_
#define CLIENT_H_

#include <vector>
#include <map>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../util/smart_ptr.h"
#include "../util/structures.h"
#include "../util/configpropertycasher.h"

using namespace std;
using namespace util;

//extern server::ServerThread* gInternetServer;

class Client
{
	private:
		string m_sWorkdir;
		string m_sConfPath;
		/*
		 * casher of defined variables in file server.conf
		 */
		Properties m_oServerFileCasher;

	protected:
	public:
		bool execute(const string& workdir, vector<string> options, string command);
};

#endif /*CLIENT_H_*/
