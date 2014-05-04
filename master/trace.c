/**
 * test for libwlocate - WLAN-based location service
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

#include "libwlocate.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>

#ifdef ENV_LINUX
#include <unistd.h>
#endif

#ifdef ENV_WINDOWS
#include <windows.h>
#endif



char notEqual(struct wloc_req *data1,struct wloc_req *data2,int num)
{
   int i,j;
   
   for (i=0; i<num; i++)
   {
      for (j=0; j<6; j++)
      {
         if (data1->bssids[i][j]!=data2->bssids[i][j]) return 1;
      }
   }
   return 0;
}



int main(int argc,char *argv[])
{
   int             ret,i,cnt=0,prevCnt=0;
   double          lat,lon;
   char            quality;
   short           ccode;
   char            country[3];
   FILE           *FHandle;
   struct wloc_req request,prevRequest;
   unsigned char   empty_bssid[6]={0,0,0,0,0,0};
   unsigned char   empty_signal=0;

#ifdef ENV_WINDOWS
   WSADATA   wsaData;

   WSAStartup((MAKEWORD(1, 1)), &wsaData);
#endif

   if ((argc>1) && (strncmp(argv[1],"-h",2)==0)) // test WLAN geolocation instead of writing the WLAN data into a trace file
   {
      printf("lwtrace\n\tscan available WLAN networks, and write them into a file libwlocate.trace for later geolocation\n");
      printf("lwtrace -t\n\ttest geolocation functionality and evaluate the current position out of available WLAN data immediately\n");
   }
   else if ((argc>1) && (strncmp(argv[1],"-t",2)==0)) // test WLAN geolocation instead of writing the WLAN data into a trace file
   {
      ret=wloc_get_location(&lat,&lon,&quality,&ccode); // call the library function to get the position...
      //...check the return value afterwards...
      if (ret==WLOC_CONNECTION_ERROR) printf("Error: could not communicate with server!\n");
      else if (ret==WLOC_LOCATION_ERROR) printf("Error: could not calculate your location, the given WLAN networks are unknown!\n");
      //...and print the position only in case the call was successful
      else if (ret==WLOC_OK)
      {
         printf("Your location: %f (lat) %f (lon)\nQuality: %d %%\n",lat,lon,quality);
         country[2]=0;
         if (wloc_get_country_from_code(ccode,country)==WLOC_OK) printf("Country: %d - %s\n",ccode,country);
         else printf("Country: unknown\n");
      }
      else printf("Error: failed due to an unknown error!\n");
   }
   else
   {
      FHandle=fopen("libwlocate.trace","ab");
      if (FHandle)
      {
         memset(&prevRequest,0,sizeof(struct wloc_req));
         while (true)
         {
            if (cnt>0)
            {
               if ((cnt==prevCnt) && (!notEqual(&request,&prevRequest,cnt)))
               {
                  for (i=0; i<cnt; i++)
                  {
                     fwrite(&request.bssids[i],1,sizeof(request.bssids[i]),FHandle);
                     fwrite(&empty_signal,1,1,FHandle);
                  }
                  for (i=cnt; i<WLOC_MAX_NETWORKS; i++)
                  {
                     fwrite(&empty_bssid,1,sizeof(request.bssids[i]),FHandle);
                     fwrite(&empty_signal,1,1,FHandle);
                  }
                  fflush(FHandle);               
                  prevCnt=cnt;
                  prevRequest=request;
               }
            }
#ifdef ENV_WINDOWS
            Sleep(1000);
#else
            sleep(1);
#endif
         }
         fclose(FHandle);
      }
      else printf("Error: could not open/append trace file libwlocate.trace!\n");
   }
#ifdef ENV_WINDOWS
   WSACleanup();
#endif
   return 0;
}

