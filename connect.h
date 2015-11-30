/**
 * libwlocate - WLAN-based location service
 * Copyright (C) 2010 Oxygenic/VWP virtual_worlds(at)gmx.de
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONNECT_H
#define CONNECT_H

extern int  tcp_recv(int sock,char *data, int len,const char *termStr,long timeout);
extern int  tcp_send(int sock, const char *msg,int len,int msecs);
extern void tcp_closesocket (int sock);
extern int  tcp_connect_to(const char *address,unsigned short connect_port);
extern void tcp_set_blocking(int sock,char block);

#endif
