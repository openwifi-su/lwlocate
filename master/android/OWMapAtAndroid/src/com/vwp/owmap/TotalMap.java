package com.vwp.owmap;

import java.io.*;
import java.net.*;
import java.util.*;

import android.content.*;
import android.graphics.*;
import android.os.Environment;

import com.vwp.libwlocate.map.*;


public class TotalMap extends MapOverlay
{
   private OWMapAtAndroid     ctx;
   static Vector<EarthCoord>  coordList=new Vector<EarthCoord>();
   private Paint              wlanColour;
   private double             latMin,latMax,lonMin,lonMax;
   private Bitmap             drawMap=null;
   private static int         m_prevZoom=-1,prevTileX=0,prevTileY=0; // static to keep previous values
   
   public TotalMap(OWMapAtAndroid ctx,String BSSID)
   {
      super();

      this.ctx=ctx;
      if (coordList.size()==0) loadMap(BSSID);
      
      wlanColour=new Paint();
      wlanColour.setARGB(255,200,0,0);
      wlanColour.setStyle(Paint.Style.STROKE);
      wlanColour.setStrokeWidth(2);
      if (m_prevZoom>0)
      {
         m_zoom=m_prevZoom;
         tileX=prevTileX;
         tileY=prevTileY;
      }
   }
   
   
   public void close()
   {
      m_prevZoom=m_zoom;
      prevTileX=tileX;
      prevTileY=tileY;
   }
   
   
   private boolean downloadData(String ownBSSID)
   {
      DataOutputStream  out;
      String            outString,inString;
      float             lat,lon;
      EarthCoord        coord;
      HttpURLConnection c=null;
      DataOutputStream  os=null;
      DataInputStream   is=null;
      int               rc;
            
      outString=ownBSSID.replace(":","").replace(".","");
      outString=outString.toUpperCase()+"\n";
      try
      {
         URL connectURL = new URL("http://www.openwlanmap.org/android/map.php");    
         c= (HttpURLConnection) connectURL.openConnection();
         if (c==null) return false;
        
         c.setDoOutput(true); // enable POST
         c.setRequestMethod("POST");
         c.addRequestProperty("Content-Type","application/x-www-form-urlencoded, *.*");
         c.addRequestProperty("Content-Length",""+outString.length());
         os = new DataOutputStream(c.getOutputStream());
         os.write(outString.getBytes());
         os.flush();
         os.close();
         outString=null;
         os=null;
         System.gc();
         
         rc = c.getResponseCode();
         if (rc != HttpURLConnection.HTTP_OK) 
         {
            OWMapAtAndroid.sendMessage(OWMapAtAndroid.ScannerHandler.MSG_SIMPLE_ALERT,0,0,ctx.getResources().getString(R.string.http_error)+" "+rc);
            return false;
         }         
         is = new DataInputStream(new BufferedInputStream(c.getInputStream()));
         out=new DataOutputStream(ctx.openFileOutput(OWMapAtAndroid.MAP_DATA_FILE,Context.MODE_PRIVATE));
         try
         {
            while (is.available()>0)
            {
               inString=is.readLine();
               lat=Integer.parseInt(inString)/1000000.0f;
               inString=is.readLine();
               lon=Integer.parseInt(inString)/1000000.0f;
               if ((lat!=0.0) && (lon!=0.0))
               {
                  if (lat>latMax) latMax=lat;
                  if (lat<latMin) latMin=lat;
                  if (lon>lonMax) lonMax=lon;
                  if (lon<lonMin) lonMin=lon;
                  out.writeFloat(lat);
                  out.writeFloat(lon);
                  coord=new EarthCoord(lat,lon);
                  coordList.addElement(coord);
               }
            }
         }
         catch (NumberFormatException nfe)
         {
            nfe.printStackTrace();
            return false;
         }
         out.close();
                  
         is.close();
         is=null;
      }
      catch (IOException ioe1)
      {
         return false;
      }
      finally
      {
         try 
         {
            if (is != null) is.close();
            if (os != null) os.close();
            if (c != null) c.disconnect();
         }
         catch (IOException ioe2)
         {
            ioe2.printStackTrace();
            return false;
         } 
      }
      if (coordList.size()<5) return false;
      return true;
   }   
      
