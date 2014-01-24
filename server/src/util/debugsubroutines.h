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
#ifndef DEBUGSUBROUTINES_H_
#define DEBUGSUBROUTINES_H_


/**********************************************************************************
 * write subroutine debug output for all lines (necessary for debugging),
 * otherwise write all subroutines output as block after passing
 * (only when subroutine defined as debug session)
 */
#ifndef __WRITEDEBUGALLLINES
//#define __WRITEDEBUGALLLINES
#endif // __WRITEDEBUGALLLINES

/**********************************************************************************
 * output on command line also the statistics
 * to calculate middle length of folder
 * for TIMER subroutines with action exact
 * and reach finish when parameter finished be defined
 */
#ifndef __showStatistic
//#define __showStatistic
#endif // __showStatistic

/**********************************************************************************
 * output statistics for all lines (necessary for debugging)
 * when definition of __WRITEDEBUGALLLINES be set,
 * otherwise write calcLengthDiff() and getLengthedTime() output
 * as hole block
 */
#ifdef __showStatistic
#ifdef __WRITEDEBUGALLLINES
#include "thread/Terminal.h"
#define termout *Terminal::instance()->out()
#endif // __WRITEDEBUGALLLINES
#endif // __showStatistic

#endif /*DEBUGSUBROUTINES_H_*/

