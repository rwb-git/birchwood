<?php

function draw_red_or_black($x1, $y1, $x2, $y2){

   global $red_pix, $plot_x1, $red, $black, $my_img;

//   if ($red_pix[(int)(($x1 + $x2) / 2) - (int)$plot_x1] == 1){     // check the average of x1 and x2 instead of just x1. if the float is on in the midpoint of a segment that might work better than the first point, because
                                                         // sometimes the float is on a while after the tank hits 100, but then there should be a long line at 100 where the float will be off in the middle.
//   if ($red_pix[$x1 - $plot_x1] == 1){

  //    imageline( $my_img->img, $x1, $y1, $x2, $y2, $red );
 //  } else {

      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );
 //  }
}

//--------------------------------------------------------- tank plot ----------------------------------------------------





   $use_tank_data = 0; // development code to be used until air sensor is ready




   imagesetthickness ( $my_img->img, 2 );


// load tank_level.txt which looks like this
//   1613415600 90 1613415783 91 1613415844 92 1613415966 94 1613416089 95 1613416202 97 1613416263 98 1613416385 100 

   if ($use_tank_data == 1){

      $_fp = fopen('tank_level.txt', "r");      // 0..100 percent of tank level, already converted from adc

   } else {

      $_fp = fopen('air_level.txt', "r");       // 0..1023 raw adc 10 bit

   }


   $numbers = explode(" ", trim(fgets($_fp)));   // milen state milen state

   fclose($_fp);

   $level = array();

   $minutes = array();

   $index = 0;

   $dra33 = getdate();
   
   $current_millenium = $dra33[0];
   
   $zero_hour = $current_millenium - 86400; // actually zero second lol

   $good_one = 0;

   $good_cnt = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $index = 1;

         $plot_mins = intval(($number - $zero_hour) / 60);

         if ($plot_mins >= 0){

            $good_one = 1;


         } else {

            $good_one = 0;

         }

      } else {

         $index = 0;

         if ($good_one == 1){

            if ($use_tank_data == 1){

               $m2 = (429.66 - 265.98) / (100.0 - 64.0);

               $b2 = 429.66 - $m2 * 100.0;

               $temp1 = ($number * $m2) + $b2 ;      // 0..100 scaled to 0.1023 so the same math will apply that will eventually be used for air sensor, also have to account for
               //$temp1 = ($number * 164.0/100.0) + 265.0 ;      // 0..100 scaled to 0.1023 so the same math will apply that will eventually be used for air sensor, also have to account for
                                                // the pressure sensor being 0 at 0.5 and 100 at 4.5, and tank values range from 64 to 100 normally, so I want 64 to be 20 and 100 to be 40

               $level[] = $temp1;
               //$level[] = $number / 2 - 5;      // 0..100, so I scaled it down to fit the air plot 50 max

            } else { // 0..1023 adc

               $level[] = $number;     // 0..1023

            }




            
               $minutes[] = $plot_mins;

               $good_cnt++;

         }
      }
   }


   // see air_pressure.ods for discussion of offsets and proof that they work, and best explanation in xbee 68.11


   $m = (40.0 + $offset_H - (20.0 + $offset_L)) / (429.66 - 265.98); // 429 is ideal adc for 40 psi, 265 for 20 psi

   $b = $offset + (20.0 + $offset_L) - $m * 265.98;

   for ($i=0;$i<$good_cnt;$i++){

      $level[$i] = $level[$i] * $m + $b;
   }



$val = 0;


$old_offset = -(300/4); //10 * $fact73;  // offset is applied after scaling, so it's 10 * fact

$fact73 = (300-$old_offset) / 50;   // this was 5 for 300 tall and 0..60, since 300/60 = 5. new range is 10-50 so 300 / 40 = 7.5, with an offset of 10 x 7.5
               // factor is for 300 pixels -> 10 to 50, or 40 points, so its 300 / 40 = 7.5
               // 