   private void loadMap(String BSSID)
   {
      DataInputStream in;
      boolean         success=true;
      float           lat,lon;
      EarthCoord      coord;

      try
      {         
         in=new DataInputStream(ctx.openFileInput(OWMapAtAndroid.MAP_DATA_FILE));
         latMin=1000.0f;
         latMax=-1000.0f;
         lonMin=1000.0f;
         lonMax=-1000.0f;
         while (in.available()>0)
         {
            lat=in.readFloat();
            lon=in.readFloat();
            if ((lat!=0) && (lon!=0))
            {
               if (lat>latMax) latMax=lat;
               if (lat<latMin) latMin=lat;
               if (lon>lonMax) lonMax=lon;
               if (lon<lonMin) lonMin=lon;
               coord=new EarthCoord(lat,lon);
               coordList.addElement(coord);
            }
         }
         in.close();
      }
      catch (IOException ioe)
      {         
         success=downloadData(BSSID);
      }
      if (!success)
      {
         ctx.scannerHandler.sendEmptyMessage(OWMapAtAndroid.ScannerHandler.MSG_DL_FAILURE);
      }
      else
      {
         double   distX,distY,maxDist;
         int      i;
         double[]        zoomList={46945.558739255015*256.0,
                                   23472.779369627508*256.0,
                                   11736.389684813754*256.0,
                                    5868.194842406877*256.0,
                                    2934.0974212034384*256.0,
                                    1467.0487106017192*256.0,
                                     733.5243553008596*256.0,
                                     366.7621776504298*256.0,
                                     183.3810888252149*256.0,
                                      91.69054441260745*256.0,
                                      45.845272206303726*256.0,
                                      22.922636103151863*256.0,
                                      11.461318051575931*256.0,
                                       5.730659025787966*256.0,
                                       2.865329512893983*256.0,
                                       1.4326647564469914*256.0,
                                       0.7163323782234957*256.0,
                                       0.35816618911174786*256.0};         
         
         distX=GeoUtils.latlon2dist(1.0,lonMin,1.0,lonMax);
         distY=GeoUtils.latlon2dist(latMin,1.0,latMax,1.0);
         if (distX>distY) maxDist=distX;
         else maxDist=distY;

         m_zoom=17;
         for (i=17; i>1; i--)
          if (maxDist/(zoomList[i]/240)>=2) m_zoom=i;

         tileX=GeoUtils.long2tilex(lonMin,m_zoom);
         tileY=GeoUtils.lat2tiley(latMax,m_zoom);
      }
   }
 
   public void doDraw(Canvas canvas,double lonMin,double lonMax,double latMin,double latMax,Bitmap drawMap)
   {
      EarthCoord coord;
      int        i;
      float      x,y;
//   vertauschte range-abfrage bei lon beachten!

      for (i=0; i<coordList.size(); i++)
      {
         coord=coordList.elementAt(i);
         
         if ((coord.lon>=lonMin) && (coord.lon<=lonMax) && (coord.lat<=latMin) && (coord.lat>=latMax))
         {      
            int useTileX=GeoUtils.long2tilex(coord.lon,m_zoom);
            int useTileY=GeoUtils.lat2tiley(coord.lat,m_zoom);
   
            float tileLon1=GeoUtils.tilex2long(useTileX,m_zoom);
            float tileLat1=GeoUtils.tiley2lat(useTileY,m_zoom);
            float tileLon2=GeoUtils.tilex2long(useTileX+1,m_zoom);
            float tileLat2=GeoUtils.tiley2lat(useTileY+1,m_zoom);
   
            y=(int)((useTileY-tileY)*256+(256.0*(coord.lat-tileLat1)/(tileLat2-tileLat1)));
            x=(int)((useTileX-tileX)*256+(256.0*(coord.lon-tileLon1)/(tileLon2-tileLon1)));
   
            canvas.drawRect(x-1,y-1,x+1,y+1,wlanColour);
         }
      }
      this.drawMap=drawMap;     
   }

   public void saveMap()
   {
      String path = Environment.getExternalStorageDirectory().toString()+"/owl_map.png";
            
      try 
      {
         FileOutputStream out = new FileOutputStream(path);
         drawMap.compress(Bitmap.CompressFormat.PNG,0,out);
         OWMapAtAndroid.sendMessage(OWMapAtAndroid.ScannerHandler.MSG_SIMPLE_ALERT,0,0,ctx.getResources().getText(R.string.map_saved_as)+" "+path);
      } 
      catch (Exception e) 
      {
         e.printStackTrace();
         OWMapAtAndroid.sendMessage(OWMapAtAndroid.ScannerHandler.MSG_SIMPLE_ALERT,0,0,ctx.getResources().getText(R.string.map_not_saved));
      }      
   }   
   
        
}
