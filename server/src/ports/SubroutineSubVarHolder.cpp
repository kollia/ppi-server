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
	SubroutineSubVarHolder::SubroutineSubVarHolder(SHAREDPTR::shared_ptr<IMeasurePattern> oFolder, const string& sSubVar)
	: m_oFolder(oFolder),
	  m_oSubroutine(SHAREDPTR::shared_ptr<IListObjectPattern>()),
	  m_sSubVar(sSubVar)
	{}

	SubroutineSubVarHolder::SubroutineSubVarHolder(SHAREDPTR::shared_ptr<IListObjectPattern> oSubroutine, const string& sSubVar)
	: m_oFolder(SHAREDPTR::shared_ptr<IMeasurePattern>()),
	  m_oSubroutine(oSubroutine),
	  m_sSubVar(sSubVar)
	{}

	auto_ptr<IValueHolderPattern> SubroutineSubVarHolder::getValue(const InformObject& who)
	{
		auto_ptr<IValueHolderPattern> oMeasureValue;

		oMeasureValue= auto_ptr<IValueHolderPattern>(new ValueHolder());
		if(m_oFolder)
			oMeasureValue->setValue(m_oFolder->getSubVar(who, m_sSubVar));
		else
			oMeasureValue->setValue(m_oSubroutine->getSubVar(who, m_sSubVar));
		return oMeasureValue;
	}

	string SubroutineSubVarHolder::checkStartPossibility()
	{
		if(m_oSubroutine)
			return m_oSubroutine->checkStartPossibility();
		/**
		 * an folder be defined
		 * for SubroutineSubVarHolder,
		 * Thus cannot start by time
		 */
		ostringstream sRv;

		sRv << "folder " << m_oFolder->getFolderName() << endl;
		sRv << "is not designed to start any action per time";
		return sRv.str();
	}
}

