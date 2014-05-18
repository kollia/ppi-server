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
#ifndef DEBUG_H_
#define DEBUG_H_

/******************************************************************************************************************\
 * development stage of ppi-server with tools
 */
#define PPI_MAJOR_RELEASE 0
#define PPI_MINOR_RELEASE 2
#define PPI_SUBVERSION 0
#define PPI_PATCH_LEVEL   0
#define PPI_REVISION_NUMBER 421

#ifndef DISTRIBUTION_RELEASE
#define DISTRIBUTION_RELEASE ""
#endif
#define PPI_SERVER_PROTOCOL 1.0

#define PPI_JAVA_MAJOR_RELEASE 0
#define PPI_JAVA_MINOR_RELEASE 2
#define PPI_JAVA_SUBVERSION 0
#define PPI_JAVA_PATCH_LEVEL   0
#define PPI_JAVA_REVISION_NUMBER 336

/*******************************************************************************************************************
 * cmake define follow symbols for preprocessor from gcc
 * for develop define the same (with -D) or set here this defines
 * the path are set here localy in the workspace. cmake do same like the second */
#ifndef PPICLIENTPATH
#define PPICLIENTPATH "client" // ${INSTALL_CLIENT_PATH}/ppi-server/client
#endif
#ifndef PPICONFIGPATH
#define PPICONFIGPATH "conf" // ${INSTALL_CONFIG_PATH}/ppi-config
#endif
#ifndef PPIDATABASEPATH
#define PPIDATABASEPATH "data" // ${INSTALL_CONFIG_PATH}/ppi-server/database
#endif
#ifndef PPILOGPATH
#define PPILOGPATH "log" // ${INSTALL_LOGGING_PATH}/ppi-server
#endif
/* for needed physical ports to have access to specific chips (options in cmake) */
//#define _OWFSLIBRARY // need also library libowcapi.so
//#define _K8055LIBRARY // need also library libK8055.so
/**/
//#define _OPENSSLLIBRARY // not used currently libssl.so
//#define _LIRCCLIENTLIBRARY // not used currently liblirc_client.so
/*******************************************************************************************************************/

#ifndef DEBUG
//#define DEBUG
#endif //DEBUG

#ifndef _APPLICATIONSTOPMESSAGES
// string for definition of _APPLICATIONSTOPMESSAGES can be
//   ""   nothing for all processes by stopping the server
//   "ppi-server"
//   "ppi-owreader"
//   "ppi-internet-server"
//   "ppi-db-server"  or
//   "ppi-log-client"
//#define _APPLICATIONSTOPMESSAGES ""
//#define _APPLICATIONTHREADSTOPMESSAGES
#endif //_APPLICATIONSTOPMESSAGES

#ifndef SHOWCLIENTSONSHELL
//#define SHOWCLIENTSONSHELL
#endif // SHOWCLIENTSONSHELL

#ifndef CONDITIONSDEBUG
// define CONDITIONDEBUG with one or more conditions in an string
// separated with an blank -> show message when condition wait or arose.
// If no condition defined it shows all conditions.
// Also the application want to wait for an condition or arose an condition
// which is not defined with getCondition("<name>") it showes an error message
// condition undefined. If an condition be set in CONDITIONDEBUG which not exist
// no condition be showen, only if an error message occures for any conditions or it is not defined
//#define CONDITIONSDEBUG "xyz"
#endif //CONDITIONSDEBUG

#ifndef MUTEXLOCKDEBUG
// define MUTEXLOCKDEBUG with one or more mutex_lock in an string
// separated with an blank -> show message when mutex be locked or unlocked.
// If no mutex defined it shows all mutex.
// Also the application want to wait for an condition or arose an condition
// who is not defined with getMutex("<name>") it showes an error message
// mutex undefined. If an mutex be set in MUTEXLOCKDEBUG which not exist
// no mutex be showen, only if an error messages occures for any mutex or it is not defined
//#define MUTEXLOCKDEBUG "xyz" //STARTSTOPTHREAD SLEEPMUTEX"
#endif //MUTEXLOCKDEBUG

#ifndef MUTEXCREATEDEBUG
// same behivior as MUTEXLOCKDEBUG
// but show creation and destroying of mutex or condition
//#define MUTEXCREATEDEBUG "STARTSTOPTHREAD SLEEPMUTEX"
#endif // MUTEXCREATEDEBUG

#ifndef _LIRCCLIENTLIBRARY
//#define _LIRCCLIENTLIBRARY
#endif //_LIRCCLIENTLIBRARY

#ifndef SERVERTIMELOG
//#define SERVERTIMELOG
#endif //SERVERTIMELOG

// show reading and writing on external ports
// and also when an client ask for debug info
// (ppi-client DEBUG -ow 1)
//#define __OWSERVERREADWRITE

//#define SINGLETHREADING
#ifdef _K8055LIBRARY
#define _EXTERNVENDORLIBRARYS
#endif //_K8055LIBRARY

#ifndef _EXTERNVENDORLIBRARYS
#ifdef _OWFSLIBRARY
#define _EXTERNVENDORLIBRARYS
#endif // _OWFSLIBRARY
#endif // _EXTERNVENDORLIBRARYS

// debugging output for LIRC components
// how long transmitting is activated
// output <folder>: <subroutine:digits>  <subroutine:after> = <time>
//#define DEBUG_ACTIVATEDLIRCOUTPUT

#endif /*DEBUG_H_*/

