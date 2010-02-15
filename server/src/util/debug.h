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

#ifndef SERVERDEBUG
//#define SERVERDEBUG
#endif // SERVERDEBUG

#ifndef ALLOCATEONMETHODSERVER
//#define ALLOCATEONMETHODSERVER "ppi-db-server"
#endif // ALLOCATEONMETHODSERVER

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
#define CONDITIONSDEBUG "xyz"
#endif //CONDITIONSDEBUG

#ifndef MUTEXLOCKDEBUG
// define MUTEXLOCKDEBUG with one or more mutex_lock in an string
// separated with an blank -> show message when mutex be locked or unlocked.
// If no mutex defined it shows all mutex.
// Also the application want to wait for an condition or arose an condition
// who is not defined with getMutex("<name>") it showes an error message
// mutex undefined. If an mutex be set in MUTEXLOCKDEBUG which not exist
// no mutex be showen, only if an error messages occures for any mutex or it is not defined
#define MUTEXLOCKDEBUG "xyz" //NEXTCOMMUNICATION THREADSAVEMETHODS"
#endif //MUTEXLOCKDEBUG

#ifndef _LIRCCLIENTLIBRARY
//#define _LIRCCLIENTLIBRARY
#endif //_LIRCCLIENTLIBRARY

#ifndef SERVERTIMELOG
//#define SERVERTIMELOG
#endif //SERVERTIMELOG

//#define SINGLETHREADING
#ifdef _K8055LIBRARY
#define _EXTERNVENDORLIBRARYS
#endif //_K8055LIBRARY

#ifndef _EXTERNVENDORLIBRARYS
#ifdef _OWFSLIBRARY
#define _EXTERNVENDORLIBRARYS
#endif // _OWFSLIBRARY
#endif // _EXTERNVENDORLIBRARYS

#endif /*DEBUG_H_*/

