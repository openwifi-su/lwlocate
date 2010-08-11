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
#ifndef ENV_WINDOWS
 #include <arpa/inet.h>
#endif

#include "libwlocate.h"
#include "connect.h"
#include "wlan.h"



int get_position(struct wloc_req *request,double *lat,double *lon,char *quality,short *ccode)
{
   struct wloc_res result;
   int             sock=0,ret;
   unsigned int    uval;

   request->version=1;
   request->length=sizeof(struct wloc_req);
   sock=tcp_connect_to("62.112.159.250",10443);
   if (sock<=0) return WLOC_CONNECTION_ERROR;
   tcp_set_blocking(sock,0); // set to non-blocking, we do not want to waid endless for a dead connection
   ret=tcp_send(sock,(char*)request,sizeof(struct wloc_req),3000);
   if (ret<(int)sizeof(struct wloc_req))
   {
      tcp_closesocket(sock);
      return WLOC_CONNECTION_ERROR;
   }
   ret=tcp_recv(sock,(char*)&result,sizeof(struct wloc_res),NULL,20000); // longer receiving timeout, evaluation of the position may need some seconds in worst case
   if (ret<(int)sizeof(struct wloc_res))
   {
      tcp_closesocket(sock);
      return WLOC_CONNECTION_ERROR;
   }
   tcp_closesocket(sock);
   if (result.result!=WLOC_RESULT_OK) return WLOC_LOCATION_ERROR;

   uval=ntohl(result.lat);
   if (uval & 0x80000000) *lat=((~uval)+1)/-10000000.0;
   else *lat=uval/10000000.0;

   uval=ntohl(result.lon);
   if (uval & 0x80000000) *lon=((~uval)+1)/-10000000.0;
   else *lon=uval/10000000.0;

   *quality=result.quality;
   *ccode=ntohs(result.ccode);
   return WLOC_OK;
}



/** please refer to libwlocate.h for a description of this function! */
WLOC_EXT_API int wloc_get_location(double *lat,double *lon,char *quality,short *ccode)
{
#ifdef ENV_LINUX
   int sock,i,j;
#endif
   struct wloc_req request;
   int             ret=0;

   memset((char*)&request,0,sizeof(struct wloc_req));
#ifdef ENV_LINUX
   // for Linux we have some special handling because only root has full access to the WLAN-hardware:
   // there a wlocd-daemon may run with root privileges, so we try to connect to it and receive the
   // BSSID data from there. Only in case this fails the way via iwtools is used 
   sock=tcp_connect_to("127.0.0.1",10444);
   if (sock>0)
   {
   	ret=tcp_recv(sock,(char*)&request,sizeof(struct wloc_req),NULL,7500);
   	tcp_closesocket(sock);
   	if (ret==sizeof(struct wloc_req))
   	{
   		ret=0;
   		for (i=0; i<WLOC_MAX_NETWORKS; i++)
   		{
   			if (request.bssids[i][0]+request.bssids[i][1]+request.bssids[i][2]+
   			    request.bssids[i][3]+request.bssids[i][4]+request.bssids[i][5]>0) ret++;
   		}
   	}
   }
#else
   ret=0;   
#endif
   if (ret==0)
   {
      if (wloc_get_wlan_data(&request)<=0)
      {
         wloc_get_wlan_data(&request); // try two times in case the device was currently used
         // in case of no success request localisation without WLAN data
      }
   }
//   for (i=0; i<WLOC_MAX_NETWORKS; i++)
//    printf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X Signal: %d\n",request.bssids[i][0] & 0xFF,request.bssids[i][1] & 0xFF,request.bssids[i][2] & 0xFF,
//                                                               request.bssids[i][3] & 0xFF,request.bssids[i][4] & 0xFF,request.bssids[i][5] & 0xFF,request.signal[i]);
   return get_position(&request,lat,lon,quality,ccode);
}



