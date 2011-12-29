package com.vwp;

import android.app.*;
import android.os.*;
import android.view.*;
import android.widget.*;
import android.net.wifi.*;
import java.util.*;
import java.net.*;
import java.io.*;
import android.content.*;
import android.graphics.*;
import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.methods.*;
import org.apache.http.impl.client.*;


class MainCanvas extends View //implements SurfaceHolder.Callback 
{
   private Bitmap locTile[][]=new Bitmap[3][3];
   private int    m_tileX,m_tileY,m_zoom=17,m_quality;
   private double m_lat,m_lon;
   private Paint  circleColour;

   
   
   public MainCanvas(Context context) 
   {
      super(context);
      
      circleColour=new Paint();
      circleColour.setARGB(255,255,0,0);
      circleColour.setStyle(Paint.Style.STROKE);
      circleColour.setStrokeWidth(3);
   }
   
   
   
   /**
    * Gets the horizontal part of a tile number out of a given position
    * @param[in] lon the longitude to get the tile number for
    * @param[in] z the zoom level to get the tile number for
    * @return the tiles x number
    */
   private int long2tilex(double lon, int z) 
   { 
      return (int)(Math.floor((lon + 180.0) / 360.0 * Math.pow(2.0, z))); 
   }



   /**
    * Gets the vertical part of a tile number out of a given position
    * @param[in] lat the latitude to get the tile number for
    * @param[in] z the zoom level to get the tile number for
    * @return the tiles y number
    */
   private int lat2tiley(double lat, int z)
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
   static double tilex2long(int x, int z) 
   {
      return x / Math.pow(2.0, z) * 360.0 - 180;
   }
    


   /**
    * Gets the latitude of the upper side of a tile out of a given vertical
    * tile number
    * @param[in] y the vertical tile number
    * @param[in] z the zoom level to get the tile number for
    * @return the latitude of the upper position of the tile
    */
   static double tiley2lat(int y, int z) 
   {
      double n = Math.PI - 2.0 * Math.PI * y / Math.pow(2.0, z);
      return 180.0 / Math.PI * Math.atan(0.5 * (Math.exp(n) - Math.exp(-n)));
   }

   
   
   private boolean loadTile(String url,int x,int y)
   {
      InputStream      in;
      FileOutputStream out;
      byte             data[];
      int              len,rLen=0;

      try 
      {
          HttpClient client = new DefaultHttpClient();
          HttpGet request = new HttpGet();
          request.setURI(new URI(url));
          HttpResponse response = client.execute(request);
          in=response.getEntity().getContent();
          len=(int)response.getEntity().getContentLength();
          if (len<=0) return false;
          data=new byte[len];
          while (rLen<len)
          {
             rLen+=in.read(data,rLen,len-rLen);
          }
          out=getContext().openFileOutput("tile_"+m_zoom+"_"+(m_tileX+x-1)+"_"+(m_tileY+y-1)+".png",Context.MODE_PRIVATE);
          out.write(data);
          out.close();
          locTile[x][y]=BitmapFactory.decodeByteArray(data,0,rLen);
          in.close();
      }
      catch (Exception e)
      {
         e.printStackTrace();
         return false;
      }
      return true;
   }
   
   /**
    * This method updates the internal locTile array that holds bitmaps of the tiles that have to be
    * displayed currently. To get the tile images it first tries to load a local PNG image. In case
    * that fails the TAH server is connected to download and save a tile image in PNG format. Then it
    * tries again to load the local PNG image - now it should be successful because it was downloaded
    * just one step before.
    * @param[in] lat the latitude of the current position which has to be displayed in center tile
    * @param[in] lon the longitude of the current position which has to be displayed in center tile
    */
   public void updateTiles(double lat,double lon,int quality)
   {
      int        x,y;
      FileInputStream in;

      m_lat=lat;
      m_lon=lon;
      m_quality=quality;
      
      m_tileX=long2tilex(lon,m_zoom);
      m_tileY=lat2tiley(lat,m_zoom);

      for (x=-1; x<=1; x++)
       for (y=-1; y<=1; y++)
      {
         locTile[x+1][y+1]=null;
         
         try
         {
            in=getContext().openFileInput("tile_"+m_zoom+"_"+(m_tileX+x)+"_"+(m_tileY+y)+".png");
            locTile[x+1][y+1]=BitmapFactory.decodeStream(in);
            in.close();
         }
         catch (IOException ioe)
         {         
            loadTile("http://tah.openstreetmap.org/Tiles/tile/"+m_zoom+"/"+(m_tileX+x)+"/"+(m_tileY+y)+".png",x+1,y+1);
         }
      }
   }
   
   
   
   public void onDraw (Canvas c)
   {
      int    x,y;
      float  cx,cy;
      double tileLat1,tileLon1,tileLat2,tileLon2;
      
      for (x=-1; x<=1; x++)
         for (y=-1; y<=1; y++)
      {
         if (locTile[x+1][y+1]!=null)
         {
            c.drawBitmap(locTile[x+1][y+1],((x+1)*256),(y+1)*256,null);
         }
      }
      
      tileLat1=tiley2lat(m_tileY,m_zoom);
      tileLat2=tiley2lat(m_tileY+1,m_zoom);  
      tileLon1=tilex2long(m_tileX,m_zoom);
      tileLon2=tilex2long(m_tileX+1,m_zoom);
      
      cy=(float)(256+(256.0*(m_lat-tileLat1)/(tileLat2-tileLat1)));
      cx=(float)(256+(256.0*(m_lon-tileLon1)/(tileLon2-tileLon1)));

      if ((cx!=0) && (cy!=0))
      {
         double zoomFactor;

         if (m_quality>0)
         {
            zoomFactor=Math.pow(2.0,(17.0-m_zoom));
            c.drawCircle(cx,cy,(float)((120-m_quality)/zoomFactor),circleColour);
         }
         else
         {
            zoomFactor=Math.pow(2.0,(10.0-m_zoom));
            c.drawCircle(cx,cy,(float)(130/zoomFactor),circleColour);
         }
      }
      
   }
   
}



public class LocDemo extends Activity 
{
   private WLocateReceiver wLocateRec=new WLocateReceiver();
   private MainCanvas      mainCanvas;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
    	
        super.onCreate(savedInstanceState);
        mainCanvas=new MainCanvas(this);
        setContentView(mainCanvas);
/*        tv = new TextView(this);
        
        setContentView(tv);*/
        
        wLocateRec.wloc_request_position(this);
    }

    class WLocateReceiver extends WLocate
    {
       protected void wloc_return_position(int ret,double lat,double lon,int quality,short ccode)
       {
          if (ret==WLocate.WLOC_OK)
          {
             mainCanvas.updateTiles(lat,lon,quality);             
          }
       }       
    }

}


