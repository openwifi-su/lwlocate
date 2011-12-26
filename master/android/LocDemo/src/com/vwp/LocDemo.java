package com.vwp;

import android.app.*;
import android.os.*;
import android.widget.*;
import android.net.wifi.*;
import java.util.*;
import java.net.*;
import java.io.*;
import android.content.*;



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
   String countryName;
}



public class LocDemo extends Activity 
{
   public static final int WLOC_OK=0;
   public static final int WLOC_CONNECTION_ERROR=1;
   public static final int WLOC_SERVER_ERROR=2;
   public static final int WLOC_LOCATION_ERROR=3;
   public static final int WLOC_ERROR=100;
   
   public static final int WLOC_RESULT_OK=1;
   public static final int WLOC_RESULT_ERROR=2;
   public static final int WLOC_RESULT_IERROR=3;
   
	TextView     tv;
	WifiManager  wifi;
	WifiReceiver receiverWifi = new WifiReceiver();
	
	public int wloc_get_country_from_code(wloc_position position)
	{
	   switch (position.ccode)
	   {
	      case 1:
	         position.countryName="DE";
	         return WLOC_OK;
	      case 2:
	         position.countryName="AT";
	         return WLOC_OK;
	      case 3:
	         position.countryName="CH";
	         return WLOC_OK;
	      case 4:
	         position.countryName="NL";
	         return WLOC_OK;
	      case 5:
	         position.countryName="BE";
	         return WLOC_OK;
	      case 6:
	         position.countryName="LU";
	         return WLOC_OK;
	      case 7:
	         position.countryName="NO";
	         return WLOC_OK;
	      case 8:
	         position.countryName="SE";
	         return WLOC_OK;
	      case 9:
	         position.countryName="DK";
	         return WLOC_OK;
	      case 10:
	         position.countryName="AF";
	         return WLOC_OK;
	      case 12:
	         position.countryName="AL";
	         return WLOC_OK;
	      case 13:
	         position.countryName="DZ";
	         return WLOC_OK;
	      case 17:
	         position.countryName="AN";
	         return WLOC_OK;
	      case 18:
	         position.countryName="AG";
	         return WLOC_OK;
	      case 19:
	         position.countryName="AR";
	         return WLOC_OK;
	      case 20:
	         position.countryName="AM";
	         return WLOC_OK;
	      case 21:
	         position.countryName="AU";
	         return WLOC_OK;
	      case 23:
	         position.countryName="BS";
	         return WLOC_OK;
	      case 24:
	         position.countryName="BH";
	         return WLOC_OK;
	      case 25:
	         position.countryName="BD";
	         return WLOC_OK;
	      case 26:
	         position.countryName="BB";
	         return WLOC_OK;
	      case 27:
	         position.countryName="BY";
	         return WLOC_OK;
	      case 28:
	         position.countryName="BZ";
	         return WLOC_OK;
	      case 29:
	         position.countryName="BJ";
	         return WLOC_OK;
	      case 30:
	         position.countryName="BM";
	         return WLOC_OK;
	      case 32:
	         position.countryName="BO";
	         return WLOC_OK;
	      case 33:
	         position.countryName="BA";
	         return WLOC_OK;
	      case 36:
	         position.countryName="BR";
	         return WLOC_OK;
	      case 37:
	         position.countryName="BN";
	         return WLOC_OK;
	      case 38:
	         position.countryName="BG";
	         return WLOC_OK;
	      case 43:
	         position.countryName="CA";
	         return WLOC_OK;
	      case 44:
	         position.countryName="CV";
	         return WLOC_OK;
	      case 47:
	         position.countryName="CL";
	         return WLOC_OK;
	      case 48:
	         position.countryName="CN";
	         return WLOC_OK;
	      case 49:
	         position.countryName="CO";
	         return WLOC_OK;
	      case 52:
	         position.countryName="CR";
	         return WLOC_OK;
	      case 53:
	         position.countryName="HR";
	         return WLOC_OK;
	      case 55:
	         position.countryName="CY";
	         return WLOC_OK;
	      case 56:
	         position.countryName="CZ";
	         return WLOC_OK;
	      case 59:
	         position.countryName="DO";
	         return WLOC_OK;
	      case 60:
	         position.countryName="EC";
	         return WLOC_OK;
	      case 61:
	         position.countryName="EG";
	         return WLOC_OK;
	      case 66:
	         position.countryName="ET";
	         return WLOC_OK;
	      case 68:
	         position.countryName="FI";
	         return WLOC_OK;
	      case 69:
	         position.countryName="FR";
	         return WLOC_OK;
	      case 73:
	         position.countryName="GH";
	         return WLOC_OK;
	      case 75:
	         position.countryName="GR";
	         return WLOC_OK;
	      case 76:
	         position.countryName="GL";
	         return WLOC_OK;
	      case 77:
	         position.countryName="GD";
	         return WLOC_OK;
	      case 78:
	         position.countryName="GU";
	         return WLOC_OK;
	      case 79:
	         position.countryName="GT";
	         return WLOC_OK;
	      case 82:
	         position.countryName="HT";
	         return WLOC_OK;
	      case 83:
	         position.countryName="HN";
	         return WLOC_OK;
	      case 84:
	         position.countryName="HK";
	         return WLOC_OK;
	      case 85:
	         position.countryName="HU";
	         return WLOC_OK;
	      case 86:
	         position.countryName="IS";
	         return WLOC_OK;
	      case 87:
	         position.countryName="IN";
	         return WLOC_OK;
	      case 88:
	         position.countryName="ID";
	         return WLOC_OK;
	      case 89:
	         position.countryName="IR";
	         return WLOC_OK;
	      case 90:
	         position.countryName="IQ";
	         return WLOC_OK;
	      case 91:
	         position.countryName="IE";
	         return WLOC_OK;
	      case 93:
	         position.countryName="IT";
	         return WLOC_OK;
	      case 94:
	         position.countryName="JM";
	         return WLOC_OK;
	      case 95:
	         position.countryName="JP";
	         return WLOC_OK;
	      case 97:
	         position.countryName="JO";
	         return WLOC_OK;
	      case 98:
	         position.countryName="KZ";
	         return WLOC_OK;
	      case 99:
	         position.countryName="KE";
	         return WLOC_OK;
	      case 102:
	         position.countryName="KR";
	         return WLOC_OK;
	      case 103:
	         position.countryName="KW";
	         return WLOC_OK;
	      case 104:
	         position.countryName="KG";
	         return WLOC_OK;
	      case 105:
	         position.countryName="LA";
	         return WLOC_OK;
	      case 106:
	         position.countryName="LV";
	         return WLOC_OK;
	      case 107:
	         position.countryName="LB";
	         return WLOC_OK;
	      case 108:
	         position.countryName="LS";
	         return WLOC_OK;
	      case 111:
	         position.countryName="LT";
	         return WLOC_OK;
	      case 115:
	         position.countryName="MY";
	         return WLOC_OK;
	      case 116:
	         position.countryName="MV";
	         return WLOC_OK;
	      case 118:
	         position.countryName="MT";
	         return WLOC_OK;
	      case 119:
	         position.countryName="MQ";
	         return WLOC_OK;
	      case 121:
	         position.countryName="MU";
	         return WLOC_OK;
	      case 123:
	         position.countryName="MX";
	         return WLOC_OK;
	      case 124:
	         position.countryName="MC";
	         return WLOC_OK;
	      case 125:
	         position.countryName="MN";
	         return WLOC_OK;
	      case 126:
	         position.countryName="MA";
	         return WLOC_OK;
	      case 127:
	         position.countryName="MZ";
	         return WLOC_OK;
	      case 131:
	         position.countryName="NZ";
	         return WLOC_OK;
	      case 133:
	         position.countryName="NI";
	         return WLOC_OK;
	      case 135:
	         position.countryName="NG";
	         return WLOC_OK;
	      case 137:
	         position.countryName="OM";
	         return WLOC_OK;
	      case 138:
	         position.countryName="PK";
	         return WLOC_OK;
	      case 141:
	         position.countryName="PA";
	         return WLOC_OK;
	      case 142:
	         position.countryName="PY";
	         return WLOC_OK;
	      case 144:
	         position.countryName="PE";
	         return WLOC_OK;
	      case 145:
	         position.countryName="PH";
	         return WLOC_OK;
	      case 147:
	         position.countryName="PL";
	         return WLOC_OK;
	      case 148:
	         position.countryName="PT";
	         return WLOC_OK;
	      case 149:
	         position.countryName="PR";
	         return WLOC_OK;
	      case 150:
	         position.countryName="QA";
	         return WLOC_OK;
	      case 151:
	         position.countryName="RO";
	         return WLOC_OK;
	      case 152:
	         position.countryName="RU";
	         return WLOC_OK;
	      case 155:
	         position.countryName="SM";
	         return WLOC_OK;
	      case 157:
	         position.countryName="SA";
	         return WLOC_OK;
	      case 158:
	         position.countryName="SN";
	         return WLOC_OK;
	      case 161:
	         position.countryName="SG";
	         return WLOC_OK;
	      case 162:
	         position.countryName="SK";
	         return WLOC_OK;
	      case 163:
	         position.countryName="SI";
	         return WLOC_OK;
	      case 166:
	         position.countryName="ZA";
	         return WLOC_OK;
	      case 167:
	         position.countryName="ES";
	         return WLOC_OK;
	      case 168:
	         position.countryName="LK";
	         return WLOC_OK;
	      case 169:
	         position.countryName="SD";
	         return WLOC_OK;
	      case 170:
	         position.countryName="SR";
	         return WLOC_OK;
	      case 172:
	         position.countryName="SY";
	         return WLOC_OK;
	      case 173:
	         position.countryName="TW";
	         return WLOC_OK;
	      case 174:
	         position.countryName="TJ";
	         return WLOC_OK;
	      case 175:
	         position.countryName="TZ";
	         return WLOC_OK;
	      case 176:
	         position.countryName="TH";
	         return WLOC_OK;
	      case 179:
	         position.countryName="TT";
	         return WLOC_OK;
	      case 180:
	         position.countryName="TN";
	         return WLOC_OK;
	      case 181:
	         position.countryName="TR";
	         return WLOC_OK;
	      case 182:
	         position.countryName="TM";
	         return WLOC_OK;
	      case 185:
	         position.countryName="UA";
	         return WLOC_OK;
	      case 186:
	         position.countryName="AE";
	         return WLOC_OK;
	      case 187:
	         position.countryName="UK";
	         return WLOC_OK;
	      case 188:
	         position.countryName="US";
	         return WLOC_OK;
	      case 189:
	         position.countryName="UY";
	         return WLOC_OK;
	      case 191:
	         position.countryName="VE";
	         return WLOC_OK;
	      case 192:
	         position.countryName="VN";
	         return WLOC_OK;
	      case 195:
	         position.countryName="ZM";
	         return WLOC_OK;
	      case 196:
	         position.countryName="ZW";
	         return WLOC_OK;
	      default:
	         return WLOC_ERROR;
	   }
	}
	
	
	
