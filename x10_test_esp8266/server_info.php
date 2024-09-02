<?php

   $server_minute = date('i');
   $server_hour = date('G'); // 0..23
   $server_second = date('s'); // 0..59
   
   $server_timezone = date('e');
   
   $dra33 = getdate();
   $derp = $dra33[0];

   echo "$server_hour:$server_minute:$server_second  $server_timezone  millenium $derp";

   echo " ";

  
   date_default_timezone_set('UTC');

   $server_minute = date('i');
   $server_hour = date('G'); // 0..23
   $server_second = date('s'); // 0..59
   $server_date = date('j'); // 1..31
   $server_day_of_week = date('w'); // 0 is sunday
               //$day2 = intval(date("w")) - 1; // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight
   
   $server_timezone = date('e');

   echo "$server_hour:$server_minute:$server_second  $server_timezone";
   
   $fpcut = fopen("dst_flag.txt", "r");

   $dst_flag = fgets($fpcut); 
   
   fclose($fpcut);

   echo "<br><br>dst flag $dst_flag<br><br>";

   if ($dst_flag == 1){

      $dst_fix = 1;

   } else {
      
      $dst_fix = 0;     // it looks like dst_flag.txt is manually edited. all i know is that when dst is "on" the value is 1. 
   }

   $total_fix = 5 - $dst_fix; // in the winter, EST = UTC - 5     in the summer EDT = UTC - 4

   if ($server_hour >= $total_fix){

      $utc_fixed = $server_hour - $total_fix;

      echo "UTC and durham EST or EDT have the same date right now, $server_date, and day of week $server_day_of_week (sunday is 0)";

   } else {

      $utc_fixed = 24 + $server_hour - $total_fix; // if total fix is 5 and server hour is 4: 24 + 4 - 5 = 23 which is correctly 5 hours behind server: 23 0 1 2 3 4
      
      echo "UTC and durham EST or EDT DO NOT have the same date right now: server date = $server_date;   server day of week =  $server_day_of_week (sunday is 0)";
   }

   echo "<br><br>UTC corrected to agree with EDT or EST is $utc_fixed which is server hour $server_hour - total fix $total_fix";

   echo "<br><br>remember that this time difference issue does not affect bar chart colors in top dir (android tablet data) because date2 is used, and is from the tablet, and there is code to handle the time zone difference";

   // server hour UTC      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
   //
   // EST = UTC - 5        19 20 21 22 23 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18
   // same day             xxxxxxxxxxxxxx yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
   //
   // EDT = UTC - 4        20 21 22 23 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
   // same day             xxxxxxxxxxx yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy

   // so, the server day is only wrong when UTC hour < total fix
   //
   //    5-25-2021 so what do i do in gallons_per_day_fake.php? it looks like i do the above calculations, and if server day is wrong, then i subtract 1 from server day of week, and make sure it's 0..6, then use that for bar colors.


/* from gallons_per_day_fake.php

                  // new code to fix leap day starts here. 3-1-2020

                  $date33 = DateTime::createFromFormat('z Y', ($day ) . ' ' . $year);  // java day 1..366  php day 0..355 so subtract 1
                  
                  $isLeapYear = $date33->format('L');

                  if (($isLeapYear == 1) && ($day > 59)){

                        if ($day == 60)  {  // for feb 29 I cannot use the normal code because php appears to "fix" it. but I can create a dateTime if I inject feb 29 and the year

                           $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . $year); 

                        } else {

                           $date33 = DateTime::createFromFormat('z Y', ($day - 2 ) . ' ' . $year); // java day 1..366  php day 0..355 so subtract 1. 5-16-2021 why do i subtract 2 here? i can't remember or find any notes. i assume that
                                                                                                   // php must not handle leap year correctly here, which seems unlikely. oh well, I'll find out in 2024 i suppose
                        }

                  }  else { // not leap year so use old code, also use this code for leap year days up to feb 28 = day of year 59 (based on jan 1 = 1 which is what is in my file; php uses jan 1 = 0 so we use $i3 -1 here)
 
                     // old code that worked until leap day 2020

                     $date33 = DateTime::createFromFormat('z Y', ($day - 1) . ' ' . $year);  // java day 1..366  php day 0..355 so subtract 1
                  }

                  $m87 = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

                  $date99 = intval($m87);

                  imagettftext($my_img->img, 12, 0,  $x + 7, $tank_y8 + 16 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text

                  if (($date99 == 1) || ($i == $stopx)){

                     $m87 = $date33->format('M');  // 'M' Jan   'F' January

                     imagettftext($my_img->img, 12, 0,  $x + 7, $tank_y8 + 32 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text
                  }



      d 	Day of the month, 2 digits with leading zeros 	01 to 31

      n 	Numeric representation of a month, without leading zeros 	1 through 12

      Y 	A full numeric representation of a year, 4 digits 	Examples: 1999 or 2003

      z 	The day of the year (starting from 0) 	0 through 365

*/

