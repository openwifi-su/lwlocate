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
	int    ret;
	double lat,lon;
	char   quality;
	short  ccode;
   char   country[3];

#ifdef ENV_WINDOWS
   WSADATA   wsaData;

   WSAStartup((MAKEWORD(1, 1)), &wsaData);
#endif

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
   
  	
#ifdef ENV_WINDOWS
   WSACleanup();
#endif
   return 0;
}

