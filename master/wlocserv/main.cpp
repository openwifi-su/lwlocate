/**
 * wlocserv - server application for WLAN-based location service
 * Copyright (C) 2009-2012 Oxygenic/VWP virtual_worlds(at)gmx.de
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

#include "maindefs.h"

#if defined (ENV_LINUX) || defined (ENV_QNX)
 #include <sys/socket.h>       /*  socket definitions        */
 #include <sys/types.h>        /*  socket types              */
 #include <arpa/inet.h>        /*  inet (3) funtions         */
 #include <unistd.h>           /*  misc. UNIX functions      */
 #include <pwd.h>
 #include <sched.h>
 #include <sys/select.h>
 #include <netinet/in.h>
#endif
#ifdef ENV_WINDOWS
 #include <windows.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <mysql/mysql.h>

#ifndef ENV_QNX
#include "getopt.h"
#endif

#include "libwlocate.h"
#include "liboapc.h"
#include "platforms.h"
#include "login.h" // contains the login information, never add this header to the repository!


struct list_entry 
{
   struct list_entry *next;                 // the next structure in the list
   int                sock;                  
   int                readLen;       
   char              *remoteIP;             // the IP of the remote connection
   unsigned long int  remoteIPNum;
   time_t             timeout;
   struct wloc_req    request;
};

#ifndef TEST_MODE
static unsigned short    srcport=10443;
static bool              daemonMode=false;
#endif
static unsigned char     dSystemExit=0; // set to true by node /system/exit/0
static struct list_entry firstEntry;
static char             *userName;

#ifdef ENV_POSIX
static bool              verbose=false;
#endif

#ifdef ENV_QNX
#define MSG_NOSIGNAL 0
#endif



/**
 * This function creates a new list_entry and adds a new client to the internal list
 * of connections
 * @param list the list where the new entry has to be added
 * @param s_sock the socket descriptor of the new client connection
 * @param t_sock the socket descriptor of the forwarded connection
 * @parem remoteIP the remote IP of the client
 * @return 0 when the operation could be finished successfully or an error code otherwise
 */
int addClient(struct list_entry *list, int s_sock,unsigned long int remoteIP)
{
   struct list_entry *n;
   struct in_addr     iAddr;
   char              *c;

   n =(struct list_entry*)malloc(sizeof(struct list_entry));
   if (!n) 
   {
      return ERR_NO_MEMORY;
   }
   memset(n,0,sizeof(struct list_entry));
   n->sock = s_sock;
   iAddr.s_addr=ntohl(remoteIP);
   c=inet_ntoa(iAddr);
   if (c) n->remoteIP=strdup(c);
   else n->remoteIP=strdup("unknown host");
   n->remoteIPNum=ntohl(remoteIP);
   n->readLen=0;
   n->next = list->next;
   n->timeout=time(NULL);
   list->next= n;
   return 0;
}



/**
 * Whenever a client closes its connection it has to be removed from the list of active
 * clients and the resources of the related list_entry-structure have to be released.
 * That has to be done by calling this function.
 * @param list the list of entries where the client has to be removed from
 * @param entry the clients list entry that has to be removed
 * @return 0 in case the operation could be completed successfully or an error code
 *         otherwise
 */
int removeClient(struct list_entry *list,struct list_entry *entry)
{
   struct list_entry *lz, *lst = NULL;
   
   if (!entry) return 0;

   if (!list->next) return ERR_LIST_EMPTY;
   for (lz = list->next; lz; lz = lz->next)
      {
      if (lz->sock == entry->sock) break;
      lst = lz;
      }
   if (!lz) return ERR_NO_SUCH_ELEMENT;
   if (lst) lst->next = lz->next;
   else list->next = lz->next;
   if (lz->sock) oapc_tcp_closesocket(lz->sock);
   if (lz->remoteIP) free(lz->remoteIP);   
   free(lz);
   return 0;
}



/**
 * This function removes all entries from a given list of connections
 * @param list the list of entries that has to be flushed completely
 */
void freeList(struct list_entry *list)
{
   while ((list->next) && (list->next->sock)) removeClient(list,list->next);
}




/**
 * This function us used together with the socet function select(), it fills the file
 * descriptor set that is required by set depending on the available socket connections
 * @parem fds the file descriptor set structure to be filled
 * @param list the list of clients from which the socket information have to be used
 * @return the maximum socket file descriptor number
 */
int fillSet(fd_set *fds,
            struct list_entry *list)
{
   int max = 0;
   struct list_entry *lz;

   for (lz = list->next; lz; lz = lz->next)
   {
      if (lz->sock > max) max = lz->sock;
      if (lz->sock>0) FD_SET(lz->sock, fds);
   }
   return max;
}



/**
 * This function can be used after a call to the socekt function select(), it retrieves
 * that client out of a given list where a event occured that was watched by select().
 * @param fds the file descriptor set structure that was modified by select()
 * @param the related list of client conmnections that belongs to the preceding caqll of
 *        select and that was used by a former call to fillSet()
 * @return the client connection that caused select() to leave its waiting state or NULL
 *        in case there is no client socket that fits to the result
 */
struct list_entry *getClient(fd_set *fds,struct list_entry *list)
{
   int                i=0;
   struct list_entry *lz;

   while(!FD_ISSET(i, fds)) i++;
   lz=list->next;
   while (lz)
      {
      if (lz->sock==i) return lz;
      lz=lz->next;
      }
   return NULL;
}



/** definitions for the command line options */
static char shortopts[] = "vVhu:";
#ifndef ENV_QNX
static struct option const longopts[] = {
  {"user",             optional_argument, NULL, 'u'},
  {"version",          no_argument, NULL, 'v'},
#ifdef ENV_LINUX
  {"verbose",          no_argument, NULL, 'V'},
#endif  
  {"help",             no_argument, NULL, 'h'},
  {NULL, no_argument, NULL, 0}
};
#endif



