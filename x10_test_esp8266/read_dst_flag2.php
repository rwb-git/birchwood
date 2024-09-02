<?php

   $fpcut = fopen("dst_flag.txt", "r");

   $dst_flag = fgets($fpcut); 
   
   fclose($fpcut);

//   echo "$dst_flag";
?>
