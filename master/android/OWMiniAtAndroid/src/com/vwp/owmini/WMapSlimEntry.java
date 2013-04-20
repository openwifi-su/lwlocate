package com.vwp.owmini;

public class WMapSlimEntry
{
   int    cnt=0;
   double lat,lon;
   String BSSID;

   
   public WMapSlimEntry(String lat,String lon)
   {
      this.lat=Double.parseDouble(lat);
      this.lon=Double.parseDouble(lon);
   }   

   public WMapSlimEntry(String BSSID,double lat,double lon)
   {
      this.BSSID=BSSID;
      this.lat=lat;
      this.lon=lon;
      cnt=1;
   }   

   public void addPos(double lat,double lon)
   {
      cnt++;
      if (((lat==0.0) && (lon==0.0)) || ((this.lat==0.0) && (this.lon==0.0)))
      {
         this.lat=0.0;
         this.lon=0.0;
         return;
      }
      this.lat+=lat;
      this.lon+=lon;
   }
   
   
}
