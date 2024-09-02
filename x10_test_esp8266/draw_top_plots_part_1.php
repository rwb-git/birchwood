<?php
/*

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
*/

   
   include 'functions.php';   // this is only called here, but leave it because smaller files are easier to manage. or bring it here. // del
 
   $debug_text = 0; // set to zero to disable debug text

   $final_float_status = 0;

   $box_cnt = 0;

   $red_pix = array();

   for ($i7=0;$i7<865;$i7++){

      $red_pix[$i7] = 0;

   }

   $x1_ra = array();
   $x2_ra = array();

   $fgname = ' '; // status_rows and test_float use this but it seems to need to be here for them to see the same variable

   $data_is_old = 0; 

   include 'read_time_file.php'; 



//------------------------------------- main program --------------------------------------------

   $transfer_bit = 0x40;
   $float_bit = 0x08;

   $flow12_bit = 0x02;  // this is no longer used

   $flow34_bit = 0x04;
   $master_bit_tab = 0x01;    // these are new for android tablet and usually are not in the status byte in the old code
   $slave_bit_tab = 0x10;


   $left_pad = 150;      // space to left of plots, where labels will go; this also = x1 for plots

   $right_pad = 150;     // space to right of plots, for labels

   $top_pad = 20;
   $bottom_pad = 50;

   $bits_h = 36;        // height of flow, trans, etc.   was 30 for 6 plots, 36 for 5
   
   $bits_draw_h = ($bits_h * 3) / 4;   // height of the blocks I draw for flow, etc.

   $on_off = 25 - $bits_h;    // was 21 for 6 plots

   $plot_stretch = 3;   // this many pixels per data point NOTE THAT THIS is baked into a lot of things, so don't change just this or the other redundant places it is defined. 0.6 factor, 864 factor, etc. are based on this assumption.

   $plot_w = 288 * $plot_stretch;     // width of plots

   $plot_x1 = $left_pad;
   $plot_x2 = $plot_x1 + $plot_w;

   $tank_plot_h = 200;       // height of tank plot  THIS IS 200 TO SIMPLIFY MATH. IF THIS IS CHANGED, TANK PLOTTING HAS TO CHANGE TOO.

   $sep = 10;     // space between slave and tank

   $tank_y1 = $sep + $top_pad + 5 * $bits_h;          // y location of top of tank plot. was 6 for 6 plots
   $tank_y2 = $tank_y1 + $tank_plot_h;      // bottom of tank plot

   $img_w = $plot_w + $left_pad + $right_pad;     // entire img width
  
?>
