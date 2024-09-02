<?php

// i compared some biz data and their milleniums and my milleniums were the same within 3 seconds. might be a good idea to check this periodically, or do lots of block dump tests

// test_block_list_style.php does the one liners, but i'm going to change to a single line so all the other code will be ok as is








// remember the 5 hour difference that will apply to all data sent from esp. or will esp apply that so that dream and biz code is simpler? 







// test_1_block.php writes both files. now change test_block to merge the new data with old_block
/*
   which is better: one long string or one record per line.

      long string is harder to edit, i suppose.

      one liners still need to be exploded, one at a time, but it might be simpler to load lines till they are all done.

      appending is probably about the same. i have to be careful manually editing biz because sometimes the next line is glued onto the preceding one.

      try one liners at first. hell, maybe code both and then decide.






*/


   include 'ping.php';     // ping does this:   $dra33 = getdate();     $current_millenium = $dra33[0]      $mil24 = $current_millenium - 86400

   // get the new data


   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 

      exit();
   }

   $data = explode(" ",trim($inp)); // i have to trim this or it adds another something at the end that duplicates the last millen

   $cnt = 0;

   $current = 0;

   $two = 2;

   $new_millens = array();

   $new_states = array();

   $new_num_recs = 0;

   foreach ($data as &$number){ // lots of my code has & on the second arg, but that's only needed to alter the array itself which typically i don't want to do

      if ($cnt == 0){   

         $current = $number;

         $new_millens[] = $current;

      } else {

         if (($cnt % $two) == 0){    // even cnt = delta

            $current = $current + $number; // change the delta to full millenium
         
            $new_millens[] = $current;

         } else {    // odd cnt = state

            $new_states[] = $number;

            $new_num_recs++;

         }
      }

      $cnt++; // after count == 0 == millenium, odd counts will be states, even will be deltas. NOTE THAT this is not array count since it counts both fields
   }



   // load the old file data


   $handle= fopen("old_block", "r");  // @fopen as seen in some examples - the @ supresses error messages

   $old_millens = array();

   $old_states = array();

   $old_num_recs = 0;


   if ($handle) {
       
       while (($buffer = fgets($handle)) !== false) {    // you can put ($handle, 4096) or whatever. 4096 is length: Reading ends when length - 1 bytes have been read, or a newline (which is included in the return value), 
                                                         // or an EOF (whichever comes first). If no length is specified, it will keep reading from the stream until it reaches the end of the line. 

            $data = explode(" ", trim($buffer));

            $old_millens[] = $data[0];

            $old_states[] = $data[1];

            $old_num_recs++;

       }
       
       if (!feof($handle)) {
           //echo "Error: unexpected fgets() fail\n";
       }

       fclose($handle);
   }










   // merge the two

   $fgname = "new_test_block.txt";

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

   if ($match == -1){ // nothing matched so keep all of the old data

      $match = $old_num_recs;
   }

   $five_hrs = 0; //5 * 3600;

   for ($i=0; $i<$match; $i++){

      $adjusted = $old_millens[$i] + $five_hrs;

      fwrite($fpm,"$adjusted $old_states[$i] ");

   }

   for ($i=0;$i<$new_num_recs;$i++){

      $adjusted = $new_millens[$i] + $five_hrs;

      fwrite($fpm,"$adjusted $new_states[$i] ");

      //fwrite($fpm,"$new_millens[$i] $new_states[$i] ");

   }
  
//   fwrite($fpm,"\n"); // is this good or bad? without this i think append works better, so skip this for now. but wait - update_type_1.php re-writes the whole file and does not do this, so it's ok like this

   fclose($fpm);


