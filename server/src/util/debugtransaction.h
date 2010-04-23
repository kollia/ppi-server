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
#ifndef DEBUGTRANSCATION_H_
#define DEBUGTRANSCATION_H_


#ifndef __FOLLOWSERVERCLIENTTRANSACTION
//#define __FOLLOWSERVERCLIENTTRANSACTION
#ifdef __FOLLOWSERVERCLIENTTRANSACTION
/********************************************+
 * allowed process: ppi-server
 * 					ppi-starter
 * 					ppi-owreader
 * 					ppi-db-server
 * 					ppi-internet-server
 * 					LogServer
 * allowed clients:	LogInterface
 * 					DbInterface
 *					OwServerQuestion-[N] N =  1 - running owreader
 *					OwInterface
 *					ProcessChecker
 */
//#define __FOLLOW_FROMPROCESS "LogServer"
//#define __FOLLOW_FROMCLIENT "LogServer"
//#define __FOLLOW_TOPROCESS "ppi-internet-server"
//#define __FOLLOW_TOCLIENT "ppi-internet-server"
// sending message -> need only the beginning string
//#define __FOLLOW_SENDMESSAGE "setOWDebug"
// ---------------------------------------------------------------------------------------------------
#endif // __FOLLOWSERVERCLIENTTRANSACTION
#endif // __FOLLOWSERVERCLIENTTRANSACTION

#endif /*DEBUGTRANSCATION_H_*/

