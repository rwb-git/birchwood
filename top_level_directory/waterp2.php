<?php

   $fpcut = fopen("jay_source_select.txt", "r");

   $select = fgets($fpcut); 
   
   fclose($fpcut);

   if ($select != 1){
      
      chdir('x10_test_esp8266');
   }

   include("new_waterp2.php"); 
    
?> 
