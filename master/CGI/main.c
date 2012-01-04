/**
 * geolocation.cgi - CGI for a standard JSON-based geolocation interface
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


//#define PRINT_DEBUG // define this to get debugging output - but please note, these data will be sent to
                      // the client through the cgi when the application is called by the webbrowser! 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <libwlocate.h>
#include <time.h>
#include <netdb.h>

#ifdef ENV_QNX
#include <netinet/in.h>
#endif

#include "JSON_parser.h"

static struct wloc_req request;
static int    wifiCnt=-1;
static char   macAddressKey=0,signalStrengthKey=0,requestAddressKey=0;
static char   doRequestAddress=0,exitLoop=0;
static size_t s_Level = 0;
static int    s_IsKey = 0;
#ifdef PRINT_DEBUG
 static const char* s_pIndention = "  ";
#endif



extern int get_position(struct wloc_req *request,double *lat,double *lon,char *quality,short *ccode);



#ifdef PRINT_DEBUG
static void print_indention()
{
   size_t i;
    
   for (i = 0; i < s_Level; ++i) printf("%s", s_pIndention);
}
#endif



/**
 * The callback function of the JSON-parser, it is called for every token and every level
 * of the JSON structure
 */
static int print(void* ctx, int type, const JSON_value* value)
{
   switch(type) {
   case JSON_T_ARRAY_BEGIN:    
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("[\n");
#endif
      s_IsKey = 0;
      ++s_Level;
      break;
   case JSON_T_ARRAY_END:
      assert(!s_IsKey);
      if (s_Level > 0) s_Level--;
#ifdef PRINT_DEBUG
      print_indention();
      printf("]\n");
#endif
      break;
   case JSON_T_OBJECT_BEGIN:
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("{\n");
#endif
      s_IsKey = 0;
      ++s_Level;
      break;
   case JSON_T_OBJECT_END:
      assert(!s_IsKey);
      if (s_Level > 0) s_Level--;
      if (s_Level==0) exitLoop=1;   
#ifdef PRINT_DEBUG
      print_indention();
      printf("}\n");
#endif
      if (wifiCnt>=0) wifiCnt++; // count up in case we're within a wifi_towers-array
      break;
   case JSON_T_INTEGER:
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("integer: "JSON_PARSER_INTEGER_SPRINTF_TOKEN"\n", value->vu.integer_value);
#endif
      s_IsKey = 0;
      if (signalStrengthKey)
      {
         signalStrengthKey=0;
         if (wifiCnt>=0)
          request.signal[wifiCnt]=(char)((100+value->vu.integer_value)*1.6); // is this really the correct calculation for a signal strength in percent?
      }
      break;
#ifdef PRINT_DEBUG
   case JSON_T_FLOAT:
      if (!s_IsKey) print_indention();
      s_IsKey = 0;
      printf("float: %f\n", value->vu.float_value); /* We wanted stringified floats */
      break;
   case JSON_T_NULL:
      if (!s_IsKey) print_indention();
      s_IsKey = 0;
      printf("null\n");
      break;
#endif
   case JSON_T_TRUE:
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("true\n");
#endif
      s_IsKey = 0;
      if (requestAddressKey)
      {
         requestAddressKey=0;
         doRequestAddress=1;
      }
      break;
   case JSON_T_FALSE:
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("false\n");
#endif
      s_IsKey = 0;
      if (requestAddressKey)
      {
         requestAddressKey=0;
         doRequestAddress=0;
      }
      break;
   case JSON_T_KEY:
      s_IsKey = 1;
#ifdef PRINT_DEBUG
      print_indention();
      printf("key = '%s', value = ", value->vu.str.value);
#endif
      if (!strcmp("wifi_towers",value->vu.str.value)) // wifi_towers-key
      {
         if (wifiCnt==-1) wifiCnt=0; // start counting
      }
      else if (!strcmp("mac_address",value->vu.str.value)) // wifi_towers-key
       macAddressKey=1;
      else if (!strcmp("signal_strength",value->vu.str.value)) // wifi_towers-key
       signalStrengthKey=1;
      else if (!strcmp("request_address",value->vu.str.value)) // wifi_towers-key
       requestAddressKey=1;
      break;   
   case JSON_T_STRING:
#ifdef PRINT_DEBUG
      if (!s_IsKey) print_indention();
      printf("string: '%s'\n", value->vu.str.value);
#endif
      s_IsKey = 0;
      if (macAddressKey)
      {
         if (wifiCnt>=0)
         {
            long long mac;
            int       i;
            
            if (strlen(value->vu.str.value)<17) // no delimiter between digits
            {
               mac=strtoll(value->vu.str.value,(char **)NULL,16);
               for (i=0; i<6; i++)
               {
                  request.bssids[wifiCnt][i]=((mac & 0xFF0000000000LL)>>40) & 0xFF;
                  mac=mac<<8;
               }
            }
            else
            {
               request.bssids[wifiCnt][0]=strtol(value->vu.str.value,(char**)NULL,16);
               request.bssids[wifiCnt][1]=strtol(value->vu.str.value+3,(char**)NULL,16);
               request.bssids[wifiCnt][2]=strtol(value->vu.str.value+6,(char**)NULL,16);
               request.bssids[wifiCnt][3]=strtol(value->vu.str.value+9,(char**)NULL,16);
               request.bssids[wifiCnt][4]=strtol(value->vu.str.value+12,(char**)NULL,16);
               request.bssids[wifiCnt][5]=strtol(value->vu.str.value+15,(char**)NULL,16);
            }
         }
         macAddressKey=0;
      }
      break;
   default:
      break;
   }
   return 1;
}



