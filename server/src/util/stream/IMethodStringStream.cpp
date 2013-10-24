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

#include <cctype>
#include <iostream>
#include <algorithm>

#include "IMethodStringStream.h"

using namespace std;

namespace util {

	pthread_mutex_t* globalSYNCMUTEX= NULL;
	vector<unsigned long long> IMethodStringStream::_syncIDs;

	IMethodStringStream::IMethodStringStream(const string& stream)
	: IParameterStringStream("")
	{
		stringDef_t content;

		content= stringConvertion(stream);
		m_nSyncID= content.syncID;
		m_sMethod= content.method;
		m_sStream.str("");
		m_sStream << content.stream;
	}

	IMethodStringStream::stringDef_t IMethodStringStream::stringConvertion(const string& stream)
	{
		bool bFail(true);
		const string syncID("syncID");
		string sSyncID;
		streampos pos;
		ostringstream check;
		istringstream istream(stream);
		stringDef_t oRv;
		string::size_type npos;

		oRv.syncID= 0;
		istream >> oRv.method;
		if(	!istream.fail() &&
			oRv.method == syncID		)
		{
			pos= istream.tellg();
			istream >> oRv.syncID;
			if(!istream.fail())
			{
				check << oRv.syncID;
				istream.seekg(static_cast<string::size_type>(pos), ios::beg);
				istream >> sSyncID;
				if(check.str() == sSyncID)
				{// string can be an answer beginning with syncID
					pos= istream.tellg();
					istream >> sSyncID;
					if(!istream.fail() &&
						sSyncID != syncID	)
					{// method-string object is an answer and no method-string be set
						istream.seekg(static_cast<string::size_type>(pos), ios::beg);
						oRv.method= "";
						bFail= false;
					}
				}else
				{// after syncID is no integer, so syncID is the method
					istream.seekg(static_cast<string::size_type>(pos), ios::beg);
					istream.clear();
				}
			}else
			{// after syncID is no integer, so syncID is the method
				istream.seekg(static_cast<string::size_type>(pos), ios::beg);
				istream.clear();
			}
		}
		if( bFail == true &&
			!istream.fail() &&
			(	isdigit(oRv.method[0]) ||
				oRv.method[0] == '\"'		)	)
		{// method-string object is an answer and no method-string be set
			oRv.method= "";
			istream.seekg(0, ios::beg);
			pos= 0;
			bFail= false;
		}
		if(	bFail == true &&
			!istream.fail() &&
			oRv.method != ""		)
		{
			pos= istream.tellg();
			istream >> sSyncID;
			if(sSyncID == syncID)
			{
				istream >> oRv.syncID;
				bFail= istream.fail();
			}
		}
		if(bFail)
			oRv.syncID= 0;
		else
			pos= istream.tellg();
		if(pos >= 0)
		{
			npos= static_cast<string::size_type>(pos);
			oRv.stream= stream.substr(npos);
		}
		return oRv;
	}

	IMethodStringStream::IMethodStringStream(IMethodStringStreamPattern& obj)
	: IParameterStringStream(obj.str())
	{
		string method;

		m_nSyncID= obj.getSyncID();
		m_sMethod= obj.getMethodName();
		m_sStream.str("");
		if(m_sMethod != method)
		{
			m_sStream.seekg(0, ios::beg);
			m_sStream.clear();
		}
	}

	IMethodStringStream& IMethodStringStream::operator = (const IMethodStringStream& obj)
	{
		IParameterStringStream::operator=(obj);
		m_nSyncID= obj.getSyncID();
		m_sMethod= obj.getMethodName();
		m_sStream.str("");
		m_sStream << obj.str(false, false);
		return *this;
	}

	IMethodStringStream* IMethodStringStream::operator = (const IMethodStringStream* obj)
	{
		m_nSyncID= obj->getSyncID();
		m_sMethod= obj->getMethodName();
		m_sStream.str("");
		m_sStream << obj->str(false, false);
		return this;
	}

