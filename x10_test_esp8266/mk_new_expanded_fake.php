<?php

/*

to do:

   make pages so i can see 1 2 4 8 hour plot

   verify no_pulses works

   verify force_float works

   force_float = 1 if there were pulses. if there are no float transitions in the plot then the float was on the whole time

   what is no_float. it isn't really clear. it sorta looks like it's a redundant version of force_float. make new rules?

      if there are no pulses don't plot any float no matter what. if the float was on and no pulses for an hour there are other issues and indicators to look at

      if there are no transitions float at all but at least two pulses that draw at least one pulse, turn float on the whole time. not necessarily valid but again if there are only
      a small number of pulses in an hour and no float transitions then something is screwed up. 

      so those are the rules:
 
         if no pulses then abort without drawing any float no matter what. = no_pulses variable

         if no float transitions and 2+ pulses then draw float all the way = force_float_on variable


   2-19-2021

   pulse and float file (and all other events) will be in a 24 hour range somewhere in this 48 hour range: esp8266 code prunes files on every event, so that means if there
   have not been events lately there might be some older than 24 hours, but all the events in a file should be within a 24 hour range that ends with the last event in that file.


   <------------------------------ yesterday ----------------------------->|<------------------------------- today ------------------------------->

   12 1  2  3  4  5  6  7  8  9  10 11 12 1  2  3  4  5  6  7  8  9  10 11 12 1  2  3  4  5  6  7  8  9  10 11 12 1  2  3  4  5  6  7  8  9  10 11 12

   typical normal plot which is always 24 hours, and almost always will span midight:

                  <----------------------------------------------------------------------> = 86400 seconds
                  SS

   typical expanded plots which may or may not span midnight: 8 hour versions:

                                                               <----------------------> = 28800 seconds
                                                               SS
                                                                                                            <---------------------->
                                                                                                            SS



   typical expanded plots which may or may not span midnight: 1 hour versions:

                                                                        <--> barely spans midnight, but it can if viewed just after midnight
                                                                                                            <-> = 3600 seconds
                                                                                                            SS
                                                                                                            

   pulse seconds after they are loaded and adjusted. this is true for the old style and the new style, at least when the range spans midnight:

   0..................................................................86400......................................................................172800

   for every plot i need to calculate s tart_seconds = SS, and then the pulse location on the plot will be relative to that. let PP = pulse seconds since yesterday midnight. the pulse
   location will be PP - SS if SS is relatve to yesterday midnight, no matter what plot is used and whether or not it spans midnight. the old complexity was either me being stupid or
   was because of the file format. the esp file format will never need that bullshit.

   so make a new s tart_seconds just used to place pulses on the x axis = SS_esp = current_millenium - plot size in hours * 3600 - yesterday midnight millenium. that will be like SS in
   each of the above examples = seconds since yesterday midnight for the beginning of the current plot. when pulse and float data is loaded, i need to convert them to this domain like this:

      1. pulse or float in file is pure millenium

      2. subtract yesterday_midnight_millenium from those values to get them same as SS

      3. then, if they are < SS they are discarded. 

      4. they can never be > (SS + plot seconds) since SS is based on current millenium at time this code runs. worst case I suppose might be 1 second error which can be ignored on plots of 1 2 4 8 hours

      5. to plot them, subtract SS from the new value calculated in step 2

   since i get up so late, test this by temporary files that show something like 16 hours or whatever it takes to include pulses before midnight

*/


 
   $plot_hours=$_GET["p"];
