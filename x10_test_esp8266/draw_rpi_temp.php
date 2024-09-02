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


   /* 5-30-3022 plot never seems to change, so save old data in a new file for comparison

         -rw-rw-r-- 1 user user  9262 Aug 12  2021 draw_rpi_temp.php
         -rw-rw-r-- 1 user user  9262 Aug 12  2021 draw_rpi_temp_old_5_30_2022.php     <<< this was before changing this file on 5-30-2022

   
   $handle= fopen("rpi_temp_data.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$inp);

   fclose($handle);

FF 7C 7F 7A 76 6D 73 72 72 6B 77 77 6E 70 6B 6D 67 67 68 6C 71 71 69 6D 70 71 76 6B 71 6C 6C 71 72 71 77 79 79 6E 00 00 00 00 79 7E 7E 7E 6F 71 78 7B 7B 76 76 7B 79 7D 7D 77 6C 6C 77 7D 7F 84 84 7F 84 81 7B 72 6C 76 70 71 7A 7B 7B 7C 80 84 7C 7F 80 78 7B 7D 77 76 76 73 7D 7A 7D 7D 7D 7D 81 88 88 7B 71 7B 84 84 88 86 84 83 84 76 79 7D 82 88 86 8C 8D 8D 81 82 82 7A 84 88 8B 8A 8A 83 83 71 81 84 81 7D 86 84 8A 8C 89 8A 90 92 8E 8D 8B 88 84 8B 8B 8B 90 7B 86 8A 84 84 8B 8F 8A 90 8D 8F 8B 90 89 8C 8E 8F 8D 8C 8E 90 88 91 88 88 88 84 8D 8E 90 91 92 92 88 88 8C 90 91 92 88 90 90 90 92 92 94 92 92 94 8E 88 8B C7 8D 8E 8C 90 90 91 92 92 96 C4 8F 8E 84 88 8E 8C 86 8E 90 92 93 94 94 92 8D 8E 8B 8F 92 92 8E 8E 92 93 95 92 92 93 93 93 91 8B 8A 88 88 8C 88 8E 8A 86 86 86 89 8C 8E 8F 89 88 8A 8F 8B 84 88 84 82 83 83 86 84 86 86 86 86 89 86 86 88 83 83 80 81 81 81 84 86 88 86 86 79 7D 7E 7F 80 7D 7F 82 82 79 7A 7B 7B 7A 79 79 72 6B 6C 6A 73 78 7F 7E 7B 00 7E 77 71 74 7B 7C 73 6F 72 72 72 79 79 7A 7B 7B 79 79 79 7B 7C 7A 77 7C 72 6B 67 6D 7D 72 71 72 76 78 7F 77 77 76 64 6F 6F 73 79 7B 76 7F 7B 77 

*/


   // three ways to read a file. 
   //
   // fgets(handle) returns one string
   //
   // file_get_contents(handle) returns entire file in one string
   //
   // file(handle) reads entire file into an array. 
   //
   // i suppose all of them work for most of my simple stuff.


   $flog = fopen("rpi_temp_log.txt","w");

   $data_is_old = 0; 

   $left_pad = 150;

   $right_pad = 150;

   $top_pad = 20;
   
   $bottom_pad = 50;

   $plot_w = 1461; // rpi has 365 values. 365 x 4 = 1460

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
   
   $pale_blue = imagecolorallocate( $my_img->img, 200,200,250 );

   $black = imagecolorallocate( $my_img->img, 0,0,0 );
   
   $red = imagecolorallocate( $my_img->img, 255,0,0 );
   
   $blue = imagecolorallocate( $my_img->img, 0,0,255 );
   
   $green = imagecolorallocate( $my_img->img, 0,255,0 );
   
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


