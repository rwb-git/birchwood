<?php          


//   this is a simple file of 366 numbers, one for each day of the year, sent in order so that the newest value from yesterday is the final value sent.


   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 

      exit();
   }
   
   $handle= fopen("rpi_temp_data.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$inp);

   fclose($handle);


?> 
