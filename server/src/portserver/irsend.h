/* 	$Id: irsend.c,v 5.5 2007/09/29 17:13:14 lirc Exp $	 */

/*

  irsend -  application for sending IR-codes via lirc

  Copyright (C) 1998 Christoph Bartelmus (lirc@bartelmus.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef IRSEND_H_
#define IRSEND_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define PACKETERROR -1
#define TRANSACTIONERROR -2

#define LIRCD "/var/run/lirc/lircd"
#define LIRC_INET_PORT 20010
#define PACKET_SIZE 256
/* three seconds */
#define TIMEOUT 3

enum packet_state
{
	P_BEGIN,
	P_MESSAGE,
	P_STATUS,
	P_DATA,
	P_N,
	P_DATA_N,
	P_END
};

void sigalrm(int sig);
const char *read_string(int fd);
int send_packet(int fd,const char *packet);
int irsend(const char* directive, const char* remote, const char* code); //, const int count);//int argc,char **argv)

#endif // IRSEND_H_
