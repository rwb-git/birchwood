<?php


   include 'draw_top_plots_part_1.php';

   $img_h = 30 + $sep*4 + 5 * $bits_h + $top_pad + $bottom_pad + $tank_plot_h*3;                                // note that this is not same for simple and huge web pages
   
//   $img_h = 20 + $sep*4 + 5 * $bits_h + $top_pad + $bottom_pad + $tank_plot_h*4;  // the old img with red tick shit

   include 'draw_top_plots_part_2.php';




// 12-12-2023 i don't use y3 and y4 anymore but they are used anyway, so fuckit this seems to fix the errors
   $tank_y3 = $sep*2 + $tank_y2; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y4 = $tank_y3 + $tank_plot_h;      // bottom of tank plot
/*   
   
   $tank_y5 = $sep*2 + $tank_y4; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y6= $tank_y5 + $tank_plot_h;      // bottom of tank plot


   $tank_y7 = $sep*2 + $tank_y6; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y8= $tank_y7 + $tank_plot_h;      // bottom of tank plot
*/

//   $tank_y3 = $sep*2 + $tank_y2; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
//   $tank_y4 = $tank_y3 + $tank_plot_h;      // bottom of tank plot
   
   
   $tank_y5 = $sep*2 + $tank_y2; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y6= $tank_y5 + $tank_plot_h;      // bottom of tank plot


   $tank_y7 = $sep*2 + $tank_y6; //op_pad + 5 * $bits_h + $tank_plot_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y8= $tank_y7 + $tank_plot_h;      // bottom of tank plot

$debug_line = $img_h;

   if ($debug_text == 1){
      
      $debug_line = $img_h; // start debug text here

      $img_h = $img_h * 4;
   }



//   --- horizonta lines for gallons total plot ------------

   $lines = 10; // use 10 for 0..100, 6 for 0..60, etc.
   $pc_y = $tank_y3;
   $dy = ($tank_y4 - $tank_y3) / $lines;


   $label_delt = 2;  // put 10 here for 10 at the next to the bottom line and 20 30 40; put 200 here for 200 400 600

   imagesetthickness ( $my_img->img, 1 );
//   draw_horizontal_grid();

// ------------------ horizontal percent lines for gpm -----------------------------------------------------------------
  


   $lines = 6; // use 10 for 0..100, 6 for 0..60, etc.
   $pc_y = $tank_y5;
   $dy = ($tank_y6 - $tank_y5) / $lines;

   $label_delt = 10;

  draw_horizontal_grid();



// ------------------ horizontal percent lines for gallons per day -----------------------------------------------------------------
  


   $lines = 10; // use 10 for 0..100, 6 for 0..60, etc.
   $pc_y = $tank_y7;
   $dy = ($tank_y8 - $tank_y7) / $lines;

   $label_delt = 2;

  draw_horizontal_grid();



// -------------------- vertical hour lines for the new plots needs to be done down here ------------------------------------------

   $h23 = $hour3 + 1;

   while ($h23 > 12) {

      $h23 = $h23 - 12;

   }

   $x33 = $plot_x1 + ((60 - $mins3) * $plot_stretch / 5);    // 5 minutes per packet

   for ($x = 0; $x < 24; $x++){

//      imageline( $my_img->img, (int)$x33, $tank_y3, (int)$x33, $tank_y4, $pale_grey);            // tank plot
      imageline( $my_img->img, (int)$x33, $tank_y5, (int)$x33, $tank_y6, $pale_grey);            // tank plot
         
      $s = sprintf("%2d",$h23);

      $h23 = $h23 + 1;

      while ($h23 > 12) {

         $h23 = $h23 - 12;

      }
      
//      imagettftext($my_img->img, 12, 0,  (int)$x33 - 10, $tank_y4 + 16 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text
      
      imagettftext($my_img->img, 12, 0,  (int)$x33 - 10, $tank_y6 + 16 ,  $text_colour, $lib_font, $s);   // adding this line of text added 353 bytes
      
      $x33 = $x33 + $dx33;

   }
   

// --------------------- line at very top

   $debug_line_sav = $debug_line;
  
   imagesetthickness ( $my_img->img, 2 );

   include 'pulse_stuff_33_fake.php';

      $new_slot = 0;
      $first_point = 0;
      $last_point = 287;



      $d1 = $tank_y3 + 200; // - (2 * $tankra[$first_point]);

//      $last_good_val = $tankra[$first_point];

      $px1 = $plot_x1;

      $gallons = 0;


