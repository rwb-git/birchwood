<?php

   // 9-23-2021 I think the new esp8266 version of this file is simplified because there is no need to look at midnight to see if pulses need to be shifted. who knows. I never document anything. at any rate, today the pulse meter was
   // not working so there are no pulses, and the esp8266 huge pages had error, which i think i corrected here by testing array_secs_cnt. why doesn't the old file crash? look at files in "download_biz..." or "download_dream..." and document
   // in those files whether or not it needs a fix. those huge pages might have been crashing too, i think, but then they were ok and I was in a flustered hurry so I'm not sure.


   $local_debug_text = 0;


   if ($local_debug_text == 1){
     
      $m88 = "revision 33";

      imagettftext($my_img->img, 12, 0,  522, 300 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text
   }

   $plot_hours = 24;
   

   $array_864 = array_fill(0,865 * 2 ,0); // 48 hours. 864 = 288 *3 and corresponds to pixels if stretch is 3. used for the gpm plot with the red ticks. see pulse_common_1
                                             // array_fill(start index, number of entries to add, value to use) so it puts 865 x 2 zeros in. 
  
   
   $SS_esp =  $current_millenium - ($plot_hours * 3600); // - $yesterday_midnight_millenium;
   // $$SS_ esp = $minutes2 * 60 - ($plot_hours * 60 * 60);        // now (minutes2) - $plot_hours
                                                                  //
                                                                  // minutes2 is minutes since midnight, and is from small_test.txt. that value comes from the tablet and the last tank packet of 288 tank level values, in script post_288.php
                                                                  //
                                                                  // but when system fails, php looks at local server millenium and adjusts minutes2 to be approximately correct = birchwood time. so, if many hours are missed, minutes2 can
                                                                  // be greater than 24 hours x 60, and s tart_secs can be a large value


   $pixels_per_minute = (288 * $plot_stretch) / ($plot_hours * 60);

   $pixels_per_sec = $pixels_per_minute / 60;

   $final_old_yesterday = 0; // newest data already in php list

   $final_old_today = 0;


   include 'load_pulses.php';


         $new_slot = 0;
         $first_point = 0;

         $d1 = $tank_y3 + $tank_plot_h; // - (2 * $tankra[$first_point]);

         $px1 = $plot_x1;

         $gallons = 0;

         $y0 = $tank_y6;

         $full_scale = 60;

         $min_gpm = 30; // ignore below 3.0
           
         // added next block 9-23-2021

         $number_of_pulses_2021 = count($array_secs);

         //imagettftext($my_img->img, 12, 0,  $left_pad / 2, $tank_y6 + 40 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text   added this line 9-23-2021
         //imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y3 + $tank_y4) / 2 + 30 ,  $text_colour, $lib_font, "  x 1000");   // img, size, angle, x, y, color, font, text
         imagettftext($my_img->img, 6, 0,  10, ($tank_y3 + $tank_y4) / 2 + 150 ,  $text_colour, $lib_font, $number_of_pulses_2021);   // img, size, angle, x, y, color, font, text
         // end of added next block 9-23-2021

      
      if ($array_secs_cnt > 0){ // 9-23-2021 crashes if there are no pulses
         
                     
                     
                     
                     $y = $array_gpm[0]; // gpm x 10; 400 = 40

                     $yfact = $tank_plot_h / ($full_scale * 10); // divide by 10 because gpm is x 10 meaning 40.7 gpm will be 407

                     $oldy2 = $y0 - ($y * $yfact); // y pixel = bottom + value * plot_h / full scale value
                     
                     $oldx = ($array_secs[0] - $SS_esp) * $pixels_per_sec; //* 864/3600; /// 20; // seconds start at zero      3600 seconds/ 864 pixels

                     if ($oldx < 0) {

                        $oldx = 0;

                     }

                              
                     $y2 = $y0 - ($y * $yfact); // 
                        

                     if ($y > $min_gpm){
                        imageline( $my_img->img, (int)$oldx + $plot_x1 , (int)$y2,$plot_x1  , (int)$y2, $black );  // horizontal at new height. if the first pulse does not overlap the left border, this should draw nothing since y should be small
                     }



                     if ($local_debug_text == 1){

                        $m8 = "full scale $full_scale  yfact $yfact plotx1 $plot_x1 y0 $y0 cnt $array_secs_cnt pix $pixels_per_sec";

                        imagettftext($my_img->img, 12, 0,  $left_pad / 2, $tank_y6 + 40 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text

                        $m8 = "array_secs[0] which is skipped, why? $array_secs[0] array_secs size $array_secs_cnt";

                        imagettftext($my_img->img, 12, 0,  $left_pad / 2, $tank_y6 + 60 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text

                     }

                     $debug_cnt864 = 0;


                     // see xbee page 64 for explanation of coordinates vs seconds etc.

                     for ($i=1; $i< $array_secs_cnt; $i++){ // why does it start at 1? maybe because this is dealing with gpm which cannot be counted until 2 pulses have arrived? if this is true, 
                                                            // then I should use the entire array_secs size for total gallons instead of the size of array_864 which is gpm. 2-18-2021 the code in the other file that displays the total value on the screen
                                                            // adds 1 to the number of non-zero values found in array_864. I suppose I could just use the count of pulses in pulses.txt...


                        $x = ($array_secs[$i] - $SS_esp) * $pixels_per_sec;


                        if (($x ) > 0){

                              $y = $array_gpm[$i]; // gpm x 10; 400 = 40

                              $array_864[(int)$x] = (int)$y;


                              $y2 = $y0 - ($y * $yfact); // 

                              if ($y2 == $oldy2){

                                 imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$y2, (int)$x+$plot_x1  , (int)$y2, $black );    // horizontal line since value did not change 

                              } else {

                                 if ($y > $min_gpm){ 

                                    imageline( $my_img->img, (int)$oldx + $plot_x1 , (int)$y2, (int)$x+$plot_x1  , (int)$y2, $black );  // horizontal at new height
                                 }
                                 
                                 if ($oldx > 0){ // don't draw on top of the left border

                                    imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$oldy2, (int)$oldx+$plot_x1  , (int)$y2, $black );    // vertical at old pulse from old height to new height; this overwrites most of the final blue pulse in a sequence
                                 }
                              }

                              $oldx = $x;

                              $oldy2 = $y2;
                              
                        } else {


                              if ($debug_text == 1){

                                 $m88 = "skip $x";

                                 $debug_line = $debug_line + 20;

                                 imagettftext($my_img->img, 12, 0,  22, $debug_line ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

                              }


                        }

                     }


                     imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$oldy2, (int)$oldx+$plot_x1  , (int)$y0 - 2, $black );    // vertical at old pulse from old height to new height; this overwrites the entire final blue pulse

            } // if array_sec_cnt > 0. added this 9-23-2021
?>
