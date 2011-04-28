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



int main(int argc,char *argv[])
{
   int             ret,i,cnt;
   double          lat,lon;
   char            quality;
   short           ccode;
   char            country[3];
   FILE           *FHandle;
   struct wloc_req request;
   unsigned char   empty_bssid[6]={0,0,0,0,0,0};
   unsigned char   empty_signal=0;

#ifdef ENV_WINDOWS
   WSADATA   wsaData;

   WSAStartup((MAKEWORD(1, 1)), &wsaData);
#endif

   if ((argc>1) && (strncmp(argv[1],"-t",2)==0)) // write the WLAN data into a trace file instead of retrieving the position immediately
   {
      FHandle=fopen("libwlocate.trace","ab");
      if (FHandle)
      {
         while (true)
         {
            cnt=wloc_get_wlan_data(&request);
/*request.bssids[0][0]=rand();
request.bssids[0][1]=rand();
request.bssids[0][2]=rand();
request.bssids[0][3]=rand();
request.bssids[0][4]=rand();
request.bssids[0][5]=rand();
request.signal[0]=90;
cnt=3;*/
            if (cnt>0)
            {
               for (i=0; i<cnt; i++)
               {
                  fwrite(&request.bssids[i],1,sizeof(request.bssids[i]),FHandle);
                  fwrite(&request.signal[i],1,1,FHandle);
               }
               for (i=cnt; i<WLOC_MAX_NETWORKS; i++)
               {
                  fwrite(&empty_bssid,1,sizeof(request.bssids[i]),FHandle);
                  fwrite(&empty_signal,1,1,FHandle);
               }
               fflush(FHandle);
            }
            sleep(1);
         }
         fclose(FHandle);
      }
      else printf("Error: could not open/append trace file libwlocate.trace!\n");
   }
   else
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
#ifdef ENV_WINDOWS
   WSACleanup();
#endif
   return 0;
}