/*
if ($plot_hours < 1){
$eraseme = 33;
}else {
$eraseme = $plot_hours;
}
*/

   $new_float_state = 0; // 8-13-2022 this crashed biz but not fork. different php? 

   if ($plot_hours < 1){ // this happens if the script is run directly or the web page does not provide a "p" argument. I do this so I can run this script directly to see error messages if php has errors

      $plot_hours = 2;

      $debug_text = 1;

   } else { // p argument was provided

      if ($plot_hours > 100){ // p > 100 means turn on debug and use p - 100 for plot hours

         $plot_hours -= 100;

         $debug_text = 1; // 1 = lots of text on screen

      } else {

         $debug_text = 0; 
      }


      if ($plot_hours < 1){

         $plot_hours = 2;
      }
     


   }



   include 'functions.php';


   function float_block($x1,$x2){

      global $plot_w, $my_img , $plot_x1 ,  $y0f ,  $plot_x2 , $green, $debug_text, $lib_font, $text_colour, $yloc;

      if ($x1 < 0){

         $x1 = 1;

      } else if ($x1 > $plot_w){

         $x1 = $plot_w - 2;

      }
 
      if ($x2 < 0){

         $x2 = 1;

      } else if ($x2 > $plot_w){

         $x2 = $plot_w - 2;

      }
    
      if ($debug_text == 1){

         $x1int = intval($x1);
         $x2int = intval($x2);

         $m8 = "float_block from pixels $x1int to $x2int (order is not important)";
      
         imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
         $yloc++;

      }

         //imagefilledrectangle ( $my_img->img , $x1 +$plot_x1 ,  $y0f ,  $plot_x1 + $x2 , $y0f - 40 ,$green);
         
         
         imagefilledrectangle ( $my_img->img , (int)$x2 +$plot_x1 ,  (int)$y0f ,  $plot_x1 + (int)$x1 , (int)$y0f - 40 ,$green);   // test whether x1 and x2 have to be "in order", by reversing them for a test. yes, order does not matter.

   }



//------------------------------------- main program --------------------------------------------
   
   
            

   
   
   $transfer_bit = 0x40;
   $float_bit = 0x08;

   $flow12_bit = 0x02;  // this is no longer used

   $flow34_bit = 0x04;
   $master_bit_tab = 0x01;    // these are new for android tablet and usually are not in the status byte in the old code
   $slave_bit_tab = 0x10;


   $left_pad = 50;      // space to left of plots, where labels will go; this also = x1 for plots

   $right_pad = 50;     // space to right of plots, for labels

   $top_pad = 20;
   $bottom_pad = 50;

   $bits_h = 36;        // height of flow, trans, etc.   was 30 for 6 plots, 36 for 5

   $on_off = 25 - $bits_h;    // was 21 for 6 plots

   $plot_stretch = 3;   // this many pixels per data point NOTE THAT THIS is baked into a lot of things, so don't change just this or the other redundant places it is defined. 0.6 factor, 864 factor, etc. are based on this assumption.

   $plot_w = 288 * $plot_stretch;     // width of plots

   $plot_x1 = $left_pad;
   $plot_x2 = $plot_x1 + $plot_w;

   $tank_plot_h = 400;       // height of tank plot  THIS IS 200 TO SIMPLIFY MATH. IF THIS IS CHANGED, TANK PLOTTING HAS TO CHANGE TOO.

   $sep = 10;     // space between slave and tank



$tank_y1 = 0;

   $img_h = $bottom_pad + $tank_plot_h;

   $tank_y5 = $top_pad ;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y2 = $tank_y1 + $tank_plot_h;      // bottom of tank plot

   $img_w = $plot_w + $left_pad + $right_pad;     // entire img width

   if ($debug_text == 1){

      $img_h += 2700;                                 // more room for debug text
   
      $yloc = intval((($tank_plot_h + $sep) / 20) +5); // yloc is used to place debug_text at y location 
   }

   $tank_y6= $tank_y5 + $tank_plot_h;      // bottom of tank plot


   $force_float_on = 0; // if there are no float transitions but 2+ pulses draw float the whole way

   $no_pulses = 0; // if there are no pulses to draw then don't draw float no matter what.

   include 'generic_1.php';

   imagesetthickness ( $my_img->img, 2 );


   $lines = 6; // use 10 for 0..100, 6 for 0..60, etc.
   $pc_y = $tank_y5;
   $dy = ($tank_y6 - $tank_y5) / $lines;

   $label_delt = 10;

   $full_scale = $lines * $label_delt;

   draw_horizontal_grid(); 



   include 'read_time_file.php';  //    $tank2         $status2           $minutes2         $date2  $yesterday_code

     //so make a new st art_seconds just used to place pulses on the x axis = SS_esp = current_millenium - plot size in hours * 3600 - yesterday midnight millenium
   $SS_esp =  $current_millenium - ($plot_hours * 3600);