/** definitions for the command line help output */
#ifdef ENV_QNX
static char const *const option_help[] = {
  " -u   switch to this users privileges",
  " -V   print out some debugging information",
  " -v   show version info",
  " -h   show this help",
  0
};
#else
static char const *const option_help[] = {
  " -u  --user      switch to the privileges of this user after creating the socket",
  #ifdef ENV_POSIX
  " -V  --verbose   print out some debugging information",
  #endif
  " -v  --version   show version info",
  " -h  --help      show this help",
  0
};
#endif


/** print out version information */
static void version(void)
{
printf ("%s %.1f %s\n%s\n", FCOMMON_NAME,FCOMMON_VERSION,FCOMMON_URL,FCOMMON_COPYRIGHT);
}



/** print out command line help information */
static void usage (char *pname)
{
printf ("usage: %s [OPTIONS]\n",pname);
printf (" --help for more information\n");
return;
}



/**
 * Get the switches and related values out of the command line parameters
 * @param argc the number of arguments as handed over by main()
 * @param argv the arguments as handed over by main()
 */
static void getSwitches(int argc,char *argv[])
{
int                optc;
char const *const *p;

if (optind == argc) return;
#ifdef ENV_QNX
while ((optc = getopt(argc, argv, shortopts)) != -1)
#else
while ((optc = getopt_long (argc, argv, shortopts, longopts, (int *) 0)) != -1)
#endif
   {
   switch (optc)
      {
#ifndef ENV_WINDOWS
      case 'V':
         verbose=true;
         break;
#endif
      case 'u':
         userName=strdup(optarg);
         break;
      case 'v':
         version();
         exit (0);
         break;
      case 'h':
         usage (argv[0]);
         for (p = option_help; *p; p++) printf ("%s\n", *p);
         exit (0);
         break;
      default:
         usage (argv[0]);
      }
   }
}



#ifndef TEST_MODE
/**
 * This function reads from the sockets and tries to fetch data from both connections. It reads
 * data only for a given time and stores them.
 * @param entry the list entry of the client where data have to be read for
 * @param timeout the maximum time in milliseconds that is allowed to be used to receive data from
 *        the clients socket
 * @return true in case a full package could be read successfully or false otherwise
 */
static bool readPackets(struct list_entry *entry, long timeout) 
{
   long /*size_t*/   rc;
   long     ctr=0;
   int      expLen;
   #ifdef ENV_WINDOWS
   long     err;
   #endif

   if (entry->readLen>=(int)offsetof(struct wloc_req,bssids))
   {
      if (entry->request.length==0)
      {
         entry->request.length=sizeof(struct wloc_req); // some old size
         showLog("WARNING: Forcing block size to %d\n",entry->request.length);
      }
#warning WORKAROUND!
      expLen=entry->request.length;
   }
   else
   {
      if (entry->readLen==0) memset(&entry->request,0,sizeof(entry->request));
      expLen=offsetof(struct wloc_req,bssids);
   }
   // data from source client side
   while (entry->readLen<expLen)
   {
      rc = recv(entry->sock,((char*)&entry->request)+entry->readLen,expLen-entry->readLen,MSG_NOSIGNAL);   
      if (rc>0)
      {
         entry->readLen+=rc;
         entry->timeout=time(NULL);
      }
      else if (rc==0) return false;
      else
      {
#ifdef ENV_WINDOWS
         err=GetLastError();
         if (err!=EAGAIN)
#else
         if ((errno!=EAGAIN) && (errno!=EINPROGRESS) && (errno!=0))
#endif
          return false;
         ctr+=oapc_util_thread_sleep(10);
      }
      if (ctr>timeout) break;
   }
   return true;
}



// Get the geolocation of a single BSSID from OpenBMap
static char get_wlan_location(double *lat,double *lon,char *bssid)
{
return 0;
   char             line[5001],body[5001],mime[101];
   int              sock,retCode;

   *lat=0;
   *lon=0;
   sock=oapc_tcp_connect_to("openbmap.org",80);
   if (sock<=0) return 0;
   sprintf(body,"bssid=%s",bssid);
   sprintf(line,"POST /api/getGPSfromWifiAPBSSID.php5 HTTP/1.1\r\nHost: openbmap.org\r\nAccept: text/xml, application/xml, */*\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s",strlen(body),body); 	
   if (oapc_tcp_send(sock,line,strlen(line),2000)!=(int)strlen(line))
   {
      printf("Sending head failed...\n");
      oapc_tcp_closesocket(sock);
      return 0;
   }
   memset(line,0,sizeof(line));
   while (oapc_tcp_recv(sock,line,1000,"\n",4000))
   {
      if (strstr(line,"HTTP/1.")==line)
      {
         retCode=atoi(line+9);
         if (retCode!=200)
         {
            oapc_tcp_closesocket(sock);
            return 0;
         };
      }
      else if (strstr(line,"Content-Type:")==line)
      {
         strncpy(mime,line+14,100);
      }
      else if ((line[0]=='\r') || (line[0]=='\n'))
      {
         memset(body,0,sizeof(body));
         while (oapc_tcp_recv(sock,body,5000,"\n",4000)>0)
         {
            if ((strstr(body,"lat")) && (strstr(body,"lng")))
            {
               char *c;
            		
               c=strstr(body,"lat=")+5;
               if ((c) && (c>body)) *lat=atof(c);
               else return 0;

               c=strstr(body,"lng=")+5;
               if ((c) && (c>body)) *lon=atof(c);
               else return 0;
               showLog("Position from openbmap.org: %f %f",*lat,*lon);
               return 1;
            }                            
            if ((strstr(body,"\r")==body) || (strstr(body,"\n")==body))
            {
               oapc_tcp_closesocket(sock);
               return 0;
            }                           
            memset(body,0,sizeof(body));
         }
      } 
      memset(line,0,sizeof(line));
   }
   oapc_tcp_closesocket(sock);
   return 0;
}



