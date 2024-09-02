<?php

   $fpcut = fopen("source_select.txt", "r");

   $select = fgets($fpcut); 
   
   fclose($fpcut);

   if ($select != 1){
      
      chdir('x10_test_esp8266');
   }

   include("water.php"); 
    
?> 
