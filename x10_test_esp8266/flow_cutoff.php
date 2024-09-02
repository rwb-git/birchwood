<?php          


//   this is a simple file of 1440 numbers, one for each minute of the day, so just overwrite any existing file.

   $cutoff=$_GET["cutoff"];
   
   $handle= fopen("flow_cutoff.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$cutoff);

   fclose($handle);


?> 
