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

#ifndef IMETHODSTRINGSTREAM_H_
#define IMETHODSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

#include "../thread/Thread.h"

#include "../../pattern/util/IMethodStringStreamPattern.h"

#include "IParameterStringStream.h"

using namespace std;
using namespace design_pattern_world;

namespace util {

	extern pthread_mutex_t* globalSYNCMUTEX;

class IMethodStringStream : 		public IParameterStringStream,
							virtual public IMethodStringStreamPattern
{
public:
	/**
	 * content holder of MethodStringStream object
	 */
	struct stringDef_t
	{
		/**
		 * synchronization ID
		 */
		unsigned long long syncID;
		/**
		 * method name
		 */
		string method;
		/**
		 * all parameters
		 */
		string stream;
	};
	/**
	 * empty constructor for object with no content
	 */
	IMethodStringStream()
	:	IParameterStringStream(""),
	 	m_nSyncID(0),
	 	m_sMethod("")
	{ m_sStream.str(""); };
	/**
	 * constructor to create IMethodStringStream object
	 *
	 * @param stream string to convert to object
	 */
	IMethodStringStream(const string& stream);
	/**
	 * constructor to create OMethodStringStream object
	 *
	 * @param obj object to convert or copy
	 */
	IMethodStringStream(IMethodStringStreamPattern& obj);
	/**
	 * copy constructor for own object
	 *
	 * @param obj other object to copy inside
	 */
	IMethodStringStream& operator = (const IMethodStringStream& obj);
	/**
	 * copy constructor for own object
	 *
	 * @param obj other object to copy inside
	 */
	IMethodStringStream* operator = (const IMethodStringStream* obj);
	/**
	 * copy constructor for normally string
	 *
	 * @param str string to convert into object
	 */
	IMethodStringStream* operator = (const string& str);
	/**
	 * compare operator for two objects
	 *
	 * @param obj other object to compare
	 */
	bool operator == (const IMethodStringStream& obj)
	{ 	if(m_nSyncID != obj.m_nSyncID) return false;
		if(m_sMethod != obj.m_sMethod) return false;
		if(m_sStream != obj.m_sStream) return false;
		return true;									}
	/**
	 * Lexicographical comparison for two objects
	 *
	 * @param obj other object for comparison
	 */
	bool operator < (const IMethodStringStream& obj)
	{	if(	m_nSyncID < obj.m_nSyncID &&
			m_sMethod < obj.m_sMethod &&
			m_sStream < obj.m_sStream	) return true;
		return false;									}
	/**
	 * string converter for constructors
	 *
	 * @param stream hole string to convert
	 * @return structure of converted string
	 */
	static stringDef_t stringConvertion(const string& stream);
	/**
	 * output of hole method string with parameters
	 *
	 * @return hole string
	 */
	virtual string getMethodName() const
	{ return m_sMethod; };
	/**
	 * create intern synchronization ID for object
	 *
	 * @param syncID synchronization ID to implement, elsewhere when not set or 0 create one
	 * @return whether new synchronization ID be created or otherwise take before defined
	 */
	virtual bool createSyncID(unsigned long long syncID= 0);
	/**
	 * return created synchronize ID when exist,
	 * elsewhere 0
	 *
	 * @return synchronize ID
	 */
	virtual unsigned long long getSyncID() const;
	/**
	 * remove synchronization ID for object which is intern set
	 */
	virtual void removeSyncID();
	/**
	 * return current hole defined string
	 *
	 * @param withSync output string with syncID when exist
	 * @param withMethod whether string should returned with method name
	 * @return defined string
	 */
	virtual string str(bool withSync= false, bool withMethod= true) const;
	/**
	 * create synchronization ID
	 *
	 * @return synchronize ID
	 */
	static unsigned long long makeSyncID();
	/**
	 * remove synchronization ID
	 *
	 * @param syncID synchronization ID to delete
	 */
	static void delSyncID(unsigned long long syncID);
	/**
	 * virtual destructor of object
	 */
	virtual ~IMethodStringStream() {};

private:
	/**
	 * synchronize ID
	 */
	unsigned long long m_nSyncID;
	/**
	 * vector of all set synchronization IDs
	 */
	static vector<unsigned long long> _syncIDs;
	/**
	 * method of object
	 */
	string m_sMethod;
};

}

#endif /* IMETHODSTRINGSTREAM_H_ */
