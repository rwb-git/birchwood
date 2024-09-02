<?php          


//   this is a simple file of 1440 numbers, one for each minute of the day, so just overwrite any existing file.


   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 

      exit();
   }
   
   $handle= fopen("flow_adc_data.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$inp);

   fclose($handle);


?> 
