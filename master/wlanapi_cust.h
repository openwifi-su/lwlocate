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
 * 
 * This code bases on the header files out of the WLAN-API from Moritz
 * Mertinkat. 
 */

#ifndef WLANAPI_H
#define WLANAPI_H


typedef enum _WLAN_INTERFACE_STATE {
    wlan_interface_state_not_ready = 0,
    wlan_interface_state_connected,
    wlan_interface_state_ad_hoc_network_formed,
    wlan_interface_state_disconnecting,
    wlan_interface_state_disconnected,
    wlan_interface_state_associating,
    wlan_interface_state_discovering,
    wlan_interface_state_authenticating
} WLAN_INTERFACE_STATE, *PWLAN_INTERFACE_STATE;

#define WLAN_MAX_NAME_LENGTH 256

typedef struct _WLAN_INTERFACE_INFO {
    GUID InterfaceGuid;
    WCHAR strInterfaceDescription[WLAN_MAX_NAME_LENGTH];
    WLAN_INTERFACE_STATE isState;
} WLAN_INTERFACE_INFO, *PWLAN_INTERFACE_INFO;



typedef struct _WLAN_INTERFACE_INFO_LIST {
    DWORD dwNumberOfItems;
    DWORD dwIndex;

    WLAN_INTERFACE_INFO InterfaceInfo[1];

} WLAN_INTERFACE_INFO_LIST, *PWLAN_INTERFACE_INFO_LIST;


#define DOT11_SSID_MAX_LENGTH      32
#define DOT11_RATE_SET_MAX_LENGTH 126


typedef struct _DOT11_SSID {
  ULONG uSSIDLength;
  UCHAR ucSSID[DOT11_SSID_MAX_LENGTH];
} DOT11_SSID, *PDOT11_SSID;


typedef UCHAR DOT11_MAC_ADDRESS[6];


typedef enum _DOT11_BSS_TYPE {
  DOT11_BSS_TYPE_UNUSED,
} DOT11_BSS_TYPE;


typedef enum _DOT11_PHY_TYPE {
  DOT11_PHY_TYPE_UNUSED,
} DOT11_PHY_TYPE;


typedef struct _WLAN_RATE_SET {
  ULONG uRateSetLength;
  USHORT usRateSet[DOT11_RATE_SET_MAX_LENGTH];
} WLAN_RATE_SET, *PWLAN_RATE_SET;


typedef struct _WLAN_BSS_ENTRY {
    DOT11_SSID dot11Ssid;
    ULONG uPhyId;
    DOT11_MAC_ADDRESS dot11Bssid;
    DOT11_BSS_TYPE dot11BssType;
    DOT11_PHY_TYPE dot11BssPhyType;
    LONG lRssi;
    ULONG uLinkQuality;
    BOOLEAN bInRegDomain;
    USHORT usBeaconPeriod;
    ULONGLONG ullTimestamp;
    ULONGLONG ullHostTimestamp;
    USHORT usCapabilityInformation;
    ULONG  ulChCenterFrequency;
    WLAN_RATE_SET     wlanRateSet; // --> to be verified, according to MSDN this member exists, according to the include of the Platform SDK it doesn't
    ULONG ulIeOffset;
    ULONG ulIeSize;
} WLAN_BSS_ENTRY, * PWLAN_BSS_ENTRY;



typedef struct _WLAN_BSS_LIST {
    DWORD dwTotalSize;
    DWORD dwNumberOfItems;
    WLAN_BSS_ENTRY wlanBssEntries[1];
} WLAN_BSS_LIST, *PWLAN_BSS_LIST;



typedef DWORD (WINAPI *WlanOpenHandleFunction)(
    DWORD dwClientVersion,
    PVOID pReserved,
    PDWORD pdwNegotiatedVersion,
    PHANDLE phClientHandle
);



typedef DWORD (WINAPI *WlanEnumInterfacesFunction)(
    HANDLE hClientHandle,
    PVOID pReserved,
    PWLAN_INTERFACE_INFO_LIST *ppInterfaceList
);



typedef DWORD (WINAPI *WlanGetNetworkBssListFunction)(
    HANDLE hClientHandle,
    const GUID *pInterfaceGuid,
    const  PDOT11_SSID pDot11Ssid,
    DOT11_BSS_TYPE dot11BssType,
    BOOL bSecurityEnabled,
    PVOID pReserved,
    PWLAN_BSS_LIST *ppWlanBssList
);



typedef DWORD (WINAPI *WlanCloseHandleFunction)(
    HANDLE hClientHandle,
    PVOID pReserved
);



typedef VOID (WINAPI *WlanFreeMemoryFunction)(
  PVOID pMemory
);



WlanOpenHandleFunction WlanOpenHandle;
WlanEnumInterfacesFunction WlanEnumInterfaces;
WlanGetNetworkBssListFunction WlanGetNetworkBssList;
WlanCloseHandleFunction WlanCloseHandle;
WlanFreeMemoryFunction WlanFreeMemory;



typedef struct 
{
   LPWSTR wszGuid;
} INTF_KEY_ENTRY, *PINTF_KEY_ENTRY;



