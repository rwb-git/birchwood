<?php








   include 'air_generic_1.php';   // this is only called here, but leave it because smaller files are easier to manage. or bring it here.  // del

// move everything up here for old tank and status

// ---- horizontal lines for tank level plot -------------------

   $lines = 4; // use 10 for 0..100, 6 for 0..60, etc.
   $pc_y = $tank_y1;
   $dy = ($tank_y2 - $tank_y1) / $lines;

   $label_delt = 20;  // put 10 here for 10 at the next to the bottom line and 20 30 40; put 200 here for 200 400 600

   draw_horizontal_grid();

// this is done in part_1   include 'read_time_file.php';




// -------------------- vertical hour lines ------------------------------------------

   $hour3 = (int) ($minutes2 / 60);    // current hour; if minutes > 0, the first hour line on the left will be this hour + 1

   $mins3 = $minutes2 - ($hour3 * 60);    // current minutes. the first hour line will be 60  - this minutes from the left border


   $h23 = $hour3 + 1;


   while ($h23 > 12) {

      $h23 = $h23 - 12;

   }

   $x33 = $plot_x1 + ((60 - $mins3) * $plot_stretch / 5);    // 5 minutes per packet

   $dx33 = $plot_stretch * 60 / 5;     // this is one hour of pixels

   for ($x = 0; $x < 24; $x++){

      imageline( $my_img->img, (int)$x33, $top_pad, (int)$x33, $tank_y1 - $sep, $pale_grey);     // upper plot
//      
      imageline( $my_img->img, (int)$x33, $tank_y1, (int)$x33, $tank_y2, $pale_grey);            // tank plot
//      imageline( $my_img->img, $x33, $tank_y3, $x33, $tank_y4, $pale_grey);            // tank plot
//      imageline( $my_img->img, $x33, $tank_y5, $x33, $tank_y6, $pale_grey);            // tank plot
         
      $s = sprintf("%2d",$h23);

      $h23 = $h23 + 1;

      while ($h23 > 12) {

         $h23 = $h23 - 12;

      }
            
      imagettftext($my_img->img, 12, 0,  (int)$x33 - 10, $tank_y2 + 16 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text
      
//      imagettftext($my_img->img, 12, 0,  $x33 - 10, $tank_y4 + 16 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, text
      
//      imagettftext($my_img->img, 12, 0,  $x33 - 10, $tank_y6 + 16 ,  $text_colour, $lib_font, $s);   // adding this line of text added 353 bytes
      
      $x33 = $x33 + $dx33;

   }
   

// --------------------- line at very top

   imageline( $my_img->img, $plot_x1, $top_pad, $plot_x2, $top_pad, $pale_grey);


   include 'read_offsets.php';



   include 'air_status_rows.php';  // this is only called here, but leave it because smaller files are easier to manage. or bring it here. 

   include 'air_tank_status.php'; // this is only called here, but leave it because smaller files are easier to manage. or bring it here. 

// THIS ASSUMES TANK PLOT IS 200 PIXELS TALL. IF THAT IS CHANGED, THIS HAS TO ADJUST TO FIT.

// ---------------  box around tank plot ------------------------------------------

imageline( $my_img->img, $plot_x1, $tank_y1, $plot_x2, $tank_y1, $black );                // horizontal lines above and below tank plot
imageline( $my_img->img, $plot_x1, $tank_y2, $plot_x2, $tank_y2, $black );

imageline( $my_img->img, $plot_x1, $tank_y1, $plot_x1, $tank_y2, $black );                // vertical lines at ends of tank plot
imageline( $my_img->img, $plot_x2, $tank_y1, $plot_x2, $tank_y2, $black );

imageline( $my_img->img, $plot_x1, $top_pad, $plot_x1, $tank_y1 - $sep, $pale_grey );     // vertical lines at ends of upper plots
imageline( $my_img->img, $plot_x2, $top_pad, $plot_x2, $tank_y1 - $sep, $pale_grey );


imagettftext($my_img->img, 22, 0,  (int)($left_pad * 1/4), (int)(($tank_y1 + $tank_y2) / 2) ,  $text_colour, $lib_font, "  PSI");   // img, size, angle, x, y, color, font, text



$s = sprintf("%2d %2d %2d",$offset, $offset_H, $offset_L);

imagettftext($my_img->img, 22, 0,  (int)($left_pad * 1/4), (int)(($tank_y1 + $tank_y2) / 2) + 40 ,  $text_colour, $lib_font, $s);   // img, size, angle, x, y, color, font, tex  

// watermark for x10 images
   
imagettftext($my_img->img, 6, 0, (int)( $left_pad * 1/4),  (int)($tank_y2)  ,  $text_colour, $lib_font, "x10");   // img, size, angle, x, y, color, font, text








?>