/** please refer to libwlocate.h for a description of this function! */
WLOC_EXT_API int wloc_get_country_from_code(short ccode,char *country)
{
   switch (ccode)
   {
      case 1:
         strncpy(country,"DE",2);
         return WLOC_OK;
      case 2:
         strncpy(country,"AT",2);
         return WLOC_OK;
      case 3:
         strncpy(country,"CH",2);
         return WLOC_OK;
      case 4:
         strncpy(country,"NL",2);
         return WLOC_OK;
      case 5:
         strncpy(country,"BE",2);
         return WLOC_OK;
      case 6:
         strncpy(country,"LU",2);
         return WLOC_OK;
      case 7:
         strncpy(country,"NO",2);
         return WLOC_OK;
      case 8:
         strncpy(country,"SE",2);
         return WLOC_OK;
      case 9:
         strncpy(country,"DK",2);
         return WLOC_OK;
      case 10:
         strncpy(country,"AF",2);
         return WLOC_OK;
      case 12:
         strncpy(country,"AL",2);
         return WLOC_OK;
      case 13:
         strncpy(country,"DZ",2);
         return WLOC_OK;
      case 17:
         strncpy(country,"AN",2);
         return WLOC_OK;
      case 19:
         strncpy(country,"AR",2);
         return WLOC_OK;
      case 20:
         strncpy(country,"AM",2);
         return WLOC_OK;
      case 21:
         strncpy(country,"AU",2);
         return WLOC_OK;
      case 23:
         strncpy(country,"BS",2);
         return WLOC_OK;
      case 24:
         strncpy(country,"BH",2);
         return WLOC_OK;
      case 25:
         strncpy(country,"BD",2);
         return WLOC_OK;
      case 26:
         strncpy(country,"BB",2);
         return WLOC_OK;
      case 27:
         strncpy(country,"BY",2);
         return WLOC_OK;
      case 28:
         strncpy(country,"BZ",2);
         return WLOC_OK;
      case 29:
         strncpy(country,"BJ",2);
         return WLOC_OK;
      case 30:
         strncpy(country,"BM",2);
         return WLOC_OK;
      case 32:
         strncpy(country,"BO",2);
         return WLOC_OK;
      case 33:
         strncpy(country,"BA",2);
         return WLOC_OK;
      case 36:
         strncpy(country,"BR",2);
         return WLOC_OK;
      case 37:
         strncpy(country,"BN",2);
         return WLOC_OK;
      case 38:
         strncpy(country,"BG",2);
         return WLOC_OK;
      case 43:
         strncpy(country,"CA",2);
         return WLOC_OK;
      case 44:
         strncpy(country,"CV",2);
         return WLOC_OK;
      case 47:
         strncpy(country,"CL",2);
         return WLOC_OK;
      case 48:
         strncpy(country,"CN",2);
         return WLOC_OK;
      case 49:
         strncpy(country,"CO",2);
         return WLOC_OK;
      case 52:
         strncpy(country,"CR",2);
         return WLOC_OK;
      case 53:
         strncpy(country,"HR",2);
         return WLOC_OK;
      case 55:
         strncpy(country,"CY",2);
         return WLOC_OK;
      case 56:
         strncpy(country,"CZ",2);
         return WLOC_OK;
      case 59:
         strncpy(country,"DO",2);
         return WLOC_OK;
      case 60:
         strncpy(country,"EC",2);
         return WLOC_OK;
      case 61:
         strncpy(country,"EG",2);
         return WLOC_OK;
      case 66:
         strncpy(country,"ET",2);
         return WLOC_OK;
      case 68:
         strncpy(country,"FI",2);
         return WLOC_OK;
      case 69:
         strncpy(country,"FR",2);
         return WLOC_OK;
      case 73:
         strncpy(country,"GH",2);
         return WLOC_OK;
      case 75:
         strncpy(country,"GR",2);
         return WLOC_OK;
      case 76:
         strncpy(country,"GL",2);
         return WLOC_OK;
      case 78:
         strncpy(country,"GU",2);
         return WLOC_OK;
      case 79:
         strncpy(country,"GT",2);
         return WLOC_OK;
      case 82:
         strncpy(country,"HT",2);
         return WLOC_OK;
      case 83:
         strncpy(country,"HN",2);
         return WLOC_OK;
      case 84:
         strncpy(country,"HK",2);
         return WLOC_OK;
      case 85:
         strncpy(country,"HU",2);
         return WLOC_OK;
      case 86:
         strncpy(country,"IS",2);
         return WLOC_OK;
      case 87:
         strncpy(country,"IN",2);
         return WLOC_OK;
      case 88:
         strncpy(country,"ID",2);
         return WLOC_OK;
      case 89:
         strncpy(country,"IR",2);
         return WLOC_OK;
      case 90:
         strncpy(country,"IQ",2);
         return WLOC_OK;
      case 91:
         strncpy(country,"IE",2);
         return WLOC_OK;
      case 93:
         strncpy(country,"IT",2);
         return WLOC_OK;
      case 94:
         strncpy(country,"JM",2);
         return WLOC_OK;
      case 95:
         strncpy(country,"JP",2);
         return WLOC_OK;
      case 97:
         strncpy(country,"JO",2);
         return WLOC_OK;
      case 98:
         strncpy(country,"KZ",2);
         return WLOC_OK;
      case 99:
         strncpy(country,"KE",2);
         return WLOC_OK;
      case 102:
         strncpy(country,"KR",2);
         return WLOC_OK;
      case 103:
         strncpy(country,"KW",2);
         return WLOC_OK;
      case 104:
         strncpy(country,"KG",2);
         return WLOC_OK;
      case 105:
         strncpy(country,"LA",2);
         return WLOC_OK;
      case 106:
         strncpy(country,"LV",2);
         return WLOC_OK;
      case 107:
         strncpy(country,"LB",2);
         return WLOC_OK;
      case 108:
         strncpy(country,"LS",2);
         return WLOC_OK;
      case 111:
         strncpy(country,"LT",2);
         return WLOC_OK;
      case 115:
         strncpy(country,"MY",2);
         return WLOC_OK;
      case 116:
         strncpy(country,"MV",2);
         return WLOC_OK;
      case 118:
         strncpy(country,"MT",2);
         return WLOC_OK;
      case 119:
         strncpy(country,"MQ",2);
         return WLOC_OK;
      case 121:
         strncpy(country,"MU",2);
         return WLOC_OK;
      case 123:
         strncpy(country,"MX",2);
         return WLOC_OK;
      case 124:
         strncpy(country,"MC",2);
         return WLOC_OK;
      case 125:
         strncpy(country,"MN",2);
         return WLOC_OK;
      case 126:
         strncpy(country,"MA",2);
         return WLOC_OK;
      case 127:
         strncpy(country,"MZ",2);
         return WLOC_OK;
      case 131:
         strncpy(country,"NZ",2);
         return WLOC_OK;
      case 133:
         strncpy(country,"NI",2);
         return WLOC_OK;
      case 135:
         strncpy(country,"NG",2);
         return WLOC_OK;
      case 137:
         strncpy(country,"OM",2);
         return WLOC_OK;
      case 138:
         strncpy(country,"PK",2);
         return WLOC_OK;
      case 141:
         strncpy(country,"PA",2);
         return WLOC_OK;
      case 142:
         strncpy(country,"PY",2);
         return WLOC_OK;
      case 144:
         strncpy(country,"PE",2);
         return WLOC_OK;
      case 145:
         strncpy(country,"PH",2);
         return WLOC_OK;
      case 147:
         strncpy(country,"PL",2);
         return WLOC_OK;
      case 148:
         strncpy(country,"PT",2);
         return WLOC_OK;
      case 149:
         strncpy(country,"PR",2);
         return WLOC_OK;
      case 150:
         strncpy(country,"QA",2);
         return WLOC_OK;
      case 151:
         strncpy(country,"RO",2);
         return WLOC_OK;
      case 152:
         strncpy(country,"RU",2);
         return WLOC_OK;
      case 155:
         strncpy(country,"SM",2);
         return WLOC_OK;
      case 157:
         strncpy(country,"SA",2);
         return WLOC_OK;
      case 158:
         strncpy(country,"SN",2);
         return WLOC_OK;
      case 161:
         strncpy(country,"SG",2);
         return WLOC_OK;
      case 162:
         strncpy(country,"SK",2);
         return WLOC_OK;
      case 163:
         strncpy(country,"SI",2);
         return WLOC_OK;
      case 166:
         strncpy(country,"ZA",2);
         return WLOC_OK;
      case 167:
         strncpy(country,"ES",2);
         return WLOC_OK;
      case 168:
         strncpy(country,"LK",2);
         return WLOC_OK;
      case 169:
         strncpy(country,"SD",2);
         return WLOC_OK;
      case 170:
         strncpy(country,"SR",2);
         return WLOC_OK;
      case 172:
         strncpy(country,"SY",2);
         return WLOC_OK;
      case 173:
         strncpy(country,"TW",2);
         return WLOC_OK;
      case 174:
         strncpy(country,"TJ",2);
         return WLOC_OK;
      case 175:
         strncpy(country,"TZ",2);
         return WLOC_OK;
      case 176:
         strncpy(country,"TH",2);
         return WLOC_OK;
      case 179:
         strncpy(country,"TT",2);
         return WLOC_OK;
      case 180:
         strncpy(country,"TN",2);
         return WLOC_OK;
      case 181:
         strncpy(country,"TR",2);
         return WLOC_OK;
      case 182:
         strncpy(country,"TM",2);
         return WLOC_OK;
      case 185:
         strncpy(country,"UA",2);
         return WLOC_OK;
      case 186:
         strncpy(country,"AE",2);
         return WLOC_OK;
      case 187:
         strncpy(country,"UK",2);
         return WLOC_OK;
      case 188:
         strncpy(country,"US",2);
         return WLOC_OK;
      case 189:
         strncpy(country,"UY",2);
         return WLOC_OK;
      case 191:
         strncpy(country,"VE",2);
         return WLOC_OK;
      case 192:
         strncpy(country,"VN",2);
         return WLOC_OK;
      case 195:
         strncpy(country,"ZM",2);
         return WLOC_OK;
      case 196:
         strncpy(country,"ZW",2);
         return WLOC_OK;
      default:
         return WLOC_ERROR;
   }
}


