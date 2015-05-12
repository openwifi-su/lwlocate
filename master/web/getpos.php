<?
include "inc/config.php";
include "inc/std_functions.php";

$link = mysql_connect(DB_HOST,DB_USER,DB_PASSWORD);
if (!mysql_select_db(DB_NAME)) $errortext=mysql_error();

$bssid=array();
$src=array();
$lat=array();
$lon=array();

$rlat=0.0;
$rlon=0.0;
$rfound=0.0;
$rcnt=0;

$max=0;
$FHandle=fopen("php://input","r");
//$FHandle=fopen("bssid.txt","r");
if ($FHandle)
{
   while(1)
   {  
      $bssid[$max]=strtoupper(trim(fgets($FHandle)));
      if (strlen($bssid[$max])<12) break;
      $bssid[$max]=str_replace(":","",$bssid[$max]);
      $bssid[$max]=strtoupper($bssid[$max]);
      
      $query="SELECT * FROM netpoints WHERE bssid='".$bssid[$max]."'";
      $entry=fetchByQuery($query);
      if (($entry) && ($entry['errcnt']<30))
      {
         $lat[$max]=$entry['lat'];
         $lon[$max]=$entry['lon'];
         $src[$max]=$entry['source'];

//echo $lat[$max]."  ".$lon[$max]."\n";

         if (($lat[$max]!=0) && ($lon[$max]!=0))
         {
            if ($src[$max]==0) // manually entered - best quality
            {
               $rlat=$rlat+($lat[$max]*12);
               $rlon=$rlon+($lon[$max]*12);
               $rfound=$rfound+12.0;
            }
            else if ($src[$max]==3) // interpolated from other positions
            {
               $rlat=$rlat+($lat[$max]*3);
               $rlon=$rlon+($lon[$max]*3);
               $rfound=$rfound+3.0;
            }
            else
            {
               $rlat=$rlat+($lat[$max]*5);
               $rlon=$rlon+($lon[$max]*5);
               $rfound=$rfound+5.0;
            }
            $rcnt++; // absolute number of results
         }
         else $src[$max]=-4; // disabled AP, so ignore it
      }
      else if (($entry) && ($entry['errcnt']>=10)) $src[$max]=-2; // error counter too big, delete at end
      else $src[$max]=-1; // no position found, write to database at end
      $max++;
   }
   if ($rcnt==0)
   {
      $ip = $_SERVER['REMOTE_ADDR'];
      $details = json_decode(file_get_contents("http://ipinfo.io/{$ip}/json"));

      $pos=strpos($details->loc,",");
      if (($details->loc) && ($pos>0))
      {
         $lat=substr($details->loc,0,$pos);
         $lon=substr($details->loc,$pos+1);

         echo "result=1\r\n";
         echo "quality=0\r\n";
         echo "lat=".$lat."\r\n";
         echo "lon=".$lon."\r\n";
         exit;
      }
   }
   if (($max<2) || ($rcnt==0)) echo "result=0\r\n";
   else
   {
      $rlat=$rlat/$rfound;
      $rlon=$rlon/$rfound;
      
      if ($rcnt>=2) // more than two points so we can check if one is far away from others
      {
         $maxdist=0.0;
         $maxdistpos=-1;
         for ($i=0; $i<$rcnt; $i++) if ($src[$i]>=0)
         {
            if ((abs($lat[$i]-$rlat)>0.004) && (abs($lat[$i]-$rlat)>$maxdist))
            {
               $maxdist=abs($lat[$i]-$rlat);
               $maxdistpos=$i;
            }
            if ((abs($lon[$i]-$rlon)>0.004) && (abs($lon[$i]-$rlon)>$maxdist))
            {
               $maxdist=abs($lon[$i]-$rlon);
               $maxdistpos=$i;
            }
         }
         if ($maxdistpos>=0) // one is out of range so remove it and do a new calculation
         {
//echo "out of range: $maxdistpos\r\n";         
            $src[$maxdistpos]=-3; // increase the error counter
            $rlat=0.0;
            $rlon=0.0;
            $rfound=0.0;
            
            for ($i=0; $i<$rcnt; $i++)
            {
               if ($src[$i]>=0) // valid position
               {
                  if ($src[$i]==0) // manually entered - best quality
                  {
                     $rlat=$rlat+($lat[$i]*12);
                     $rlon=$rlon+($lon[$i]*12);
                     $rfound=$rfound+12.0;
                  }
                  else if ($src[$i]==3) // interpolated from other positions
                  {
                     $rlat=$rlat+($lat[$i]*3);
                     $rlon=$rlon+($lon[$i]*3);
                     $rfound=$rfound+3.0;
                  }
                  else
                  {
                     $rlat=$rlat+($lat[$i]*5);
                     $rlon=$rlon+($lon[$i]*5);
                     $rfound=$rfound+5.0;
                  }
               }
            }
            $rlat=$rlat/$rfound;
            $rlon=$rlon/$rfound;
         }
      }
      
      echo "result=1\r\n";
      if ($rfound+$rcnt>90) echo "quality=90\r\n";
      else echo "quality=".($rfound+$rcnt)."\r\n";
      echo "lat=".$rlat."\r\n";
      echo "lon=".$rlon."\r\n";

      if ($rcnt>2) for ($i=0; $i<$max; $i++) // enough results so we can write back unknown BSSIDs
      {
         if ($src[$i]==-1)
         {
            $query="INSERT INTO netpoints (bssid, lat, lon, timestamp, source) VALUES ('$bssid[$i]', $rlat, $rlon, ".time().", 3)";
            setByQuery($query);
         }
         else if ($src[$i]==-2)
         {
            $query="DELETE FROM netpoints WHERE bssid='$bssid[$i]'";
            setByQuery($query);
         }
         else if ($src[$i]==-3)
         {
            $query="UPDATE netpoints SET errcnt=errcnt+1 WHERE bssid='$bssid[$i]'";
            setByQuery($query);
         }
      }
   }
   fclose($FHandle);
}

?>
