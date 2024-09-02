<?php    

   

//   echo "OK"; new stuff just checks for 200 = ok

//   if (!($inp = file_get_contents('php://input'))) {     //  The function returns the read data or FALSE on failure. 


      //echo "FAIL";
      http_response_code(400);
    //  exit();

   
  // }

//      $fgname = "erase_new_fake.txt";
      //$fgname = "test_block.txt";
      
  //    $fpm = fopen($fgname, 'r');    // 'a' for append
      //$fpm = fopen($fgname, 'w');    // 'a' for append
/*



604065B0_1_1D3F_0_637A_1_1312_0_27CF_1_FBB_0_272F_1_E8E_0_7609_1_F87_0_    from dream zilla

604065B0_1_1D3F_0_637A_1_1312_0_27CF_1_FBB_0_272F_1_E8E_0_7609_1_F87_0_    POST sent
code 200

      $size = count($yesterday_raf); 

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$yesterday_raf[$i] . " " . $yesterday_on_off_f[$i] . " ");

      }
            
      fwrite($fpm,",");
      
       $size = count($today_raf);   

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$today_raf[$i] . " " . $today_on_off_f[$i] . " ");

      }
   }

   fwrite($fpm,"," . $new_yesterdays_code);     // i believe this is actually today for the float file. who knows why.

   $data = explode(",", $inp);   // yesterday string , today string , yesterdays_code = 2017233 (wrong, date is simply date of month, like 22, in range 1..31)

   $numbers = explode(" ", trim($data[0]));  // yesterday string

   $new_yesterday_ra = array();

   $new_yesterday_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $new_yesterday_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $new_yesterday_on_off[] = $number;
      }
   }
604065B0 1 1D3F 0 637A 1 1312 0 27CF 1 FBB 0 272F 1 E8E 0 7609 1 F87 0  ...  size 
1614833072 1 7487 0 25466 1 4882 0 10191 1 4027 0 10031 1 3726 0 30217 1 3975 0  ...  size 
   */

//   fwrite($fpm,$inp);

 //  fwrite($fpm," ... ");
   $data = fgets($fpm);

 //  var_dump($data);

   $dd = explode(",",trim($data));

//   var_dump($dd);
//   $data = explode(" ",trim(fgets($fpm)));

//   $data = explode("1",trim($inp)); // trim removes leading and trailing whitespace
/*
   foreach ($data as &$number){
   
      fwrite($fpm,$number," ");

   }
*/
   $size = count($data);

 //  fwrite($fpm," size ",$size);
 //  fwrite($fpm,$inp);

   
/*

   $f2 = fopen("test_block.txt","r");

   $d2 = explode("_",trim(fgets($f2)));

   $s2 = count($d2);

   fclose($f2);
*/

  // fwrite($fpm," s2 ",$s2);



   fclose($fpm);
/*
   $fgn2 = "test_block2.txt";
   $f2 = fopen($fgn2, 'w');    // 'a' for append

   fwrite($f2,"xxxa");
   fwrite($f2,$data);
   fwrite($f2,"xxaax");

   fclose($f2);
*/
// 1-21-2019 remove block of code that handles obsolete float_new.txt; old version of this file is in backup_7


/* commented out 1-21-2019

// ------------ read the old data ------------------

   $final_old_yesterday = 0; // newest data already in php list

   $final_old_today = 0;
   
   $fgname = "float_new.txt";

   $_fp = fopen($fgname, "r");

   $data = explode(",", trim(fgets($_fp)));   // yesterday string , today string , yesterdays date

   fclose($_fp);

   $numbers = explode(" ", trim($data[0]));  // yesterday string = secs 1 secs 0 secs 1   for on = 1 and off = 0

   $yesterday_ra = array();

   $yesterday_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $yesterday_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $yesterday_on_off[] = $number;
      }
   }
      
   $yesterday_size = count($yesterday_ra);

   $final_old_yesterday = $yesterday_ra[$yesterday_size - 1];
   
   $numbers = explode(" ", trim($data[1]));   // today string

   $today_ra = array();

   $today_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $today_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $today_on_off[] = $number;
      }
   }

   $today_size = count($today_ra);

   $final_old_today = $today_ra[$today_size - 1];



   $yesterdays_code = $data[2];   // float file uses today's date here


*/



// ---------- read the new data ------------------

