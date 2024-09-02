<?php

   final class bits {

      public $img;

      public function __construct($w,$h) {

         $this->img = imagecreate($w,$h);
      
      }

      public function __destruct() {
         
         if(is_resource($this->img)) {
            
            imagedestroy($this->img);
         }
      }

   }


   // three ways to read a file. 
   //
   // fgets(handle) returns one string
   //
   // file_get_contents(handle) returns entire file in one string
   //
   // file(handle) reads entire file into an array. 
   //
   // i suppose all of them work for most of my simple stuff. i want to have flow_cutoff integer in a file by itself called flow_cutoff.txt


   $fpmax = fopen("flow_lower_max.txt", "r");

   $flow_lower_max = fgets($fpmax);
   
   fclose($fpmax);

   $flow_lower_max_old = 55;  // 5-3-2023 this is from image i saved swap_lines_2023_4_11 on the day i swapped the wires and it made a huge change
   $flow_lower_good = 30;     // this is about what it was 3 weeks after i swapped the lines, on 5-3-2023
   $flow_lower_good2 = 23;     // swapped box 3 in and the lower line got even better 6-9-2023

   $flow_upper_minimum_new = 195;   // 5-19-2023 during rain after I spray painted the exposed part of the stainless steel screws. 204 after some rain
                                    // 8-27-2023 is it getting worse? reduce from 204 to 195


   $fpmin = fopen("flow_upper_minimum.txt", "r");

   $flow_upper_minimum = fgets($fpmin);
   
   fclose($fpmin);


   $fpcut = fopen("flow_cutoff.txt", "r");

   $flow_cutoff = fgets($fpcut); // this worked. file_get_contents($fpcut) seemed to return nothing. when i wrote it to log file, there was a linefeed at the end of $flow_cutoff, but the number still worked correctly:
   //
   //
   //
   //    >>>102         this is what the log file had, minus the tabs i added here, when i wrote >>>$flow_cutoff<<< to the file, so there was a linefeed in the variable that did not hurt it when i used it as an integer
   //    <<<

//   $flow_cutoff = file_get_contents($fpcut); failed. appeared to put nothing at all in $flow_cutoff, or maybe it put an array or something. at any rate fgets() works in a simple manner so use it

   fclose($fpcut);

   $flog = fopen("flow_log.txt","w");

   fwrite($flog,">>>$flow_cutoff<<<\n");

/*
   if ($flow_cutoff == 102){ // good

      fwrite($flog,"flow cutoff read ok\n");

   } else {
      
      fwrite($flog,"flow cutoff read failed. change to 200\n");

      $flow_cutoff = 200; // shows that the file read failed
   }
*/

   $data_is_old = 0; 

   $left_pad = 150;

   $right_pad = 150;

   $top_pad = 20;
   
   $bottom_pad = 50;

   $plot_w = 1440; 

   $plot_x1 = $left_pad;
   
   $plot_x2 = $plot_x1 + $plot_w;

   $tank_plot_h = 800;     

   $tank_y1 = 10; 
   
   $tank_y2 = $tank_y1 + $tank_plot_h; 

   $img_w = $plot_w + $left_pad + $right_pad;     // entire img width

   $img_h =  $top_pad + $bottom_pad + $tank_plot_h;

   $my_img = new bits( $img_w, $img_h );
   
   imagesetthickness ( $my_img->img, 1 ); // affects line drawing

   $rwb_background = imagecolorallocate( $my_img->img, 255, 255, 255 ); //  The first call to imagecolorallocate() fills the background color in palette-based images - images created using imagecreate(). 

   $pale_grey = imagecolorallocate( $my_img->img, 200,200,200 );

   $black = imagecolorallocate( $my_img->img, 0,0,0 );
   
   $red = imagecolorallocate( $my_img->img, 255,0,0 );
   
   $blue = imagecolorallocate( $my_img->img, 0, 0, 255);
   
   $green = imagecolorallocate( $my_img->img, 0, 255, 0);
   $orange = imagecolorallocate( $my_img->img, 200, 200, 0);
   $orange33 = imagecolorallocate( $my_img->img, 110, 250, 77);
   $orange2 = imagecolorallocate( $my_img->img, 200, 0,200);
   $orange3 = imagecolorallocate( $my_img->img, 100,200,255);
   $blue3 = imagecolorallocate( $my_img->img, 170,170,215);
   
   $lib_font = 'LiberationSans-Bold.ttf';
   
   // ---- horizontal lines 

   $lines = 10; // use 10 for 0..100, 6 for 0..60, etc.
   
   $pc_y = $tank_y1;
   
   $dy = ($tank_y2 - $tank_y1) / $lines;

   $label_delt = 256 / 10;  // put 10 here for 10 at the next to the bottom line and 20 30 40; put 200 here for 200 400 600


