<?php    


   $enable_echo = 0;
   
//   $fgname = "float_state.txt";

   $_fp = fopen($fgname, "r");

   $numbers = explode(" ", trim(fgets($_fp)));   // milen state milen state

      //1613263755 0 1613297779 1 1613301520 0 1613323903 1 1613329288 0 1613348817 1 

   fclose($_fp);

//   $yesterday_raf = array();

//   $yesterday_on_off_f = array();

   $esp_float = array();   // pixel values to plot 

   $states = array();

   $minutes = array();

   $index = 0;

   $dra33 = getdate();
   
   $current_millenium = $dra33[0];
   
   $zero_hour = $current_millenium - 86400; // actually zero second lol

   $good_one;

   $current_state = -1;

   $good_cnt = 0;


   foreach ($numbers as &$number){

      if ($index == 0){

   //      $yesterday_raf[] = $number;

         $index = 1;

         $secs = $current_millenium - $number;

         $hrs = intval($secs / 3600);

         $secs2 = $secs - $hrs * 3600;

         $mins = intval($secs2 / 60);

         $secs2 = $secs2 - $mins * 60;
         
         if ($enable_echo == 1) echo "$number  seconds since this event is $secs  which is $hrs:$mins:$secs2<br> <br>";

         $plot_mins = intval(($number - $zero_hour) / 60);

         if ($enable_echo == 1) echo "plot minutes $plot_mins<br><br>";

         if ($plot_mins >= 0){

            $good_one = 1;


         } else {

            $good_one = 0;

         }

         // for the plot i need to convert this either to minutes since 24 hours ago, or scale it from 0..863 to fit the plot
         // also, if the first state is turn off = 0, add a block from 0 to that
         // and if the final state is turn on = 1, add a block from that to the end



      } else {

         $index = 0;
         
     //    $yesterday_on_off_f[] = $number;
         
         if ($enable_echo == 1) echo "$number  <br> <br>";

         if ($good_one == 1){

            if ($number != $current_state){ // ignore strings of 0 0 0 or 1 1 1

               $states[] = $number;
            
               $minutes[] = $plot_mins;

               $current_state = $number;

               $good_cnt++;

            }

         }
      }
   }


   $state = $states[0];


   if ($enable_echo == 1) echo "summary<br><br>";

   for ($ii=0;$ii<$good_cnt;$ii++){

      if ($enable_echo == 1) echo "good state $states[$ii] minutes $minutes[$ii]<br><br>";


   }

   // now make an array to draw the boxes. 
   // consider all the situations:
   //
   // 0 1 0 1 0         make a start box so the while() loop begins with indexes 1 and 2
   //   1 2 3 4         good cnt = 5. for loop two times: good_cnt - start box = 4
   //
   // 0 1 0 1 0 1       make a start box so the while() loop begins with indexes 1 and 2 and there is a box on the far end
   //   1 2 3 4 5       good_cnt = 6  for loop two times: good cnt - start box - end box = 4
   //
   // 1 0 1 0 1         while loop begins with 0 and 1 and there is a box on the far end
   // 0 1 2 3 4         good_cnt = 5 for loop two times: good_cnt - end box = 4
   //
   // 1 0 1 0 1 0       while loop begins with 1 and 0
   // 0 1 2 3 4 5       good cnt = 6 for loop 3 times
   //
   // 0                 make a start box and quit good cnt - 1 = 0
   // 1                 make a box on the end and quit  good cnt - 1 = 0
   //
   // 0 1               make a start box and a box on the end good cnt - start -end = 0
   // 
   // 1 0               one normal box good cnt = 2



   $start_index = 0;
   
//   $x1_ra = array();
//   $x2_ra = array();


//   $factor = 1.0; //864.0 / 1440.0; // convert minutes in 24 hours to pixels
   $factor = 864.0 / 1440.0; // convert minutes in 24 hours to pixels

   $loop_cnt = $good_cnt;

   $box_cnt = 0;

   if ($states[0] == 0){ // it was already on so add a box

      $start_index = 1;

      $x1_ra[] = 0;
      $x2_ra[] = intval((float)$minutes[0] * $factor);

      $loop_cnt --;
      $box_cnt++;
   }


   if ($states[$good_cnt - 1] == 1){ // add box on end

      $x1_ra[] = intval((float)$minutes[$good_cnt - 1] * $factor);
      $x2_ra[] = 863;

      $box_cnt++;

      $loop_cnt --;
   }


   $done = 0;

   $this_state = 0;

   for ($boxes=0;$boxes<($loop_cnt / 2);$boxes++){

      $box_cnt++;

      $x1_ra[] = intval((float)$minutes[$start_index++] * $factor);
      $x2_ra[] = intval((float)$minutes[$start_index++] * $factor);

   }
   


   for ($boxes=0;$boxes<$box_cnt;$boxes++){

      if ($enable_echo == 1) echo"x1_ra $x1_ra[$boxes]    x2_ra $x2_ra[$boxes]<br><br>";

   }

      





?>
