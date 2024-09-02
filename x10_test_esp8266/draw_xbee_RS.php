<?php


/*


$today = getdate();
print_r($today);
?>
The above example will output something similar to:

Array
(
    [seconds] => 40
    [minutes] => 58
    [hours]   => 21
    [mday]    => 17
    [wday]    => 2
    [mon]     => 6
    [year]    => 2003
    [yday]    => 167
    [weekday] => Tuesday
    [month]   => June
    [0]       => 1055901520



*/

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
   // i suppose all of them work for most of my simple stuff.



  // $today = getdate();

  // $month = $today[mon];

   $doy = date('z');

   $flog = fopen("xbee_RS_log.txt","w");

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
   
   $lib_font = 'LiberationSans-Bold.ttf';
   
   // ---- horizontal lines 

   $lines = 10; // use 10 for 0..100, 6 for 0..60, etc.
   
   $pc_y = $tank_y1;
   
   $dy = ($tank_y2 - $tank_y1) / $lines;
   
   // xbee decimal range is 6 to 54   hex is 0x06 to 0x36

   $label_delt = 60 / 10;  // put 10 here for 10 at the next to the bottom line and 20 30 40; put 200 here for 200 400 600

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

   $f2= fopen("xbee_RS_data.txt", "r");

   $d2 = explode(" ",trim(fgets($f2)));

   $s2 = count($d2);

   fclose($f2);

   $flows = array();

   foreach ($d2 as $val){     // this gives us 1440 values that range from 0 to 255,  flow is 1 data per minute

      $flows[] = hexdec($val);
   }

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

   $y_ratio = $tank_plot_h / 60.0;

   $y1 = $tank_y1 + $tank_plot_h - ($flows[$current_data] * $y_ratio);

   for ($i=($current_data);$i<$s2;$i++){

      $y2 = $tank_y1 + $tank_plot_h - ($flows[$i] * $y_ratio);

      $x2 = $x1 + 4.0; 
      
//      imageline( $my_img->img, $x1, $y1, $x2, $y2, $black );

      if ($flows[$i] < 255){


         if ($i == $doy){

            imagefilledrectangle ( $my_img->img, $x1, $tank_y2, $x2,$y2, $red );
         } else {

            imagefilledrectangle ( $my_img->img, $x1, $tank_y2, $x2,$y2, $pale_blue );
         }

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
   $cutoff =  $tank_y1 + $tank_plot_h - (6 * $y_ratio);
   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 

   $cutoff =  $tank_y1 + $tank_plot_h - (54 * $y_ratio);   // xbee decimal range is 6 to 54   hex is 0x06 to 0x36
   imageline( $my_img->img, $plot_x1, $cutoff, $plot_x2, $cutoff, $red ); 

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
