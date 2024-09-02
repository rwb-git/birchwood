<?php

// research gallons_per_day_fake.php date issue vs. server timezone by getting the code from that file and polling all day long, both sites, dream and x10. 
//
// actually, this is not the date, but is the day of the week, which is used for the color bars in the gallons per day bar chart


   $day2 = intval(date("w")); // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight

   echo "$day2";


      // w 	Numeric representation of the day of the week 	0 (for Sunday) through 6 (for Saturday

/*
   $fpcut = fopen("dst_flag.txt", "r");

   $dst_flag = fgets($fpcut); 
   
   fclose($fpcut);

   echo "$dst_flag";
*/







?>