//   $SS_esp =  $current_millenium - ($plot_hours * 3600) - $yesterday_midnight_millenium;



   $pixels_per_minute = (288 * $plot_stretch) / ($plot_hours * 60);

   $pixels_per_sec = $pixels_per_minute / 60;


   if ($debug_text == 1){
    
//      $hrss = intval($s tart_secs / 3600);

  //    $minsss = intval(($s tart_secs - ($hrss * 3600)) / 60);

      $hrm2 = intval($minutes2 / 60);

      $minm2 = $minutes2 - ($hrm2 * 60);

      $m = " plot_hours " . $plot_hours  .  "       minutes2 " . $minutes2 . "  ".$hrm2.":".$minm2 . " pixels_per_minute $pixels_per_minute";

      imagettftext($my_img->img, 12, 0,  22, $yloc * 20 ,  $text_colour, $lib_font, $m);   // img, size, angle, x, y, color, font, text
      $yloc++;
  
   }




// ---------------------------- pulse section ---------------------------------------------------------------------------
// ---------------------------- pulse section ---------------------------------------------------------------------------
// ---------------------------- pulse section ---------------------------------------------------------------------------


   include 'load_pulses.php';

   include "expanded_common.php";

   $new_slot = 0;
   $first_point = 0;

$tank_y3 = 0;

   $d1 = $tank_y3 + $tank_plot_h; // - (2 * $tankra[$first_point]);

   $px1 = $plot_x1;

   $gallons = 0;
      
   $y0 = $tank_y5 + $tank_plot_h; // y pixel at bottom of this plot

   $y0f = $y0 - 2;


   // THIS ASSUMES TANK PLOT IS 200 PIXELS TALL. IF THAT IS CHANGED, THIS HAS TO ADJUST TO FIT.

   // ---------------  box around tank plot ------------------------------------------

   imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x2, $tank_y5, $black );                // horizontal lines above and below tank plot
   imageline( $my_img->img, $plot_x1, $tank_y6, $plot_x2, $tank_y6, $black );

   imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x1, $tank_y6, $black );                // vertical lines at ends of tank plot
   imageline( $my_img->img, $plot_x2, $tank_y5, $plot_x2, $tank_y6, $black );







// ---------------------------- end of pulse section ---------------------------------------------------------------------------
// ---------------------------- end of pulse section ---------------------------------------------------------------------------
// ---------------------------- end of pulse section ---------------------------------------------------------------------------




//----------------------------- float section -----------------------------------------------------------------------
//----------------------------- float section -----------------------------------------------------------------------
//----------------------------- float section -----------------------------------------------------------------------


   $final_old_yesterday = 0; // newest data already in php list

   $final_old_today = 0;
   
   $fgname = "float_state.txt";

   $_fp = fopen($fgname, "r");
   
   $numbers = explode(" ", trim(fgets($_fp)));

   fclose($_fp);

   $rod1 = 0;
   $rod2 = 0;
   $rod3 = 0;
   
   $array_fsecs = array();

   $array_fstate = array();

   $index = 0;
   $good_data = 0;

   $final_float = 0;

   foreach ($numbers as &$number){

      $rod1++;

      if ($index == 0){

         $temp73 = $number; // - $yesterday_midnight_millenium; // so temp73 is seconds from yesterday midnight to this pulse. SS_esp is seconds from yesterday midnight to start of this plot
         //$temp73 = $number - $yesterday_midnight_millenium; // so temp73 is seconds from yesterday midnight to this pulse. SS_esp is seconds from yesterday midnight to start of this plot

         if ($temp73 >= $SS_esp){ // if this is not negative, this float transition is in this plot
   
            // so make a new s tart_seconds just used to place pulses on the x axis = SS_esp = current_millenium - plot size in hours * 3600 - yesterday midnight millenium, so it's seconds since midnight yesterday to the start of this plot

            $rod2++;

            $array_fsecs[] = $temp73 - $SS_esp; // we are storing the number of seconds from start of this plot to this float event

            $good_data = 1;

  



         }

         $index = 1;

      } else {
         $index = 0;
      
         if ($good_data == 1){

            $rod3++;

            $good_data = 0;
         
            $array_fstate[] = $number;


            $final_float = $number;
          
            if ($debug_text == 1){

               $ind32 = $rod2 - 1;

               $secs88 = $array_fsecs[$ind32];

               $hrs88 = intval($secs88 / 3600);

               $mins88 = intval(($secs88 - ($hrs88 * 3600)) / 60);
               
               $st88 = $array_fstate[$ind32];

               $m8 = "float event elapsed time since plot start $hrs88:$mins88  state $st88";
            
               imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
               $yloc++;

            }

         }

      }
   }


   $farray_cnt = count($array_fsecs); //$today_size;

   

