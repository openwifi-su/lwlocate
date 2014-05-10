package com.vwp.locdemo;

import android.app.*;
import android.hardware.*;
import android.os.*;
import android.view.*;
import android.view.View.*;
import android.widget.*;
import android.content.*;
import android.content.res.*;
import android.graphics.*;

import java.net.*;
import java.io.*;
import java.util.concurrent.locks.*;
import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.methods.*;
import org.apache.http.impl.client.*;

import com.vwp.libwlocate.*;
import com.vwp.libwlocate.map.*;


/*class MainCanvas extends View 
{
   private Bitmap  locTile[][];
   private int     m_tileX,m_tileY;
   public  int     xOffs,yOffs,lastWidth=0,lastHeight=0;
   private float   m_radius;
   private double  m_lat,m_lon;
   private Paint   circleColour,fillColour;
   private Lock    lock=new ReentrantLock();
   public  int     m_zoom=17;
   private LocDemo locDemo;

   
   
   public MainCanvas(LocDemo locDemo) 
   {
      super(locDemo);
      this.locDemo=locDemo;
      
      circleColour=new Paint();
      circleColour.setARGB(255,255,0,0);
      circleColour.setStyle(Paint.Style.STROKE);
      circleColour.setStrokeWidth(2);

      fillColour=new Paint();
      fillColour.setARGB(75,255,0,0);
      fillColour.setStyle(Paint.Style.FILL);

      updateScreenOrientation();
   }
   
   
   
   private void updateScreenOrientation()
   {
      Display display;

      display=((WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay(); 
      if ((lastWidth==display.getWidth()) && (lastHeight==display.getHeight())) return;
      lastWidth=display.getWidth();
      lastHeight=display.getHeight();
      setMinimumWidth(lastWidth);
      setMinimumHeight(lastHeight);
      lock.lock();
      xOffs=lastWidth/256;
      xOffs=(int)Math.ceil(xOffs/3.0);
      if (xOffs<1) xOffs=1;
      yOffs=lastHeight/256;
      yOffs=(int)Math.ceil(yOffs/3.0);
      if (yOffs<1) yOffs=1;
      locTile=new Bitmap[xOffs*3][yOffs*3];                     
      lock.unlock();
   }
   
      
   private int long2tilex(double lon, int z) 
   { 
      return (int)(Math.floor((lon + 180.0) / 360.0 * Math.pow(2.0, z))); 
   }


   private int lat2tiley(double lat, int z)
   { 
      return (int)(Math.floor((1.0 - Math.log( Math.tan(lat * Math.PI/180.0) + 1.0 / Math.cos(lat * Math.PI/180.0)) / Math.PI) / 2.0 * Math.pow(2.0, z))); 
   }
    

   static double tilex2long(int x, int z) 
   {
      return x / Math.pow(2.0, z) * 360.0 - 180;
   }
    

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
          request.setHeader("User-Agent","LocDemo WLocate demo application");
          HttpResponse response = client.execute(request);
          in=response.getEntity().getContent();
          len=(int)response.getEntity().getContentLength();
          if (len<=0) return false;
          data=new byte[len];
          while (rLen<len)
          {
             rLen+=in.read(data,rLen,len-rLen);
          }
          out=getContext().openFileOutput("tile_"+m_zoom+"_"+(m_tileX+x-xOffs)+"_"+(m_tileY+y-yOffs)+".png",Context.MODE_PRIVATE);
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
   
   
   public void updateViewTiles(double lat,double lon,float radius)
   {
      int             x,y,cnt=0;
      FileInputStream in;
      Message         msg; 
      
      try
      {
         Thread.sleep(250);
      }
      catch (InterruptedException ie)
      {
         
      }
      m_lat=lat;
      m_lon=lon;
      m_radius=radius;
      
      m_tileX=long2tilex(lon,m_zoom);
      m_tileY=lat2tiley(lat,m_zoom);

      lock.lock();
      for (x=-xOffs; x<=xOffs; x++)
       for (y=-yOffs; y<=yOffs; y++)
      {
         cnt++;
         
         msg=new Message();
         msg.arg1=LocDemo.UIHandler.MSG_UPD_PRG_DLG;
         msg.arg2=cnt;
         locDemo.uiHandler.sendMessage(msg);
         
         locTile[x+xOffs][y+yOffs]=null;
         
         try
         {
            in=getContext().openFileInput("tile_"+m_zoom+"_"+(m_tileX+x)+"_"+(m_tileY+y)+".png");
            locTile[x+xOffs][y+yOffs]=BitmapFactory.decodeStream(in);
            in.close();
         }
         catch (IOException ioe)
         {         
            loadTile("http://tiles.virtualworlds.de/tile.php?z="+m_zoom+"&x="+(m_tileX+x)+"&y="+(m_tileY+y),x+xOffs,y+yOffs);
         }
      }
      lock.unlock();
      msg=new Message();
      msg.arg1=LocDemo.UIHandler.MSG_CLOSE_PRG_DLG;
      locDemo.uiHandler.sendMessage(msg);
   }
      
   
   public void onDraw (Canvas c)
   {
      int    x,y;
      float  cx,cy;
      double tileLat1,tileLon1,tileLat2,tileLon2;

      super.onDraw(c);
      if (!lock.tryLock()) return;
      for (x=-xOffs; x<=xOffs; x++)
         for (y=-yOffs; y<=yOffs; y++)
      {
         if (locTile[x+xOffs][y+yOffs]!=null)
         {
            c.drawBitmap(locTile[x+xOffs][y+yOffs],((x+xOffs)*256),(y+yOffs)*256,null);
         }
      }
      
      tileLat1=tiley2lat(m_tileY,m_zoom);
      tileLat2=tiley2lat(m_tileY+1,m_zoom);  
      tileLon1=tilex2long(m_tileX,m_zoom);
      tileLon2=tilex2long(m_tileX+1,m_zoom);
      
      cy=(float)(256*yOffs+(256.0*(m_lat-tileLat1)/(tileLat2-tileLat1)));
      cx=(float)(256*xOffs+(256.0*(m_lon-tileLon1)/(tileLon2-tileLon1)));

      if ((cx!=0) && (cy!=0))
      {
         double zoomFactor;
         float  radius;

         zoomFactor=Math.pow(2.0,(17.0-m_zoom));
         radius=(float)((m_radius*1.193)/zoomFactor);
         if (radius<6) radius=6;
         c.drawCircle(cx,cy,radius,fillColour);
         c.drawCircle(cx,cy,radius,circleColour);
      }
      lock.unlock();
      updateScreenOrientation();
   }
   
}*/



