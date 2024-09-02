<?php

   $fpcut = fopen("source_select.txt", "r");

   $select = fgets($fpcut); 
   
   fclose($fpcut);

   echo "$select";
?>
