package com.vwp.libwlocate.map;

import android.graphics.*;

public class MapOverlay
{
   protected int m_zoom,tileX,tileY; // static to keep previous values
   
   public void doDraw(Canvas canvas,double lonMin,double lonMax,double latMin,double latMax,Bitmap drawMap)
   {

   }
   
   /**
    * Overwrite this method to clean up things and to store e.g. current zoom, tileX and tileY
    * values for later use.
    */
   public void close()
   {
      
   }
}