static int getCountryNum(const char *countryTxt)
{
   if (!strncmp(countryTxt,"DE",2)) return 1;
   else if (!strcmp(countryTxt,"AT")) return 2;
   else if (!strcmp(countryTxt,"CH")) return 3;
   else if (!strcmp(countryTxt,"NL")) return 4;
   else if (!strcmp(countryTxt,"BE")) return 5;
   else if (!strcmp(countryTxt,"LU")) return 6;
   else if (!strcmp(countryTxt,"NO")) return 7;
   else if (!strcmp(countryTxt,"SE")) return 8;
   else if (!strcmp(countryTxt,"DK")) return 9;
   else if (!strcmp(countryTxt,"AL")) return 12;
   else if (!strcmp(countryTxt,"DZ")) return 13;
   else if (!strcmp(countryTxt,"AR")) return 19;
   else if (!strcmp(countryTxt,"AU")) return 21;
   else if (!strcmp(countryTxt,"BS")) return 23;
   else if (!strcmp(countryTxt,"BH")) return 24;
   else if (!strcmp(countryTxt,"BD")) return 25;
   else if (!strcmp(countryTxt,"BB")) return 26;
   else if (!strcmp(countryTxt,"BY")) return 27;
   else if (!strcmp(countryTxt,"BZ")) return 28;
   else if (!strcmp(countryTxt,"BJ")) return 29;
   else if (!strcmp(countryTxt,"BM")) return 30;
   else if (!strcmp(countryTxt,"BA")) return 33;
   else if (!strcmp(countryTxt,"BR")) return 36;
   else if (!strcmp(countryTxt,"BG")) return 38;
   else if (!strcmp(countryTxt,"CA")) return 43;
   else if (!strcmp(countryTxt,"CL")) return 47;
   else if (!strcmp(countryTxt,"CN")) return 48;
   else if (!strcmp(countryTxt,"CO")) return 49;
   else if (!strcmp(countryTxt,"CR")) return 52;
   else if (!strcmp(countryTxt,"HR")) return 53;
   else if (!strcmp(countryTxt,"CZ")) return 56;
   else if (!strcmp(countryTxt,"PR")) return 59;
   else if (!strcmp(countryTxt,"EC")) return 60;
   else if (!strcmp(countryTxt,"EG")) return 61;
   else if (!strcmp(countryTxt,"Es")) return 65;
   else if (!strcmp(countryTxt,"FI")) return 68;
   else if (!strcmp(countryTxt,"FR")) return 69;
   else if (!strcmp(countryTxt,"HN")) return 73;
   else if (!strcmp(countryTxt,"GR")) return 75;
   else if (!strcmp(countryTxt,"GL")) return 76;
   else if (!strcmp(countryTxt,"GU")) return 78;
   else if (!strcmp(countryTxt,"GT")) return 79;
   else if (!strcmp(countryTxt,"HK")) return 84;
   else if (!strcmp(countryTxt,"HU")) return 85;
   else if (!strcmp(countryTxt,"IS")) return 86;
   else if (!strcmp(countryTxt,"IN")) return 87;
   else if (!strcmp(countryTxt,"ID")) return 88;
   else if (!strcmp(countryTxt,"IR")) return 89;
   else if (!strcmp(countryTxt,"IQ")) return 90;
   else if (!strcmp(countryTxt,"IE")) return 91;
   else if (!strcmp(countryTxt,"IL")) return 92;
   else if (!strcmp(countryTxt,"IT")) return 93;
   else if (!strcmp(countryTxt,"JM")) return 94;
   else if (!strcmp(countryTxt,"JP")) return 95;
   else if (!strcmp(countryTxt,"JO")) return 97;
   else if (!strcmp(countryTxt,"KZ")) return 98;
   else if (!strcmp(countryTxt,"KE")) return 99;
   else if (!strcmp(countryTxt,"KR")) return 102;
   else if (!strcmp(countryTxt,"KW")) return 103;
   else if (!strcmp(countryTxt,"KG")) return 104;
   else if (!strcmp(countryTxt,"LB")) return 107;
   else if (!strcmp(countryTxt,"LS")) return 108;
   else if (!strcmp(countryTxt,"LV")) return 105;
   else if (!strcmp(countryTxt,"LT")) return 111;
   else if (!strcmp(countryTxt,"MY")) return 115;
   else if (!strcmp(countryTxt,"MV")) return 116;
   else if (!strcmp(countryTxt,"MQ")) return 119;
   else if (!strcmp(countryTxt,"MX")) return 123;
   else if (!strcmp(countryTxt,"MC")) return 124;
   else if (!strcmp(countryTxt,"MA")) return 126;
   else if (!strcmp(countryTxt,"MZ")) return 127;
   else if (!strcmp(countryTxt,"NZ")) return 131;
   else if (!strcmp(countryTxt,"NI")) return 133;
   else if (!strcmp(countryTxt,"NG")) return 135;
   else if (!strcmp(countryTxt,"PK")) return 138;
   else if (!strcmp(countryTxt,"PA")) return 141;
   else if (!strcmp(countryTxt,"PY")) return 142;
   else if (!strcmp(countryTxt,"PE")) return 144;
   else if (!strcmp(countryTxt,"PH")) return 145;
   else if (!strcmp(countryTxt,"PL")) return 147;
   else if (!strcmp(countryTxt,"PT")) return 148;
   else if (!strcmp(countryTxt,"QA")) return 150;
   else if (!strcmp(countryTxt,"RO")) return 151;
   else if (!strcmp(countryTxt,"RU")) return 152;
   else if (!strcmp(countryTxt,"SM")) return 155;
   else if (!strcmp(countryTxt,"SA")) return 157;
   else if (!strcmp(countryTxt,"SN")) return 158;
   else if (!strcmp(countryTxt,"SG")) return 161;
   else if (!strcmp(countryTxt,"SK")) return 162;
   else if (!strcmp(countryTxt,"SI")) return 163;
   else if (!strcmp(countryTxt,"ZA")) return 166;
   else if (!strcmp(countryTxt,"ES")) return 167;
   else if (!strcmp(countryTxt,"SD")) return 169;
   else if (!strcmp(countryTxt,"SY")) return 172;
   else if (!strcmp(countryTxt,"TW")) return 173;
   else if (!strcmp(countryTxt,"TJ")) return 174;
   else if (!strcmp(countryTxt,"TZ")) return 175;
   else if (!strcmp(countryTxt,"TH")) return 176;
   else if (!strcmp(countryTxt,"TT")) return 179;
   else if (!strcmp(countryTxt,"TN")) return 180;
   else if (!strcmp(countryTxt,"TR")) return 181;
   else if (!strcmp(countryTxt,"UA")) return 185;
   else if (!strcmp(countryTxt,"AE")) return 186;
   else if (!strcmp(countryTxt,"GB")) return 187;
   else if (!strcmp(countryTxt,"US")) return 188;
   else if (!strcmp(countryTxt,"UY")) return 189;
   else if (!strcmp(countryTxt,"VE")) return 191;
   else if (!strcmp(countryTxt,"VN")) return 192;
   else if (!strcmp(countryTxt,"ZM")) return 195;
   else return -1;
}
#endif // TEST_MODE


