<?php

   $fpcut = fopen("jay_source_select.txt", "r");

   $select = fgets($fpcut); 
   
   fclose($fpcut);

   if ($select != 1){      // 1 = use old style top dir code avr data. 2 = new style x10 code esp data
      
      chdir('x10_test_esp8266');
   }

   include("new_water2.php"); 
    
?> 
