<?php          


// this applies to tank, flow, trans and float but does not apply to pulse. these files have the form millenium value millenium value, whereas pulse is a simple list of milleniums

   
   include 'ping.php';     // ping does this:   $dra33 = getdate(), and $current_millenium = $dra[0] and $mil24 = current - 86400

   $new_state=$_GET["state"];

   // remove records older than 24 hours then re-write file with new record added

   $fp = fopen($fgname, "r");

   $numbers = explode(" ", trim(fgets($fp)));   // one string: millenium 1 millenium 0 etc. for trans, float, flow. tank is milen level milen level etc

   fclose($fp);

   $milens = array();

   $old_states = array();

   $index = 0;

   $cnt = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         if ($number >= $mil24){
            $milens[] = $number;
            $cnt++;
            $keep = 1;

         } else {

            $keep = 0;
         }

         $index = 1;

      } else {

         $index = 0;
  
         if ($keep == 1){
            $old_states[] = $number;
         }
      }
   }

   $fp = fopen($fgname, "w");

   for ($i=0;$i<$cnt;$i++){

      fwrite($fp, $milens[$i]);
      fwrite($fp, " ");

      fwrite($fp, $old_states[$i]);
      fwrite($fp, " ");
   }

   fwrite($fp, $current_millenium);    // this is the new record which uses server time, so any delay between master - esp - php will affect this. later on i think i want to change this to use a timestamp from esp which will have
                                       // been created when the event occurred, so any delay will not affect it, and furthermore that approach allows esp to dump blocks if web has been down, similar to how float, gallons, and pulses
                                       // are handled by android
   fwrite($fp, " ");

   fwrite($fp, $new_state);
   fwrite($fp, " ");

   fclose($fp);


?>