if ($good_cnt >2){

   $factor = 864.0 / 1440.0; // convert minutes in 24 hours to pixels

   $x1 = $plot_x1 + intval((float)$minutes[0] * $factor);
   $x2 = $plot_x1;
      
   $y1 = intval($tank_y1 + 300 - $old_offset -($fact73 * $level[0] )); // fact73 was 2 for old tank 200 tall, 3.33 for air 200 tall, 5 for air 300 tall
   
   // tank was 0..100. air will be 10..50, for a range of 40 with offset of 10
   
   $y2 = $y1;

   //$y1 = $tank_y1 + 200 - (2 * $level[0]);  old tank values were 0..100, air will be 0..60?

   if ($x1 > $plot_x1){    // quite often the first data point will not be exactly at the left margin, so draw a line to it

      //draw_red_or_black($plot_x1, $y1, $x1, $y1);
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );
   }

   for ($i = 1; $i < ($good_cnt); $i++){    //------------------------------- plot tank level graph ------------------------------------------

      // don't let it droop if value is 100

      if ($level[$i-1] == 50){
         
         $x2 = $plot_x1 + intval((float)$minutes[$i] * $factor);
            
         $y2 = intval($tank_y1 + 300 - $old_offset - ($fact73 * $level[$i]));

            imageline( $my_img->img, $x1, $y1, $x2-1, $y1, $black );
            
            imageline( $my_img->img, $x2-1, $y1, $x2, $y2, $black );
         
      } else { // normal plot

         // issue: it was steady at 75 for several hours but new plot droops the whole way. should I check for large separation on x axis and add another point so it does not droop? the tank
         // of course cannot do that, but the sensor does sometimes drift over time. when the sensor does drift, this is going to make the line jagged

         $x2 = $plot_x1 + intval((float)$minutes[$i] * $factor);
            
         $y2 = intval($tank_y1 + 300 - $old_offset - ($fact73 * $level[$i]));
         //$y2 = intval($tank_y1 + 300 - ($fact73 * ($level[$i] - $offset)));

         if (($x2 - $x1) > 20){              // if x2 - x1 is large add another point. 20 pixels x 1440 minutes / 864 pixels = 33 minutes. in normal operation the pump is always off for at least an hour

      //      draw_red_or_black($x1, $y1, $x2 - 3, $y1);
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

            $x1 = $x2 - 2;
         }
         
      //   draw_red_or_black($x1, $y1, $x2, $y2);
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );
      }
   
      $x1 = $x2;
      $y1 = $y2;
      
      $val = $level[$i]; // save final value
   }

   if ($x2 < ($plot_x1 + 864)){  // extend final point to right margin. the following screwed up and drew it red until additional tank values were posted, but that's because the float was on for a while after tank hit 100. i changed draw_red_or_black
                                 // to check the midpoint instead of the first pixel, which seems to have fixed this case.

                                 // tank level  92 1613520349 94 1613520410 95 1613520532 97 1613520654 98 1613520715100 

                                 // float 1613517797 11613520917 0 
      if ($x1 > $plot_x1){

         if ($final_float_status > 0){ // make it red. draw_red_or_black() sometimes makes the last tiny bit black, so force it here

            imageline( $my_img->img, $x1, $y1, $plot_x1 + 864, $y2, $red );

         } else {
            
            draw_red_or_black($x1, $y1, $plot_x1 + 864, $y2);
         }
      }
   }
}

$big_num_x = $plot_x1 + 864 + 40;


if ($data_is_old == 1){
   
   $s = sprintf("??");

   imagettftext($my_img->img, 32, 0, $big_num_x   , ($tank_y1 + $tank_y2)/2,  $red, $lib_font_reg, $s);   // img, size, angle, x, y, color, font, text

} else {

   $s = sprintf("%d",$val);

   imagettftext($my_img->img, 32, 0, $big_num_x , ($tank_y1 + $tank_y2)/2,  $text_colour, $lib_font_reg, $s);   // img, size, angle, x, y, color, font, text
}


?>