	public int get_position(wloc_req request,wloc_position position)
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
         wloc_get_country_from_code(position);
		}
		catch (Exception e)
		{
		   e.printStackTrace();
		   return WLOC_CONNECTION_ERROR;
		}
		return WLOC_OK;
	}
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
    	
        super.onCreate(savedInstanceState);
        tv = new TextView(this);
        
        setContentView(tv);
        
/*        wifi = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        registerReceiver(receiverWifi, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
        wifi.startScan(); */
        
        {
           wloc_req      request;
           wloc_position pos;
           int           ret;
           
           request=new wloc_req();
           request.bssids[0][0]=(byte)0x00;
           request.bssids[0][1]=(byte)0x01;
           request.bssids[0][2]=(byte)0xE3;
           request.bssids[0][3]=(byte)0x49;
           request.bssids[0][4]=(byte)0xA8;
           request.bssids[0][5]=(byte)0xC7;
           
           pos=new wloc_position();
           ret=get_position(request,pos);
           tv.append("\n"+ret+"\n"+pos.lat+"  "+pos.lon);
        }
    }

    class WifiReceiver extends BroadcastReceiver 
    {
       public void onReceive(Context c, Intent intent) 
       {
         List<ScanResult> configs=wifi.getScanResults();
   		 for (ScanResult config : configs) 
   		 {
   		    tv.append("\n\n" + config.BSSID+"  "+config.level);
   		 }
      }
   }

}


