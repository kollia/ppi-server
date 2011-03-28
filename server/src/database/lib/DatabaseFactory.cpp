/**
 *   This file 'DatabaseFactory.cpp' is part of ppi-server.
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

#include "DatabaseFactory.h"
#include "Database.h"

namespace ppi_database
{
	IPPIDatabasePattern* DatabaseFactory::getChoosenDatabase(IPropertyPattern* serverproperties, IChipConfigReaderPattern* chipreader)
	{
		return new Database(serverproperties, chipreader);
	}
}
