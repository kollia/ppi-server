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

#include "OMethodStringStream.h"
#include "IMethodStringStream.h"

namespace util {

OMethodStringStream::OMethodStringStream(const string& stream)
{
	IMethodStringStream::stringDef_t content;

	content= IMethodStringStream::stringConvertion(stream);
	m_nSyncID= content.syncID;
	m_sMethod= content.method;
	m_sStream.str("");
	m_sStream << content.stream;
}

OMethodStringStream::OMethodStringStream(const IMethodStringStreamPattern& obj)
{
	m_sMethod= obj.getMethodName();
	m_nSyncID= obj.getSyncID();
	m_sStream.str("");
	m_sStream << obj.str();
}

OMethodStringStream* OMethodStringStream::operator = (const OMethodStringStream& obj)
{
	m_nSyncID= obj.getSyncID();
	m_sMethod= obj.getMethodName();
	m_sStream.str("");
	m_sStream << obj.str(false, false);
	return this;
}

OMethodStringStream* OMethodStringStream::operator = (const string& str)
{
	IMethodStringStream::stringDef_t content;

	content= IMethodStringStream::stringConvertion(str);
	m_nSyncID= content.syncID;
	m_sMethod= content.method;
	m_sStream.str("");
	m_sStream << content.stream;
	return this;
}

bool OMethodStringStream::createSyncID(unsigned long long syncID/*= 0*/)
{
	if(syncID == 0)
	{
		if(m_nSyncID == 0)
		{
			m_nSyncID= IMethodStringStream::makeSyncID();
			return true;
		}
	}else
	{
		if(m_nSyncID == 0)
		{
			m_nSyncID= syncID;
			return true;
		}
	}
	return false;
}

unsigned long long OMethodStringStream::getSyncID() const
{
	return m_nSyncID;
}

void OMethodStringStream::removeSyncID()
{
	IMethodStringStream::delSyncID(m_nSyncID);
	m_nSyncID= 0;
}

string OMethodStringStream::str(bool withSync/*= false*/, bool withMethod/*= true*/) const
{
	string sRv, stream;
	ostringstream oID;

	if(withMethod)
	{
		sRv= m_sMethod;
		if(sRv != "")
			sRv+= " ";
	}
	if(	withSync &&
		m_nSyncID != 0)
	{
		oID << "syncID " << m_nSyncID;
		sRv+= oID.str();
	}
	stream= OParameterStringStream::str();
	if(stream != "")
	{
		if(sRv != "")
			sRv+= " ";
		sRv+= stream;
	}
	return sRv;
}

string OMethodStringStream::parameters() const
{
	return OParameterStringStream::str();
}


OMethodStringStream::~OMethodStringStream() {}

}// namespace util