/** please refer to libwlocate.h for a description of this function! */
WLOC_EXT_API char* wloc_get_countryname_from_code(short ccode)
{
	#define MAX_COUNTRY_NUM 196

   static char countrynames[MAX_COUNTRY_NUM][42]={"Deutschland","�sterreich","Schweiz","Nederland","Belgi�","Luxemburg","Norge","Sverige",
   	                                          "Danmark",
	                                          "Afghanistan","�land Islands","Albania","Algeria","American Samoa","Andorra","Angola",
                                                  "Anguilla","Antigua And Barbuda","Argentina","Armenia","Australia","Azerbaijan",
                                                  "Bahamas","Bahrain","Bangladesh","Barbados","Belarus","Belize","Benin","Bermuda","Bhutan",
                                                  "Bolivia","Bosnia And Herzegovina","Botswana","Bouvet Island","Brazil","Brunei Darussalam",
                                                  "Bulgaria","Burkina Faso","Burundi",
                                                  "Cambodia","Cameroon","Canada","Cape Verde","Central African Republic","Chad","Chile",
                                                  "China","Colombia","Comoros","Congo","Costa Rica","Croatia","Cuba","Cyprus","Czech Republic",
                                                  "Djibouti","Dominica","Dominican Republic",
	                                          "Ecuador","Egypt","El Salvador","Equatorial Guinea","Eritrea","Estonia","Ethiopia",
       	                                          "Fiji","Finland","France",
             	                                  "Gabon","Gambia","Georgia","Ghana","Gibraltar","Greece","Greenland","Grenada","Guam",
                   	                          "Guatemala","Guernsey","Guyana",
                                                  "Haiti","Honduras","Hong Kong","Hungary",
                                                  "Iceland","India","Indonesia","Iran","Iraq","Ireland","Israel","Italy",
                                                  "Jamaica","Japan","Jersey","Jordan",
                                                  "Kazakhstan","Kenya","Kiribati","Korea, Democratic People's Republic Of","Korea, Republic Of",
	                                          "Kuwait","Kyrgyzstan",
       	                                          "Lao People's Democratic Republic","Latvia","Lebanon","Lesotho","Liberia",
             	                                  "Libyan Arab Jamahiriya","Lithuania",
                   	                          "Macao","Madagascar","Malawi","Malaysia","Maldives","Mali","Malta","Martinique","Mauritania",
                                                  "Mauritius","Mayotte","Mexico","Monaco","Mongolia","Morocco","Mozambique",
                                                  "Namibia","Nauru","Nepal","New Caledonia","New Zealand","Nicaragua","Niger","Nigeria","Niue",
                                                  "Oman",
                                                  "Pakistan","Palau","Palestine","Panama","Papua New Guinea","Paraguay","Peru","Philippines",
	                                          "Pitcairn","Poland","Portugal","Puerto Rico",
       	                                          "Qatar",
                                                  "Romania","Russian Federation","Rwanda",
                                                  "Samoa","San Marino","Sao Tome And Principe","Saudi Arabia","Senegal","Seychelles",
                                                  "Sierra Leone","Singapore","Slovakia","Slovenia","Solomon Islands","Somalia","South Africa",
                                                  "Spain","Sri Lanka","Sudan","Suriname","Swaziland","Syrian Arab Republic",
                                                  "Taiwan","Tajikistan","Tanzania","Thailand","Togo","Tonga","Trinidad And Tobago","Tunisia",
                                                  "Turkey","Turkmenistan","Tuvalu",
	                                          "Uganda","Ukraine","United Arab Emirates","United Kingdom","United States of America",
       	                                          "Uruguay","Uzbekistan",
             	                                  "Venezuela","Viet Nam",
                   	                          "Western Sahara","Yemen","Zambia","Zimbabwe"};
   if ((ccode<1) || (ccode>=MAX_COUNTRY_NUM)) return NULL;
   return countrynames[ccode-1];
}

