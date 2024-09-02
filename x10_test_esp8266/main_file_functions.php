

<?php             # this works: http://birchwood.co.nf/get.php/?tank=75&status=37

// this file has functions called by the web pages we look at, that include other php scripts to create images. those other scripts typically use functions.php



function prune_log_file(){    //----------------------------------------------------------------------------

   //$fp = fopen('ip_log.txt', 'a');    // 'a' for append
   $handle = fopen("ip_log.txt", "r");

   $contents = array();

   if ($handle) {
       while (($line = fgets($handle)) !== false) {

           $contents[] = $line;
      }

      fclose($handle);
   }

   $count = count($contents);

   $size = (int) ($count * 3/4);    // remove about 1/4 of the file

   if ($count >$size){  

      $handle = fopen("ip_log.txt", "w");

      if ($handle) {

         for ($x= ($count - $size); $x<$count; $x++){

           fwrite($handle,$contents[$x]);
         }

         fclose($handle);
      }
   }
}



function show_elapsed($phs){     //---------------------------------------------------------------------------

//   global $millenium;      // this is just for verification of php global var scope and is not actually used in this function

   $seconds_since_last_packet = $phs;
   
   $daysec = 24*60*60;

   $days4 = (int) ($phs / $daysec);

   $phs = $phs - $days4 * $daysec;

   $hrs4 = (int) ($phs / 3600);

   $phs = $phs - $hrs4 * 3600;

   $mins4 = (int) ($phs / 60);

   $phs = $phs - $mins4 * 60;

   // packets should be received at least every five minutes, but don't panic until about 15 minutes?

   if ($seconds_since_last_packet > (15 * 60)){ // >>>>>>>>>> make sure this agrees with read_time_file.php, which uses integer math and might differ by about one minute, which is ok <<<<<<<<<<<<

      echo "<br>";

      echo "Data is not up-to-date: time since last data was received: ";

      if ($days4 > 0){

         echo "$days4 days, ";
      }

      if ($hrs4 > 0){

         echo "$hrs4 hours, ";
      }

      if ($mins4 > 0){

         echo "$mins4 minutes, ";
      }

      echo "$phs seconds. Data is received every five minutes when everything works properly. <br>";
   }
 //     echo "verify millenium is global >>>$millenium.<<< phs = >>>$phs<<< <br>";  yeah, if millenium is declared global in this function, and is created in read_time_file.php, then it works as it should. no need to declare it in the main file
 //  that includes read_time_file.php. when php code is "included", it's the same as typing it in the file at that point. don't confuse that with functions, moron.
}



function handle_time_and_IP_log(){

   global $millenium;

   $dra = getdate();

   show_elapsed($dra[0] - $millenium);

   $ip = $_SERVER['REMOTE_ADDR'];

   $fp = fopen('ip_log.txt', 'a');    // 'a' for append

   fwrite($fp, $ip);
   fwrite($fp, " ");
 
   fwrite($fp, $dra['hours']);
   fwrite($fp, " ");

   fwrite($fp, $dra['minutes']);
   fwrite($fp, " ");

   fwrite($fp, $dra['seconds']);
   fwrite($fp, " ");

   fwrite($fp, $dra['mday']);
   fwrite($fp, " ");

   fwrite($fp, "\n");

   fclose($fp);

   $ip_filesize = filesize("ip_log.txt");    // size in bytes. hmm. prune at 20 kb and see what that looks like?
                                             // 57 lines was about 1450 bytes
   $max_file_size = 20 *1024;

   if ($ip_filesize > $max_file_size){

      prune_log_file();       // removes about 1/4 of the file
   }
}

?>






