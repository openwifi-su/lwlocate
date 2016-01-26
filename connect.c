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

#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef ENV_WINDOWS
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #pragma comment (lib, "ws2_32.lib")
 #define MSG_NOSIGNAL 0
#else
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <sys/ioctl.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <netdb.h>
#endif

#ifndef ENV_WINDOWSCE
 #include <fcntl.h>
 #include <errno.h>
#else
 #ifndef EAGIAN
  #define EAGAIN 11 // really the correct value? there is no errno.h for WinCE
 #endif
#endif


#ifdef ENV_QNX
 #define MSG_NOSIGNAL 0
#endif


static int util_thread_sleep(int msecs)
{
#ifdef ENV_WINDOWS
   Sleep(msecs);
#else
   usleep(msecs*1000);
#endif
   return msecs;
}


/**
Receive data from a socket connection
@param[in] sock the identifier of the opened socket connection
@param[in] data the memory area where the received data have to be stored into
@param[in] len the maximum length of data that have to be read
@param[in] termStr an optional termination string; when this value is not NULL and the character
           defined here is received the function returns
@param[in] timeout when this time is exceeded the function returns also if not all data could
           be read; this parameter is valid only in case the socket is non-blocking
*/
int tcp_recv(int sock,char *data, int len,const char *termStr,long timeout)
{
   long /*size_t*/   rc;
   long     ctr=0,readLen=0;
   #ifdef ENV_WINDOWS
   long     err;
   #endif

   // data from source client side
   while (readLen<len)
   {
      rc = recv(sock,data+readLen,1/*len-readLen*/,MSG_NOSIGNAL);
      if (rc>0)
      {
         readLen+=rc;
         if (termStr)
         {
            if (readLen+1<len) data[readLen+1]=0;
            if (strstr(data,termStr)) return readLen;
         }
         if (readLen>=len) return readLen;
      }
      else if (rc==0) return readLen;
      else
      {
#ifdef ENV_WINDOWS
         err=GetLastError();
         if ((err!=EAGAIN) && (err!=WSAEWOULDBLOCK))
#else
         if ((errno!=EAGAIN) && (errno!=EINPROGRESS) && (errno!=0))
#endif
          return readLen;
         ctr+=10;
         util_thread_sleep(10);
      }
      if (ctr>timeout) break;
   }
   return readLen;
}



/**
Send data to a socket connection
@param[in] sock the identifier of the opened socket connection
@param[in] msg the data that have to be send
@param[in] len the length of the data
@param[in] msecs when this time is exceeded the function returns also if not all data could
           be sent; this parameter is valid only in case the socket is non-blocking
*/
int tcp_send(int sock, const char *msg,int len,int msecs)
{
   int    rlen=0;
   int    ctr=0,val;
#ifdef ENV_WINDOWS
   int    errno;
#else

   errno=0;
#endif
   while ((rlen<len) && (ctr<msecs))
   {
#ifdef ENV_LINUX
      val=send(sock,msg+rlen,len-rlen,MSG_NOSIGNAL);
#else
      val=send(sock,msg+rlen,len-rlen,0);
#endif
      if (val>=0) rlen+=val;
      else
      {
#ifndef ENV_WINDOWS
         if (errno==EAGAIN) ctr-=2; // in case of eagain we expect a longer send-timeout
#else
         errno=WSAGetLastError();
         if (errno==WSAEWOULDBLOCK) ctr-=2; // in case of eagain we expect a longer send-timeout
#endif
         else if (errno!=0)
         {
            rlen=-1;
            break;
         }
#ifndef ENV_WINDOWS
         errno=0;
#endif
      }
      if (rlen<len)
      {
         util_thread_sleep(2);
         ctr+=2;
      }
      if ((rlen==0) && (ctr>msecs/2)) break;
   }
   return rlen;
}



/**
Closes an opened socket connection
@param[in] sock the socket that has to be closed
*/
void tcp_closesocket (int sock)
{
#ifdef ENV_WINDOWS
   shutdown(sock,SD_SEND);
   shutdown(sock,SD_RECEIVE);
   closesocket(sock);
#else
   shutdown(sock,SHUT_WR);
   shutdown(sock,SHUT_RD);
   if (close (sock)<0) perror("close failed");
#endif
}



/**
Tries to establish a client connection to a (remote) server socket
@param[in] address address of the remote server in style a.b.c.d or www.domain.tld
@return the socket identifier of the established connection or a value <=0 in case of an error
*/
int tcp_connect_to(const char *address) {
  struct addrinfo hints, *servinfo, *p;
  int s;
  int r;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // Set TCP protocol

  if ((r = getaddrinfo(address, "http", &hints, &servinfo)) != 0) {
    perror("getaddrinfo: wrong URL %s\n" + strlen(gai_strerror(r)));
    return -1;
  }

  // connect to the first addr that we can.
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
      close(s);
      perror("connect");
      continue;
    }
    break; // if we get here, connection must have established
  }

  if (p == NULL) {
    // if there above for does not got an connection
    perror("failed to connect\n");
    return -1;
  }
  freeaddrinfo(servinfo); // all done with this structure
  return s;
}


/**
Configures the blocking mode of an opened socket
@param[in] sock identifier of the socket to configure
@param[in] block 1 to set the socket to blocking mode, 0 to set it to non-blocking
*/
void tcp_set_blocking(int sock,char block)
{
   int flags;

#ifndef ENV_WINDOWS
   flags=fcntl(sock,F_GETFL, 0);
   if (block) flags&=~O_NONBLOCK;
   else flags|=O_NONBLOCK;
   fcntl(sock,F_SETFL, flags);
#else
   if (block) flags=0;
   else flags=13;
   ioctlsocket(sock,FIONBIO,(unsigned long*)&flags);
#endif
}