// ---------------- plot float ------------------------


   // if float state is 1 then plot something continuous until float state is 0. pale green or whatever. huge filled rectangles make sense.

/* this code is from load_pulses.php:

      if ($array_secs_cnt > 2){

         $force_float = 1; // if there are no float transitions, this will draw float the whole way

      } else {

         $no_pulses = 1; // no pulses will be drawn so don't draw float no matter what
      }
*/


   $pixels_per_sec = $pixels_per_minute / 60;
      
   $y0 = $tank_y5 + $tank_plot_h; // y pixel at bottom of this plot

   $y0f = $y0 - 2;
   
   
   if ($no_pulses == 0){
    
      if ($debug_text == 1){

         $m8 = "pulses were present in this plot period";
      
         imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
         $yloc++;

      }

      if ($farray_cnt > 0){

         $new_float_state = $array_fstate[0]; 
      
         $oldx = $array_fsecs[0] * $pixels_per_sec;

         if ($new_float_state == 0){ // float was on but turned off so draw block back to beginning
       
            if ($debug_text == 1){

               $m8 = "first float event is turn off, so draw float back to left margin";
            
               imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
               $yloc++;

            }


            float_block($oldx, 1);

         }

      } else { // no float info so see if any pulses

         if ($force_float == 1){ // we had pulses but no float transitions so it was on all the time
 
            if ($debug_text == 1){

               $m8 = "no float events were found, but there were pulses, so draw float entire plot assuming it had to be on";
            
               imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
               $yloc++;

            }

            float_block($plot_w - 1, 1);

         }
      }

      $yfact = $tank_plot_h / ($full_scale * 10); // divide by 10 because gpm is x 10 meaning 40.7 gpm will be 407
$x1=0;
      for ($i=1; $i< $farray_cnt; $i++){

         $x = $array_fsecs[$i] * $pixels_per_sec;

         if ($x  > 0){

               $new_float_state = $array_fstate[$i]; 

               if ($new_float_state == 0){ // float turned off after being on so draw a block

                  if (($x1 + $plot_x1) > ($plot_x2 - 1)){ // don't draw green block beyond right end of plot
                
                     if ($debug_text == 1){

                        $m8 = "draw float block from turn on to turn off, but it seems to go past right end, so stop it there";
                     
                        imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
                        $yloc++;

                     }

                     float_block($old_x, $plot_w -1);

                     
                  } else { // normal block 
          
                     if ($debug_text == 1){

                        $m8 = "draw a normal block that turns on then turns off";
                     
                        imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
                        $yloc++;

                     }

                     float_block($oldx, $x);

                  }
               }

               $oldx = $x;
         }
      }
      
      if ($new_float_state > 0){ // float was on at the end

         if ($debug_text == 1){

            $m8 = "float was on at end of plot period, so draw block from last turn on event to right end";
         
            imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
            $yloc++;

         }


         float_block($oldx, $plot_w - 1);

      }


      if (($farray_cnt == 0) && ($final_float > 0)){ 


         if ($debug_text == 1){

            $m8 = "no float events, but final_float is > 0, so draw block entire plot";
         
            imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
            $yloc++;

         }


         float_block(2, $plot_w - 1);

      }

/* this is covered above and does not appear to be needed

      if (($farray_cnt == 1) && ($final_float == 0)){ 

         if ($debug_text == 1){

            $m8 = "there was only 1 float event, and final_float was off, so draw float block from start to that float turn off event";
         
            imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
            $yloc++;

         }


      
         float_block($oldx, 1);

      }

   */

   } else { // no pulses so don't plot any float stuff

 
      if ($debug_text == 1){

         $m8 = "no pulses were present in this plot period, so no need to plot any float events which should not be here anyway";
      
         imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
         $yloc++;

      }



   }