$xx=0;
   for ($x = 0; $x < $lines; $x++){
   
      if ($x > 0){
         imageline( $my_img->img, $plot_x1 + $xx, $pc_y, $plot_x2, $pc_y, $pale_grey );
      }

      if ($x < ($lines - 1)){  // example lines = 7: the last label will be x < 6 == x = 5

         $s = sprintf("%d",(($lines - 1) * $label_delt) - ($x * $label_delt));   // example lines = 7: the next to the top line is the first one labeled: (7-1) x 10 - (0 x 10) = 60. the next to the bottom line will be labeled (7-1)x10 - (5x10) = 60 - 50 = 10
        
         $size = 12;

         imagettftext($my_img->img, $size, 0,  $plot_x2 + 10, $pc_y + $dy + $size/2 ,  $black, $lib_font, $s); 
      }

      $pc_y = $pc_y + $dy;
   }





   include "read_time_file.php";
   /*
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

   if ($server_hour < 5){

      $minutes2 = $server_minute + 60 * ($server_hour + 19); // est is 5 hours different. minutes2 is only used to show the time legends. I don't think it is ever used for anything else. if so, why? - in this file i use it to wrap the flow adc data
   
   } else {
      
      $minutes2 = $server_minute + 60 * ($server_hour - 5); // est is 5 hours different
   }

   // all the code below this line was added in 2020 so I can show blanks in status and tank level and other plots if data is more than 5 minutes old

   $dra33 = getdate();
   
   $current_millenium = $dra33[0];

   $secs_since_last_packet = $current_millenium - $millenium;     // NOTE THAT old files calculate seconds_since_last_packet, so I might want to change the name here, and get rid of the old calcs

   // this returns a float if any remainder from integer division:     $minutes_since_last_packet = $secs_since_last_packet / 60;
   
   $minutes_since_last_packet = intdiv($secs_since_last_packet,60);

//   $minutes_since_last_packet = 15 * 60;
*/
/*
   if ($minutes_since_last_packet > 15){     // >>>>>>>>>>> make sure this agrees with all the other files that show the text warning. they compare to seconds, so this might be off by one minute which is ok <<<<<<<<<

      $data_is_old = 1;

      $shift_arrays_by_this_much = intdiv($minutes_since_last_packet,5);

      $pivot_index = 288 - $shift_arrays_by_this_much;

      $minutes2 = $minutes2 + $shift_arrays_by_this_much * 5;      // don't use minutes_since_last packet here. this way it will be 5 minute blocks

   } else {

      $data_is_old = 0;
   }
*/
// -------------------- vertical hour lines ------------------------------------------

   $hour3 = (int) ($minutes2 / 60);    // current hour; if minutes > 0, the first hour line on the left will be this hour + 1

   $mins3 = $minutes2 - ($hour3 * 60);    // current minutes. the first hour line will be 60  - this minutes from the left border

   $h23 = $hour3 + 1;

   while ($h23 > 12) {

      $h23 = $h23 - 12;
   }

   $x33 = $plot_x1 + ((60 - $mins3));    // initially this will be the first hour line, then it will be incremented for each hour line. this plot has one minute per pixel = 24 x 60 = 1440 pixels and 1440 data points

   $dx33 = 60;     // this is one hour of pixels

   for ($x = 0; $x < 24; $x++){
      
      imageline( $my_img->img, $x33, $tank_y1, $x33, $tank_y2, $pale_grey);            // tank plot
         
      $s = sprintf("%2d",$h23);

      $h23 = $h23 + 1;

      while ($h23 > 12) {

         $h23 = $h23 - 12;
      }
            
      imagettftext($my_img->img, 12, 0,  $x33 - 10, $tank_y2 + 16 ,  $black, $lib_font, $s);   // img, size, angle, x, y, color, font, text
      
      $x33 = $x33 + $dx33;
   }

   // flow adc plot ------------------------------------------------------------------
  
  /*
      tank plot is simple because everything is done in UTC epoch, esp and php, and the left end of the plot is simply "now" - 86400 seconds. minutes2 is adjusted for dst just to put the right numbers in the legend, and has
      no effect on plotting tank level or anything else. all the data is UTC based.

      except for flow adc which is saved in esp by the minute, 1440 values for 24 hours, and that array is sent as is, and this code finds the starting point. so, this code has to adjust for dst:

         esp stores flow adc per todays_seconds(). right now my clock says 7:07 pm DST, and that's what the flow adc plot legend says, but the data is off by 1 hour.

            todays_seconds = 65296 which is 18:08:16, which is 6:08, not DST, but EST, so it looks like esp never considers dst at all. i use a button to set dst_flag on php, but that should be automated eventually.
         
         in the winter, data stored at 0 is midnight on esp and midnight on php and is midnight on the legend

         in the summer, data stored at 0 is midnight EST and 1 am DST; when that data is stored, the clock on the wall says 1 am

            so, if dst_flag is set, the code here needs to subtract 60 from minutes2


  */
   imagesetthickness ( $my_img->img, 2 ); // affects line drawing

   $f2= fopen("flow_adc_data.txt", "r");

   $d2 = explode(" ",trim(fgets($f2)));

   $s2 = count($d2);

   fclose($f2);

   $flows = array();

   foreach ($d2 as $val){     // this gives us 1440 values that range from 0 to 255,  flow is 1 data per minute

      $flows[] = hexdec($val);
   }

   $x1 = $plot_x1;

   // the data is sent from esp as midnight..midnight, so we need to start at the current minute and go to the end, then resume at the beginning and go to the current minute

   if ($dst_flag == 1){ // this flag adds 60 to minutes2 in read_time_file so we have to undo that here since esp ignores dst. the legend will be correct because it uses minutes2 before this correction

      $current_data = $minutes2 - 60;
      
      if ($current_data < 0){ // this only happens the first hour after midnight, so I'll probably never test it. jay doesn't see this plot anyway.
      
         $current_data = $current_data + 1440;
      }

   } else {    // 11-8-2021 dst bug + php 8.0 needs to init current_data

      $current_data = $minutes2;

   }

   $y_ratio = $tank_plot_h / 256.0;

   $y1 = $tank_y1 + $tank_plot_h - ($flows[$current_data] * $y_ratio);

   for ($i=($current_data + 1);$i<$s2;$i++){

      $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

      $x2 = $x1 + 1.0; 
      
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

      $x1 = $x2;
      $y1 = $y2;
   }

   for ($i=1;$i<$current_data;$i++){

      $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

      $x2 = $x1 + 1.0; 
      
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

      $x1 = $x2;
      $y1 = $y2;
   }
