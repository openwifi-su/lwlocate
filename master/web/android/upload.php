<?

include_once "geoutils.php";
include_once "../inc/config.php";

$userid=0;
$usertag="";
$teamid="";
$userflags=0;
$mapCreated=0;

$link = mysql_connect(DB_HOST,DB_USER,DB_PASSWORD); // <---- set username and password here
if (!mysql_select_db(DB_NAME,$link)) echo mysql_error();

$minLat=1000;  $minLon=1000;
$maxLat=-1000; $maxLon=-1000;

$newAPs=0;
$updAPs=0;
$delAPs=0;
$newPoints=0;

$FHandle=fopen("php://input","r");
if ($FHandle)
{
   $idbssid=strtoupper(trim(fgets($FHandle)));
   $teamid=$idbssid;
   // first line contains unique identifier
   $query="SELECT idx FROM users WHERE bssid='$idbssid'";
   $res=mysql_db_query("wmap",$query);
   if ((!$res) || (mysql_num_rows($res)<=0))
   {
      $query="INSERT INTO users (bssid, count) VALUES ('".$idbssid."', '0')";
      mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }
      $query="SELECT idx FROM users WHERE bssid='$idbssid'";
      $res=mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }
   }
   $result=mysql_fetch_array($res);
   $userid=$result[0];
if ($userid==2028) exit(0);
   
   $line=fgets($FHandle);
   while (strlen($line)>0)
   {
      $bssid="";
      $line=str_replace(",",".",$line);
      
      sscanf($line,"%s\t%s\t%f\t%f",$id,$bssid,$latitude,$longitude);
//if (strlen($bssid)<12) exit(0);

      $idnum=intval($id);
      if ($id=='o')
      {
         // no longer storing open WLANs
      }
      else if ($id=="U")
      {
         // store freifunk network
         $query="UPDATE netpoints SET flags='64' WHERE bssid='$bssid'";
         mysql_db_query("wmap",$query);       
      }
/*      else if ($id=="D")
      {
         $newPoints=$newPoints+intval($bssid);
         $query="UPDATE users SET count=count+".intval($bssid)." WHERE idx='$userid'";
         mysql_db_query("wmap",$query);
         if (mysql_errno()!=0)
         {
            echo mysql_error();
            exit;
         }      
      }*/
      else if ($id=="E") $teamid=$bssid;
      else if ($id=="F") $userflags=$bssid;
      else if ($id=="L")
      {
         $query="UPDATE users SET lastupload='".time()."', lastlat='".$latitude."', lastlon='".$longitude."' WHERE idx='$userid'";
         mysql_db_query("wmap",$query);
         exit;
      }
      else if ($id=="T") $usertag=$bssid;
      else if ($idnum>0)
      {
         if ($idnum>5) $idnum=5;
         $bssid=strtoupper(str_replace(":","",$bssid));

         if ((strlen($bssid)==12) && 
             ($latitude<=90) && ($latitude>=-90) && 
             ($longitude<=180) && ($longitude>=-180))
         {
            $query="SELECT * FROM netpoints where bssid='$bssid'";
            $res=mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }


            if (($latitude!=0) && ($longitude!=0))
            {
               if ($minLat>$latitude) $minLat=$latitude;
               if ($maxLat<$latitude) $maxLat=$latitude;
               if ($minLon>$longitude) $minLon=$longitude;
               if ($maxLon<$longitude) $maxLon=$longitude;
            }

            if ((!$res) || (mysql_num_rows($res)<=0))
            {
               $query="DELETE FROM netpoints where bssid='".strtoupper($bssid)."'"; // delete in case it exists as interpolated position
               $res=mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }
               
         
               $query="INSERT INTO netpoints (bssid, lat, lon, timestamp, source, userid) VALUES ('".strtoupper($bssid)."', '$latitude', '$longitude', '".time()."', '7', '$userid')";
               mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }


               $newAPs++;
               $newPoints=$newPoints+$idnum;
               $query="UPDATE users SET count=count+".$idnum." WHERE idx='$userid'";
               mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }

            }
            else
            {
               $result=mysql_fetch_array($res);
               if (($result) /*&& ($result['source']!=0)*/ &&
                   ((abs($result['lat']-$latitude)>0.003) || (abs($result['lon']-$longitude)>0.003) || 
                    ($result['source']==3) || ($result['source']==4) || ($result['source']==5)))
               {
                  $query="DELETE FROM netpoints WHERE bssid='".strtoupper($bssid)."'";
                  mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }


                  $query="INSERT INTO netpoints (bssid, lat, lon, timestamp, source, userid) VALUES ('".strtoupper($bssid)."', '$latitude', '$longitude', '".time()."', '7', '$userid')";
                  mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }


                  if (($latitude==0.0) && ($longitude==0.0)) $delAPs++;
                  $updAPs++;
                  $newPoints=$newPoints+$idnum;
                  $query="UPDATE users SET count=count+".$idnum." WHERE idx='$userid'";
                  mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }

               }
               else if (($result) && ($result['lat']!=0) && ($result['lon']!=0) &&
                        (($result['source']==3) || ($result['source']==4) || ($result['source']==5) || 
                         ($result['source']==6) || ($result['source']==7)))
               {
                  $newlat=(($result['lat']*10.0)+$latitude)/11.0;
                  $newlon=(($result['lon']*10.0)+$longitude)/11.0;
                     
                  $query="UPDATE netpoints SET lat='$newlat', lon='$newlon', timestamp=".time()." WHERE bssid='".strtoupper($bssid)."'";
                  mysql_db_query("wmap",$query);
      if (mysql_errno()!=0)
      {
         echo mysql_error();
         exit;
      }
  
                  $updAPs++;
               }
            }
         }
         $query="UPDATE netpoints SET flags=0 WHERE bssid='$bssid'";
         mysql_db_query("wmap",$query);       
         if (($latitude==0) and ($longitude==0))
         {
            $query="UPDATE netpoints SET source=0 where bssid='$bssid'";
            mysql_db_query("wmap",$query);       
         }
      }
     $line=fgets($FHandle);
   }
}
fclose($FHandle);

