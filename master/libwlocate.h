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

#ifndef LIBWLOCATE_H
#define LIBWLOCATE_H

#ifndef WLOC_EXT_API
 #ifdef ENV_LINUX
  #define WLOC_EXT_API extern
 #endif

 #ifdef ENV_QNX
  #define WLOC_EXT_API extern
 #endif

 #ifdef ENV_WINDOWS
  #define WLOC_EXT_API __declspec(dllexport)
 #endif
#endif

#ifndef __cplusplus
 typedef unsigned char bool;
 #define false 0
 #define true  1
#endif

#ifdef ENV_WINDOWS
 #include <windows.h>
#endif

#ifdef ENV_QNX
 #include <stdint.h>
#endif

#define WLOC_MAX_NETWORKS 16

#pragma pack(1) // 1 byte alignment, calculation speed doesn't matters but data transfer sizes

// internally used communication structures and defines ======================================================================
struct wloc_req
{
	unsigned char version,length;
	char          bssids[WLOC_MAX_NETWORKS][6];
   char          signal[WLOC_MAX_NETWORKS];
   unsigned long cgiIP;
};

#define WLOC_RESULT_OK    1  // a position could be calculated
#define WLOC_RESULT_ERROR 2  // the location could not be retrieved
#define WLOC_RESULT_IERROR 3 // an internal error occured, no data are available

struct wloc_res
{
	char           version,length;
	char           result,iresult,quality;
	char           cres6,cres7,cres8;    // reserved variables
	int            lat,lon;
	short          ccode;
	unsigned short wres34,wres56,wres78; // reserved variables
};
// end of internally used communication structures and defines ================================================================



// public defines and function definitions ====================================================================================
#define WLOC_OK               0 // result is OK, location could be retrieved
#define WLOC_CONNECTION_ERROR 1 // could not send data to/receive data from server
#define WLOC_SERVER_ERROR     2// could not connect to server to get position data
#define WLOC_LOCATION_ERROR   3 // could not retrieve location, detected WLAN networks are unknown
#define WLOC_ERROR          100 // some other error

#ifdef __cplusplus
extern "C" 
{
#endif
  /**
   * This function retrieves the current geographic position of a system, the returned
   * position values can be used directly within maps like OpenStreetMap or Google Earth
   * @param[out] lat the latitude of the geographic position
   * @param[out] lon the longitude of the geographic position
   * @param[out] quality the percentual quality of the returned position, the given result
   *             is as more exact as closer the quality value is to 100%, as smaller this
   *             value is as bigger is the possible maximum deviation between returned
   *             and the real position
   * @return only in case the returned value is equal WLOC_OK the values given back via the
   *             functions parameters can be used; in case an error occurred an error code
   *             WLOC_xxx is returned and the position and quality values given back are
   *             undefined and don't have to be used
   */
   WLOC_EXT_API int wloc_get_location(double *lat,double *lon,char *quality,short *ccode);
   
   /**
    * This function is used internally on step before the geolocation is calculated. It
    * checks which WLAN networks are accessible at the moment with wich signal strength and
    * fills the request structure wloc_req with these data. So this function can be called
    * in order to check the number of available networks without performing any geolocation.
    * @param[out] request a structure of type wloc_req that is filled with the WLAN data;
    *             BSSID entries of this structure that are set to 00-00-00-00-00-00 are
    *             unused and do not contain valid WLAN information
    * @return the retruned value is equal to the number of WLAN networks that have been found,
    *             only in case it is greater than 0 the value given back via the functions
    *             parameter can be used, elsewhere the structures contents are undefined
    */
   WLOC_EXT_API int wloc_get_wlan_data(struct wloc_req *request);
   
   /**
    * Using this function the numeric country code that is returned by the wloc_get_location
    * function can be decoded to a two-character identifier.
    * @param[in] ccode a country code value that has to be bigger than 0
    * @param[out] country this parameter has to point to a char array with a length of at least
    *            two, here the country identifier is stored that belongs to the given code;
    *            in case the function returns an error the contents of this variable are
    *            undefined
    * @return WLOC_OK in case a valid and known country code was given, WLOC_ERROR in case the
    *            country code is unknown    
    */
   WLOC_EXT_API int wloc_get_country_from_code(short ccode,char *country);

   /**
    * Using this function the numeric country code that is returned by the wloc_get_location
    * function can be decoded to a full-length country name. The name is returned as null-
    * terminated string that does not need to be copied, it is allocated by the library and will
    * be released by it.
    * @param[in] ccode a country code value that has to be bigger than 0
    * @return NULL in case the country code could not be decoded or the country name in case of
    *            success    
    */
   WLOC_EXT_API char* wloc_get_countryname_from_code(short ccode);

   WLOC_EXT_API int get_position(struct wloc_req *request,double *lat,double *lon,char *quality,short *ccode);
#ifdef __cplusplus
}
#endif

#endif

