package com.vwp;

import java.io.*;
import java.net.*;
import java.util.*;
import android.content.*;
import android.net.wifi.*;



class wloc_req
{
   public static final int WLOC_MAX_NETWORKS=16;
   
   public byte     version,length;
   public byte[][] bssids=new byte[WLOC_MAX_NETWORKS][6];  
   public byte[]   signal=new byte[WLOC_MAX_NETWORKS];
   public int      cgiIP;
   
   wloc_req()
   {
      version=1;
      length=118;
      cgiIP=0;
   }
}



class wloc_res
{
   public byte  version,length;
   public byte  result,iresult,quality;
   public byte  cres6,cres7,cres8;
   public int   lat,lon;    
   public short ccode;
   public short wres34,wres56,wres78;
}



class wloc_position
{
   wloc_position()
   {   
   }
   
   double lat,lon;
   byte   quality;
   short  ccode;
}



public class WLocate
{
   public static final int WLOC_OK=0;
   public static final int WLOC_CONNECTION_ERROR=1;
   public static final int WLOC_SERVER_ERROR=2;
   public static final int WLOC_LOCATION_ERROR=3;
   public static final int WLOC_ERROR=100;
   
   private static final int WLOC_RESULT_OK=1;
   private static final int WLOC_RESULT_ERROR=2;
   private static final int WLOC_RESULT_IERROR=3;
   
   WifiManager  wifi;
   WifiReceiver receiverWifi = new WifiReceiver();
   
   public String wloc_get_country_from_code(short ccode)
   {
      switch (ccode)
      {
         case 1:
            return "DE";
         case 2:
            return "AT";
         case 3:
            return "CH";
         case 4:
            return "NL";
         case 5:
            return "BE";
         case 6:
            return "LU";

         case 7:
            return "NO";

         case 8:
            return "SE";

         case 9:
            return "DK";

         case 10:
            return "AF";

         case 12:
            return "AL";

         case 13:
            return "DZ";

         case 17:
            return "AN";

         case 18:
            return "AG";

         case 19:
            return "AR";

         case 20:
            return "AM";

         case 21:
            return "AU";

         case 23:
            return "BS";

         case 24:
            return "BH";

         case 25:
            return "BD";

         case 26:
            return "BB";

         case 27:
            return "BY";

         case 28:
            return "BZ";

         case 29:
            return "BJ";

         case 30:
            return "BM";

         case 32:
            return "BO";

         case 33:
            return "BA";

         case 36:
            return "BR";

         case 37:
            return "BN";

         case 38:
            return "BG";

         case 43:
            return "CA";

         case 44:
            return "CV";

         case 47:
            return "CL";

         case 48:
            return "CN";

         case 49:
            return "CO";

         case 52:
            return "CR";

         case 53:
            return "HR";

         case 55:
            return "CY";

         case 56:
            return "CZ";

         case 59:
            return "DO";

         case 60:
            return "EC";

         case 61:
            return "EG";

         case 66:
            return "ET";

         case 68:
            return "FI";

         case 69:
            return "FR";

         case 73:
            return "GH";

         case 75:
            return "GR";

         case 76:
            return "GL";

         case 77:
            return "GD";

         case 78:
            return "GU";

         case 79:
            return "GT";

         case 82:
            return "HT";

         case 83:
            return "HN";

         case 84:
            return "HK";

         case 85:
            return "HU";

         case 86:
            return "IS";

         case 87:
            return "IN";

         case 88:
            return "ID";

         case 89:
            return "IR";

         case 90:
            return "IQ";

         case 91:
            return "IE";

         case 93:
            return "IT";

         case 94:
            return "JM";

         case 95:
            return "JP";

         case 97:
            return "JO";

         case 98:
            return "KZ";

         case 99:
            return "KE";

         case 102:
            return "KR";

         case 103:
            return "KW";

         case 104:
            return "KG";

         case 105:
            return "LA";

         case 106:
            return "LV";

         case 107:
            return "LB";

         case 108:
            return "LS";

         case 111:
            return "LT";

         case 115:
            return "MY";

         case 116:
            return "MV";

         case 118:
            return "MT";

         case 119:
            return "MQ";

         case 121:
            return "MU";

         case 123:
            return "MX";

         case 124:
            return "MC";

         case 125:
            return "MN";

         case 126:
            return "MA";

         case 127:
            return "MZ";

         case 131:
            return "NZ";

         case 133:
            return "NI";

         case 135:
            return "NG";

         case 137:
            return "OM";

         case 138:
            return "PK";

         case 141:
            return "PA";

         case 142:
            return "PY";

         case 144:
            return "PE";

         case 145:
            return "PH";

         case 147:
            return "PL";

         case 148:
            return "PT";

         case 149:
            return "PR";

         case 150:
            return "QA";

         case 151:
            return "RO";

         case 152:
            return "RU";

         case 155:
            return "SM";

         case 157:
            return "SA";

         case 158:
            return "SN";

         case 161:
            return "SG";

         case 162:
            return "SK";

         case 163:
            return "SI";

         case 166:
            return "ZA";

         case 167:
            return "ES";

         case 168:
            return "LK";

         case 169:
            return "SD";
         case 170:
            return "SR";
         case 172:
            return "SY";
         case 173:
            return "TW";
         case 174:
            return "TJ";
         case 175:
            return "TZ";
         case 176:
            return "TH";
         case 179:
            return "TT";
         case 180:
            return "TN";
         case 181:
            return "TR";
         case 182:
            return "TM";
         case 185:
            return "UA";
         case 186:
            return "AE";
         case 187:
            return "UK";
         case 188:
            return "US";
         case 189:
            return "UY";
         case 191:
            return "VE";
         case 192:
            return "VN";
         case 195:
            return "ZM";
         case 196:
            return "ZW";
         default:
            return null;
      }
   }
   
   
   