/*
   $cutoff = $tank_y1 + $tank_plot_h - (242 * $y_ratio);

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 


   $cutoff =  $tank_y1 + $tank_plot_h - (230 * $y_ratio);

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 


   $cutoff =  $tank_y1 + $tank_plot_h - (204 * $y_ratio);

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 


   $cutoff =  $tank_y1 + $tank_plot_h - (76 * $y_ratio);

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 


   $cutoff =  $tank_y1 + $tank_plot_h - (51 * $y_ratio);

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 

*/
   
   $upper_minimum =  $tank_y1 + $tank_plot_h - ($flow_upper_minimum * $y_ratio);
   imageline( $my_img->img, $plot_x1, $upper_minimum, $plot_x2, $upper_minimum, $blue ); 
   $s = sprintf("%d",($flow_upper_minimum));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $upper_minimum,  $blue, $lib_font, $s); 
   $s = sprintf("rain upper min before spray painted screws");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $upper_minimum + 30,  $blue, $lib_font, $s); 
   
/*   
   $lower_max =  $tank_y1 + $tank_plot_h - ($flow_lower_max * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $green ); 
   $s = sprintf("%d",($flow_lower_max));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $green, $lib_font, $s); 
*/

   //$flow_lower_max_old = 55;  // 5-3-2023 this is from image i saved swap_lines_2023_4_11 on the day i swapped the wires and it made a huge change
   //$flow_lower_good = 30;     // this is about what it was 3 weeks after i swapped the lines, on 5-3-2023


   $lower_max =  $tank_y1 + $tank_plot_h - ($flow_lower_good2 * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $orange33 ); 
   $s = sprintf("%d",($flow_lower_good2));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $orange33, $lib_font, $s); 


   $lower_max =  $tank_y1 + $tank_plot_h - ($flow_lower_good * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $orange ); 
   $s = sprintf("%d",($flow_lower_good));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $orange, $lib_font, $s); 


   $lower_max = $tank_y1 + $tank_plot_h - ($flow_lower_max_old * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $orange2 ); 
   $s = sprintf("%d",($flow_lower_max_old));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $orange2, $lib_font, $s); 



   $lower_max = $tank_y1 + $tank_plot_h - ($flow_upper_minimum_new * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $orange3 ); 
   $s = sprintf("%d",($flow_upper_minimum_new));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $orange3, $lib_font, $s); 
   $s = sprintf("rain min after spray painted screws. was 204 initially");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $lower_max + 30,  $orange3, $lib_font, $s); 



   $lower_max = $tank_y1 + $tank_plot_h - (150 * $y_ratio);
   imageline( $my_img->img, $plot_x1, $lower_max, $plot_x2, $lower_max, $blue3 ); 
   $s = sprintf("%d",150);
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $lower_max,  $blue3, $lib_font, $s); 
   $s = sprintf("11-22-2023 heavy rain maybe leaves");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $lower_max + 30 ,  $blue3, $lib_font, $s); 




   $cutoff =  $tank_y1 + $tank_plot_h - ($flow_cutoff * $y_ratio);
   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 

   // 4-10-2023 show values for the three lines (5-3-2023 move some up there to show additional values using same var - lower_max


   



   $s = sprintf("the actual threshold value");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 570, $cutoff + 30,  $red, $lib_font, $s); 
   $s = sprintf("%d",($flow_cutoff));
   imagettftext($my_img->img, $size, 0,  $plot_x2 + 70, $cutoff,  $red, $lib_font, $s); 

   



   $s = sprintf("the plot got this high before i swapped the wires");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $cutoff - 30,  $orange2, $lib_font, $s); 


   $s = sprintf("the plot was about here right after i swapped the wires");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $cutoff + 30,  $orange, $lib_font, $s); 


   $s = sprintf("the plot was about here right after i changed to box 3");
   imagettftext($my_img->img, $size, 0,  $plot_x1 + 70, $cutoff + 60,  $orange33, $lib_font, $s); 

   // ---------------  box around tank plot ------------------------------------------

   imageline( $my_img->img, $plot_x1, $tank_y1, $plot_x2, $tank_y1, $black );                // horizontal lines above and below tank plot
   imageline( $my_img->img, $plot_x1, $tank_y2, $plot_x2, $tank_y2, $black );

   imageline( $my_img->img, $plot_x1, $tank_y1, $plot_x1, $tank_y2, $black );                // vertical lines at ends of tank plot
   imageline( $my_img->img, $plot_x2, $tank_y1, $plot_x2, $tank_y2, $black );

   header( "Content-type: image/png" );

   imagepng( $my_img->img );

   imagedestroy( $my_img->img );

   fclose($flog);


?>
