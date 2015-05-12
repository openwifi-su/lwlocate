<?

function latlon2dist($lat1,$lon1,$lat2,$lon2)
{
   $R = 6371; // km
   $dLat = deg2rad($lat2-$lat1);
   $dLon = deg2rad($lon2-$lon1); 
   $a = sin($dLat/2.0) *sin($dLat/2.0) +cos(deg2rad($lat1)) *cos(deg2rad($lat2)) *sin($dLon/2.0) *sin($dLon/2.0); 
   $c = 2.0*atan2(sqrt($a),sqrt(1-$a)); 
   return ($R *$c);
}



/*function long2tilex($lon,$z)
{
   return floor((($lon + 180.0) / 360.0) * pow(2.0, $z));
}



function lat2tiley($lat,$z)
{
   return floor((1.0 - log(tan($lat*pi()/180.0) + 1.0 / cos($lat*pi()/180.0)) / pi()) /2.0 * pow(2.0, $z));
}




function tilex2long($x,$z)
{
   $n = pow(2.0, $z);
   return $x/ $n * 360.0 - 180.0;
}



function tiley2lat($y,$z)
{
   $n=pi()-2.0*pi()*$y/pow(2.0,$z);
   return 180.0/pi()*atan(0.5*(exp($n)-exp(-$n)));
//   $n = pow(2, $z);
//   return rad2deg(atan(sinh(pi() * (1 - 2 * $y/ $n))));
}



function getTileImage($z,$x,$y)
{
   $handle = fopen("http://tiles.virtualworlds.de/tile.php?z=$z&x=$x&y=$y", "rb");
   if ($handle)
   {
      $contents = stream_get_contents($handle);
      fclose($handle);
      $img=imagecreatefromstring($contents);
      if ($img) return $img;
   }

   return imagecreate(256,256);
} */

?>