   public void wloc_request_position(Context ctx)
   {
      wifi = (WifiManager) ctx.getSystemService(Context.WIFI_SERVICE);
      ctx.registerReceiver(receiverWifi, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
      wifi.startScan();   
   }
   
   
   
   private int get_position(wloc_req request,wloc_position position)
   {
      Socket wlocSocket;
      DataInputStream  din;
      DataOutputStream dout;
      int              i;
      wloc_res         result=new wloc_res();
      
      try
      {
         wlocSocket=new Socket("62.112.159.250",10443);
         dout=new DataOutputStream(wlocSocket.getOutputStream());
         din=new DataInputStream(wlocSocket.getInputStream());
         dout.write(request.version);
         dout.write(request.length);
         for (i=0; i<wloc_req.WLOC_MAX_NETWORKS; i++)
          dout.write(request.bssids[i],0,6);
         for (i=0; i<wloc_req.WLOC_MAX_NETWORKS; i++)
          dout.writeByte(request.signal[i]);
         dout.writeInt(request.cgiIP);
         dout.flush();

         result.version=din.readByte();
         result.length=din.readByte();
         result.result=din.readByte();
         result.iresult=din.readByte();
         result.quality=din.readByte();
         result.cres6=din.readByte();
         result.cres7=din.readByte();
         result.cres7=din.readByte();
         result.lat=din.readInt();
         result.lon=din.readInt();
         result.ccode=din.readShort();
         result.wres34=din.readShort();
         result.wres56=din.readShort();
         result.wres78=din.readShort();
         if (result.result!=WLOC_RESULT_OK)
         {
            wlocSocket.close();
            return WLOC_LOCATION_ERROR;
         }
         position.lat=result.lat/10000000.0;
         position.lon=result.lon/10000000.0;
         position.quality=result.quality;
      }
      catch (Exception e)
      {
         e.printStackTrace();
         return WLOC_CONNECTION_ERROR;
      }
      return WLOC_OK;
   }

   
   
   class WifiReceiver extends BroadcastReceiver 
   {
      public void onReceive(Context c, Intent intent) 
      {
         int           netCnt=0,ret=WLOC_LOCATION_ERROR;
         wloc_req      request;
         wloc_position pos=null;
         
         List<ScanResult> configs=wifi.getScanResults();
         request=new wloc_req();
         for (ScanResult config : configs) 
         {            
            {
               String bssidStr[];

               bssidStr=config.BSSID.split(":");
               request.bssids[netCnt][0]=(byte)Integer.parseInt(bssidStr[0],16);
               request.bssids[netCnt][1]=(byte)Integer.parseInt(bssidStr[1],16);
               request.bssids[netCnt][2]=(byte)Integer.parseInt(bssidStr[2],16);
               request.bssids[netCnt][3]=(byte)Integer.parseInt(bssidStr[3],16);
               request.bssids[netCnt][4]=(byte)Integer.parseInt(bssidStr[4],16);
               request.bssids[netCnt][5]=(byte)Integer.parseInt(bssidStr[5],16);
               request.signal[netCnt]=(byte)Math.abs(config.level);               
            }
            
            netCnt++;
            if (netCnt>=wloc_req.WLOC_MAX_NETWORKS) break;   
         }        
         pos=new wloc_position();
         ret=get_position(request,pos);
         wloc_return_position(ret,pos.lat,pos.lon,pos.quality,pos.ccode);
     }
  }
   
   
   protected void wloc_return_position(int ret,double lat,double lon,int quality,short ccode)
   {
      
   }
   
   
}
