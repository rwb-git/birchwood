<?php

// the best analysis and explanation of this code, particularly pad_days and how it works, is in text file fix_gallons_new_year. there is a good summary at the top of that file.

// this url is on both biz and dream, and looks at things like leap year calculations, UTC vs EDT and EST, etc.,   http://www.fork20.xyz/birchwood/x10_test_esp8266/server_info.php

// read gallons per day file

      $handle = fopen("gallons_new", "r");
      //$handle = fopen("gallons_2017", "r");

// typical data from that file. 
//
//            2019362 103
//            2019363 109
//            2019364 99
//            2019365 119
//            2020001 95

// jan 1 2020 fucked up, and I think the error was due to leap year. I changed the leap year adjustment in this file to look at last year, rather than this year, so on jan 1 2021 it should use the new code.
//
// also, below I discuss the fact that using last_year as 2017 is probably adding 365 phony elements to the array every day of the year, but since it works, leave it alone for now. the mistake
// is that I have code that should only trigger in january, when the first part of gallons_new has days like 355 and the january section has days like 1,2,3, and I need to adjust so that the first 
// day in the array is at 0. but since I never fixed the stupid idea of hard coding last_year as 2017, I think that on every day from jan 1 2019 onward it has added 365 phony elements. 
//
// the only question remaining is whether or not it will adjust correctly for January in years that follow leap year, like jan 2021