/**
 * This is the main loop, it is called by main() after several necessary initializations have been done.
 * This main loop accepts new incoming connections, tries to read data from all available connections,
 * parses received data blocks, invokes the required (re)action for the received data and sends a
 * response back to the client.
 */
static bool mainLoop(void* /*hInstance*/)
{
#ifndef TEST_MODE
   int                list_s;                /*  listening socket          */
   unsigned long      remoteIP;
   unsigned int       num_fields;
   struct in_addr     in_address;
   int                c,max;
   fd_set             fds;
   MYSQL_RES         *sqlResult=NULL;
   MYSQL_ROW          row;
   int                on;
   time_t             currTime=time(NULL);
#else
   struct list_entry  client_mem;
#endif
   MYSQL             *conn=NULL;
   char               query[1000+1];
   struct list_entry *client;
   int                country;
   struct wloc_res    result;
   time_t             lastDBAccess=0;
   double             lat[WLOC_MAX_NETWORKS],lon[WLOC_MAX_NETWORKS];
   double             latTotal,lonTotal,sumTotal;
   int                i,j,sum;
   bool               doRecalculate,wasIPbased;

   result.version=1;
   result.length=sizeof(struct wloc_res);

#ifdef TEST_MODE
   client=&client_mem;
   memset(client,0,sizeof(struct list_entry));
   client->request.bssids[0][0]=1;
   client->request.bssids[1][0]=2;
//   client->request.bssids[2][0]=3;
//   client->request.bssids[3][0]=4;
//   client->request.bssids[4][0]=5;
#else
#ifdef ENV_POSIX
   if (daemonMode)
#endif
   showLog("Starting %s...",FCOMMON_NAME);
   #ifdef ENV_POSIX
   if (verbose) printf("Listening...\n");
   #endif
   list_s=oapc_tcp_listen_on_port(srcport,"62.112.140.13");
   if (list_s<=0)
   {
      #ifdef ENV_WINDOWS
      errno=GetLastError();
      #else
      perror("");
      #endif
      showLog("ERROR: could not create listening socket %s:%d, %d!","api.openwlanmap.org",srcport,errno);
      return false;
   }
   oapc_tcp_set_blocking(list_s,0);
   if (setsockopt(list_s,SOL_SOCKET,SO_KEEPALIVE,(char*)&on,sizeof(on))<0)
   {
       #ifdef ENV_WINDOWS
       errno=GetLastError();
       #else
       perror("");
       #endif
      showLog("WARNING: could not set socket options, %d!",errno);
   }
   showLog("INFO: binding to %s:%d","api.openwlanmap.org",srcport);

#ifdef ENV_POSIX
   if (userName)
   {
      struct passwd *pwd;

      pwd=getpwnam(userName);
      if (!pwd) showLog("ERROR: unknown username %s!\n",optarg);
      else
      {
         if (setgid(pwd->pw_uid)!=0) showLog("ERROR: Setting group failed");
         if (setuid(pwd->pw_uid)!=0) showLog("ERROR: Setting user failed");
      }
      free(userName);
   }

   if (daemonMode) daemon(0,0);
   #endif
   while ((!dSystemExit) && (!LeaveServer()))
   {
      FD_ZERO(&fds);
      max = fillSet(&fds, &firstEntry);
      FD_SET(list_s, &fds);
      if (list_s > max) max = list_s;
      select(max + 1, &fds, NULL, NULL, NULL);
   
      if (FD_ISSET(list_s, &fds))
      {
         c=oapc_tcp_accept_connection(list_s, &remoteIP);
         if (c>0)
         {
#ifdef ENV_WINDOWS
            in_address.S_un.S_addr=ntohl(remoteIP);
#else
            in_address.s_addr=ntohl(remoteIP);
#endif
            oapc_tcp_set_blocking(c,0);
            addClient(&firstEntry,c,remoteIP);
            showLog("New client accepted from %s",inet_ntoa(in_address));
         }
      }
      else
      {
         // get the client that sends new data at the moment
         client = getClient(&fds,&firstEntry);
         if (client)
         {
            if ((!readPackets(client,5)) && (client->readLen==0))
            {
               // remove a client if the connection was lost
               showLog("Connection lost to %s",client->remoteIP);
               removeClient(&firstEntry, client);
            }
            if ((client->readLen>(int)offsetof(struct wloc_req,bssids)) && (client->readLen>=client->request.length))
            {
               oapc_util_thread_sleep(550); // do not hurry with the responses to avoid heavy load
               if ((conn) && (lastDBAccess+60<time(NULL)))
               {
                  showLog("Closing DB connection for refresh-cycle");
                  mysql_close(conn);
                  conn=NULL;
               }
               if (!conn)
               {
                  conn = mysql_init(NULL);
                  if (conn)
                  {
                     if (!mysql_real_connect(conn,DB_HOST,DB_UNAME,DB_PWD,DB_NAME,0,NULL,0)) // login data are simple defines stored in login.h which is NOT part of the repository!
                     {
                        mysql_close(conn);
                        conn=NULL;                     	
                     }   	                
                     else showLog("DB connection established successfully");
                  }
               }
               country=-1; 
               wasIPbased=false;
               if (!conn) // no access to DB, return error
               {
                  result.result=WLOC_RESULT_IERROR;
                  result.iresult=1;
               }
               else
               {

#endif // TEST_MODE

                  result.result=WLOC_RESULT_OK;
                  result.lat=0;
                  result.lon=0;
                  lastDBAccess=time(NULL);
                  result.quality=0;
                  latTotal=0.0;
                  lonTotal=0.0;
                  sumTotal=0.0;
                  for (i=0; i<WLOC_MAX_NETWORKS; i++) // retrieve data of requested bssids
                  {
                     sum=0;
                     lat[i]=-1000;
                     if ((client->request.signal[i]<=0) || (client->request.signal[i]>100)) client->request.signal[i]=60;
                     
                     for (j=0; j<6; j++) sum+=client->request.bssids[i][j];
                     #ifdef ENV_POSIX
                     if (verbose) printf("Next AP: %02X%02X%02X%02X%02X%02X, sum=%d\n",
                                         client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                         client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF,
                                         sum);
                     #endif
                     if (sum!=0)
                     {
                        snprintf(query,1000,"SELECT lat,lon,country,source FROM netpoints WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                 client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                 client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
//printf("%s\n",query);                                 
#ifndef TEST_MODE
                        if (mysql_query(conn,query)!=0)
                        {
                           showLog("Query error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                        }
                        else
#endif 
                        {
                           //mysql_field_count() to check if there would be a result -> save DB load?
#ifndef TEST_MODE
                           sqlResult=mysql_store_result(conn);
                           if (!sqlResult) 
                           {
                              showLog("Result error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                           }
                           else
#endif
                           {
#ifndef TEST_MODE
                              if (mysql_num_rows(sqlResult)>0)
#endif
                              {
#ifndef TEST_MODE
                                 #ifdef ENV_POSIX
                                 if (verbose) printf("Got SQL result...\n");
                                 #endif
                                 num_fields = mysql_num_fields(sqlResult);
                                 if (num_fields<2)
                                 {
                                    showLog("Fields error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                                 }
                                 else if ((row = mysql_fetch_row(sqlResult)) && (row[0]) && (row[1]) && (row[3]))
                                 {
                                    lat[i]=atof(row[0]);
                                    lon[i]=atof(row[1]);   
                                    if (row[2])
                                    {
                                       sum=atoi(row[2]);
                                       if ((sum>0) && (sum<250)) country=sum;
                                    }
                                    else country=-1;
                                    if ((!row[3]) || (row[3][0]=='3')) client->request.signal[i]/=2; // interpolated location
                                    else if (row[3][0]=='0') client->request.signal[i]*=2;           // manually entered by users of homepage / owners of AP
                                    else if (row[3][0]=='4') client->request.signal[i]/=3;           // g-fetched Location
                                    else if (row[3][0]=='5') client->request.signal[i]/=3;           // g-collected data
                                    // 1 - imported from other db
                                    // 2 - scanned for my own
                                    // 6 - uploads from wardrivers
                                    // 7 - uploaded by OWLMap@Android App
                                    #ifdef ENV_POSIX
                                    if (verbose) printf("Result for %02X%02X%02X%02X%02X%02X: Lat: %f Lon: %f, Signal: %d\n",
                                                        client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                                        client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF,
                                                        lat[i],lon[i],client->request.signal[i]);
                                    #endif
                                 }
                                 else
                                 {
                                    #ifdef ENV_POSIX
                                    if (verbose) printf("Error fetching row %p %p %p\n",row[0],row[1],row[3]);
                                    #endif                                 
                                 }
#else
                                 if (i==0)
                                 {
                                 	lat[i]=48.141; lon[i]=11.60123;
                                 }
                                 else if (i==1)
                                 {
                                 	lat[i]=48.14575;  lon[i]=11.60505;
                                 }
                                 else if (i==2)
                                 {
                                 	lat[i]=48.140767; lon[i]=11.599917;
                                 }
                                 else if (i==3)
                                 {
                                 	lat[i]=48.140717; lon[i]=11.59952;
                                 }
                                 else if (i==4)
                                 {
                                 	lat[i]=48.140825; lon[i]=11.5999;
                                 }
#endif
                              }
#ifndef TEST_MODE
                              else
                              {
                                 char countryTxt[100];
                              
                                 #ifdef ENV_POSIX
                                 if (verbose) printf("New AP %02X%02X%02X%02X%02X%02X, checking at external source\n",
                                                     client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                                     client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                 #endif
                                 snprintf(countryTxt,99,"%02X-%02X-%02X-%02X-%02X-%02X",
                                          client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                          client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                          client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                 if (get_wlan_location(&lat[i],&lon[i],countryTxt)) // get info from Google
                                 {
                                    #ifdef ENV_POSIX
                                    if (verbose) printf("New AP %02X%02X%02X%02X%02X%02X, adding to own database\n",
                                                        client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                                        client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                    #endif
                                    snprintf(query,1000,"INSERT INTO netpoints (bssid, lat, lon, timestamp, source, country) VALUES ('%02X%02X%02X%02X%02X%02X', '%f', '%f', '%ld', '4', '%d')",
                                             client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                             client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                             client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF,
                                             lat[i],lon[i],(unsigned long)time(NULL),0);
printf("Query: %s\n",query);
                                    mysql_query(conn,query);
                                    client->request.signal[i]=25; // accuracy is quite poor, so set a small weight for this position
                                 }
                                 else
                                 {
                                    #ifdef ENV_POSIX
                                    if (verbose) printf("New AP %02X%02X%02X%02X%02X%02X, not found at external source\n",
                                                        client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                                        client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                    #endif
                                    client->request.signal[i]=-1; // mark as not found
                                    lat[i]=-1000;
                                    lon[i]=-1000;
                                 }
                              }
                              mysql_free_result(sqlResult);
#endif
                              if (lat[i]>-900)
                              {
                                 #ifdef ENV_POSIX
                                 if (verbose) printf("AP %02X%02X%02X%02X%02X%02X, updating use counter\n",
                                                     client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,client->request.bssids[i][2] & 0xFF,
                                                     client->request.bssids[i][3] & 0xFF,client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                 #endif
                                 snprintf(query,1000,"UPDATE netpoints SET usecnt=usecnt+1 WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                          client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                          client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                          client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
//showLog("Query: %s\n",query);                                 
                                 mysql_query(conn,query);
                                 if ((client->request.signal[i]<=0) || (client->request.signal[i]>100)) client->request.signal[i]=60;
                                 latTotal+=(lat[i]*client->request.signal[i]/100.0);
                                 lonTotal+=(lon[i]*client->request.signal[i]/100.0);
                                 sumTotal+=(client->request.signal[i]/100.0);
                                 result.quality++;
                              }
                           }
                        }
                     }
                  }
                  doRecalculate=false;
                  if (result.quality==0)
                  {
                     char         countryTxt[3];

                     #ifdef ENV_POSIX
                     if (verbose) printf("No position, performing IP-based localisation\n");
                     #endif
                     if (client->request.cgiIP==0) client->request.cgiIP=client->remoteIPNum;
                      snprintf(query,1000,"SELECT ipoints.lat,ipoints.lon,ipoints.ccode FROM ips,ipoints WHERE ips.fromip<=%u AND ips.toip>=%u AND ips.kidx=ipoints.idx",
                               ntohl(client->request.cgiIP),ntohl(client->request.cgiIP));
printf("%s\n",query);
                     if (mysql_query(conn,query)!=0)
                     {
                        showLog("Query error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                        result.result=WLOC_RESULT_ERROR; // nothing found
                     }
                     else
                     {
                        //mysql_field_count() to check if there would be a result -> save DB load?
                        sqlResult=mysql_store_result(conn);
                        if (!sqlResult)
                        {
                           showLog("Result error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                           result.result=WLOC_RESULT_ERROR; // nothing found
                        }
                        else
                        {
                           if (mysql_num_rows(sqlResult)>0)
                           {
                              num_fields = mysql_num_fields(sqlResult);
                              if (num_fields<3)
                              {
                                 result.result=WLOC_RESULT_ERROR; // nothing found
                                 showLog("Fields error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                              }
                              else if ((row = mysql_fetch_row(sqlResult)) && (row[0]) && (row[1]) && (row[2]))
                              {
                                 int ival;
//printf("Lat: %s - %f - %d Lon: %s - %f - %d\n",row[0],atof(row[0])*10000000.0,(int)(atof(row[0])*10000000.0),row[1],atof(row[1])*10000000.0,(int)(atof(row[1])*10000000.0));
                                 ival=(int)(atof(row[0])*10000000.0);
                                 result.lat=htonl(ival);
                                 ival=(int)(atof(row[1])*10000000.0);
                                 result.lon=htonl(ival);
                                 strncpy(countryTxt,row[2],2);
                                 countryTxt[2]=0;
                                 result.quality=0; // poor IP-based accuracy
                                 country=getCountryNum(countryTxt);
                                 result.result=WLOC_RESULT_OK;
                                 wasIPbased=true;
                              }
                           }
                           else
                           {
                              result.result=WLOC_RESULT_ERROR; // nothing found
                              showLog("Row-number-error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                           }
                        }
                     }
                  }
                  else
                  {
                     latTotal/=sumTotal;
                     lonTotal/=sumTotal;
                     #ifdef ENV_POSIX
                     if (verbose) printf("Intermediate position: %f %f\n",latTotal,lonTotal);
                     #endif

                     if (result.quality>2)      // if there are more than two hits we can check if one of them is out of range
                     {                     	
                        for (i=0; i<WLOC_MAX_NETWORKS; i++)
                        {
                           double latMaxDist=0.0,lonMaxDist=0.0;
                           int    maxPos=-1;
                        
                           for (j=0; j<WLOC_MAX_NETWORKS; j++)
                           {
                              if (lat[j]>-900)
                              {
                                 if ((fabs(latTotal-lat[j])>latMaxDist) || (fabs(lonTotal-lon[j])>lonMaxDist))
                                 {
                                    latMaxDist=fabs(latTotal-lat[j]);
                                    lonMaxDist=fabs(lonTotal-lon[j]);
                                    maxPos=j;
                                    }
                                 }
                           }
                           if ((latMaxDist>0.003) || (lonMaxDist>0.003))
                           {
                              #ifdef ENV_POSIX
                              if (verbose) printf("Maximum error distances %f %f for AP %02X%02X%02X%02X%02X%02X, increasing error count\n",
                                                  latMaxDist,lonMaxDist,
                                                  client->request.bssids[maxPos][0] & 0xFF,client->request.bssids[maxPos][1] & 0xFF,
                                                  client->request.bssids[maxPos][2] & 0xFF,client->request.bssids[maxPos][3] & 0xFF,
                                                  client->request.bssids[maxPos][4] & 0xFF,client->request.bssids[maxPos][5] & 0xFF);
                              #endif                           	
                              snprintf(query,1000,"UPDATE netpoints SET errcnt=errcnt+1, usecnt=usecnt-1 WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                       client->request.bssids[maxPos][0] & 0xFF,client->request.bssids[maxPos][1] & 0xFF,
                                       client->request.bssids[maxPos][2] & 0xFF,client->request.bssids[maxPos][3] & 0xFF,
                                       client->request.bssids[maxPos][4] & 0xFF,client->request.bssids[maxPos][5] & 0xFF);
                              showLog("Updating error counter for %02X:%02X:%02X:%02X:%02X:%02X",
                                      client->request.bssids[maxPos][0] & 0xFF,client->request.bssids[maxPos][1] & 0xFF,
                                      client->request.bssids[maxPos][2] & 0xFF,client->request.bssids[maxPos][3] & 0xFF,
                                      client->request.bssids[maxPos][4] & 0xFF,client->request.bssids[maxPos][5] & 0xFF);
                              mysql_query(conn,query);
                              
                              // delete from DB in case of too much errors
                              snprintf(query,1000,"SELECT errcnt, usecnt FROM netpoints WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                       client->request.bssids[maxPos][0] & 0xFF,client->request.bssids[maxPos][1] & 0xFF,
                                       client->request.bssids[maxPos][2] & 0xFF,client->request.bssids[maxPos][3] & 0xFF,
                                       client->request.bssids[maxPos][4] & 0xFF,client->request.bssids[maxPos][5] & 0xFF);
                              if (mysql_query(conn,query)!=0)
                               showLog("Query error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                              else
                              {
                                 //mysql_field_count() to check if there would be a result -> save DB load?
                                 sqlResult=mysql_store_result(conn);
                                 if (!sqlResult)
                                  showLog("Result error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                                 else
                                 {
                                    if (mysql_num_rows(sqlResult)>0)
                                    {
                                       num_fields = mysql_num_fields(sqlResult);
                                       if (num_fields<2)
                                        showLog("Fields error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                                       else if ((row = mysql_fetch_row(sqlResult)) && (row[0]) && (row[1]))
                                       {
                                          int errval,useval;

                                          errval=(int)atoi(row[0]);
                                          useval=(int)atoi(row[1]);
                                          
                                          if (((errval>=useval) && (useval>20)) || (errval>40))
                                          {
                                             snprintf(query,1000,"DELETE FROM netpoints WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                                      client->request.bssids[maxPos][0] & 0xFF,client->request.bssids[maxPos][1] & 0xFF,
                                                      client->request.bssids[maxPos][2] & 0xFF,client->request.bssids[maxPos][3] & 0xFF,
                                                      client->request.bssids[maxPos][4] & 0xFF,client->request.bssids[maxPos][5] & 0xFF);
                                             mysql_query(conn,query);
showLog("Delete Entry Query: %s\n",query);                                 
                                          }
                                       }                                       
                                    }
                                 }
                              }
                              lat[maxPos]=-2000;
                              lon[maxPos]=-2000;
                              result.quality--;

                              latTotal=0.0;
                              lonTotal=0.0;
                              sumTotal=0.0;
                              for (j=0; j<WLOC_MAX_NETWORKS; j++)
                              {
                                 if (lat[j]>-900)
                                 {
                                    if ((client->request.signal[j]<=0) || (client->request.signal[j]>100)) client->request.signal[j]=60;
                                    latTotal+=(lat[j]*client->request.signal[j]/100.0);
                                    lonTotal+=(lon[j]*client->request.signal[j]/100.0);
                                    sumTotal+=(client->request.signal[j]/100.0);
printf("2: %f %f %f - %d\n",lat[j],lon[j],sumTotal,client->request.signal[j]);
                                 }
                              }
                              latTotal/=sumTotal;
                              lonTotal/=sumTotal;
                              if (result.quality<=1) break;
                           }
                        }
                        	
/*                           if (lat[i]>-900)
                           {
                              if ((fabs(latTotal-lat[i])>0.003) || (fabs(lonTotal-lon[i])>0.003))
                              {
                                 snprintf(query,1000,"UPDATE netpoints SET errcnt=errcnt+1 WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                          client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                          client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                          client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                 showLog("Updating error counter for %02X:%02X:%02X:%02X:%02X:%02X",
                                         client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                         client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                         client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
//                                 mysql_query(conn,query);
                                 lat[i]=-2000;
                                 lon[i]=-2000;
                                 doRecalculate=true;
                                 result.quality--;
                              }  
                           }
                        }*/
                        
                        if (result.quality<=0)
                        {
                           result.quality=0;
                           result.result=WLOC_RESULT_ERROR; // nothing found
                        }
                     }
                     else // only two bssid's so we only can check if they are close to each other or not
                     {
                        for (i=0; i<WLOC_MAX_NETWORKS; i++)
                        {
                           if (lat[i]>-900)
                           {
                              if ((fabs(latTotal-lat[i])>0.00225) || (fabs(lonTotal-lon[i])>0.00225)) // smaller error threshold because we have less values for averaging
                              {
                                 result.quality=0;
                                 result.result=WLOC_RESULT_ERROR; // nothing found

                                 snprintf(query,1000,"UPDATE netpoints SET errcnt=errcnt+1 WHERE bssid='%02X%02X%02X%02X%02X%02X'",
                                          client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                          client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                          client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
                                 showLog("Updating error counter for %02X:%02X:%02X:%02X:%02X:%02X",
                                         client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                         client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                         client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF);
//                                 mysql_query(conn,query);
                              }
                           }
                        }
                     }
                     if ((result.quality>=3) && (!doRecalculate)) // enough known WLANs with no error found to store the other ones with interpolated values
                     {
                        for (i=0; i<WLOC_MAX_NETWORKS; i++)
                        {
                           if (client->request.signal[i]==-1)
                           {
                              snprintf(query,1000,"INSERT INTO netpoints (bssid, lat, lon, timestamp, source, country) VALUES ('%02X%02X%02X%02X%02X%02X', '%f', '%f', '%ld', '3', '%d')",
                                       client->request.bssids[i][0] & 0xFF,client->request.bssids[i][1] & 0xFF,
                                       client->request.bssids[i][2] & 0xFF,client->request.bssids[i][3] & 0xFF,
                                       client->request.bssids[i][4] & 0xFF,client->request.bssids[i][5] & 0xFF,
                                       latTotal,lonTotal,(unsigned long)time(NULL),country);
printf("Query: %s\n",query);                                 
                              mysql_query(conn,query);
                           }
                        }
                     }
                     result.lat=htonl((int)(latTotal*10000000.0));
                     result.lon=htonl((int)(lonTotal*10000000.0));
                  }
                  if ((result.quality>0) && (result.quality<3)) result.quality=1; // with less than 3 valid BSSIDs we have the lowest quality
#ifdef TEST_MODE
printf("Returning Result: %d Quality: %d Lat: %f Lon: %f Country: %d\n",
       result.result,result.quality,ntohl(result.lat)/10000000.0,ntohl(result.lon)/10000000.0,country);
#else                  
printf("Returning Result: %d Quality: %d Lat: %f Lon: %f Country: %d\n",
       result.result,result.quality,ntohl(result.lat)/10000000.0,ntohl(result.lon)/10000000.0,country);
               }
               result.quality*=10;                       // send a percentual value
               if (result.quality>99) result.quality=99; // never give 100%
               result.ccode=htons(country);
               oapc_tcp_send(client->sock,(char*)&result,sizeof(struct wloc_res),500);
               if (!wasIPbased)
               {
                  // check the geolocation of the related users IP and correct it in case it differs too much
/*
                  if (client->request.cgiIP==0) client->request.cgiIP=client->remoteIPNum;
                  snprintf(query,1000,"SELECT ipoints.lat,ipoints.lon FROM ips,ipoints WHERE ips.fromip<=%u AND ips.toip>=%u AND ips.kidx=ipoints.idx",
                           ntohl(client->request.cgiIP),ntohl(client->request.cgiIP));
                  if (mysql_query(conn,query)!=0)
                  {
                     showLog("Query error: %s %d %s\n",__FILE__,__LINE__,mysql_error(conn));
                     result.result=WLOC_RESULT_ERROR; // nothing found
                  }
                  else
                  {
                     //mysql_field_count() to check if there would be a result -> save DB load?
                     sqlResult=mysql_store_result(conn);
                     if (sqlResult)
                     {
                        if (mysql_num_rows(sqlResult)>0)
                        {
                           num_fields = mysql_num_fields(sqlResult);
                           if (num_fields>=2)
                           {
                              if ((row = mysql_fetch_row(sqlResult)) && (row[0]) && (row[1]) && (row[2]))
                              {
                                 if ((fabs(atof(row[0])-latTotal)>0.4) || (fabs(atof(row[1])-lonTotal)>0.4))
                                 {
                                    // update IP position
test SQL-statement!
                                    snprintf(query,1000,"UPDATE ips,ipoints SET ipoints.lat='%f',ipoints.lon='%f' WHERE ips.fromip<=%u AND ips.toip>=%u AND ips.kidx=ipoints.idx",
                                             ((int)(latTotal*1000.0))/1000.0,((int)(lonTotal*1000.0))/1000.0,
                                             ntohl(client->request.cgiIP),ntohl(client->request.cgiIP));
                                    mysql_query(conn,query);
                                 }
                              }
                           }
                        }
                     }
                  }
*/
               }
               showLog("Closing connection to %s with response %d",client->remoteIP,result.result);
               removeClient(&firstEntry, client);
            }
         }
         currTime=time(NULL);
         if (currTime%10==0)
         {
            struct list_entry *list;
            
            list=&firstEntry;
            while (list)
            {
               if ((list->timeout!=0) && (list->timeout+60<time(NULL)))
               {
                  showLog("Removing timed-out connection from %s (%d - %d)",client->remoteIP,list->timeout,time(NULL));
                  removeClient(&firstEntry,list);
                  break; // only remove one client per try
               }
               list=list->next;
            }
         }
      }
   }
   freeList(&firstEntry);
   if (conn) mysql_close(conn);
#endif // TEST_MODE
   return true;
}



#ifdef ENV_WINDOWS
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
WSADATA   wsaData;

#else
int main(int argc,char *argv[])
{
void*          hInstance=NULL;
#endif
#ifdef ENV_WINDOWS
int            argc;
char          *argv[100];
//unsigned short argv0[300];

WSAStartup((MAKEWORD(1, 1)), &wsaData);
argc=splitCmdLine(GetCommandLine(),argv);
//GetModuleFileName(NULL,argv0,300);
appName="wlocserv.exe";//argv0;
#else
setupSignalTraps();
//appName=argv[0];
#endif
memset(&firstEntry,0,sizeof(firstEntry));
getSwitches(argc,argv);
{
   srand(time(NULL));

   #ifdef ENV_POSIX
   if (verbose) printf("Starting mainloop...\n");
   #endif

   if ((!dSystemExit) && (mainLoop(hInstance)))
   {
      #ifdef ENV_WINDOWS
      while (!LeaveServer())
      {
         oapc_util_thread_sleep(100);
      }
      #endif
   }
}
#ifdef ENV_WINDOWS
WSACleanup();
#endif
#ifdef ENV_POSIX
if (verbose) printf("Exiting mainloop...\n");
#endif

#ifdef ENV_POSIX
if (verbose) printf("Releasing buffered data...\n");
#endif
showLog("Exiting server!");               
return 0;
}




