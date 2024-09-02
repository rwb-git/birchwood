<?php    

/*

   2-20-2021 esp8266 use same exact approach except delete the part where php counts pulses. assume esp knows more about pulse count than php does, for now.


      5-7-2019 prune test_gallons_fix.txt


      2-17-2019

         start work on the issue where tablet app crashes and sends the wrong gallons for that day when midnight rolls around

         this file will look at the pulse list and calculate gallons from that, and will use the higher value

         at first, don't change anything except add code to this file to create a temporary text file showing what this script calculates from the pulse file.

            if that works ok, change the code to use the larger value and either stop writing the temporary text file, or maybe use it to log when the pulse count was
            used to override the gallons url. <<<<<<<<<< do this.





      11-14-2017

      string from tablet is daycode gallons daycode gallons, 10 or so entries, and is sent every hour or so until one or more acks

      append any missing items to gallons_new and gallons_all text files: file is datecode gallons on one line

      prune gallons_new when it gets to ?? maybe 40 lines down to 35 lines?

      see gallons_per_day.php on how to read a file that has daycode and gallons on each line

      use float_new.php to see how to read a url string that has pairs of values


*/

 

function prune_log_file(){    //----------------------------------------------------------------------------

   //$fgnamep = "test_gallons_fix.txt";
   //$fp = fopen('ip_log.txt', 'a');    // 'a' for append
   $handle = fopen("test_gallons_fix.txt", "r");

   $contents = array();

   if ($handle) {
       while (($line = fgets($handle)) !== false) {


           $contents[] = $line;

      }

      fclose($handle);

   }

   $count = count($contents);

   $size = (int) ($count * 3/4);    // remove about 1/4 of the file

   if ($count >$size){  

      $handle = fopen("test_gallons_fix.txt", "w");

      

      if ($handle) {


         for ($x= ($count - $size); $x<$count; $x++){

           fwrite($handle,$contents[$x]);
         }

         fclose($handle);


      }

   }


}


  

// delete this for esp8266 which will look for 200 instead of messing with "OK"   
// echo "OK";

   $gpm=$_GET["g"];
   


// from gallons_per_day.php ----------------------------------

      $handle = fopen("gallons_new", "r");

      $save_codes = array(); // save for pruning
      $save_gals = array();
      
      if ($handle) {

          while (($line = fgets($handle)) !== false) {

               $numbers = explode(" ", trim($line));

               $index=0;

               $last_code = 0;

               foreach ($numbers as &$number){     // if there is a blank line the code below will use the data from the preceding line

                     $number = intval($number);

                     if ($index == 0){

                        $last_code = $number;

                        $save_codes[] = $number;

                     } else {

                        $save_gals[] = $number;

                     }

                     $index = $index + 1;
               }

         } // while ($line... read the file

         fclose($handle);

      }


   // I only care about the last line. I assume that I do not have any new data that is older than the final line in the file.




// ---------- read the new data ------------------

   

   $data = explode("_", trim($gpm));   // datecode gallons datecode gallons etc. I'm not sure i should trim() this

   $index = 0;

   $new_codes = array();

   $new_gallons = array();

   foreach ($data as &$number){

      if ($index == 0){

         $index = 1;


         if ($number > 0){             // it reads a blank value at the end. maybe because I end the url with a _

            $new_codes[] = $number;

            $url_datecode = $number;  // this will save the final datecode which will be used to see if pulse file has been shifted for midnight
         }

      } else {
         
         $index = 0;

         $new_gallons[] = $number;

         $save_new_gallons = $number;        // this will have the final value
      }
   }


   $use_this = $save_new_gallons; // new php will always use value from esp8266















// new code section 2-19-2019 to use largest gallons count from url or from pulse file, since app fucks up whenever it restarts and reads value from file. that
// value seems to be wrong a lot so I plan to stop using it, and rely on this php code to correct screw ups


// read pulse file and determine which data set is for yesterday; it might not have been shifted yet

// calculate gallons from pulses, and add a line to a temporary file I will monitor to see if this code works. 



// ------------ read the old pulse data ------------------
  
