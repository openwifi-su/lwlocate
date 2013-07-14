package com.vwp.libwlocate.map;

import java.io.*;

import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.methods.*;
import org.apache.http.impl.client.*;

import android.content.*;
import android.graphics.*;
import android.os.*;

public class GeoUtils
{
   public static final int MODE_OSM=1;
   public static final int MODE_GMAP=2;
   public static final int MODE_GSMAP=3;
      
   private String     cachePath=null;
   private int        mode=MODE_OSM,mirrorCnt=1;
   
   public GeoUtils(int mode)
   {
      this.mode=mode;
      String state = Environment.getExternalStorageState();
      if(state.equals(Environment.MEDIA_MOUNTED))
      {         
         File nomedia;
         
         cachePath=Environment.getExternalStorageDirectory().getPath()+"/com.vwp.geoutils/";
         nomedia=new File(cachePath+".nomedia");
         if (!nomedia.exists()) try
         {
            FileOutputStream out;
            
            out=new FileOutputStream(nomedia);
            out.flush();
            out.close();
         }
         catch (IOException ioe)
         {
         }
      }
   }

   public void setMode(int mode)
   {
      this.mode=mode;
   }
   
   private Bitmap downloadTile(String url,int x,int y,int zoom,Context ctx,String tilePath,boolean store)
   {
      Bitmap              bm;
      BufferedInputStream in;
      FileOutputStream    out;
      byte                data[];
      int                 len,rLen=0; 

      try 
      {
          HttpClient client = new DefaultHttpClient();
          HttpGet request = new HttpGet(url);
          request.setHeader("User-Agent","Android Browser");
          HttpResponse response = client.execute(request);
          in=new BufferedInputStream(response.getEntity().getContent());
          len=(int)response.getEntity().getContentLength();
          if (len<=0) return null;
          data=new byte[len];
          while (rLen<len)
          {
             rLen+=in.read(data,rLen,len-rLen);
          }
          if (len<100)
          {
/*             String error=new String(data);
             error=error;*/
             in.close();
             return null;
          }
          if (store)
          {
             if (tilePath==null)
              out=ctx.openFileOutput("tile_"+zoom+"_"+(x)+"_"+(y)+".png",Context.MODE_PRIVATE);
             else
              out=new FileOutputStream(tilePath);
             out.write(data);
             out.close();
             in.close();
          }
          bm=BitmapFactory.decodeByteArray(data,0,rLen);
          request.abort();
      }
      catch (Exception e)
      {
         e.printStackTrace();
         return null;
      }
      return bm;
   }

   public Bitmap loadMapTile(Context ctx,int x, int y, int z,boolean allowDownload)
   {
      Bitmap tile;
      
      tile=loadMapTile(ctx,x,y,z,allowDownload,mode);
      if ((tile==null) && (mode==MODE_GSMAP)) tile=loadMapTile(ctx,x,y,z,allowDownload,MODE_GMAP); 
      return tile;
   }   
   
