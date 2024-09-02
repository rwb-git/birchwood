<?php

include 'draw_top_plots_part_1.php';

$img_h = $sep + 5 * $bits_h + $top_pad + $bottom_pad + $tank_plot_h;                                // entire img height. not same for simple and huge web pages

include 'draw_top_plots_part_2.php';

imagesetthickness ( $my_img->img, 5 );

header( "Content-type: image/png" );

imagepng( $my_img->img );

imagedestroy( $my_img->img );


?> 
