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


class MainCanvas extends View 
{
   private Bitmap  locTile[][];
   private int     m_tileX,m_tileY,m_lastOrientation=-1;
   public  int     xOffs,yOffs;
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
   
   
   
   public void updateScreenOrientation()
   {
      Display display;
      int     orientation;

      display=((WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay(); 
      orientation=display.getOrientation();
      lock.lock();
      xOffs=display.getWidth()/256;
      xOffs=(int)Math.ceil(xOffs/3.0);
      yOffs=display.getHeight()/256;
      yOffs=(int)Math.ceil(yOffs/3.0);
      if (orientation!=m_lastOrientation)
      {
         m_lastOrientation=orientation;
         if (m_lastOrientation==Configuration.ORIENTATION_PORTRAIT)
         {
            int swp;
            
            swp=xOffs; 
            xOffs=yOffs; 
            yOffs=swp;
         }
         locTile=new Bitmap[xOffs*3][yOffs*3];                     
      }
      lock.unlock();
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
   
   
      
   /**
    * This method updates the internal locTile array that holds bitmaps of the tiles that have to be
    * displayed currently. To get the tile images it first tries to load a local PNG image. In case
    * that fails the TAH server is connected to download and save a tile image in PNG format. Then it
    * tries again to load the local PNG image - now it should be successful because it was downloaded
    * just one step before.
    * @param[in] lat the latitude of the current position which has to be displayed in center tile
    * @param[in] lon the longitude of the current position which has to be displayed in center tile
    */
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
            loadTile("http://otile1.mqcdn.com/tiles/1.0.0/osm/"+m_zoom+"/"+(m_tileX+x)+"/"+(m_tileY+y)+".png",x+xOffs,y+yOffs);
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
   }
   
}



public class LocDemo extends Activity implements OnClickListener,SensorEventListener,Runnable 
{
   private WLocateReceiver wLocateRec;
   private MainCanvas      mainCanvas;
   private Button          zoomInButton,zoomOutButton,refreshButton,infoButton;
   private SensorManager   mSensorManager;
   private Sensor          mAccelerometer;
   private double          lat,lon;
   private float           radius;
   private Thread          updateThread;
   private boolean         updateRunning=false;
   public  UIHandler       uiHandler=new UIHandler();
   private Context         ctx;
   private ProgressDialog  progDlg;
      
   
   class UIHandler extends Handler
   {
      public static final int MSG_OPEN_PRG_DLG=2;
      public static final int MSG_CLOSE_PRG_DLG=3;
      public static final int MSG_UPD_PRG_DLG=4;
      
      public void handleMessage(Message msg) 
      {
         switch (msg.arg1)
         {
            case MSG_OPEN_PRG_DLG:
            {
               progDlg=new ProgressDialog(ctx);
               progDlg.setTitle((String)msg.obj);
               progDlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
               progDlg.setMax(msg.arg2);
               progDlg.show();         
               break;
            }
            case MSG_UPD_PRG_DLG:
            {
               progDlg.setProgress(msg.arg2);
               break;
            }
            case MSG_CLOSE_PRG_DLG:
            {
               mainCanvas.invalidate();
               if (progDlg==null) return;
               progDlg.dismiss();
               progDlg=null;
               break;
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
        mainCanvas=new MainCanvas(this);
        
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);      
        
        FrameLayout mainLayout = new FrameLayout(this);

        LinearLayout navButtons = new LinearLayout (this);

        zoomInButton = new Button(this);
        zoomInButton.setText("+");
        zoomInButton.setWidth(60);
        zoomInButton.setEnabled(false);
        zoomInButton.setOnClickListener(this);
        
        zoomOutButton = new Button(this);
        zoomOutButton.setText("-");
        zoomOutButton.setWidth(60);
        zoomOutButton.setOnClickListener(this);

        refreshButton = new Button(this);
        refreshButton.setText("Refresh Position");
        refreshButton.setOnClickListener(this);
        
        infoButton = new Button(this);
        infoButton.setText("About LocDemo");
        infoButton.setOnClickListener(this);
                
        navButtons.addView(zoomInButton);       
        navButtons.addView(zoomOutButton);       
        navButtons.addView(refreshButton);
        navButtons.addView(infoButton);
        mainLayout.addView(mainCanvas);
        mainLayout.addView(navButtons);
        setContentView(mainLayout);        
        
        wLocateRec.wloc_request_position(0);
    }
   
    
    
   public void onResume() 
   {
       super.onResume();
       mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
   }
   
   

   public void onPause() 
   {
       super.onPause();
       mSensorManager.unregisterListener(this);
   }    
    
    
    public void onAccuracyChanged(Sensor sensor, int accuracy) 
    {
    }
    
    
    
    public void onSensorChanged(SensorEvent event) 
    {
       if (event.sensor.getType()==Sensor.TYPE_ORIENTATION) mainCanvas.updateScreenOrientation();
    }

    
    
    private synchronized void updateTiles(double lat,double lon,float radius)
    {
       if (updateRunning) return;
       updateRunning=true;
       this.lat=lat;
       this.lon=lon;
       this.radius=radius;
       updateThread=new Thread(this);
       updateThread.start();
    }
    
    
    
    public void run()
    {    
       Message msg;

       msg=new Message();
       msg.arg1=LocDemo.UIHandler.MSG_OPEN_PRG_DLG;
       msg.arg2=(mainCanvas.xOffs*2+1)*(mainCanvas.yOffs*2+1);
       msg.obj="Loading map tiles...";
       uiHandler.sendMessage(msg);
       
       mainCanvas.updateViewTiles(lat,lon,radius);
       updateRunning=false;
    }
    
    
    
    public void onBackButton()
    {
       finish();
       System.exit(0);
    }
    
    
    public void onClick(View v)
    {
       if (v==refreshButton) wLocateRec.wloc_request_position(0);
       else if (v==infoButton)
       {
          AlertDialog ad = new AlertDialog.Builder(this).create();  
          ad.setCancelable(false);  
          ad.setMessage("LocDemo Version 0.9 is (c) 2012 by Oxy/VWP\nIt demonstrates the usage of WLocate which does NOT use the Google(tm) services and is available under the terms of the GNU Public License\nFor more details please refer to http://www.openwlanmap.org");  
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
          if (v==zoomInButton) mainCanvas.m_zoom++;
          else if (v==zoomOutButton) mainCanvas.m_zoom--;
          if (mainCanvas.m_zoom>=17)
          {
             mainCanvas.m_zoom=17;
             zoomInButton.setEnabled(false);
          }
          else zoomInButton.setEnabled(true);
          if (mainCanvas.m_zoom<=3)
          {
             mainCanvas.m_zoom=3;
             zoomOutButton.setEnabled(false);
          }
          else zoomOutButton.setEnabled(true);
          updateTiles(lat,lon,radius);
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
          if (ret==WLocate.WLOC_OK)
          {
             updateTiles(lat,lon,radius);
          }
       }       
    }

}


