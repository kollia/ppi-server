/**
 *   This file 'ppi_server_clients.h' is part of ppi-server.
 *   Created on: 12.07.2015
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


#ifndef PPI_SERVER_CLIENTS_H_
#define PPI_SERVER_CLIENTS_H_

/**
 * definition of process
 * where running all working lists
 */
#define __WORKINGLIST_PROCESS "ppi-server"
/**
 * definition of client name
 * which control all working lists
 */
#define __WORKINGLIST_CONTROL_CLIENT "ProcessChecker"
/**
 * definition of process name
 * in which running the external port server
 */
#define __EXTERNALPORT_PROCESSES "ppi-owreader"
/**
 * begin definition of client name
 * for external ports
 * This name should have an number as follow
 */
#define __EXTERNALPORT_CLIENT_BEGINSTR "OwServerQuestion-"



#endif /* PPI_SERVER_CLIENTS_H_ */
