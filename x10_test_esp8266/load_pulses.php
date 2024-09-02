<?php

   $fgname = "pulses.txt";

   $_fp = fopen($fgname, "r");

   $numbers = explode(" ", trim(fgets($_fp)));     // yesterday string , today string , yesterdays_code = 2017233
                                                //
                                                // in the current weird case, I'm simulating system failure for about 12 hours. there are pulses in the "today" section that need to appear yesterday, which is happening already with the 
                                                // old code for the GPM plot, presumeably because it uses minutes2 which I have adjusted, but the plot with the red ticks is missing
   fclose($_fp);


// 3-18-2021 added this for php 8. these 3 lines are in functions but php says zero hour is not defined here
$dra33 = getdate();

$current_millenium = $dra33[0];

$zero_hour = $current_millenium - 86400; // actually zero second lol


   $array_secs = array();
   $first_found = 0;

   $preceding_number = 0;

   foreach ($numbers as &$number){

      if ($number > $zero_hour){

         $array_secs[] = $number; // - $yesterday_midnight_millenium;

         if ($first_found == 0){

            $first_found = 1;

            $preceding = $preceding_number; ;
         }
      }
      
      $preceding_number = $number;  // save the previous number. for the expanded views (1 hr, 2, 4, 8) this enables gpm for the first pulse block which might extend off the left margin. for the 24 hour view this won't work with esp data
                                    // because it prunes on each write
   }

      $array_secs_cnt = count($array_secs);

      $array_gpm = array();

      // --------- calc gpm ---------------


         for ($i=0; $i<$array_secs_cnt; $i++){

               $denom33 = $array_secs[$i] - $preceding;     // before esp, i had all the pulse data for yesterday, but esp prunes on every pulse, so the initial value will always be 0. all this does is eliminate drawing of the
                                                            // first pulse in the ragged GPM chart, which is ok. in the expanded charts like 1 hour it matters slightly but not enough to care about. or if I do want it, just
                                                            // change pulse pruning to leave one pulse from the preceding 5 or 10 minutes, and add code in the new files to calculate preceding from that if it exists, and to
                                                            // do whatever else it takes - need to look at the plots while pulses extend just beyond left margin for all the types to verify behavior.

               if ($denom33 != 0){

                  $array_gpm[] = 60000 / ($array_secs[$i] - $preceding);            // gpm is stored x 10: 43.2 is stored 432.   gpm = 100 * 60 / secs, so use 60000 / secs
               } else {
                  
                  $array_gpm[] = 0;            // gpm is stored x 10: 43.2 is stored 432.   gpm = 100 * 60 / secs, so use 60000 / secs

               }

            $preceding = $array_secs[$i];
         }

//         $test_secs = $s tart_secs; // at least one pulse that is off the left end needs to be included so that the first pulse that is inside the plot is connected by a line to the left edge of the plot

         if ($debug_text == 1){
    
            if ($array_secs_cnt > 20){

               $start1 = $array_secs_cnt - 20;     // start_1 just used to print debug text on screen

            } else {

               $start1 = 0;
            }

               
            $m88 = "just show the last 20 pulses that are on this plot. ";

            imagettftext($my_img->img, 12, 0,  22, $yloc*20 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

            $yloc++;

            $y1 = 0;

            // array_secs are pure millenium. SS_esp is milenium for plot start. minutes2 is the actual current minutes since midnight. combine all this to get actual clock time?
            //
            // array_secs - SS_esp give seconds since plot start for each pulse = secs44
            // 
            // minutes2 - plot_hours * 60 gives minutes since midnight for plot start = minutes55
            //
            // so we can calc minutes since midnight for each pulse by adding  secs44 to minutes55 and converting to hr:min

            $seconds55 = ($minutes2 - ($plot_hours * 60)) * 60; // seconds since midnight for the start of this plot

            $y1 = $yloc * 20;

            $yloc++;
            
            $m88 = "lp: minutes2 $minutes2  seconds since midnight for start of this plot $seconds55";

            imagettftext($my_img->img, 12, 0,  22, $y1 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

            for ($i= $start1; $i<$array_secs_cnt; $i++){

               $y1 = $yloc * 20;

               $yloc++;

               $seconds44 = $array_secs[$i] - $SS_esp; // seconds for this pulse since the beginning of this plot. if this is negative the pulses are not on the plot

               if ($seconds44 > 0){

                  $seconds33 = $seconds44 + $seconds55; // seconds since midnight for this pulse

                  $hrs3 = intval($seconds33 / 3600);      // array_secs is now pure millenium, so subtract SS_esp which is millenium for start of this plot
                  $mins3 = intval(($seconds33 - ($hrs3 * 3600)) / 60);

                  if ($hrs3 > 23){

                     $hrs3 = $hrs3 - 24;
                  }

                  $gpm88 = $array_gpm[$i] / 10.0;
                  
                  $m88 = "lp: pulse millenium " . $array_secs[$i] . " pulse real clock time   ".$hrs3.":".$mins3."    gpm $gpm88";

                  imagettftext($my_img->img, 12, 0,  22, $y1 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text
               }

            }

            $gpm_size = count($array_gpm);
               
//            $m88 = "lp:array size $array_secs_cnt gpm size $gpm_size  start secs $s tart_secs   days_to_plot $days_to_plot    yesterday_code $yesterday_code  new code $new_yesterday_code";
  //          //$m88 = "array size $array_secs_cnt gpm size $gpm_size  start secs $st art_secs   days_to_plot $days_to_plot   yesterday $yesterday_date  yesterday_code $yesterday_code  new code $new_yesterday_code";

    //        imagettftext($my_img->img, 12, 0,  22, $yloc*20 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

            $yloc++;


         }



?>
