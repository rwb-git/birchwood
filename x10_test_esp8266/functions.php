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




function draw_horizontal_grid(){  // $lines is the total including the top edge and bottom edge. $label_delt is the value on the next to the bottom line and the increment for all lines above it; for example, 200 will give you 200 400 600...

   global $lib_font, $dy, $lines, $my_img, $plot_x1,  $plot_y2, $pc_y, $label_delt, $pale_grey, $plot_x2, $text_colour, $blue;

   for ($x = 0; $x < $lines; $x++){
   
      if ($x > 0){
         
         if (($x==6) || ($x==8)){
            imageline( $my_img->img, (int)$plot_x1 , (int)$pc_y, (int)$plot_x2, (int)$pc_y, $blue );
         } else {
         
            imageline( $my_img->img, (int)$plot_x1 , (int)$pc_y, (int)$plot_x2, (int)$pc_y, $pale_grey );
         }



//         imageline( $my_img->img, (int)$plot_x1 , (int)$pc_y, (int)$plot_x2, (int)$pc_y, $pale_grey );
      }

      if ($x < ($lines - 1)){  // example lines = 7: the last label will be x < 6 == x = 5

         $s = sprintf("%d",(($lines - 1) * $label_delt) - ($x * $label_delt));   // example lines = 7: the next to the top line is the first one labeled: (7-1) x 10 - (0 x 10) = 60. the next to the bottom line will be labeled (7-1)x10 - (5x10) = 60 - 50 = 10
        
         $size = 12;

         imagettftext($my_img->img, $size, 0,  $plot_x2 + 10, (int)($pc_y + $dy + $size/2) ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text
         
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



function do_esp_float_bits22($type22){ // $type22 = 1 to save red_pix, so use float for that

   global $current_y, $bits_h,  $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2,$fgname,$red_pix;


   $enable_echo = 0;

   $_fp = fopen($fgname, "r");

   $numbers = explode(" ", trim(fgets($_fp)));   // milen state milen state

      //1613263755 0 1613297779 1 1613301520 0 1613323903 1 1613329288 0 1613348817 1 

   fclose($_fp);

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

      } else {

         $index = 0;
         
         if ($enable_echo == 1) echo "$number  <br> <br>";

         if ($good_one == 1){

            if ($number != $current_state){ // ignore strings of 0 0 0 or 1 1 1

               $states[] = $number;
            
               $minutes[] = $plot_mins;

               $current_state = $number;

               $good_cnt++;

            }
         } else {

            $current_state = $number;
         }
      }
   }

   if ($good_cnt < 1) { // 12-25-2022
   
      if ($current_state > 0){

         $good_cnt = 1;

         $states[] = $current_state;

         $minutes[] = 1; // should this be zero?

//         $current_state = $previous_state;
      }
   }

   if ($good_cnt > 0){ // if there is no data for last 24 hours php will crash because arrays are empty

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

               $retval = 1;      // retval is used to print On or Off
            } else {

               $retval = 0;
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

            $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data
            
            $y_upper = $current_y - $bits_draw_h;     // top of the blocks drawn here

            imageline( $my_img->img, $plot_x1 , $current_y, $plot_x2, $current_y, $pale_grey );

            $px1 = $plot_x1;

            for ($i=0;$i<$box_cnt;$i++){
               imagefilledrectangle ( $my_img->img, $x1_ra[$i] + $plot_x1, $y_upper, $plot_x1 + $x2_ra[$i],$current_y, $bitcol );

               if ($type22 == 1){

                  for ($i7=$x1_ra[$i]; $i7<($x2_ra[$i]); $i7++){


                     $red_pix[$i7] = 1;
                  }
               }

            }

   } else { // no data for last 24 hours

      $retval = 0;
            
      $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data

   }




   return $retval; // ($val & $bit);

}

function do_old_style_bits(){ // modify the old code to do master


   global $current_y, $bits_h, $plot_x1, $first_point, $last_point, $statusra, $plot_stretch, $my_img, $bitcol, $bits_draw_h,
            $pale_grey, $plot_x1, $plot_x2, $red;

   $current_y = $current_y + $bits_h;    // this y value is at the bottom of the plot for this data
   
   $y_upper = $current_y - $bits_draw_h;     // top of the blocks drawn here

   imageline( $my_img->img, $plot_x1 , $current_y, $plot_x2, $current_y, $pale_grey );

   imagefilledrectangle ( $my_img->img, $plot_x1, $y_upper, $plot_x1 + 863,$current_y, $red );

   $px1 = $plot_x1;
/*

ping_master.txt looks like this, and has values roughly 5 minutes apart, but irregular because every event writes a timestamp to this file. 

1613595114 1 1613595216 1 1613595223 1 1613595231 1 1613595335 1 1613595354 1 1613595456 1 1613595537 1 

so for starters, draw a red box the entire plot width, then draw something for each of these entries

current ping millenium 
1 613 596 826

   current - 86400 = 1 613 510 426

start of ping_master.txt

1613 525 431 1 1613525455 1 1613525553 1 1613525736 1

so there is a gap at the beginning of about 4.17 hours which agrees with the plot based on visual inspection.

end of ping_master.txt


 1613596627 1 1613596680 1 1613596825 1 1613596826 1 1 613 596 859 1 

the end value shown is a minute or so past the current ping = ok, but the plot stops at 8 am, or about 8.2 hours short of the current time about 4:20


boxes go from to 135 to 558: add this to plot_x1 == 285 to 708. why is last one wrong?

plotx1 is 150, and plot is 864 wide

to test it, i removed all of these from the end, and it shows a red block about 25 minutes wide. 

      7805                                                                                                                                           9080  = 21 minutes which is ok
1613597805 1 1613597829 1 1613597927 1 1613597961 1 1613597988 1 1613598095 1 1613598116 1 1613598137 1 1613598138 1 1613598438 1 1613598759 1 1613599080 1 

also deleted these:
 1613597622 1 1613597683 1 1613597697 1 1613597794 1 1613599401 1 1613599723 1 


so with it ending like this, the red starts about 45 minutes from the end: 1613599723 = curent ping, so 9723 - 7566 / 60 = 36 minutes, ok. 
 1613597433 1 1613597472 1 1613597490 1 1613597566 1

ok, maybe fixed. should be red block from about 4:35 pm to about 5:20 pm



*/






   $enable_echo = 0;
         
   if ($enable_echo == 1) echo "plotx1 $plot_x1<br><br>";

   $_fp = fopen("ping_master.txt", "r");

   $numbers = explode(" ", trim(fgets($_fp)));   // milen state milen state

      //1613263755 0 1613297779 1 1613301520 0 1613323903 1 1613329288 0 1613348817 1 
      // except master looks like this. since every value is 1 i really should just get rid of them: 1613595114 1 1613595216 1 1613595223 1 1613595231 1 1613595335 1 1613595354 1 1613595456 1 1613595537 1 

   fclose($_fp);

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

      } else {

         $index = 0;
         
         if ($enable_echo == 1) echo "$number  <br> <br>";

         if ($good_one == 1){

            $states[] = $number;
         
            $minutes[] = $plot_mins;

            $current_state = $number;

            $good_cnt++;
         }
      }
   }

   $state = $states[0];

   if ($enable_echo == 1) echo "summary<br><br>";

   for ($ii=0;$ii<$good_cnt;$ii++){

      if ($enable_echo == 1) echo "good state $states[$ii] minutes $minutes[$ii]<br><br>";
   }

   $start_index = 0;

   $factor = 864.0 / 1440.0; // convert minutes in 24 hours to pixels

   $loop_cnt = $good_cnt;

   $box_cnt = 0;

   $done = 0;

   $this_state = 0;

   for ($boxes=0;$boxes<($loop_cnt);$boxes++){

      $box_cnt++;

      $x1_ra[] = intval((float)$minutes[$start_index] * $factor);

      $end_minutes = $minutes[$start_index] + 5;

      if ($end_minutes > 1439){

         $end_minutes = 1439;
      }

      $x2_ra[] = intval((float)($end_minutes) * $factor); // this determines how many minutes each ping accounts for, so once it's all debugged, set this to at least 6 minutes?. using 4 it shows a narrow red line about every 1.3 hours or whatever

      $start_index++;
   }

   for ($boxes=0;$boxes<$box_cnt;$boxes++){

      if ($enable_echo == 1) echo"x1_ra $x1_ra[$boxes]    x2_ra $x2_ra[$boxes]<br><br>";
   }

   $px1 = $plot_x1;

   for ($i=0;$i<$box_cnt;$i++){
      imagefilledrectangle ( $my_img->img, $x1_ra[$i] + $plot_x1, $y_upper, $plot_x1 + $x2_ra[$i],$current_y, $bitcol );
   }





















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
//   return ($val & $bit);
}

?>
