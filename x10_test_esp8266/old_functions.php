<?php



function compare_date_codes($yest,$new_yest){  // test to see if new_yest is one day before yest, which happens when no pulses have been received after midnight. yest is yesterday, and new_yest should be the day before that

   // remember that the names are stupid. yesterday should be a day after new_yesterday if everything is normal and the data has not been shifted.

   $year = intval($yest / 1000);  // typical datecode is like 2021033 for the 33rd day in 2021

   $new_year = intval($new_yest / 1000);

   $day = $yest - $year * 1000;

   $new_day = $new_yest - $new_year * 1000;

//   echo "<br>year $year   new year $new_year   day $day   new day $new_day <br>";


   if ($year == $new_year){ 

      if ($day == ($new_day + 1)){

         return (1); // everything is normal      

      } else {

  //       echo "<br> fail 1 year same but day wrong<br>";
         
         return (0); // something is wrong, so discard the old data
      }

   } else { // if everything is normal, this happens only on jan 2 when yesterday is this year day 1, and new yesterday is last year day 365 or 366

      if ($day == 1){

         if (($new_year % 4) == 0){ // last year was a leap year

            if ($new_day == 366){

               return (1); // everything is normal      

            } else {

    //           echo "<br> fail 2  year differ, last year leap, new day not 366<br>";
               return (0); // something is wrong, so discard the old data
            }
         } else { // last year was not a leap year

            if ($new_day == 365){

               return (1); // everything is normal      

            } else {

      //         echo "<br> fail 3  year differ, last year not leap, new day not 365<br>";
               return (0); // something is wrong, so discard the old data
            }
         }

      } else { // today is not jan 2, so the years should match
               
        // echo "<br> fail 4 year differ, day not 1<br>";
         return (0); // something is wrong, so discard the old data
      }
   }
}


