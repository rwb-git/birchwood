<?php          

// this is for named arguments in the URL itself

//   $state=$_GET["state"];


   $dra33 = getdate();
   
   $current_millenium = $dra33[0];

   $fp = fopen('ping.txt', 'w');    // 'a' for append

   fwrite($fp, $dra33[0]);
   fwrite($fp, " ");
   
   fclose($fp);


   $fp = fopen('ping_master.txt', 'r');   // 'a' for append. this file is used to plot the master packet bar in the status rows. if esp gets packets from master it will send them to php or ping php if no new event has occurred.
                                          // so, if master is dead, esp will not ping and this will put red blocks in the master row. 

   $numbers = explode(" ", trim(fgets($fp)));   // one string: millenium 1 millenium 0 etc. for trans, float, flow. tank is milen level milen level etc

   fclose($fp);

   $pmilens = array();

   $pold_states = array();

   $pindex = 0;

   $pcnt = 0;

   $mil24 = $current_millenium - 86400; // we want to keep the last 24 hours of records so skip anything lower than mil24

   foreach ($numbers as &$number){

      if ($pindex == 0){

         if ($number >= $mil24){
            $pmilens[] = $number;
            $pcnt++;
            $keep = 1;

         } else {

            $keep = 0;
         }

         $pindex = 1;

      } else {

         $pindex = 0;
  
         if ($keep == 1){
            $pold_states[] = $number;
         }
      }
   }

   $fp = fopen('ping_master.txt', 'w');    // 'a' for append

   for ($i=0;$i<$pcnt;$i++){

      fwrite($fp, $pmilens[$i]);
      fwrite($fp, " ");

      fwrite($fp, $pold_states[$i]);
      fwrite($fp, " ");
   }

   fwrite($fp, $current_millenium);

   fwrite($fp, " 1 ");

   fclose($fp);



?>



