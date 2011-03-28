/**
 *   This file 'DatabaseFactory.h' is part of ppi-server.
 *   Created on: 26.03.2011
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

#ifndef DATABASEFACTORY_H_
#define DATABASEFACTORY_H_

#include "../../pattern/util/IPPIDatabasePattern.h"
#include "../../pattern/util/ipropertypattern.h"
#include "../../pattern/util/IChipConfigReaderPattern.h"

using namespace design_pattern_world;

namespace ppi_database
{

	class DatabaseFactory
	{
	public:
		/**
		 * factory to choose database.<br />
		 * This factory class do not delete created database object by ending
		 * of any called class or hole process.
		 *
		 * @param serverproperties server properties (server.conf) to know which database is to create
		 * @param chipreader reference to default chip reader to know on which time the value can be deleted from database.<br />
		 * 					 If only needed actually state of database (no execute for working will be needed), this reference can be NULL.
		 */
		static IPPIDatabasePattern* getChoosenDatabase(IPropertyPattern* serverproperties, IChipConfigReaderPattern* chipreader);
	};

}

#endif /* DATABASEFACTORY_H_ */
