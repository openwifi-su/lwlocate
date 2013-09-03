package com.vwp.locdemo;

import com.vwp.libwlocate.*;
import com.vwp.libwlocate.map.*;

import android.app.*;
import android.os.*;
import android.view.*;
import android.view.View.*;
import android.widget.*;
import android.content.*;
import android.graphics.*;


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
          ad.setMessage("LocDemo Version 1.2 is (c) 2013 by Oxy/VWP\nIt demonstrates the usage of WLocate which does NOT use the Google(tm) services and is available under the terms of the GNU Public License\nFor more details please refer to http://www.openwlanmap.org");  
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


