package com.vwp.owmap;

import android.content.*;

public class BootUpReceiver extends BroadcastReceiver
{

   @Override
   public void onReceive(Context context, Intent intent) 
   {            
      Intent i = new Intent(context, OWMapAtAndroid.class);  
      i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      i.putExtra("autostarted",true);
      context.startActivity(i);  
   }

}
