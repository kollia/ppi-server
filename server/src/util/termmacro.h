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
#ifndef TERMMACRO_H_
#define TERMMACRO_H_

/**
 * dfine terminal class for output
 * or regular cout for tout if developer
 * want to have the normaly way
 */
#include "Terminal.h"
#define tout Terminal::instance()
#define newline Terminal::instance().newline()

//#include <iostream>
//#define tout cout
//#define newline cout.endl()

#endif /*TERMMACRO_H_*/
