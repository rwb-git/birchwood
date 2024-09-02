<?php          


// this applies to pulse which only stores millenium. all the others have records with two fields

   
   include 'ping.php';     // ping does this:   $dra33 = getdate();     $current_millenium = $dra33[0]      $mil24 = $current_millenium - 86400
   
   // remove records older than 24 hours then re-write file with new record added
   
   $fgname = 'pulses.txt'; 

   $fp = fopen($fgname, "r");

   $numbers = explode(" ", trim(fgets($fp)));   // one string: millenium 1 millenium 0 etc. for trans, float, flow. tank is milen level milen level etc

   fclose($fp);

   $milens = array();

   $cnt = 0;

   foreach ($numbers as &$number){

      if ($number >= $mil24){    // compare timestamp to mil24 calculated by ping.php up there in this file
         $milens[] = $number;
         $cnt++;
      }
   }

   $fp = fopen($fgname, "w");

   for ($i=0;$i<$cnt;$i++){

      fwrite($fp, $milens[$i]);
      fwrite($fp, " ");
   }

   fwrite($fp, $current_millenium);    // add this pulse
   fwrite($fp, " ");

   fclose($fp);


?>



