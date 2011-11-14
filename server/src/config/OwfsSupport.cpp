/**
 *   This file 'OwfsSupport.cpp' is part of ppi-server.
 *   Created on: 13.11.2011
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

#ifdef _OWFSLIBRARY

#include <stdlib.h>

#include "OwfsSupport.h"

int OwfsSupport::execute(const ICommandStructPattern* params)
{
	string examplefile("maxim_examples.conf");

	if(params->hasOption("file"))
		examplefile= params->getOptionContent("file");
	return EXIT_SUCCESS;
}

#endif /* _OWFSLIBRARY */