//----------------------------- end of float section -----------------------------------------------------------------------
//----------------------------- end of float section -----------------------------------------------------------------------
//----------------------------- end of float section -----------------------------------------------------------------------








   // --------------------- this plots 60 minutes of pulses starting at 0. to plot the last 60 minutes, subtract an offset = seconds of the first pulst to be plotted.
   //
       

   // so how does the new file format tell me when flow starts and stops? use the float signal from status. 
   //
   //    if float is off between two pulses
   //
   //       the first one ends a sequence so go to zero <- meaning the last pulse before the float stopped
   //
   //       the first pulse after the float goes on starts a sequence, and does not have a gpm value; the height comes from the second pulse. the file will have a zero value or might be 1 to indicate the state of this pulse
   


   imagettftext($my_img->img, 12, 0,  0, ($tank_y5 + $tank_y6) / 2 ,  $text_colour, $lib_font, "  GPM");   // img, size, angle, x, y, color, font, text
   
//   $m8 = "$eraseme";
  // imagettftext($my_img->img, 12, 0,  0, ($tank_y5 + $tank_y6) / 2 ,  $text_colour, $lib_font,$m8);   // img, size, angle, x, y, color, font, text
  

  /*
   if ($debug_text == 1){

      $m8 = "mn: yloc $yloc  ";

   
      imagettftext($my_img->img, 12, 0,  22,  $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
      $yloc++;
      //imagettftext($my_img->img, 12, 0,  0,  $tank_y6 +60 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text

   }
*/

   $min_gpm = 30; // ignore below 3.0

   $y = $array_gpm[0]; // gpm x 10; 400 = 40

   $yfact = $tank_plot_h / ($full_scale * 10); // divide by 10 because gpm is x 10 meaning 40.7 gpm will be 407

   $oldy2 = $y0 - ($y * $yfact); // y pixel = bottom + value * plot_h / full scale value
   
   $oldx = ($array_secs[0] - $SS_esp) * $pixels_per_sec; //* 864/3600; /// 20; // seconds start at zero      3600 seconds/ 864 pixels

   if ($oldx < 0) {

      $oldx = 0;

   }

   // draw the first pulse 
            
   $y2 = $y0 - ($y * $yfact); // 
      
   if ($oldx > 0){ // don't draw on left border


      if ($plot_hours < 9){ // debug with plot hours large has too many blue pulse line and i can't see float block

         imageline( $my_img->img, $oldx+$plot_x1 , $y0 - 3, $oldx+$plot_x1  , $y2 + 1, $blue );    // show the pulse. if this is the first pulse in a sequence it will be tiny and will be hidden by the next red vertical line
      }
   }

   if ($y > $min_gpm){
      imageline( $my_img->img, $oldx + $plot_x1 , $y2,$plot_x1  , $y2, $red );  // horizontal at new height. if the first pulse does not overlap the left border, this should draw nothing since y should be small
   }

