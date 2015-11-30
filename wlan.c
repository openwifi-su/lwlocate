/**
 * libwlocate - WLAN-based location service
 * Copyright (C) 2010-2012 Oxygenic/VWP virtual_worlds(at)gmx.de
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

#ifdef ENV_WINDOWS
#include <windows.h>
#include <objbase.h>
#include <wtypes.h>

#if (_MSC_VER>1400)
 #include <wlanapi.h>
 #pragma comment(lib, "wlanapi.lib") 
#endif
#include "wlanapi_cust.h"
#endif

#include "libwlocate.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef ENV_WINDOWS
/**
 * Works with Windows Vista and newer
 */
static int WinMethod1(struct wloc_req *request)
{
#if (_MSC_VER<=1400)
   static HINSTANCE                 wlan_library=NULL;
#endif
          HANDLE                    hClient = NULL;
          DWORD                     dwCurVersion = 0;
          DWORD                     dwResult = 0;
          int                       iRet = 0,i,j,cnt;    
          WCHAR                     GuidString[40] = {0};
          PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
          PWLAN_INTERFACE_INFO      pIfInfo = NULL;
          PWLAN_BSS_LIST            pBssList=NULL;
		  PWLAN_BSS_ENTRY           pBssEntry=NULL;

#if (_MSC_VER<=1400)
   if (!wlan_library)
   {
      wlan_library = LoadLibrary("wlanapi");
      if (!wlan_library) return 0;
      WlanOpenHandle = (WlanOpenHandleFunction)GetProcAddress(wlan_library, "WlanOpenHandle");
      WlanEnumInterfaces = (WlanEnumInterfacesFunction)GetProcAddress(wlan_library, "WlanEnumInterfaces");
      WlanGetNetworkBssList = (WlanGetNetworkBssListFunction)GetProcAddress(wlan_library, "WlanGetNetworkBssList");
      WlanCloseHandle = (WlanCloseHandleFunction)GetProcAddress(wlan_library, "WlanCloseHandle");
      WlanFreeMemory = (WlanFreeMemoryFunction)GetProcAddress(wlan_library, "WlanFreeMemory");
      if ((!WlanOpenHandle) || (!WlanEnumInterfaces) || (!WlanGetNetworkBssList) ||
          (!WlanCloseHandle) || (!WlanFreeMemory))
      {
         FreeLibrary(wlan_library);
         wlan_library=NULL;
         return 0;
      }
   }
#endif
   dwResult = WlanOpenHandle(1, NULL, &dwCurVersion, &hClient); 
   if (dwResult != ERROR_SUCCESS) return 0;
    
   dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList); 
   if (dwResult != ERROR_SUCCESS)  
   {
      WlanCloseHandle(hClient,NULL);
      return 0;
   }
   cnt=-1;
   for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) 
   {
      pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
      dwResult=WlanGetNetworkBssList(hClient,&pIfInfo->InterfaceGuid,NULL,dot11_BSS_type_any,FALSE,NULL,&pBssList);
      if (dwResult!=ERROR_SUCCESS) continue;
      for (j=0; j<(int)pBssList->dwNumberOfItems; j++)
      {
		  char *c;

         cnt++;
		 pBssEntry=&pBssList->wlanBssEntries[j];
		 c=(char*)&pBssList->wlanBssEntries[j];
         memcpy(request->bssids[cnt],pBssEntry->dot11Bssid,6);
//         request->signal[cnt]=(char)pBssEntry->uLinkQuality;
         if (cnt>=WLOC_MAX_NETWORKS) break;
      }
      if (pBssList != NULL) WlanFreeMemory(pBssList); // ???
      if (cnt>=WLOC_MAX_NETWORKS) break;
   }
   if (pIfList != NULL) WlanFreeMemory(pIfList);
   WlanCloseHandle(hClient,NULL);
   return cnt+1;
}


/**
 * Works with Windows XP >=SP2 and newer, outdated with Windows Vista and newer
 */
