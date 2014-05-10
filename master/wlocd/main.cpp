#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "libwlocate.h"

#ifdef ENV_QNX
 #define MSG_NOSIGNAL 0
#endif



int main(void)
{
   int                s,c,ret;
   socklen_t          addr_len;
   struct sockaddr_in addr;

   s = socket(PF_INET, SOCK_STREAM, 0);
   if (s == -1)
   {
       perror("socket() failed");
       return 1;
   }

#ifdef ENV_QNX
   addr.sin_addr.s_addr =inet_addr("0.0.0.0");   // strange behaviour: my QNX does not accept 127.0.0.1 to bind server sockets to it
#else
   addr.sin_addr.s_addr =inet_addr("127.0.0.1"); // this daemon is acting only local
#endif
   addr.sin_port = htons(10444);
   addr.sin_family = AF_INET;

   if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
   {
      perror("bind() failed");
      return 2;
   }

   if (listen(s, 3) == -1)
   {
      perror("listen() failed");
      return 3;
   }

//   daemon(0,0);
   while(true)
   {
      addr_len = sizeof(addr);
      c = accept(s, (struct sockaddr*)&addr, &addr_len);
      if (c>0)
      {
         struct wloc_req request;

         memset(&request,0,sizeof(struct wloc_req));
         ret=wloc_get_wlan_data(&request);
         if (ret<2) wloc_get_wlan_data(&request); // try two times in case the hardware was occupied or not able to find all networks
         if (ret>0) // no conversion from host to network byteorder necessary because we're always working on the same host
          send(c,&request,sizeof(struct wloc_req),MSG_NOSIGNAL);
         close(c);
      }
   }
   close(s);
   return 0;
}

