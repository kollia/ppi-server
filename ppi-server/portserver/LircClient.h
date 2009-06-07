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
#ifndef LIRCCLIENT_H_
#define LIRCCLIENT_H_

#include "../util/debug.h"

#ifdef _LIRCCLIENTLIBRARY
#include "lirc/lirc_client.h"


#include "../ports/portbaseclass.h"

namespace ports
{
	/**
	 * class represent communication with LIRC.<br />
	 * to use compiling with lirc
	 * application need package liblircclient-dev
	 * and also library lirc_client with option -llirc_client
	 * by creating with make
	 *
	 * @autor Alexander Kolli
	 * @version 1.0.0.
	 */
	class LircClient : public portBase
	{
	public:
		/**
		 * create initalization of LircClient
		 *
		 * @param folderName name of folder in whitch procedures are running
		 * @param subroutineName name of subroutine inside the folder
		 */
		LircClient(string folderName, string subroutineName);
		/**
		 * create initalization of LircClient.<br />
		 * Constructor for extendet objects
		 *
		 * @param type type of extendet class
		 * @param folderName name of folder in whitch procedures are running
		 * @param subroutineName name of subroutine inside the folder
		 */
		LircClient(string type, string folderName, string subroutineName);
		/**
		 * look on lirc client liprary, whether any signal set
		 */
		virtual bool measure();
		/**
		 * destruct instance of LircClient
		 */
		virtual ~LircClient();

	protected:
		/**
		 * initial Lirc Object
		 */
		void init();

	private:
	#ifdef _LIRCCLIENTLIBRARY
		/**
		 * structure of main lirc configureation
		 */
		struct lirc_config *m_ptLircConfig;
	#endif //_LIRCCLIENTLIBRARY
	};
}

#endif //_LIRCCLIENTLIBRARY
#endif /*LIRCCLIENT_H_*/