/*

// ------------ draw the plot showing gallons per day that starts at zero and grows 100 gallons at each pulse.  ------------------------------------------

   if ($debug_text == 1){
      
     
      $m88 = "mk_pulse.. revision 77";

      imagettftext($my_img->img, 12, 0,  522, 350 ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text


   }



      //$ra_size = count($array_864); // ra_size not used or needed any more since I added sane_index

      if ($debug_text == 1){

         $m88 = "minutes2 before plotting red pulse ticks $minutes2  ";

         $debug_line_sav = $debug_line_sav + 20;

         imagettftext($my_img->img, 12, 0,  722, $debug_line_sav ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

      }

         // see xbee page 64 for explanation of coordinates vs seconds etc.
         // see xbee page 64 for explanation of coordinates vs seconds etc.
         // see xbee page 64 for explanation of coordinates vs seconds etc.


      for ($sane_index = 0; $sane_index<864; $sane_index++){
                                                                           //
                                                                           // 24 x 60 = 1440 minutes in one day
                                                                           //
                                                                           // 1440 * 0.6 = 864 which is the number of data points in array_864? 24 x 60 / 864 = 
      
                                                                           // $array_864 = array_fill(0,865 * 2 ,0); // 48 hours. 864 = 288 *3 and corresponds to pixels if stretch is 3
                                                                           // array_fill(start index, number of entries to add, value to use) so it puts 865 x 2 zeros in. 
                                                                           //
                                                                           // just after midnight, minutes2 is very low, so this starts at zero and goes to 1439, which is the number of pixels to plot for plot_stretch = 3, 3 x 288 x 2 days = 
                                                                           //
                                                                           // one day = 24 hours x 60 = 1440 minutes which is what the big plot shows
                                                                           //
                                                                           // one packet every five minutes = 1440 / 5 = 288 packets in 24 hours. this is what tank data and status is. pulses can come at any time and are timestamped to the second after midnight
                                                                           //
                                                                           // so, if plot stretch = 3 which it always has been, the plot is 288 x 3 = 864 pixels wide, and each pixel is for 5/3 minutes = 10/6 minutes
                                                                           //
                                                                           // array_864 seems to be for two complete days, so if the current time today is minutes2, then that corresponds to this time 24 hours ago, which is where we want to start our plot
                                                                           //
                                                                           // example: minutes2 = 120 minutes = 2 am today, but in the array it will be 2 am yesterday: 120 * 0.6 = 72, the index of the first GPM value
                                                                           //
                                                                           // ignore that for now, since it works under normal conditions. but i'm looking at system failure. right now, in my test, it has been dead for about 16 hours and
                                                                           // minutes2 is 1960, and it looks like this code will have an array index out of bounds at index = 865*2 - 1 = 1729
                                                                           //
                                                                           //    (1960 + 1439) * 0.6 = 2039 so it's already too big. it starts at 1960 x 0.6 = 1176

         $px2 = $px1 + 1;
            
         $val = $array_864[$sane_index];     // val is GPM x 10, so it shouldn't be much higher than about 600 if pumps don't change


         if (($val > 800) || ($val < 0)){ // what is actual max value going to be? my test file has some shit values like 6666 and 15000

            $val = 0;
         }

         if ($val > 0) {

            $gallons ++; //= 100;


            if ($debug_text == 1){

               $m88 = "val $val   index $sane_index  ";

               $debug_line_sav = $debug_line_sav + 20;

               imagettftext($my_img->img, 12, 0,  722, $debug_line_sav ,  $text_colour, $lib_font, $m88);   // img, size, angle, x, y, color, font, text

            }

         }
         
         $d2 = $tank_y4 - $gallons;  // 20,000 is full scale

            imageline( $my_img->img, $px1, $d1, $px2, $d2, $black );
            
            if ($val > 0){
               imageline( $my_img->img, $px2, $d2 -20, $px2, $d2-30, $red );  // red tick mark for each pulse
            }


         $px1 = $px2;

         $d1 = $d2;

      }
      
      $s = sprintf("%.1f",($gallons + 1)/10.0); // add 1 to account for the first pulse which is not in array_864
                                                // 2-18-2021 I counted the pulses in pulses_count_example.txt which has 144. old code is showing 14.5, new code is showing 14.4, so I suppose the new code is correct. BUT WAIT. i
                                                // downloaded the old pulse file and it has 145 pulses, so the old code is correct and somehow esp missed one pulse. maybe due to me programming while pulses were coming in, or esp just missed one.

      imagettftext($my_img->img, 22, 0, $px2 + 40  , ($tank_y3 + $tank_y4)/2,  $text_colour, $lib_font_reg, $s);   // img, size, angle, x, y, color, font, text

*/