   private Bitmap loadMapTile(Context ctx,int x, int y, int z,boolean allowDownload,int useMode)
   {
      FileInputStream fin;
      Bitmap          tile=null;
      String          tilePath=null;
      
      try
      {
         if (cachePath==null)
         {
            fin=ctx.openFileInput("tile_"+z+"_"+x+"_"+y+".png");
         }
         else
         {
            File   pathFile;
            
            if (useMode==MODE_OSM) tilePath=cachePath+"o";
            else if (useMode==MODE_GMAP) tilePath=cachePath+"g";
            else tilePath=cachePath+"s";
            tilePath=tilePath+"/"+z+"/"+x+"/";
            pathFile=new File(tilePath);
            pathFile.mkdirs();
            
            File nomediaFile=new File(cachePath+".nomedia");
            try
            {
               nomediaFile.createNewFile();
               nomediaFile.renameTo(new File(cachePath+".nomedia"));
            }
            catch (IOException ioe)
            {
          	  
            }
            
            tilePath=tilePath+y+".png";
            fin=new FileInputStream(tilePath);
         }
         tile=BitmapFactory.decodeStream(fin);
         fin.close();
         if ((tile==null) || (tile.getWidth()<=0) || (tile.getHeight()<=0)) throw new IOException();
      }
      catch (IOException ioe)
      {         
         tile=null;
         if (allowDownload)
         {
            if (useMode==MODE_OSM)
            {
               switch (mirrorCnt)
               {
                  case 1:
                     tile=downloadTile("http://a.tile.openstreetmap.org/"+z+"/"+x+"/"+y+".png",x,y,z,ctx,tilePath,true);
                     break;
                  case 2:
                      tile=downloadTile("http://b.tile.openstreetmap.org/"+z+"/"+x+"/"+y+".png",x,y,z,ctx,tilePath,true);
                      break;
                  case 3:
                      tile=downloadTile("http://c.tile.openstreetmap.org/"+z+"/"+x+"/"+y+".png",x,y,z,ctx,tilePath,true);
                      break;
               }
            }
            else if (useMode==MODE_GMAP) tile=downloadTile("http://mt"+mirrorCnt+".google.com/vt/v=w2.97&z="+z+"&x="+x+"&y="+y,x,y,z,ctx,tilePath,true);
            else tile=downloadTile("http://khm"+mirrorCnt+".google.com/kh/v=99&z="+z+"&x="+x+"&y="+y,x,y,z,ctx,tilePath,true);
            mirrorCnt++;
            if (mirrorCnt>3) mirrorCnt=1;
         }
      }
      return tile;
   }   
   
   private static double deg2rad(double deg) 
   {
      return (deg * Math.PI / 180.0);
   }
   
   public static double latlon2dist(double lat1, double lon1,double lat2,double lon2)
   {
      double R = 6371; // km
      double dLat = deg2rad(lat2-lat1);
      double dLon = deg2rad(lon2-lon1);
      double a = Math.sin(dLat/2.0) * Math.sin(dLat/2.0) + Math.cos(deg2rad(lat1)) * Math.cos(deg2rad(lat2)) * Math.sin(dLon/2.0) * Math.sin(dLon/2.0);
      double c = 2.0*Math.atan2(Math.sqrt(a),Math.sqrt(1.0-a));
      return (R *c);
   }   
   
   /**
    * Gets the horizontal part of a tile number out of a given position
    * @param[in] lon the longitude to get the tile number for
    * @param[in] z the zoom level to get the tile number for
    * @return the tiles x number
    */
   public static int long2tilex(double lon, int z) 
   { 
      return (int)(Math.floor((lon + 180.0) / 360.0 * Math.pow(2.0, z))); 
   }

   /**
    * Gets the vertical part of a tile number out of a given position
    * @param[in] lat the latitude to get the tile number for
    * @param[in] z the zoom level to get the tile number for
    * @return the tiles y number
    */
   public static int lat2tiley(double lat, int z)
   { 
      return (int)(Math.floor((1.0 - Math.log( Math.tan(lat * Math.PI/180.0) + 1.0 / Math.cos(lat * Math.PI/180.0)) / Math.PI) / 2.0 * Math.pow(2.0, z))); 
   }

   /**
    * Gets the longitude of the side edge of a tile out of a given horizontal
    * tile number
    * @param[in] x the horizontal tile number
    * @param[in] z the zoom level to get the tile number for
    * @return the longitude of the left position of the tile
    */
   public static float tilex2long(int x, int z) 
   {
      return (float)(x / Math.pow(2.0, z) * 360.0-180.0);
   }

   /**
    * Gets the latitude of the upper side of a tile out of a given vertical
    * tile number
    * @param[in] y the vertical tile number
    * @param[in] z the zoom level to get the tile number for
    * @return the latitude of the upper position of the tile
    */
   public static float tiley2lat(int y, int z) 
   {
      double n = Math.PI - 2.0 * Math.PI * y / Math.pow(2.0, z);
      return (float)(180.0 / Math.PI * Math.atan(0.5 * (Math.exp(n) - Math.exp(-n))));
   }
}
