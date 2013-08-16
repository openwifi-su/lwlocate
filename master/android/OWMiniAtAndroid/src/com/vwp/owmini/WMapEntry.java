package com.vwp.owmini;

import android.preference.PreferenceManager;
import android.view.*;
import android.widget.*;
import android.content.*;
import android.widget.ImageView;


public class WMapEntry
{
   String           BSSID,SSID;
           double   firstLat,firstLon;
   private double   lastLat=0.0,lastLon=0.0,avgLat=0.0,avgLon=0.0;
   private int      avgCtr=0;
   long             lastUpdate;
   TableRow         row;
           TextView latView=null,lonView=null,addInfoView=null;
   private TextView bssidView;
           int      listPos=0;
           int      flags=0;

   static final int FLAG_UI_USED       =0x0001;
   static final int FLAG_IS_VISIBLE    =0x0002;
   static final int FLAG_POS_CHANGED   =0x0004;
   static final int FLAG_IS_OPEN       =0x0008;
   static final int FLAG_IS_NOMAP      =0x0010;   
   static final int FLAG_IS_FREIFUNK   =0x0020;   
   static final int FLAG_IS_FREEHOTSPOT=0x0040;   
   static final int FLAG_IS_THECLOUD   =0x0080;   
   
   public WMapEntry(String BSSID,String SSID,double lat,double lon,int listPos)
   {
      this.BSSID=BSSID;
      this.SSID=SSID;
      this.listPos=listPos;
      firstLat=lat; lastLat=lat;
      firstLon=lon; lastLon=lon;
      addAvgPos(lat,lon);
      lastUpdate=System.currentTimeMillis();
      flags|=FLAG_POS_CHANGED;
   }
//--.
      
   
   void createUIData(Context ctx)
   {
      String showType;
            
      row=new TableRow(ctx);
      row.setGravity(Gravity.LEFT);
      
      TableLayout.LayoutParams tableRowParams=
        new TableLayout.LayoutParams
        (TableLayout.LayoutParams.FILL_PARENT,TableLayout.LayoutParams.WRAP_CONTENT);

      tableRowParams.setMargins(2,2,18,2);
      
      row.setLayoutParams(tableRowParams);
      
      ImageView img=new ImageView(ctx);
      if ((flags & FLAG_IS_FREIFUNK)!=0) img.setImageResource(R.drawable.wifi_frei);
      else if ((flags & FLAG_IS_FREEHOTSPOT)!=0) img.setImageResource(R.drawable.wifi_freehotspot);
      else if ((flags & FLAG_IS_THECLOUD)!=0) img.setImageResource(R.drawable.wifi_cloud);
      else if ((flags & FLAG_IS_OPEN)!=0) img.setImageResource(R.drawable.wifi_open);
      else img.setImageResource(R.drawable.wifi);
      row.addView(img);
      
      TextView cntText=new TextView(ctx);
      OWMiniAtAndroid.setTextStyle(ctx,cntText);
      cntText.setText(listPos+". ");
      row.addView(cntText);
      
      bssidView=new TextView(ctx);
      OWMiniAtAndroid.setTextStyle(ctx,bssidView);
      
      SharedPreferences SP = PreferenceManager.getDefaultSharedPreferences(ctx);
      showType=SP.getString("showType","1");
      if (showType.equalsIgnoreCase("1")) bssidView.setText(BSSID+" ");
      else if (showType.equalsIgnoreCase("2")) bssidView.setText(SSID+" ");
      else if (showType.equalsIgnoreCase("3")) bssidView.setText(SSID+" / "+BSSID+" ");

      row.addView(bssidView);
      
      latView=new TextView(ctx);
      OWMiniAtAndroid.setTextStyle(ctx,latView);
      row.addView(latView,new TableRow.LayoutParams(TableRow.LayoutParams.WRAP_CONTENT));
      
      lonView=new TextView(ctx);
      OWMiniAtAndroid.setTextStyle(ctx,lonView);
      row.addView(lonView,new TableRow.LayoutParams(TableRow.LayoutParams.WRAP_CONTENT));	   

      if ((flags & FLAG_IS_NOMAP)!=0)
      {
         cntText.setTextColor(0xFFFFAAAA);      
         bssidView.setTextColor(0xFFFFAAAA);      
         latView.setTextColor(0xFFFFAAAA);      
         lonView.setTextColor(0xFFFFAAAA);         
      }
      else if ((flags & (FLAG_IS_FREIFUNK|FLAG_IS_FREEHOTSPOT|FLAG_IS_THECLOUD))!=0)
      {
         cntText.setTextColor(0xFFAAFFAA);      
         bssidView.setTextColor(0xFFAAFFAA);      
         latView.setTextColor(0xFFAAFFAA);      
         lonView.setTextColor(0xFFAAFFAA);
      }
      else if ((flags & FLAG_IS_OPEN)!=0)
      {
         cntText.setTextColor(0xFFAAAAFF);      
         bssidView.setTextColor(0xFFAAAAFF);      
         latView.setTextColor(0xFFAAAAFF);      
         lonView.setTextColor(0xFFAAAAFF);
      }
      flags|=FLAG_UI_USED;
   }
   
   
   
   public double getLat()
   {
      double d;
      
      d=avgLat/avgCtr;
      return (firstLat+lastLat+d)/3.0;
   }
   
   
   
   public double getLon()
   {
      double d;
      
      d=avgLon/avgCtr;
      return (firstLon+lastLon+d)/3.0;      
   }
   
   
   
   public boolean posIsValid()
   {
      if ((lastLat==0.0) || (lastLon==0.0)) return false;
      if (Math.abs(lastLat-firstLat)>0.0035) return false;
      if (Math.abs(lastLon-firstLon)>0.0035) return false;
      return true;
   }

   private void addAvgPos(double lat,double lon)
   {
      avgCtr++;
      avgLat+=lat;
      avgLon+=lon;
   }
   
   public void setPos(double lat,double lon)
   {
      lastLat=lat;
      lastLon=lon;
      addAvgPos(lat,lon);
      lastUpdate=System.currentTimeMillis();
      flags|=FLAG_POS_CHANGED;
   }

   
        
}


