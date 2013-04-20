package com.vwp.owmap;

import android.content.*;
import android.app.*;
import android.graphics.*;
import android.graphics.Paint.*;
import android.view.*;

import com.vwp.libwlocate.*;

class HUDView extends ViewGroup 
{
   private Paint           mLoadPaint,mBGPaint;
   private String          value;
   private KeyguardManager myKM;

   public HUDView(Context context) 
   {
       super(context);

       myKM = (KeyguardManager)context.getSystemService(Context.KEYGUARD_SERVICE);
       mLoadPaint = new Paint();
       mLoadPaint.setAntiAlias(true);
       mLoadPaint.setTextSize(105);
       mLoadPaint.setStyle(Style.FILL);
       mLoadPaint.setTypeface(Typeface.DEFAULT_BOLD);
//       mLoadPaint.setTextAlign(Paint.Align.LEFT);
       mLoadPaint.setARGB(240, 217,217,225);
       mBGPaint=new Paint(mLoadPaint);
       mBGPaint.setARGB(200, 40,40,60);
   }

   @Override
   protected void onDraw(Canvas canvas) 
   {
      if (!ScanService.scanData.hudCounter) return;
      if (!myKM.inKeyguardRestrictedInputMode())
      {
         if (ScanService.scanData.appVisible) return;
      }

      super.onDraw(canvas);
      canvas.drawText(value,4,86, mBGPaint);
      canvas.drawText(value, 0,82, mLoadPaint);
   }

   @Override
   protected void onLayout(boolean arg0, int arg1, int arg2, int arg3, int arg4) 
   {
   }

   @Override
   public boolean onTouchEvent(MotionEvent event) 
   {
       return true;
   }
   
   void setValue(int ival)
   {
      value=""+ival;
   }
   
   void setMode(int ival)
   {
      if (ival==loc_info.LOC_METHOD_GPS) mLoadPaint.setARGB(240, 217,217,225);
      else if (ival==loc_info.LOC_METHOD_LIBWLOCATE) mLoadPaint.setARGB(240, 255,255,100);
      else mLoadPaint.setARGB(240, 255,100,100);
   }
}