// show gallons per day in text instead of the old red tick shit



      for ($sane_index = 0; $sane_index<864; $sane_index++){
                                                                           //
                                                                           // 24 x 60 = 1440 minutes in one day
                                                                           //
                                                                           // 1440 * 0.6 = 864 which is the number of data points in array_864? 24 x 60 / 864 = 
      
                                                                           // $array_864 = array_fill(0,865 * 2 ,0); // 48 hours. 864 = 288 *3 and corresponds to pixels if stretch is 3
                                                                           // array_fill(start index, number of entries to add, value to use) so it puts 865 x 2 zeros in. 
                                                                           //
                                                                           // just after midnight, minutes2 is very low, so this starts at zero and goes to 1439, which is the number of pixels to plot for plot_stretch = 3, 3 x 288 x 2 days = 
                                                                           //
                                                                           // one day = 24 hours x 60 = 1440 minutes which is what the big plot shows
                                                                           //
                                                                           // one packet every five minutes = 1440 / 5 = 288 packets in 24 hours. this is what tank data and status is. pulses can come at any time and are timestamped to the second after midnight
                                                                           //
                                                                           // so, if plot stretch = 3 which it always has been, the plot is 288 x 3 = 864 pixels wide, and each pixel is for 5/3 minutes = 10/6 minutes
                                                                           //
                                                                           // array_864 seems to be for two complete days, so if the current time today is minutes2, then that corresponds to this time 24 hours ago, which is where we want to start our plot
                                                                           //
                                                                           // example: minutes2 = 120 minutes = 2 am today, but in the array it will be 2 am yesterday: 120 * 0.6 = 72, the index of the first GPM value
                                                                           //
                                                                           // ignore that for now, since it works under normal conditions. but i'm looking at system failure. right now, in my test, it has been dead for about 16 hours and
                                                                           // minutes2 is 1960, and it looks like this code will have an array index out of bounds at index = 865*2 - 1 = 1729
                                                                           //
                                                                           //    (1960 + 1439) * 0.6 = 2039 so it's already too big. it starts at 1960 x 0.6 = 1176

            
         $val = $array_864[$sane_index];     // val is GPM x 10, so it shouldn't be much higher than about 600 if pumps don't change


         if (($val > 800) || ($val < 0)){ // what is actual max value going to be? my test file has some shit values like 6666 and 15000

            $val = 0;
         }

         if ($val > 0) {

            $gallons ++; //= 100;
         }
      }
      
      $s = sprintf("Gallons in last 24 hours: %.1f",($gallons + 1)*100.0); // add 1 to account for the first pulse which is not in array_864
                                                // 2-18-2021 I counted the pulses in pulses_count_example.txt which has 144. old code is showing 14.5, new code is showing 14.4, so I suppose the new code is correct. BUT WAIT. i
                                                // downloaded the old pulse file and it has 145 pulses, so the old code is correct and somehow esp missed one pulse. maybe due to me programming while pulses were coming 
                                                // in, or esp just missed one.

      imagettftext($my_img->img, 22, 0, $px1  , $tank_y8 + 70,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text

















// THIS ASSUMES TANK PLOT IS 200 PIXELS TALL. IF THAT IS CHANGED, THIS HAS TO ADJUST TO FIT.

//imageline( $my_img->img, $plot_x1, $tank_y3, $plot_x2, $tank_y3, $black );                // horizontal lines above and below tank plot
//imageline( $my_img->img, $plot_x1, $tank_y4, $plot_x2, $tank_y4, $black );

//imageline( $my_img->img, $plot_x1, $tank_y3, $plot_x1, $tank_y4, $black );                // vertical lines at ends of tank plot
//imageline( $my_img->img, $plot_x2, $tank_y3, $plot_x2, $tank_y4, $black );



imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x2, $tank_y5, $black );                // horizontal lines above and below tank plot
imageline( $my_img->img, $plot_x1, $tank_y6, $plot_x2, $tank_y6, $black );

imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x1, $tank_y6, $black );                // vertical lines at ends of tank plot
imageline( $my_img->img, $plot_x2, $tank_y5, $plot_x2, $tank_y6, $black );

imageline( $my_img->img, $plot_x1, $tank_y7, $plot_x2, $tank_y7, $black );                // horizontal lines above and below tank plot
imageline( $my_img->img, $plot_x1, $tank_y8, $plot_x2, $tank_y8, $black );

imageline( $my_img->img, $plot_x1, $tank_y7, $plot_x1, $tank_y8, $black );                // vertical lines at ends of tank plot
imageline( $my_img->img, $plot_x2, $tank_y7, $plot_x2, $tank_y8, $black );








// --------------------- line at very top


//   imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y3 + $tank_y4) / 2 ,  $text_colour, $lib_font, "  Gallons");   // img, size, angle, x, y, color, font, text
   
//   imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y3 + $tank_y4) / 2 + 30 ,  $text_colour, $lib_font, "  x 1000");   // img, size, angle, x, y, color, font, text
   
   imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y7 + $tank_y8) / 2 ,  $text_colour, $lib_font, "  Gallons");   // img, size, angle, x, y, color, font, text
   
   imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y7 + $tank_y8) / 2 + 30 ,  $text_colour, $lib_font, "  x 1000");   // img, size, angle, x, y, color, font, text
   
   imagettftext($my_img->img, 12, 0,  $left_pad / 2, ($tank_y5 + $tank_y6) / 2 ,  $text_colour, $lib_font, "  GPM");   // img, size, angle, x, y, color, font, text


   include 'gallons_per_day_fake.php';








   //-------------------------------------------------------------------------------------------------------------------------

   imagesetthickness ( $my_img->img, 5 );

   header( "Content-type: image/png" );

   imagepng( $my_img->img );

   imagedestroy( $my_img->img );


?> 
