<?
include_once "config.php";


function correctPhonenumber($mphone)
{
$mphone=str_replace(" ","",$mphone);
if (substr($mphone,0,1)=="+") $mphone="00".substr($mphone,1);
else if (substr($mphone,0,2)!="00") $mphone="00".$mphone;

return $mphone;
}



function fIn($in)
{
$in=strip_tags($in);
$in=stripslashes($in);
$in=addslashes($in);

return $in;
}



function fInS($name)
{
$in=$_GET[$name];
if (empty($in)) $in=$_POST[$name];
return fIn($in);
}


function fetchByQuery($query)
{
global $link;

$result = mysql_query($query,$link);
if ((!$result) || ($result==1))
   {
   //echo mysql_error();
   return 0;
   }
$entries=mysql_num_rows($result);
if ($entries==0)
   {
   echo $str_db_error;
   return 0;
   }
return mysql_fetch_array($result);
}



function setByQuery($query)
{
global $link;

$result = mysql_query($query,$link);
if (!$result)
   {
   return mysql_error();
   }
return "";
}



function fetchNextListEntry()
{
global $listresult;

return mysql_fetch_array($listresult);
}



function fetchListByQuery($query)
{
global $link,$listresult;

$listresult = mysql_query($query,$link);
if (!$listresult)
   {
//   echo "fetchListByQuery(): ".mysql_error(); // do not print - fails when tablesdon't exist!
   return 0;
   }
$entries=mysql_num_rows($listresult);
if ($entries==0)
   {
   return 0;
   }
return $entries;
}



function fetchNextListEntry2()
{
global $listresult2;

return mysql_fetch_array($listresult2);
}



function fetchListByQuery2($query)
{
global $link,$listresult2;

$listresult2 = mysql_query($query,$link);
if (!$listresult2)
   {
   echo "fetchListByQuery(): ".mysql_error();
   return 0;
   }
$entries=mysql_num_rows($listresult2);
if ($entries==0)
   {
   return 0;
   }
return $entries;
}



function resultByQuery($query,$resultnum)
{
global $link,$result;

$result = mysql_query($query,$link);
if (!$result)
   {
   echo mysql_error();
   return 0;
   }
return mysql_result($result,$resultnum);
}



?>