int main(int argc, char* argv[]) 
{
   int                        next_char,inLength,ret;
   FILE                      *input,*log;
   double                     lat,lon;
   char                       quality;
   short                      ccode;
   JSON_config                config;
   const char                *p;
   struct JSON_parser_struct *jc = NULL;
   char                       retStr[2500],addrStr[1000];
   unsigned long int          remoteIP=0;

   memset(&request,0,sizeof(struct wloc_req));
   srand(time(NULL));
   log=fopen("/tmp/wcgi.log","a");
   p=getenv("CONTENT_LENGTH" );
   if (!p)
   {
      if (log) fprintf(log,"Warning: CONTENT_LENGTH not found!\n");
      inLength=10000; //should be enough?
   }
   else
   {
      inLength=atoi(p);
      fprintf(log,"CONTENT_LENGTH: %d\n",inLength);
      if (inLength<10)
      {
         if (log)
         {
            fprintf(log,"Error: CONTENT_LENGTH too small (%d)!\n",inLength);
            fclose(log);
         }
         return 0;
      } 
   }
    
   p=getenv("REMOTE_ADDR");
   if (p)
   {
      struct hostent    *host;
      struct sockaddr_in addr;

      host = gethostbyname(p);
      if (host)
      {
         addr.sin_addr = *(struct in_addr*) host->h_addr;
         remoteIP=addr.sin_addr.s_addr;
      }
      if (log) fprintf(log,"Remote-IP: %s - %lu\n",p,(unsigned long)ntohl(remoteIP));
   }

   if (log)
   {
      p=getenv("REQUEST_METHOD");
      if (p)
      {
         fprintf(log,"Request-Method: %s\n",p);
         if (strcmp(p,"GET")==0)
         {
            p=getenv("QUERY_STRING");
            if (p)
            {
               fprintf(log,"Query-String: %s\n",p);
            }
         }
      }
   }
   
   init_JSON_config(&config);
    
   config.depth                  = 19;
   config.callback               = &print;
   config.allow_comments         = 1;
   config.handle_floats_manually = 0;
 
   jc = new_JSON_parser(&config);
    
   input =stdin;
   printf("Content-type: text/html\r\n\r\n");
   while ((input) && (!exitLoop) && (inLength>0))
   {
      next_char = fgetc(input);
      if (next_char==EOF) break;
      if (log) fprintf(log,"%c",next_char);
      inLength--;
      if (next_char <= 0) break;
      if (log) fprintf(log,"%c",(char)next_char);
      if (!JSON_parser_char(jc, next_char)) 
      {
         if (log) fprintf(log,"\n");
         fprintf(stderr, "JSON_parser_char: syntax error in stream");
         break;
      }
   }
   if (log) fprintf(log," - %d\n",inLength);
   if (!JSON_parser_done(jc)) fprintf(stderr, "JSON_parser_end: syntax error\n");
    
   delete_JSON_parser(jc);

   if (log) fprintf(log,"\nWiFi-Towers: %d\n",wifiCnt);    
   request.cgiIP=remoteIP; // is already network byteorder;
   ret=get_position(&request,&lat,&lon,&quality,&ccode);
   if (ret==WLOC_OK)
   {
      int accuracy;
      char country[3];
	 
      if (quality>0) accuracy=(int)(1.20*(101.0-quality));
      else accuracy=22500;
      if ((doRequestAddress) && (wloc_get_country_from_code(ccode,country)==WLOC_OK)) sprintf(addrStr,",\"address\":{\"country_code\":\"%s\"}",country);
      else strcpy(addrStr,"");
      snprintf(retStr,2000,"{\"location\":{\"latitude\":%f,\"longitude\":%f%s,\"accuracy\":%d},\"access_token\":\"2:%d%d:%d%d\"}\r\n\r\n",
               lat,lon,addrStr,accuracy,rand(),rand(),rand(),rand());
   }
   else
   {
      snprintf(retStr,2000,"{\"access_token\":\"2:%d%d:%d%d\"}\r\n\r\n",rand(),rand(),rand(),rand());
   }
   if (log)
   {
      fprintf(log,"RET: %d - %s (%lu)\n\n",ret,retStr,request.cgiIP);
      fclose(log);
   }
   printf("%s",retStr);
   return 0;
}