$teamid_arr=array();
$tag_arr=array();
$points_arr=array();
$team_members=array();

$query="SELECT teamid, tag, SUM(count), COUNT(*) AS c FROM users WHERE teamid!='' GROUP BY teamid HAVING c>1";
$res=mysql_db_query("wmap",$query);
$entries=mysql_num_rows($res);
$FHandle=fopen("teamlist.php","w");
if (($entries>0) && ($FHandle!=0))
{
   for ($i=0; $i<$entries; $i++)
   {
      $data=mysql_fetch_array($res);
      $teamid_arr[$i]=$data['teamid'];
      $tag_arr[$i]=strip_tags($data['tag']);
      $points_arr[$i]=$data[2];
      $team_members[$i]=$data[3];
   }
   $rankcnt=0;
   do
   {
      $maxval=0;
      $maxidx=0;
      for ($i=0; $i<$entries; $i++)
      {
         if ($points_arr[$i]>$maxval)
         {
            $maxval=$points_arr[$i];
            $maxidx=$i;
         }
      }
      if ($maxval>0)
      {
         $rankcnt++;

         $query="SELECT tag FROM users where bssid='".strtoupper($teamid_arr[$maxidx])."'";
         $res=mysql_db_query("wmap",$query);
         if ($res) $result=mysql_fetch_array($res);

         if (($res) && ($result) && (strlen($result['tag'])>0)) $teamtag=$result['tag'];
         else $teamtag=$tag_arr[$maxidx];

         if ($rankcnt<=3)
         {
            $stag="<B><FONT COLOR=red>";
            $stagoff="</FONT></B>";
         }
         else if ($rankcnt<=10)
         {
            $stag="<B><FONT COLOR=blue>";
            $stagoff="</FONT></B>";
         }
         else if ($rankcnt<=20)
         {
            $stag="<B>";
            $stagoff="</B>";
         }
         else 
         {
            $stag="";
            $stagoff="";
         }
         fprintf($FHandle,"<TR><TD>".$stag."%d.".$stagoff."</TD><TD>".$stag."%d".$stagoff."</TD><TD>".$stag."%s".$stagoff."</TD><TD>".$stag."%d".$stagoff."</TD><TD>".$stag."%d".$stagoff."</TD></TR>",$rankcnt,$points_arr[$maxidx],utf8_decode(strip_tags($teamtag)),$team_members[$maxidx],$points_arr[$maxidx]/$team_members[$maxidx]);
         $points_arr[$maxidx]=0;
      }
   }
   while ($maxval>0);
   fclose($FHandle);      
}



