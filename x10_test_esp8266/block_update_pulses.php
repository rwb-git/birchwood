<?php

   // same as other block update script except pulses do not have states, just millens


   $fgname = "rwb_pulse_log.txt";

   $flog = fopen($fgname, 'w');   

   fwrite($flog,"start log\n\n");



//   $five_hrs = 5 * 3600;
   

   include 'ping.php';     // ping does this:   $dra33 = getdate();     $current_millenium = $dra33[0]      $mil24 = $current_millenium - 86400

   fwrite($flog,"from ping: current mil $current_millenium  mil24 = 24 hours ago $mil24\n");

   // get the new data


   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 

      exit();
   }

   fwrite($flog,"input $inp\n\n");
   
   $data = explode(" ",trim($inp)); // i have to trim this or it adds another something at the end that duplicates the last millen

   $cnt = 0;

   $current = 0;

   $new_millens = array();

   $new_num_recs = 0;

   fwrite($flog,"now convert all of those to proper millens, and add 5 hours to change EST to UTC\n");

   foreach ($data as &$number){ // lots of my code has & on the second arg, but that's only needed to alter the array itself which typically i don't want to do

      if ($cnt == 0){   

         $current = $number; // + $five_hrs;;
//         $current = $number + $five_hrs;;

         $new_millens[] = $current;
   
         fwrite($flog,"new millens $current\n\n");
         
         $new_num_recs++;  // 4-26-2021 added this line. i think this is why it was missing pulses.

      } else {

         $current = $current + $number; // change the delta to full millenium
      
         $new_millens[] = $current;
      
         fwrite($flog,"new millens $current\n\n");

         $new_num_recs++;
      }

      $cnt++; // after count == 0 == millenium, odd counts will be states, even will be deltas. NOTE THAT this is not array count since it counts both fields
   }



   // load the old file data


   $handle= fopen($fblock, "r");  // @fopen as seen in some examples - the @ supresses error messages

   $old_millens = array();

   $old_num_recs = 0;

   if ($handle) {
       
       while (($buffer = fgets($handle)) !== false) {    // you can put ($handle, 4096) or whatever. 4096 is length: Reading ends when length - 1 bytes have been read, or a newline (which is included in the return value), 
                                                         // or an EOF (whichever comes first). If no length is specified, it will keep reading from the stream until it reaches the end of the line. 
            fwrite($flog,"old file buffer $buffer\n\n");

            $data = explode(" ", trim($buffer));

            foreach ($data as $item){
         
               $old_millens[] = $item;
         
               fwrite($flog,"old millens $item\n\n");

               $old_num_recs++;

            }
       }
       
       if (!feof($handle)) {
           //echo "Error: unexpected fgets() fail\n";
       }

       fclose($handle);
   }






   fwrite($flog,"old num recs $old_num_recs\n\n");

   // merge the two

   $fpm = fopen($fblock, 'w');   

   // the first millen in the new data set:

   $first_new_millen = $new_millens[0];
      
   fwrite($flog,"first new millen $first_new_millen\n");

   // scan the old data. if any record is within ?? seconds of this, discard all that come after, and they will be replaced by the new data block which appears to overlap. otherwise just put the two blocks together, but
   // only keep old data that is less than new data

   $hi_test = $first_new_millen + 30; // no two events should ever be this close

   $lo_test = $first_new_millen - 30;
   
   fwrite($flog,"hi test $hi_test    lo test $lo_test\n");

   $match = -1;

   for ($i=0; $i<$old_num_recs;$i++){

      fwrite($flog,"testing i = $i  old millens $old_millens[$i]\n");

      if (($old_millens[$i] <= $hi_test) && ($old_millens[$i] >= $lo_test)){

         $match = $i;   // keep all the old records lower than $i

         break;
      }
   }

   fwrite($flog,"match $match\n\n");

   if ($match == -1){ // nothing matched so keep all of the old data

      $match = $old_num_recs;
   }
   
   fwrite($flog,"match $match\n\n");

   for ($i=0; $i<$match; $i++){

      $adjusted = $old_millens[$i];

      if ($adjusted >= $mil24){

         if($adjusted < $first_new_millen){ // just keep old data that comes before new data. a weird error was happening

            fwrite($fpm,"$adjusted ");
      
            fwrite($flog,"save rec from old data: $adjusted\n");
         }
      }
   }

   for ($i=0;$i<$new_num_recs;$i++){

      $adjusted = $new_millens[$i]; // + $five_hrs;
      
      if ($adjusted >= $mil24){

         fwrite($fpm,"$adjusted ");
         
         fwrite($flog,"save rec from url data: $adjusted\n");
      }
   }

   fclose($fpm);

   fclose($flog);







?> 
