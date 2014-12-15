/**
 *   This file 'SubroutineSubVarHolder.cpp' is part of ppi-server.
 *   Created on: 06.04.2014
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

#include "SubroutineSubVarHolder.h"

namespace ports
{
	SubroutineSubVarHolder::SubroutineSubVarHolder(IListObjectPattern* oSubroutine, const string& sSubVar)
	: m_oSubroutine(oSubroutine),
	  m_sSubVar(sSubVar)
	{
		if(sSubVar == "changed")
			oSubroutine->setChangedSubVar(this);
	}

	auto_ptr<IValueHolderPattern> SubroutineSubVarHolder::getValue(const InformObject& who)
	{
		auto_ptr<IValueHolderPattern> oMeasureValue;

		oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
		oMeasureValue->setValue(m_oSubroutine->getSubVar(who, m_sSubVar));
		return oMeasureValue;
	}
}