/*   

   $data = explode(",", $inp);   // yesterday string , today string , yesterdays_code = 2017233 (wrong, date is simply date of month, like 22, in range 1..31)


   $numbers = explode(" ", trim($data[0]));  // yesterday string

   $new_yesterday_ra = array();

   $new_yesterday_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $new_yesterday_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $new_yesterday_on_off[] = $number;
      }
   }

   $numbers = explode(" ", trim($data[1]));   // today string

   $new_today_ra = array();

   $new_today_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $new_today_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $new_today_on_off[] = $number;
      }
   }





   $new_yesterdays_code = $data[2];
*/

/* commented out 1-21-2019

// ---------- check for midnight and insert the new data-------------------
//
//
//    instead of comparing thousands of items, make these assumptions:
//
//       php has not missed anything within the list it already has, meaning look at the newest item in yesterday and the newest item in today. if the new data has items newer, append to the old php list
//
//       the newest data in both lists is at the ends, so compare the final item in the old php list to items at the end of the new list until a lower item is found. typically this will only find one entry
//       at the end of the today list, but when the internet is screwed up there might be lots of items and they might be in yesterday or today

   


   if ($new_yesterdays_code != $yesterdays_code){

      // midnight has passed so ignore the old yesterday data and treat the old today data as yesterday

      $index = count($new_yesterday_ra);

      while ($final_old_today < $new_yesterday_ra[$index]){

         $today_ra[] = $new_yesterday_ra[$index];
         
         $today_on_off[] = $new_yesterday_on_off[$index];

         $index--;

         if ($index < 0){

            break;
         }
      }

      // now write to file: old today , new today, code

      $fgname = "float_new.txt";
      
      $fpm = fopen($fgname, 'w');    // 'a' for append

      $size = count($today_ra);    // old today_ra will be writen as yesterday

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$today_ra[$i] . " " . $today_on_off[$i] . " ");

      }
            
      fwrite($fpm,",");
      
       $size = count($new_today_ra);   

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$new_today_ra[$i] . " " . $new_today_on_off[$i] . " ");

      }
            
   
   } else { // not midnight so check both days

      $index = count($new_yesterday_ra) - 1;

      while ($final_old_yesterday < $new_yesterday_ra[$index]){  // find oldest (smallest) value that is not in php list

         $index--;

         if ($index < 0){

            break;
         }
      }

      $index++;

      for ($i=$index; $i<count($new_yesterday_ra); $i++){


         $yesterday_ra[] = $new_yesterday_ra[$i];
         
         $yesterday_on_off[] = $new_yesterday_on_off[$i];
      }

      $index = count($new_today_ra) - 1;

      while ($final_old_today < $new_today_ra[$index]){  // find oldest (smallest) value that is not in php list

         $index--;

         if ($index < 0){

            break;
         }
      }

      $index++;

      for ($i=$index; $i<count($new_today_ra); $i++){


         $today_ra[] = $new_today_ra[$i];
         
         $today_on_off[] = $new_today_on_off[$i];
      }

      $fgname = "float_new.txt";
      
      $fpm = fopen($fgname, 'w');    // 'a' for append

      $size = count($yesterday_ra); 

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$yesterday_ra[$i] . " " . $yesterday_on_off[$i] . " ");

      }
            
      fwrite($fpm,",");
      
       $size = count($today_ra);   

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$today_ra[$i] . " " . $today_on_off[$i] . " ");

      }
   }

   fwrite($fpm,"," . $new_yesterdays_code);     // i believe this is actually today for the float file. who knows why.
   
   fclose($fpm);



*/
















// 11-12-2017 new code to test fake stuf ---------------------------------------------------------------------------------------------------------------f
// 11-12-2017 new code to test fake stuf ---------------------------------------------------------------------------------------------------------------f
// 11-12-2017 new code to test fake stuf ---------------------------------------------------------------------------------------------------------------f



// ------------ read the old data ------------------
/*
   $final_old_yesterday = 0; // newest data already in php list

   $final_old_today = 0;
   
   $fgname = "float_new_fake.txt";

   $_fp = fopen($fgname, "r");

   $data = explode(",", trim(fgets($_fp)));   // yesterday string , today string , yesterdays date

   fclose($_fp);

   $numbers = explode(" ", trim($data[0]));  // yesterday string = secs 1 secs 0 secs 1   for on = 1 and off = 0

   $yesterday_raf = array();

   $yesterday_on_off_f = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $yesterday_raf[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $yesterday_on_off_f[] = $number;
      }
   }
      
   $yesterday_size = count($yesterday_raf);

   $final_old_yesterday = $yesterday_raf[$yesterday_size - 1];


   if (strlen($data[1]) > 3){
      
      $numbers = explode(" ", trim($data[1]));   // today string

      $today_raf = array();

      $today_on_off_f = array();

      $index = 0;

      foreach ($numbers as &$number){

         if ($index == 0){

            $today_raf[] = $number;

            $index = 1;

         } else {

            $index = 0;
            
            $today_on_off_f[] = $number;
         }
      }

      $today_size = count($today_raf);

      $final_old_today = $today_raf[$today_size - 1];

   } else {

      $today_size = -1;

      $final_old_today = 0;

   }


   $yesterdays_code = $data[2];   // float file uses today's date here


*/