public class LocDemo extends Activity implements OnClickListener
{
   private WLocateReceiver       wLocateRec;
   private MapView               mapView;
   private Button                refreshButton,infoButton;
   public  UIHandler             uiHandler=new UIHandler();
   private MyOverlay             mapOverlay;
   private double                m_lat=0.0,m_lon=0.0;
   private float                 m_radius=-1.0f;
   private static Context        ctx;
   private static ProgressDialog progDlg;
      
   
   class MyOverlay extends MapOverlay
   {
      private Paint circleColour,fillColour;
      
      public MyOverlay()
      {
         m_zoom=17;
         circleColour=new Paint();
         circleColour.setARGB(255,255,0,0);
         circleColour.setStyle(Paint.Style.STROKE);
         circleColour.setStrokeWidth(2);

         fillColour=new Paint();
         fillColour.setARGB(75,255,0,0);
         fillColour.setStyle(Paint.Style.FILL);
      }
      
   
      public void doDraw(Canvas canvas,double lonMin,double lonMax,double latMin,double latMax,Bitmap drawMap)
      {
         if (m_radius<=0) return;
         
         if ((m_lon>=lonMin) && (m_lon<=lonMax) && (m_lat<=latMin) && (m_lat>=latMax))
         {      
            double zoomFactor;
            float  radius;
            float  x,y;
            
            int useTileX=GeoUtils.long2tilex(m_lon,m_zoom);
            int useTileY=GeoUtils.lat2tiley(m_lat,m_zoom);
   
            float tileLon1=GeoUtils.tilex2long(useTileX,m_zoom);
            float tileLat1=GeoUtils.tiley2lat(useTileY,m_zoom);
            float tileLon2=GeoUtils.tilex2long(useTileX+1,m_zoom);
            float tileLat2=GeoUtils.tiley2lat(useTileY+1,m_zoom);
   
            y=(int)((useTileY-tileY)*256+(256.0*(m_lat-tileLat1)/(tileLat2-tileLat1)));
            x=(int)((useTileX-tileX)*256+(256.0*(m_lon-tileLon1)/(tileLon2-tileLon1)));
   
            zoomFactor=Math.pow(2.0,(17.0-m_zoom));
            radius=(float)((m_radius*1.2)/zoomFactor);
            if (radius<6) radius=6;
            canvas.drawCircle(x,y,radius,fillColour);
            canvas.drawCircle(x,y,radius,circleColour);
         }

      }
      
   }
   
   
   static class UIHandler extends Handler
   {      
      public static final int MSG_OPEN_PRG_DLG=2;
      public static final int MSG_CLOSE_PRG_DLG=3;
      public static final int MSG_POSITION_FAILED=5;
      
