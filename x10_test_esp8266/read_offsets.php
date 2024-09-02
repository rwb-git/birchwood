<?php
   $filename = 'air_calib.txt';   // offset, offset_H, offset_L

   $offset = 0;
   $offset_H = 0;
   $offset_L = 0;
   
   if (file_exists($filename)) {

    
      $fp = fopen($filename, 'r');   // r read, w write, r+ read and write

      $numbers = explode(" ", trim(fgets($fp)));   // one string offset offset_H offset_L

      fclose($fp);

      $pindex = 0;

      foreach ($numbers as &$number){

         if ($pindex == 0){

            $offset = $number;
         }


         if ($pindex == 1){

            $offset_H = $number;
         }


         if ($pindex == 2){

            $offset_L = $number;
         }

         $pindex = $pindex + 1;
      }

   }



?>
