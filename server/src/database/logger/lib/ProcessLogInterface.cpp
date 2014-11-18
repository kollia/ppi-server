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

#include <boost/algorithm/string/replace.hpp>

#include "ProcessLogInterface.h"

#include "../../../util/thread/Thread.h"
#include "../../../util/stream/OMethodStringStream.h"

using namespace util;

namespace logger
{
	ProcessLogInterface* ProcessLogInterface::_instance= NULL;

	ProcessLogInterface::ProcessLogInterface(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait)
	:	LogInterface("LogServer", identifwait, wait),
	 	ProcessInterfaceTemplate(process, "LogInterface", "LogServer", connection, NULL)
	{
	}

	bool ProcessLogInterface::initial(const string& process, IClientConnectArtPattern* connection, const int identifwait, const bool wait/*= true*/)
	{
		if(_instance == NULL)
		{
			_instance= new ProcessLogInterface(process, connection, identifwait, wait);
		}
		return true;
	}

	void ProcessLogInterface::deleteObj()
	{
		if(_instance)
			delete _instance;
	}

	ProcessLogInterface::~ProcessLogInterface()
	{
		if(_instance)
			_instance= NULL;
	}

	EHObj ProcessLogInterface::openConnection(string toopen/*= ""*/)
	{
		if(ExternClientInputTemplate::hasOpenGetConnection())
			return m_pSocketError;
		return ExternClientInputTemplate::openSendConnection(toopen);
	}

}
