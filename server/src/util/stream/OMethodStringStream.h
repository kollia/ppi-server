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

#ifndef OMETHODSTRINGSTREAM_H_
#define OMETHODSTRINGSTREAM_H_

//#include <iostream>
#include <sstream>
#include <string>

#include "../../pattern/util/IMethodStringStreamPattern.h"

#include "OParameterStringStream.h"

using namespace std;
using namespace design_pattern_world;

namespace util {

class OMethodStringStream 	: 			public OParameterStringStream,
								virtual public IMethodStringStreamPattern
{
public:
	/**
	 * empty constructor for object with no content
	 */
	OMethodStringStream()
	:	m_nSyncID(0),
	 	m_sMethod("")
	{ m_sStream.str(""); };
	/**
	 * constructor to create OMethodStringStream object
	 *
	 * @param stream string to convert to object
	 */
	OMethodStringStream(const string& stream);
	/**
	 * constructor to create OMethodStringStream object
	 *
	 * @param obj <code>IMethodStringStream</code> object to convert
	 */
	OMethodStringStream(const IMethodStringStreamPattern& obj);
	/**
	 * copy constructor for own object
	 *
	 * @param obj other object to copy inside
	 */
	OMethodStringStream* operator = (const OMethodStringStream& obj);
	/**
	 * copy constructor for normally string
	 */
	OMethodStringStream* operator = (const string& str);
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
	 * constructor to create OMethodStringStream object from new object.<br>
	 * New object overwrite method name and parameter stream
	 *
	 * @param method name of method in object
	 */
	//OMethodStringStream(const OMethodStringStream& method);
	/**
	 * output of hole method string with parameters
	 *
	 * @param withSync output string with syncID when exist
	 * @param withMethod whether string should returned with method name
	 * @return hole string
	 */
	virtual string str(bool withSync= false, bool withMethod= true) const;
	/**
	 * output of all parameters in object
	 *
	 * @return parameter string
	 */
	string parameters() const;
	/**
	 * virtual destructor of object
	 */
	virtual ~OMethodStringStream();

private:
	/**
	 * synchronize ID
	 */
	unsigned long long m_nSyncID;
	/**
	 * method of object
	 */
	string m_sMethod;

};

}

#endif /* OMETHODSTRINGSTREAM_H_ */