function draw_horizontal_grid(){  // $lines is the total including the top edge and bottom edge. $label_delt is the value on the next to the bottom line and the increment for all lines above it; for example, 200 will give you 200 400 600...

   global $lib_font, $dy, $lines, $my_img, $plot_x1, $xx, $plot_y2, $pc_y, $label_delt, $pale_grey, $plot_x2, $text_colour;

   for ($x = 0; $x < $lines; $x++){
   
      if ($x > 0){
         imageline( $my_img->img, $plot_x1 + $xx, $pc_y, $plot_x2, $pc_y, $pale_grey );
      }

      if ($x < ($lines - 1)){  // example lines = 7: the last label will be x < 6 == x = 5

         $s = sprintf("%d",(($lines - 1) * $label_delt) - ($x * $label_delt));   // example lines = 7: the next to the top line is the first one labeled: (7-1) x 10 - (0 x 10) = 60. the next to the bottom line will be labeled (7-1)x10 - (5x10) = 60 - 50 = 10
        
         $size = 12;

         imagettftext($my_img->img, $size, 0,  $plot_x2 + 10, $pc_y + $dy + $size/2 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text
         
         //imagettftext($my_img->img, 12, 0,  $plot_x2 + 10, $pc_y + $dy * 1.2 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text

      }

      $pc_y = $pc_y + $dy;

   }
}



function show_status($ret){
   
   global $data_is_old, $my_img, $plot_x2, $current_y, $on_off, $red, $text_colour, $lib_font;

   if ($data_is_old == 0){

      if ($ret > 0){

         imagettftext($my_img->img, 12, 0,  $plot_x2 + 10, $current_y+ $on_off ,  $text_colour, $lib_font, "On");   // img, size, angle, x, y, color, font, text

      } else {
         
         imagettftext($my_img->img, 12, 0,  $plot_x2 + 10, $current_y+$on_off ,  $text_colour, $lib_font, "Off");   // img, size, angle, x, y, color, font, text

      }

   } else {
         
         imagettftext($my_img->img, 12, 0,  $plot_x2 + 10, $current_y+ $on_off ,  $red, $lib_font, "??");   // img, size, angle, x, y, color, font, text

   }
}


function draw_block($x1,$y1,$x2,$y2,$color){

   global $current_y, $bits_h, $plot_x1, $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2;

// imagefilledrectangle ( resource $image , int $x1 , int $y1 , int $x2 , int $y2 , int $color ) : bool

// Creates a rectangle filled with color in the given image starting at point 1 and ending at point 2. 0, 0 is the top left corner of the image.


}



function do_esp_float_bits22(){

   global $current_y, $bits_h, $plot_x1, $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2,$x1_ra,$x2_ra,$box_cnt;

   $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data
   
   $y_upper = $current_y - $bits_draw_h;     // top of the blocks drawn here

   imageline( $my_img->img, $plot_x1 + $xx, $current_y, $plot_x2, $current_y, $pale_grey );

   $px1 = $plot_x1;

   // imagefilledrectangle ( resource $image , int $x1 , int $y1 , int $x2 , int $y2 , int $color ) : bool


//   for ($i=0;$i<2;$i++){
   for ($i=0;$i<$box_cnt;$i++){
      imagefilledrectangle ( $my_img->img, $x1_ra[$i] + $plot_x1, $y_upper, $plot_x1 + $x2_ra[$i],$current_y, $bitcol );

   }

//   imagefilledrectangle ( $my_img->img, $x1_ra[0] + $plot_x1, $y_upper, $plot_x1 + $x2_ra[0],$current_y, $bitcol );
//   imagefilledrectangle ( $my_img->img, $x1_ra[1] + $plot_x1, $y_upper, $plot_x1 + $x2_ra[1],$current_y, $bitcol );

   return ($val & $bit);

}



function do_esp_float_bits(){

   global $current_y, $bits_h, $plot_x1, $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2;

   $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data
   
   $y_upper = $current_y - $bits_draw_h;     // top of the blocks drawn here

   imageline( $my_img->img, $plot_x1 + $xx, $current_y, $plot_x2, $current_y, $pale_grey );

   $px1 = $plot_x1;

   // this is very much like the nightmare code for old pulse and float plots. that was such a mess I'm going to start over here without looking at it, at least for now
   //
   // i can provide an array of turnon times and turnoff times in minutes from "zero" which will be the left end of the plot. 
   //
   // this code needs a draw_block() function that gets simple xy corners and fills the blocks, which I think i did before, and it might pay to glance at that code now that I think about it, but hold off


// imagefilledrectangle ( resource $image , int $x1 , int $y1 , int $x2 , int $y2 , int $color ) : bool

   imagefilledrectangle ( $my_img->img, $plot_x1 + 50, $y_upper, $plot_x1 + 200,$current_y, $bitcol );
   


/*

   for ($x = 0; $x <= 863; $x++){   // first point in old code was 0. last point was 287. plot stretch was always 3 so the actual plot has 288 * 3 = 864 dots for 24 hours or 1440 minutes
                                    // so there's no need for my data here to be based on seconds. minutes is fine for this plot. the big pulse plots might benefit from seconds based data, maybe not. 
                                    // i think the whole seconds timing thing was for accurate GPM calculations and it just made it simpler (?) for both pulses and float to be seconds based. nobody
                                    // looks at those plots anyway, and if they do I doubt if they care much about the precision of the float vs pulse timing
      $val = $float_mins[$x];

      if (($val & $bit) > 0){

         for ($xx = 0; $xx < $plot_stretch; $xx++){

            imageline( $my_img->img, $px1 + $xx, $y_upper, $px1+$xx, $current_y, $bitcol );

         }

      }

      $px1 = $px1 + $plot_stretch;

   }

   */
/*
   for ($x = $first_point; $x <= $last_point; $x++){  

      $val = $statusra[$x];

      if (($val & $bit) > 0){

         for ($xx = 0; $xx < $plot_stretch; $xx++){

            imageline( $my_img->img, $px1 + $xx, $y_upper, $px1+$xx, $current_y, $bitcol );

         }

      }

      $px1 = $px1 + $plot_stretch;

   }
*/
   return ($val & $bit);

}

function do_bits($bit){


   global $current_y, $bits_h, $plot_x1, $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2;

   $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data
   
   $y_upper = $current_y - $bits_draw_h;     // top of the blocks drawn here

   imageline( $my_img->img, $plot_x1 + $xx, $current_y, $plot_x2, $current_y, $pale_grey );

   $px1 = $plot_x1;

   for ($x = $first_point; $x <= $last_point; $x++){

      $val = $statusra[$x];

      if (($val & $bit) > 0){

         for ($xx = 0; $xx < $plot_stretch; $xx++){

            imageline( $my_img->img, $px1 + $xx, $y_upper, $px1+$xx, $current_y, $bitcol );

         }

      }

      $px1 = $px1 + $plot_stretch;

   }

   return ($val & $bit);
}

final class bits {

   public $img;

   public function __construct($w,$h) {

      $this->img = imagecreate($w,$h);
//      $this->img = imagecreatetruecolor($w,$h);  // needed for antialias
   
   }

   public function __destruct() {
      
      if(is_resource($this->img)) {
         
         imagedestroy($this->img);
      }
   }

}


?>
