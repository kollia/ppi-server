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

#include "SSLConnect.h"

#ifdef _OPENSSLLIBRARY
#include <string>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

namespace server
{

	SSLConnect::SSLConnect()
	{
	}

	bool SSLConnect::init()
	{
		//BIO* bio;

		/* Initializing OpenSSL */
		SSL_load_error_strings();
		ERR_load_BIO_strings();
		OpenSSL_add_all_algorithms();

		return true;
	}

	IFileDescriptorPattern* SSLConnect::listen()
	{
		return NULL;
	}
	string SSLConnect::getLastDescriptorAddress()
	{
		string address;

		return address;
	}
	SSLConnect::~SSLConnect()
	{
	}

}
#endif //_OPENSSLLIBRARY