/*

   if ($debug_text == 1){

      $m8 = "mn:full scale $full_scale  yfact $yfact  new float state $new_float_state  s tart secs $s tart_secs end secs ".  ($s tart_secs + ($plot_hours * 3600)) . " pixels_per_sec $pixels_per_sec";

      imagettftext($my_img->img, 12, 0,  $left_pad / 2, $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
      $yloc++;

   }
*/
   for ($i=1; $i< $array_secs_cnt; $i++){


      // this seems to handle the left end of the plot ok. I've included 1000 seconds of old pulses up there, which will not be plotted, but somehow they make the plot go all the way to the left edge if the sequence overlaps that end.
      //
      // why is this needed? why doesn't setting oldx = 0 accomplish the same thing? in the following code, if x < 0 nothing is done. but wait. if I don't include any extra off-screen pulses, the first oldx up there before this loop
      // is inside the plot. but why doesn't it even plot the first blue pulse line if no pulses exist outside the plot?
      //
      // consider that. first pulse is > start secs. it's the one up there at [0] in the arrays. the first one processed here is at [1], so it's actually the second pulse inside the plot. so if i get rid of all the off-screen pulses i
      // need to draw the stuff for pulse [0] before this loop. but the advantage of the - wait. save this as mk_expanded_9 since this works.

      $x = (($array_secs[$i] ) - $SS_esp) * $pixels_per_sec;

            if ($debug_text == 1){

               if ($i > ($array_secs_cnt - 40)){

//               $m8 = "plotting now: array_secs $array_secs[$i] minus 86400 " . ($array_secs[$i] - 86400) . " start secs $s tart_secs  end secs " . ($s tart_secs + ($plot_hours * 3600)) . "  x $x   ";

  //             imagettftext($my_img->img, 12, 0,  $left_pad / 2, $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
    //           $yloc++;
               }

            }

      if (($x ) > 0){

            // plot width for old style was 288 x 3 pixels = 864
            //
            // if 864 = one hour then it is 3600 seconds

            $y = $array_gpm[$i]; // gpm x 10; 400 = 40

 
          $y2 = $y0 - ($y * $yfact); // 

/*
            if ($debug_text == 1){


               $m8 = "y = gpm = $y   y2 $y2  oldy2 $oldy2  mingpm $min_gpm   y0  $y0  yfact $yfact ";

               imagettftext($my_img->img, 12, 0,  $left_pad / 2, $yloc*20 ,  $text_colour, $lib_font, $m8);   // img, size, angle, x, y, color, font, text
               $yloc++;

            }
*/

            if ($y2 == $oldy2){

               imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$y2, (int)$x+$plot_x1  , (int)$y2, $red );    // horizontal line since value did not change 

            } else {

               if ($y > $min_gpm){ 

                  imageline( $my_img->img, (int)$oldx + $plot_x1 , (int)$y2, (int)$x+$plot_x1  , (int)$y2, $red );  // horizontal at new height
               }
               
               if ($oldx > 0){ // don't draw on top of the left border

                  imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$oldy2, (int)$oldx+$plot_x1  , (int)$y2, $red );    // vertical at old pulse from old height to new height; this overwrites most of the final blue pulse in a sequence
               }
            }
               
            if ($plot_hours < 9){ // debug with plot hours large has too many blue pulse line and i can't see float block
               imageline( $my_img->img, (int)$x+$plot_x1 , (int)$y0 - 2, (int)$x+$plot_x1  , (int)$y2 + 1, $blue );    // show the pulse; the first pulse usually has a tiny value so not much is drawn here and is obscurred by the next vertical red
            }

            $oldx = $x;

            $oldy2 = $y2;
      }
   }


   imageline( $my_img->img, (int)$oldx+$plot_x1 , (int)$oldy2, (int)$oldx+$plot_x1  , (int)$y0 - 2, $red );    // vertical at old pulse from old height to new height; this overwrites the entire final blue pulse


   // THIS ASSUMES TANK PLOT IS 200 PIXELS TALL. IF THAT IS CHANGED, THIS HAS TO ADJUST TO FIT.

   // ---------------  box around tank plot ------------------------------------------

   imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x2, $tank_y5, $black );                // horizontal lines above and below tank plot
   imageline( $my_img->img, $plot_x1, $tank_y6, $plot_x2, $tank_y6, $black );

   imageline( $my_img->img, $plot_x1, $tank_y5, $plot_x1, $tank_y6, $black );                // vertical lines at ends of tank plot
   imageline( $my_img->img, $plot_x2, $tank_y5, $plot_x2, $tank_y6, $black );





   //-------------------------------------------------------------------------------------------------------------------------

   imagesetthickness ( $my_img->img, 5 );

   header( "Content-type: image/png" );

   imagepng( $my_img->img );

   imagedestroy( $my_img->img );


?> 
