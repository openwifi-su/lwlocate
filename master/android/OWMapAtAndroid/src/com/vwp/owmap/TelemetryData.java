package com.vwp.owmap;

public class TelemetryData
{
   float accelX,accelY,accelZ;
   float orientX,orientY,orientZ;
   float corrAccelX=0.0f,corrAccelY=0.0f,corrAccelZ=0.0f;
   float corrOrientY=0.0f,corrOrientZ=0.0f;
   float accelMax=9.81f;
   int   accelCnt,orientCnt;

   TelemetryData()
   {
      reset();
   }
   
   void setAccelMax(float max)
   {
      accelMax=max;
   }
   
   
   void addAccel(float x,float y,float z)
   {
      accelX+=x;
      accelY+=y;
      accelZ+=z;
      accelCnt++;
   }

   
   void corrAccel(float x,float y,float z)
   {
      corrAccelX+=x;
      corrAccelY+=y;
      corrAccelZ+=z;
   }
   
   
   void addOrient(float x,float y,float z)
   {
      orientX+=x;
      orientY+=y;
      orientZ+=z;
      orientCnt++;
   }

   
   void corrOrient(float y,float z)
   {
      corrOrientY+=y;
      corrOrientZ+=z;
   }
   
   
   void set(TelemetryData data)
   {
      accelMax=data.accelMax;
      accelX=(data.accelX/data.accelCnt)-data.corrAccelX;
      accelY=(data.accelY/data.accelCnt)-data.corrAccelY;
      accelZ=(data.accelZ/data.accelCnt)-data.corrAccelZ;      
      accelCnt=1;
      orientX=data.orientX/data.orientCnt;
      orientY=(data.orientY/data.orientCnt)-data.corrOrientY;
      orientZ=(data.orientZ/data.orientCnt)-data.corrOrientZ;
      orientCnt=1;
   }
   
   void reset()
   {
      accelX=0.0f;  accelY=0.0f;  accelZ=0.0f;
      accelCnt=0;      
      orientX=0.0f; orientY=0.0f; orientZ=0.0f;
      orientCnt=0;      
   }

}