/*


new_test_block.txt


top 5 are from bottom of old block
1614800072 1
1614802559 0
1614811025 1
1614821907 0
1614831098 1


these five are similar to but different from top of old block; all the ones below here are from new data

the current flowa has 12 records but it's hard to see the actual values, so just check the first and last ones on spreadsheet timestamps.ods


03/04/2021 04:44:32 this agrees with show_file

1614833072 1
1614840559 0
1614866025 1
1614870907 0
1614881098 1



1614885125 0
1614895156 1
1614898882 0
1614929099 1
1614933074 0
1614957492 1
1614962832 0
03/05/2021 16:47:12 this agrees with show_file




fix old_block: it had 2 '1' states together
1614800072 1     
1614802559 0 
1614811025 1
1614821907 0 
1614831098 1
1614832070 0
1614833070 1
1614840552 0
1614866035 1
1614870917 0
1614881092 1


new_test_block.txt after fixing old_block
1614800072 1
1614802559 0
1614811025 1
1614821907 0
1614831098 1
1614832070 0 from here up are old_block

1614833072 1
1614840559 0
1614866025 1
1614870907 0
1614881098 1
1614885125 0
1614895156 1
1614898882 0
1614929099 1
1614933074 0
1614957492 1
1614962832 0





this looks ok. run again tomorrow and see if it merges on the new data cleanly.

saturday 3-6 

old block before
1614800072 1     
1614802559 0 
1614811025 1
1614821907 0 
1614831098 1
1614832070 0
1614833070 1
1614840552 0
1614866035 1
1614870917 0
1614881092 1 

new_test_block.txt before
1614800072 1
1614802559 0
1614811025 1
1614821907 0
1614831098 1
1614832070 0
1614833072 1
1614840559 0
1614866025 1
1614870907 0
1614881098 1
1614885125 0
1614895156 1
1614898882 0 
1614929099 1
1614933074 0
1614957492 1
1614962832 0


first new millen 1614929099
hi test 1614929129    lo test 1614929069
testing i = 0  old millens 1614800072
testing i = 1  old millens 1614802559
testing i = 2  old millens 1614811025
testing i = 3  old millens 1614821907
testing i = 4  old millens 1614831098
testing i = 5  old millens 1614832070
testing i = 6  old millens 1614833070
testing i = 7  old millens 1614840552
testing i = 8  old millens 1614866035
testing i = 9  old millens 1614870917
testing i = 10  old millens 1614881092
1614800072 1
1614802559 0
1614811025 1
1614821907 0
1614831098 1
1614832070 0
1614833070 1
1614840552 0
1614866035 1
1614870917 0
1614881092 1 from old block

1614929099 1 new from here down
1614933074 0
1614957492 1
1614962832 0 
1614981439 1 new
1614987322 0      add 5 hours = 5 x 3600 == 1615005322 which is 3 seconds lower than value in biz file



biz flow_state.txt looks like this at 8:30 am saturday march 3 2021

1614947101 1 1614951075 0 1614975495 1 1614980833 0 1614999441 1 1615005325 0 

new_test_block.txt

1614818072 1 1614820559 0 1614829025 1 1614839907 0 1614849098 1 1614850070 0 1614851070 1 1614858552 0 1614884035 1 1614888917 0 1614899092 1 1614947099 1 1614951074 0 1614975492 1 1614980832 0 1614999439 1 1615005322 0 1615037671 1 
                                                                                                                                               1614947101 1 1614951075 0 1614975495 1 1614980833 0 1614999441 1 1615005325 0 
                                                                                                                                               +2 sec       +1 sec       +3           +1           +2           +3           new data








new_test_block.txt
1614818072 1 1614820559 0 1614829025 1 1614839907 0 1614849098 1 1614850070 0 1614851070 1 1614858552 0 1614884035 1 1614888917 0 1614899092 1 1614947099 1 1614951074 0 1614975492 1 1614980832 0 1614999439 1 1615005322 0 1615037671 1 1615041684 0 
                                                                                                                                               1614947101 1 1614951075 0 1614975495 1 1614980833 0 1614999441 1 1615005325 0 


flow_state.txt

1614947101 1 1614951075 0 1614975495 1 1614980833 0 1614999441 1 1615005325 0 


*/






















?> 
