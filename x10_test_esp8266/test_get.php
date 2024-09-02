<?php



   echo "123";

   /*


   $fgname = "rwb_log.txt";

   $flog = fopen($fgname, 'w');   

   fwrite($flog,"start log\n\n");



   $five_hrs = 5 * 3600;
   

   include 'ping.php';     // ping does this:   $dra33 = getdate();     $current_millenium = $dra33[0]      $mil24 = $current_millenium - 86400

   // get the new data


   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 

      exit();
   }

   fwrite($flog,"input $inp\n\n");
   
   $data = explode(" ",trim($inp)); // i have to trim this or it adds another something at the end that duplicates the last millen

   $cnt = 0;

   $current = 0;

   $two = 2;

   $new_millens = array();

   $new_states = array();

   $new_num_recs = 0;

   foreach ($data as &$number){ // lots of my code has & on the second arg, but that's only needed to alter the array itself which typically i don't want to do

      if ($cnt == 0){   

         $current = $number + $five_hrs;;

         $new_millens[] = $current;
   
         fwrite($flog,"new millens $current\n\n");

      } else {

         if (($cnt % $two) == 0){    // even cnt = delta

            $current = $current + $number; // change the delta to full millenium
         
            $new_millens[] = $current;
         
            fwrite($flog,"new millens $current\n\n");

         } else {    // odd cnt = state

            $new_states[] = $number;

            $new_num_recs++;

         }
      }

      $cnt++; // after count == 0 == millenium, odd counts will be states, even will be deltas. NOTE THAT this is not array count since it counts both fields
   }



   // load the old file data


   $handle= fopen("flow_state.txt", "r");  // @fopen as seen in some examples - the @ supresses error messages
//   $handle= fopen("old_block", "r");  // @fopen as seen in some examples - the @ supresses error messages

   $old_millens = array();

   $old_states = array();

   $old_num_recs = 0;


   if ($handle) {
       
       while (($buffer = fgets($handle)) !== false) {    // you can put ($handle, 4096) or whatever. 4096 is length: Reading ends when length - 1 bytes have been read, or a newline (which is included in the return value), 
                                                         // or an EOF (whichever comes first). If no length is specified, it will keep reading from the stream until it reaches the end of the line. 

            fwrite($flog,"old file buffer $buffer\n\n");

            $data = explode(" ", trim($buffer));

            $index = 0;

            foreach ($data as $item){
               
               if ($index == 0){
            
                  $old_millens[] = $item;
            
                  fwrite($flog,"old millens $item\n\n");

                  $index = 1;

                  $old_num_recs++;

               } else {

                  $old_states[] = $item;

                  $index = 0;

               }


            }

       }
       
       if (!feof($handle)) {
           //echo "Error: unexpected fgets() fail\n";
       }

       fclose($handle);
   }






   fwrite($flog,"old num recs $old_num_recs\n\n");




   // merge the two

   $fgname = "flow_state.txt";
//   $fgname = "new_test_block.txt";

   $fpm = fopen($fgname, 'w');   




   // the first millen in the new data set:

   $first_new_millen = $new_millens[0];
      
//   fwrite($fpm,"first new millen $first_new_millen\n");

   // scan the old data. if any record is within ?? seconds of this, discard all that come after, and they will be replaced by the new data block which appears to overlap. otherwise just put the two blocks together

   $hi_test = $first_new_millen + 30; // no two events should ever be this close

   $lo_test = $first_new_millen - 30;
   
//   fwrite($fpm,"hi test $hi_test    lo test $lo_test\n");

   $match = -1;

   for ($i=0; $i<$old_num_recs;$i++){

//      fwrite($fpm,"testing i = $i  old millens $old_millens[$i]\n");

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
      //$adjusted = $old_millens[$i] + $five_hrs; old should already be ok

      fwrite($fpm,"$adjusted $old_states[$i] ");

   }

   for ($i=0;$i<$new_num_recs;$i++){

      $adjusted = $new_millens[$i]; // + $five_hrs;

      fwrite($fpm,"$adjusted $new_states[$i] ");

      //fwrite($fpm,"$new_millens[$i] $new_states[$i] ");

   }
  

   fclose($fpm);

   fclose($flog);

*/

?> 