/*
   3-18-2021 php 8 says date2 not defined, and that's why the color bar chart has been wrong. i deleted that code but i need to analyze what actually happens in the new code

      user@user-GA-78LMT-USB3-R2:/mtv/fake_home/html_birchwood/x10_test_esp8266$ grep "date(" *
         
         functions.php:   $dra33 = getdate();
         functions.php:   $dra33 = getdate();
         
         gallons_per_day_fake.php:         $php_day = 1 + intval(date("z"));  // java day of year is 1... while php is 0...
         gallons_per_day_fake.php:         $php_hour = intval(date("G"));  // 0..23   does not have leading zeros
         gallons_per_day_fake.php:         $php_minute = intval(date("i"));  // 00..59   has leading zeros
         gallons_per_day_fake.php:         $php_date = intval(date("j"));  // 1..31
         gallons_per_day_fake.php:               $day2 = intval(date("w")) - 1; // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight
         
         load_pulses.php:$dra33 = getdate();
         
         main_file_functions.php:   $dra = getdate();
         
         ping.php:   $dra33 = getdate();
         
         post_288.php:   $dra = getdate();
         
         pulse.php:   include 'ping.php';     // ping does this:   $dra33 = getdate();     $current_millenium = $dra33[0]      $mil24 = $current_millenium - 86400
         
         read_time_file.php:   $server_minute = date('i');
         read_time_file.php:   $server_hour = date('G'); // 0..23
         read_time_file.php:   $server_second = date('s'); // 0..59
         read_time_file.php:   $server_timezone = date('e');
         read_time_file.php:   $dra33 = getdate();
         
         tank_status.php:   $dra33 = getdate();
         
         update_type_1.php:   include 'ping.php';     // ping does this:   $dra33 = getdate(), and $current_millenium = $dra[0] and $mil24 = current - 86400
         
         water_normal_8_hrs.php:   $dra = getdate();
         
         water_test_exp.php:   $dra = getdate();
      
      
      user@user-GA-78LMT-USB3-R2:/mtv/fake_home/html_birchwood/x10_test_esp8266$ grep UTC *
      
         
         block_update_pulses.php:   fwrite($flog,"now convert all of those to proper millens, and add 5 hours to change EST to UTC\n");
         block_update_type_1.php:   fwrite($flog,"now convert all of those to proper millens, and add 5 hours to change EST to UTC\n");
         
         draw_flow_adc.php:   date_default_timezone_set('UTC');
         
         esp_todo.txt:   online biz pulse old-style: are these master seconds since midnight? is the new file UTC?
         
         read_time_file.php:   date_default_timezone_set('UTC');
         read_time_file.php:      // biz server minute 18  hour 15  second 45  timezone UTC
         read_time_file.php:      // dream after date_default.. server minute 21  hour 15  second 58  timezone UTC
         
         server_date.txt:server minute 21  hour 15  second 58  timezone UTC

   i think this is true: 

      date_default_timezone affects both date and getdate

      all the things i get from date() like hour and second will agree with the millenium i get from dra33 = getdate()

      this is all UTC on my esp and php. the only two things that matter are the legends which i shift by 5 hours and maybe 1 for dst, and the colors in the daily gallon bar chart

      if the bars are colored wrong within 5 hours of midnight does anyone actually care, except me



*/



      if ($handle) {

         $ra_index = 0;

         $dy87 = 20;

         $debug = 0;

         $new_y = $tank_y8 + 20;

         // added this block 5-25-2021. the code that is indented a lot is new; the old code is indented normally, 
                         
//                  $server_minute = date('i');
//                  $server_hour = date('G'); // 0..23
//                  $server_second = date('s'); // 0..59
//                  $server_date = date('j'); // 1..31
//                  $server_day_of_week = date('w'); // 0 is sunday
                              //$day2 = intval(date("w")) - 1; // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight
                  
//                  $server_timezone = date('e');

//                  echo "$server_hour:$server_minute:$server_second  $server_timezone";
   
                           date_default_timezone_set('UTC');
 
         $php_day = 1 + intval(date("z"));  // java day of year is 1... while php is 0...
         
         $php_hour = intval(date("G"));  // 0..23   does not have leading zeros
         
         $php_minute = intval(date("i"));  // 00..59   has leading zeros
         
         $php_date = intval(date("j"));  // 1..31
                
                  $f7pcut = fopen("dst_flag.txt", "r");

                  $dst_flag = fgets($f7pcut); 
                  
                  fclose($f7pcut);

//                  echo "<br><br>dst flag $dst_flag<br><br>";

                  if ($dst_flag == 1){

                     $dst_fix = 1;

                  } else {
                     
                     $dst_fix = 0;     // it looks like dst_flag.txt is manually edited. all i know is that when dst is "on" the value is 1. 
                  }

                  $total_fix = 5 - $dst_fix; // in the winter, EST = UTC - 5     in the summer EDT = UTC - 4

                  if ($php_hour >= $total_fix){
                  
                     $need_to_fix_server_day = 0;

//                     $utc_fixed = $php_hour - $total_fix;

  //                   echo "UTC and durham EST or EDT have the same date right now, $server_date, and day of week $server_day_of_week (sunday is 0)";

                  } else {
                     
                     $need_to_fix_server_day = 1;

//                     $utc_fixed = 24 + $php_hour - $total_fix; // if total fix is 5 and server hour is 4: 24 + 4 - 5 = 23 which is correctly 5 hours behind server: 23 0 1 2 3 4
                     
    //                 echo "UTC and durham EST or EDT DO NOT have the same date right now: server date = $server_date;   server day of week =  $server_day_of_week (sunday is 0)";
                  }

      //            echo "<br><br>UTC corrected to agree with EDT or EST is $utc_fixed which is server hour $server_hour - total fix $total_fix";

        //          echo "<br><br>remember that this time difference issue does not affect bar chart colors in top dir (android tablet data) because date2 is used, and is from the tablet, and there is code to handle the time zone difference";
         
         // end of block added 5-25-2021. 
         // end of block added 5-25-2021. 
         // end of block added 5-25-2021. 

                  

         $oldest_year_in_file = 0;  // not used in this file

         $pad_days = 0;

         $last_year = 2017;

         /*
            8-13-2017

               my time 8:26 am

               php time  12:18 = 1 pm meaning 4 hours ahead of me. server is in germany == this only happens between about 8pm and midnight if I am getting packets and keeping date2 up to date

               internet says berlin is 2:18, which probably means my minutes are off, and maybe there's a DST issue. 

                  but the main point is that they are way ahead of me, so at midnight their day of year and date should always be the new value.

               change tablet to use network time: before, it was 8:27 vs pc 8:29. after it was 8:21

               if php date is not same as my date, use php day - 1 since germany is ahead of me <- done below.
         */
         
          $first_day = -1;

          $old_index = 0;

          $fake_day = 0;

          while (($line = fgets($handle)) !== false) {

            /*
               need to use day of year from file to insert in proper array slot so that missing days are shown as zero

               the final value read is for yesterday but the idea still works. just use the first day as zero.


            */
              
               $numbers = explode(" ", trim($line));

               $index=0;

               $code = 0;

               $gallons = 0;                       // clear this in case of a blank line


               foreach ($numbers as &$number){     // if there is a blank line the code below will use the data from the preceding line

                  $number = intval($number);

                  if ($index == 0){

                     $code = $number;

                  } else {

                     $gallons = $number;
                  }

                  $index = $index + 1;
               }

               if ($gallons > 0){

                  $year = intval($code / 1000);

                  if ($last_year < $year){

                     $last_year = $year;   // so today is jan 1 2020 and it's showing jan 1 twice. gallons_new file has 2019 for every line except the last 2020

                     $pad_days += 365;    // since last_year is initialized as 2017 this will add 365 here, and now last_year == 2019. when we get to the final line I think pad_days will be 365 * 2

                     if ((($year-1) % 4) == 0){ // last year was leap year, and the year in the file just changed to the following year, so the last day of last year is 366

                        $pad_days ++;  

                     }

                  }

                  $day = intval($code - ($year * 1000));

                  if ($first_day == -1){
   
                     $first_day = $day;            // on january 1 this will be 1. maybe add 365 for 2018, and worry about 2019 later? or add 365/366 for each year beyond the earliest in the file?

                  }

                  $ra_index = $day - $first_day + $pad_days;   // on january 1, december 31 will be 365 - 1 = 364. ordinarily 

                  // forget leap year for the moment: HOW DOES THIS EVER WORK with 2017 as last_year initially? won't it add 365 every day of the year, which will screw up in every month except january? does it
                  // use the next block of code and add 365 phony elements to the array? I think it does, and maybe just ignore that error since it works?


                  // on jan 1 2020 this is 1 - 332 (value at top of file which has 35 lines) + 365 * 1 when reading the first few lines of the file. but when it gets to the end of the file 
                  // and sees 2020 it will add 365 + 1 to pad_days
                  //
                  // here's how pad_days is supposed to work, I think: during january, the first half of the file has days like 355, while the last half has days like 10. but in this situation
                  // first day will be something like 350, so ra_index might be 5 - 350 + 365 = 20 which is right since there are 15 days from december and 5 from jan
                  //
                  // but since last_year is initialized to 2017 how does it avoid adding 365 twice when today is january, meaning once for dec 2019 and again for jan 2020


                  if ($ra_index > ($old_index + 1)){   // example old 4, ra 6. loop will start at 5 and end at 5. this makes missing days appear as blanks instead of being skipped

                     for ($r5= $old_index + 1; $r5 < $ra_index; $r5++){

                        $gallons_ra[$r5] = 1;

                        $year_ra[$r5] = $year;

                        $day_ra[$r5] = $fake_day; 

                        $fake_day++;

                     }

                  }

                  $old_index = $ra_index;
 
                  $year_ra[$ra_index] = $year;

                  $day_ra[$ra_index] = $day;   // day is from datecode like 2017122 == 122 = day of year from java which uses 1..366 while php uses 0..365

                  $fake_day = $day + 1;

                  $gallons_ra[$ra_index] = $gallons;

               }  // if gallons on that line in the file is > 0

         } // read each line in gallons_new file

         $ra_index++; // old code had this extra inc, so it's easiest to use day for index by doing this 

          fclose($handle);

            $x = $plot_x2 - 2;

            if ($ra_index > 0){


               $day2 = intval(date("w")) - 1; // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight

                  // I subract 1 because the plot starts at yesterday

                  if ($need_to_fix_server_day == 1){ // for a few hours each night, UTC has already gone to the next day, so this corrects the bar colors during those hours

                     $day2 -= 1;
                  }

               if ($day2 < 0){   // day2 is used for the colors that make sunday easy to identify

                  $day2 = $day2 + 7; //6;

               }
/*
               if ($php_date != $date2){ // germany has already moved to tomorrow == this should only happen late in the day after 8 or so; after midnight my date2 should be the same

// php 8 broke because date2 not defined. date2 is old-style code for yesterday from the small time file. day2 is used for weekday colors in bar chart, and this is why new is not same color as old for sunday, etc.


                  $day2--;

                  if ($day2 < 0){

                     $day2 = 6;

                  }


               }
*/
               $days = array("S", "M", "T", "W", "T", "F", "S");  // is this ever used? not in this file.


               // with 24 pixels per day, 288 x 3 / 24 = 36 days.

               $bars = 30; // show this many days

               $dx = 864 / $bars;

               if ($ra_index > $bars){  // 288 * 3 = 2^5 x 3^3 so I can use  48 36 24 18 12

                  $stopx = $ra_index - $bars;

               } else {

                  $stopx = 0;

               }


//$m87 = 'abc '. $ra_index-1 . '  ' .  $stopx . ' ' . count($year_ra) . ' ' . count($day_ra) . ' ' . count($gallons_ra); // this is 60 and 31 

//imagettftext($my_img->img, 12, 0, (int) 7, $tank_y8 + 32 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text
               

               for ($i=$ra_index - 1; $i >=$stopx; $i--){  // undo the odd ++ up there.

                  
                     $year = $year_ra[$i]; 

                     $day = $day_ra[$i]; 

                     $gallons = $gallons_ra[$i];


//if($i == ($stopx)){            // 2023 30 1
//if($i == ($ra_index - 1)){     // 0 0 118
//if($i == ($ra_index - 2)){       // 2023 58 1         today is dec 12 2023  doy 346

//$m87 = 'abc '. $year . '  ' .  $day . ' ' . $gallons; 

//imagettftext($my_img->img, 12, 0, (int) 7, $tank_y8 + 32 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text
//}

                  if ($gallons > 1){

                     $y = $tank_y8 - $gallons; // gallons is x 100 so the range is already 0..200
                 
                     imagefilledrectangle ( $my_img->img , (int)$x-20 , (int) $y ,  (int)$x-2 , $tank_y8-2 ,$colors[$day2]);
                  }
                     
               
                  $x -= $dx;





                  // new code to fix leap day starts here. 3-1-2020


                  // 12-12-2023 this date33 looks wrong, but it's only to check leap year, i think
                  $date33 = DateTime::createFromFormat('Y z',$year . ' ' . $day);  // fixed for php 8.0.10 8-27-2021    java day 1..366  php day 0..355 so subtract 1

                  //$date33 = DateTime::createFromFormat('z Y', ($day ) . ' ' . $year);  // java day 1..366  php day 0..355 so subtract 1
                  
                  $isLeapYear = $date33->format('L');

                  if (($isLeapYear == 1) && ($day > 59)){

                        if ($day == 60)  {  // for feb 29 I cannot use the normal code because php appears to "fix" it. but I can create a dateTime if I inject feb 29 and the year

                           $date33 = DateTime::createFromFormat('n d Y',2 . ' ' . 29 . ' ' . $year); 

                        } else {

                           $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 2 )); // fixed for php 8.0.10 8-27-2021   java day 1..366  php day 0..355 so subtract 1. 
                                                                                                   // 5-16-2021 why do i subtract 2 here? see server_info.php which is online dream/x10_test... and demonstrates how the code works, and it looks ok
                           //$date33 = DateTime::createFromFormat('z Y', ($day - 2 ) . ' ' . $year); // java day 1..366  php day 0..355 so subtract 1. 5-16-2021 why do i subtract 2 here? see server_info.php which is online dream/x10_test... and
                                                                                                   // demonstrates how the code works, and it looks ok
                        }

                  }  else { // not leap year so use old code, also use this code for leap year days up to feb 28 = day of year 59 (based on jan 1 = 1 which is what is in my file; php uses jan 1 = 0 so we use $i3 -1 here)
 
                     // old code that worked until leap day 2020

                     $date33 = DateTime::createFromFormat('Y z', $year . ' ' . ($day - 1));  // fixed for php 8.0.10 8-27-2021    java day 1..366  php day 0..355 so subtract 1

                     //$date33 = DateTime::createFromFormat('z Y', ($day - 1) . ' ' . $year);  // java day 1..366  php day 0..355 so subtract 1
                  }



                  // this fails in 8.3, not sure why. uncaught error:    $m87 = $date33->format('d'); 
/*
                  try {


                     $m87 = 2; //$m87 = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1
                  } catch (Exception $e){

                  }


 12-12-2023 the day i swapped esp8266 corrupted gallons new file; maybe unrelated. do I need to sanitize input to that file?
2023340 105
2023341 123
2023342 114
2023343 118
2023344 141
zÿ?2023345 118      once i removed the trash from the beginning of this line it fixed the error.

i added this crap here because the php crash was saying this was the fatal error
*/
if (($day >= 0) && ($day < 367)){
                  $m87 = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1
} else {
                  $m87 = -1;
}
                  $date99 = intval($m87);

                  imagettftext($my_img->img, 12, 0, (int) $x + 7, $tank_y8 + 16 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text

                  if (($date99 == 1) || ($i == $stopx)){

                     $m87 = $date33->format('M');  // 'M' Jan   'F' January

                     imagettftext($my_img->img, 12, 0, (int) $x + 7, $tank_y8 + 32 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text
                  }









                  if ($debug < 6) {

                     $debug++;
                  }

                  $day2--;

                  if ($day2 < 0){

                     $day2 = 6;

                  }

               }  // for ra_index -1 down to stopx

               $debug++;

            }  // if ra_index > 0. I guess this handles empty files?


            $debug++;

            $debug++;



            /*

                  I should use plot stretch to make this easy to change in the future, so plot 288 days which is way more than enough, or maybe reduce it?

                     288 = 9 x 32  = 3^2 x 2^5 so there's a huge number of ways to divide it up integrally.

                        and include plot_stretch too...

                  so begin with 288 days and look at it. probably way too skinny.



                  bool imagefilledrectangle ( resource $image , int $x1 , int $y1 , int $x2 , int $y2 , int $color )

                  Creates a rectangle filled with color in the given image starting at point 1 and ending at point 2. 0, 0 is the top left corner of the image.

            */

         // watermark
            
         if ($need_to_fix_server_day == 1){ // for a few hours each night, UTC has already gone to the next day, so this corrects the bar colors during those hours

            imagettftext($my_img->img, 6, 0,  $left_pad * 1/4,  $tank_y2 + 10,  $text_colour, $lib_font, "fix");   // img, size, angle, x, y, color, font, text

         } else {
            
            imagettftext($my_img->img, 6, 0, (int)( $left_pad * 1/4),  $tank_y2 + 10 ,  $text_colour, $lib_font, "no_fix");   // img, size, angle, x, y, color, font, text

         }
             

      } else {
         
         imagettftext($my_img->img, 12, 0, (int)( $left_pad / 2), $new_y ,  $text_colour, $lib_font, "can't open gallons_new");   // img, size, angle, x, y, color, font, text
      }















?>
