<?php


   $_fp = fopen("ping.txt", "r");

   $numbers = explode(" ", trim(fgets($_fp)));

   foreach ($numbers as &$number){

      $millenium = $number;
   }

   fclose($_fp);
   
   date_default_timezone_set('UTC');

   $server_minute = date('i');
   $server_hour = date('G'); // 0..23
   $server_second = date('s'); // 0..59
   
   $server_timezone = date('e');

/*

   // debug crap to see if dream date() is same as biz date()

   $fp = fopen('server_date.txt', 'w');    // 'a' for append

   fwrite($fp, "server minute $server_minute  hour $server_hour  second $server_second  timezone $server_timezone");
   
   fclose($fp);
      // biz server minute 18  hour 15  second 45  timezone UTC

      // dream server minute 18  hour 7  second 48  timezone America/Los_Angeles

      // dream after date_default.. server minute 21  hour 15  second 58  timezone UTC
      // web says LA is -8 hours

      // so this is either 15 - 7 = 8 hours behind, or 16 hours ahead. does it matter, so long as i adjust. i think this only applies to legend, but i
      // could be wrong about that. so do i need two fucking scripts or can i tell where i am programmatically? right now, before i correct this, biz plot correctly says 10, but dream says 2, so just
      // assume dream is 8 hours behind. i think biz is in germany so that probably makes sense

      // i could put a little text file to ID the location, or to contain the correction value.

*/

   if ($server_hour < 5){

      $minutes2 = $server_minute + 60 * ($server_hour + 19); // est is 5 hours different. minutes2 is only used to show the time legends. I don't think it is ever used for anything else. if so, why?
   
   } else {
      
      $minutes2 = $server_minute + 60 * ($server_hour - 5); // est is 5 hours different
   }

   include "read_dst_flag2.php"; // this echos the value, so it will go back to esp server. if that's an issue, set a flag to disable or make a separate file

   if ($dst_flag == 1){

      $minutes2 += 60; // spring ahead if dst is on

   }


   // all the code below this line was added in 2020 so I can show blanks in status and tank level and other plots if data is more than 5 minutes old

   $dra33 = getdate();
   
   $current_millenium = $dra33[0];

   $secs_since_last_packet = $current_millenium - $millenium;     // NOTE THAT old files calculate seconds_since_last_packet, so I might want to change the name here, and get rid of the old calcs

   // this returns a float if any remainder from integer division:     $minutes_since_last_packet = $secs_since_last_packet / 60;
   
   $minutes_since_last_packet = intdiv($secs_since_last_packet,60);

//   $minutes_since_last_packet = 15 * 60;

   if ($minutes_since_last_packet > 15){     // >>>>>>>>>>> make sure this agrees with all the other files that show the text warning. they compare to seconds, so this might be off by one minute which is ok <<<<<<<<<

      $data_is_old = 1;

      $shift_arrays_by_this_much = intdiv($minutes_since_last_packet,5);

      $pivot_index = 288 - $shift_arrays_by_this_much;

// esp calculates minutes2 from server clock so this is wrong      $minutes2 = $minutes2 + $shift_arrays_by_this_much * 5;      // don't use minutes_since_last packet here. this way it will be 5 minute blocks

   } else {

      $data_is_old = 0;

   }


/*
         the division operator ("/") returns a float value unless the two operands are integers (or strings that get converted to integers) and the numbers are evenly divisible, 
         in which case an integer value will be returned. For integer division, see intdiv().

         intdiv ( int $dividend , int $divisor ) : int

         Returns the integer quotient of the division of dividend by divisor.
*/

//   $secs_since_yesterday_midnight = 86400 + $minutes2 * 60;

   // subtract that from current milenium to get milenium for yesterday midnight. 
   
//   $yesterday_midnight_millenium = $current_millenium - $secs_since_yesterday_midnight;      // i used to think this made sense. and it might still be used when esp has to rarely upload full file block due to internet outage, but
                                                                                             // otherwise this looks like yet another logical failure on my part. how is this point in time relevant to anything other than the example mentioned?
   


?>
