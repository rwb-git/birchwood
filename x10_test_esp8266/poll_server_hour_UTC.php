<?php
   
   date_default_timezone_set('UTC');

   $hour24 = intval(date("G")); // G returns hour 0..23

   echo "$hour24";

?>
