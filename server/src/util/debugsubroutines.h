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

/**********************************************************************************
 * implementation to follow setting behavior
 * from one folder:subroutine to write into folder:subroutine
 * and inform from folder to restart
 * for this behavior comment out the __followSETbehaviorFrom directive
 * define as integer from 0 to 6
 * and __followSETbehaviorTo directive with same integer or higher
 * later when now where information lost it can be made closer the output numbers
 * 0 - get from SHELL command
 * 1 - writing into subroutine object inside list (from SHELL or external command)
 * 2 - subroutine get information to change
 * 3 - subroutine want to inform other folder
 * 4 - folder get information
 * 5 - folder remove information because folder start running after information
 *     before really know to should start also for this one
 * 6 - folder start for the information
 * for all definitions are regular expressions allowed
 */
//#define __followSETbehaviorFrom 0
#ifdef __followSETbehaviorFrom
#define __followSETbehaviorTo 6
#define __followSETbehaviorFromFolder "Raff._Zeit_timer"
#define __followSETbehaviorFromSubroutine ""
#define __followSETbehaviorToFolder "power_switch_set"
#define __followSETbehaviorToSubroutine ""
#endif // __followSETbehaviorFrom

#endif /*DEBUGSUBROUTINES_H_*/