// -------------------- vertical hour lines ------------------------------------------

   $x33 = $plot_x1; // + ((60 - $mins3));    // initially this will be the first hour line, then it will be incremented for each hour line. this plot has one minute per pixel = 24 x 60 = 1440 pixels and 1440 data points

   $dx33 = 1460.0 / 12.0;     
               
   $month = array("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");  

   for ($x = 1; $x < 13; $x++){
      
//      imageline( $my_img->img, $x33, $tank_y1, $x33, $tank_y2, $pale_grey);            // tank plot
         
//      $s = sprintf("%2d",$x);
            
      imagettftext($my_img->img, 12, 0,  $x33 - 10, $tank_y2 + 16 ,  $black, $lib_font, $month[$x-1]);   // img, size, angle, x, y, color, font, text
      
      $x33 = $x33 + $dx33;
   }


   // rpi temperature plot ------------------------------------------------------------------
  
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

   $f2= fopen("rpi_temp_data.txt", "r");

   $f2_data = fgets($f2);

   $d2 = explode(" ",trim($f2_data));

   //$d2 = explode(" ",trim(fgets($f2)));

   $s2 = count($d2);

   fclose($f2);

   $flows = array();

   foreach ($d2 as $val){     // this gives us 1440 values that range from 0 to 255,  flow is 1 data per minute

      $flows[] = hexdec($val);
   }
 

   // this failed until i used new variable names (appended "aa"). why?

   $f2aa= fopen("rpi_temp_data_old.txt", "r");

   $f2_dataaa = fgets($f2aa);

   $d2aa = explode(" ",trim($f2_dataaa));

   //$s2 = count($d2);

   fclose($f2aa);

   $flows_old = array();

   $change_count = 0;

   foreach ($d2aa as $val){     // this gives us 1440 values that range from 0 to 255,  flow is 1 data per minute

      $flows_old[] = hexdec($val);

   }

   for ($i=0;$i<$s2;$i++){

      if ($flows[$i] != $flows_old[$i]){

         $change_count ++;

      }
   //   $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

   }




   $handle= fopen("rpi_temp_data_old.txt", "w");
   
   fwrite($handle,$f2_data);

   fclose($handle);


   $x1 = $plot_x1;

/*
   // the data is sent from esp as midnight..midnight, so we need to start at the current minute and go to the end, then resume at the beginning and go to the current minute

   if ($dst_flag == 1){ // this flag adds 60 to minutes2 in read_time_file so we have to undo that here since esp ignores dst. the legend will be correct because it uses minutes2 before this correction

      $current_data = $minutes2 - 60;
      
      if ($current_data < 0){ // this only happens the first hour after midnight, so I'll probably never test it. jay doesn't see this plot anyway.
      
         $current_data = $current_data + 1440;
      }
   }
*/

   imagesetthickness ( $my_img->img, 1 ); // affects line drawing
   $current_data = 0;

   $y_ratio = $tank_plot_h / 256.0;

   $y1 = $tank_y1 + $tank_plot_h - ($flows[$current_data] * $y_ratio);

   for ($i=($current_data);$i<$s2;$i++){

      $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

      $x2 = $x1 + 4.0; 
      
//      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

      if ($flows[$i] < 255){

         imagefilledrectangle ( $my_img->img, $x1, $tank_y2, $x2,$y2, $pale_blue );
         imagerectangle ( $my_img->img, $x1, $tank_y2, $x2,$y2, $black );
      }

      $x1 = $x2;
//      $y1 = $y2;
   }
   imagesetthickness ( $my_img->img, 2 ); // affects line drawing

/*
   for ($i=1;$i<$current_data;$i++){

      $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

      $x2 = $x1 + 1.0; 
      
      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

      $x1 = $x2;
      $y1 = $y2;
   }
   */

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
   $cutoff =  $tank_y1 + $tank_plot_h - (176 * $y_ratio);   // rpi maximum temperature is about 176

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 

   $cutoff =  $tank_y1 + $tank_plot_h - (150 * $y_ratio);   // rpi maximum observed value

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $blue ); 
   
   $cutoff =  $tank_y1 + $tank_plot_h - (102 * $y_ratio);   // rpi min observed value

   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $green ); 
      
   imagettftext($my_img->img, 12, 0,  200, 50 ,  $black, $lib_font, "blue line is 150 =  max observed value");   // img, size, angle, x, y, color, font, text
   
   imagettftext($my_img->img, 12, 0,  200, 70 ,  $black, $lib_font, "green line is 102 =  min observed value");   // img, size, angle, x, y, color, font, text
   
   $m88 = "change count = $change_count";

   imagettftext($my_img->img, 12, 0,  200, 90 ,  $black, $lib_font, $m88);   // img, size, angle, x, y, color, font, text
//   imagettftext($my_img->img, 12, 0,  $x33 - 10, $tank_y2 + 16 ,  $black, $lib_font, $month[$x-1]);   // img, size, angle, x, y, color, font, text

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