	IMethodStringStream* IMethodStringStream::operator = (const string& str)
	{
		stringDef_t content;

		content= stringConvertion(str);
		m_nSyncID= content.syncID;
		m_sMethod= content.method;
		m_sStream.str("");
		m_sStream << content.stream;
		return this;
	}

	bool IMethodStringStream::createSyncID(unsigned long long syncID/*= 0*/)
	{
		if(syncID == 0)
		{
			if(m_nSyncID == 0)
			{
				m_nSyncID= makeSyncID();
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

	unsigned long long IMethodStringStream::makeSyncID()
	{
		unsigned long long nRv;

		if(globalSYNCMUTEX == NULL)
			globalSYNCMUTEX= Thread::getMutex("globalSYNCMUTEX");
		LOCK(globalSYNCMUTEX);
		if(_syncIDs.empty())
		{
			nRv= 1;
			_syncIDs.push_back(nRv);

		}else
		{
			nRv= 1;
			sort(_syncIDs.begin(), _syncIDs.end());
			for(vector<unsigned long long>::iterator it= _syncIDs.begin(); it != _syncIDs.end(); ++it)
			{
				if(*it != nRv)
					break;
				++nRv;
			}
			_syncIDs.push_back(nRv);
		}
		UNLOCK(globalSYNCMUTEX);
		//cout << "  --------------------------  create syncID " << nRv << "  --------------------------" << endl;
		return nRv;
	}

	unsigned long long IMethodStringStream::getSyncID() const
	{
		return m_nSyncID;
	}

	void IMethodStringStream::removeSyncID()
	{
		delSyncID(m_nSyncID);
		m_nSyncID= 0;
	}

	void IMethodStringStream::delSyncID(unsigned long long syncID)
	{
		vector<unsigned long long>::iterator found;

		if(syncID == 0)
			return;
		//cout << "  --------------------------  delete syncID " << syncID << "  --------------------------" << endl;
		LOCK(globalSYNCMUTEX);
		found= find(_syncIDs.begin(), _syncIDs.end(), syncID);
		if(found != _syncIDs.end())
			_syncIDs.erase(found);
		UNLOCK(globalSYNCMUTEX);
	}

	string IMethodStringStream::str(bool withSync/*= false*/, bool withMethod/*= true*/) const
	{
		string sRv, stream;

		if(withMethod)
		{
			sRv= m_sMethod;
			if(sRv != "")
				sRv+= " ";
		}
		if(	withSync &&
			m_nSyncID > 0)
		{
			ostringstream ID;

			sRv+= "syncID ";
			ID << m_nSyncID;
			sRv+= ID.str();
		}
		stream= m_sStream.str();
		if(stream != "")
		{
			if(sRv != "")
				sRv+= " ";
			sRv+= stream;
		}
		return sRv;
	}
#if 0
		string sRv;
		string::size_type nLen;
		ostringstream oStream;

		if(m_sMethod != "")
			oStream << m_sMethod << " ";
		if(	withSync &&
			m_nSyncID != 0	)
		{
			oStream << "syncID " << m_nSyncID << " ";
		}
		sRv= IParameterStringStream::str();
		nLen= m_sMethod.size();
		if(sRv == m_sMethod)
			sRv= "";
		if(	nLen > 0 &&
			sRv.size() > nLen	)
		{
			if(sRv.substr(0, nLen) == m_sMethod )
				sRv= sRv.substr(nLen + 1);
		}
		nLen= string("syncID").size();
		if(	nLen < sRv.size() &&
			sRv.substr(0, nLen) == "syncID"	)
		{
			size_t pos;
			sRv= sRv.substr(nLen + 1);
			pos= sRv.find(' ', 0);
			if(pos != string::npos)
				sRv= sRv.substr(pos + 1);
			else
				sRv= "";
		}
		sRv= oStream.str() + sRv;
		return sRv;
	}
#endif

}// namespace util
