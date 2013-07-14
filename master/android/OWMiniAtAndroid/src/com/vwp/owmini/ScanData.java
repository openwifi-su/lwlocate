package com.vwp.owmini;

import java.util.*;
import java.util.concurrent.locks.*;

import android.content.*;
import android.location.*;
import android.net.wifi.*;
import android.widget.TextView;

public class ScanData 
{
           Lock              lock=new ReentrantLock();
           Vector<WMapEntry> wmapList=new Vector<WMapEntry>();
           OWMiniAtAndroid    ctx;
   private int               flags=OWMiniAtAndroid.FLAG_NO_NET_ACCESS,storedValues;
   private int               freeHotspotWLANs=0;
           boolean           isActive=true,scanningEnabled=true,hudCounter=false,appVisible=false;
           int               viewMode=OWMiniAtAndroid.VIEW_MODE_MAIN,threadMode=OWMiniAtAndroid.THREAD_MODE_SCAN,
                             uploadedCount=0,uploadedRank=0,uploadThres=0;
           WifiManager       wifiManager;
   private double            lat,lon;
           TextView          bigCntTextHud;
           String            ownBSSID;
           HUDView           mView;
           Thread            watchThread=null;
           ScanService       service=null;

   
   ScanData()
   {      
   }
   
   void init(OWMiniAtAndroid ctx)
   {
      this.ctx=ctx;
      wifiManager= (WifiManager) ctx.getSystemService(Context.WIFI_SERVICE);      
      wifiManager.setWifiEnabled(true);
      
      LocationManager location= (LocationManager)ctx.getSystemService(Context.LOCATION_SERVICE);
      if(!location.isProviderEnabled(LocationManager.GPS_PROVIDER ))
       ctx.simpleAlert(ctx.getResources().getString(R.string.gpsdisabled_warn),null,OWMiniAtAndroid.ALERT_GPSWARN);
   }
   
   
   
   void setFlags(int flags)
   {
	   lock.lock();
	   this.flags=flags;
	   lock.unlock();
	   
      if ((flags & OWMiniAtAndroid.FLAG_NO_NET_ACCESS)!=0)
	    wifiManager.createWifiLock(WifiManager.WIFI_MODE_SCAN_ONLY,"OpenWLANMapMini");
      else
       wifiManager.createWifiLock(WifiManager.WIFI_MODE_FULL,"OpenWLANMapMini");
   }

   
   void setLatLon(double lat,double lon)
   {
      lock.lock();
      this.lat=lat;
      this.lon=lon;
      lock.unlock();
   }
   
   
   double getLat()
   {
      double d;
      
      lock.lock();
      d=lat;
      lock.unlock();
      return d;
   }

   
   double getLon()
   {
      double d;
      
      lock.lock();
      d=lon;
      lock.unlock();
      return d;
   }

   
      
   int getFlags()
   {
	   int val;
	   
	   lock.lock();
	   val=flags;
	   lock.unlock();
	   return val;
   }

   void setStoredValues(int storedValues)
   {
	   this.storedValues=storedValues;
   }

   int incStoredValues()
   {
	   storedValues++;

	   OWMiniAtAndroid.sendMessage(OWMiniAtAndroid.ScannerHandler.MSG_UPD_AP_COUNT,storedValues,0,null);
	   return storedValues;
   }

   
   
   int getStoredValues()
   {
      return storedValues;
   }

      
   
   void setFreeHotspotWLANs(int freeHotspotWLANs)
   {
      this.freeHotspotWLANs=freeHotspotWLANs;
   }

   int incFreeHotspotWLANs()
   {
      freeHotspotWLANs++;
      return freeHotspotWLANs;
   }

   
   int getFreeHotspotWLANs()
   {
      return freeHotspotWLANs;
   }


}
