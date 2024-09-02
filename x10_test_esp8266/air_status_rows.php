<?php


//include 'functions.php';

imagesetthickness ( $my_img->img, 1 );

$current_y = $top_pad; 
/*

imagettftext($my_img->img, 12, 0,  $left_pad / 2, $current_y+5+16 ,  $text_colour, $lib_font, "      Float");   // img, size, angle, x, y, color, font, text

$bitcol = imagecolorallocate( $my_img->img, 0x7d, 0xaa, 0x74 );

$fgname = 'float_state.txt';


$ret = do_esp_float_bits22(1);

$final_float_status = $ret; // used to make sure final segment in tank plot is red 

show_status($ret);


imagettftext($my_img->img, 12, 0,  $left_pad / 2, $current_y+5+16 ,  $text_colour, $lib_font, "       Flow");   // img, size, angle, x, y, color, font, text


$bitcol = imagecolorallocate( $my_img->img, 0xa4, 0x69, 0x69 );
$fgname = 'flow_state.txt';

$ret = do_esp_float_bits22(0);

show_status($ret);
*/
imagettftext($my_img->img, 12, 0,  $left_pad * 0.4, $current_y+5+16 ,  $text_colour, $lib_font, "Transfer 1");   // img, size, angle, x, y, color, font, text


$bitcol = imagecolorallocate( $my_img->img, 0xbc, 0xa1, 0x7d );
$fgname = 'trans1_state.txt';

$ret = do_esp_float_bits22(0);

show_status($ret);


imagettftext($my_img->img, 12, 0,  $left_pad * 0.4, $current_y+5+16 ,  $text_colour, $lib_font, "Transfer 2");   // img, size, angle, x, y, color, font, text

$bitcol = imagecolorallocate( $my_img->img, 0x7d, 0x8f, 0x8f );
$fgname = 'trans2_state.txt';

$ret = do_esp_float_bits22(0);

show_status($ret);
/*
   imagettftext($my_img->img, 12, 0,  $left_pad / 2, $current_y+5+16 ,  $text_colour, $lib_font, "   Master");   // img, size, angle, x, y, color, font, text

   $bitcol = imagecolorallocate( $my_img->img, 0x9f, 0xad, 0xbc );
   do_old_style_bits();
*/
?>
