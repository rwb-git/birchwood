<html>
 <head>
  <title>Birchwood Water new 2</title>
 </head>
 <body>
 

<img src="mk_pulse_img_new_fake.php" alt="Image created by a PHP script">

<!-- php begins here ------------------------------------------------------------------------------------------------------------------------>

<?php

   // water2.php and water2_new_fake.php should be identical, and I shouldn't have used two different files

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

   $seconds_since_last_packet = $phs;
   
   $daysec = 24*60*60;

   $days4 = (int) ($phs / $daysec);

   $phs = $phs - $days4 * $daysec;

   // this looks wrong. why not use total $phs??? $seconds_since_last_packet = $phs;

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
      
}


//-------------------------------- main body -------------------------------------------------------------------------------------------------------------


//---------------- read last data block to see what time newest data is --------------------------------

//$_fp = fopen("small_test_new33.txt", "r");

$_fp = fopen("small_test.txt", "r");

$numbers = explode(" ", trim(fgets($_fp)));

$tank2=0;               // old values from file are tank2, status2, etc. at this moment, I use date2 to determine midnite and minutes2 to see if packets have been missed
$status2=0;
$minutes2=0;
$date2=0;

$index = 0;

foreach ($numbers as &$number){

   $number = intval($number);

   if ($index == 0){
      $tank2=$number;
   } elseif($index == 1){
      $status2=$number;
   } elseif ($index == 2){
      $minutes2=$number;
   } elseif ($index == 3){

      $date2 = $number;
   } elseif ($index == 4){

      $mil2 = $number;


   }

   $index = $index + 1;
}

fclose($_fp);

   $dra = getdate();

   show_elapsed($dra[0] - $mil2);

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

//   $max_file_size = 1000;        // test on small file
   $max_file_size = 20 *1024;

   if ($ip_filesize > $max_file_size){

      prune_log_file();       // removes about 1/4 of the file

   }

?>




<!-- html resumes here ------------------------------------------------------------------------------------------------------------------------>

<br>



<a href="menu.html"><center><img src="menu.png"></center></a>



 </body>
</html>