/*

   $fgname = "erase_new_fake.txt";

   $_fp = fopen($fgname, "r");

   $datap = explode(",", trim(fgets($_fp)));   // yesterday string , today string , yesterdays_code = 2017233

   fclose($_fp);

   $numbers = explode(" ", trim($datap[0]));  // yesterday string

   $raf1 = array();

   foreach ($numbers as &$number){

      $raf1[] = $number;
   }
      
   $size1 = count($raf1);   // <<<<<<<<< this x 100 = gallons, so it's the same format as values in gallons_all, like 2019044 123  where 123 x 100 = gallons



   if (strlen($datap[1]) > 3){                         //<<<<<<<<<<<<<<<<<<<<<<<<< this block is different from block above for erase_new.txt

      
      $numbers = explode(" ", trim($datap[1]));   // today string

      $raf2 = array();

      foreach ($numbers as &$number){

         $raf2[] = $number;
      }
         
      $size2 = count($raf2);   // <<<<<<<<< this x 100 = gallons, so it's the same format as values in gallons_all, like 2019044 123  where 123 x 100 = gallons


   } else {

      $size2 = 0;

   }



   $codep = $datap[2];           // <<<<<<<<< this is the thing we want to compare to the datecode from the gallons url, to see which of the values from pulse file is for yesterday


   $fgnamep = "test_gallons_fix.txt";

   if (file_exists($fgnamep)){

      $fpmp = fopen($fgnamep, 'a');    // 'a' for append

   } else {

      $fpmp = fopen($fgnamep, 'w');    // 'a' for append
   }
         
   fwrite($fpmp,"\n\n url datecode: " . $url_datecode . "   datecode in pulse file (erase_new_fake.txt):  " . $codep . "\n\n");


   //$url_datecode = $number;  // this will save the final datecode which will be used to see if pulse file has been shifted for midnight

   if ($url_datecode == $codep){    // if equal, then the final line in gallons_all is for the same day as the first set of data, because the pulse data has been shifted.
         
         fwrite($fpmp,"pulses in first set = " . $size1 . "    pulse data already shifted, so count first pulse set\n\n");

         if ($size1 > $save_new_gallons){ // value from file is > value from url so use it

            fwrite($fpmp,$size1 . " = value from file and it is larger than url value = " . $save_new_gallons . " so use value from file\n\n");

            $use_this = $size1;

         } else {

            fwrite($fpmp,$size1 . " = value from file and it is smaller than url value = " . $save_new_gallons . " so use url value\n\n");
            
            $use_this = $save_new_gallons;
         }


   } else {
         
         fwrite($fpmp,"pulses in second set = " . $size2 . " pulse data has not been shifted, so count second pulse set\n\n");

         if ($size2 > $save_new_gallons){ // value from file is > value from url so use it

            fwrite($fpmp,$size2 . " = value from file and it is larger than url value = " . $save_new_gallons . " so use value from file\n\n");
            
            $use_this = $size2;

         } else {

            fwrite($fpmp,$size2 . " = value from file and it is smaller than url value = " . $save_new_gallons . " so use url value\n\n");

            $use_this = $save_new_gallons;
         }
   }

   fwrite($fpmp,"use this value " . $use_this . "\n\n");


      $size = count($new_codes);
      for ($i=0; $i<$size; $i++){

         $this_code = $new_codes[$i];

       //  fwrite($fpmp,"new code " . $this_code . "\n\n");

      }
  
//   fclose($fpmp);


   // "use_this" is the gallons value that will replace the final gallons value in the url, to be written to the files


*/