// ---------- read the new data ------------------

  
/* old comment block from way before 1-21-2019


   $data = explode(",", $inp);   // yesterday string , today string , yesterdays_code = 2017233


   $numbers = explode(" ", trim($data[0]));  // yesterday string

   $new_yesterday_ra = array();

   $new_yesterday_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $new_yesterday_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $new_yesterday_on_off[] = $number;
      }
   }

   $numbers = explode(" ", trim($data[1]));   // today string

   $new_today_ra = array();

   $new_today_on_off = array();

   $index = 0;

   foreach ($numbers as &$number){

      if ($index == 0){

         $new_today_ra[] = $number;

         $index = 1;

      } else {

         $index = 0;
         
         $new_today_on_off[] = $number;
      }
   }


   $new_yesterdays_code = $data[2];

*/




// ---------- check for midnight and insert the new data-------------------
//
//
//    instead of comparing thousands of items, make these assumptions:
//
//       php has not missed anything within the list it already has, meaning look at the newest item in yesterday and the newest item in today. if the new data has items newer, append to the old php list
//
//       the newest data in both lists is at the ends, so compare the final item in the old php list to items at the end of the new list until a lower item is found. typically this will only find one entry
//       at the end of the today list, but when the internet is screwed up there might be lots of items and they might be in yesterday or today

   
/*

   if ($new_yesterdays_code != $yesterdays_code){

      // midnight has passed so ignore the old yesterday data and treat the old today data as yesterday

      $index = count($new_yesterday_ra);

      while ($final_old_today < $new_yesterday_ra[$index]){

         $today_raf[] = $new_yesterday_ra[$index];
         
         $today_on_off_f[] = $new_yesterday_on_off[$index];

         $index--;

         if ($index < 0){

            break;
         }
      }

      // now write to file: old today , new today, code

      $fgname = "float_new_fake.txt";
      
      $fpm = fopen($fgname, 'w');    // 'a' for append

      $size = count($today_raf);    // old today_ra will be writen as yesterday

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$today_raf[$i] . " " . $today_on_off_f[$i] . " ");

      }
            
      fwrite($fpm,",");
      
       $size = count($new_today_ra);   

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$new_today_ra[$i] . " " . $new_today_on_off[$i] . " ");

      }
            
   
   } else { // not midnight so check both days

      $index = count($new_yesterday_ra) - 1;

      while ($final_old_yesterday < $new_yesterday_ra[$index]){  // find oldest (smallest) value that is not in php list

         $index--;

         if ($index < 0){

            break;
         }
      }

      $index++;

      for ($i=$index; $i<count($new_yesterday_ra); $i++){


         $yesterday_raf[] = $new_yesterday_ra[$i];
         
         $yesterday_on_off_f[] = $new_yesterday_on_off[$i];
      }

      $index = count($new_today_ra) - 1;

      while ($final_old_today < $new_today_ra[$index]){  // find oldest (smallest) value that is not in php list

         $index--;

         if ($index < 0){

            break;
         }
      }

      $index++;

      for ($i=$index; $i<count($new_today_ra); $i++){


         $today_raf[] = $new_today_ra[$i];
         
         $today_on_off_f[] = $new_today_on_off[$i];
      }

      $fgname = "float_new_fake.txt";
      
      $fpm = fopen($fgname, 'w');    // 'a' for append

      $size = count($yesterday_raf); 

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$yesterday_raf[$i] . " " . $yesterday_on_off_f[$i] . " ");

      }
            
      fwrite($fpm,",");
      
       $size = count($today_raf);   

      for ($i=0; $i<$size; $i++){

            fwrite($fpm,$today_raf[$i] . " " . $today_on_off_f[$i] . " ");

      }
   }

   fwrite($fpm,"," . $new_yesterdays_code);     // i believe this is actually today for the float file. who knows why.
   
   fclose($fpm);
*/


?>