      public void handleMessage(Message msg) 
      {
         switch (msg.arg1)
         {
            case MSG_OPEN_PRG_DLG:
            {
               progDlg=new ProgressDialog(ctx);
               progDlg.setCancelable(false);
               progDlg.setCanceledOnTouchOutside(false);
               progDlg.setTitle((String)msg.obj);
               progDlg.show();         
               break;
            }
            case MSG_CLOSE_PRG_DLG:
            {
//               mainCanvas.invalidate();
               if (progDlg==null) return;
               progDlg.dismiss();
               progDlg=null;
               break;
            }
            case MSG_POSITION_FAILED:
            {
               if (progDlg!=null)
               {
                  progDlg.dismiss();
                  progDlg=null;
               }
               AlertDialog ad = new AlertDialog.Builder(ctx).create();  
               ad.setCancelable(false);
               ad.setMessage("Current position could not be evaluated!");  
               ad.setButton("OK", new DialogInterface.OnClickListener() {  
                   @Override  
                   public void onClick(DialogInterface dialog, int which) {  
                       dialog.dismiss();                      
                   }  
               });  
               ad.show();           
            }
            default:
               break;
         }            
      }     
   }
   
      
   
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {    	
        super.onCreate(savedInstanceState);
        ctx=this;
        setTitle("LocDemo - free and open location based service demo");
        wLocateRec=new WLocateReceiver(this);

        mapView=new MapView(this,true,GeoUtils.MODE_OSM);
        mapOverlay=new MyOverlay();
        mapView.setOverlay(mapOverlay);

        setContentView(R.layout.main);
        
        
        FrameLayout mainLayout=(FrameLayout)findViewById(R.id.rootLayout);
        mainLayout.addView(mapView);
        
        LinearLayout navButtons = new LinearLayout (this);


        refreshButton = new Button(this);
        refreshButton.setText("Refresh Position");
        refreshButton.setOnClickListener(this);

        infoButton = new Button(this);
        infoButton.setText("About LocDemo");
        infoButton.setOnClickListener(this);

        navButtons.addView(refreshButton);
        navButtons.addView(infoButton);
        mainLayout.addView(navButtons);        
                                
        Message msg;

        msg=new Message();
        msg.arg1=LocDemo.UIHandler.MSG_OPEN_PRG_DLG;
        msg.obj="Evaluating position / Loading map tiles...";
        uiHandler.sendMessage(msg);
        
        wLocateRec.wloc_request_position(0);
    }
   
    
    
   public void onResume() 
   {
       super.onResume();
   }
   
   

   public void onPause() 
   {
       super.onPause();
   }    
    
        
    
    public void onBackButton()
    {
       finish();
       System.exit(0);
    }
    
    
    public void onClick(View v)
    {
       if (v==refreshButton)
       {
          Message msg;

          msg=new Message();
          msg.arg1=LocDemo.UIHandler.MSG_OPEN_PRG_DLG;
          msg.obj="Loading map tiles...";
          uiHandler.sendMessage(msg);

          wLocateRec.wloc_request_position(0);
       }
       else if (v==infoButton)
       {
          AlertDialog ad = new AlertDialog.Builder(this).create();  
          ad.setCancelable(false);
          ad.setMessage("LocDemo Version 2.0 is (c) 2012-2014 by Oxy/VWP\nIt demonstrates the usage of WLocate which does NOT use the Google(tm) services and is available under the terms of the GNU Public License\nFor more details please refer to http://www.openwlanmap.org");  
          ad.setButton("OK", new DialogInterface.OnClickListener() {  
              @Override  
              public void onClick(DialogInterface dialog, int which) {  
                  dialog.dismiss();                      
              }  
          });  
          ad.show();           
       }
       else
       {
          Message msg;

          msg=new Message();
          msg.arg1=LocDemo.UIHandler.MSG_OPEN_PRG_DLG;
          msg.obj="Loading map tiles...";
          uiHandler.sendMessage(msg);
       }
    }
    
    

    class WLocateReceiver extends WLocate
    {
       
       public WLocateReceiver(Context ctx)
       {
          super(ctx);   
       }
       
       
       
       protected void wloc_return_position(int ret,double lat,double lon,float radius,short ccode)
       {
          Message msg;

          msg=new Message();
          msg.arg1=LocDemo.UIHandler.MSG_CLOSE_PRG_DLG;
          uiHandler.sendMessage(msg);

          if (ret==WLocate.WLOC_OK)
          {
             mapView.setCenterLocation(lat,lon);
             m_lat=lat;
             m_lon=lon;
             m_radius=radius;
          }
          else
          {
             msg.arg1=LocDemo.UIHandler.MSG_POSITION_FAILED;
             uiHandler.sendMessage(msg);
          }
       }       
    }

}