static int WinMethod2(struct wloc_req *request)
{
   static HINSTANCE               wzc_library=NULL;
          INTFS_KEY_TABLE         interface_list;
          ADAPTER_INFO            adapter_info;
          INTF_ENTRY              interface_data;
          DWORD                   result,dwOutFlags;
          int                     i,j,length,cnt,data_until_padding;
          PNDIS_802_11_BSSID_LIST pList;
   const  unsigned char          *buffer_end;
          PNDIS_WLAN_BSSID        pBssid;

   if (wzc_library==NULL) 
   {
      wzc_library = LoadLibrary("wzcsapi");
      if (!wzc_library) return 0;
      WZCEnumInterfaces = (WZCEnumInterfacesFunction)GetProcAddress(wzc_library, "WZCEnumInterfaces");
      WZCQueryInterface = (WZCQueryInterfaceFunction)GetProcAddress(wzc_library, "WZCQueryInterface");
#if (_MSC_VER<=1400)
      WZCRefreshInterface = (WZCRefreshInterfaceFunction)GetProcAddress(wzc_library, "WZCRefreshInterface");

      if ((!WZCEnumInterfaces) || (!WZCQueryInterface) || (!WZCRefreshInterface))
#else
      if ((!WZCEnumInterfaces) || (!WZCQueryInterface))
#endif
      {
         FreeLibrary(wzc_library);
         wzc_library=0;
         return 0;
      }
   }

   memset(&interface_list, 0, sizeof(INTFS_KEY_TABLE));
   result = WZCEnumInterfaces(NULL, &interface_list);
   if (result != ERROR_SUCCESS)  return 0;
   cnt=-1;
   for (i = 0; i<(int)interface_list.dwNumIntfs; ++i) 
   {
      memset(&interface_data, 0, sizeof(INTF_ENTRY));
      interface_data.wszGuid = interface_list.pIntfs[i].wszGuid;
      dwOutFlags = 1;
      result = WZCQueryInterface(NULL, INTF_DESCR, &interface_data, &dwOutFlags);
      if (result != ERROR_SUCCESS) 
      {
         LocalFree(interface_list.pIntfs);
         return 0;
      }
      length = wcslen(interface_list.pIntfs[i].wszGuid);
      if (length > 0 && length < ADAPTER_NAME_LENGTH) 
      {
         memset(&adapter_info, 0, sizeof(adapter_info));
         wcscpy(adapter_info.name, interface_list.pIntfs[i].wszGuid);
         length = wcslen(interface_data.wszDescr);
         if (length > 0 && length < ADAPTER_DESCRIPTION_LENGTH) wcscpy(adapter_info.description, interface_data.wszDescr);

         memset(&interface_data, 0, sizeof(INTF_ENTRY));
         interface_data.wszGuid =interface_list.pIntfs[i].wszGuid;

         result = WZCQueryInterface(NULL, INTF_BSSIDLIST | INTF_LIST_SCAN, &interface_data, &dwOutFlags);
         if (result != ERROR_SUCCESS) 
         { 
            LocalFree(interface_list.pIntfs);
            return 0;
         }
         if ((dwOutFlags & INTF_BSSIDLIST) != INTF_BSSIDLIST) 
         {
            printf("WZC: Interface query consistency failure: incorrect flags\n");
            LocalFree(interface_list.pIntfs);
            return 0;
         }
         if (interface_data.rdBSSIDList.dwDataLen == 0 || interface_data.rdBSSIDList.dwDataLen < sizeof(NDIS_802_11_BSSID_LIST)) 
         {
            data_until_padding = (UCHAR*)&interface_data.padding1 - (UCHAR*)&interface_data;

            // this is a hack to support Windows XP SP2 with WLAN Hotfix and SP3
            memmove((UCHAR*)&interface_data + data_until_padding, (UCHAR*)&interface_data + data_until_padding + 8, sizeof(interface_data) - data_until_padding - 8);
            if (interface_data.rdBSSIDList.dwDataLen == 0 || interface_data.rdBSSIDList.dwDataLen < sizeof(NDIS_802_11_BSSID_LIST)) 
            {
               // cleanup
               printf("WZC: Interface query consistency failure: no data or incorrect data length (length: %ld)\n", interface_data.rdBSSIDList.dwDataLen);
               LocalFree(interface_list.pIntfs);
               LocalFree(interface_data.rdBSSIDList.pData);
               return 0;
            }
         }
         pList =(NDIS_802_11_BSSID_LIST*)(interface_data.rdBSSIDList.pData);
         pBssid =(PNDIS_WLAN_BSSID)(&pList->Bssid[0]);
         buffer_end =(unsigned char*)(pBssid) + interface_data.rdBSSIDList.dwDataLen;
         for (j= 0; j<(int)pList->NumberOfItems; j++) 
         {
            cnt++;
            if (pBssid->Length < sizeof(NDIS_WLAN_BSSID) || ((unsigned char*)(pBssid) + pBssid->Length > buffer_end)) 
            {
               // cleanup
               LocalFree(interface_list.pIntfs);
               LocalFree(interface_data.rdBSSIDList.pData);
               printf("WZC: Bssid structure looks odd. Break!\n");
               return cnt;
            }
            memcpy(request->bssids[cnt],pBssid->MacAddress,6);
//            request->signal[cnt]=(char)((100+pBssid->Rssi)*1.6); // is this really the correct calculation for a signal strength in percent?
            pBssid=(PNDIS_WLAN_BSSID)((unsigned char*)(pBssid) + pBssid->Length);
         }
         LocalFree(interface_data.rdBSSIDList.pData);
      }
   }
   LocalFree(interface_list.pIntfs);

   return cnt+1;
}
#endif



#ifdef ENV_LINUX
extern int iw_fill_structure(struct wloc_req *request);
#endif

WLOC_EXT_API int wloc_get_wlan_data(struct wloc_req *request)
{
#ifdef ENV_WINDOWS
   int ret;

   // here we have to try which one of the methods works because there is no stable and standardised API
   // available on Windows that could be used securely
   ret=WinMethod1(request);
   if (ret==0) ret=WinMethod2(request);

   return ret;
#else
 #ifdef ENV_LINUX
   return iw_fill_structure(request);
 #else
  #ifdef ENV_QNX
   #warning WLAN functionality not implemented, library will never use the more exact WLAN positioning!
  #else
   #error Not supported!
  #endif
 #endif
#endif
   return 0; // no networks found
}


