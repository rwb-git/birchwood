<?php


   $my_img = new bits( $img_w, $img_h );


   $background = imagecolorallocate( $my_img->img, 255, 255, 255 );

   $pale_grey = imagecolorallocate( $my_img->img, 200,200,200 );
   $grey2 = imagecolorallocate( $my_img->img, 165,155,155 );
   $grey3 = imagecolorallocate( $my_img->img, 144,144,164 );

   $text_colour = imagecolorallocate( $my_img->img, 0, 0, 0 );

   $line_colour = imagecolorallocate( $my_img->img, 128, 255, 0 );
   
   $blue = imagecolorallocate( $my_img->img, 155, 155, 255 );

   $black = imagecolorallocate( $my_img->img, 0,0,0 );
   
   $green = imagecolorallocate( $my_img->img, 100,255,100 );


   $red = imagecolorallocate( $my_img->img, 255,0,0 );
   
   $pale_blue = imagecolorallocate( $my_img->img, 220, 220, 255 );


   $colors = array(  imagecolorallocate( $my_img->img, 70, 70, 145 ),
                     imagecolorallocate( $my_img->img, 90, 90, 150 ),
                     imagecolorallocate( $my_img->img, 110, 110, 155 ),
                     imagecolorallocate( $my_img->img, 130, 130, 160 ),
                     imagecolorallocate( $my_img->img, 150, 150, 165 ),
                     imagecolorallocate( $my_img->img, 170, 170, 170 ),
                     imagecolorallocate( $my_img->img, 190, 190, 175 ));


   $lib_font = 'LiberationSans-Bold.ttf';
   
   $lib_font_reg = 'LiberationSans-Regular.ttf';



?>
