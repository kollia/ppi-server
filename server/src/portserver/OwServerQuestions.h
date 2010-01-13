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

#ifndef OWSERVERQUESTIONS_H_
#define OWSERVERQUESTIONS_H_

#include <vector>

#include "../util/process.h"

#include "owserver.h"

namespace server {

using namespace util;
using namespace design_pattern_world::server_pattern;

class OwServerQuestions :	public Process {
public:
	/**
	 * constructor to create question port for owserver
	 *
	 * @param process in which process the question-server is running
	 * @param client name of client to identify by server
	 * @param getConnection on which connection from outside the server is reachable to get questions
	 * @param own pointer to own OWServer in which running
	 */
	OwServerQuestions(const string& process, const string& client, IClientConnectArtPattern* getConnection, OWServer* own)
	:	Process(process, client, NULL, getConnection, true),
		m_oServer(own),
		m_nPos(0)
	{ };
	virtual ~OwServerQuestions() {};

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
	 * @return defined error code from extended class
	 */
	virtual int init(void *args);
	/**
	 * method to running thread
	 * in the extended class.<br />
	 * This method starting again when ending without an sleeptime
	 * if the method stop() isn't call.
	 *
	 * @return defined error code from extended class
	 */
	virtual int execute();
	/**
	 * method to ending the thread.<br />
	 * This method will be called if any other or own thread
	 * calling method stop().
	 */
	virtual void ending();

private:
	/**
	 * OWServer in which running
	 */
	OWServer* m_oServer;
	/**
	 * answer string vector of question result
	 */
	vector<string> m_vAnswer;
	/**
	 * position of answer
	 */
	vector<string>::size_type m_nPos;
	/**
	 * one answer string
	 */
	string m_sAnswer;
};

}

#endif /* OWSERVERQUESTIONS_H_ */
