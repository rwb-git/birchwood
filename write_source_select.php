<?php          


   $cutoff=$_GET["cutoff"];
   
   $handle= fopen("source_select.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$cutoff);

   fclose($handle);


?> 
