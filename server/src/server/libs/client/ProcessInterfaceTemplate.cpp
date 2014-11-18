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

#include "ProcessInterfaceTemplate.h"
#include "../../../util/stream/OMethodStringStream.h"

namespace util
{
	bool ProcessInterfaceTemplate::running()
	{
		string answer;
		OMethodStringStream running("running");

		answer= sendMethod(m_sSendTo, running, true);
		if(answer == "true")
			return true;
		return false;
	}

	EHObj ProcessInterfaceTemplate::stop(const bool bWait/*= true*/)
	{
		string answer;
		OMethodStringStream stop("stop");

		m_pSocketError->clear();
		answer= sendMethod(m_sSendTo, stop, bWait);
		m_pSocketError->setErrorStr(answer);
		if(m_pSocketError->getErrorType() == IEH::UNKNOWN)
			m_pSocketError->clear();
		return m_pSocketError;
	}

	bool ProcessInterfaceTemplate::stopping()
	{
		string answer;
		OMethodStringStream running("stopping");

		answer= sendMethod(m_sSendTo, running, true);
		if(answer == "true")
			return true;
		return false;
	}
}