//   $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . 2020); // n d Y = month 1..12, date 01..31, year 2020 
   
   echo "<br><br> ";

   echo "php day of year is 0..365. my original format from android java uses 1.366 which is what online calendars use. in all the following i use the php format for simplicity, so add 1 to compare to online calendars.";
   
   echo "<br><br> ";
   echo "so, 58 here would be 59 online calendar which is feb 28 in 2020, and is correct below";
   echo "<br><br> ";
   echo "59 here (60 online calendar) should be feb 29 in 2020, but shows march 1 below which is wrong OR AT LEAST IT USED TO BE, NOW IT IS CORRECT. fucking php changed some shit";
   echo "<br><br> ";
   echo "60 here (61 online calendar) should be march 1 in 2020, but shows march 2 below which is wrong - nope, not anymore. i did not change anything except the format order cuz php fucked that up";
   echo "<br><br> ";
   
   echo "or, heaven forbid, php fixed the bug that i used to compensate for.......which in a perfect world makes sense";
   echo "<br><br> ";
   $doy = 58;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doy); // n d Y = month 1..12, date 01..31, year 2020       stupid shit php used to allow 'z Y' now it crashes
   //$date33 = DateTime::createFromFormat('z Y',$doy . ' '  . 2020); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";

   echo "i input day of year $doy for  2020, and it returns date $dom and month $mon";

   $doy++;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doy); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";

   echo "i input day of year $doy for  2020, and it returns date $dom and month $mon";


   $doy++;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doy); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";

   echo "i input day of year $doy for  2020, and it returns date $dom and month $mon";


   $doy++;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doy); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";

   echo "i input day of year $doy for  2020, and it returns date $dom and month $mon";


   $doy++;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doy); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";
   echo "i input day of year $doy for  2020, and it returns date $dom and month $mon";



   echo "<br><br> ";
   echo "<br><br> ";
   echo "so, in gallons_per_day_fake.php, i fudge things for days after feb 28";

/*
                        if ($day == 60)  {  // for feb 29 I cannot use the normal code because php appears to "fix" it. but I can create a dateTime if I inject feb 29 and the year

                           $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . $year); 

                        } else {

                           $date33 = DateTime::createFromFormat('z Y', ($day - 2 ) . ' ' . $year); // java day 1..366  php day 0..355 so subtract 1. 5-16-2021 why do i subtract 2 here? i can't remember or find any notes. i assume that
                                                                                                   // php must not handle leap year correctly here, which seems unlikely. oh well, I'll find out in 2024 i suppose
                        }

*/
   echo "<br><br> ";

   echo "for leap day, feb 29 which is 60 online calendar and 59 here, i force month feb and date 29, and it works";
                           
   $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . 2020); 

 
   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";

   echo "it returns date $dom and month $mon";

   echo "<br><br> ";
   echo "for the rest of the days in a leap year, i use the ordinary format except i subtract an extra day. in normal years i subtract 1 to convert java range to php range, so for the rest of leap year i subtract 2";
   echo "<br><br> ";
 
   $doy = 61;
   $doyfix = $doy - 2;

   $date33 = DateTime::createFromFormat('Y z',2020 . ' ' . $doyfix); // n d Y = month 1..12, date 01..31, year 2020 

   $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

   $mon = $date33->format('M');  // 'M' Jan   'F' January

   echo "<br><br> ";



   echo "i input day of year $doy for  2020, but i subtract 2 to use $doyfix, and it returns date $dom and month $mon";
 
   echo "<br><br> ";

  
   echo "the following uses the actual code from gallons_per_day_fake.php AND NOTE THAT IT NO LONGER WORKS RIGHT. php changed some shit because this used to work by fixing a php bug that no longer happens";


   echo "<br><br> ";

   for($year = 2019;$year < 2026;$year++){

      for ($day = 57; $day<64; $day++){

         $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day ));  // java day 1..366  php day 0..355 so subtract 1

         $isLeapYear = $date33->format('L');
      
         if (($isLeapYear == 1) && ($day > 59)){

            if ($day == 60)  {  // for feb 29 I cannot use the normal code because php appears to "fix" it. but I can create a dateTime if I inject feb 29 and the year

               $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . $year); 

            } else {

               $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 2 )); // java day 1..366  php day 0..355 so subtract 1. 5-16-2021 why do i subtract 2 here? i can't remember or find any notes. i assume that
                                                                                       // php must not handle leap year correctly here, which seems unlikely. oh well, I'll find out in 2024 i suppose
            }

         }  else { // not leap year so use old code, also use this code for leap year days up to feb 28 = day of year 59 (based on jan 1 = 1 which is what is in my file; php uses jan 1 = 0 so we use $i3 -1 here)
    
           // old code that worked until leap day 2020

           $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 1));  // java day 1..366  php day 0..355 so subtract 1
         }

         $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

         $mon = $date33->format('M');  // 'M' Jan   'F' January

         echo "<br>";

         echo "$year day of year $day date  $dom  month $mon";

      }

      echo "<br>";
      echo "<br>";
   }

   echo "the following USES CORRECTED as of 1-2-2024 code from gallons_per_day_fake.php";


   echo "<br><br> ";

   for($year = 2019;$year < 2026;$year++){

      for ($day = 57; $day<64; $day++){
/*
         $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day ));  // java day 1..366  php day 0..355 so subtract 1

         $isLeapYear = $date33->format('L');
     
     
         if (($isLeapYear == 1) && ($day > 59)){

            if ($day == 60)  {  // for feb 29 I cannot use the normal code because php appears to "fix" it. but I can create a dateTime if I inject feb 29 and the year

               $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . $year); 

            } else {

               $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 2 )); // java day 1..366  php day 0..355 so subtract 1. 5-16-2021 why do i subtract 2 here? i can't remember or find any notes. i assume that
                                                                                       // php must not handle leap year correctly here, which seems unlikely. oh well, I'll find out in 2024 i suppose
            }

         }  else { // not leap year so use old code, also use this code for leap year days up to feb 28 = day of year 59 (based on jan 1 = 1 which is what is in my file; php uses jan 1 = 0 so we use $i3 -1 here)
    
           // old code that worked until leap day 2020

           $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 1));  // java day 1..366  php day 0..355 so subtract 1
         }
*/


         $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 1));  // java day 1..366  php day 0..355 so subtract 1

         $dom = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1

         $mon = $date33->format('M');  // 'M' Jan   'F' January

         echo "<br>";

         echo "$year day of year $day date  $dom  month $mon";

      }

      echo "<br>";
      echo "<br>";
   }

?>
