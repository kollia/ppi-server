/**
 *   This file 'ExternPort.h' is part of ppi-server.
 *   Created on: 28.01.2011
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

#ifndef subEXTERNPORT_H_
#define subEXTERNPORT_H_

#include "Subroutine.h"

namespace subroutines
{

	class ExternPort : public Subroutine
	{
	public:
		ExternPort(ofstream& out, const string& type, const string& name);
		virtual void pid(const string& content, const string& desc= "");
		virtual void ppin(const string& content, const string& desc= "");
		virtual void ppriority(const string& content, const string& desc= "");
		using Subroutine::pvalue;
		using Subroutine::pbegin;
		using Subroutine::pwhile;
		using Subroutine::pend;
		using Subroutine::action;
	};

}

#endif /* subEXTERNPORT_H_ */