/*$query="SELECT teamid, tag, SUM(count), COUNT(*) AS c FROM users WHERE teamid!='' GROUP BY teamid HAVING c>1";
$res=mysql_db_query("wmap",$query);
$entries=mysql_num_rows($res);
if ($entries>0)
{
   $FHandle=fopen("teamlist.php","w");
   for ($i=0; $i<$entries; $i++)
   {
      $data=mysql_fetch_array($res);
      fprintf($FHandle,"<TR><TD>%d</TD><TD>%s</TD></TR>",$data[2],$data['tag']);
   }
   fclose($FHandle);
} */


$timeouttime=4000000;

$query="UPDATE users SET tag='".$usertag."', teamid='".$teamid."', lastupload='".time()."', flags='".$userflags."' WHERE idx='$userid'";
mysql_db_query("wmap",$query);
$error=mysql_error();

/*
$query="UPDATE users SET flags='0' WHERE lastupload < ".(time()-$timeouttime);
mysql_db_query("wmap",$query);
$error=mysql_error();
*/

$rank=0;
$query="SELECT * FROM users where count>0 and lastupload > ".(time()-$timeouttime)." ORDER BY count DESC";
$res=mysql_db_query("wmap",$query);
$entries=mysql_num_rows($res);
if ($entries>0)
{
   $FHandle=fopen("toplist.php","w");
   $totalPoints=0;
   if ($FHandle)
   {
      for ($i=0; $i<$entries; $i++)
      {
         $data=mysql_fetch_array($res);
         if ($data['idx']==$userid) $rank=$i+1;
         if ($FHandle)
         {
            if ($i<3)
            {
               $stag="<B><FONT COLOR=red>";
               $stagoff="</FONT></B>";
            }
            else if ($i<10)
            {
               $stag="<B><FONT COLOR=blue>";
               $stagoff="</FONT></B>";
            }
            else if ($i<20)
            {
               $stag="<B>";
               $stagoff="</B>";
            }
            else 
            {
               $stag="";
               $stagoff="";
            }
            $totalPoints+=$data['count'];
            fprintf($FHandle,"<TR>");
            fprintf($FHandle,"<TD ALIGN=RIGHT>".$stag.($i+1).$stagoff);
            fprintf($FHandle,".</TD><TD ALIGN=RIGHT>".$stag.$data['count'].$stagoff."</TD><TD ALIGN=LEFT>".$stag.utf8_decode(strip_tags($data['tag'])).$stagoff);
            if (($data['flags'] & 2)==2) fprintf($FHandle," <A HREF='android/".$data['idx'].".png' TARGET=_blank><IMG SRC='android/map.png' BORDER=0 VALIGN=MIDDLE></A>");
            fprintf($FHandle,"</TD>");      
            fprintf($FHandle,"</TR>");
         }
      }
   }
   fclose($FHandle);
}

$query="SELECT count FROM users WHERE idx='$userid'";
$res=mysql_db_query("wmap",$query);
$error=mysql_error();

$result=mysql_fetch_array($res);
$error=mysql_error();

$count=$result['count'];

echo "120\n$count\n$rank\n$newAPs\n$updAPs\n$delAPs\n$newPoints\n\n";

if (($userflags & 2) == 0) @unlink($userid.".png");
else
{
   $runstat="php createmap.php $userid 1>/dev/null 2>/dev/null &";
   exec($runstat);
   $mapCreated=1;
}

$newPoints++; //to avoid dicision by zero
$riskCnt=0;
$dLat=latlon2dist($minLat,0,$maxLat,0);
$dLon=latlon2dist(0,$minLon,0,$maxLon);
$ap_area=$newPoints/($dLat*$dLon);

if ($newPoints>1800) $riskCnt++;
if ($dLat>550) $riskCnt++;
if ($dLon>550) $riskCnt++;
if ($dLat+$dLon>2250) $riskCnt++;
if ($ap_area>3000) $riskCnt++;

if (($riskCnt>=3) && ($userid!=104))
{
   mail("virtual_worlds@gmx.de",
        "OWLMap-Warning!",
        "Warning for User $userid:\n\nriskCnt=$riskCnt\nnewPoints=$newPoints\ndLat=$dLat\ndLon=$dLon\nap_area=$ap_area\n\nminLat=$minLat\nmaxLat=$maxLat\nminLon=$minLon\nmaxLon=$maxLon\n\nhttp://www.openwlanmap.org/android/$userid.png");

   if ($mapCreated==0)
   {
      $runstat="php createmap.php $userid 1>/dev/null 2>/dev/null &";
      exec($runstat);
   }        
}


?>