typedef struct 
{
   DWORD dwNumIntfs;
   PINTF_KEY_ENTRY pIntfs;
} INTFS_KEY_TABLE, *PINTFS_KEY_TABLE;



typedef DWORD (WINAPI *WZCEnumInterfacesFunction)(LPWSTR pSrvAddr, PINTFS_KEY_TABLE pIntfs);



typedef struct 
{
   DWORD   dwDataLen;
   LPBYTE  pData;
} RAW_DATA, *PRAW_DATA;



typedef struct 
{
   LPWSTR wszGuid;
   LPWSTR wszDescr;
   ULONG ulMediaState;
   ULONG ulMediaType;
   ULONG ulPhysicalMediaType;
   INT nInfraMode;
   INT nAuthMode;
   INT nWepStatus;
   ULONG padding1[2];  // 16 chars on Windows XP SP3 or SP2 with WLAN Hotfix installed, 8 chars otherwise
   DWORD dwCtlFlags;
   DWORD dwCapabilities;
   RAW_DATA rdSSID;
   RAW_DATA rdBSSID;
   RAW_DATA rdBSSIDList;
   RAW_DATA rdStSSIDList;
   RAW_DATA rdCtrlData;
   BOOL bInitialized;
   ULONG padding2[64];  // for security reason ...
} INTF_ENTRY, *PINTF_ENTRY;



typedef DWORD (WINAPI *WZCQueryInterfaceFunction)(LPWSTR pSrvAddr, DWORD dwInFlags, PINTF_ENTRY pIntf, LPDWORD pdwOutFlags);



typedef wchar_t ADAPTER_NAME;
typedef wchar_t ADAPTER_DESCRIPTION;
typedef char AP_NAME;



#define ADAPTER_NAME_LENGTH        256
#define ADAPTER_DESCRIPTION_LENGTH 256
#define AP_NAME_LENGTH             256
#define INTF_DESCR         (0x00010000)
#define INTF_BSSIDLIST     (0x04000000)
#define INTF_LIST_SCAN     (0x08000000)



typedef UCHAR NDIS_802_11_MAC_ADDRESS[6];


typedef struct _NDIS_802_11_SSID 
{
   ULONG SsidLength;
   UCHAR Ssid [32];
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;



typedef UCHAR NDIS_802_11_RATES_EX[16];
typedef LONG NDIS_802_11_RSSI;
typedef UCHAR NDIS_802_11_RATES_EX[16];



typedef enum _NDIS_802_11_NETWORK_TYPE 
{
   Ndis802_11FH,
   Ndis802_11DS,
   Ndis802_11NetworkTypeMax,
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;



typedef struct _NDIS_802_11_CONFIGURATION_FH 
{
   ULONG Length;
   ULONG HopPattern;
   ULONG HopSet;
   ULONG DwellTime;
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;



typedef struct _NDIS_802_11_CONFIGURATION
{
   ULONG  Length;
   ULONG  BeaconPeriod;
   ULONG  ATIMWindow;
   ULONG  DSConfig;
   NDIS_802_11_CONFIGURATION_FH  FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;



typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE 
{
   Ndis802_11IBSS,
   Ndis802_11Infrastructure,
   Ndis802_11AutoUnknown,
   Ndis802_11InfrastructureMax,
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;



typedef struct _NDIS_WLAN_BSSID_EX
{
   ULONG  Length;
   NDIS_802_11_MAC_ADDRESS  MacAddress;
   UCHAR  Reserved[2];
   NDIS_802_11_SSID  Ssid;
   ULONG  Privacy;
   NDIS_802_11_RSSI  Rssi;
   NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
   NDIS_802_11_CONFIGURATION  Configuration;
   NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
   NDIS_802_11_RATES_EX  SupportedRates;
   ULONG  IELength;
   UCHAR  IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;



typedef struct _NDIS_WLAN_BSSID 
{
   UCHAR padding1[4];
   ULONG Length;
   UCHAR padding2[4];
   NDIS_802_11_MAC_ADDRESS MacAddress;
   UCHAR Reserved[2];
   NDIS_802_11_SSID Ssid;
   ULONG Privacy;
   NDIS_802_11_RSSI Rssi;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;



typedef struct _NDIS_802_11_BSSID_LIST 
{
   ULONG NumberOfItems;
   NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;



typedef DWORD (WINAPI *WZCRefreshInterfaceFunction)
(
   LPWSTR pSrvAddr,
   DWORD dwInFlags,
   PINTF_ENTRY pIntf,
   LPDWORD pdwOutFlags
);



typedef struct _ADAPTER_INFO 
{
   ADAPTER_NAME name[ADAPTER_NAME_LENGTH];
   ADAPTER_DESCRIPTION description[ADAPTER_DESCRIPTION_LENGTH];
} ADAPTER_INFO;



WZCEnumInterfacesFunction WZCEnumInterfaces;
WZCQueryInterfaceFunction WZCQueryInterface;
WZCRefreshInterfaceFunction WZCRefreshInterface;



#endif