// look at each data set in the new list. assume they are in order. if daycode > than last_code insert the new stuff
// but if the file is large just save the new data and re-write the entire file

   if (count($save_codes) > 60){ // wait and write the entire file

      $prune = 1;

      $size = count($new_codes);

      for ($i=0; $i<$size; $i++){

         $this_code = $new_codes[$i];

         if ($this_code > $last_code){

            $last_code = $this_code;

            $save_codes[] = $this_code;

            $save_gals[] = $new_gallons[$i];
         }
      }

   } else { // just append the new stuff

      $prune = 0;

      $fgname = "gallons_new";

      if (file_exists($fgname)){

         $fpm = fopen($fgname, 'a');    // 'a' for append

      } else {

         $fpm = fopen($fgname, 'w');    // 'a' for append
      }

      $size = count($new_codes);

            
//      fwrite($fpmp,"prepare to append to gallons_new " . $size . " = size of new_codes " . $last_code . " = last_code\n\n");

      for ($i=0; $i<$size; $i++){

         $this_code = $new_codes[$i];

            
//         fwrite($fpmp,"appending ... this_code = " . $this_code . "\n\n");

         if ($this_code > $last_code){

            if ($i == ($size - 1)){

               fwrite($fpm,$this_code . " " . $use_this . "\n");     // make sure last value is the largest gallons count found in the code above

            } else {

               fwrite($fpm,$this_code . " " . $new_gallons[$i] . "\n");

            }

            $last_code = $this_code;

            $save_codes[] = $this_code;            // i don't think this is used again if prune == 0

            $save_gals[] = $new_gallons[$i];       // i don't think this is used again if prune == 0

         }

      }
      
      fclose($fpm);

   }




// now do the same for the huge perpetual list


      $last_code = 0; // I had to add this while testing a code change since the fake fffgallons_all file is empty


      $handle = fopen("gallons_all", "r");
      
      if ($handle) {

          while (($line = fgets($handle)) !== false) {

               $numbers = explode(" ", trim($line));

               $index=0;

               $last_code = 0;

               foreach ($numbers as &$number){     // if there is a blank line the code below will use the data from the preceding line

                     $number = intval($number);

                     if ($index == 0){

                        $last_code = $number;      // this is confusing but i think it is supposed to get the date code from the last line in the file, since it is reset to 0 for each exploded line
                     }

                     $index = $index + 1;
               }

         } // while ($line... read the file

         fclose($handle);

      }

   // I only care about the last line. I assume that I do not have any new data that is older than the final line in the file.




// look at each data set in the new list. assume they are in order. if daycode > than last_code insert the new stuff


   $fgname = "gallons_all";

   if (file_exists($fgname)){

      $fpm = fopen($fgname, 'a');    // 'a' for append

   } else {

      $fpm = fopen($fgname, 'w');    // 'a' for append
   }

   $size = count($new_codes);
   
//   fwrite($fpmp,"prepare to append to gallons_all " . $size . " = size of new_codes " . $last_code . " = last_code\n\n");

   for ($i=0; $i<$size; $i++){

      $this_code = $new_codes[$i];
         
//      fwrite($fpmp,"appending ... this_code = " . $this_code . "\n\n");

      if ($this_code > $last_code){

         if ($i == ($size - 1)){

            fwrite($fpm,$this_code . " " . $use_this . "\n");     // make sure last value is the largest gallons count found in the code above

         } else {

            fwrite($fpm,$this_code . " " . $new_gallons[$i] . "\n");
         }

         $last_code = $this_code;

      }

   }
   
   fclose($fpm);



// now prune the small list if needed

   $lines_in_file = count($save_codes);

   if ($prune == 1){
   
      $fgname = "gallons_new";

      $fpm = fopen($fgname, 'w');    // 'a' for append

      $start = $lines_in_file - 33;    // if it has 44 lines this will be 11. we will keep lines 11..44

      for ($i=$start; $i<$lines_in_file;$i++){
      
         if ($i == ($lines_in_file - 1)){

            fwrite($fpm,$save_codes[$i] . " " . $use_this . "\n");     // make sure last value is the largest gallons count found in the code above

         } else {

            fwrite($fpm,$save_codes[$i] . " " . $save_gals[$i] . "\n");
         }

      }
   
      fclose($fpm);

   }


//   fclose($fpmp);  // close the log file - this is the old log file that said whether it used url or pulse file. i guess i don't want this in x10?


   // added 5-7-2019 prune if too big
/*
   //$ip_filesize = filesize("test_gallons_fix.txt");    // size in bytes. hmm. prune at 20 kb and see what that looks like?
                                             // 57 lines was about 1450 bytes

//   $max_file_size = 1000;        // test on small file
   $max_file_size = 20 *1024;

   if ($ip_filesize > $max_file_size){

      prune_log_file();       // removes about 1/4 of the file

   }

*/



?>
