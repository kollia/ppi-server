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

#ifndef PROCESSCHECKER_H_
#define PROCESSCHECKER_H_

#include <string>

#include "util/thread/Thread.h"

#include "server/libs/client/ExternClientInputTemplate.h"

using namespace std;
using namespace util;
using namespace design_pattern_world::server_pattern;

class ProcessChecker : 	public Thread,
						public ExternClientInputTemplate {
public:
	/**
	 * constructor to create answer port in ppi-server.<br/>
	 * By cancel this ProcessChecker object, first and second parameter object will be also delete in parent class.
	 *
	 * @param getConnection on which connection from outside the server is reachable to get questions
	 */
	ProcessChecker(IClientConnectArtPattern* sendConnection, IClientConnectArtPattern* getConnection, const unsigned short nExistOW)
	:	Thread("ProcessChecker", true),
		ExternClientInputTemplate("ppi-server", "ProcessChecker", sendConnection, getConnection),
		m_nEndPos(0),
		m_nExistOW(nExistOW)
		{};
	/**
	 * destructor of object
	 */
	virtual ~ProcessChecker() {};

protected:
	/**
	 * method to initial the thread
	 * in the extended class.<br />
	 * this method will be called before running
	 * the method execute
	 *
	 * @param args user defined parameter value or array,<br />
	 * 				comming as void pointer from the external call
	 * 				method start(void *args).
	 * @return object of error handling
	 */
	OVERWRITE EHObj init(void *args);
	/**
	 * method to running thread
	 * in the extended class.<br />
	 * This method starting again when ending without an sleeptime
	 * if the method stop() isn't call.
	 *
	 * @return whether should start thread again
	 */
	OVERWRITE bool execute();
	/**
	 * method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending();

private:
	/**
	 * answers for getting questions
	 */
	string m_sAnswer;
	/**
	 * position of ending
	 */
	unsigned short m_nEndPos;
	/**
	 * how much one wire server exists
	 */
	const unsigned short m_nExistOW;
};

#endif /* PROCESSCHECKER_H_ */
