<?

include_once "geoutils.php";


function long2tilex($lon,$zoom)
{
   return floor((($lon + 180) / 360) * pow(2, $zoom));
}



function lat2tiley($lat,$zoom)
{
   return floor((1 - log(tan(deg2rad($lat)) + 1 / cos(deg2rad($lat))) / pi()) /2 * pow(2, $zoom));
}




function tilex2long($xtile,$zoom)
{
   $n = pow(2, $zoom);
   return $xtile / $n * 360.0 - 180.0;
}



function tiley2lat($ytile,$zoom)
{
   $n = pow(2, $zoom);
   return rad2deg(atan(sinh(pi() * (1 - 2 * $ytile / $n))));
}



function getTileImage($z,$x,$y)
{
   $handle = fopen("http://c.tile.openstreetmap.org/$z/$x/$y.png","rb");
   //$handle = fopen("http://tiles.virtualworlds.de/tile.php?z=$z&x=$x&y=$y", "rb");
   if ($handle)
   {
      $contents = stream_get_contents($handle);
      fclose($handle);
      $img=imagecreatefromstring($contents);
      if ($img) return $img;
   }

   return imagecreate(256,256);
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// zoom level => tile coverage in m
$zoomList=array("1" => 46945.558739255015*256.0,
                "2" => 23472.779369627508*256.0, 
                "3" =>11736.389684813754*256.0,
                "4" => 5868.194842406877*256.0,
                "5" => 2934.0974212034384*256.0,
                "6" => 1467.0487106017192*256.0,
                "7" => 733.5243553008596*256.0,
                "8" => 366.7621776504298*256.0,
                "9" => 183.3810888252149*256.0,
                "10" => 91.69054441260745*256.0,
                "11" => 45.845272206303726*256.0,
                "12" => 22.922636103151863*256.0,
                "13" => 11.461318051575931*256.0,
                "14" => 5.730659025787966*256.0,
                "15" => 2.865329512893983*256.0,
                "16" => 1.4326647564469914*256.0,
                "17" => 0.7163323782234957*256.0,
                "18" => 0.35816618911174786*256.0);
$latList=array();
$lonList=array();
$listCnt=0;
$min_latitude=1000.0;
$min_longitude=1000.0;
$max_latitude=-1000.0;
$max_longitude=-1000.0;

$userid=$argv[1];
if ($userid<=0) return;

$link = mysql_connect("127.0.0.1","oxy","grump02F");
if (!mysql_select_db("wmap",$link))
{
   echo mysql_error();
   return;
}

$query="SELECT lat,lon FROM netpoints WHERE userid='$userid'";
$res=mysql_db_query("wmap",$query);
echo mysql_error();
$entries=mysql_num_rows($res);
if ($entries>0)
{
   for ($i=0; $i<$entries; $i++)
   {
      $data=mysql_fetch_array($res);
      if (($data['lat']!=0) && ($data['lon']!=0))
      {
         $latList[$listCnt]=$data['lat'];
         if ($latList[$listCnt]>$max_latitude) $max_latitude=$latList[$listCnt];
         if ($latList[$listCnt]<$min_latitude) $min_latitude=$latList[$listCnt];
         $lonList[$listCnt]=$data['lon'];
         if ($lonList[$listCnt]>$max_longitude) $max_longitude=$lonList[$listCnt];
         if ($lonList[$listCnt]<$min_longitude) $min_longitude=$lonList[$listCnt];
         $listCnt++;
      }
   }
}
else
{
   echo mysql_error();
   return;
}




$distX=latlon2dist(1,$min_longitude,1,$max_longitude);
$distY=latlon2dist($min_latitude,1,$max_latitude,1);
if ($distX>$distY) $maxDist=$distX;
else $maxDist=$distY;

echo "MaxDist: $maxDist\n";

$useZoom=17;
for ($i=17; $i>1; $i--)
{
   if ($maxDist/($zoomList["$i"]/256)<2)
   {
      $useZoom=$i;
      break;
   }
}

echo "Zoom: $useZoom \n";
if ($useZoom<5) $useZoom=5;
{
   $tileX=long2tilex($min_longitude,$useZoom);
   $tileY=lat2tiley($min_latitude,$useZoom)-2;

   $tile_longitude=tilex2long($tileX+4,$useZoom);
   $tile_latitude=tiley2lat($tileY+4,$useZoom);
   if (($tile_latitude>$max_latitude) || ($tile_longitude>$max_longitude))
   {
      $useZoom--;
      $tileX=long2tilex($min_longitude,$useZoom);
      $tileY=lat2tiley($min_latitude,$useZoom)-2;
      echo "Expanding zoom...\n";
   }


   $my_img=imagecreatetruecolor(1024,1024);
   for ($x=0; $x<4; $x++)
    for ($y=0; $y<4; $y++)
   {
      $tile_img=getTileImage($useZoom,$tileX+$x,$tileY+$y);
      imagecopy($my_img,$tile_img,256*$x,256*$y,0,0,256,256);
      imagedestroy($tile_img);
   }
   imagefilter($my_img,IMG_FILTER_CONTRAST,15);
   imagefilter($my_img,IMG_FILTER_BRIGHTNESS,25);


echo $min_longitude."  ".$min_latitude."  ".$max_longitude."  ".$max_latitude."\n";
echo $tileX."  ".$tileY."\n";
   $min_longitude=tilex2long($tileX,$useZoom);
   $max_longitude=tilex2long(($tileX+4),$useZoom);
   $min_latitude=tiley2lat($tileY+4,$useZoom);
   $max_latitude=tiley2lat(($tileY),$useZoom);
echo $min_longitude."  ".$min_latitude."  ".$max_longitude."  ".$max_latitude."\n";
   $pos_colour = imagecolorallocatealpha( $my_img,50,50,150,110);
   $query="SELECT lat,lon FROM netpoints WHERE userid<>'$userid' AND lat>$min_latitude AND lat<$max_latitude AND lon>$min_longitude AND lon<$max_longitude LIMIT 150000";
   echo $query."\n";
   $res=mysql_db_query("wmap",$query);
   echo mysql_error();
   $entries=mysql_num_rows($res);
echo $entries."\n";   
   if ($entries>0)
   {
      for ($i=0; $i<$entries; $i++)
      {
         $data=mysql_fetch_array($res);
         if (($data['lat']!=0) && ($data['lon']!=0))
         {
            $t_lat=$data['lat'];
            $t_lon=$data['lon'];

            $m_tileX=long2tilex($t_lon,$useZoom);
            $m_tileY=lat2tiley($t_lat,$useZoom);

            $tileLon1=tilex2long($m_tileX,$useZoom);
            $tileLat1=tiley2lat($m_tileY,$useZoom);
            $tileLon2=tilex2long($m_tileX+1,$useZoom);
            $tileLat2=tiley2lat($m_tileY+1,$useZoom);

            $y=($m_tileY-$tileY)*256+(256.0*($t_lat-$tileLat1)/($tileLat2-$tileLat1));
            $x=($m_tileX-$tileX)*256+(256.0*($t_lon-$tileLon1)/($tileLon2-$tileLon1));
//echo $m_tileX."\t".$tileX."\t".$x."\t".$y."\n";
            imagerectangle($my_img,$x-1,$y-1,$x+1,$y+1,$pos_colour);
         }
      }
   }
      


   $pos_colour = imagecolorallocatealpha( $my_img, 240,0,0,60);

   $cnt=0;
   $prevLat=0;
   $prevLon=0;
   $distance=0;
   for ($i=0; $i<$listCnt; $i++)
   {
      $t_lat=$latList[$i];
      $t_lon=$lonList[$i];

      $m_tileX=long2tilex($t_lon,$useZoom);
      $m_tileY=lat2tiley($t_lat,$useZoom);

      $tileLon1=tilex2long($m_tileX,$useZoom);
      $tileLat1=tiley2lat($m_tileY,$useZoom);
      $tileLon2=tilex2long($m_tileX+1,$useZoom);
      $tileLat2=tiley2lat($m_tileY+1,$useZoom);

      $y=($m_tileY-$tileY)*256+(256.0*($t_lat-$tileLat1)/($tileLat2-$tileLat1));
      $x=($m_tileX-$tileX)*256+(256.0*($t_lon-$tileLon1)/($tileLon2-$tileLon1));
//echo $m_tileX."\t".$tileX."\t".$x."\t".$y."\n";
      imagerectangle($my_img,$x-2,$y-2,$x+2,$y+2,$pos_colour);
   }
   imagetruecolortopalette($my_img,FALSE,256);
   imagepng($my_img,$userid.".png",9,PNG_ALL_FILTERS);
}
?>



