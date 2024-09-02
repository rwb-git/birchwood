<?php

            $array_secs_cnt = count($array_secs);

            $array_gpm = array();


         // --------- calc gpm ---------------



            for ($i=0; $i<$array_secs_cnt; $i++){

               $denom33 = $array_secs[$i] - $preceding;

               if ($denom33 != 0){

                  $array_gpm[] = 60000 / ($array_secs[$i] - $preceding);            // gpm is stored x 10: 43.2 is stored 432.   gpm = 100 * 60 / secs, so use 60000 / secs
               } else {
                  
                  $array_gpm[] = 0;            // gpm is stored x 10: 43.2 is stored 432.   gpm = 100 * 60 / secs, so use 60000 / secs

               }

               $preceding = $array_secs[$i];


            }

            if ($array_secs_cnt > 20){

               $start1 = $array_secs_cnt - 20;     // start_1 just used to print debug text on screen

            } else {

               $start1 = 0;

            }




// 3-18-2019 everthing down to here looked same as pulse_stuff_33_fake.php, but now the code is different


            $newest_pulse_secs = $array_secs[$array_secs_cnt - 1];


            $no_float = 0;

            $force_float = 0;

            if ($array_secs_cnt > 2){ // lol more failsafe ways to plot float. if float never turned on or off during the plot period, and there were pulses, draw the float on full time

               if ($SS_esp <= $newest_pulse_secs){  // but ignore pulses that happened before the plot start

                  $force_float = 1;
               } else {

                  $no_pulses = 1;
               }
            } else {

               $no_pulses = 1;
            }



            // ---------- now check minutes2 and newest pulse time and adjust plot if minutes2 is behind newest pulse packet; the point here is to make sure that the last packets are on the plot and
            // do not hang off the right hand end due to the fact that pulses may come in before the big normal packet that updates the time file where minutes2 comes from. this should not be
            // affected by the size of the plot, 1 hour or 8 hours, but only if the newest pulse comes after minutes2; push minutes2 a bit further so that all packets are on the plot. it's a
            // visual thing related only to how nice the plot looks if they don't hang off the end.



               
            $msecs = $minutes2 * 60;
            
            if ($debug_text == 1){


               $yloc++;

            }


            // --------------- vertical lines for quarter hour marks -----------------------------------

            $block_minutes = $plot_hours * 5; // show 12 divisions (blocks) for all plots. so, block minutes = hours x 60 / 12 which is hours x 5


            

            $offset = $block_minutes - ($minutes2 % $block_minutes);       // offset is where the first legend time value is printed, and the first vertical line if there is any lines or tick marks
            //
            // minutes2 modulo block minutes gives us 0 if we are ending on an integral block, or some other value if we are ending in the middle of a block


            $secs = (($minutes2 - $plot_hours * 60) + $offset) * 60; //SS_esp + 60 * $offset;   // seconds at first vertical line and time annotation; minutes2 is right now minutes since midnight today. the plot ends at minutes2, so
                                                                                                // we subtract plot_hours*60 to get the true time at the left end of the plot, in minutes since midnight today, which will be displayed in 12 hour format

            if ($secs < 0){ // plot extends back past midnight

               $secs += 86400;

            }

            
            $first_mark = $offset * $pixels_per_minute + $plot_x1;  // pixel location of first line

            $delta_pix = $pixels_per_minute * $block_minutes;   // pixels between vertical lines

            if ($debug_text == 1){
               
              $s = " block minutes $block_minutes   offset $offset   secs   $secs   first mark $first_mark   delta pix $delta_pix";

               imagettftext($my_img->img, 12, 0,  30,  $yloc * 20 ,  $text_colour, $lib_font, $s); // img, size, angle, x, y, color, font, text
               $yloc++;
            }



            for ($i=0;$i< 12;$i++){
            
               
               $hour3 = (int) ($secs / 3600);   
               
               $mins3 = $secs / 60 - ($hour3 * 60);

               if ($hour3 == 0){

                  $hour3 = 12;

               } else while ($hour3 > 12){

                  $hour3 -= 12;
               }

               if ($hour3 < 1){

                  $hour3 += 12;
               }

               if ($mins3 > 0){

                  $s = $hour3 . ":" . $mins3; 

               } else {
               
                  $s = $hour3 ; 

               }

               $secs += $block_minutes * 60; // 15 minutes per line

               $bbox = imagettfbbox(12, 0, $lib_font,  $s);   // size angle font text

               $xcenter = ($bbox[2] - $bbox[0])/2;

               imagettftext($my_img->img, 12, 0,  (int)$first_mark - (int)$xcenter, $tank_y6 + 16 ,  $text_colour, $lib_font, $s); // img, size, angle, x, y, color, font, text

               $first_mark += $delta_pix;

            }

               
               // watermark for x10. it can't be further to the right unless i change the img width, so leave it like this. hmm, on the right gets clipped sometimes, so put at far left?

               $s = "x10";
               
               imagettftext($my_img->img, 6, 0,  10 , $tank_y6 + 16 ,  $text_colour, $lib_font, $s); // img, size, angle, x, y, color, font, text
               //imagettftext($my_img->img, 6, 0,  $first_mark - $xcenter , $tank_y6 + 16 ,  $text_colour, $lib_font, $s); // img, size, angle, x, y, color, font, text

?>
