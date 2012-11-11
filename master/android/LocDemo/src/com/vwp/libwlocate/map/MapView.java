package com.vwp.libwlocate.map;

import android.content.*;
import android.graphics.*;
import android.view.*;

public class MapView extends View implements Runnable
{
   private static final int INVALID_POINTER_ID = -1;   

   private int                scrWidth,scrHeight,tileWidth,tileHeight;
   private float              mLastTouchX=0.0f,mLastTouchY=0.0f,mLastTouchX2=0.0f,mLastTouchY2=0.0f,imgOffsetX=0.0f,imgOffsetY=0.0f;
   private Bitmap             locMap=null;
   private Matrix             matrix=new Matrix();
   private Thread             mapThread;
   private boolean            mapThreadRunning,breakMapThread,allowNetAccess;
   private int                mActivePointerId = INVALID_POINTER_ID,mActivePointerId2 = INVALID_POINTER_ID;
   private float              mScaleFactor = 1.0f;
   private GeoUtils           geoUtils;
   private MapOverlay         useOverlay=new MapOverlay();
   
   public MapView(Context ctx,boolean allowNetAccess,int mode)
   {
      super(ctx);

      Display display;

      this.allowNetAccess=allowNetAccess;
            
      if (mode==GeoUtils.MODE_OSM) geoUtils=new GeoUtils(GeoUtils.MODE_OSM);
      else geoUtils=new GeoUtils(GeoUtils.MODE_GMAP);
      
      setBackgroundColor(0xFF555570);
      display=((WindowManager) ctx.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay(); 
      scrWidth=display.getWidth();
      scrHeight=display.getHeight();
      setMinimumHeight(scrWidth);
      setMinimumWidth(scrHeight);
      tileWidth=(int)Math.ceil(scrWidth/256.0)+1;
      tileHeight=(int)Math.ceil(scrHeight/256.0)+1;
   }

   public MapView(Context ctx) throws RuntimeException
   {
      super(ctx);
      throw new RuntimeException("Illegal construction, use MapView(Context,boolean,int) instead!");
   }
   
   public void setOverlay(MapOverlay overlay)
   {
      useOverlay=overlay;
	   if (!mapThreadRunning)
	   {
         mapThread=new Thread(this);
         mapThread.start();
      }
   }
   
   public boolean onTouchEvent(MotionEvent ev) 
   {
      if (mapThreadRunning) return true; // no updates when download is running
      final int action = ev.getAction();
      switch (action & MotionEvent.ACTION_MASK) 
      {
         case MotionEvent.ACTION_DOWN: 
         {
            mLastTouchX=ev.getX();
            mLastTouchY=ev.getY();
            mActivePointerId = ev.getPointerId(0);
            break;            
         }
         case MotionEvent.ACTION_POINTER_DOWN: 
         {
            mLastTouchX2=ev.getX();
            mLastTouchY2=ev.getY();
            if (ev.getPointerCount()>1)
             mActivePointerId2 = ev.getPointerId(1);
            break;
         }
         case MotionEvent.ACTION_MOVE: 
         {
            int   pointerIndex;
            float x=0.0f,y=0.0f;
            
            try
            {
               if ((mActivePointerId!=INVALID_POINTER_ID) || (mActivePointerId2!=INVALID_POINTER_ID))
               {
                  pointerIndex= ev.findPointerIndex(mActivePointerId);
                  x= ev.getX(pointerIndex);
                  y= ev.getY(pointerIndex);
               }
               if ((mActivePointerId!=INVALID_POINTER_ID) && (mActivePointerId2!=INVALID_POINTER_ID))
               {
                  float d1,d2;
                     
                  pointerIndex = ev.findPointerIndex(mActivePointerId2);
                  if (pointerIndex<0) return false;
                  float x2 = ev.getX(pointerIndex);
                  float y2 = ev.getY(pointerIndex);
                                    
                  d1=android.util.FloatMath.sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
                  d2=android.util.FloatMath.sqrt((mLastTouchX-mLastTouchX2)*(mLastTouchX-mLastTouchX2)+
                                      (mLastTouchY-mLastTouchY2)*(mLastTouchY-mLastTouchY2));

/*                  if ((Math.abs(d1)>300) || (Math.abs(d2)>300))
                  {
                     int i=0;
                     i=i;
                  }*/
                  
                  
                  if ((d1>0) && (d2>0))
                  {
                     float w,h,s;
                     
                     s=d1/d2;                     
                     mScaleFactor*=s;
                     matrix.postScale(s,s);
                     w=(scrWidth-(scrWidth*s))/2.0f;
                     h=(scrHeight-(scrHeight*s))/2.0f;
                     
                     matrix.postTranslate(w,h);
                     imgOffsetX+=w;
                     imgOffsetY+=h;                                    
                  }
                  
                  mLastTouchX2 = x2;
                  mLastTouchY2 = y2;
               }
               else if (mScaleFactor==1.0)
               {
                  if ((Math.abs(x-mLastTouchX)>100) || (Math.abs(y-mLastTouchY)>100))
                  {
                     int i=0;
                     i=i;
                  }
                  
                  mScaleFactor=1.0f;
                  imgOffsetX+=(x-mLastTouchX);
                  imgOffsetY+=(y-mLastTouchY);
                  matrix.setTranslate(imgOffsetX,imgOffsetY);
               }
               
               if ((mActivePointerId!=INVALID_POINTER_ID) || (mActivePointerId2!=INVALID_POINTER_ID))
               {             
                  mLastTouchX = x;
                  mLastTouchY = y;
               }
            }
            catch (ArrayIndexOutOfBoundsException aioobe)
            {
               // can be caused by
               // pointerIndex= ev.findPointerIndex(mActivePointerId);
               // x= ev.getX(pointerIndex);
               // above which seems to be a Android bug
            }
            invalidate();
            break;
         }
         case MotionEvent.ACTION_UP: 
         case MotionEvent.ACTION_POINTER_UP: 
         {
            breakMapThread=true;
            mActivePointerId = INVALID_POINTER_ID;
            mActivePointerId2 = INVALID_POINTER_ID;
            restartMap();
            break;
         }
      }      
      return true;
   }
  

   private void restartMap()
   {
      breakMapThread=true;
      postInvalidate();
      if (mapThread!=null) try
      {
         mapThread.join(1500);
      }
      catch (InterruptedException ie)
      {
         
      }
      mapThread=new Thread(this);
      mapThread.start();      
   }


   
   public void setCenterLocation(double lat,double lon)
   {
      breakMapThread=true;
      
      useOverlay.tileX=GeoUtils.long2tilex(lon,useOverlay.m_zoom);
      useOverlay.tileY=GeoUtils.lat2tiley(lat,useOverlay.m_zoom);
      imgOffsetX+=scrWidth/2.0f;
      imgOffsetY+=scrHeight/2.0f;
      restartMap();
   }
   
   
   public void run()
   {
      int        x,y;
      Bitmap     locTile;
      Canvas     canvas;
      float      useLon,useLat,useLonMax,useLatMax;

      System.gc();
      if ((useOverlay.tileX==0) && (useOverlay.tileY==0)) return;
      mapThreadRunning=true;
      try
      {
         if (locMap==null) locMap=Bitmap.createBitmap(tileWidth*256,tileHeight*256,Bitmap.Config.ARGB_8888);
      }
      catch (OutOfMemoryError oome)
      {
         return;
      }
      locMap.eraseColor(Color.BLACK);
      canvas=new Canvas(locMap);
      
      useLon=(GeoUtils.tilex2long(useOverlay.tileX,useOverlay.m_zoom)+GeoUtils.tilex2long(useOverlay.tileX+1,useOverlay.m_zoom))/2.0f;
      useLat=(GeoUtils.tiley2lat(useOverlay.tileY,useOverlay.m_zoom)+GeoUtils.tiley2lat(useOverlay.tileY+1,useOverlay.m_zoom))/2.0f;
      
      while (mScaleFactor>=2.0)
      {
         mScaleFactor/=2.0f;
         useOverlay.m_zoom++;
         if (useOverlay.m_zoom>17)
         {
            useOverlay.m_zoom=17;
            imgOffsetX-=(scrWidth-(scrWidth*2.0))/2.0;
            imgOffsetY-=(scrHeight-(scrHeight*2.0))/2.0;            
         }
      }
      while (mScaleFactor<=0.5)
      {
         mScaleFactor*=2.0f;
         useOverlay.m_zoom--;
         if (useOverlay.m_zoom<4)
         {
            useOverlay.m_zoom=4;
            imgOffsetX-=(scrWidth-(scrWidth*2.0))/2.0;
            imgOffsetY-=(scrHeight-(scrHeight*2.0))/2.0;            
         }
      }
      mScaleFactor=1.0f;

/*      if ((wasScaleOp) && (lonCenter!=0.0f) && (latCenter!=0.0f))
      {
         tileX=GeoUtils.long2tilex(lonCenter,m_zoom);
         tileY=GeoUtils.lat2tiley(latCenter,m_zoom);
         
         tileX-=(scrWidth / 256);
         tileY-=(scrWidth / 256);
      }
      else*/
      {
         useOverlay.tileX=GeoUtils.long2tilex(useLon,useOverlay.m_zoom);
         useOverlay.tileY=GeoUtils.lat2tiley(useLat,useOverlay.m_zoom);
      }
      useLon=GeoUtils.tilex2long(useOverlay.tileX-1,useOverlay.m_zoom);
      useLat=GeoUtils.tiley2lat(useOverlay.tileY-1,useOverlay.m_zoom);
      useLonMax=GeoUtils.tilex2long(useOverlay.tileX+tileWidth+1,useOverlay.m_zoom);
      useLatMax=GeoUtils.tiley2lat(useOverlay.tileY+tileHeight+1,useOverlay.m_zoom);      
      
/*      if (imgOffsetX>scrWidth)
      {
         imgOffsetX=scrWidth/2;
      }
      if (imgOffsetX<=-scrWidth)
      {
         imgOffsetX=-scrWidth/2;         
      }
      if (imgOffsetY>scrHeight)
      {
         imgOffsetY=scrHeight/2;
      }
      if (imgOffsetY<=-scrHeight)
      {
         imgOffsetY=-scrHeight/2;         
      }*/
      
      while (imgOffsetX>0)
      {
         imgOffsetX-=256.0f;
         useOverlay.tileX--;
      }
      while (imgOffsetX<=-256.0)
      {
         imgOffsetX+=256.0f;
         useOverlay.tileX++;
      }
      while (imgOffsetY>0)
      {
         imgOffsetY-=256.0f;
         useOverlay.tileY--;
      }
      while (imgOffsetY<=-256.0)
      {
         imgOffsetY+=256.0f;
         useOverlay.tileY++;
      }
      
      // calculate lat/lon of screen center point
/*      {
         int useTileX=tileX,useTileY=tileY;
         
         x=(int)((scrWidth/2.0f)+imgOffsetX);
         y=(int)((scrHeight/2.0f)+imgOffsetY);
         
         useTileX+=(x / 256);
         x=x % 256;
         useTileY+=(y / 256);
         y=y % 256;

         float tileLon1=GeoUtils.tilex2long(useTileX,m_zoom);
         float tileLat1=GeoUtils.tiley2lat(useTileY,m_zoom);
         float tileLon2=GeoUtils.tilex2long(useTileX+1,m_zoom);
         float tileLat2=GeoUtils.tiley2lat(useTileY+1,m_zoom);
         
         lonCenter=(((tileLon2-tileLon1)/256.0f)*x)+tileLon1;
         latCenter=(((tileLat2-tileLat1)/256.0f)*y)+tileLat1;
      }*/
      
      
      matrix.setTranslate(imgOffsetX,imgOffsetY);
      breakMapThread=false;
      mapThreadRunning=false;
      for (x=0; x<tileWidth; x++)
       for (y=0; y<tileHeight; y++)
      {
         if (breakMapThread) return;
         locTile=geoUtils.loadMapTile(getContext(),useOverlay.tileX+x,useOverlay.tileY+y,useOverlay.m_zoom,allowNetAccess);
         if (locTile!=null) canvas.drawBitmap(locTile,x*256,y*256,null);
         postInvalidate();
      }
      useOverlay.onDraw(canvas,useLon,useLonMax,useLat,useLatMax,locMap);
      postInvalidate();
      System.gc();
   }

   Bitmap getBitmap()
   {
      return locMap;
   }
   
   public void onDraw (Canvas c)
   {
      super.onDraw(c);
      if (locMap!=null) c.drawBitmap(locMap,matrix,null);
   }
      
}
